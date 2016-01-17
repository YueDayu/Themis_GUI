#include "types.h"
#include "x86.h"
#include "defs.h"
#include "msg.h"
#include "spinlock.h"

#include "window_manager.h"

int isInRect(win_rect *rect, int x, int y)
{
	return (x >= rect->xmin && x <= rect->xmax && y >= rect->ymin && y <= rect->ymax);
}

void createRectBySize(win_rect *rect, int xmin, int ymin, int width, int height)
{
	rect->xmin = xmin;
	rect->xmax = xmin + width;
	rect->ymin = ymin;
	rect->ymax = ymin + height;
}

//return non-zero if buf is full
int enqueue(msg_buf *buf, message *msg)
{
	if (buf->cnt >= MSG_BUF_SIZE) return 1;
	++buf->cnt;
	buf->data[buf->rear] = *msg;
	if ((++buf->rear) >= MSG_BUF_SIZE) buf->rear = 0;
	return 0;
}

//return non-zero if buf is empty
int dequeue(msg_buf *buf, message *result)
{
	if (buf->cnt == 0) return 1;
	--buf->cnt;
	*result = buf->data[buf->front];
	if ((++buf->front) >= MSG_BUF_SIZE) buf->front = 0;
	return 0;
}

void initqueue(msg_buf *buf)
{
	buf->front = buf->rear = buf->cnt = 0;
}

#define MAX_WINDOW_CNT 50

//linked-list of windows
static struct
{
	window wnd;
	int next, prev;
} windowlist[MAX_WINDOW_CNT];

static struct
{
	float x, y;
} wm_mouse_pos;


static int windowlisthead, emptyhead;
static int focus;

struct spinlock wmlock;

void addToListHead(int *head, int idx)
{
	windowlist[idx].prev = -1;
	windowlist[idx].next = *head;
	if (*head != -1)
		windowlist[*head].prev = idx;
	*head = idx;
}

void removeFromList(int *head, int idx)
{
	if (*head == idx) *head = windowlist[*head].next;
	if (windowlist[idx].prev != -1)
		windowlist[windowlist[idx].prev].next = windowlist[idx].next;
	if (windowlist[idx].next != -1)
		windowlist[windowlist[idx].next].prev = windowlist[idx].prev;
}

void wmInit()
{
	windowlisthead = -1;
	emptyhead = 0;
	int i;
	for (i = 0; i < MAX_WINDOW_CNT; ++i)
	{
		windowlist[i].next = i + 1;
		windowlist[i].prev = i - 1;
	}
	windowlist[0].prev = -1;
	windowlist[MAX_WINDOW_CNT-1].next = -1;
	
	wm_mouse_pos.x = 100;
	wm_mouse_pos.y = 100;
	
	initlock(&wmlock, "wmlock");
}

void focusWindow(int handler)
{
	focus = handler;
}

//return window handler on succuss, -1 if unsuccessful
int createWindow(int width, int height, const char *title)
{
	if (emptyhead == -1) return -1;
	uint len = strlen(title);
	if (len >= MAX_TITLE_LEN) return -1;
	
	acquire(&wmlock);
	
	int idx = emptyhead;
	removeFromList(&emptyhead, idx);
	addToListHead(&windowlisthead, idx);
	
	initqueue(&windowlist[idx].wnd.buf);
	createRectBySize(&windowlist[idx].wnd.contents, 100, 100, width, height);
	createRectBySize(&windowlist[idx].wnd.titlebar, 100, 85, width, 15);
	memmove(windowlist[idx].wnd.title, title, len);
	
	//TODO draw window
	focusWindow(idx);
	
	release(&wmlock);
	
	return idx;
}

int createDesktopWindow()
{
	//TODO create full-screen window without titlebar
	return 0;
}

#define MOUSE_SPEED_X 0.005f
#define MOUSE_SPEED_Y -0.005f;

void dispatchMessage(int handler, message *msg)
{
	enqueue(&windowlist[handler].wnd.buf, msg);
}

void wmHandleMessage(message *msg)
{
	acquire(&wmlock);
	
	message newmsg;
	switch (msg->msg_type)
	{
	case M_MOUSE_MOVE:
		wm_mouse_pos.x += msg->params[0] * MOUSE_SPEED_X;
		wm_mouse_pos.y += msg->params[1] * MOUSE_SPEED_Y;
		if (wm_mouse_pos.x > SCREEN_WIDTH) wm_mouse_pos.x = SCREEN_WIDTH;
		if (wm_mouse_pos.y > SCREEN_HEIGHT) wm_mouse_pos.y = SCREEN_HEIGHT;
		if (wm_mouse_pos.x < 0) wm_mouse_pos.x = 0;
		if (wm_mouse_pos.y < 0) wm_mouse_pos.y = 0;
		//TODO redraw mouse cursor
		drawMouse(screen, 0, wm_mouse_pos.x, wm_mouse_pos.y);
		break;
	case M_MOUSE_DOWN:
		//TODO handle focus changes
		newmsg = *msg;
		newmsg.params[0] = wm_mouse_pos.x;
		newmsg.params[1] = wm_mouse_pos.y;
		newmsg.params[2] = msg->params[0];
		dispatchMessage(focus, &newmsg);
		break;
	case M_MOUSE_UP:
		//TODO
		newmsg = *msg;
		newmsg.params[0] = wm_mouse_pos.x;
		newmsg.params[1] = wm_mouse_pos.y;
		newmsg.params[2] = msg->params[0];
		dispatchMessage(focus, &newmsg);
		break;
	case M_KEY_DOWN:
		dispatchMessage(focus, msg);
		break;
	case M_KEY_UP:
		dispatchMessage(focus, msg);
		break;
	default:
		break;
	}
	
	release(&wmlock);
}

//return number of message (0 if buf is empty, 1 if not)
int wmGetMessage(int handler, message *res)
{
	acquire(&wmlock);
	//TODO check handler valid
	int ret = dequeue(&windowlist[handler].wnd.buf, res);
	release(&wmlock);
	if (ret) return 0;
	else return 1;
}


int sys_createwindow()
{
	int w, h;
	char *title;
	argint(0, &w);
	argint(1, &h);
	argstr(2, &title);
	return createWindow(w, h, title);
}

int sys_getmessage()
{
	int h;
	message *res;
	argint(0, &h);
	argptr(1, (char**)(&res), sizeof(message));
	return wmGetMessage(h, res);
}

