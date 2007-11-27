#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glext.h"
#include "glut.h"

#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "libjpeg.lib")
#pragma comment(lib, "glut32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

typedef unsigned char ubyte;

#include <math.h>
#define PI 3.141592653585
#define DEG2RAD(n) (PI * n / 180)
#define RAD2DEG(n) (180 * n / PI)
#define cosd(n)    cos(DEG2RAD(n))
#define sind(n)	   sin(DEG2RAD(n))
#define tand(n)    tan(DEG2RAD(n))

#define       vec3f_mag(vec) sqrt(vec3f_dot(vec, vec))
extern float  vec3f_dot(float[3], float[3]);
extern float *vec3f_scale(float[3], float, float[3]);
extern float *vec3f_norm(float[3]);
extern float *vec3f_add(float[3], float[3], float[3]);
extern float *vec3f_sub(float[3], float[3], float[3]);
extern float  vec3f_dist(float[3], float[3]);

extern void  text_output(int, int, char *, ...);