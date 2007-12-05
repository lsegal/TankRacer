#include "common.h"
#include "game.h"

static GLuint menuTexture;
static int frame = 0;
static float bottom = 40;
static float y = 0;
static float speed = 0.05;
static float gravity = 0.03;
static int animDone = 0;
static int width, height;

static void Menu_StartGame() {
	glDeleteTextures(1, &menuTexture);
	Game_Init();
	Game_Resize(width, height);
}

static void resize(int w, int h) {
	width = w;
	height = h;
}

static void specialKeyFunc(int c, int x, int y) {
	Menu_StartGame();
}

static void keyFunc(unsigned char c, int x, int y) {
	if (c == 27) exit(0);
	Menu_StartGame();
}

static void run(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 100, 100, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);
	glDisable(GL_CULL_FACE);
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, menuTexture);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 0); glVertex2d(30, y-35);
	glTexCoord2d(1, 0); glVertex2d(70, y-35);
	glTexCoord2d(1, 1); glVertex2d(70, y);
	glTexCoord2d(0, 1); glVertex2d(30, y);
	glEnd();
	glEnable(GL_CULL_FACE);

	if (!animDone) {
		speed += gravity;
		if (y >= bottom) {
			speed = -0.6 * speed;
		}
		if (fabs(speed) < 0.005 && y >= bottom) {
			animDone = 1;
		}
		else {
			y += speed;
		}
	}
	else {
		glColor3d(1, 0, 0);

		if (sin(frame / 10) > 0)
			text_output(35, 80, "PRESS ANY KEY TO START");
	}

	frame++;

	glutSwapBuffers();
}

void Menu_Init() {
	menuTexture = load_texture_jpeg("textures/tankracer.jpg");

	glutReshapeFunc(resize);
	glutKeyboardFunc(keyFunc);
	glutSpecialFunc(specialKeyFunc);
	glutIdleFunc(run);
	glutDisplayFunc(run);
}
