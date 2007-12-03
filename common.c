#include "common.h"
#include <stdarg.h>

/* Dot product of two vectors */
float vec3f_dot(float a[3], float b[3]) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

/* Normalizes a vector */
float *vec3f_norm(float vec[3]) {
	float n = vec3f_mag(vec);
	if (n != 0) vec3f_scale(vec, 1.0/n, vec);
	return vec;
}

/* Cross product of two vectors */
float *vec3f_cross(float v1[3], float v2[3], float out[3]) {
	out[0] = v1[1] * v2[2] - v2[1] * v1[2];
	out[1] = v1[0] * v2[2] - v2[0] * v1[2];
	out[2] = v1[0] * v2[1] - v2[0] * v1[1];
	return out;
}

/* Returns the (normalized) `normal` given three points lying in a plane (`p1`, `p2`, `p3`) */
float *vec3f_normalp(float p1[3], float p2[3], float p3[3], float normal[3]) {
	float l1[3], l2[3];
	vec3f_sub(p3, p1, l1);
	vec3f_sub(p2, p1, l2);
	return vec3f_norm(vec3f_cross(p1, p2, normal));
}

/* Scales a vector by `scale` into `out` */
float *vec3f_scale(float vec[3], float scale, float out[3]) {
	memcpy(out, vec, sizeof(float[3]));
	if (scale != 1.0) {
		out[0] *= scale; out[1] *= scale; out[2] *= scale;
	}
	return out;
}

/* Subtracts vector `end` by `begin` and stores result in `dir` */
float *vec3f_sub(float begin[3], float end[3], float dir[3]) {
	int i;
	for (i = 0; i < 3; i++) dir[i] = end[i] - begin[i];
	return dir;
}

/* Adds vector `end` to `begin` and stores the result in `dir` */
float *vec3f_add(float begin[3], float end[3], float dir[3]) {
	int i;
	for (i = 0; i < 3; i++) dir[i] = begin[i] + end[i];
	return dir;
}

/* Returns the vector distance between two points */
float vec3f_dist(float p1[3], float p2[3]) {
	float dv[3];
	return vec3f_mag(vec3f_sub(p1, p2, dv));
}

/* Returns -1 if a point is behind a plane, 0 if it lies in the plane and 1 if it's in front */
float vec3f_classify(float point[3], float plane[3], float intercept) {
	float dist = vec3f_dot(point, plane) - intercept;
	if (dist < -EPSILON) return -1; /* Behind   */
	if (dist >  EPSILON) return  1; /* In front */
	else				 return  0; /* On plane */
}

/* Rotates point `p1` by `angle` degrees around line `line` passing through point `p2` */
float *vec3f_rotp(float p1[3], float p2[3], float line[3], float angle, float out[3]) {
	float x, y, z, a, b, c, u, v, w, k, ca, sa, mag;
	mag = vec3f_mag(line);
	ca = cosd(angle); sa = sind(angle);
	x = p1[0]; y = p1[1]; z = p1[2];
	a = p2[0]; b = p2[1]; c = p2[2];
	u = line[0] / mag; v = line[1] / mag; w = line[2] / mag;

	/* 
	 * Here comes the fun 
	 * Ref: http://www.mines.edu/~gmurray/ArbitraryAxisRotation/ArbitraryAxisRotation.html 
	 *
	 * This could be heavily optimized, but there isn't enough time in the day...
	 */
	k = v*v + w*w;
	out[0] = a*k + u * (-b*v - c*w + u*x + v*y + w*z) + 
		((x-a)*k + u * (b*v + c*w - v*y - w*z)) * ca + 
		mag * (b*w - c*v - w*y + v*z) * sa;

	k = u*u + w*w;
	out[1] = b*k + v * (-a*u - c*w + u*x + v*y + w*z) + 
		((y-b)*k + v * (a*u + c*w - u*x - w*z)) * ca +
		mag * (-a*w + c*u + w*x - u*z) * sa;

	k = u*u + v*v;
	out[2] = c*k + w * (-a*u - b*v + u*x + v*y + w*z) + 
		((z-c)*k + w * (a*u + b*v - u*x - v*y)) * ca +
		mag * (a*v - b*u - v*x + u*y) * sa;

	mag = u*u + v*v + w*w;
	out[0] /= mag; out[1] /= mag; out[2] /= mag;

	return out;
}

