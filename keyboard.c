#include "common.h"
#include "keyboard.h"

static int key_states[300];

static void keyboard_down(unsigned char key, int x, int y);
static void keyboard_up(unsigned char key, int x, int y);
static void keyboard_special_down(int key, int x, int y);
static void keyboard_special_up(int key, int x, int y);

/* Returns the current state of a key. If `caseSensitive` is nonzero,
 * ignores case of a letter (w == W). If `switchOff` is nonzero,
 * the function will toggle off the key state from returning a pushed
 * value until it is released.
 */
int Keyboard_GetState(int key, int caseSensitive, int switchOff)
{
	int a, b, retval = 0;

	if (key < 256 && caseSensitive) {
		a = tolower(key);
		b = toupper(key);
		retval = key_states[a] == 1 || key_states[b] == 1;

		/* Switch the key off until it is released */
		if (retval && switchOff) {
			key_states[a] = -1;
			key_states[b] = -1;
		}
	}
	else {
		retval = key_states[key] == 1;

		/* Switch the key off until it is released */
		if (retval && switchOff) {
			key_states[key] = -1;
		}
	}
	
	return retval;
}

void Keyboard_Init()
{
	memset(key_states, 0, sizeof(key_states));

	glutKeyboardFunc(keyboard_down);
	glutKeyboardUpFunc(keyboard_up);
	glutSpecialFunc(keyboard_special_down);
	glutSpecialUpFunc(keyboard_special_up);
}

void keyboard_down(unsigned char key, int x, int y) 
{
	key_states[key] = 1;
}

static void keyboard_up(unsigned char key, int x, int y)
{
	key_states[toupper(key)] = 0;
	key_states[tolower(key)] = 0;
}

static int keyboard_glut_to_key(int key) {
	switch (key) {
		case GLUT_KEY_F1:			return KEY_F1;
		case GLUT_KEY_F2:			return KEY_F2;
		case GLUT_KEY_F3:			return KEY_F3;
		case GLUT_KEY_F4:			return KEY_F4;
		case GLUT_KEY_F5:			return KEY_F5;
		case GLUT_KEY_F6:			return KEY_F6;
		case GLUT_KEY_F7:			return KEY_F7;
		case GLUT_KEY_F8:			return KEY_F8;
		case GLUT_KEY_F9:			return KEY_F9;
		case GLUT_KEY_F10:			return KEY_F10;
		case GLUT_KEY_F11:			return KEY_F11;
		case GLUT_KEY_F12:			return KEY_F12;
		case GLUT_KEY_LEFT:			return KEY_LEFT;
		case GLUT_KEY_UP:			return KEY_UP;
		case GLUT_KEY_RIGHT:		return KEY_RIGHT;
		case GLUT_KEY_DOWN:			return KEY_DOWN;
		case GLUT_KEY_PAGE_UP:		return KEY_PAGE_UP;
		case GLUT_KEY_PAGE_DOWN:	return KEY_PAGE_DOWN;
		case GLUT_KEY_HOME:			return KEY_HOME;
		case GLUT_KEY_END:			return KEY_END;
		case GLUT_KEY_INSERT:		return KEY_INSERT;
	}
	return -1;
}

static void keyboard_special_down(int key, int x, int y)
{
	key_states[keyboard_glut_to_key(key)] = 1;
}

static void keyboard_special_up(int key, int x, int y)
{
	key_states[keyboard_glut_to_key(key)] = 0;
}
