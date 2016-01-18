#include "types.h"
#include "color.h"
#include "msg.h"
#include "user.h"
#include "fcntl.h"
#include "themis_ui.h"

void createWindow(window *win, const char* title) {
    if (win->width > MAX_WIDTH || win->height > MAX_HEIGHT) {
        win->handler = -1;
        return;
    }
    win->window_buf = malloc(win->width * win->height * 3);
    if (!win->window_buf) {
        win->handler = -1;
    }
    // TODO: system call -- create windows and return handler
}

void destroyWindow(window *win) {
    // TODO: free all widget space
    free(win->window_buf);
}

// system call
void updateWindow(window *win, int x, int y, int w, int h) {

}

void setWidgetSize(Widget *w, int x, int y, int w, int h) {
    w->size.x = x;
    w->size.y = y;
    w->size.height = h;
    w->size.width = w;
}

int addImageWidget(window *win, RGB *image, int x, int y, int w, int h) {
    if (win->widget_number >= MAX_WIDGET) {
        return;
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

int addLabelWidget(window *win, RGBA c, const char* text, int x, int y, int w, int h) {
    if (win->widget_number >= MAX_WIDGET) {
        return;
    }
    Label *l = malloc(sizeof(Label));
    l->color = c;
    strcpy(l->text, text);
    Widget *widget = &win->widgets[win->widget_number];
    widget->type = LABEL;
    widget->context = l;
    setWidgetSize(widget, x, y, w, h);
    win->widget_number++;
    return (win->widget_number - 1);
}

int addButtonWidget(window *win, RGBA c, RGBA bc, const char* text, Handler handler, int x, int y, int w, int h) {
    if (win->widget_number >= MAX_WIDGET) {
        return;
    }
    Button *b = malloc(sizeof(Button));
    b->bg_color = bc;
    b->color = c;
    strcpy(b->text, text);
    b->onLeftClick = handler;
    Widget *widget = &win->widgets[win->widget_number];
    w->context.button = b;
    w->type = BUTTON;
    setWidgetSize(widget, x, y, w, h);
    win->widget_number++;
    return (win->widget_number - 1);
}

