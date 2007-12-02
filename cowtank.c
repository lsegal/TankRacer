#include "common.h"
#include "tank.h"

#define NUM_TEXTURES 10
const enum TextureType { 
	TEXTURE_TREADS, 
	TEXTURE_WHEELS, 
	TEXTURE_INNER_WHEELS, 
	TEXTURE_HULL_SIDE,
	TEXTURE_HULL_TOP,
	TEXTURE_TURRET_SIDE,
	TEXTURE_TURRET_TOP,
	TEXTURE_BARREL,
	TEXTURE_HIGHLIGHT,
	TEXTURE_MACHINEGUN
} TextureType;

static void Tank_Draw_Hull();
static void Tank_Build_Hull();
static void Tank_Draw_NotWheels(Object *self, Tank *tank);
static void Tank_Draw_Turret(Tank *tank);
static void Tank_Draw_Wheels(Tank *tank);
static void Tank_Draw(Object *self, Tank *tank);

static GLuint textures[NUM_TEXTURES];
static GLuint hullList = 0;
static BOOL loaded_textures = FALSE;

/* Textures */
static char *texture_names[] = {
	"textures/tank/treads.raw", /* TEXTURE_TREADS */
	"textures/tank/wheel.raw",  /* TEXTURE_WHEELS */
	"textures/tank/wheel2.raw",  /* TEXTURE_INNER_WHEELS */
	"textures/tank/hull_side.raw", /* TEXTURE_HULL_SIDE */
	"textures/tank/hull_top.raw", /* TEXTURE_HULL_TOP */
	"textures/tank/turret_side.raw", /* TEXTURE_TURRET_SIDE */
	"textures/tank/turret_top.raw", /* TEXTURE_TURRET_TOP */
	"textures/tank/barrel.raw", /* TEXTURE_BARREL */
	"textures/tank/highlight.raw", /* TEXTURE_HIGHLIGHT */
	"textures/tank/machinegun.raw", /* TEXTURE_MACHINEGUN */
};
static int texture_sizes[][2] = {
	{256, 256}, /* TEXTURE_TREADS */
	{200, 200}, /* TEXTURE_WHEELS */
	{200, 200}, /* TEXTURE_INNER_WHEELS */
	{512, 128}, /* TEXTURE_HULL_SIDE */
	{256, 256}, /* TEXTURE_HULL_TOP */
	{1280,128}, /* TEXTURE_TURRET_SIDE */
	{256, 256}, /* TEXTURE_TURRET_TOP */
	{128, 128}, /* TEXTURE_BARREL */
	{256, 256}, /* TEXTURE_HIGHLIGHT */
	{256, 256}, /* TEXTURE_MACHINEGUN */
};

/* Constructor for cow tank */
void Cowtank_Init(Tank *self) {
	int i;

	Tank_Init(self); /* Call constructor on superclass */

	/* Set draw function */
	self->obj.drawFunc  = (ObjectCallbackFunc*)Tank_Draw;

	/* Load textures */
	if (!loaded_textures) {
		for (i = 0; i < NUM_TEXTURES; i++) {
			textures[i] = load_texture_raw(texture_names[i], 
				texture_sizes[i][0], texture_sizes[i][1], GL_REPEAT);
		}
	}
	loaded_textures = TRUE;

	/* Build the tank hull */
	if (!hullList) {
		Tank_Build_Hull();
	}

	/* Initialize basic object constants */
	self->obj.mass = 1;
	self->obj.maxAccel = 0.03;
	self->obj.maxSpeed = 5;
	self->obj.width = 1.2;
	self->obj.height = 1.2;
	self->obj.length = 2;
	self->turnAbility = 1;
	self->tankBlur = FALSE;
}

/*--------------------------------------------------
 * void Tank_Draw(Object *self, Tank *tank)
 * 
 * Description:
 *
 * Draws a tank. This is the top of the heirarchy
 * for the tank drawing commands. The bottom of the tank's wheels
 * touch {0,0,0}, but all other drawing subroutines 
 * draw centered around {0,0,0} unless otherwise noted.
 *
 *-----------------------------------------------*/
