#include "common.h"
#include "game.h"
#include "bsp.h"

static int numPlayers = 2;
static int totalLaps = 3;
static int DEBUG = 0;
static Player playerList[2];
static float windowWidth;
static float windowHeight;
static float frame = 0;
static bspfile *bsp;
static char mapName[128];
static float gravity = 0.37338;
typedef void (*TankInitProc)(Tank *, ...);
static float ambFunc;

static int paused = 0;
static int pause_start = 0;//for the pause timer
static float pause_start_time=0;
static float pause_end_time=0;
static float pause_duration = 0;
static float total_paused_time = 0; 

static int mouse = 0;
static int allowBlur = 0;

static GLuint skyTexture;
static GLuint cloudTexture;
static GLuint dirtTexture;
static GLUquadricObj *quad;

static float time_now; 
static int game_end = 0;
static int game_started;
static float game_start_time;
static float time_since_start;
static int down_count;
static int count_finished;
static float game_count_down; //the "time left" shown on screen
const float time_limit = 240;//max allowed time for the game

static void Game_Run();
static void Game_Render();

const enum TankTypes {
	TANK_COWTANK,
	TANK_SPIDERTANK,
	TANK_NTANK,
	TANK_YTANK
	
};

static void Player_Init(int playerNum, TankInitProc tankInitProc) {
	Camera_Init(&playerList[playerNum].camera, bsp);	/* Initialize the camera */
	tankInitProc(&playerList[playerNum].tank);			/* Initialize the tank */

	playerList[playerNum].numLast = 10;
	memset(playerList[playerNum].lastDir, 0, sizeof(float[3]) * playerList[playerNum].numLast);
	
	playerList[playerNum].pengine = NULL;
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
				/* Tank type */
				if (!strcmp(actionName, "tank")) {
					playerList[pnum].tankType = atoi(keyName);
					continue;
				}

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
			if (!strcmp(key, "noclip")) {
				if (atoi(val) != 0) {
					mouse = TRUE;
				}
			}
			if (!strcmp(key, "maxplayers")) {
				numPlayers = atoi(val);
			}
			if (!strcmp(key, "maxlaps")) {
				totalLaps = atoi(val);
			}
		}
	}

	fclose(file);
}

static void Game_Start() {
	float grav[3] = { 0, -gravity * 0.05, 0 };
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

		//ParticleEngine_Free(playerList[playerNum].pengine);
		//playerList[playerNum].pengine = ParticleEngine_Init(100, 10, 5, -2.87, dirtTexture, NULL, NULL, grav);
	}

	Game_Resize(windowWidth, windowHeight);
	
	//these parameters help to initialize the timer.
	game_started = 1; //add one more parameter here to signal the timer.
	game_start_time = glutGet(GLUT_ELAPSED_TIME)/1000.00;
	count_finished = 0;
	paused = 0;
	game_end = 0;
}

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

void Game_Init() {
	int i;

	memset(playerList, 0, sizeof(playerList));

	glutIdleFunc(run);
	glutDisplayFunc(render);
	glutReshapeFunc(resize);

	/* Texture settings for lightmap modulation */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2.0f);


	/* Load the keyboard */
	Keyboard_Init();

	/* Read configuration file */
	Game_Read_Config();

	/* Load sky texture */
	skyTexture = load_texture_jpeg("textures/sky1.jpg");
	cloudTexture = load_texture_jpeg("textures/sky2.jpg");
	//dirtTexture = load_texture_jpeg("textures/dirtspeck.jpg");
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
		TankInitProc tproc = NULL;

		switch (playerList[i].tankType) {
			case TANK_COWTANK:    tproc = Cowtank_Init; break;	
			case TANK_SPIDERTANK: tproc = Spidertank_Init; break;
			case TANK_NTANK:	  tproc = NTank_Init; break;
			case TANK_YTANK:	  tproc = YTank_Init; break;
			
		}

		Player_Init(i, tproc);
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

