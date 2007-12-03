#include "common.h"
#include "tank.h"

#define NUM_TEXTURES 3
const enum TextureType { 
	TEXTURE_SF, 
	TEXTURE_SMOKE, 
	TEXTURE_CAMOUFLAGES, 
} TextureType;

static void nDrawTank(Object *self, Tank *tank);

static GLuint textures[NUM_TEXTURES];
static BOOL loaded_textures = FALSE;

/* Textures */
static char *texture_names[] = {
	"textures/tank/sf.jpg",
	"textures/tank/smoke.jpg",
	"textures/tank/camouflages.jpg",
};

/* Constructor for cow tank */
void NTank_Init(Tank *self) {
	int i;

	Tank_Init(self);

	self->obj.drawFunc = (ObjectCallbackFunc*)nDrawTank;

	/* Load textures */
	if (!loaded_textures) {
		for (i = 0; i < NUM_TEXTURES; i++) {
			textures[i] = load_texture_jpeg(texture_names[i]);
		}
	}
	loaded_textures = TRUE;

	self->obj.width = 1.3;
	self->obj.height = 0.8;
	self->obj.length = 2.4;
	self->turnAbility = 0.6;
	self->obj.mass = 1.5;
}


typedef GLfloat Normal[3];
Normal normal[] = {
	{ 0.0, 0.0, 1.0 },
	{ 1.0, 0.0, 0.0 },
	{ 0.0, -1.0, 0.0 },
	{ 0.0, 1.0, 0.0 },
	{ 0.0, 0.0, -1.0 },
	{ -1.0, 0.0, 0.0 }
};			

typedef GLfloat Point[3];
Point pt[] = {
	{ -1.0, -1.0, 1.0 },
	{ -1.0, 1.0, 1.0 },
	{ 1.0, 1.0, 1.0 },
	{ 1.0, -1.0, 1.0 },
	{ -1.0, -1.0, -1.0 },
	{ -1.0, 1.0, -1.0 },
	{ 1.0, 1.0, -1.0 },
	{ 1.0, -1.0, -1.0 } };	// Defion of the vertices of the cube.


/***************************************************************************************/
// Used to generate the body using vertex 
void drawBody(void)
{
	glBindTexture (GL_TEXTURE_2D, textures[TEXTURE_CAMOUFLAGES]); 
	glPushMatrix();
	glColor3f(0.2, 0.5, 0.1);
	glBegin(GL_QUADS);			// Start Drawing Quads
	// Front Face
	//glColor3f(0.1, 0.1, 0.1);
	glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Facing Forward
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -1.0f,  3.0f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 6.0f, -1.0f,  3.0f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 8.0f,  1.0f,  3.0f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.0f,  1.0f,  3.0f);	// Top Left Of The Texture and Quad

	glNormal3f( 0.0f, 0.0f,-1.0f);		// Normal Facing Away
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.0f, -1.0f, -3.0f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.0f,  1.0f, -3.0f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 8.0f,  1.0f, -3.0f);	// Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 6.0f, -1.0f, -3.0f);	// Bottom Left Of The Texture and Quad

	// Top Face
	glNormal3f( 0.0f, 1.0f, 0.0f);		// Normal Facing Up
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.0f,  1.0f, -3.0f);	// Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.0f,  1.0f,  3.0f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 8.0f,  1.0f,  3.0f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 8.0f,  1.0f, -3.0f);	// Top Right Of The Texture and Quad

	// Bottom Face
	glNormal3f( 0.0f,-1.0f, 0.0f);		// Normal Facing Down
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-6.0f, -1.0f, -3.0f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 6.0f, -1.0f, -3.0f);	// Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 6.0f, -1.0f,  3.0f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.0f, -1.0f,  3.0f);	// Bottom Right Of The Texture and Quad

	// Right face
	glNormal3f( 1.0f, 0.0f, 0.0f);		// Normal Facing Right
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 6.0f, -1.0f, -3.0f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 8.0f,  1.0f, -3.0f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 8.0f,  1.0f,  3.0f);	// Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 6.0f, -1.0f,  3.0f);	// Bottom Left Of The Texture and Quad

	// Left Face
	glNormal3f(-1.0f, 0.0f, 0.0f);		// Normal Facing Left
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.0f, -1.0f, -3.0f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.0f, -1.0f,  3.0f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.0f,  1.0f,  3.0f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.0f,  1.0f, -3.0f);	// Top Left Of The Texture and Quad
	glEnd();					// Done Drawing Quads
	glPopMatrix();
}

