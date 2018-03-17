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
logging_func_ptr logging_func;

void debug_log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vfprintf(stderr, fmt, args);

    char buf[256];
    vsnprintf(buf, 256, fmt, args);
    logging_func(buf);

    va_end(args);
}

PyObject* log_interceptor_write(PyObject* self, PyObject* args) {
    const char *msg;
    if (!PyArg_ParseTuple(args, "s", &msg))
        return NULL;

    printf("%s", msg);
    logging_func(msg);

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
    /* Do python and module init */
    PyImport_AppendInittab("robotpy_sim_core", PyInit_robotpy_sim_core);
    PyImport_AppendInittab("log_interceptor", PyInit_log_interceptor);

    Py_Initialize();

    PyObject* sim_core_module = PyImport_ImportModule("robotpy_sim_core");
    PyObject* interceptor_module = PyImport_ImportModule("log_interceptor");

    /* Load the specified python file */
    PyObject* robot_filename = PyUnicode_DecodeFSDefault(robot_file);
    PyObject* robot_cls = load_robot_class(robot_filename);

    Py_DECREF(robot_filename);

    if(robot_cls != NULL) {
        /* call noui_sim.initialize_robot with the loaded robot class */
        initialize_robot(robot_cls);
        Py_DECREF(robot_cls);
    } else {
        if(PyErr_Occurred())
            PyErr_Print();
        debug_log("Cannot find Robot class in file \"%s\"\n", robot_file);
        return -1;
    }

    Py_DECREF(interceptor_module);
    Py_DECREF(sim_core_module);

    return 0;
}

/* Wraps tick_robot. */
void CALL_CONV robot_step() {
    tick_robot(0);
}

void CALL_CONV set_robot_mode(const char* mode, short enabled) {
    PyObject* pMode = PyUnicode_DecodeUTF8(mode, strlen(mode), "strict");
    PyObject* pEnabled = PyBool_FromLong(enabled);

    set_robot_mode_impl(pMode, pEnabled);

    if(PyErr_Occurred())
        PyErr_Print();

    Py_XDECREF(pMode);
    Py_XDECREF(pEnabled);
}

void CALL_CONV set_joystick_axis(int stick, int axis, float value) {
    set_joystick_axis_impl(stick, axis, value);
}

void CALL_CONV set_joystick_button(int stick, int btn, short value) {
    PyObject* pValue = PyBool_FromLong(value);

    set_joystick_button_impl(stick, btn, pValue);

    Py_XDECREF(pValue);
}

/* Gets the PWM output value for a particular channel from hal_data. */
double CALL_CONV get_pwm_value(int channel) {
    PyObject* pwmDict = get_dict_in_hal_data("pwm", channel);

    if(pwmDict != NULL) {
        PyObject* pValue = PyMapping_GetItemString(pwmDict, "value");
        Py_DECREF(pwmDict);

        if(pValue != NULL && PyFloat_Check(pValue)) {
            double v = PyFloat_AsDouble(pValue);
            Py_DECREF(pValue);

            if(!PyErr_Occurred()) {
                return v;
            } else {
                PyErr_Print();
                debug_log("Error occured converting value for PWM channel %i", channel);
                return 0.0f;
            }
        } else {
            Py_XDECREF(pValue);

            if(PyErr_Occurred())
                PyErr_Print();
            debug_log("Could not get valid value for PWM channel %i", channel);
            return 0.0f;
        }
    } else {
        if(PyErr_Occurred())
            PyErr_Print();
        debug_log("Could not get HAL data for PWM channel %i", channel);
        return 0.0f;
    }
}

/* Wraps Py_Finalize. */
void CALL_CONV finalize_python() {
    Py_Finalize();
}
