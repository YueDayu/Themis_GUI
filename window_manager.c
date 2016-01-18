#include "types.h"
#include "x86.h"
#include "param.h"
#include "defs.h"
#include "msg.h"
#include "spinlock.h"
#include "gui_base.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

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
	struct proc* proc;
	window wnd;
	int next, prev;
} windowlist[MAX_WINDOW_CNT];

static struct
{
	float x, y;
} wm_mouse_pos, wm_last_mouse_pos;


static int windowlisthead, emptyhead;
static int desktopHandler = -100;
static int focus;
static int frontcnt;

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

	wm_mouse_pos.x = SCREEN_WIDTH / 2;
	wm_mouse_pos.y = SCREEN_HEIGHT / 2;

	wm_last_mouse_pos = wm_mouse_pos;
	
	frontcnt = 0;

	initlock(&wmlock, "wmlock");
}

void getWindowRect(int handler, win_rect *res)
{
    window *wnd = &windowlist[handler].wnd;
    res->xmin = wnd->titlebar.xmin;
    res->ymin = wnd->titlebar.ymin;
    res->xmax = wnd->titlebar.xmax;
    res->ymax = wnd->contents.ymax + 3;
}

void drawWindow(int layer, int handler)
{
	window *wnd = &windowlist[handler].wnd;
    struct RGBA barcolor, wndcolor, txtcolor, closecolor;
    barcolor.R = 170; barcolor.G = 150; barcolor.B = 100; barcolor.A = 255;
    if (layer == 2) barcolor.R = barcolor.G = barcolor.B = 140;
    wndcolor.R = wndcolor.G = wndcolor.B = wndcolor.A = 255;
    closecolor.R = 200; closecolor.G = 50; closecolor.B = 10;
    txtcolor = wndcolor;

    struct RGB *dst;
    if (layer == 2) dst = screen_buf2;
    else if (layer == 1) dst = screen_buf1;
    else dst = screen;

	if (handler != desktopHandler) {
		drawRectByCoord(dst, wnd->titlebar.xmin, wnd->titlebar.ymin, wnd->titlebar.xmax, wnd->contents.ymax + 3,
						barcolor);
		drawRectByCoord(dst, wnd->titlebar.xmax - 30, wnd->titlebar.ymin, wnd->titlebar.xmax - 3, wnd->titlebar.ymax,
						closecolor);
		drawRectByCoord(dst, wnd->contents.xmin, wnd->contents.ymin, wnd->contents.xmax, wnd->contents.ymax, wndcolor);
		drawString(dst, wnd->titlebar.xmin + 5, wnd->titlebar.ymin + 3, wnd->title, txtcolor);
	}

    if (layer >= 2)
        clearRectByCoord(screen_buf1, screen_buf2, wnd->titlebar.xmin, wnd->titlebar.ymin, wnd->titlebar.xmax, wnd->contents.ymax + 3);
    if (layer >= 1)
        clearRectByCoord(screen, screen_buf1, wnd->titlebar.xmin, wnd->titlebar.ymin, wnd->titlebar.xmax, wnd->contents.ymax + 3);

    //TODO fire REDRAW message to application
}

void focusWindow(int handler)
{
    if (frontcnt) return;

	if (handler != desktopHandler) {
		removeFromList(&windowlisthead, handler);
		addToListHead(&windowlisthead, handler);
	}

	//redraw all occluded window
	int p, q;
	for (p = windowlisthead; p != -1; p = windowlist[p].next) q = p;
	for (p = q; p != windowlisthead; p = windowlist[p].prev) drawWindow(2, p);
	drawWindow(1, windowlisthead);
	drawMouse(screen, 0, wm_mouse_pos.x, wm_mouse_pos.y);

	focus = handler;
}