/***************************************************************************************/
void drawDisks(GLdouble innerRadius, GLdouble outerRadius, GLint slices, GLint rings,
	GLdouble startAngle, GLdouble sweepAngle)

	// Used to generate a cylinder shape.
{	
	GLUquadricObj* pObj;

	glBindTexture (GL_TEXTURE_2D, textures[TEXTURE_CAMOUFLAGES]);	
	glColor4f(1.0, 1.0, 1.0, 1.0);

	pObj = gluNewQuadric();
	// Creates a new quadrics object and returns a pointer to it.
	gluQuadricDrawStyle(pObj, GLU_FILL);

	gluPartialDisk(pObj, innerRadius, outerRadius, slices, rings, startAngle, sweepAngle);
	gluDeleteQuadric(pObj);

	// Free the Quadric
}
/**************************** DRAW TURRETE************/
// Used to generate the turret using vertix same as the body 
void drawTurret(void)
{
	glBindTexture (GL_TEXTURE_2D, textures[TEXTURE_SF]);
	glPushMatrix();
	glColor3f(0.1, 0.7, 0.1);
	glBegin(GL_QUADS);			// Start Drawing Quads
	// Front Face

	glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Facing Forward
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.0f, 1.0f,  1.5f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 4.0f, 1.0f,  1.5f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 3.0f, 2.0f,  1.5f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-3.0f, 2.0f,  1.5f);	// Top Left Of The Texture and Quad

	glNormal3f( 0.0f, 0.0f,-1.0f);		// Normal Facing Away
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.0f, 1.0f, -1.5f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-3.0f, 2.0f, -1.5f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 3.0f, 2.0f, -1.5);	// Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 4.0f, 1.0f, -1.5f);	// Bottom Left Of The Texture and Quad

	// Top Face
	glNormal3f( 0.0f, 1.0f, 0.0f);		// Normal Facing Up
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.0f,  1.0f, -1.5f);	// Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.0f,  1.0f,  1.5f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 4.0f,  1.0f,  1.5f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 4.0f,  1.0f, -1.5f);	// Top Right Of The Texture and Quad

	// Bottom Face
	glNormal3f( 0.0f,-1.0f, 0.0f);		// Normal Facing Down
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-3.0f, 2.0f, -1.5f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 3.0f, 2.0f, -1.5f);	// Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 3.0f, 2.0f,  1.5f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-3.0f, 2.0f,  1.5f);	// Bottom Right Of The Texture and Quad

	// Right face
	glNormal3f( 1.0f, 0.0f, 0.0f);		// Normal Facing Right
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 4.0f, 1.0f, -1.5f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 3.0f,  2.0f, -1.5f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 3.0f,  2.0f,  1.5f);	// Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 4.0f, 1.0f,  1.5f);	// Bottom Left Of The Texture and Quad

	// Left Face
	glNormal3f(-1.0f, 0.0f, 0.0f);		// Normal Facing Left
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.0f, 1.0f, -1.5f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.0f, 1.0f,  1.5f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-3.0f,  2.0f,  1.5f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-3.0f,  2.0f, -1.5f);	// Top Left Of The Texture and Quad

	glEnd();					// Done Drawing Quads
	glPopMatrix();
}
/****************************************************************************************/
void drawuCylinder(float fTopR, float fBottomR, float fHeight)
	// Used to generate a cylinder shape.
{
	GLUquadricObj* pObj;
	// To keep the original texture intact we need to set the current color to WHITE.

	pObj = gluNewQuadric();
	// Creates a new quadrics object and returns a pointer to it.
	gluQuadricDrawStyle(pObj, GLU_FILL);

	gluCylinder(pObj, fTopR, fBottomR, fHeight, 20, 20);
	// Draw the cylinder with a radius : fRadius.
	gluDeleteQuadric(pObj);
	// Free the Quadric

}
/****************************************************************************************/
void drawCylinder(float fTopR, float fBottomR, float fHeight)
	// Used to generate a cylinder shape.

