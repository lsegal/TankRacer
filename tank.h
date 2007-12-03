#ifdef __CPLUSPLUS
extern "C" {
#endif

#include "object.h"

/* Tank Model */
typedef struct Tank {
	Object		obj;				/* Object holder */
	GLdouble	barrelAngles[3];	/* Input 1: Barrel angles (needed for drawing) */
	double		treadPositions[2];  /* Input 2: Tread positions (needed for drawing) */

	/* Extra optional parameters (implemented by Cowtank) */
	BOOL		tankBlur;
	BOOL		treadBlur;
	BOOL		rusted;

	/* Add any other parameters here */
	double		turnAbility;
} Tank;

/* Base class */
extern void Tank_Init(Tank *tank);

/* Name your tanks! */
extern void Cowtank_Init(Tank *tank);
extern void Spidertank_Init(Tank *tank);
extern void YTank_Init(Tank *tank);
extern void NTank_Init(Tank *tank);

#ifdef __CPLUSPLUS
}
#endif