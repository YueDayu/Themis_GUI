#include "types.h"
#include "color.h"
#include "msg.h"
#include "themis_ui.h"
#include "user.h"
#include "fcntl.h"

RGB image[800 * 600];

int main() {
    int h, w;
    window desktop;
    desktop.width = MAX_WIDTH;
    desktop.height = MAX_HEIGHT;
    UI_createWindow(&desktop, "", 0);
    int res = read24BitmapFile("desktop.bmp", image, &h, &w);
    printf(1, "res: %d\n", res);
//    RGBA color;
//    color.A = 155;
//    color.R = 155;
//    color.G = 155;
//    color.B = 255;

    addImageWidget(&desktop, image, 0, 0, MAX_WIDTH, MAX_HEIGHT);
//    addLabelWidget(&desktop, color, "Hello World", 100, 100, 100, 20);
    addFileListWidget(&desktop, "/", 0, 0, 0, MAX_WIDTH, MAX_HEIGHT - 25);

    drawAllWidget(&desktop);

//    int hwnd = createwindow(0, 0, "", 0, 0);
//    updatewindow(hwnd, 0, 0, w, h);
//    struct message msg;

//    int hwnd2 = createwindow(600, 200, "hello window 2", 0, 0);
//    int hwnd1 = createwindow(600, 200, "hello window", 0, 0);
//
//    while(1)
//    {
//        if (getmessage(hwnd1, &msg))
//        {
//            if (msg.msg_type == WM_WINDOW_CLOSE) {
//                destroywindow(hwnd1);
//                hwnd1 = -1;
//            }
//            if (msg.msg_type == M_KEY_DOWN)
//            {
//                if (msg.params[0] == 'a') break;
//            }
//        }
//        if (getmessage(hwnd2, &msg))
//        {
//            if (msg.msg_type == WM_WINDOW_CLOSE) {
//                destroywindow(hwnd2);
//                hwnd2 = -1;
//            }
//            if (msg.msg_type == M_KEY_DOWN)
//            {
//                if (msg.params[0] == 'a') break;
//            }
//        }
//    }

//    destroywindow(hwnd);
    while(1);;

}
