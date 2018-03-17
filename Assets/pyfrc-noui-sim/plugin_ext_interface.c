/* Provides a wrapper interface for using the RobotPy simulation core from
 * other libraries / executables.
 */

#include <Python.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "robotpy_sim_core.h"

#if defined(__GNUC__)

#define CALL_CONV __attribute__((cdecl))

#elif defined(_MSC_VER)

#define CALL_CONV __cdecl

#endif

typedef void (*logging_func_ptr)(const char*);
logging_func_ptr logging_func = NULL;

void debug_log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vfprintf(stderr, fmt, args);

    if(logging_func != NULL) {
        char buf[256];
        vsnprintf(buf, 256, fmt, args);
        logging_func(buf);
    }

    va_end(args);
}

PyObject* log_interceptor_write(PyObject* self, PyObject* args) {
    const char *msg;
    if (!PyArg_ParseTuple(args, "s", &msg))
        return NULL;

    printf("%s", msg);
    if(logging_func != NULL) {
        logging_func(msg);
    }

    return Py_BuildValue("");
}

PyObject* log_interceptor_flush(PyObject* self, PyObject* args) {
    return Py_BuildValue("");
}

PyMethodDef log_interceptor_methods[] = {
    {"write", log_interceptor_write, METH_VARARGS, ""},
    {"flush", log_interceptor_flush, METH_VARARGS, ""},
    {0, 0, 0, 0},
};

PyModuleDef log_interceptor_module = {
    PyModuleDef_HEAD_INIT,
    "log_interceptor",
    "",
    -1,
    log_interceptor_methods,
};

PyMODINIT_FUNC PyInit_log_interceptor(void) {
    PyObject* m = PyModule_Create(&log_interceptor_module);
    PySys_SetObject("stdout", m);
    PySys_SetObject("stderr", m);
    return m;
}

/* Set a function to additionally write logging messages to. */
void CALL_CONV set_logging_function(logging_func_ptr func_ptr) {
    logging_func = func_ptr;
}


/* Initializes Python, loads the specified file, sets up RobotPy, and
 * starts a Robot class running.
 */
int CALL_CONV load_robot(const char* robot_file) {
    int ret = 0;
    PyObject *sim_core_module = NULL, *interceptor_module = NULL;
    PyObject *robot_filename = NULL, *robot_cls = NULL;

    if(!Py_IsInitialized()) {
        /* Do python and module init */
        PyImport_AppendInittab("robotpy_sim_core", PyInit_robotpy_sim_core);
        PyImport_AppendInittab("log_interceptor", PyInit_log_interceptor);

        Py_Initialize();

        debug_log("Python interpreter initialized, importing modules...");
    } else {
        debug_log("Python interpreter already initialized, importing modules...");
    }

    sim_core_module = PyImport_ImportModule("robotpy_sim_core");
    if(sim_core_module == NULL) {
        if(PyErr_Occurred())
            PyErr_Print();
        debug_log("Cannot import robotpy_sim_core module");

        ret = -3;
        goto out;
    }

    interceptor_module = PyImport_ImportModule("log_interceptor");
    if(interceptor_module == NULL) {
        if(PyErr_Occurred())
            PyErr_Print();
        debug_log("Cannot import log_interceptor module");

        ret = -4;
        goto out;
    }

    /* Load the specified python file */
    robot_filename = PyUnicode_DecodeFSDefault(robot_file);
    if(robot_filename == NULL) {
        if(PyErr_Occurred())
            PyErr_Print();
        debug_log("Cannot decode robot script filename");
        ret = -2;
        goto out;
    }

    robot_cls = load_robot_class(robot_filename);

    if(robot_cls != NULL) {
        /* call noui_sim.initialize_robot with the loaded robot class */
        initialize_robot(robot_cls);
        Py_DECREF(robot_cls);
    } else {
        if(PyErr_Occurred())
            PyErr_Print();
        debug_log("Cannot find Robot class in file \"%s\"\n", robot_file);

        ret = -1;
        goto out;
    }

out:
    Py_XDECREF(robot_filename);
    Py_XDECREF(sim_core_module);
    Py_XDECREF(interceptor_module);

    return ret;
}

/* Wraps tick_robot. */
void CALL_CONV robot_step() {
    tick_robot(0);
}

void CALL_CONV set_robot_mode(const char* mode, short enabled) {
    PyObject *pMode = NULL, *pEnabled = NULL;

    pMode = PyUnicode_DecodeUTF8(mode, strlen(mode), "strict");
    if(pMode == NULL || !PyUnicode_Check(pMode)) {
        if(PyErr_Occurred())
            PyErr_Print();
        debug_log("set_robot_mode: Could not decode mode");
        goto out;
    }

    pEnabled = PyBool_FromLong(enabled);
    if(pEnabled == NULL || !PyBool_Check(pEnabled)) {
        if(PyErr_Occurred())
            PyErr_Print();
        debug_log("set_robot_mode: Got invalid value for enabled");
        goto out;
    }

    set_robot_mode_impl(pMode, pEnabled);

    if(PyErr_Occurred())
        PyErr_Print();

out:
    Py_XDECREF(pMode);
    Py_XDECREF(pEnabled);
}

void CALL_CONV set_joystick_axis(int stick, int axis, float value) {
    set_joystick_axis_impl(stick, axis, value);
}

void CALL_CONV set_joystick_button(int stick, int btn, short value) {
    PyObject* pValue = NULL;
    pValue = PyBool_FromLong(value);

    if(pValue != NULL && PyBool_Check(pValue)) {
        set_joystick_button_impl(stick, btn, pValue);
    } else {
        if(PyErr_Occurred())
            PyErr_Print();
        debug_log("set_joystick_button: Invalid value passed for \'value\'");
    }

    Py_XDECREF(pValue);
}

/* Gets the PWM output value for a particular channel from hal_data. */
double CALL_CONV get_pwm_value(int channel) {
    PyObject *pwmDict = NULL, *pValue = NULL;
    double v = 0.0f;

    pwmDict = get_dict_in_hal_data("pwm", channel);

    if(pwmDict != NULL) {
        pValue = PyMapping_GetItemString(pwmDict, "value");

        if(pValue != NULL && PyFloat_Check(pValue)) {
            v = PyFloat_AsDouble(pValue);

            if(PyErr_Occurred()) {
                PyErr_Print();
                debug_log("get_pwm_value: Error occured converting value for PWM channel %i", channel);
                v = 0;
            }
        } else {
            if(PyErr_Occurred())
                PyErr_Print();
            debug_log("get_pwm_value: Could not get valid value for PWM channel %i", channel);
            v = 0;
        }
    } else {
        if(PyErr_Occurred())
            PyErr_Print();
        debug_log("get_pwm_value: Could not get HAL data for PWM channel %i", channel);
        v = 0;
    }

    Py_XDECREF(pwmDict);
    Py_XDECREF(pValue);
    return v;
}

/* Wraps Py_Finalize. */
void CALL_CONV finalize_python() {
    debug_log("Cleaning up robotpy_sim_core...");

    Py_Finalize();
    logging_func = NULL;
}
