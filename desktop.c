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

	window testwindow;
    testwindow.width = 300;
    testwindow.height = 200;
    UI_createWindow(&testwindow, "test window", 0);

	window testwindow2;
    testwindow2.width = 600;
    testwindow2.height = 100;
    UI_createWindow(&testwindow2, "test window 2", 0);


    drawAllWidget(&desktop);


    while(1);;

}
