#include "common.h"
#include "tank.h"

#define yNUM_TEXTURES 2
const enum yTextureType { 
	TEXTURE_YF,
	TEXTURE_SPIDY
} TextureType;

static GLuint textures[yNUM_TEXTURES];
static BOOL loaded_textures = FALSE;

static void drawYTank(Object *, Tank *);

/* Textures */
static char *texture_names[] = { 
	"textures/tank/yf11.jpg",
	"textures/tank/spides.jpg",
};

/* Constructor for cow tank */
void YTank_Init(Tank *self) {
	int i;

	Tank_Init(self);

	self->obj.drawFunc = (ObjectCallbackFunc*)drawYTank;

	/* Load textures */
	if (!loaded_textures) {
		for (i = 0; i < yNUM_TEXTURES; i++) {
			textures[i] = load_texture_jpeg(texture_names[i]);
		}
	}
	loaded_textures = TRUE;

	self->obj.width = 2;
	self->obj.height = 2;
	self->obj.length = 2;
}

typedef GLfloat Point[3];
Point pts[] = {
	{ -1.0, -1.0, 1.0 },
	{ -1.0, 1.0, 1.0 },
	{ 1.0, 1.0, 1.0 },
	{ 1.0, -1.0, 1.0 },
	{ -1.0, -1.0, -1.0 },
	{ -1.0, 1.0, -1.0 },
	{ 1.0, 1.0, -1.0 },
	{ 1.0, -1.0, -1.0 } };	// Definition of the vertices of the cube.

typedef GLfloat Normal[3];
Normal normals[] = {
	{ 0.0, 0.0, 1.0 },
	{ 1.0, 0.0, 0.0 },
	{ 0.0, -1.0, 0.0 },
	{ 0.0, 1.0, 0.0 },
	{ 0.0, 0.0, -1.0 },
	{ -1.0, 0.0, 0.0 }
};	// Definition of the face normal for each side of the cube.

GLdouble eqn[4] = {1.0, 1.0, 1.0, -2.0};	// Clipping planes ..
static int AnimateIncrement = 1;  // Time step for animation
static float CurrentStep = 0; //initial position of the turret
/********************
All tank dimensions go here
**********************/
static float body_length = 23;
static float body_height = 3;
static float body_width = 10;
static float body_clearance = 1.5;

static float turret_height = 3;
static float turret_length = 16;
static float turret_width = 11;
static float turret_center_Z = 1.5;

static float wheel_radius = 1;
static float wheel_thickness = 3;

static float trackcover_length = 23;
static float trackcover_width = 3.5;
static float trackcover_height = 2;

/**************************************
All track teeth parameters here
***************************************/
//for moving
static int move = 0;

static int number_teeth = 48; 

static float tooth_width = 3;
static float tooth_thickness = 0.25;
static float tooth_length = 0.9;
static double teeth_E_space = 0.10;
static double teeth_C_space = 1;

static double start_center_Z = -10.5;
static double start_center_Y = -1.0;
static double current_center_Z = 0;
static double current_center_Y = 0;
static double next_center_Z = 0;
static double next_center_Y = 0;
static double theta_increment = 60.0;

static double estimate_arc = 1; //current_center_Z - 9;
static double current_theta = 0; //aligned with -ive direction of the y axis
static double next_theta = 0;
static double current_theta_rad;
static double next_theta_rad;

static float track_movement = 150; //this is the speed at which the track moves.
static double track_increment; 
static double track_angle_increment_rad;
static double track_angle_increment;

/****************************************************************************************/
void ydrawSide (int v0, int v1, int v2, int v3, int n)
	// Draws one side of the cube and colors it.
{			
	glBegin(GL_QUADS);
	// All primitive drawings happen between glBegin / glEnd calls.	
	glNormal3fv(normals[n]);			
	glVertex3fv(pts[v0]);
	// OpenGL call to provide a vertex.
	glVertex3fv(pts[v1]);
	glVertex3fv(pts[v2]);
	glVertex3fv(pts[v3]);
	glEnd();
}

/****************************************************************************************/

