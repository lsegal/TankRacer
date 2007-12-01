#ifndef OBJECT_H
#define OBJECT_H

typedef void (ObjectCallbackFunc)(void*,void*);

typedef struct Object {
	float position[3];		/* Position point */
	float direction[3];		/* The direction the tank points in (not to be confused with its velocity) */
	float velocity[3];		/* Velocity vector */
	float upAngles[3];		/* Up angle (normalized vector) */
	float force[3];			/* Force vector (not normalized) */
	float frame;			/* Current animation frame */
	float speed;			/* Magnitude component of velocity */
	float accel;			/* Magnitude component of acceleration */
	float maxSpeed;			/* Maximum speed magnitude */
	float maxAccel;			/* Maximum accel magnitude */
	float mass;				/* Mass for force calculation */
	float length;			/* Object length */
	float width;			/* Object width */
	float height;			/* Object height */
	int	  onGround;			/* Is object on the ground */

	ObjectCallbackFunc *thinkFunc;		/* Think function called by world */
	ObjectCallbackFunc *drawFunc;		/* Draw function called by world */
	void *funcData;						/* Object specific data passed to callback functions */
} Object;

#endif /* OBJECT_H */