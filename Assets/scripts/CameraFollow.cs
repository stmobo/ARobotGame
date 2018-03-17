using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraFollow : MonoBehaviour {
    public GameObject objectToFollow;
    private Transform xfmToFollow;

	// Use this for initialization
	void Start () {
        xfmToFollow = objectToFollow.transform;
	}
	
	// Update is called once per frame
	void LateUpdate () {
        Vector3 newPos = xfmToFollow.position;
        newPos.z = -10f;
        transform.position = newPos;
	}
}
