#include "types.h"
#include "color.h"
#include "msg.h"
#include "user.h"
#include "fcntl.h"
#include "themis_ui.h"

void createWindow(window *win, const char* title, int alwaysfront) {
    if (win->width > MAX_WIDTH || win->height > MAX_HEIGHT) {
        win->handler = -1;
        return;
    }
    win->window_buf = malloc(win->width * win->height * 3);
    if (!win->window_buf) {
        win->handler = -1;
    }
    createwindow(win->width, win->height, title, win->window_buf, alwaysfront);
}

void freeWidget(Widget *widget) {
    switch (widget->type){
        case IMAGE:
            free(widget->context.image);
            break;
        case LABEL:
            free(widget->context.label);
            break;
        case BUTTON:
            free(widget->context.button);
            break;
        case INPUT:
            free(widget->context.input);
            break;
        case TEXT_AREA:
            free(widget->context.textArea);
            break;
        default:
            free(widget->context.fileList);
            break;
    }
}

void destroyWindow(window *win) {
    free(win->window_buf);
    int i;
    for (i = 0; i < win->widget_number; i++) {
        if (win->widgets[i].type == FILE_LIST) {
            IconView *p;
            IconView *temp;
            int j;
            for (j = 0; j < FILE_TYPE_NUM; j++) {
                free(win->widgets[i].context.fileList->image[i]);
            }
            for (p = win->widgets[i].context.fileList->file_list; p; p = temp) {
                temp = p->next;
                free(p);
            }
        }
        freeWidget(&win->widgets[i]);
    }
    destroywindow(win->handler);
}

// system call
void updateWindow(window *win, int x, int y, int w, int h) {
    updatewindow(win->handler, x, y, w, h);
}

void setWidgetSize(Widget *widget, int x, int y, int w, int h) {
    widget->size.x = x;
    widget->size.y = y;
    widget->size.height = h;
    widget->size.width = w;
}

int addImageWidget(window *win, RGB *image, int x, int y, int w, int h) {
    if (win->widget_number >= MAX_WIDGET) {
        return -1;
    }
    Image *i = malloc(sizeof(Image));
    i->image = image;
    Widget *widget = &win->widgets[win->widget_number];
    widget->type = IMAGE;
    widget->context.image = i;
    setWidgetSize(widget, x, y, w, h);
    win->widget_number++;
    return (win->widget_number - 1);
}

int addLabelWidget(window *win, RGBA c, char* text, int x, int y, int w, int h) {
    if (win->widget_number >= MAX_WIDGET) {
        return -1;
    }
    Label *l = malloc(sizeof(Label));
    l->color = c;
    strcpy(l->text, text);
    Widget *widget = &win->widgets[win->widget_number];
    widget->type = LABEL;
    widget->context.label = l;
    setWidgetSize(widget, x, y, w, h);
    win->widget_number++;
    return (win->widget_number - 1);
}

int addButtonWidget(window *win, RGBA c, RGBA bc, char* text, Handler handler, int x, int y, int w, int h) {
    if (win->widget_number >= MAX_WIDGET) {
        return -1;
    }
    Button *b = malloc(sizeof(Button));
    b->bg_color = bc;
    b->color = c;
    strcpy(b->text, text);
    b->onLeftClick = handler;
    Widget *widget = &win->widgets[win->widget_number];
    widget->context.button = b;
    widget->type = BUTTON;
    setWidgetSize(widget, x, y, w, h);
    win->widget_number++;
    return (win->widget_number - 1);
}

int addInputWidget(window *win, RGBA c, char *text, int x, int y, int w, int h) {
    if (win->widget_number >= MAX_WIDGET) {
        return -1;
    }
    Input *i = malloc(sizeof(Input));
    i->color = c;
    i->current_pos = 0;
    strcpy(i->text, text);
    // TODO: add default handler
    Widget *widget = &win->widgets[win->widget_number];
    widget->context.input = i;
    widget->type = INPUT;
    setWidgetSize(widget, x, y, w, h);
    win->widget_number++;
    return (win->widget_number - 1);
}

int addTextAreaWidget(window *win, RGBA c, char *text, int x, int y, int w, int h) {
    if (win->widget_number >= MAX_WIDGET) {
        return -1;
    }
    TextArea *t = malloc(sizeof(TextArea));
    t->color = c;
    t->current_pos = 0;
    t->current_line = 0;
    t->begin_line = 0;
    strcpy(t->text, text);
    // TODO: add default handler
    Widget *widget = &win->widgets[win->widget_number];
    widget->context.textArea = t;
    widget->type = TEXT_AREA;
    setWidgetSize(widget, x, y, w, h);
    win->widget_number++;
    return (win->widget_number - 1);
}

int addFileListWidget(window *win, char *path, char direction, char scrollable, int x, int y, int w, int h) {
    if (win->widget_number >= MAX_WIDGET) {
        return -1;
    }
    FileList *f = malloc(sizeof(FileList));
    f->direction = direction;
    f->scrollable = scrollable;
    strcpy(f->path, path);
    int i;
    int res;
    int temp_w, temp_h;
    for (i = 0; i < FILE_TYPE_NUM; i++) { // read file
        (f->image[i]) = (RGBA *)malloc(ICON_IMG_SIZE * ICON_IMG_SIZE * 3);
        res = readBitmapFile(file_image_path[i], f->image[i], &temp_h, &temp_w);
        if (res < 0) {
            printf(1, "read file image error \n");
            for (i-- ; i >= 0; i--) {
                free(f->image[i]);
            }
            return -1;
        }
    }
    // TODO: read file list and add to queue
    // TODO: set default handler
    Widget *widget = &win->widgets[win->widget_number];
    widget->context.fileList = f;
    widget->type = FILE_LIST;
    setWidgetSize(widget, x, y, w, h);
    win->widget_number++;
    return (win->widget_number - 1);
}
