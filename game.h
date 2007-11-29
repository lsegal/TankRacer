#include "tank.h"
#include "camera.h"
#include "keyboard.h"

typedef struct Player {
	Camera	camera;
	Tank	tank;
} Player;

extern void Game_Init();
extern void Game_Render();
extern void Game_Run();
extern void Game_Resize(int, int);