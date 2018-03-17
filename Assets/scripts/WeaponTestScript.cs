using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WeaponTestScript : MonoBehaviour {
    HingeJoint2D weaponHinge;
    Rigidbody2D weaponRB;
    float motForce = 1000f;

    void Start()
    {
        weaponHinge = GetComponent<HingeJoint2D>();
        weaponRB = GetComponent<Rigidbody2D>();
    }

    void FixedUpdate()
    {
        float motPwr = Input.GetAxis("Fire1");

        weaponRB.AddForceAtPosition(
            transform.right * motPwr * motForce,
            transform.position + (transform.up * 0.5f)
        );
    }
}