/****************************************************************************************/
void ydrawCube (unsigned int key)
	// Draws the cube.
	// The key parameter could be used to decide upon the textures to be used for the 
	// respective shapes to be rendered !! Think :))

{
	ydrawSide(0, 3, 2, 1, 0);
	ydrawSide(3, 7, 6, 2, 1);
	ydrawSide(0, 4, 7, 3, 2);
	ydrawSide(1, 2, 6, 5, 3);
	ydrawSide(7, 4, 5, 6, 4);
	ydrawSide(4, 0, 1, 5, 5);
}

/**************************************************************************************/
void ydrawSphere(float fRadius, GLint slices, GLint rings)
	// Used to generate a Sphere shape.
{   
	GLUquadricObj* pObj;

	glColor4f(1, 1, 1, 1);
	pObj =  gluNewQuadric();
	// Creates a new quadrics object and returns a pointer to it.

	gluQuadricDrawStyle(pObj, GLU_FILL);

	gluSphere(pObj, fRadius,slices, rings);
	// Draw the sphere with a radius : fRadius.
	gluDeleteQuadric(pObj);
	// Free the Quadric

}
/********************************************************************************/
// Draw a textured cube
void drawTCube(float size)
{
	glBindTexture (GL_TEXTURE_2D, textures[TEXTURE_SPIDY]);
	glColor4f(1, 1, 1, 1);


	glPushMatrix();
	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-size/2, size/2, -size/2);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-size/2, size/2, size/2);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(size/2, size/2, size/2);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(size/2, size/2, -size/2);
	}
	glEnd();

	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-size/2, -size/2, -size/2);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-size/2, -size/2, size/2);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(size/2, -size/2, size/2);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(size/2, -size/2, -size/2);
	}
	glEnd();

	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-size/2, -size/2, size/2);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-size/2, size/2, size/2);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(size/2, size/2, size/2);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(size/2, -size/2, size/2);
	}
	glEnd();

	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-size/2, -size/2, -size/2);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-size/2, size/2, -size/2);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(size/2, size/2, -size/2);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(size/2, -size/2, -size/2);
	}
	glEnd();

	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(size/2, -size/2, -size/2);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(size/2, -size/2, size/2);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(size/2, size/2, size/2);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(size/2, size/2, -size/2);
	}
	glEnd();

	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-size/2, -size/2, -size/2);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-size/2, -size/2, size/2);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-size/2, size/2, size/2);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-size/2, size/2, -size/2);
	}
	glEnd();
	glPopMatrix();

}
/****************************************************************************************/
void ydrawCylinder(float fTopR, float fBottomR, float fHeight)
	// Used to generate a cylinder shape.
{
	// Bind the texture stored at the zero index of g_Texture[]
	GLUquadricObj *pObj = gluNewQuadric();				

	glPushMatrix();
	glBindTexture (GL_TEXTURE_2D, textures[TEXTURE_YF]);
	glPushAttrib(GL_ALL_ATTRIB_BITS); 
	glColor4f(1.0, 1.0, 1.0, 1.0);
	// This command is important as the current color largely decides the color of the texture rendered.
	// To keep the original texture intact we need to set the current color to WHITE.

	// Creates a new quadrics object and returns a pointer to it.
	gluQuadricNormals(pObj,GLU_SMOOTH);
	// specify one normal per vertex.
	gluQuadricOrientation(pObj, GLU_OUTSIDE);
	// For the quadrics object pObj,orientation is either GLU_OUTSIDE (the default) 
	// or GLU_INSIDE, which controls the direction in which normals are pointing. 
	gluQuadricTexture(pObj, GLU_TRUE);						
	// This turns on texture coordinates for our Quadric.
	gluQuadricDrawStyle(pObj, GLU_FILL);				

	gluCylinder(pObj, fTopR, fBottomR, fHeight, 20, 20);
	// Draw the cylinder with a radius : fRadius.
	gluDeleteQuadric(pObj);								
	// Free the Quadric
	glPopAttrib();
	glPopMatrix();
}