static void handle_end_of_game(int i) {
	char *msg;
	int playerNum = i;
	if (playerList[playerNum].lapNumber + 1 < totalLaps) {
			msg = "YOU LOSE";
		}
		else {
			msg = "YOU WIN";
		}
		text_output2(42, 50, GLUT_BITMAP_TIMES_ROMAN_24, msg);

		if (game_count_down>0){
			text_output(78, 92, "Time Left: %.2f", game_count_down);
		}
		else {
			text_output(78, 92, "Time Left: 0.00");
		}
			text_output(45, 92, "GAME OVER");
			text_output(40, 42, "Press F1 to restart.");
		return;


}
static void Game_HandleKeys() {
	int i, driving = 0, turning = 0, d;
	Object *obj;
	float z[] = {0,0,0}, up[] = {0,1,0}, tmp[3];

	if (Keyboard_GetState(KEY_ESC, FALSE, TRUE)) exit(0);
	if (Keyboard_GetState('p', FALSE, TRUE)) {
		paused = !paused;
		pause_start = !pause_start;
		if (pause_start){
			pause_start_time = glutGet(GLUT_ELAPSED_TIME)/1000.00;
		}
		else {
			pause_end_time = glutGet(GLUT_ELAPSED_TIME)/1000;
			pause_duration = pause_end_time - pause_start_time;
			total_paused_time = total_paused_time + pause_duration;
		}
		
	}
	if (Keyboard_GetState(KEY_F1, FALSE, TRUE)) {
		Game_Start();
	}

	if (paused ||  down_count > 0 || game_end) return;
	

	if (Keyboard_GetState(KEY_F2, FALSE, TRUE)) {
		if (glIsEnabled(GL_TEXTURE_2D)) {
			glDisable(GL_TEXTURE_2D);
		}
		else {
			glEnable(GL_TEXTURE_2D);
		}
	}
	if (Keyboard_GetState(KEY_F3, FALSE, TRUE)) {
		DEBUG = !DEBUG;
	}
	if (Keyboard_GetState(KEY_F4, FALSE, TRUE)) {
		frame = 0;
	}
	if (Keyboard_GetState(KEY_F5, FALSE, TRUE)) {
		allowBlur = !allowBlur;
	}
	if (Keyboard_GetState(KEY_F6, FALSE, TRUE)) {
		mouse = !mouse;
	}

	for (i = 0; i < numPlayers; i++) {
		obj = &playerList[i].tank.obj;

		driving = 0;
		turning = 0;

		if (obj->onGround) {
			if (Keyboard_GetState(playerList[i].forwardKey, TRUE, FALSE)) {
				driving = 1;
			}
			if (Keyboard_GetState(playerList[i].backKey, TRUE, FALSE)) {
				driving = -1;
			}
			if (Keyboard_GetState(playerList[i].leftKey, TRUE, FALSE)) { /* turn left */
				if (obj->speed != 0) {
					turning = 1;
					if (!driving) d = obj->speed >= 0 ? 1 : -1;
					else d = driving;
					vec3f_rotp(obj->direction, z, up, 
						pow(playerList[i].tank.turnAbility * (obj->speed - obj->maxSpeed/2), 2) * d, obj->direction);
				}
			}
			if (Keyboard_GetState(playerList[i].rightKey, TRUE, FALSE)) { /* turn right */
				if (obj->speed != 0) {
					turning = 1;
					if (!driving) d = obj->speed >= 0 ? 1 : -1;
					else d = driving;
					vec3f_rotp(obj->direction, z, up, 
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
			if (allowBlur) {
				playerList[i].tank.tankBlur = turning;
			}
			else {
				playerList[i].tank.tankBlur = 0;
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
	float mag, dist;

	for (i = 0; i < numPlayers; i++) {
		obj = &playerList[i].tank.obj;

		if (mouse) { /* Noclip mode */
			vec3f_set(obj->force, tmp);
			vec3f_norm(tmp);
			vec3f_scale(tmp, 0.5, tmp);
			vec3f_add(tmp, obj->position, obj->position);
			vec3f_clear(obj->force);
			continue;
		}

		vec3f_clear(up);

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

		/* Get direction vector */
		obj->onGround = 0;
		Game_GenerateHitbox(obj, hitbox);
		for (x = 0; x < 8; x++) {
			vec3f_clear(hitboxadjust);
			vec3f_scale(obj->direction, -obj->length, tmp);
			vec3f_add(hitboxadjust, tmp, hitboxadjust);
			vec3f_scale(obj->upAngles, obj->height, tmp);
			vec3f_add(hitboxadjust, tmp, hitboxadjust);

			if (x >= 4) { /* Only use direction for top 4 points */
				vec3f_set(obj->force, tmp);
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
					vec3f_scale(cnormal, 1.3, cnormal); /* Walls are strong to not let player through */
				}

				if (strstr(bsp->data.textures[cface->texture].name, "grass")) {
					fric *= 1.25;
				}
				if (strstr(bsp->data.textures[cface->texture].name, "oil1")) {
					fric *= 0;
				}

				if (DEBUG) {
					printf("Player %d collided with '%s'\n", i, bsp->data.textures[cface->texture].name);
				}

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
			mag = 0.2 * obj->mass / (4 * obj->speed);
			if (fabs(mag) > 1.0f) mag = 1.0f;
			if (fabs(mag) < 0.05f) mag = 1.0f;
			vec3f_scale(tmp, vec3f_dot(tmp, obj->velocity) * mag, tmp);
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
		if (cface = bsp_face_collision(bsp, obj->position, ftot, FALSE)) {
			/* This total force has gone too far */
			dist = vec3f_planedist(obj->position, ftot, cface->normal, bsp->data.vertexes[cface->vertex].position);
			
			vec3f_scale(cface->normal, vec3f_dot(cface->normal, ftot), tmp);
			vec3f_sub(tmp, ftot, ftot);
//			vec3f_norm(ftot);
//			vec3f_scale(ftot, dist * 0.9, ftot);
			obj->onGround = TRUE;
		}

		vec3f_set(ftot, obj->force);

		vec3f_add(obj->velocity, obj->position, obj->position);
		vec3f_scale(obj->force, 1/obj->mass, obj->velocity);

		if (obj->position[1] < -2.875) {
			obj->position[1] = -2.87;
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
					playerList[i].checkpoint = 'S';
					if (playerList[i].lapNumber >= totalLaps) {
						game_end = 1;
					}
				}
				else if ((newCheckpoint == '1' && playerList[i].checkpoint == 'S') ||
						(newCheckpoint > playerList[i].checkpoint)) {
					playerList[i].checkpoint = newCheckpoint;
				}
			}
		}
	}
}

static void Game_HandleParticleEngine() {
	int i;
	for (i = 0; i < numPlayers; i++) {
		vec3f_set(playerList[i].tank.obj.position, playerList[i].pengine->startPosition);
		vec3f_scale(playerList[i].tank.obj.force, -0.05, playerList[i].pengine->startForce);
		playerList[i].pengine->startForce[1] += 0.05;
		ParticleEngine_Run(playerList[i].pengine);
	}

}

void Game_Run() {
	Game_HandleKeys();

	if (!paused) {
		Game_HandleDaylight();
		Game_RunPhysics();
		Game_Checkpoint();

		if (DEBUG) printf("Finished frame %.0f\n", frame);

		frame++;
	}
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

		if (DEBUG) {
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
		}

		glPushMatrix();
		glTranslatef(tank->obj.position[0], tank->obj.position[1], tank->obj.position[2]);

		/* Find out if there is lightvol info at the point */
		light = bsp_lightvol(bsp, tank->obj.position);
		if (light) {
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);

			pos[0] = 20.4 * cosd((float)light->dir[1] * 360.0 / 255.0);
			pos[1] = 20.4 * cosd((float)light->dir[0] * 360.0 / 255.0);
			pos[2] = 20.4 * -sind((float)light->dir[1] * 360.0 / 255.0);
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
//	Game_HandleParticleEngine();
}

static void Game_Render_Overlay(int playerNum) {
	Camera *camera = &playerList[playerNum].camera;
	Object *obj = &playerList[playerNum].tank.obj;

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 100, 0, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3d(1, 0, 0);
	text_output(2, 92, "Player %d", playerNum+1);
	if (!game_end) {
		text_output(2, 2, "Lap number: %d %s", playerList[playerNum].lapNumber + 1,
			playerList[playerNum].lapNumber + 1 == totalLaps ? "(FINAL LAP!)" : "");
	}

	if (game_end){
		handle_end_of_game(playerNum);
		}

	glColor3d(0, 1, 0);
	text_output(2, 50, playerList[playerNum].centerText);

	glColor3f(1, 0, 1);
	if ((game_started == 1) && (count_finished == 0)) {
		if (!paused && !game_end) {
			time_now = glutGet(GLUT_ELAPSED_TIME)/1000.00;
		}
		time_since_start = time_now - game_start_time - total_paused_time;
		down_count = 5-(int)time_since_start;
		if ((down_count > 0)&& (!paused)){
			text_output(45, 90, "%d", down_count);	
		}
		if ((down_count == 0)&&(!game_end)){
			text_output(45, 90, "GO!!!");
		}
		if ((down_count<0) && (!game_end)){
			if (!paused){
				time_now = glutGet(GLUT_ELAPSED_TIME)/1000.00 - 6;
			}
			time_since_start = time_now - game_start_time - total_paused_time;
			game_count_down = time_limit - time_since_start;
			text_output(78, 92, "Time Left: %.2f", game_count_down);
			if (game_count_down <= 0){
				game_end = 1;
		
			}//end of time_limit
		}//down_count <0
	}//if game_started and not game_end
	 
	
	

	if (paused || game_end) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_ALPHA, GL_SRC_ALPHA);
		glColor4f(0.2, 0.2, 0.2, 0.5);
		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(100, 0);
		glVertex2f(100, 100);
		glVertex2f(0, 100);
		glEnd();
		glDisable(GL_BLEND);

		glColor4f(1, 1, 1, 1);

		if (game_end) {
		handle_end_of_game(playerNum);
		return;
		}

		if (!game_end) {
			text_output2(42, 50, GLUT_BITMAP_TIMES_ROMAN_24, "PAUSED");
		}
	}
}

void Game_Render() {
	int i;

	for (i = 0; i < numPlayers; i++) {
		Game_Render_Scene(i);		/* Draw the scene (map) */
		Game_Render_Overlay(i);		/* Draw the overlay for the player */
	}
}