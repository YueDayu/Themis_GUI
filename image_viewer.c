#include "types.h"
#include "color.h"
#include "msg.h"
#include "themis_ui.h"
#include "user.h"
#include "fcntl.h"

RGB image[64 * 64];

int main(int argc, char *argv[]) {
//    int h, w;
    window viewer;
    viewer.width = 400;
    viewer.height = 300;
    printf(1, "Hello! %s\n", argv[1]);
    UI_createWindow(&viewer, "Image Viewer", 0);
//    if (argc > 1) {
//        int res = read24BitmapFile(argv[1], image, &h, &w);
//        if (res == 0) {
//            addImageWidget(&viewer, image, (viewer.width - w) / 2, (viewer.height - h) / 2, w, h);
//        }
//    }

//    drawAllWidget(&viewer);
//    mainLoop(&viewer);
    while (1);;
    return 0;
}
