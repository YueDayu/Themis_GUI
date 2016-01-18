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

