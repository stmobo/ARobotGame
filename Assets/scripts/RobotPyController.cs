using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public class RobotPyController : MonoBehaviour {
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void LoggingDelegate(string msg);

    static void LoggingCallback(string msg)
    {
        Debug.Log("[Python] " + msg);
    }

    [DllImport("robotpy_sim_core", CallingConvention = CallingConvention.Cdecl)]
    private static extern int load_robot(string file);

    [DllImport("robotpy_sim_core", CallingConvention = CallingConvention.Cdecl)]
    private static extern void robot_step();

    [DllImport("robotpy_sim_core", CallingConvention = CallingConvention.Cdecl)]
    private static extern void set_robot_mode(string mode, bool enabled);

    [DllImport("robotpy_sim_core", CallingConvention = CallingConvention.Cdecl)]
    private static extern double get_pwm_value(int channel);

    [DllImport("robotpy_sim_core", CallingConvention = CallingConvention.Cdecl)]
    private static extern void finalize_python();

    [DllImport("robotpy_sim_core", CallingConvention = CallingConvention.Cdecl)]
    private static extern void set_logging_function(IntPtr fp);

    [DllImport("robotpy_sim_core", CallingConvention = CallingConvention.Cdecl)]
    private static extern void set_joystick_axis(int stick, int axis, float value);

    [DllImport("robotpy_sim_core", CallingConvention = CallingConvention.Cdecl)]
    private static extern void set_joystick_button(int stick, int axis, bool value);

    private MovementController move_ctrl; 

    // Use this for initialization
    void Start () {
        move_ctrl = GetComponent<MovementController>();

        LoggingDelegate logging_delegate = new LoggingDelegate(LoggingCallback);
        IntPtr fn_ptr = Marshal.GetFunctionPointerForDelegate(logging_delegate);

        set_logging_function(fn_ptr);

        int ret = load_robot(Path.Combine(Application.dataPath, "robot.py"));
        print("Loaded robot, retval = " + ret.ToString());

        set_robot_mode("teleop", true);
	}
    
    void FixedUpdate () {
        float horiz = Input.GetAxis("Horizontal");
        float vert = Input.GetAxis("Vertical");

        set_joystick_axis(0, 0, horiz);
        set_joystick_axis(0, 1, vert);

        robot_step();

        double pwm_left = get_pwm_value(0);
        double pwm_right = get_pwm_value(1);

        move_ctrl.left = (float)pwm_left;
        move_ctrl.right = (float)pwm_right;
	}

    void OnDisable()
    {
        finalize_python();
    }
}
