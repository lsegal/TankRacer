#include "common.h"
#include "bsp.h"
#include "camera.h"
#include "extensions/ARB_multitexture_extension.h"

float width, height;
bspfile *bsp;
Camera  *camera[2];

float dir = 1;
int moving = 0;

static void cleanup() {
	bsp_free(bsp);
	//Camera_Free(camera[0]);
	//Camera_Free(camera[1]);
	exit(0);
}

static void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	width = w;
	height = h;
}

static void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (moving) {
		Camera_MoveInDirection(camera[0], dir / 5);
		if (camera[1]) Camera_SetPosition(camera[1], camera[0]->position);
	}

	if (camera[1]) {
		glViewport(0, height/2, width, height/2);
		camera[0]->aspectRatio = 2 * width / height;
		Camera_Render(camera[0]);

		glViewport(0, 0, width, height/2);
		camera[1]->aspectRatio = 2 * width / height;
		Camera_Render(camera[1]);
	}
	else {
		camera[0]->aspectRatio = 2 * width / height;
		Camera_Render(camera[0]);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3d(1, 0, 0);
	text_output(2, 2, "Coordinates: %.2f, %.2f, %.2f", 
		camera[0]->position[0], camera[0]->position[1], camera[0]->position[2]);

	glutSwapBuffers();
}

int lastx = 0, lasty = 0;
static void mouse(int x, int y) {
	double a, b;
	a = x - width/2;
	b = y - height/2;
	camera[0]->viewAngles[0] += a - lastx;
	camera[0]->viewAngles[1] += b - lasty;
	if (camera[1]) {
		camera[1]->viewAngles[0] += a - lastx;
		camera[1]->viewAngles[1] -= b - lasty;
	}
	lastx = a;
	lasty = b;

	glutPostRedisplay();
}

static void key(unsigned char key, int x, int y) {
	if (key == 27) {
		cleanup();
	}
	if (key == 'd') {
		camera[0]->viewAngles[0] -= 1;
		if (camera[1]) camera[1]->viewAngles[0] -= 1;
	}
	if (key == 'a') {
		camera[0]->viewAngles[0] += 1;
		if (camera[1]) camera[1]->viewAngles[0] -= 1;
	}
	if (key == 's') {
		dir = -1;
		moving = 1;
	}
	if (key == 'w') {
		dir = 1;
		moving = 1;
	}
}

static void keyup(unsigned char key, int x, int y) {
	if (key == 's') {
		moving = 0;
	}
	if (key == 'w') {
		moving = 0;
	}
}


static void mousebutton(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON) {
		dir = 1;
		moving = (state == GLUT_DOWN ? 1 : 0);
	}
	glutPostRedisplay();
}

static void init_gl() {
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ACCUM);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(600, 150);
	glutCreateWindow("Quake3BSP");
	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);
	glutPassiveMotionFunc(mouse);
	glutKeyboardFunc(key);
	glutKeyboardUpFunc(keyup);
	glutMouseFunc(mousebutton);

	/* Initialize GL extensions */
	SetUpARB_multitexture();

	/* Initialize basic clear colour/depth setting */
    glClearColor(0.0, 0.0, 0.0, 0.0);
	
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

static void init_game() {
	float origin[] = { 0.0f, 1.0f, 0.0f };

	bsp = bsp_load("maps/tankracer.bsp");
	if (!bsp) {
		exit(1);
	}
	
	camera[0] = malloc(sizeof(Camera));
	camera[1] = NULL;
	camera[1] = malloc(sizeof(Camera));
	Camera_Init(camera[0], bsp);
	Camera_Move(camera[0], origin);
	Camera_Init(camera[1], bsp);
	Camera_Move(camera[1], origin);
	camera[1]->viewAngles[0] = 180;
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	init_gl();
	init_game();

	glutMainLoop();

	return 0;
}