void Tank_Draw(Object *self, Tank *tank) {
	int tankBody, i;
	GLfloat spec[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat specOff[] = {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat amb[] =  {0.7f, 0.7f, 0.7f, 1.0f};
	GLfloat diff[] = {0.7f, 0.5f, 0.5f, 1.0f};

	glPushMatrix();
	glPushAttrib(GL_LIGHTING_BIT);
		/* Set main tank colour */
		glColor3d(0.3, 0.3, 0.3);

		/* Set material properties */
		if (tank->rusted) {
			glMaterialfv(GL_FRONT, GL_SPECULAR, specOff);
		}
		else {
			glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
		}
		glMaterialf (GL_FRONT, GL_SHININESS, 10.0f);

		/* Lift tank up so wheels hit 0y */
		glTranslated(0, 0.18, 0);

		if (tank->tankBlur && fabs(self->speed) > 0.01) { /* Motion blur */
			double s = fabs(self->speed / self->maxSpeed);

			/* Load the tank body into a display list because it doesn't change */
			tankBody = glGenLists(1);
			glNewList(tankBody, GL_COMPILE_AND_EXECUTE);
			Tank_Draw_NotWheels(self, tank);
			glEndList();

			glAccum(GL_LOAD, 1);
			for (i = 1; i <= s * 5; i++) {
				double q = (s * i) / 100;
				double t = (self->speed >= 0 ? -1 : 1) * q * 2;

				/* Blur the tank in the negative velocity direction */
				glPushMatrix();
				glTranslated(t * self->force[0], t * self->force[1], t * self->force[2]);
				glScaled(1 + q * 2, 1 + q, 1 + q);
				glCallList(tankBody);
				glPopMatrix();

				/* Multiply the result in the color buffer with the accum buffer and re-expose */
				glAccum(GL_MULT, 0.9);
				glAccum(GL_ACCUM, 0.1);
			}
			glAccum(GL_RETURN, 1.0);	/* Return the final image to color buffer */
			glDeleteLists(tankBody, 1); /* Clear the memory for the list */
		} 
		else {
			Tank_Draw_NotWheels(self, tank);
		}

		glPushMatrix();
			glTranslated(0, -0.12, 0);
			Tank_Draw_Wheels(tank);
		glPopMatrix();

		glEnable(GL_COLOR_MATERIAL);
	glPopAttrib();
	glPopMatrix();
}

/* Draw the non wheel portion of the tank (for motion blurring) */
static void Tank_Draw_NotWheels(Object *self, Tank *tank) {
	double frame = tank->treadPositions[0] + tank->treadPositions[1];

	glPushMatrix();
		/* Make upper portion of tank bounce a bit from the engine */
		if (fabs(self->speed) > 0.01) {
			glTranslated(0, 0.001 * sin(40 * frame / (self->speed / self->maxSpeed)), 0);
		}

		glCallList(hullList);

		glPushMatrix();
		glPushAttrib(GL_TEXTURE_BIT);
			glTranslated(0, 0.22, 0);
			glRotated(tank->barrelAngles[1], 0, 1, 0); /* Barrel yaw */

			Tank_Draw_Turret(tank);
		glPopAttrib();
		glPopMatrix();

	glPopMatrix();
}

/* Compiles the hull into a display list */
static void Tank_Build_Hull() {
	hullList = glGenLists(1);
	glNewList(hullList, GL_COMPILE);
	Tank_Draw_Hull();
	glEndList();
}

/* Draw one side of the hull */
static void Tank_Draw_Hull_Side() {
	GLdouble side_top[][3] = {
		{-0.5,	0,		0},
		{-0.3,	0.15,	0},
		{-0.3,	0,		0},
		{0.3,	0,		0},
		{0.3,	0.15,	0},
		{0.5,	0,		0},
	};

	GLdouble side_mid[][3] = {
		{-0.5,	0,		0},
		{-0.4,	-0.07,	-0.02},
		{-0.4,	0,		0},
		{0.4,	0,		0},
		{0.4,	-0.07,	-0.02},
		{0.5,	0,		0},
	};

	GLdouble side_bot[][3] = {
		{-0.4,	-0.1,	-0.02},
		{0.4,	-0.1,	-0.02},
		{0.4,	-0.07,	-0.02},
		{-0.4,	-0.07,	-0.02},
	};

	GLdouble normal2[] = { 0, -sin(DEG2RAD(15)), cos(DEG2RAD(15)) };

	glPushMatrix();
	glPushAttrib(GL_TEXTURE_BIT | GL_CURRENT_BIT);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_HULL_SIDE]);

	glBegin(GL_TRIANGLES);
		glNormal3d(0, 0, 1);
		glTexCoord2d(0.0, 0.6);	glVertex3dv((GLdouble *)&side_top[0]);
		glTexCoord2d(0.2, 0.6); glVertex3dv((GLdouble *)&side_top[2]);
		glTexCoord2d(0.2, 0.0);	glVertex3dv((GLdouble *)&side_top[1]);
		glTexCoord2d(0.8, 0.6);	glVertex3dv((GLdouble *)&side_top[3]);
		glTexCoord2d(1.0, 0.6);	glVertex3dv((GLdouble *)&side_top[5]);
		glTexCoord2d(0.8, 0.0);	glVertex3dv((GLdouble *)&side_top[4]);

		glNormal3dv((GLdouble *)&normal2);
		glTexCoord2d(0.1, 0.88);	glVertex3dv((GLdouble *)&side_mid[1]);
		glNormal3d(0, 0, 1);
		glTexCoord2d(0.1, 0.60);	glVertex3dv((GLdouble *)&side_mid[2]);
		glTexCoord2d(0.0, 0.60);	glVertex3dv((GLdouble *)&side_mid[0]);
		glNormal3dv((GLdouble *)&normal2);
		glTexCoord2d(0.9, 0.88);	glVertex3dv((GLdouble *)&side_mid[4]);
		glNormal3d(0, 0, 1);
		glTexCoord2d(1.0, 0.60);	glVertex3dv((GLdouble *)&side_mid[5]);
		glTexCoord2d(0.9, 0.60);	glVertex3dv((GLdouble *)&side_mid[3]);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3d(0, 0, 1);
		glTexCoord2d(0.2, 0.6);		glVertex3dv((GLdouble *)&side_top[2]);
		glTexCoord2d(0.8, 0.6);		glVertex3dv((GLdouble *)&side_top[3]);
		glTexCoord2d(0.8, 0.0);		glVertex3dv((GLdouble *)&side_top[4]);
		glTexCoord2d(0.2, 0.0);		glVertex3dv((GLdouble *)&side_top[1]);

		glNormal3dv((GLdouble *)&normal2);
		glTexCoord2d(0.1, 0.88);	glVertex3dv((GLdouble *)&side_mid[1]);
		glTexCoord2d(0.9, 0.88);	glVertex3dv((GLdouble *)&side_mid[4]);
		glNormal3d(0, 0, 1);
		glTexCoord2d(0.9, 0.6);		glVertex3dv((GLdouble *)&side_mid[3]);
		glTexCoord2d(0.1, 0.6);		glVertex3dv((GLdouble *)&side_mid[2]);

		glTexCoord2d(0.1, 1.0);		glVertex3dv((GLdouble *)&side_bot[0]);
		glTexCoord2d(0.9, 1.0);		glVertex3dv((GLdouble *)&side_bot[1]);
		glTexCoord2d(0.9, 0.88);	glVertex3dv((GLdouble *)&side_bot[2]);
		glTexCoord2d(0.1, 0.88);	glVertex3dv((GLdouble *)&side_bot[3]);

		glNormal3d(0, 0, -1); /* Face is 2 sided */
		glTexCoord2d(0.1, 0.88);	glVertex3dv((GLdouble *)&side_bot[3]);		
		glTexCoord2d(0.9, 0.88);	glVertex3dv((GLdouble *)&side_bot[2]);
		glTexCoord2d(0.9, 1.0);		glVertex3dv((GLdouble *)&side_bot[1]);
		glTexCoord2d(0.1, 1.0);		glVertex3dv((GLdouble *)&side_bot[0]);
	glEnd();

	glPopAttrib();
	glPopMatrix();
}