//return window handler on succuss, -1 if unsuccessful
int createWindow(int width, int height, const char *title, struct RGB *buf, int alwaysfront)
{
	if (emptyhead == -1) return -1;
	uint len = strlen(title);
	if (len >= MAX_TITLE_LEN) return -1;
	if (alwaysfront && frontcnt) return -1;

	acquire(&wmlock);

	int idx = emptyhead;
	removeFromList(&emptyhead, idx);
	addToListHead(&windowlisthead, idx);

	initqueue(&windowlist[idx].wnd.buf);
	//initial window position according to idx
	int offsetX = (100 + idx * 47) % (SCREEN_WIDTH - 130) + 30;
	int offsetY = (100 + idx * 33) % (SCREEN_HEIGHT - 130) + 30;
	if (len == 0 && desktopHandler == -100) {
		desktopHandler = idx;
		createRectBySize(&windowlist[idx].wnd.contents, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		createRectBySize(&windowlist[idx].wnd.titlebar, 0, 0, SCREEN_WIDTH, 0);
	} else {
		createRectBySize(&windowlist[idx].wnd.contents, offsetX, offsetY, width, height);
		createRectBySize(&windowlist[idx].wnd.titlebar, offsetX - 3, offsetY - 20, width + 6, 20);
		memmove(windowlist[idx].wnd.title, title, len);
	}
	windowlist[idx].proc = proc; // remember current process
	windowlist[idx].wnd.alwaysfront = alwaysfront;
	windowlist[idx].wnd.content_buf = buf;
	
    //drawing is completed in focusWindow EXCEPT when there was an always-front window
	focusWindow(idx);
	if (frontcnt)
	{
	    drawWindow(2, idx);
	    drawMouse(screen, 0, wm_mouse_pos.x, wm_mouse_pos.y);
	}
	
    if (alwaysfront) ++frontcnt;

	release(&wmlock);

	return idx;
}

int createDesktopWindow()
{
	//TODO create full-screen window without titlebar
	return 0;
}

void destroyWindow(int handler)
{
    acquire(&wmlock);

    if (handler != focus) focusWindow(handler);
    //clear window on screen
    window *wnd = &windowlist[handler].wnd;
    clearRectByCoord(screen_buf1, screen_buf2, wnd->titlebar.xmin, wnd->titlebar.ymin, wnd->titlebar.xmax, wnd->contents.ymax + 3);
    clearRectByCoord(screen, screen_buf2, wnd->titlebar.xmin, wnd->titlebar.ymin, wnd->titlebar.xmax, wnd->contents.ymax + 3);
    drawMouse(screen, 0, wm_mouse_pos.x, wm_mouse_pos.y);
    
    if (wnd->alwaysfront) --frontcnt;

    //choose next window to focus
    int newfocus = windowlist[handler].next;
    removeFromList(&windowlisthead, handler);
    addToListHead(&emptyhead, handler);
    if (newfocus != -1) focusWindow(newfocus);

    release(&wmlock);
}

#define MOUSE_SPEED_X 0.6f
#define MOUSE_SPEED_Y -0.6f;

void dispatchMessage(int handler, message *msg)
{
	enqueue(&windowlist[handler].wnd.buf, msg);
}

void wmHandleMessage(message *msg)
{
	acquire(&wmlock);

	message newmsg;
	int p;
	switch (msg->msg_type)
	{
	case M_MOUSE_MOVE:
		wm_last_mouse_pos = wm_mouse_pos;
		wm_mouse_pos.x += msg->params[0] * MOUSE_SPEED_X;
		wm_mouse_pos.y += msg->params[1] * MOUSE_SPEED_Y;
		if (wm_mouse_pos.x > SCREEN_WIDTH) wm_mouse_pos.x = SCREEN_WIDTH;
		if (wm_mouse_pos.y > SCREEN_HEIGHT) wm_mouse_pos.y = SCREEN_HEIGHT;
		if (wm_mouse_pos.x < 0) wm_mouse_pos.x = 0;
		if (wm_mouse_pos.y < 0) wm_mouse_pos.y = 0;
		//redraw mouse cursor
		clearMouse(screen, screen_buf1, wm_last_mouse_pos.x, wm_last_mouse_pos.y);
		drawMouse(screen, 0, wm_mouse_pos.x, wm_mouse_pos.y);
		break;
	case M_MOUSE_DOWN:
		//handle focus changes
		if (frontcnt == 0)
		{
		    for (p = windowlisthead; p != -1; p = windowlist[p].next)
		    {
			    if (isInRect(&windowlist[p].wnd.titlebar, wm_mouse_pos.x, wm_mouse_pos.y) ||
			        isInRect(&windowlist[p].wnd.contents, wm_mouse_pos.x, wm_mouse_pos.y))
			    {
			        if (p != focus) focusWindow(p);
			        break;
			    }
		    }
		}
		if (isInRect(&windowlist[focus].wnd.contents, wm_mouse_pos.x, wm_mouse_pos.y))
		{
		    newmsg = *msg;
		    //coordinate transformation (from screen to window)
		    newmsg.params[0] = wm_mouse_pos.x - windowlist[focus].wnd.contents.xmin;
		    newmsg.params[1] = wm_mouse_pos.y - windowlist[focus].wnd.contents.ymin;
		    newmsg.params[2] = msg->params[0];
		    dispatchMessage(focus, &newmsg);
		}
		else if (wm_mouse_pos.x + 30 > windowlist[focus].wnd.titlebar.xmax) //close
		{
		    newmsg.msg_type = WM_WINDOW_CLOSE;
		    dispatchMessage(focus, &newmsg);
		}
		break;
	case M_MOUSE_UP:
		//TODO
		newmsg = *msg;
	    newmsg.params[0] = wm_mouse_pos.x - windowlist[focus].wnd.contents.xmin;
	    newmsg.params[1] = wm_mouse_pos.y - windowlist[focus].wnd.contents.ymin;
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

inline int min(int x, int y) { return x < y ? x : y; }
inline int max(int x, int y) { return x > y ? x : y; }
inline int clamp(int x, int l, int r) { return min(r, max(l, x)); }

void wmUpdateWindow(int handler, int xmin, int ymin, int width, int height)
{
    acquire(&wmlock);
    
    window *wnd = &windowlist[handler].wnd;
    win_rect updrect;
    updrect.xmin = clamp(max(xmin, 0) + wnd->contents.xmin, 0, SCREEN_WIDTH);
    updrect.ymin = clamp(max(ymin, 0) + wnd->contents.ymin, 0, SCREEN_HEIGHT);
    updrect.xmax = clamp(min(xmin + width + wnd->contents.xmin, wnd->contents.xmax), 0, SCREEN_WIDTH);
    updrect.ymax = clamp(min(ymin + height + wnd->contents.ymin, wnd->contents.ymax), 0, SCREEN_HEIGHT);
    
    if (windowlist[handler].prev == -1)
    {
        int wndwidth = wnd->contents.xmax - wnd->contents.xmin;
        int wndheight = wnd->contents.ymax - wnd->contents.ymin;
        draw24ImagePart(screen_buf1, windowlist[handler].wnd.content_buf, 
                        updrect.xmin, updrect.ymin, wndwidth, wndheight,
                        max(xmin, 0), max(ymin, 0), updrect.xmax - updrect.xmin, updrect.ymax - updrect.ymin);
        clearRectByCoord(screen, screen_buf1, updrect.xmin, updrect.ymin, updrect.xmax, updrect.ymax);
        drawMouse(screen, 0, wm_mouse_pos.x, wm_mouse_pos.y);
        release(&wmlock);
        return;
    }
    
    static win_rect rects[MAX_WINDOW_CNT];
    int rectcnt = 0;
    int p, i, j;
    for (p = windowlisthead; p != handler; p = windowlist[p].next)
    {
        getWindowRect(p, &rects[rectcnt]);
        rects[rectcnt].xmin = clamp(rects[rectcnt].xmin, 0, SCREEN_WIDTH);
        rects[rectcnt].xmax = clamp(rects[rectcnt].xmax, 0, SCREEN_WIDTH);
        rects[rectcnt].ymin = clamp(rects[rectcnt].ymin, 0, SCREEN_HEIGHT);
        rects[rectcnt].ymax = clamp(rects[rectcnt].ymax, 0, SCREEN_HEIGHT);
        rectcnt++;
    }
    
    #define xsize 810
    #define ysize 610
    #define rsize 110
    //const int xsize = 800 + 10, ysize = 600 + 10;
    static int xcount[xsize], ycount[ysize];
    //static int rsize = MAX_WINDOW_CNT * 2 + 10;
    static int xrev[rsize], yrev[rsize];
    for (i = 0; i < xsize; ++i) xcount[i] = 0;
    for (i = 0; i < ysize; ++i) ycount[i] = 0;
    xcount[updrect.xmin] = 1; xcount[updrect.xmax] = 1;
    ycount[updrect.ymin] = 1; ycount[updrect.ymax] = 1;
    for (i = 0; i < rectcnt; ++i)
    {
        xcount[rects[i].xmin] = 1;
        xcount[rects[i].xmax] = 1;
        ycount[rects[i].ymin] = 1;
        ycount[rects[i].ymax] = 1;
    }
    xrev[0] = 0; yrev[0] = 0;
    for (i = 1; i < xsize; ++i)
    {
        if (xcount[i]) xrev[xcount[i - 1] + 1] = i;
        xcount[i] += xcount[i - 1];
    }
    for (i = 1; i < ysize; ++i)
    {
        if (ycount[i]) yrev[ycount[i - 1] + 1] = i;
        ycount[i] += ycount[i - 1];
    }

    
    static int subrects[rsize][rsize];
    int mxmin, mymin, mxmax, mymax;
    for (i = 0; i < rsize; ++i)
        for (j = 0; j < rsize; ++j) subrects[i][j] = 0;
    for (i = 0; i < rectcnt; ++i)
    {
        mxmin = xcount[rects[i].xmin];
        mxmax = xcount[rects[i].xmax];
        mymin = ycount[rects[i].ymin];
        mymax = ycount[rects[i].ymax];
        subrects[mxmin][mymin]++;
        subrects[mxmin][mymax]--;
        subrects[mxmax][mymin]--;
        subrects[mxmax][mymax]++;
    }
    for (i = 1; i < rsize; ++i)
        for (j = 1; j < rsize; ++j)
            subrects[i][j] += subrects[i - 1][j] + subrects[i][j - 1] - subrects[i - 1][j - 1];
    mxmin = xcount[updrect.xmin];
    mxmax = xcount[updrect.xmax];
    mymin = ycount[updrect.ymin];
    mymax = ycount[updrect.ymax];
    int wndwidth = wnd->contents.xmax - wnd->contents.xmin;
    int wndheight = wnd->contents.ymax - wnd->contents.ymin;
    for (i = mxmin; i < mxmax; ++i)
        for (j = mymin; j < mymax; ++j)
        {
            if (subrects[i][j] <= 0)
            {
                draw24ImagePart(screen_buf2, wnd->content_buf, xrev[i], yrev[j], wndwidth, wndheight,
                                xrev[i] - wnd->contents.xmin, yrev[j] - wnd->contents.ymin, xrev[i+1]-xrev[i], yrev[j+1]-yrev[j]);
                clearRectByCoord(screen_buf1, screen_buf2, xrev[i], yrev[j], xrev[i+1], yrev[j+1]);
                clearRectByCoord(screen, screen_buf1, xrev[i], yrev[j], xrev[i+1], yrev[j+1]);
            }
        }
    
    drawMouse(screen, 0, wm_mouse_pos.x, wm_mouse_pos.y);
    
    release(&wmlock);
}

//return number of message (0 if buf is empty, 1 if not)
int wmGetMessage(int handler, message *res)
{
	if (handler < 0 || handler >= MAX_WINDOW_CNT) {
		return 0;
	}
	// TODO: I'm not sure... Maybe 'proc' means porcess which is running; so this if can verify the handler.
	if (proc != windowlist[handler].proc) {
		return 0;
	}
	acquire(&wmlock);
	int ret = dequeue(&windowlist[handler].wnd.buf, res);
	release(&wmlock);
	if (ret) return 0;
	else return 1;
}

int sys_createwindow()
{
	int w, h;
	char *title;
	RGB *buf;
	int af;
	argint(0, &w);
	argint(1, &h);
	argstr(2, &title);
	argptr(3, (char**)(&buf), sizeof(RGB));
	argint(4, &af);
	return createWindow(w, h, title, buf, af);
}

int sys_destroywindow()
{
    int h;
    argint(0, &h);
    destroyWindow(h);
    return 0;
}

int sys_getmessage()
{
	int h;
	message *res;
	argint(0, &h);
	argptr(1, (char**)(&res), sizeof(message));
	return wmGetMessage(h, res);
}

int sys_updatewindow()
{
    int wd, x, y, w, h;
    argint(0, &wd);
    argint(1, &x);
    argint(2, &y);
    argint(3, &w);
    argint(4, &h);
    wmUpdateWindow(wd, x, y, w, h);
    return 0;
}

int sys_draw24Image() {
	int handler;
	int i;
	RGB *image;
	int width, height;
	int x, y;
	argint(0, &handler);
	argint(1, &i);
	image = (RGB *) i;
	argint(2, &x);
	argint(3, &y);
	argint(4, &width);
	argint(5, &height);
	if (handler < 0 || handler >= MAX_WINDOW_CNT) {
		return 1;
	}
	window *wnd = &windowlist[handler].wnd;
	draw24Image(screen_buf1, image, x + wnd->contents.xmin, y + wnd->contents.ymin,
				width, height, wnd->contents.xmax, wnd->contents.ymax);
	return 0;
}
