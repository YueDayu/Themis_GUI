#ifndef __WINDOW_MANAGER_H__
#define __WINDOW_MANAGER_H__

#define MSG_BUF_SIZE 50
#define MAX_TITLE_LEN 50

typedef struct win_rect
{
	int xmin, xmax, ymin, ymax;
} win_rect;

typedef struct msg_buf
{
	message data[MSG_BUF_SIZE];
	int front, rear, cnt;
} msg_buf;

typedef struct window
{
	win_rect contents;
	win_rect titlebar;
	msg_buf buf;
	char title[MAX_TITLE_LEN];
} window;

#endif