{
	// Bind the texture stored at the zero index of g_Texture[]
	GLUquadricObj *pObj = gluNewQuadric();				

	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS); 
	glColor4f(1.0, 1.0, 1.0, 1.0);
	// This command is important as the current color largely decides the color of the texture rendered.
	// To keep the original texture intact we need to set the current color to WHITE.
	glBindTexture (GL_TEXTURE_2D, textures[TEXTURE_SF]);
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

void drawSide (int v0, int v1, int v2, int v3, int n)
	// Draws one side of the cube and colors it.
{
	glBegin(GL_QUADS);
	// All primitive drawings happen between glBegin / glEnd calls.	
	glNormal3fv(normal[n]);			
	glVertex3fv(pt[v0]);
	// OpenGL call to provide a vertex.
	glVertex3fv(pt[v1]);
	glVertex3fv(pt[v2]);
	glVertex3fv(pt[v3]);
	glEnd();
}

/****************************************************************************************/
void drawCube (unsigned int key)
	// Draws the cube.
{
	drawSide(0, 3, 2, 1, 0);
	drawSide(3, 7, 6, 2, 1);
	drawSide(0, 4, 7, 3, 2);
	drawSide(1, 2, 6, 5, 3);
	drawSide(7, 4, 5, 6, 4);
	drawSide(4, 0, 1, 5, 5);
}
/****************************************************************************************/
void drawSphere(float fRadius, GLint slices, GLint rings)
	// Used to generate a Sphere shape.
{
	// Creates a new quadrics object and returns a pointer to it.
	GLUquadricObj *pObj = gluNewQuadric();				
	glColor4f(0.51,0.51,0.51, 0.7);

	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS); 
	glBindTexture(GL_TEXTURE_2D,textures[TEXTURE_CAMOUFLAGES]);
	// specify one normal per vertex.
	gluQuadricNormals(pObj,GLU_SMOOTH);
	// For the quadrics object pObj,orientation is either GLU_OUTSIDE (the default) 
	// or GLU_INSIDE, which controls the direction in which normals are pointing. 
	gluQuadricOrientation(pObj, GLU_OUTSIDE);
	// This turns on texture coordinates for our Quadric.
	gluQuadricTexture(pObj, GLU_TRUE);						
	gluQuadricDrawStyle(pObj, GLU_FILL);				
	// Draw the sphere with a radius
	gluSphere(pObj, fRadius,slices, rings);
	// Free the Quadric
	gluDeleteQuadric(pObj);															
	glPopAttrib();
	glPopMatrix();

}

