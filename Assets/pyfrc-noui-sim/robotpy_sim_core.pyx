import wpilib
import hal_impl.sim_hooks
import hal_impl.functions
import hal_impl.mode_helpers
from hal_impl.data import hal_data
import threading
import time

def _robot_thread_main(robot):
    try:
        robot.startCompetition()
    except KeyboardInterrupt:
        pass
    except:
        raise

cpdef public void tick_robot():
    hal_impl.functions.hooks.notifyDSData()

cdef public void set_robot_mode_impl(mode, enabled):
    hal_impl.mode_helpers.set_mode(mode, enabled)

cdef public object get_hal_data():
    return hal_data

cdef public object get_dict_in_hal_data(char *main_key, int channel):
    key = (<bytes>main_key).decode('utf-8')

    if isinstance(hal_data[key], list):
        return hal_data[key][channel]
    else:
        return hal_data[key]

cdef public void set_joystick_axis_impl(int joystick_index, int axis, float val):
    hal_data['joysticks'][joystick_index]['axes'][axis] = val

cdef public void set_joystick_button_impl(int joystick_index, int btn, val):
    hal_data['joysticks'][joystick_index]['buttons'][btn] = val

cdef public object load_robot_class(filename):
    import importlib.util

    spec = importlib.util.spec_from_file_location("robot_mod", filename)
    robot_module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(robot_module)

    return getattr(robot_module, 'Robot')

cdef public void initialize_robot(object robot_cls):
    import pyfrc
    from pyfrc import config
    from pyfrc import sim

    config.mode = 'sim'

    hal_impl.functions.reset_hal()
    wpilib.RobotBase.initializeHardwareConfiguration()

    robot = robot_cls()

    # Initialization done, start the robot now...
    print("Starting robot thread...")
    robot_thread = threading.Thread(
        target=_robot_thread_main,
        args=(robot,),
        daemon=True
    )
    robot_thread.start()
