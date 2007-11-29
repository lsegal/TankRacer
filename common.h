#ifdef WIN32
#	include <windows.h>
#else
#	define BOOL int
#	define FALSE 0
#	define TRUE  1
#endif

#ifdef __APPLE__
#	define  GL_EXT_texture_env_combine 1
#	include <OpenGL/gl.h>
#	include <OpenGL/glu.h>
#	include	<OpenGL/glext.h>
#	include <GLUT/glut.h>
#else
#	include <GL/gl.h>
#	include <GL/glu.h>
#	include "glut.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#pragma comment(lib, "libjpeg.lib")
#pragma comment(lib, "glut32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

typedef unsigned char ubyte;

#include <math.h>
#define EPSILON 0.03125f
#define PI 3.141592653585
#define DEG2RAD(n) (PI * n / 180)
#define RAD2DEG(n) (180 * n / PI)
#define cosd(n)    cos(DEG2RAD(n))
#define sind(n)	   sin(DEG2RAD(n))
#define tand(n)    tan(DEG2RAD(n))

#define       vec3f_mag(vec) sqrt(vec3f_dot(vec, vec))
#define		  vec3f_set(src, dst) vec3f_scale(src, 1.0f, dst)
extern float  vec3f_dot(float[3], float[3]);
extern float *vec3f_scale(float[3], float, float[3]);
extern float *vec3f_norm(float[3]);
extern float *vec3f_add(float[3], float[3], float[3]);
extern float *vec3f_sub(float[3], float[3], float[3]);
extern float  vec3f_dist(float[3], float[3]);
extern float  vec3f_classify(float[3], float[3], float);

extern float *mat4f_mult(float[16], float[16], float[16]);

extern void  text_output(int, int, char *, ...);