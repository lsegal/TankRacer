#ifdef __CPLUSPLUS
extern "C" {
#endif

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
#	include "opengl/glut.h"
#	include "opengl/glext.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#pragma comment(lib, "libjpeg/libjpeg.lib")
#pragma comment(lib, "opengl/glut32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

typedef unsigned char ubyte;

#include <math.h>
#define EPSILON 0.003125f
#define PI 3.141592653585
#define DEG2RAD(n) (PI * n / 180)
#define RAD2DEG(n) (180 * n / PI)
#define cosd(n)    cos(DEG2RAD(n))
#define sind(n)	   sin(DEG2RAD(n))
#define tand(n)    tan(DEG2RAD(n))

/* Symbolic names for array indices */
const enum AxisNames { X, Y, Z } AxisNames;
const enum RotationNames { ROLL, YAW, PITCH } RotationNames;

#define       vec3f_mag(vec) sqrt(vec3f_dot(vec, vec))
#define		  vec3f_set(src, dst) vec3f_scale(src, 1.0f, dst)
#define		  vec3f_clear(vec) memset(vec, 0, sizeof(float[3]))
extern float  vec3f_dot(float[3], float[3]);
extern float *vec3f_cross(float[3], float[3], float[3]);
extern float *vec3f_normalp(float[3], float[3], float[3], float[3]);
extern float *vec3f_scale(float[3], float, float[3]);
extern float *vec3f_norm(float[3]);
extern float *vec3f_add(float[3], float[3], float[3]);
extern float *vec3f_sub(float[3], float[3], float[3]);
extern float  vec3f_dist(float[3], float[3]);
extern float  vec3f_planedist(float[3], float[3], float[3], float[3]);
extern float  vec3f_classify(float[3], float[3], float);
extern float *vec3f_rotp(float[3], float[3], float[3], float, float[3]);

extern int	  point_in_hitbox(float[3], float[8][3]);

extern float *mat4f_mult(float[16], float[16], float[16]);

extern void  text_output(int, int, char *, ...);
extern void  text_output2(int, int, void *, char *, ...);
extern void  draw_cube(double, double, double);

extern GLuint load_texture_jpeg(char *);
extern GLuint load_texture_tga(char *);
extern GLuint load_texture_raw(const char *, int, int, int);

#ifdef __CPLUSPLUS
}
#endif