/* Draw the half of the top of the hull */
static void Tank_Draw_Half_Top() {
	GLdouble top_top[][3] = {
		{-0.25,	0,		-0.3},
		{-0.25, 0,		 0.3},
		{-0.05, 0.15,	-0.3},
		{-0.05, 0.15,	 0.3},
		{ 0.25, 0.15,	-0.3},
		{ 0.25, 0.15,	 0.3}
	};

	GLdouble top_mid[][3] = {
		{-0.15, -0.07,	-0.28},
		{-0.15, -0.07,	 0.28},
		{-0.25, 0,		 0.3},
		{-0.25,	0,		-0.3},
	};

	GLdouble top_bot[][3] = {
		{-0.15, -0.1,	-0.15},
		{-0.15, -0.1,	 0.15},
		{-0.15, -0.07,	 0.15},
		{-0.15,	-0.07,	-0.15},
	};

	GLdouble normal1[] = { -cos(RAD2DEG(35)), -sin(RAD2DEG(35)), 0 };
	GLdouble normal2[] = { -cos(RAD2DEG(36.86)), sin(RAD2DEG(36.86)), 0 };

	glPushAttrib(GL_TEXTURE_BIT | GL_CURRENT_BIT);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_HULL_TOP]);
	glPushMatrix();
		glBegin(GL_QUADS);
			glNormal3d (-1, 0, 0);
			glTexCoord2d(0, 0.25);		glVertex3dv((GLdouble *)&top_bot[0]);
			glTexCoord2d(0, 0.75);		glVertex3dv((GLdouble *)&top_bot[1]);
			glTexCoord2d(0.043, 0.75);	glVertex3dv((GLdouble *)&top_bot[2]);
			glTexCoord2d(0.043, 0.25);	glVertex3dv((GLdouble *)&top_bot[3]);
			
			glNormal3dv((GLdouble *)&normal1);
			glTexCoord2d(0.043, 0);	glVertex3dv((GLdouble *)&top_mid[0]);
			glTexCoord2d(0.043, 1);	glVertex3dv((GLdouble *)&top_mid[1]);
			glNormal3d (-1, 0, 0);
			glTexCoord2d(0.217, 1);	glVertex3dv((GLdouble *)&top_mid[2]);
			glTexCoord2d(0.217, 0);	glVertex3dv((GLdouble *)&top_mid[3]);
		glEnd();
		glBegin(GL_QUAD_STRIP);
			glNormal3d (-1, 0, 0);
			glTexCoord2d(0.217, 0);	glVertex3dv((GLdouble *)&top_top[0]);
			glTexCoord2d(0.217, 1);	glVertex3dv((GLdouble *)&top_top[1]);
			glNormal3dv((GLdouble *)&normal2);
			glTexCoord2d(0.573, 0);	glVertex3dv((GLdouble *)&top_top[2]);
			glTexCoord2d(0.573, 1);	glVertex3dv((GLdouble *)&top_top[3]);
			glNormal3d (0, 1, 0);
			glTexCoord2d(1.0, 0);	glVertex3dv((GLdouble *)&top_top[4]);
			glTexCoord2d(1.0, 1);	glVertex3dv((GLdouble *)&top_top[5]);
		glEnd();
	glPopMatrix();
	glPopAttrib();
}

