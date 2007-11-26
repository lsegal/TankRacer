#include "common.h"

float vec3f_dot(float a[3], float b[3]) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
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