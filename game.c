#include "common.h"
#include "game.h"
#include "bsp.h"

const int numPlayers = 2;
static Player playerList[2];
static float windowWidth;
static float windowHeight;
static float frame = 0;
static bspfile *bsp;
typedef void (*TankInitProc)(Tank *, ...);

void Player_Init(int playerNum, TankInitProc tankInitProc) {
	Camera_Init(&playerList[playerNum].camera, bsp);	/* Initialize the camera */
	tankInitProc(&playerList[playerNum].tank);			/* Initialize the tank */

	playerList[playerNum].tank.obj.position[0] = 8;
	playerList[playerNum].tank.obj.position[1] = -2.87;
	playerList[playerNum].tank.obj.position[2] = 6.5 + 4 * playerNum;

	playerList[playerNum].tank.obj.direction[0] = 1; /* Point in the positive X */
}

void Game_Init() {
	int i;

	memset(playerList, 0, sizeof(playerList));

	/* Load the keyboard */
	Keyboard_Init();

	/* Load the map */
	bsp = bsp_load("maps/tankracer.bsp");
	if (!bsp) exit(1);

	/* Setup player info */
	for (i = 0; i < numPlayers; i++) {
		Player_Init(i, Cowtank_Init);
	}
}

void Game_Resize(int w, int h) {
	int i;

	windowWidth = w; 
	windowHeight = h;

	/* Set the aspect ratio of each camera's rendering space */
	for (i = 0; i < numPlayers; i++) {
		playerList[i].camera.aspectRatio = numPlayers * (windowWidth / windowHeight);
	}
}

static void Game_HandleKeys() {
	if (Keyboard_GetState(KEY_ESC, FALSE, TRUE)) exit(0);
	if (Keyboard_GetState('w', TRUE, FALSE)) {
		playerList[0].tank.obj.position[0] += 0.2;
		playerList[0].tank.obj.speed += 0.2;
	}
	if (Keyboard_GetState('s', TRUE, FALSE)) {
		playerList[0].tank.obj.position[0] -= 0.2;
	}
	if (Keyboard_GetState('a', TRUE, FALSE)) {
		playerList[0].tank.obj.position[2] -= 0.2;
	}
	if (Keyboard_GetState('d', TRUE, FALSE)) {
		playerList[0].tank.obj.position[2] += 0.2;
	}
	if (Keyboard_GetState('g', TRUE, FALSE)) {
		playerList[0].tank.obj.position[1] += 0.2;
	}
	if (Keyboard_GetState('b', TRUE, FALSE)) {
		playerList[0].tank.obj.position[1] -= 0.2;
	}
}

void Game_Run() {
	Game_HandleKeys();
	frame++;
}

static void Game_Set_Camera(int playerNum) {
	float pos[3], dir[3], temp[3];
	Object *obj = &playerList[playerNum].tank.obj;

	vec3f_set(obj->direction, dir);
	vec3f_scale(dir, -2, dir);
	dir[2] = 0;
	dir[1] = 1.3;/*
	dir[0] = 2 * cosd(2 * frame);
	dir[2] = 2 * sind(2 * frame);
	dir[1] = 1;*/

	vec3f_set(obj->position, pos);
	vec3f_add(pos, dir, pos);
	vec3f_set(obj->direction, temp);
	vec3f_scale(temp, 5, temp);
	vec3f_add(obj->position, temp, temp);
	vec3f_sub(pos, temp, dir);

	Camera_SetPosition(&playerList[playerNum].camera, pos, dir);
}

static void Game_Render_Scene(int playerNum) {
	Tank *tank;
	lightvol *light;
	int i;
	float pos[3], amb[3], diff[3];

	/* Set the viewport in the window */
	glViewport(0, (numPlayers - playerNum - 1) * (windowHeight/(float)numPlayers), 
		windowWidth, (windowHeight/(float)numPlayers));
	
	glColor3d(0.8, 1, 0.7);

	/* Position the camera behind tank (always) */
	Game_Set_Camera(playerNum);

	/* Render the scene from the camera */
	Camera_Render(&playerList[playerNum].camera);

	/* Draw all objects */
	/* Replace this with WORLD_DRAW */ 
	for (i = 0; i < numPlayers; i++) {
		tank = &playerList[i].tank;

		glPushMatrix();
		glTranslatef(tank->obj.position[0], tank->obj.position[1], tank->obj.position[2]);

		/* Find out if there is lightvol info at the point */
		light = bsp_lightvol(bsp, tank->obj.position);
		if (light) {
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);

			pos[0] = 2.4 * cosd((float)light->dir[1] * 360.0 / 255.0);
			pos[1] = 2.4 * cosd((float)light->dir[0] * 360.0 / 255.0);
			pos[2] = 2.4 * -sind((float)light->dir[1] * 360.0 / 255.0);
			/*
			sprintf(playerList[i].centerText, "angle(%.2f, %.2f) -> pos(%.2f, %.2f, %.2f)",
				(float)light->dir[0] * 360.0 / 255.0, 
				(float)light->dir[1] * 360.0 / 255.0, 
				pos[0], pos[1], pos[2]);
			*/
			glPushMatrix();
			glTranslatef(pos[0], pos[1], pos[2]);
			pos[0] = 0; pos[1] = 0; pos[2] = 0;
			glLightfv(GL_LIGHT0, GL_POSITION, pos);
			glPopMatrix();

			diff[0] = (float)light->directional[0] / 255.0;
			diff[1] = (float)light->directional[1] / 255.0;
			diff[2] = (float)light->directional[2] / 255.0;
			glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
			glLightfv(GL_LIGHT0, GL_SPECULAR, diff);

			amb[0] = (float)light->ambient[0] / 255.0;
			amb[1] = (float)light->ambient[1] / 255.0;
			amb[2] = (float)light->ambient[2] / 255.0;
			glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		}

		glRotatef(180, 0, 1, 0);
		//glRotated(frame, 1, 0, 0);
		glScalef(2, 2, 2);
		tank->obj.drawFunc(&tank->obj, tank->obj.funcData);
		glPopMatrix();

		if (light) {
			glDisable(GL_LIGHTING);
			glDisable(GL_LIGHT0);
		}
	}
}

static void Game_Render_Overlay(int playerNum) {
	Camera *camera = &playerList[playerNum].camera;
	Object *obj = &playerList[playerNum].tank.obj;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 100, 0, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3d(1, 0, 0);
	text_output(2, 92, "Player %d", playerNum+1);

	text_output(2, 2, "Coordinates: %.2f, %.2f, %.2f", 
		camera->position[0], camera->position[1], camera->position[2]);

	glColor3d(1,1,1);
	text_output(2, 50, playerList[playerNum].centerText);
}

void Game_Render() {
	int i;

	for (i = 0; i < numPlayers; i++) {
		Game_Render_Scene(i);		/* Draw the scene (map) */
		Game_Render_Overlay(i);		/* Draw the overlay for the player */
	}
}