/* Returns nonzero if a point is inside a hitbox */
int point_in_hitbox(float p[3], float hitbox[8][3]) {
	int compare[6] = {0,0,0,0,0,0};
	int i;

	/* Basic algorithm:
	 * Count the number of x values smaller, number of x values greater, y values smaller, etc..
	 * and then make sure that each count is equal to 4, since there should be 4 points smaller
	 * than the reference point on x, 4 more pointers greater on x with the same on y and z.
	 *
	 * Note that the term "greater" and "smaller" should be read as "X or equal to". This also means
	 * that "4 points" should be read as "at least 4 points". This will only occur if one dimension
	 * of the hitbox is zero and the point lies on the plane.
	 *
	 * The benefit of this method is that we don't care which sides of the hitbox are right, left,
	 * top, bottom, back, front.
	 */
	for (i = 0; i < 8; i++) {
		if (hitbox[i][0] >= p[0]) compare[0]++;
		if (hitbox[i][0] <= p[0]) compare[1]++;
		if (hitbox[i][1] >= p[1]) compare[2]++;
		if (hitbox[i][1] <= p[1]) compare[3]++;
		if (hitbox[i][2] >= p[2]) compare[4]++;
		if (hitbox[i][2] <= p[2]) compare[5]++;
	}

	/* Check for hit */
	for (i = 0; i < 6; i++) {
		if (compare[i] < 4) return 0;
	}

	return 1;
}

/* Distance from a point to a plane in a specific vector direction */
float vec3f_planedist(float point[3], float dir[3], float plane[3], float planepoint[3]) {
	float tmp[3];
	vec3f_sub(planepoint, point, tmp);
	return -vec3f_dot(plane, tmp) / vec3f_dot(plane, dir);
}

/* Multiplies a 4x4 matrix by another */
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

/* Outputs text on the screen, use glColor*() to set the colours prior to call */
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

/* Outputs text on the screen, use glColor*() to set the colours prior to call */
void text_output2(int x, int y, void *font, char *fmt, ...) {
	char buf[1024];
	int i = 0;

	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 1024, fmt, args);
	va_end(args);

	glRasterPos2f(x, y);
	while (buf[i]) {
		glutBitmapCharacter(font, buf[i++]);
	}
}

/* Draws a cube centered around 0,0,0 with length/height/width of 
 * wx/wy/wz respectively, length being calculated on x, height being
 * calculated on y and width on z. This call also can map textures, unlike
 * glutSolidCube
 */
void draw_cube(double wx, double wy, double wz) {
	wx /= 2; wy /= 2; wz /= 2; /* Simplify expressions by dividing by 2 here */
	glBegin(GL_QUADS);
		/* Top side, meaning all points are located at Y=wy/2 */
		glNormal3d(0, 1, 0); /* Normal faces up */
		glTexCoord2d(0, 0); glVertex3d(-wx, wy,  wz); 
		glTexCoord2d(1, 0); glVertex3d( wx, wy,  wz); 
		glTexCoord2d(1, 1); glVertex3d( wx, wy, -wz); 
		glTexCoord2d(0, 1); glVertex3d(-wx, wy, -wz); 

		/* Front side, meaning all points are located at Z=wz/2 */
		glNormal3d(0, 0, 1); /* Normal faces up */
		glTexCoord2d(0, 0); glVertex3d(-wx, -wy, wz);  
		glTexCoord2d(1, 0); glVertex3d( wx, -wy, wz); 
		glTexCoord2d(1, 1); glVertex3d( wx,  wy, wz);
		glTexCoord2d(0, 1); glVertex3d(-wx,  wy, wz);

		/* Bottom side, meaning all points are located at Y=-wy/2 */
		glNormal3d(0, 1, 0); /* Normal faces down */
		glTexCoord2d(0, 0); glVertex3d(-wx, -wy, -wz); 
		glTexCoord2d(1, 0); glVertex3d( wx, -wy, -wz); 
		glTexCoord2d(1, 1); glVertex3d( wx, -wy,  wz); 
		glTexCoord2d(0, 1); glVertex3d(-wx, -wy,  wz); 

		/* Back side, meaning all points are located at Z=-wz/2 */
		glNormal3d(0, 0, -1); /* Normal faces up */
		glTexCoord2d(0, 0); glVertex3d(-wx,  wy, -wz); 
		glTexCoord2d(1, 0); glVertex3d( wx,  wy, -wz); 
		glTexCoord2d(1, 1); glVertex3d( wx, -wy, -wz); 
		glTexCoord2d(0, 1); glVertex3d(-wx, -wy, -wz); 

		/* Left side, meaning all points are located at X=-wx/2 */
		glNormal3d(1, 0, 0); /* Normal faces up */
		glTexCoord2d(0, 0); glVertex3d(-wx, -wy,  wz); 
		glTexCoord2d(1, 0); glVertex3d(-wx,  wy,  wz); 
		glTexCoord2d(1, 1); glVertex3d(-wx,  wy, -wz);
		glTexCoord2d(0, 1); glVertex3d(-wx, -wy, -wz);

		/* Right side, meaning all points are located at X=wx/2 */
		glNormal3d(-1, 0, 0); /* Normal faces up */
		glTexCoord2d(0, 0); glVertex3d(wx,  wy, -wz); 
		glTexCoord2d(1, 0); glVertex3d(wx,  wy,  wz); 
		glTexCoord2d(1, 1); glVertex3d(wx, -wy,  wz); 
		glTexCoord2d(0, 1); glVertex3d(wx, -wy, -wz); 
	glEnd();
}