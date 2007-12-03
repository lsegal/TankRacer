#ifndef HAVE_CAMERA_H
#define HAVE_CAMERA_H

#ifdef __CPLUSPLUS
extern "C" {
#endif

#include "bsp.h"

typedef struct Camera {
	float		position[3];	/* Camera position */
	float		direction[3];	/* Look direction  */
	float		upAngles[3];	
	float		fov;			/* Field of view */
	float		aspectRatio;	/* Aspect ratio */

	bspfile		*bsp;
	leaf		*currentLeaf;
	int			*faceList;
	int			 numFaces;
	int			*visitedFaces;

	float		frustum[6][4];
} Camera;

void Camera_Init(Camera *, bspfile *);
void Camera_Free(Camera *);
void Camera_SetPosition(Camera *, float[3], float[3]);
void Camera_Move(Camera *, float[3]);
void Camera_MoveInDirection(Camera *, float);
void Camera_Render(Camera *);

#ifdef __CPLUSPLUS
}
#endif

#endif /* HAVE_CAMERA_H */