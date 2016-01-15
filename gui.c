#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "spinlock.h"
#include "gui_base.h"
#include "color.h"
#include "character.h"

GUI_MODE_INFO GUI_INFO;
RGB *screen, *screen_buf1, *screen_buf2;
struct spinlock gui_lock;

void drawPointAlpha(RGB* color, RGBA origin);

void initGUI() {
    uint GraphicMem=KERNBASE+0x1028;
    uint baseAdd=*((uint*)GraphicMem);
    uchar *base=(uchar*)baseAdd;
    cprintf("Bits per pixel: %x\n",*((uchar*)(KERNBASE+0x1019)));
    screen = (RGB*)base;
    RGB *b = screen;
    uint x,y;
    for (x=0;x<SCREEN_WIDTH;x++)
        for (y=0;y<SCREEN_HEIGHT;y++)
        {
            b->B=0xFF;
            b->G=0;
            b->R=0;
            b++;
        }

//    GUI_INFO = *((GUI_MODE_INFO *)(GUI_BUF << 4));
//    screen = (RGB *)GUI_INFO.PhysBasePtr;
//    screen_buf1 = (RGB *)(GUI_INFO.PhysBasePtr + 0x15f900);
//    screen_buf2 = (RGB *)(GUI_INFO.PhysBasePtr + 0x2bf200);
//    initlock(&gui_lock, "gui");
}

void sayHello() {
    RGBA color;
    color.R = 255;
    color.A = 255;
    color.G = 0;
    color.B = 0;
    drawString(screen, 100, 100, "Hello World", color);
}

void drawPoint(RGB* color, RGB origin) {
    color->R = origin.R;
    color->G = origin.G;
    color->B = origin.B;
}

void drawPointAlpha(RGB* color, RGBA origin) {
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
    alpha = (float)origin.A / 255;
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
    for (i = 0; i < CHARACTER_HEIGHT; i++) {
        for (j = 0; j < CHARACTER_WIDTH; j++) {
            if (character[ord][i][j] == 1) {
                t = buf + (y + i) * SCREEN_WIDTH + x + j;
                drawPointAlpha(t, color);
            }
        }
    }
    return CHARACTER_WIDTH;
}

void drawString(RGB *buf, int x, int y, char *str, RGBA color) {
    int offset_x = 0;

    while (*str != '\0') {
        offset_x += drawCharacter(buf, x + offset_x, y, *str, color);
        str++;
    }
}