/* Draw tank hull */
static void Tank_Draw_Hull() {
	glPushMatrix();
		/* Left Side */
		glPushMatrix();
			glTranslated(0, 0, 0.3);
			Tank_Draw_Hull_Side();
		glPopMatrix();

		/* Right side */
		glPushMatrix();
			glTranslated(0, 0, -0.3);
			glRotated(180, 0, 1, 0);
			Tank_Draw_Hull_Side();
		glPopMatrix();

		/* Top */
		glPushMatrix();
			glTranslated(-0.25, 0, 0);
			Tank_Draw_Half_Top();
		glPopMatrix();
		glPushMatrix();
			glTranslated(0.25, 0, 0);
			glRotated(180, 0, 1, 0);
			Tank_Draw_Half_Top();
		glPopMatrix();
	glPopMatrix();
}

/* Draw main turret block */
static void Tank_Draw_Turret_Block() {
	int i;
	GLfloat top[][3] = {
		{0,		0.07,	-0.2},
		{-0.2,	0.07,	-0.15},
		{-0.2,	0.07,	 0.15},
		{0,		0.07,	 0.2},
		{0.2,	0.07,	 0.15},
		{0.2,	0.07,	-0.15}
	};
	float v[3], texCoord = 0, totalTex = 1.42462;

	glPushMatrix();
	glPushAttrib(GL_TEXTURE_BIT | GL_CURRENT_BIT);

		glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_TURRET_SIDE]);
		glBegin(GL_QUAD_STRIP);
			for (i = 0; i < 7; i++) {
				vec3f_set(top[i%6], v);
				v[1] = 0;
				vec3f_norm(v);
				glNormal3fv(v);

				if (i > 0) {
					vec3f_sub(top[i%6], top[i-1], v);
					texCoord += vec3f_mag(v);
				}

				glTexCoord2f(texCoord / totalTex, 1); 
				glVertex3f(top[i%6][X],  0.07, top[i%6][Z]);
				glTexCoord2f(texCoord / totalTex, 0); 
				glVertex3f(top[i%6][X], -0.07, top[i%6][Z]);
			}
		glEnd();
		
		glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_TURRET_TOP]);
		glBegin(GL_POLYGON);
			glNormal3d(0, 1, 0);
			for (i = 0; i < 6; i++) {
				glTexCoord2f((top[i][X] + 0.2) / 0.4, (top[i][Z] + 0.2) / 0.4);
				glVertex3fv((GLfloat *)&top[i]);
			}
		glEnd();
	glPopAttrib();
	glPopMatrix();
}