/****************************************************************************************/
void drawDisk(GLdouble innerRadius, GLdouble outerRadius, GLint slices, GLint rings,
	GLdouble startAngle, GLdouble sweepAngle)
	// Used to generate a cylinder shape.
{		
	GLUquadricObj* pObj;

	glBindTexture (GL_TEXTURE_2D, textures[TEXTURE_YF]);
	glPushAttrib(GL_ALL_ATTRIB_BITS); 
	glColor4f(1.0, 1.0, 1.0, 1.0);

	pObj = gluNewQuadric();
	// Creates a new quadrics object and returns a pointer to it.
	gluQuadricDrawStyle(pObj, GLU_FILL);

	gluPartialDisk(pObj, innerRadius, outerRadius, slices, rings, startAngle, sweepAngle);
	// Draw the cylinder with a radius : fRadius.
	gluDeleteQuadric(pObj);
}
/*********************************************/
void drawCarbOfEngine(void)
{
	int i;
	glPushMatrix();
	glColor3f(0.9, 0.9, 0.9);
	for(i = 0; i < 20; i++)
	{
		glTranslatef(0.2, 0.0, 0.0);
		glBegin(GL_QUADS);
		glVertex3f(-1,1,1);
		glVertex3f(-1,1,-1);
		glVertex3f(-1,-1,-1);
		glVertex3f(-1,-1,1);
		glEnd();
	}
	glPopMatrix();
}

void drawMG(void)
{

	glPushMatrix();
	/*
	MG has 4 parts: barrel, body, support and ammo-drum. The center of the body is 
	at the center of the local coord system. Color = black.
	*/

	/* this is MG body */

	glColor3f(0.5,0.5,0.5);
	glScalef(1, 2, 5);
	drawTCube(1.0);
	glPopMatrix();

	/* now draw MG barrel */

	glPushMatrix();
	//	glColor3f(1, 1, 1);
	glTranslatef(0, 0, 2.5);
	ydrawCylinder(0.4, 0.4, 8);
	glPushMatrix();
	glTranslatef(0,0,8);
	drawDisk(0, 0.4, 36, 20, 0, 360);
	glPopMatrix();
	glPopMatrix();


	/* daw ammo drum */
	glPushMatrix();
	glColor3f(0.5,0.5,0.5);
	glTranslatef(1.75, -1, -0.5);
	glScalef(2.5, 2.5, 0.9);
	//glutWireCube(1.0);
	drawTCube(1.0);
	glPopMatrix();

}

void drawBarrel(void)
{
	GLdouble calibre = 0.5;
	glPushMatrix();
	glColor3f(0.5,0.5,0.5);
	ydrawCylinder(calibre, calibre, 15);
	ydrawCube(1.0);
	glPushMatrix();
	glTranslatef(0, 0, 15);
	drawDisk(0, calibre, 36, 20, 0, 360);
	glPopMatrix();
	glPopMatrix();
}


void drawTuret(void)
{

	float CountIncrement= 1;
	float MaxAngle = 5;
	float MinAngle = -5;

	float CurrentCount = 0;
	float AngleIncrement = 0;
	float CurrentGunAngle = 10;

	float CurrentMGAngle = 0;
	float CurrentMGCount = 0;

	int CurrentDirection = 0; //0 for moving upward, 1 for downward

	glPushMatrix();
	glColor3f(0.5,0.5,0.5);
	glScalef(turret_width, turret_height, turret_length);
	drawTCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, turret_length/2); //center z is turret front coord
	glRotatef(CurrentGunAngle, 1,0,0);
	glRotatef(-10, 1,0,0);
	drawBarrel();
	glPopMatrix();

	/* draw MG support */
	glPushMatrix();

	glTranslatef(-2.5, 3, 1);
	glScalef(0.5, 0.5, 0.5);
	glColor3f(0.5, .5, 0.5);
	glTranslatef(0, -1, 1);
	glRotatef(90, 1, 0, 0);
	ydrawCylinder(0.5, 0.5, 2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-2.5, 3, 1);
	glRotatef(CurrentMGAngle, 0,0,1);
	glScalef(0.5, 0.5, 0.5);
	drawMG();
	glPopMatrix();
}
/********************************************************************************/
void drawTrackCover(void)
{

	glPushMatrix();
	glColor3f(0.5,0.5,0.5);
	glScalef(trackcover_width, trackcover_height, trackcover_length);
	drawTCube(1.0);
	glPopMatrix();

}
/********************************************************************************/
void drawLeftTrackCover(void)
{
	glPushMatrix();
	glTranslatef(body_width/2 + trackcover_width/2, body_height/2-trackcover_height/2, 0);
	drawTrackCover();
	glPopMatrix();
}
/********************************************************************************/
void drawRightTrackCover(void)
{
	glPushMatrix();
	glTranslatef(-(body_width/2 + trackcover_width/2), body_height/2-trackcover_height/2, 0);
	drawTrackCover();
	glPopMatrix();
}
/********************************************************************************/
void drawWheel(void)
{
	int wheel_radius = 1;
	int wheel_thickness = 3;
	glPushMatrix();
	glColor3f(0.5,0.5,0.5);
	ydrawCylinder(wheel_radius, wheel_radius, wheel_thickness);
	glPushMatrix();
	glColor3f(0.5,0.5,0.5); 
	drawDisk(0, wheel_radius, 36, 20, 0, 360);
	glTranslatef(0,0,3);
	drawDisk(0, wheel_radius, 36, 20, 0, 360);
	glPopMatrix();
	glPopMatrix();
}

