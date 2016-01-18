// message definitions and constants

#ifndef __MSG_H__
#define __MSG_H__

#include "types.h"

#define M_KEY_UP 1
#define M_KEY_DOWN 2

#define M_KEY_CTRL 2
#define M_KEY_ALT 4
#define M_KEY_SHIFT 1

#define M_MOUSE_MOVE 3
#define M_MOUSE_DOWN 4
#define M_MOUSE_UP 5

#define WM_WINDOW_CLOSE 100
#define WM_WINDOW_REDRAW 101

typedef struct message {
	int msg_type;
	int params[10];
} message;

#endif

