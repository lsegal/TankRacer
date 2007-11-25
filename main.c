#include "common.h"
#include "bsp.h"

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"
#include "glext.h"
#include "extensions/ARB_multitexture_extension.h"

double aspect, angles[2], origin[3] = {0,1,0};
double width, height;
bspfile *bsp;

int moving = 0;

static void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	width = w;
	height = h;
	aspect = (double)w / (double)h;
}

static void display(void) {
	GLfloat pos[] = { -0.3, 0.7, -2.0, 1.0 };
	GLfloat spec[] = { 0.7, 0.7, 0.7, 1.0 };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, aspect, 0.01, 100000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (moving) {
		origin[0] += 0.1 * cosd(angles[0] / 2);
		origin[2] += 0.1 * sind(angles[0] / 2);
	}

	gluLookAt(origin[0], origin[1], origin[2], 
		origin[0] + cosd(angles[0] / 2), 
		origin[1] + tand(angles[1] / 5), 
		origin[2] + sind(angles[0] / 2), 0, 1, 0);

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	bsp_draw(bsp);

	glutSwapBuffers();
}

int lastx = 0, lasty = 0;
static void mouse(int x, int y) {
	double a, b;
	a = x - width/2;
	b = y - height/2;
	angles[0] += a - lastx;
	angles[1] += b - lasty;
	lastx = a;
	lasty = b;

	glutPostRedisplay();
}

static void key(unsigned char key, int x, int y) {
	if (key == 27) exit(0);
	if (key == 'd') angles[0] += 10;
	if (key == 'a') angles[0] -= 10;
}

static void mousebutton(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON) {
		moving = (state == GLUT_DOWN ? 1 : 0);
	}
	glutPostRedisplay();
}

static void init_gl() {
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ACCUM);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(400, 150);
	glutCreateWindow("Quake3BSP");
	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);
	glutPassiveMotionFunc(mouse);
	glutKeyboardFunc(key);
	glutMouseFunc(mousebutton);

	/* Initialize GL extensions */
	SetUpARB_multitexture();

	/* Initialize basic clear colour/depth setting */
    glClearColor(1.0, 1.0, 1.0, 1.0);
	
	/* Enable texturing */
	glEnable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);

	/* Texture settings for lightmap modulation */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2.0f);

	/* Enable lighting */
//	glEnable(GL_LIGHTING);
//	glEnable(GL_COLOR_MATERIAL);
//	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	/* Smooth shading */
	glShadeModel(GL_SMOOTH);

	/* Enable depth buffer so we can remove reverse facing polys */
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

static void init_bsp() {
	bsp = bsp_load("test1.bsp");
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	init_gl();
	init_bsp();

	glutMainLoop();

	return 0;
}
