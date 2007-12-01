#include "common.h"
#include "game.h"
#include "bsp.h"

const int numPlayers = 2;
static Player playerList[2];
static float windowWidth;
static float windowHeight;
static float frame = 0;
static bspfile *bsp;
static char mapName[128];
static float gravity = 0.37338;
typedef void (*TankInitProc)(Tank *, ...);

static void Player_Init(int playerNum, TankInitProc tankInitProc) {
	Camera_Init(&playerList[playerNum].camera, bsp);	/* Initialize the camera */
	tankInitProc(&playerList[playerNum].tank);			/* Initialize the tank */

	playerList[playerNum].tank.obj.position[0] = 8;
	playerList[playerNum].tank.obj.position[1] = -2.87;
	playerList[playerNum].tank.obj.position[2] = 6.5 + 4 * playerNum;

	playerList[playerNum].tank.obj.direction[0] = 1; /* Point in the positive X */
}

static void Game_Read_Config() {
	int pnum, *keyPtr, keyVal;
	char line[256], actionName[20], keyName[20], key[20], val[64];
	FILE *file = fopen("config.txt", "r");

	while (!feof(file)) {
		fgets(line, 256, file);
		if (line[0] == ';' || line[0] == '#') continue; /* Ignore comment lines */

		if (sscanf(line, "%d %s %s", &pnum, actionName, keyName) > 0) {
			/* This key value is for player `pnum` */
			if (pnum >= 0 && pnum < numPlayers) {
				/* Key names */
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

				/* Action names */
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
		else if (sscanf(line, "%s %s", key, val) > 0) {
			if (!strcmp(key, "map")) {
				sprintf(mapName, "maps/%s.bsp", val);
			}
			if (!strcmp(key, "gravity")) {
				gravity = atof(val);
			}
		}
	}

	fclose(file);
}

void Game_Init() {
	int i;

	memset(playerList, 0, sizeof(playerList));

	/* Load the keyboard */
	Keyboard_Init();

	/* Read configuration file */
	Game_Read_Config();

	/* Load the map */
	bsp = bsp_load(mapName);
	if (!bsp) {
		fprintf(stderr, "Critical: could not load map '%s'\n", mapName);
		exit(1);
	}

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
	int i, driving = FALSE;
	float z[] = {0,0,0}, tmp[3];

	if (Keyboard_GetState(KEY_ESC, FALSE, TRUE)) exit(0);

	for (i = 0; i < numPlayers; i++) {
		if (Keyboard_GetState(playerList[i].forwardKey, TRUE, FALSE)) {
			driving = TRUE;
		}
		if (Keyboard_GetState(playerList[i].backKey, TRUE, FALSE)) {
			vec3f_sub(playerList[i].tank.obj.direction, 
					playerList[i].tank.obj.position, 
					playerList[i].tank.obj.position);
		}
		if (Keyboard_GetState(playerList[i].leftKey, TRUE, FALSE)) { /* turn left */
			vec3f_rotp(playerList[i].tank.obj.direction, z, playerList[i].tank.obj.upAngles, 5, playerList[i].tank.obj.direction);
		}
		if (Keyboard_GetState(playerList[i].rightKey, TRUE, FALSE)) { /* turn right */
			vec3f_rotp(playerList[i].tank.obj.direction, z, playerList[0].tank.obj.upAngles, -5, playerList[i].tank.obj.direction);
		}
		if (Keyboard_GetState('g', TRUE, FALSE)) {
			playerList[i].tank.obj.position[1] += 0.2;
		}
		if (Keyboard_GetState('b', TRUE, FALSE)) {
			playerList[i].tank.obj.position[1] -= 0.2;
		}

		if (driving || fabs(playerList[i].tank.obj.speed) > EPSILON) {
			vec3f_set(playerList[i].tank.obj.direction, tmp);
			vec3f_scale(tmp, 0.2, tmp);
			vec3f_add(playerList[i].tank.obj.force, 
					tmp, playerList[i].tank.obj.force);
		}
	}
}

static void Game_GenerateHitbox(Object *obj, float hitbox[8][3]) {
	int i, n = -1;
	float w = obj->width / 2, h = obj->height / 2, l = obj->length / 2;

	for (i = 0; i < 8; i++) { 
		vec3f_set(obj->position, hitbox[i]);

		if (i < 4)	hitbox[i][1] -= h;
		else		hitbox[i][1] += h;

		if ((i & 1) == 0) n = -n;
		hitbox[i][0] += n * w;
		hitbox[i][2] += ((i & 1) == 0 ? 1 : -1) * l;
	}
}

void Game_Run() {
	int i, x;
	float grav[] = {0, -gravity, 0}, nforce[3], tmp[3], nmag, fric = 0.1;
	float fin[] = {0, 0, 0}, dfin[3], up[] = {0, 0, 0}, hitbox[8][3], ffric[3];
	Object *obj;
	face *cface;

	Game_HandleKeys();

	for (i = 0; i < numPlayers; i++) {
		obj = &playerList[i].tank.obj;

		/* Add forces to get total force acting on object */
		vec3f_set(grav, fin);
		vec3f_add(obj->force, fin, fin);

		/* Get delta force */
		vec3f_set(fin, dfin);
		vec3f_norm(dfin);
		vec3f_scale(dfin, EPSILON, dfin);

		/* Find normal forces acting on each point of the hitbox */
		Game_GenerateHitbox(obj, hitbox);
		for (x = 0; x < 8; x++) {
			vec3f_add(hitbox[x], dfin, tmp);
			if (cface = bsp_face_collision(bsp, hitbox[x], tmp)) {
				printf("Collision with '%s'\n", bsp->data.textures[cface->texture].name);

				fric += 0.1;

				if (x < 4) { /* Normal forces generated by bottom half of hitbox help create upangles */
					vec3f_add(up, cface->normal, up);
				}

				/* Calculate the normal force */
				nmag = vec3f_dot(cface->normal, fin);
				vec3f_scale(cface->normal, nmag, nforce);

				/* Add normal force to force vector */
				vec3f_sub(nforce, fin, fin);
			}
		}

		/* Set upangles */
		if (fabs(vec3f_mag(up)) > EPSILON) {
			vec3f_norm(up);
			vec3f_set(up, obj->upAngles);
		}

		/* Set force and add it to position */
		vec3f_set(fin, obj->force);
		sprintf(playerList[i].centerText, "%.2f %.2f %.2f", obj->velocity[0], obj->velocity[1], obj->velocity[2]);
		obj->speed = vec3f_mag(fin);

		/* Add friction */
		if (fric > 1.0f) fric = 1.0f;
		vec3f_set(fin, ffric);
		vec3f_norm(ffric);
		vec3f_scale(ffric, fric * obj->speed, ffric);
		vec3f_sub(ffric, fin, fin);
		obj->speed = vec3f_mag(fin);

		vec3f_add(fin, obj->velocity, obj->velocity);
		vec3f_add(obj->velocity, obj->position, obj->position);
		vec3f_clear(obj->force);
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

	glColor3d(0, 0, 0);
	text_output(2, 50, playerList[playerNum].centerText);
}

void Game_Render() {
	int i;

	for (i = 0; i < numPlayers; i++) {
		Game_Render_Scene(i);		/* Draw the scene (map) */
		Game_Render_Overlay(i);		/* Draw the overlay for the player */
	}
}
