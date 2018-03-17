using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MovementController : MonoBehaviour {
    Animator animator;
    Rigidbody2D rb2d;

    private float driveBase = 0.4f; // unity units
    private float maxVel = 0.75f; // unity units per second

    public float left = 0.0f;
    public float right = 0.0f;

	// Use this for initialization
	void Start () {
        animator = GetComponent<Animator>();
        rb2d = GetComponent<Rigidbody2D>();
    }

	void FixedUpdate () {
        //float left = Mathf.Clamp(vert + horiz, -1.0f, 1.0f);
        //float right = Mathf.Clamp(vert - horiz, -1.0f, 1.0f);

        float left_vel = left * maxVel;
        float right_vel = right * maxVel;

        float vfwd = (left_vel + right_vel) / 2.0f;
        float vrot = Mathf.Rad2Deg * (right_vel - left_vel) / driveBase;

        rb2d.velocity = transform.right * vfwd; // yes, right-- look at the red axis
        rb2d.angularVelocity = vrot;

        animator.SetFloat("Left", left);
        animator.SetFloat("Right", right);
    }
}
