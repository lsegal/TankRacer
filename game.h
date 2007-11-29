#include "tank.h"
#include "camera.h"
#include "keyboard.h"

typedef struct Player {
	Camera	camera;
	Tank	tank;

	char	bottomText[128];
	char	centerText[128];
} Player;

extern void Game_Init();
extern void Game_Render();
extern void Game_Run();
extern void Game_Resize(int, int);