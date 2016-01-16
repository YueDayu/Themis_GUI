#ifndef __ASSEMBLER__

// 24 bit RGB. used in GUI Utility
typedef struct RGB {
    unsigned char B;
    unsigned char G;
    unsigned char R;
} RGB;

// 32 bit RGBA. used above GUI Utility
typedef struct RGBA {
    unsigned char B;
    unsigned char G;
    unsigned char R;
    unsigned char A;
} RGBA;

#endif
