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

    addImageWidget(&desktop, image, 0, 0, MAX_WIDTH, MAX_HEIGHT);
    addFileListWidget(&desktop, "/", 0, 0, 0, MAX_WIDTH, MAX_HEIGHT - 25);

    drawAllWidget(&desktop);

    mainLoop(&desktop);


    while(1);;

}
