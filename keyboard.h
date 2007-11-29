#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifdef __CPLUSPLUS
extern "C" {
#endif

const enum VirtualKeys {
	KEY_ESC = 27,
	KEY_F1 = 256,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_LEFT,
	KEY_UP,
	KEY_RIGHT,
	KEY_DOWN,
	KEY_PAGE_UP,
	KEY_PAGE_DOWN,
	KEY_HOME,
	KEY_END,
	KEY_INSERT
} VirtualKeys;

extern int  Keyboard_GetState(int, int, int);
extern void Keyboard_Init();

#ifdef __CPLUSPLUS
}
#endif

#endif /* KEYBOARD_H */