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

static void Player_Init(int playerNum, TankInitProc tankInitProc) {
	Camera_Init(&playerList[playerNum].camera, bsp);	/* Initialize the camera */
	tankInitProc(&playerList[playerNum].tank);			/* Initialize the tank */

	playerList[playerNum].tank.obj.position[0] = 8;
	playerList[playerNum].tank.obj.position[1] = -2.87;
	playerList[playerNum].tank.obj.position[2] = 6.5 + 4 * playerNum;

	playerList[playerNum].tank.obj.direction[0] = 1; /* Point in the positive X */
}

static void Game_Read_KeyConfig() {
	int pnum, *keyPtr, keyVal;
	char line[256], actionName[20], keyName[20];
	FILE *file = fopen("config.txt", "r");

	while (!feof(file)) {
		fgets(line, 256, file);
		sscanf(line, "%d %s %s", &pnum, actionName, keyName);

		/* Set keys */
		if (pnum >= 0 && pnum < numPlayers) {
			if (keyName[1] == 0) {
				keyVal = keyName[0];
			}
			else if (!strcmp(keyName, "left")) {
				keyVal = KEY_LEFT;
			}
			else if (!strcmp(keyName, "right")) {
				keyVal = KEY_RIGHT;
			}
			else if (!strcmp(keyName, "up")) {
				keyVal = KEY_UP;
			}
			else if (!strcmp(keyName, "down")) {
				keyVal = KEY_DOWN;
			}

			if (!strcmp(actionName, "left")) {
				keyPtr = &playerList[pnum].leftKey;
			}
			else if (!strcmp(actionName, "right")) {
				keyPtr = &playerList[pnum].rightKey;
			}
			else if (!strcmp(actionName, "forward")) {
				keyPtr = &playerList[pnum].forwardKey;
			}
			else if (!strcmp(actionName, "back")) {
				keyPtr = &playerList[pnum].backKey;
			}

			*keyPtr = keyVal;
		}
	}

	fclose(file);
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

	/* Read configuration file */
	Game_Read_KeyConfig();
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
	int i;
	float z[] = {0,0,0}, tmp[3];

	if (Keyboard_GetState(KEY_ESC, FALSE, TRUE)) exit(0);

	for (i = 0; i < numPlayers; i++) {
		if (Keyboard_GetState(playerList[i].forwardKey, TRUE, FALSE)) {
			vec3f_set(playerList[i].tank.obj.direction, tmp);
			vec3f_scale(tmp, 0.2, tmp);
			vec3f_add(playerList[i].tank.obj.position, 
					tmp, playerList[i].tank.obj.position);
		}
		if (Keyboard_GetState(playerList[i].backKey, TRUE, FALSE)) {
			vec3f_sub(playerList[i].tank.obj.direction, 
					playerList[i].tank.obj.position, 
					playerList[i].tank.obj.position);
		}
		if (Keyboard_GetState(playerList[i].leftKey, TRUE, FALSE)) {
			vec3f_rotp(playerList[i].tank.obj.direction, z, playerList[i].tank.obj.upAngles, 5, playerList[i].tank.obj.direction);
		}
		if (Keyboard_GetState(playerList[i].rightKey, TRUE, FALSE)) {
			vec3f_rotp(playerList[i].tank.obj.direction, z, playerList[0].tank.obj.upAngles, -5, playerList[i].tank.obj.direction);
		}
		if (Keyboard_GetState('g', TRUE, FALSE)) {
			playerList[i].tank.obj.position[1] += 0.2;
		}
		if (Keyboard_GetState('b', TRUE, FALSE)) {
			playerList[i].tank.obj.position[1] -= 0.2;
		}
	}
}

void Game_Run() {
	int i;
	float gravity[] = {0, -0.37338, 0}, tmp[3];
	plane *cplane;

	Game_HandleKeys();

	for (i = 0; i < numPlayers; i++) {
		vec3f_add(gravity, playerList[i].tank.obj.force, playerList[i].tank.obj.force);

		vec3f_add(playerList[i].tank.obj.position, playerList[i].tank.obj.force, tmp);
		if (cplane = bsp_simple_collision(bsp, playerList[i].tank.obj.position, tmp, NULL, NULL)) {
		//	vec3f_set(cplane->normal, playerList[i].tank.obj.upAngles);
		}
		else {
		//	vec3f_add(playerList[i].tank.obj.force, playerList[i].tank.obj.position, playerList[i].tank.obj.position);
		}
	}

	frame++;
}

static void Game_SetCamera(int playerNum) {
	float pos[3], dir[3], temp[3];
	Object *obj = &playerList[playerNum].tank.obj;

	vec3f_set(obj->direction, dir);
	vec3f_scale(dir, -2, dir);
	dir[1] = 1.3;

	vec3f_set(obj->position, pos);
	vec3f_add(pos, dir, pos);
	vec3f_set(obj->direction, temp);
	vec3f_scale(temp, 5, temp);
	vec3f_add(obj->position, temp, temp);
	vec3f_sub(pos, temp, dir);

	sprintf(playerList[playerNum].centerText, "%.2f %.2f %.2f", obj->direction[0], obj->direction[1], obj->direction[2]);

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
	Game_SetCamera(playerNum);

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

		glRotatef(RAD2DEG(atan2(tank->obj.upAngles[X], tank->obj.upAngles[Y])), 0, 0, 1); /* PITCH */
		glRotatef(RAD2DEG(atan2(tank->obj.upAngles[2], tank->obj.upAngles[1])), 1, 0, 0); /* ROLL */
		glRotatef(-RAD2DEG(atan2(tank->obj.direction[2], tank->obj.direction[0])) - 180, 0, 1, 0); /* YAW */

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
