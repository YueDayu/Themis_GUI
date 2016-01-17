#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "spinlock.h"
#include "gui_base.h"
#include "character.h"
#include "bitmap.h"
#include "mouse_shape.h"

struct spinlock screen_lock;
struct spinlock buf1_lock;
struct spinlock buf2_lock;

RGB *screen = 0;
RGB *screen_buf1 = 0;
RGB *screen_buf2 = 0;

void initGUI() {
    uint GraphicMem = KERNBASE + 0x1028;
    uint baseAdd = *((uint *) GraphicMem);
    screen = (RGB *) baseAdd;
    SCREEN_WIDTH = *((ushort *) (KERNBASE + 0x1012));
    SCREEN_HEIGHT = *((ushort *) (KERNBASE + 0x1014));
    screen_size = (SCREEN_WIDTH * SCREEN_HEIGHT) * 3;
    screen_buf1 = (RGB *) (baseAdd + screen_size);
    screen_buf2 = (RGB *) (baseAdd + screen_size * 2);
    initlock(&screen_lock, "screen");
    initlock(&buf1_lock, "buffer_1");
    initlock(&buf2_lock, "buffer_2");

    mouse_color[0].G = 0;
    mouse_color[0].B = 0;
    mouse_color[0].R = 0;
    mouse_color[1].G = 200;
    mouse_color[1].B = 200;
    mouse_color[1].R = 200;

    uint x, y;
    uchar *b = (uchar *) screen;
    for (x = 0; x < SCREEN_WIDTH; x++)
        for (y = 0; y < SCREEN_HEIGHT; y++) {
            b[0] = 0xFF;
            b[1] = 0x00;
            b[2] = 0x00;
            b += 3;
        }

    cprintf("@Screen Width:   %d\n", SCREEN_WIDTH);
    cprintf("@Screen Height:  %d\n", SCREEN_HEIGHT);
    cprintf("@Bits per pixel: %d\n", *((uchar *) (KERNBASE + 0x1019)));
    cprintf("@Video card drivers initialized successfully.\n");

    wmInit();
}

void acquireGUILock(RGB *buf) {
    switch (buf) {
        case screen:
            acquire(&screen_lock);
            break;
        case screen_buf1:
            acquire(&buf1_lock);
            break;
        case screen_buf2:
            acquire(&buf2_lock);
        default:
            break;
    }
}

void releaseGUILock(RGB *buf) {
    switch (buf) {
        case screen:
            release(&screen_lock);
            break;
        case screen_buf1:
            release(&buf1_lock);
            break;
        case screen_buf2:
            release(&buf2_lock);
            break;
        default:
            break;
    }
}

void drawPoint(RGB *color, RGB origin) {
    color->R = origin.R;
    color->G = origin.G;
    color->B = origin.B;
}

void drawPointAlpha(RGB *color, RGBA origin) {
    float alpha;
    if (origin.A == 255) {
        color->R = origin.R;
        color->G = origin.G;
        color->B = origin.B;
        return;
    }
    if (origin.A == 0) {
        return;
    }
    alpha = (float) origin.A / 255;
    color->R = color->R * (1 - alpha) + origin.R * alpha;
    color->G = color->G * (1 - alpha) + origin.G * alpha;
    color->B = color->B * (1 - alpha) + origin.B * alpha;
}

int drawCharacter(RGB *buf, int x, int y, char ch, RGBA color) {
    int i, j;
    RGB *t;
    int ord = ch - 0x20;
    if (ord < 0 || ord >= (CHARACTER_NUMBER - 1)) {
        return -1;
    }
    acquireGUILock(buf);
    for (i = 0; i < CHARACTER_HEIGHT; i++) {
        if (y + i > SCREEN_HEIGHT) {
            break;
        }
        if (y + i < 0) {
            continue;
        }
        for (j = 0; j < CHARACTER_WIDTH; j++) {
            if (character[ord][i][j] == 1) {
                if (x + j > SCREEN_WIDTH) {
                    break;
                }
                if (x + j < 0) {
                    continue;
                }
                t = buf + (y + i) * SCREEN_WIDTH + x + j;
                drawPointAlpha(t, color);
            }
        }
    }
    releaseGUILock(buf);
    return CHARACTER_WIDTH;
}

void drawString(RGB *buf, int x, int y, char *str, RGBA color) {
    int offset_x = 0;

    while (*str != '\0') {
        offset_x += drawCharacter(buf, x + offset_x, y, *str, color);
        str++;
    }
}

