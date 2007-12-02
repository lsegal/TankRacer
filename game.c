#include "common.h"
#include "game.h"
#include "bsp.h"

const int numPlayers = 2;
const int totalLaps = 2;
static Player playerList[2];
static float windowWidth;
static float windowHeight;
static float frame = 0;
static bspfile *bsp;
static char mapName[128];
static float gravity = 0.37338;
typedef void (*TankInitProc)(Tank *, ...);
static float ambFunc;

static GLuint skyTexture;
static GLuint cloudTexture;
static GLuint fogTexture;
static GLUquadricObj *quad;

static void Player_Init(int playerNum, TankInitProc tankInitProc) {
	Camera_Init(&playerList[playerNum].camera, bsp);	/* Initialize the camera */
	tankInitProc(&playerList[playerNum].tank);			/* Initialize the tank */

	playerList[playerNum].numLast = 10;
	memset(playerList[playerNum].lastDir, 0, sizeof(float[3]) * playerList[playerNum].numLast);
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

static void Game_Start() {
	int playerNum;

	for (playerNum = 0; playerNum < numPlayers; playerNum++) {
		vec3f_clear(playerList[playerNum].tank.obj.velocity);
		vec3f_clear(playerList[playerNum].tank.obj.direction);
		vec3f_clear(playerList[playerNum].tank.obj.force);

		playerList[playerNum].tank.obj.position[0] = 10 + (playerNum / 2) * 3;
		playerList[playerNum].tank.obj.position[1] = -2.875;
		playerList[playerNum].tank.obj.position[2] = 6.5 + 4 * playerNum;

		playerList[playerNum].tank.obj.direction[0] = 1; /* Point in the positive X */

		playerList[playerNum].checkpoint = 'S';
		playerList[playerNum].lapNumber = 0;
	}

	Game_Resize(windowWidth, windowHeight);
}

void Game_Init() {
	int i;

	memset(playerList, 0, sizeof(playerList));

	/* Load the keyboard */
	Keyboard_Init();

	/* Read configuration file */
	Game_Read_Config();

	/* Load sky texture */
	skyTexture = load_texture_jpeg("textures/sky1.jpg");
	cloudTexture = load_texture_jpeg("textures/sky2.jpg");
	fogTexture = load_texture_jpeg("textures/clouds.jpg");
	quad = gluNewQuadric();
	ambFunc = 0.0f;

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

	/* Start the game */
	Game_Start();
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
	int i, driving = 0, d;
	Object *obj;
	float z[] = {0,0,0}, tmp[3];

	if (Keyboard_GetState(KEY_ESC, FALSE, TRUE)) exit(0);

	if (Keyboard_GetState('c', FALSE, TRUE)) {
		Game_Start();
	}
	if (Keyboard_GetState('m', FALSE, TRUE)) {
		if (glIsEnabled(GL_TEXTURE_2D)) {
			glDisable(GL_TEXTURE_2D);
		}
		else {
			glEnable(GL_TEXTURE_2D);
		}
	}


	for (i = 0; i < numPlayers; i++) {
		obj = &playerList[i].tank.obj;

		driving = 0;

		if (obj->onGround) {
			if (Keyboard_GetState(playerList[i].forwardKey, TRUE, FALSE)) {
				driving = 1;
			}
			if (Keyboard_GetState(playerList[i].backKey, TRUE, FALSE)) {
				driving = -1;
			}
			if (Keyboard_GetState(playerList[i].leftKey, TRUE, FALSE)) { /* turn left */
				if (obj->speed != 0) {
					if (!driving) d = obj->speed >= 0 ? 1 : -1;
					else d = driving;
					vec3f_rotp(obj->direction, z, obj->upAngles, 
						pow(playerList[i].tank.turnAbility * (obj->speed - obj->maxSpeed/2), 2) * d, obj->direction);
				}
			}
			if (Keyboard_GetState(playerList[i].rightKey, TRUE, FALSE)) { /* turn right */
				if (obj->speed != 0) {
					if (!driving) d = obj->speed >= 0 ? 1 : -1;
					else d = driving;
					vec3f_rotp(obj->direction, z, obj->upAngles, 
						-pow(playerList[i].tank.turnAbility * (obj->speed - obj->maxSpeed/2), 2) * d, obj->direction);
				}
			}
			if (Keyboard_GetState('g', TRUE, FALSE)) {
				obj->force[1] += 0.2;
			}
			if (Keyboard_GetState('b', TRUE, FALSE)) {
				obj->force[1] -= 0.2;
			}

			if (driving || fabs(obj->speed) > EPSILON) {
				vec3f_set(obj->direction, tmp);
				vec3f_scale(tmp, obj->maxAccel * driving, tmp);
				vec3f_add(obj->force, tmp, obj->force);
			}
		}
	}
}

static void Game_GenerateHitbox(Object *obj, float hitbox[8][3]) {
	int i, n = -1;
	float right[3], tw[3], tl[3], th[3] = {0, 0, 0};
	float w = obj->width / 2, h = obj->height, l = obj->length / 2;

	vec3f_cross(obj->direction, obj->upAngles, right);
	for (i = 0; i < 8; i++) { 
		vec3f_set(obj->position, hitbox[i]);

		if (i == 4) {
			vec3f_scale(obj->upAngles, h, th);
		}
		if ((i & 1) == 0) n = -n;

		vec3f_scale(right, n * w, tw);
		vec3f_scale(obj->direction, ((i & 1) == 0 ? 1 : -1) * l, tl);

		vec3f_add(hitbox[i], tw, hitbox[i]);
		vec3f_add(hitbox[i], tl, hitbox[i]);
		vec3f_add(hitbox[i], th, hitbox[i]);
	}
}

/* Generates the light attenuation from the sun */
static void Game_HandleDaylight() {
	if (frame / 100 < 0.38) {
		ambFunc = frame / 100;
	}
	else {
		ambFunc = 0.5 * sin(frame/1000) + 0.38;
		if (ambFunc < 0.28) ambFunc = 0.28;
	}
	if (ambFunc < 0.1) ambFunc = 0.1;
}

/* Returns nonzero if the obj1 is inside obj2. Also sets forces if there was a collision */
static int Game_PlayerCollision(Object *obj1, Object *obj2) {
	int i;
	float box1[8][3], box2[8][3];
	float remf[3], addf[3], tmp[3], addmag;
	float **closest = NULL, dist;
	Object *hitter, *hittee;
	
	/* Generate hitboxes */
	Game_GenerateHitbox(obj1, box1);
	Game_GenerateHitbox(obj2, box2);

	for (i = 0; i < 8; i++) {
		if (point_in_hitbox(box1[i], box2)) {
			printf("Object collision\n");
			
			/* Get distance between tanks */
			dist = vec3f_dist(obj1->position, obj2->position) / (obj1->length + obj2->length);

			if (vec3f_mag(obj1->force) > vec3f_mag(obj2->force)) {
				hitter = obj1; hittee = obj2;
			}
			else {
				hitter = obj2; hittee = obj1;
			}

			vec3f_set(hitter->force, tmp);
			vec3f_norm(tmp);

			/* Find out how much force can be applied to obj2 */
			addmag = vec3f_dot(tmp, hittee->direction);
			vec3f_scale(hittee->direction, addmag * dist, addf);

			/* Remaining force is added to obj1 */
			vec3f_scale(hitter->direction, -(1-fabs(addmag)) * dist, remf);

			/* Add forces */
			//vec3f_sub(remf, hitter->force, hitter->force); /* Don't add anything to hitter */
			vec3f_add(addf, hittee->force, hittee->force);

			return 1;
		}
	}
	return 0;
}

static void Game_RunPhysics() {
	int i, x;
	float dir[3], hitbox[8][3], ftot[3], tmp[3], up[] = {0, 0, 0};
	float hitboxadjust[] = {0, EPSILON, 0};
	float grav[] = { 0, -gravity, 0 }, cnormal[3];
	Object *obj, *obj2;
	face *cface;
	float angle, fric;

	for (i = 0; i < numPlayers; i++) {
		obj = &playerList[i].tank.obj;

		fric = 0.05; /* World friction */

		/* Let the object think */
		obj->thinkFunc(obj, obj->funcData);

		/* Tank vs Tank collision */
		for (x = 0; x < numPlayers; x++) {
			if (x == i) continue;
			obj2 = &playerList[x].tank.obj;
			if (!Game_PlayerCollision(obj, obj2)) {
				Game_PlayerCollision(obj2, obj);
			}
		}

		/* Add gravity */
		vec3f_set(obj->force, ftot);
		vec3f_scale(grav, obj->mass, tmp);
		vec3f_add(ftot, grav, ftot);

		vec3f_set(ftot, dir);
		vec3f_norm(dir);
		vec3f_scale(dir, 20 * EPSILON, dir);

		/* Get direction vector */
		obj->onGround = 0;
		Game_GenerateHitbox(obj, hitbox);
		for (x = 0; x < 8; x++) {
			vec3f_clear(hitboxadjust);
			vec3f_scale(obj->direction, -10 * EPSILON, tmp);
			vec3f_add(hitboxadjust, tmp, hitboxadjust);
			vec3f_scale(obj->upAngles, 10 * EPSILON, tmp);
			vec3f_add(hitboxadjust, tmp, hitboxadjust);

			if (x >= 4) { /* Only use direction for top 4 points */
				vec3f_scale(obj->force, 20 * EPSILON, tmp);
				cface = bsp_face_collision(bsp, hitbox[x], tmp, FALSE);
			}
			else if (x < 4) {
				cface = bsp_face_collision(bsp, hitbox[x], dir, FALSE);
			}

			if (cface) {
				obj->onGround = 1;

				vec3f_set(cface->normal, cnormal);

				angle = acos(vec3f_dot(cnormal, obj->upAngles));
				if (x < 4 && angle < PI/8) {
					vec3f_add(cnormal, up, up);
				}
				else if (angle >= PI/8) {
					cnormal[1] = 0;
				}

				if (strstr(bsp->data.textures[cface->texture].name, "grass")) {
					fric += 0.03;
				}

#ifdef _PRINTDEBUG
				printf("Player %d collided with '%s'\n", i, bsp->data.textures[cface->texture].name);
#endif
				vec3f_scale(cnormal, vec3f_dot(cnormal, ftot), tmp);
				vec3f_sub(tmp, ftot, ftot);
			}
			else {
				vec3f_add(hitbox[x], ftot, tmp); 
				if (tmp[1] < -2.875) {
					obj->onGround = 1;
					vec3f_sub(grav, ftot, ftot);
					break;
				}
			}
		}
		if (vec3f_mag(up) != 0) {
			vec3f_norm(up);
			vec3f_set(up, obj->upAngles);
		}

		if (fabs(obj->speed) > EPSILON) {
			/* Apply friction to left and right directions (relative to tank) */
			vec3f_cross(obj->direction, obj->upAngles, tmp);
			vec3f_norm(tmp);
			vec3f_scale(tmp, vec3f_dot(tmp, obj->velocity) * 0.9, tmp);
			vec3f_sub(tmp, ftot, ftot);

			/* Apply friction opposite to driving direction */
			vec3f_set(obj->direction, tmp);
			vec3f_norm(tmp);
			vec3f_scale(tmp, vec3f_dot(tmp, obj->velocity) * fric, tmp);
			vec3f_sub(tmp, ftot, ftot);
		}
		
		if (fabs(vec3f_mag(ftot)) < EPSILON) {
			vec3f_clear(ftot);
		}

		vec3f_set(ftot, obj->force);

		vec3f_add(obj->velocity, obj->position, obj->position);
		vec3f_scale(obj->force, 1/obj->mass, obj->velocity);

		if (obj->position[1] < -2.875) {
		//	obj->position[1] = -2.87;
		}

		obj->speed = vec3f_mag(obj->velocity);
	}
}

static void Game_Checkpoint() {
	int i;
	face *cface;
	float origin[3], dir[3];
	char *checkName, newCheckpoint;

	for (i = 0; i < numPlayers; i++) {
		vec3f_set(playerList[i].tank.obj.position, origin);
		origin[1] += playerList[i].tank.obj.height / 2;
		vec3f_set(playerList[i].tank.obj.direction, dir);
		vec3f_scale(dir, -playerList[i].tank.obj.speed, dir);

		/* Check if tank is inside a checkpoint */
		if (cface = bsp_face_collision(bsp, origin, dir, TRUE)) {
			checkName = strstr(bsp->data.textures[cface->texture].name, "checkpoint");
			if (checkName) {
				newCheckpoint = checkName[10];

#ifdef _PRINTDEBUG
				printf("Collision with checkpoint %c\n", newCheckpoint);
#endif

				if (newCheckpoint == 'S' && playerList[i].checkpoint == '2') {
					playerList[i].lapNumber++;
				}
				else if ((newCheckpoint == '1' && playerList[i].checkpoint == 'S') ||
						(newCheckpoint > playerList[i].checkpoint)) {
					playerList[i].checkpoint = newCheckpoint;
				}
			}
		}
	}
}

void Game_Run() {
	Game_HandleKeys();
	Game_HandleDaylight();
	Game_RunPhysics();
	Game_Checkpoint();
	frame++;
}

static void Game_SetCamera(int playerNum) {
	int i;
	float pos[3], dir[3], temp[3];
	Object *obj = &playerList[playerNum].tank.obj;
	float *direction = obj->direction;

	for (i = playerList[playerNum].numLast-1; i >= 0; i--) {
		if (vec3f_mag(playerList[playerNum].lastDir[i]) != 0) {
			direction = (float *)&playerList[playerNum].lastDir[i];
			break;
		}
	}
	vec3f_set(direction, dir);
	vec3f_scale(dir, -2 * (obj->speed + 1), dir);
	dir[1] = 1.3;

	vec3f_set(obj->position, pos);
	vec3f_add(pos, dir, pos);
	vec3f_set(direction, temp);
	vec3f_scale(temp, 5, temp);
	vec3f_add(obj->position, temp, temp);
	vec3f_sub(pos, temp, dir);

	Camera_SetPosition(&playerList[playerNum].camera, pos, dir);

	for (i = playerList[playerNum].numLast-1; i >= 0; i--) {
		vec3f_set(playerList[playerNum].lastDir[i-1], playerList[playerNum].lastDir[i]);
	}
	vec3f_set(obj->direction, playerList[playerNum].lastDir[0]);
}

static void Game_Draw_Sky() {
	float amb = ambFunc;

	gluQuadricOrientation(quad, GLU_INSIDE); 
	gluQuadricTexture(quad, GL_TRUE);

	if (amb < 0.0f) amb = 0.0f;
	if (amb > 0.5f) amb = 0.5f; 

	/* Draw sky sphere */
	glColor3f(0.1 + amb, 0.1 + amb, 0.1 + amb);
	glBindTexture(GL_TEXTURE_2D, skyTexture);
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glRotatef(-frame/10, 1, 0, 0);
	gluSphere(quad, 200, 64, 64);
	glPopMatrix();

	/* Draw cloud sphere */
	glColor4f(0.3 + amb, 0.3 + amb / 2, 0.3 + amb, 0.3);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);
	glBindTexture(GL_TEXTURE_2D, cloudTexture);
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glRotatef(frame/10, 1, -1, 1);
	gluSphere(quad, 180, 64, 64);
	glPopMatrix();
	glDisable(GL_BLEND);
}

