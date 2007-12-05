#include "common.h"
#include "bsp.h"
#include "game.h"

#ifdef WIN32
#	include "extensions/ARB_multitexture_extension.h"
#endif

static void init_gl() {
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ACCUM);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(600, 150);
	glutCreateWindow("Tank Racer");

#ifdef WIN32
	/* Initialize GL extensions */
	SetUpARB_multitexture();
#endif

	/* Initialize basic clear colour/depth setting */
    glClearColor(1.0, 1.0, 1.0, 0.0);
	
	/* Enable texturing */
	glEnable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);

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

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	init_gl();
	Menu_Init();

	glutMainLoop();

	return 0;
}