/********************************************************************************/
void drawWheelz(void)
{
	int wheel_radius = 1;
	int wheel_thickness = 3;

	/*draw wheels on the left*/

	int i, numb_wheels = 8;
	GLdouble C_space = 3; //space between the centers of 2 adjacent wheels
	GLdouble E_space = 1; //space between the edges of 2 adjacent wheels
	GLdouble wheel_start_Z = -(7 * wheel_radius + 3.5 * E_space);

	for (i = 0; i < numb_wheels; i++) {
		glPushMatrix();
		glTranslatef(0, 0, wheel_start_Z + i* C_space);
		glRotatef(90,0,1,0);
		drawWheel();
		glPopMatrix();
	}
}

/********************************************************************************/
void drawLeftWheels(){
	glPushMatrix();
	glTranslatef(body_width/2, -(body_height/2 + body_clearance - wheel_radius), 0);
	drawWheelz();
	glPopMatrix();

}
/********************************************************************************/
void drawRightWheels(){
	glPushMatrix();
	glTranslatef(-body_width/2, -(body_height/2 + body_clearance - wheel_radius), 0);
	glRotatef(180, 0,1, 0);
	drawWheelz();
	glPopMatrix();

}
/********************************************************************************/
void drawTooth(void)
{
	glPushMatrix();
	glColor3f(0, 0, 0);
	glScalef(tooth_width, tooth_thickness, tooth_length);
	//glutWireCube   (1.0);
	glutSolidCube(1.0);
	glPopMatrix();
}
/********************************************************************************/
void drawTeeth(void)
{

	int i; 
	if (move == 0) {
		track_increment = 0.0;
	}
	else if (move == 1) {
		track_increment = 5*track_movement;
		move = 0;
	}
	else if (move == -1) {
		track_increment = -5*track_movement;
		move = 0;
	}
	else if (move == 2) {
		track_increment = track_movement;
	}
	else if (move == -2) {
		track_increment = -track_movement;
	}

	track_angle_increment_rad = track_increment/wheel_radius;
	track_angle_increment = (int)(track_angle_increment_rad / 180 * PI) % 360; 

	//next_center_Y = start_center_Y;
	next_center_Z = next_center_Z + track_increment;
	next_theta = next_theta + track_angle_increment;

	for (i = 0; i < number_teeth; i++){		
		if (next_center_Z <= -10.5 ) // as long as we are at the arc part of the rear wheel
		{
			current_theta = next_theta; //then the angle from previous calculation is good
			current_theta_rad = current_theta/180*PI;
			current_center_Y = next_center_Y; //+ wheel_radius * cos(current_theta); //calculate current y, z
			current_center_Z = next_center_Z; //- wheel_radius * sin(current_theta);

			glPushMatrix();
			glTranslatef(0, 0, -10.5);
			glRotatef(current_theta, 1,0,0);
			glTranslatef(0, -1, 0);
			drawTooth();
			glPopMatrix();

			next_theta = current_theta + theta_increment; //find out the next angle
			next_theta_rad = next_theta/180*PI;
			next_center_Y = - wheel_radius * cos(next_theta_rad); //calculate next y z for checking
			next_center_Z = -10.5 - wheel_radius * sin(next_theta_rad);
		}


		else if ((next_center_Z > -10.5) &&(next_center_Z <= 10.5)&& (next_center_Y > 0))
		{
			current_center_Z = current_center_Z + teeth_C_space;
			current_center_Y = 1;

			glPushMatrix();
			glTranslatef(0, current_center_Y, current_center_Z);
			drawTooth();
			glPopMatrix();

			next_center_Y = current_center_Y;
			next_center_Z = current_center_Z + teeth_C_space;
		}

		else if ((next_center_Z > 10.5) &&(next_theta >= 180)) // as long as we are at the arc part of the front wheel
		{
			current_theta = next_theta; //then the angle from previous calculation is good
			current_theta_rad = current_theta/180*PI;
			current_center_Y = next_center_Y; //+ wheel_radius * cos(current_theta); //calculate current y, z
			current_center_Z = next_center_Z; //- wheel_radius * sin(current_theta);

			glPushMatrix();
			glTranslatef(0, 0, 10.5);
			glRotatef(current_theta, 1,0,0);
			glTranslatef(0, -1, 0);
			drawTooth();
			glPopMatrix();

			next_theta = current_theta + theta_increment; //find out the next angle
			next_theta_rad = next_theta/180*PI;
			next_center_Y = - wheel_radius * cos(next_theta_rad); //calculate next y z for checking
			next_center_Z = 10.5 - wheel_radius * sin(next_theta_rad);
		}


		else if ((next_center_Z > -10.5) &&(next_center_Z <= 10.5) && (next_center_Y < 0))
		{
			current_center_Z = current_center_Z - teeth_C_space;
			current_center_Y = -1;

			glPushMatrix();
			glTranslatef(0, current_center_Y, current_center_Z);
			drawTooth();
			glPopMatrix();

			next_center_Y = current_center_Y;
			next_center_Z = current_center_Z - teeth_C_space;
		}
	}

}
/********************************************************************************/
void drawLeftTeeth(void){
	glPushMatrix();
	glTranslatef(body_width/2+tooth_width/2, -(body_height/2 + body_clearance - wheel_radius), 0);
	drawTeeth();
	glPopMatrix();
}
/********************************************************************************/
void drawRightTeeth(void){
	glPushMatrix();
	glTranslatef(-(body_width/2+tooth_width/2), -(body_height/2 + body_clearance - wheel_radius), 0);
	drawTeeth();
	glPopMatrix();
}
/********************************************************************************/
void ydrawBody(void)
{
	//this is the main frame

	glPushMatrix();

	glColor3f(0.0, 0.8, 0.0); //set to green
	glScalef(body_width, body_height, body_length); //body of the tank is aligned along Z axis
	drawTCube(1.0);
	glPopMatrix();

	glPushMatrix();	
	glTranslatef(0, body_height/2, -8.5);
	glRotatef(90, 0,1,0);
	glScalef(1, 0.1, 3 );
	drawCarbOfEngine();
	glPopMatrix();


	glPushMatrix();
	drawLeftTeeth();
	drawRightTeeth();	
	glPopMatrix();


	glPushMatrix();		
	drawLeftWheels();
	drawRightWheels();
	glPopMatrix();

	glPushMatrix();
	drawLeftTrackCover();
	drawRightTrackCover();
	glPopMatrix();
}


/****************************************************************************************/
void drawYTank(Object *self, Tank *tank)
{	
	glPushMatrix();
	glScaled(0.043478, 0.043478, 0.043478);
	ydrawBody();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, (turret_height/2 + body_height/2), turret_center_Z); //turret centered above the body, 1.5 towards the front.
	// center height is (turret_height + body_height)/2
	//x axis no change
	drawTuret();
	glPopMatrix();	

	glutSwapBuffers();
	glFlush();

}