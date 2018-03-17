using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SpinningBladeController : MonoBehaviour {
    HingeJoint2D weaponHinge;
    Rigidbody2D weaponRB;
    float motForce = 1000f;

    public float motPwr = 0.0f;

    void Start()
    {
        weaponHinge = GetComponent<HingeJoint2D>();
        weaponRB = GetComponent<Rigidbody2D>();
    }

    void FixedUpdate()
    {
        weaponRB.AddForceAtPosition(
            transform.right * motPwr * motForce,
            transform.position + (transform.up * 0.5f)
        );
    }
}