/* Draws the bulge on the tank barrel from 0,n on Z */
static void Tank_Draw_Turret_Barrel_Bulge() {
	GLUquadricObj *quad = gluNewQuadric();
	gluQuadricOrientation(quad, GLU_OUTSIDE); /* Normals point inwards for some reason */
	gluQuadricTexture(quad, GL_TRUE);

	glPushAttrib(GL_TEXTURE_BIT);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_HIGHLIGHT]);

	glPushMatrix();
		gluCylinder(quad,  0.03, 0.033, 0.01, 16, 1);
		glTranslated(0, 0, 0.01);
		gluCylinder(quad, 0.033, 0.033, 0.07, 16, 1);
		glTranslated(0, 0, 0.07);
		gluCylinder(quad, 0.033,  0.03, 0.01, 16, 1);
	glPopMatrix();
	glPopAttrib();

	gluDeleteQuadric(quad);
}

/* Draws a barrel from 0,1 on Z */
static void Tank_Draw_Turret_Barrel() {
	GLUquadricObj *quad = gluNewQuadric();
	gluQuadricOrientation(quad, GLU_OUTSIDE); 
	gluQuadricTexture(quad, GL_TRUE);

	/* Main barrel */
	glPushMatrix();
		gluCylinder(quad, 0.03, 0.03, 0.5, 16, 1);
		gluQuadricOrientation(quad, GLU_INSIDE); 
		gluCylinder(quad, 0.03, 0.03, 0.5, 16, 1);
	glPopMatrix();

	glPushMatrix();
	glPushAttrib(GL_CURRENT_BIT);
		/* Bulge 1 */
		glPushMatrix();
			glTranslated(0, 0, 0.04);
			Tank_Draw_Turret_Barrel_Bulge();
		glPopMatrix();

		/* Bulge 2 */
		glPushMatrix();
			glTranslated(0, 0, 0.38);
			Tank_Draw_Turret_Barrel_Bulge();
		glPopMatrix();
	glPopAttrib();
	glPopMatrix();

	gluDeleteQuadric(quad);
}

