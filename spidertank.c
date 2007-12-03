#include "common.h"
#include "tank.h"

#define KNUM_TEXTURES 3
const enum KTextureType { 
	TEXTURE_CHAIN, 
	TEXTURE_SPIDERMAN, 
	TEXTURE_SPIDY, 
} TextureType;

static GLuint textures[KNUM_TEXTURES];
static BOOL loaded_textures = FALSE;

static void drawTank(Object *self, Tank *tank);

/* Textures */
static char *texture_names[] = {
	"textures/tank/chain.jpg",
	"textures/tank/spiderman.jpg",
	"textures/tank/spides.jpg",
};

/* Constructor for SpiderTank */
void Spidertank_Init(Tank *self) {
	int i;

	Tank_Init(self);

	self->obj.drawFunc = (ObjectCallbackFunc*)drawTank;

	/* Load textures */
	if (!loaded_textures) {
		for (i = 0; i < KNUM_TEXTURES; i++) {
			textures[i] = load_texture_jpeg(texture_names[i]);
		}
	}
	loaded_textures = TRUE;

	/* Initialize basic object constants */
	self->obj.width = 1;
	self->obj.height = 1.2;
	self->obj.length = 2;
}

/********************************************************************************/
// Cylinder with textures
void drawTexCylinder(float fTopR, float fBottomR, float fHeight)
// Used to generate a cylinder shape.
{
	// Bind the texture stored at the zero index of g_Texture[]
	GLUquadricObj *pObj = gluNewQuadric();				

	glColor4f(0.51,0.51,0.51, 0.7);
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS); 
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_SPIDY]);


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
void drawKCylinder(float fTopR, float fBottomR, float fHeight)
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
void drawkDisk(GLdouble innerRadius, GLdouble outerRadius, GLint slices, GLint rings,
			   GLdouble startAngle, GLdouble sweepAngle)

			   // Used to generate a cylinder shape.
{	
	GLUquadricObj* pObj;

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glBindTexture (GL_TEXTURE_2D, textures[TEXTURE_CHAIN]);		

	pObj = gluNewQuadric();
	// Creates a new quadrics object and returns a pointer to it.
	gluQuadricDrawStyle(pObj, GLU_FILL);

	gluPartialDisk(pObj, innerRadius, outerRadius, slices, rings, startAngle, sweepAngle);
	gluDeleteQuadric(pObj);

	// Free the Quadric
}
/********************************************************************************/
// Similar to above, except draws a textured sphere
void drawTexSphere(float fRadius, GLint slices, GLint rings)
{
	// Creates a new quadrics object and returns a pointer to it.
	GLUquadricObj *pObj = gluNewQuadric();				

	glColor4f(0.51,0.51,0.51, 0.7);

	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS); 
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_SPIDY]);
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