void drawImage(RGB *buf, RGBA *img, int x, int y, int width, int height) {
    int i, j;
    RGB *t;
    RGBA *o;
    acquireGUILock(buf);
    for (i = 0; i < height; i++) {
        if (y + i >= SCREEN_HEIGHT) {
            break;
        }
        if (y + i < 0) {
            continue;
        }
        for (j = 0; j < width; j++) {
            if (x + j >= SCREEN_WIDTH) {
                break;
            }
            if (x + j < 0) {
                continue;
            }
            t = buf + (y + i) * SCREEN_WIDTH + x + j;
            o = img + (height - i) * width + j;
            drawPointAlpha(t, *o);
        }
    }
    releaseGUILock(buf);
}

void draw24Image(RGB *buf, RGB *img, int x, int y, int width, int height) {
    int i;
    RGB *t;
    RGB *o;
    int max_line = (SCREEN_WIDTH - x) < width ? (SCREEN_WIDTH - x) : width;
    acquireGUILock(buf);
    for (i = 0; i < height; i++) {
        if (y + i >= SCREEN_HEIGHT) {
            break;
        }
        if (y + i < 0) {
            continue;
        }
        t = buf + (y + i) * SCREEN_WIDTH + x;
        o = img + (height - i) * width;
        memmove(t, o, max_line * 3);
    }
    releaseGUILock(buf);
}

void drawRect(RGB *buf, int x, int y, int width, int height, RGBA fill)
{
	int i, j;
	RGB *t;
    acquireGUILock(buf);
	for (i = 0; i < height; i++)
	{
		if (y + i < 0) continue;
		if (y + i >= SCREEN_HEIGHT) break;
		for (j = 0; j < width; j++)
		{
			if (x + j < 0) continue;
			if (x + j >= SCREEN_WIDTH) break;
			t = buf + (y + i) * SCREEN_WIDTH + x + j;
			drawPointAlpha(t, fill);
		}
	}
    releaseGUILock(buf);
}

void drawRectByCoord(RGB *buf, int xmin, int ymin, int xmax, int ymax, RGBA fill)
{
	drawRect(buf, xmin, ymin, xmax - xmin, ymax - ymin, fill);
}

void clearRect(RGB *buf, RGB *temp_buf, int x, int y, int width, int height)
{
	int i, j, offset;
	RGB *t, *o;
    acquireGUILock(buf);
	for (i = 0; i < height; i++)
	{
		if (y + i < 0) continue;
		if (y + i >= SCREEN_HEIGHT) break;
		for (j = 0; j < width; j++)
		{
			if (x + j < 0) continue;
			if (x + j >= SCREEN_WIDTH) break;
			offset = (y + i) * SCREEN_WIDTH + x + j;
			t = buf + offset;
			o = temp_buf + offset;
			drawPoint(t, *o);
		}
	}
    releaseGUILock(buf);
}


void clearRectByCoord(RGB *buf, RGB *temp_buf, int xmin, int ymin, int xmax, int ymax)
{
	clearRect(buf, temp_buf, xmin, ymin, xmax - xmin, ymax - ymin);
}


void clearMouse(RGB*, RGB*,int, int);

void drawMouse(RGB *buf, int mode, int x, int y) {
    int i, j;
    RGB *t;
    acquireGUILock(buf);
    for (i = 0; i < MOUSE_HEIGHT; i++) {
        if (y + i >= SCREEN_HEIGHT) {
            break;
        }
        if (y + i < 0) {
            continue;
        }
        for (j = 0; j < MOUSE_WIDTH; j++) {
            if (x + j >= SCREEN_WIDTH) {
                break;
            }
            if (x + j < 0) {
                continue;
            }
            uchar temp = mouse_pointer[mode][i][j];
            if (temp) {
                t = buf + (y + i) * SCREEN_WIDTH + x + j;
                drawPoint(t, mouse_color[temp - 1]);
            }
        }
    }
    releaseGUILock(buf);
}

void clearMouse(RGB *buf, RGB *temp_buf, int x, int y) {
    RGB *t;
    RGB *o;
    int i;
    acquireGUILock(buf);
    for (i = 0; i < MOUSE_HEIGHT; i++) {
        if (y + i >= SCREEN_HEIGHT) {
            break;
        }
        if (y + i < 0) {
             continue;
        }
        t = buf + (y + i) * SCREEN_WIDTH + x;
        o = temp_buf + (y + i) * SCREEN_WIDTH + x;
        memmove(t, o, MOUSE_WIDTH * 3);
    }
    releaseGUILock(buf);
}

void sys_hello() {
    RGB *image;
    int i;
    int h, w;
    argint(0, &i);
    argint(1, &h);
    argint(2, &w);
    cprintf("size: %d * %d", h, w);
    image = (RGB *) i;
    RGBA color;
    color.A = 200;
    color.G = 255;
    draw24Image(screen_buf2, image, 0, 0, w, h);
    draw24Image(screen_buf1, image, 0, 0, w, h);
    draw24Image(screen, image, 0, 0, w, h);
    drawString(screen, 100, 200, "Hello World!", color);
}

