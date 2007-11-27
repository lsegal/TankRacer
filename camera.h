#ifndef HAVE_CAMERA_H
#define HAVE_CAMERA_H

#include "bsp.h"

typedef struct Camera {
	float		position[3];
	float		viewAngles[3];
	float		upAngles[3];
	float		fov;
	float		aspectRatio;

	bspfile		*bsp;
	int			*faceList;
	int			 numFaces;
	int			*visitedFaces;
} Camera;

void Camera_Init(Camera *, bspfile *);
void Camera_Free(Camera *);
void Camera_Move(Camera *, float[3]);
void Camera_MoveInDirection(Camera *, float);
void Camera_Render(Camera *);

#endif /* HAVE_CAMERA_H */