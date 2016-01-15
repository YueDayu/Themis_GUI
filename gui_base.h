#define GUI_BUF 0x9000

#ifndef __ASSEMBLER__

ushort SCREEN_WIDTH;
ushort SCREEN_HEIGHT;
int screen_size;

// 24 bit RGB. used in GUI Utility
typedef struct RGB {
    unsigned char B;
    unsigned char G;
    unsigned char R;
} RGB;

// 32 bit RGBA. used above GUI Utility
typedef struct RGBA {
    unsigned char A;
    unsigned char B;
    unsigned char G;
    unsigned char R;
} RGBA;

RGB *screen = 0;
RGB *screen_buf1 = 0;
RGB *screen_buf2 = 0;

#endif
