#include "msg.h"
#include "types.h"
#include "defs.h"

int handleMessage(message* msg)
{
	if (msg->msg_type == M_KEY_UP)
	{
		cprintf("KEY UP: %x, %x\n", msg->params[0], msg->params[1]);
	}
	else if (msg->msg_type == M_KEY_DOWN)
	{
		cprintf("KEY DOWN: %x, %x\n", msg->params[0], msg->params[1]);
	}
	else if (msg->msg_type == M_MOUSE_MOVE)
	{
		cprintf("MOUSE MOVE: dx=%d, dy=%d, btn=%x\n", msg->params[0], msg->params[1], msg->params[2]);
	}
	else if (msg->msg_type == M_MOUSE_DOWN)
	{
		cprintf("MOUSE DOWN: btn=%x\n", msg->params[0]);
	}
	else if (msg->msg_type == M_MOUSE_UP)
	{
		cprintf("MOUSE UP: btn=%x\n", msg->params[0]);
	}
	return 0;
}

