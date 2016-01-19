#include "types.h"
#include "color.h"
#include "msg.h"
#include "themis_ui.h"
#include "user.h"
#include "fcntl.h"

int main(int argc, char *argv[]) {
    window viewer;
    viewer.width = 400;
    viewer.height = 300;
    UI_createWindow(&viewer, "File Explorer", 0);
    
    if (argc > 1) {
    	addFileListWidget(&viewer, argv[1], 1, 0, 0, 400, 300);
    }
    else {
    	addFileListWidget(&viewer, "/", 1, 0, 0, 400, 300);
    }

    drawAllWidget(&viewer);
    mainLoop(&viewer);
    return 0;
}