static void Tank_Draw_Turret_Hub() {
	GLUquadricObj *quad = gluNewQuadric();
	gluQuadricOrientation(quad, GLU_OUTSIDE); /* Point normals outwards */
	gluQuadricTexture(quad, GL_TRUE);

	glPushMatrix();
		/* Move entire wheel back by 0.05 on Z because glu object
		 * Draw from 0 to h on the Z axis */
		glTranslated(0, 0, -0.05);

		glPushMatrix();
			gluCylinder(quad, 0.05, 0.05, 0.1, 16, 1);
			gluQuadricOrientation(quad, GLU_INSIDE); /* Point normals outwards */
			gluDisk(quad, 0, 0.05, 16, 1);
		glPopMatrix();

		glPushMatrix();
			glTranslated(0, 0, 0.1);
			glRotated(180, 0, 1, 0); /* To get the normal pointing the right way */
			gluDisk(quad, 0, 0.05, 16, 1);
		glPopMatrix();
	glPopMatrix();
		
	gluDeleteQuadric(quad);
}

/* Draw the thicker lip of barrel on z axis */
static void Tank_Draw_Machinegun_Barrel_Lip() {
	GLUquadricObj *quad = gluNewQuadric();
	gluQuadricOrientation(quad, GLU_OUTSIDE); 
	gluQuadricTexture(quad, GL_TRUE);

	glPushMatrix();
		gluDisk(quad, 0.01, 0.013, 16, 1);
		gluCylinder(quad, 0.013, 0.013, 0.02, 16, 1);
		gluQuadricOrientation(quad, GLU_INSIDE);
		gluDisk(quad, 0.01, 0.013, 16, 1);
		gluCylinder(quad, 0.013, 0.013, 0.02, 16, 1);
		gluQuadricOrientation(quad, GLU_OUTSIDE);
		glTranslated(0, 0, 0.02);
		gluDisk(quad, 0.01, 0.013, 16, 1);
		gluQuadricOrientation(quad, GLU_INSIDE);
		gluDisk(quad, 0.01, 0.013, 16, 1);
	glPopMatrix();

	gluDeleteQuadric(quad);
}

/* Draws the machinegun barrel down z axis */
static void Tank_Draw_Machinegun_Barrel() {
	GLUquadricObj *quad = gluNewQuadric();
	gluQuadricOrientation(quad, GLU_OUTSIDE); 
	gluQuadricTexture(quad, GL_TRUE);

	glPushMatrix(); 
		/* Main barrel */
		gluCylinder(quad, 0.01, 0.01, 0.2, 16, 1);
		gluQuadricOrientation(quad, GLU_INSIDE); 
		gluCylinder(quad, 0.01, 0.01, 0.2, 16, 1);

		/* Lip of barrel (thicker) */
		glTranslated(0, 0, 0.18);
		Tank_Draw_Machinegun_Barrel_Lip();
	glPopMatrix();

	gluDeleteQuadric(quad);
}

/* Draw the base of the machine gun */
static void Tank_Draw_Machinegun_Base() {
	glPushMatrix();
		draw_cube(0.015, 0.05, 0.011);

		glPushMatrix();
			glTranslated(0, 0.055, 0);
			draw_cube(0.04, 0.06, 0.06);
		glPopMatrix();
	glPopMatrix();
}

