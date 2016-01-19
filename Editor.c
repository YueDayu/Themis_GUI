#include "types.h"
#include "color.h"
#include "msg.h"
#include "themis_ui.h"
#include "user.h"
#include "fcntl.h"

char filename[40];

void saveBackBtn(window *win, int index, message* msg) {
    printf(1, "saving\n");
    int file = open(filename, 1);
    int i;
    for (i = 0; i < win->widget_number; i++) {
        if (win->widgets[i].type == TEXT_AREA) {
            break;
        }
    }
    write(file, win->widgets[i].context.textArea->text, 512);
    close(file);
    printf(1, "saved\n");
}

int main(int argc, char *argv[]) {
    RGBA black;
    black.A = 255;
    black.R = 0;
    black.G = 0;
    black.B = 0;
    RGBA red;
    red.A = 255;
    red.R = 0;
    red.G = 255;
    red.B = 0;
    window editor;
    editor.width = 400;
    editor.height = 300;
    UI_createWindow(&editor, "Editor", 0);
    int file = -1;
    if (argc > 1) {
        file = open(argv[1], 0);
        strcpy(filename, argv[1]);
    } else {
        strcpy(filename, "new.txt");
    }
    int panel = addTextAreaWidget(&editor, black, "", 0, 0, 400, 275);
    addButtonWidget(&editor, black, red, "save", saveBackBtn, 350, 278, 40, 20);
    if (file >= 0) {
        read(file, editor.widgets[panel].context.textArea->text, 512);
        close(file);
    }

    drawAllWidget(&editor);
    mainLoop(&editor);
    return 0;
}
