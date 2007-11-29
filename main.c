#include "common.h"
#include "bsp.h"
#include "game.h"

#ifdef WIN32
#	include "extensions/ARB_multitexture_extension.h"
#endif

static void resize(int w, int h) {
	Game_Resize(w, h);
}

static void run(void) {
	Game_Run();
	glutPostRedisplay();
}

static void render(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Game_Render();
	glutSwapBuffers();
}

static void init_gl() {
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ACCUM);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(600, 150);
	glutCreateWindow("Quake3BSP");
	glutIdleFunc(run);
	glutDisplayFunc(render);
	glutReshapeFunc(resize);

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

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	init_gl();
	Game_Init();

	glutMainLoop();

	return 0;
}