static void Tank_Draw_Machinegun() {
	glPushMatrix();
		Tank_Draw_Machinegun_Base();

		glPushMatrix();
			glScaled(0.7, 1, 1);

			glPushMatrix();
				glTranslated(-0.02, 0.043, 0);
				glRotated(-90, 0, 1, 0);
				glScaled(0.7, 0.7, 0.8);
				Tank_Draw_Machinegun_Barrel();
			glPopMatrix();

			glPushMatrix();
				glTranslated(-0.02, 0.07, 0);
				glRotated(-90, 0, 1, 0);
				Tank_Draw_Machinegun_Barrel();
			glPopMatrix();
		glPopMatrix();
	glPopMatrix();
}

/* Draws the whole turret with the center of the turrent block around 0,0 */
static void Tank_Draw_Turret(Tank *tank) { 
	glPushMatrix();
	glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT);
		Tank_Draw_Turret_Block();

		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 20.0f);

		glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_BARREL]);
		glPushMatrix();
			glTranslated(-0.2, 0, 0);
			glRotated(tank->barrelAngles[2], 0, 0, 1); /* Barrel pitch */
			glRotated(-90, 0, 1, 0);
			Tank_Draw_Turret_Barrel();
		glPopMatrix();

		glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_HIGHLIGHT]);
		glPushMatrix(); /* Draw semisphere in front of barrel so it can rotate */
			glTranslated(-0.2, 0, 0);
			glScaled(0.45, 1, 0.8);
			Tank_Draw_Turret_Hub();
		glPopMatrix();

		glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_MACHINEGUN]);
		glPushMatrix();
			glTranslated(-0.05, 0.08, -0.07);
			glRotated(tank->barrelAngles[0], 1, 0, 0); /* Machinegun roll */
			glScaled(0.8, 0.8, 0.8);
			Tank_Draw_Machinegun();
		glPopMatrix();

		/* Antenna doesnt shine */
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
		glPushMatrix(); /* Draw an antenna */
			glRotated(-4 - 2 * sin((tank->treadPositions[0] + tank->treadPositions[1]) / 12), 0, 0, 1);
			glTranslated(0.08, 0.14, 0.09);
			draw_cube(0.002, 0.14, 0.002);
		glPopMatrix();
	glPopAttrib();
	glPopMatrix();
}

/* Draws one piece of the tread around 0,0 */
static void Tank_Draw_Tread_Piece() {
	glPushMatrix();
		glScaled(0.35, 1, 1);

		draw_cube(0.17, 0.011, 0.103);

		glPushMatrix();
			glTranslated(-0.05, -0.005, 0);
			draw_cube(0.03, 0.01, 0.103);
		glPopMatrix();		

		glPushMatrix();
			glTranslated(0, -0.005, 0);
			draw_cube(0.03, 0.01, 0.103);
		glPopMatrix();		

		glPushMatrix();
			glTranslated(0.05, -0.005, 0);
			draw_cube(0.03, 0.01, 0.103);
		glPopMatrix();		
	glPopMatrix();
}

/* Draws the whole tread around 0,0 */
static void Tank_Draw_Tread(Tank *tank, double start) {
	double numTreads = 45.0;
	int i;
	double theta, thetar, x, y;
	for (i = 0; i < numTreads; i++) {
		theta = (int)((360.0/numTreads * i) + start) % 360;
		thetar = theta * PI / 180;

		glPushMatrix();
			x = 0.4  * cos(thetar);
			y = 0.06 * sin(thetar);

			glTranslated(x, 0, 0);

			if (y > 0) {
				glTranslated(0, 0.102, 0);
				glRotated(180, 0, 0, 1);
			}
			else {
				if (fabs(x) > 0.3) {
					glTranslated(0.02 * x/fabs(x), (fabs(x)-0.3)/0.25 * 0.2, 0);
					glRotated(1000 * x*x*x, 0, 0, 1);
				}
			}

			Tank_Draw_Tread_Piece();
		glPopMatrix();
	}
}