/********************************************************************************/
// Draw a textured cube
void drawTexCube(float size)
{
	glBindTexture (GL_TEXTURE_2D, textures[TEXTURE_SPIDERMAN]);
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
/********************************************************************************/
// Creates the cannon
void drawCannon()
{
	glPushMatrix();
	glColor4f(1, 0, 0, 1);
	glBindTexture (GL_TEXTURE_2D, textures[TEXTURE_SPIDY]);

	// The cylinders representing the cannon
	glPushMatrix();
	glTranslatef(0, 0.5, 0.25); 
	drawTexCylinder(0.1, 0.1, 0.3);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, 0.55); 
	drawTexCylinder(0.1, 0.15, 0.05);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, 0.6); 
	drawTexCylinder(0.15, 0.15, 0.15);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, 0.75); 
	drawTexCylinder(0.15, 0.1, 0.05);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, 0.8); 
	drawTexCylinder(0.1, 0.1, 0.8);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, 1.6); 
	drawTexCylinder(0.1, 0.05, 0.1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.5, 1.7); 
	drawTexCylinder(0.05, 0.05, 0.25);
	glPopMatrix();

	glPopMatrix();
}
/********************************************************************************/
// Creates the machine gun
void drawMachineGun()
{
	glPushMatrix();
	glColor4f(0, 1, 0, 1);
	glBindTexture (GL_TEXTURE_2D, textures[TEXTURE_CHAIN]);
	glPushMatrix();
	// The cylinder representing part of the machine gun
	glTranslatef(0, 0.6, 0);
	glRotatef(-90, 1, 0, 0);
	drawTexCylinder(0.05, 0.05, 0.25);
	glPopMatrix();

	glPushMatrix();
	// The sphere connecting the parts of the machine gun
	glTranslatef(0, 0.85, 0);
	drawTexSphere(0.065,15,15);
	glPopMatrix();

	glPushMatrix();
	// The cylinder representing the barrel of the machine gun
	glTranslatef(0, 0.88, -0.03);
	drawTexCylinder(0.025, 0.025, 0.4);
	glPopMatrix();

	glPopMatrix();
}
/********************************************************************************/
// Creates the threads
void drawThreads()
{
	// The following correspond to the various internal threads
	// One side
	glPushMatrix();

	glPushMatrix();
	glTranslatef(0.3, -0.45, 0.8);
	glRotatef(90, 0, 1, 0);
	drawTexCylinder(0.2, 0.2, 0.2);
	drawkDisk(0, .2, 36, 20, 0, 360);
	glPopMatrix();

	glPushMatrix();
	glPushMatrix();

	glTranslatef(0.3, -0.45, 0.4);
	glRotatef(90, 0, 1, 0);
	drawTexCylinder(0.2, 0.2, 0.2);
	drawkDisk(0, .2, 36, 20, 0, 360);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.3, -0.45, 0);
	glRotatef(90, 0, 1, 0);
	drawTexCylinder(0.2, 0.2, 0.2);
	drawkDisk(0, .2, 36, 20, 0, 360);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.3, -0.45, -0.4);
	glRotatef(90, 0, 1, 0);
	drawTexCylinder(0.2, 0.2, 0.2);
	drawkDisk(0, .2, 36, 20, 0, 360);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.3, -0.45, -0.8);
	glRotatef(90, 0, 1, 0);
	drawTexCylinder(0.2, 0.2, 0.2);
	drawkDisk(0, .2, 36, 20, 0, 360);
	glPopMatrix();

	// The other side
	glPushMatrix();
	glTranslatef(-0.5, -0.45, 0.8);
	glRotatef(90, 0, 1, 0);
	drawTexCylinder(0.2, 0.2, 0.2);
	drawkDisk(0, .2, 36, 20, 0, 360);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.5, -0.45, 0.4);
	glRotatef(90, 0, 1, 0);
	drawTexCylinder(0.2, 0.2, 0.2);
	drawkDisk(0, .2, 36, 20, 0, 360);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.5, -0.45, 0);
	glRotatef(90, 0, 1, 0);
	drawTexCylinder(0.2, 0.2, 0.2);
	drawkDisk(0, .2, 36, 20, 0, 360);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.5, -0.45, -0.4);
	glRotatef(90, 0, 1, 0);
	drawTexCylinder(0.2, 0.2, 0.2);
	drawkDisk(0, .2, 36, 20, 0, 360);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.5, -0.45, -0.8);
	glRotatef(90, 0, 1, 0);
	drawTexCylinder(0.2, 0.2, 0.2);
	drawkDisk(0, .2, 36, 20, 0, 360);
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0, 0, 0);

	glBegin(GL_QUADS);
	glVertex3f(-0.55, -0.25, 1.25);
	glVertex3f(-0.25, -0.25, 1.25);
	glVertex3f(-0.25, -0.65, 0.9);
	glVertex3f(-0.55, -0.65, 0.9);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(-0.55, -0.25, -1.25);
	glVertex3f(-0.25, -0.25, -1.25);
	glVertex3f(-0.25, -0.65, -0.9);
	glVertex3f(-0.55, -0.65, -0.9);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(-0.55, -0.25, 1.25);
	glVertex3f(-0.25, -0.25, 1.25);
	glVertex3f(-0.25, -0.25, -1.25);
	glVertex3f(-0.55, -0.25, -1.25);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(-0.55, -0.65, 0.9);
	glVertex3f(-0.25, -0.65, 0.9);
	glVertex3f(-0.25, -0.65, -0.9);
	glVertex3f(-0.55, -0.65, -0.9);
	glEnd();

	// The other half
	glBegin(GL_QUADS);
	glVertex3f(0.55, -0.25, 1.25);
	glVertex3f(0.25, -0.25, 1.25);
	glVertex3f(0.25,-0.65,0.9);
	glVertex3f(0.55,-0.65,0.9);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(0.55, -0.25, -1.25);
	glVertex3f(0.25, -0.25, -1.25);
	glVertex3f(0.25,-0.65,-0.9);
	glVertex3f(0.55,-0.65,-0.9);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(0.55, -0.25, 1.25);
	glVertex3f(0.25, -0.25, 1.25);
	glVertex3f(0.25, -0.25, -1.25);
	glVertex3f(0.55, -0.25, -1.25);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(0.55, -0.65, 0.9);
	glVertex3f(0.25, -0.65, 0.9);
	glVertex3f(0.25,-0.65,-0.9);
	glVertex3f(0.55,-0.65,-0.9);
	glEnd();
	glPopMatrix();

	glPopMatrix();
}
/********************************************************************************/
//main function to draw kiran's tank 
void drawTank(Object *self, Tank *tank)
{
	glDisable(GL_CULL_FACE);
	glPushMatrix();
	glTranslated(0, 0.2, 0);
	glRotatef(-90, 0, 1, 0);
	glScalef(0.4,0.4,0.4);
	glPushMatrix();

	glScalef(1.0, .5 ,2.5);
	drawTexCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.25, 0);
	drawTexSphere(0.5,15,15);
	glPopMatrix();

	// Draw the cannon
	drawCannon();

	// Draw the machine gun
	drawMachineGun();

	// Draw the threads
	drawThreads();

	glPopMatrix();
	glPopMatrix();
	glEnable(GL_CULL_FACE);
}

