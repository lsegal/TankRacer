#include "common.h"

float vec3f_dot(float a[3], float b[3]) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float *vec3f_norm(float vec[3]) {
	float n = vec3f_mag(vec);
	if (n != 0) vec3f_scale(vec, 1.0/n, vec);
	return vec;
}

float *vec3f_scale(float vec[3], float scale, float out[3]) {
	memcpy(out, vec, sizeof(float[3]));
	if (scale != 1.0) {
		out[0] *= scale; out[1] *= scale; out[2] *= scale;
	}
	return out;
}

float *vec3f_sub(float begin[3], float end[3], float dir[3]) {
	int i;
	for (i = 0; i < 3; i++) dir[i] = end[i] - begin[i];
	return dir;
}

float *vec3f_add(float begin[3], float end[3], float dir[3]) {
	int i;
	for (i = 0; i < 3; i++) dir[i] = begin[i] + end[i];
	return dir;
}

float vec3f_dist(float p1[3], float p2[3]) {
	float dv[3];
	return vec3f_mag(vec3f_sub(p1, p2, dv));
}

/* Returns -1 if a point is behind a plane, 0 if it lies in the plane and 1 if it's in front */
float vec3f_classify(float point[3], float plane[3], float intercept) {
	float dist = vec3f_dot(point, plane) - intercept;
	if (dist < -EPSILON) return -1;
	if (dist >  EPSILON) return  1;
	else				 return  0;
}

float *mat4f_mult(float m1[16], float m2[16], float out[16]) {
    int i, j, k;
    float sum;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			sum = 0;
			for (k = 0; k < 4; k++) {
				sum += m1[j*4+k] * m2[k*4+i];
			}
			out[j*4+i] = sum;
		}
	}

	return out;
}

void text_output(int x, int y, char *fmt, ...) {
	char buf[1024];
	int i = 0;

	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 1024, fmt, args);
	va_end(args);

	glRasterPos2f(x, y);
	while (buf[i]) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buf[i++]);
	}
}