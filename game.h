#include "tank.h"
#include "camera.h"
#include "keyboard.h"

typedef struct Player {
	int		playerNum;
	Camera	camera;
	Tank	tank;

	int		forwardKey;
	int		backKey;
	int		leftKey;
	int		rightKey;

	char	bottomText[128];
	char	centerText[128];

	float   lastDir[10][3];
	float   numLast;
} Player;

extern void Game_Init();
extern void Game_Render();
extern void Game_Run();
extern void Game_Resize(int, int);