/****************************************************************************************/
// Used to generate a cylinder shape to be used for the canon.
void nDrawCannon(void)
{	

	glPushMatrix();
	glScalef(6, 6, 6);
	glTranslatef(-.55, -.6 ,-.45);

	glPushMatrix();
	glColor4f(1, 0, 0, 1);

	glScalef(0.55, 0.45, .75);
	glColor3f(0.3, 0.5, 0.3);
	glTranslatef(-1.0, 3.3 ,11.5);
	glRotatef(110,0,0,1);
	glTranslatef(-2.3, -2, -11.5);
	glRotatef(90,1,0,0);
	// The cylinders representing the cannon
	glPushMatrix();
	glTranslatef(0, 0.5, 0.25); 
	drawCylinder(0.1, 0.1, 0.3);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, 0.55); 
	drawCylinder(0.1, 0.15, 0.05);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, 0.6); 
	drawCylinder(0.15, 0.15, 0.15);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, 0.75); 
	drawCylinder(0.15, 0.1, 0.05);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, 0.8); 
	drawCylinder(0.1, 0.1, 0.8);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, 1.6); 
	drawCylinder(0.1, 0.05, 0.1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, 1.7); 
	drawCylinder(0.05, 0.05, 0.25);
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();
}
/****************************************************************************************/
// Used to generate the machine gun.
void drawGun(void)
{
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glPushMatrix();
	glTranslatef(1.0, 0.0, 0.0);
	glScalef(0.3,0.3,0.4);
	glTranslatef(2.0, 9.75, 0.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	drawuCylinder(0.2, 0.2, 7.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(2.0, 3.0, 0.0);
	glScalef(0.75,0.15,0.1);
	drawCube(2);
	glPopMatrix();		

	glPushMatrix();
	glTranslatef(2.0, 2.5, 0.0);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	//draw the tripod for the machine gun using a triangle
	glBegin(GL_TRIANGLES);		// Drawing Using Triangles
	glVertex3f( 0.0f, 0.5f, 0.0f);		// Top
	glVertex3f(-0.5f,-0.5f, 0.0f);		// Bottom Left
	glVertex3f( 0.5f,-0.5f, 0.0f);		// Bottom Right
	glEnd();					// Finished Drawing
	glPopMatrix();
	glPopMatrix();
}
/****************************************************************************************/
void drawDome(void)
{
	glColor3f(0.2, 0.5, 0.1);
	glPushMatrix();	
	glRotatef(270.0, 1.0, 0.0, 0.0);
	glTranslatef(0,0,1.0);
	drawSphere(1.5, 15, 23);
	glPopMatrix();
}

/****************************************************************************************/
// Used to generate the wheels of the tank
void drawWheels(void)
{ 
	glColor3f(0.3, 0.5, 0.3);
	glPushMatrix();
	glPushMatrix();
	glTranslatef(-2.5,0.0, -3.5);
	drawDisks(0, 1.0, 36, 20, 0, 360);
	drawCylinder(1.0, 1.0, 7.0);
	glTranslatef(0,0,7.0);
	drawDisks(0, 1.0, 36, 20, 0, 360);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0,0.0, -3.5);
	drawDisks(0, 1.0, 36, 20, 0, 360);
	drawCylinder(1.0, 1.0, 7.0);
	glTranslatef(0,0,7.0);
	drawDisks(0, 1.0, 36, 20, 0, 360);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(2.5,0.0, -3.5);
	drawDisks(0, 1.0, 36, 20, 0, 360);
	drawCylinder(1.0, 1.0, 7.0);
	glTranslatef(0,0,7.0);
	drawDisks(0, 1.0, 36, 20, 0, 360);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(5,0.0, -3.5);
	drawDisks(0, 1.0, 36, 20, 0, 360);
	drawCylinder(1.0, 1.0, 7.0);
	glTranslatef(0,0,7.0);
	drawDisks(0, 1.0, 36, 20, 0, 360);
	glPopMatrix();

	glPopMatrix();
}
/****************************************************************************************/
void nDrawTank(Object *self, Tank *tank)
{
	glDisable(GL_CULL_FACE);
	glPushMatrix();
	glTranslatef(0.2, 0.1, 0);
	glRotatef(180, 0, 1, 0);
	glScalef(0.1,0.1,0.1);
	drawBody();
	nDrawCannon();
	drawWheels();
	drawGun();
	drawTurret();
	drawDome();
	glPopMatrix();
	glEnable(GL_CULL_FACE);
}

