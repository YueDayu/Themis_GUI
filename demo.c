#include "types.h"
#include "color.h"
#include "msg.h"
#include "themis_ui.h"
#include "user.h"
#include "fcntl.h"

char filename[40];

int main(int argc, char *argv[]) {
    window demo;
    demo.width = 400;
    demo.height = 300;
    UI_createWindow(&demo, "Demo", 0);
    message msg;

    int step = 0;
    while (1) {
        int i, j;
        for (i = 0; i < 300; ++i)
            for (j = 0; j < 400; ++j)
            {
                RGB * t = demo.window_buf + (i * 400) + j;
                t->R = (i * j + step) % 255;
                t->B = (i + step) % 200 + 25;
                t->G = (i * j + step) % 100 + 100;
            }

        step++;
        drawAllWidget(&demo);
        if (getmessage(demo.handler, &msg) != 0) {
            if (msg.msg_type == WM_WINDOW_CLOSE) {
                UI_destroyWindow(&demo);
                break;
            }
        }
    }
//    mainLoop(&editor);
    return 0;
}