static void Game_Render_Scene(int playerNum) {
	Tank *tank;
	lightvol *light;
	int i;
	float pos[3], amb[3], diff[3];

	/* Set the viewport in the window */
	glViewport(0, (numPlayers - playerNum - 1) * (windowHeight/(float)numPlayers), 
		windowWidth, (windowHeight/(float)numPlayers));
	
	/* Position the camera behind tank (always) */
	Game_SetCamera(playerNum);

	/* Set ambient light */
	glColor3d(0.6*ambFunc, 0.8*ambFunc, 0.5*ambFunc);
	//glColor3d(0.6, 0.8, 0.5);

	/* Render the scene from the camera */
	Camera_Render(&playerList[playerNum].camera);

	/* Draw sky"box" (spheres) */
	Game_Draw_Sky();

	/* Draw all objects */
	/* Replace this with WORLD_DRAW */ 
	for (i = 0; i < numPlayers; i++) {
		tank = &playerList[i].tank;

#ifndef _PRINTDEBUG
		glBegin(GL_LINES);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_LINE_SMOOTH);
		glColor3d(1,0,0);
		{
			float a[3], b[3];
			vec3f_set(tank->obj.position, a);
			vec3f_set(tank->obj.force, b);
			vec3f_scale(b, 5, b);
			vec3f_add(b, tank->obj.position, b);
			a[1] += tank->obj.height + 0.05;
			b[1] += tank->obj.height + 0.05;
			glVertex3fv(a);
			glVertex3fv(b);
		}
		glEnd();

		glBegin(GL_LINES);
		glColor3d(1,1,1);
		glPointSize(2);
		{
			float hb[8][3];
			int n;
			Game_GenerateHitbox(&tank->obj, hb);
			for (n = 0; n < 8; n++) 
				glVertex3fv(hb[n]);

			glVertex3fv(hb[0]);
			glVertex3fv(hb[2]);
			glVertex3fv(hb[1]);
			glVertex3fv(hb[3]);
			glVertex3fv(hb[4]);
			glVertex3fv(hb[6]);
			glVertex3fv(hb[5]);
			glVertex3fv(hb[7]);
			glVertex3fv(hb[0]);
			glVertex3fv(hb[4]);
			glVertex3fv(hb[1]);
			glVertex3fv(hb[5]);
			glVertex3fv(hb[2]);
			glVertex3fv(hb[6]);
			glVertex3fv(hb[3]);
			glVertex3fv(hb[7]);
		}
		glEnd();
#endif

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

			diff[0] = (float)light->directional[0] / 255.0 + ambFunc;
			diff[1] = (float)light->directional[1] / 255.0 + ambFunc;
			diff[2] = (float)light->directional[2] / 255.0 + ambFunc;
			glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
			glLightfv(GL_LIGHT0, GL_SPECULAR, diff);

			amb[0] = (float)light->ambient[0] / 255.0 + ambFunc;
			amb[1] = (float)light->ambient[1] / 255.0 + ambFunc;
			amb[2] = (float)light->ambient[2] / 255.0 + ambFunc;
			glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		}

		glRotatef(-RAD2DEG(atan2(tank->obj.upAngles[X], tank->obj.upAngles[Y])), 0, 0, 1); /* PITCH */
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

	text_output(2, 2, "Coordinates: %.2f, %.2f, %.2f - Lap number: %d", 
		camera->position[0], camera->position[1], camera->position[2], playerList[playerNum].lapNumber + 1);

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