/* Draws one wheel around 0,0 */
static void Tank_Draw_Wheel() {
	GLUquadricObj *quad = gluNewQuadric();
	gluQuadricOrientation(quad, GLU_OUTSIDE); /* Point normals outwards */
	gluQuadricTexture(quad, GL_TRUE);

	glPushAttrib(GL_TEXTURE_BIT);
	glPushMatrix();
		/* Move entire wheel back by 0.05 on Z because glu object
		 * Draw from 0 to h on the Z axis */
		glTranslated(0, 0, -0.05);

		glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_INNER_WHEELS]);
		gluCylinder(quad, 0.05, 0.05, 0.1, 16, 1);
		
		glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_WHEELS]);
		gluQuadricOrientation(quad, GLU_INSIDE); /* Point normals outwards */
		gluDisk(quad, 0, 0.05, 16, 1);

		glPushMatrix();
			glTranslated(0, 0, 0.1);
			gluQuadricOrientation(quad, GLU_OUTSIDE); /* Point normals outwards */
			gluDisk(quad, 0, 0.05, 16, 1);
		glPopMatrix();
	glPopMatrix();
	glPopAttrib();
		
	gluDeleteQuadric(quad);
}

/* Draws the set of wheels including the tread centered at 0,0 */
static void Tank_Draw_WheelSet(Tank *tank, double wheelStart) {
	int i, numwheels = 6;
	GLdouble shift = 0.105;
	GLdouble start = -shift * (GLdouble)numwheels / 2 + shift / 2;

	glPushMatrix();
		/* Front elevated wheel */
		glPushMatrix();
			glTranslated(start - shift * 0.77, 0.023, 0);
			glRotated(wheelStart * 3, 0, 0, 1);
			glScaled(0.55, 0.55, 1);
			Tank_Draw_Wheel();
		glPopMatrix();

		/* Main wheels */
		glPushMatrix();
			glTranslated(start, 0, 0);

			for (i = 0; i < numwheels; i++) {
				glPushMatrix();
					glRotated(wheelStart * 3 + i * 25, 0, 0, 1);
					glTranslated(0.002, 0, 0);

					Tank_Draw_Wheel();
				glPopMatrix();

				glTranslated(shift, 0, 0);

			}
		glPopMatrix();

		/* Back elevated wheel */
		glPushMatrix();
			glTranslated(start + (numwheels-1) * shift + shift * 0.77, 0.02, 0);
			glRotated(wheelStart * 3, 0, 0, 1);
			glScaled(0.55, 0.55, 1);
			Tank_Draw_Wheel();
		glPopMatrix();
	glPopMatrix();

	/* Treads */
	glPushMatrix();
	glPushAttrib(GL_TEXTURE_BIT);
		glTranslated(0, -0.05, 0);
		glScaled(0.9, 1, 1);
		glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_TREADS]);
		Tank_Draw_Tread(tank, wheelStart);
	glPopAttrib();
	glPopMatrix();
}

/* Draw two treads with wheels on each side of the tank */
static void Tank_Draw_Wheels(Tank *tank) { 
	int x, i;

	for (x = 0; x < 2; x++) {
		/* Wheel Set */
		glPushMatrix();
			glTranslated(0, 0, (x==0?1:-1) * 0.21);
			Tank_Draw_WheelSet(tank, tank->treadPositions[x]);

			if (tank->treadBlur && fabs(tank->obj.speed) > 0.01) { /* Motion blur */
				double s = fabs(tank->obj.speed / tank->obj.maxSpeed);

				glAccum(GL_LOAD, 1); /* Load whatever we rendered so far into the accum buffer */

				for (i = 1; i <= s * 6; i++) { /* 6 Passes */
					double q = (s * i) / 100;

					glPushMatrix();
					glScaled(1, 1 + q * 2, 1 + q * 2); /* Blur the treads outwards */
					Tank_Draw_WheelSet(tank, tank->treadPositions[x] - q * 100);
					glPopMatrix();

					/* Multiply result with previous image and re-expose to get the values to 1 */
					glAccum(GL_MULT, 0.8);
					glAccum(GL_ACCUM, 0.2);
				}

				/* Return our final image back to the color buffer and continue rendering */
				glAccum(GL_RETURN, 1.0);
			}

		glPopMatrix();
	}
}