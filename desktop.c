#include "types.h"
#include "color.h"
#include "msg.h"
#include "user.h"
#include "fcntl.h"

RGB image[800 * 600];

int main() {
    int h, w;
    int res = read24BitmapFile("desktop.bmp", image, &h, &w);
    printf(1, "res: %d\n", res);

    int hwnd = createwindow(0, 0, "");
    struct message msg;

    draw24Image(hwnd, image, 0, 0, w, h);
    int hwnd2 = createwindow(600, 200, "hello window 2");
    int hwnd1 = createwindow(600, 200, "hello window");

    while(1)
    {
        if (getmessage(hwnd1, &msg))
        {
            if (msg.msg_type == WM_WINDOW_CLOSE) {
                destroywindow(hwnd1);
                hwnd1 = -1;
            }
            if (msg.msg_type == M_KEY_DOWN)
            {
                if (msg.params[0] == 'a') break;
            }
        }
        if (getmessage(hwnd2, &msg))
        {
            if (msg.msg_type == WM_WINDOW_CLOSE) {
                destroywindow(hwnd2);
                hwnd2 = -1;
            }
            if (msg.msg_type == M_KEY_DOWN)
            {
                if (msg.params[0] == 'a') break;
            }
        }
    }

//    destroywindow(hwnd);
    while(1);;

}