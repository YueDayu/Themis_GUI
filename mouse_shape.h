#ifndef __ASSEMBLER__

#define MOUSE_MODE 2
#define MOUSE_HEIGHT 18
#define MOUSE_WIDTH 15

RGB mouse_color[2];

uchar mouse_pointer[MOUSE_MODE][MOUSE_HEIGHT][MOUSE_WIDTH] = {
    {
        {2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 0, 0, 0},
        {2, 1, 1, 1, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 2, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0},
        {2, 1, 2, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0},
        {2, 2, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0}
    }, {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 1, 1, 1, 2, 2, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 1, 0, 0},
        {0, 1, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 1, 0},
        {1, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1},
        {1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
        {0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
        {0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
        {0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
        {0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
        {0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
        {0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0},
        {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}
    }
};

#endif