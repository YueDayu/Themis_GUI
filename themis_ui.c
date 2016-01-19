#include "types.h"
#include "stat.h"
#include "color.h"
#include "msg.h"
#include "user.h"
#include "fcntl.h"
#include "themis_ui.h"
#include "character.h"
#include "fs.h"

void drawImageWidget(window *win, int index);
void drawLabelWidget(window *win, int index);
void drawButtonWidget(window *win, int index);
void drawInputWidget(window *win, int index);
void drawTextAreaWidget(window *win, int index);
void drawFileListWidget(window *win, int index);

void fileListDoubleClickHandler(window *win, int index, message *msg);

char file_image_path[FILE_TYPE_NUM][MAX_SHORT_STRLEN] = {"explorer.bmp",
                                                         "txt.bmp",
                                                         "pic.bmp",
                                                         "exec.bmp",
                                                         "folder.bmp",
                                                         "unknow.bmp"};

void UI_createWindow(window *win, const char* title, int alwaysfront) {
    if (win->width > MAX_WIDTH || win->height > MAX_HEIGHT) {
        win->handler = -1;
        return;
    }
    win->window_buf = malloc(win->width * win->height * 3);
    if (!win->window_buf) {
        win->handler = -1;
        return;
    }
    memset(win->window_buf, 255, win->height * win->width * 3);
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

void UI_destroyWindow(window *win) {
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
void updatePartWindow(window *win, int x, int y, int w, int h) {
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
    widget->paint = drawImageWidget;
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
    widget->paint = drawLabelWidget;
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
    widget->paint = drawButtonWidget;
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
    widget->paint = drawInputWidget;
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
    widget->paint = drawTextAreaWidget;
    widget->context.textArea = t;
    widget->type = TEXT_AREA;
    setWidgetSize(widget, x, y, w, h);
    win->widget_number++;
    return (win->widget_number - 1);
}

char* UI_fmtname(char *path)
{
    char *p;
    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;
    return p;
}

void UI_ls(char *path, Widget *widget)
{
    char buf[512], *p, *tmpName;
    int fd;
    struct dirent de;
    struct stat st;
    IconView *node = 0;
    int j;
    if ((fd = open(path, 0)) < 0) {
        return;
    }
    if (fstat(fd, &st) < 0) {
        return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0) {
            continue;
        }
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if (stat(buf, &st) < 0) {
            continue;
        }
        tmpName = UI_fmtname(buf);
        if (strcmp(tmpName, ".") == 0 || strcmp(tmpName, "..") == 0 || strcmp(tmpName, "desktop") == 0
            || st.type == T_DEV || strcmp(tmpName, "desktop.bmp") == 0) {
            continue;
        }
        IconView *iconView = malloc(sizeof(IconView));
        iconView->next = 0;
        for (j = 0; j < strlen(tmpName); j++) {
            if (tmpName[j] == '.')
                break;
        }
        switch (st.type) {
            case T_DIR:
                iconView->image = widget->context.fileList->image[FOLDER_FILE];
                break;
            case T_FILE:
                if (strcmp(tmpName, "README") == 0)
                    iconView->image = widget->context.fileList->image[UNKNOWN_FILE];
                else if (tmpName[j+1] == 'b' && tmpName[j+2] == 'm' && tmpName[j+3] == 'p')
                    iconView->image = widget->context.fileList->image[BMP_FILE];
                else if (tmpName[j+1] == 't' && tmpName[j+2] == 'x' && tmpName[j+3] == 't')
                    iconView->image = widget->context.fileList->image[TEXT_FILE];
                else
                    iconView->image = widget->context.fileList->image[EXEC_FILE];
                break;
        }
        strcpy(iconView->text, tmpName);
        if (!widget->context.fileList->file_list) {
            widget->context.fileList->file_list = iconView;
        }
        if (!node) {
            node = iconView;
        } else {
            node->next = iconView;
            node = iconView;
        }
        widget->context.fileList->file_num += 1;
    }
}

int addFileListWidget(window *win, char *path, int direction, int x, int y, int w, int h) {
    if (win->widget_number >= MAX_WIDGET) {
        return -1;
    }
    FileList *f = malloc(sizeof(FileList));
    f->direction = direction;
    f->file_num = 0;
    strcpy(f->path, path);
    int i;
    int res;
    int temp_w, temp_h;
    for (i = 0; i < FILE_TYPE_NUM; i++) { // read file
        (f->image[i]) = (RGBA *)malloc(ICON_IMG_SIZE * ICON_IMG_SIZE * 4);
        res = readBitmapFile(file_image_path[i], f->image[i], &temp_h, &temp_w);
        if (res < 0) {
            printf(1, "read file image error \n");
            for (i-- ; i >= 0; i--) {
                free(f->image[i]);
            }
            return -1;
        }
    }
    // TODO: set default handler
    f->onDoubleClick = fileListDoubleClickHandler;
    Widget *widget = &win->widgets[win->widget_number];
    widget->paint = drawFileListWidget;
    widget->context.fileList = f;
    widget->type = FILE_LIST;
    setWidgetSize(widget, x, y, w, h);
    UI_ls(path, widget);
    win->widget_number++;
    return (win->widget_number - 1);
}

// paint function

void drawPointAlpha(RGB *color, RGBA origin) {
    float alpha;
    if (origin.A == 255) {
        color->R = origin.R;
        color->G = origin.G;
        color->B = origin.B;
        return;
    }
    if (origin.A == 0) {
        return;
    }
    alpha = (float) origin.A / 255;
    color->R = color->R * (1 - alpha) + origin.R * alpha;
    color->G = color->G * (1 - alpha) + origin.G * alpha;
    color->B = color->B * (1 - alpha) + origin.B * alpha;
}

int drawCharacter(window *win, int x, int y, char ch, RGBA color) {
    int i, j;
    RGB *t;
    int ord = ch - 0x20;
    if (ord < 0 || ord >= (CHARACTER_NUMBER - 1)) {
        return -1;
    }
    for (i = 0; i < CHARACTER_HEIGHT; i++) {
        if (y + i > win->height) {
            break;
        }
        if (y + i < 0) {
            continue;
        }
        for (j = 0; j < CHARACTER_WIDTH; j++) {
            if (character[ord][i][j] == 1) {
                if (x + j > win->width) {
                    break;
                }
                if (x + j < 0) {
                    continue;
                }
                t = win->window_buf + (y + i) * win->width + x + j;
                drawPointAlpha(t, color);
            }
        }
    }
    return CHARACTER_WIDTH;
}

void drawString(window *win, int x, int y, char *str, RGBA color, int width) {
    int offset_x = 0;

    while (*str != '\0') {
        if (x + offset_x >= win->width || offset_x >= width) { // if too long
            break;
        }
        offset_x += drawCharacter(win, x + offset_x, y, *str, color);
        str++;
    }
}

void drawImage(window *win, RGBA *img, int x, int y, int width, int height) {
    int i, j;
    RGB *t;
    RGBA *o;
    for (i = 0; i < height; i++) {
        if (y + i >= win->height) {
            break;
        }
        if (y + i < 0) {
            continue;
        }
        for (j = 0; j < width; j++) {
            if (x + j >= win->width) {
                break;
            }
            if (x + j < 0) {
                continue;
            }
            t = win->window_buf + (y + i) * win->width + x + j;
            o = img + (height - i - 1) * width + j;
            drawPointAlpha(t, *o);
        }
    }
}

void draw24Image(window *win, RGB *img, int x, int y, int width, int height) {
    int i;
    RGB *t;
    RGB *o;
    int max_line = (win->width - x) < width ? (win->width - x) : width;
    for (i = 0; i < height; i++) {
        if (y + i >= win->height) {
            break;
        }
        if (y + i < 0) {
            continue;
        }
        t = win->window_buf + (y + i) * win->width + x;
        o = img + (height - i - 1) * width;
        memmove(t, o, max_line * 3);
    }
}

void drawRect(window *win, RGB color, int x, int y, int width, int height) {
    if (x >= win->width || x + width < 0 || y >= win->height || y + height < 0
        || x < 0 || y < 0 || width < 0 || height < 0) {
        return;
    }
    int i;
    int max_line = (win->width - x) < width ? (win->width - x) : width;
    RGB *t = win->window_buf + y * win->width + x;
    for (i = 0; i < max_line; i++) {
        *(t + i) = color;
    }
    if (y + height < win->height) {
        RGB *o = win->window_buf + (y + height) * win->width + x;
        memmove(o, t, max_line * 3);
    }
    int max_height = (win->height - y) < height ? (win->height - x) : height;
    for (i = 0; i < max_height; i++) {
        *(t + i * win->width) = color;
    }
    if (x + width < win->width) {
        t = win->window_buf + y * win->width + x + win->width;
        for (i = 0; i < max_height; i++) {
            *(t + i * win->width) = color;
        }
    }
}

void drawFillRect(window *win, RGBA color, int x, int y, int width, int height) {
    if (x >= win->width || x + width < 0 || y >= win->height || y + height < 0
        || x < 0 || y < 0 || width < 0 || height < 0) {
        return;
    }
    int i, j;
    RGB *t;
    for (i = 0; i < height; i++) {
        if (y + i >= win->height) {
            break;
        }
        if (y + i < 0) {
            continue;
        }
        for (j = 0; j < width; j++) {
            if (j + x >= win->width) {
                break;
            }
            if (j + x < 0) {
                continue;
            }
            t = win->window_buf + (y + i) * win->width + (x + j);
            drawPointAlpha(t, color);
        }
    }
}

void draw24FillRect(window *win, RGB color, int x, int y, int width, int height) {
    if (x >= win->width || x + width < 0 || y >= win->height || y + height < 0
        || x < 0 || y < 0 || width < 0 || height < 0) {
        return;
    }
    int i, j;
    int max_line = (win->width - x) < width ? (win->width - x) : width;
    RGB *t, *o;
    t = win->window_buf + y * win->width + x;
    for (i = 0; i < height; i++) {
        if (y + i >= win->height) {
            break;
        }
        if (y + i < 0) {
            continue;
        }
        if (i == 0) {
            for (j = 0; j < max_line; j++) {
                *(t + j) = color;
            }
        } else {
            o = win->window_buf + (y + i) * win->width + x;
            memmove(o, t, max_line * 3);
        }
    }
}

void drawImageWidget(window *win, int index) {
    Widget *w = &(win->widgets[index]);
    draw24Image(win, w->context.image->image, w->size.x, w->size.y, w->size.width, w->size.height);
}

void drawLabelWidget(window *win, int index) {
    Widget *w = &(win->widgets[index]);
    drawString(win, w->size.x, w->size.y, w->context.label->text, w->context.label->color, w->size.width);
}

void drawButtonWidget(window *win, int index) {
    Widget *w = &(win->widgets[index]);
    RGB black;
    black.R = 0;
    black.G = 0;
    black.B = 0;
    drawFillRect(win, w->context.button->bg_color, w->size.x, w->size.y, w->size.width, w->size.height);
    drawRect(win, black, w->size.x, w->size.y, w->size.width, w->size.height);
    drawString(win, w->size.x, w->size.y, w->context.button->text,  w->context.button->color, w->size.width);
}

void drawInputWidget(window *win, int index) {
    Widget *w = &(win->widgets[index]);
    RGB black, white;
    black.R = 0; black.G = 0; black.B = 0;
    white.R = 255; white.G = 255; white.B = 255;
    draw24FillRect(win, white, w->size.x, w->size.y, w->size.width, w->size.height);
    drawRect(win, black, w->size.x, w->size.y, w->size.width, w->size.height);
    drawString(win, w->size.x + 3, w->size.y + 2, w->context.button->text, w->context.input->color, w->size.width - 3);
}

void drawTextAreaWidget(window *win, int index) {
}

void drawFileListWidget(window *win, int index) {
    Widget *w = &(win->widgets[index]);

    if (w->context.fileList->direction == 0) {
        int max_num_y = w->size.height / ICON_VIEW_SIZE;
        int max_num_x = w->size.width / ICON_VIEW_SIZE;
        int offset_y = w->size.height % ICON_VIEW_SIZE / max_num_y;
        int offset_x = w->size.width % ICON_VIEW_SIZE / max_num_x;
        int current_x = 0;
        int current_y = 0;
        IconView *p = w->context.fileList->file_list;
        int i;
        RGBA black;
        black.A = 255; black.R = 0; black.G = 0; black.B = 0;
        for (i = 0; i < w->context.fileList->file_num; i++) {
            drawImage(win, p->image, offset_x + current_x * ICON_VIEW_SIZE + 13,
                      offset_y + current_y * ICON_VIEW_SIZE + 4, ICON_IMG_SIZE, ICON_IMG_SIZE);
            if (strlen(p->text) <= 9) {
                drawString(win, offset_x + current_x * ICON_VIEW_SIZE + (9 - strlen(p->text)) * 9 / 2 + 5,
                           offset_y + current_y * ICON_VIEW_SIZE + 4 + ICON_IMG_SIZE, p->text, black,
                           ICON_VIEW_SIZE - 2);
            } else {
                char temp[10];
                int j;
                for (j = 0; j < 9; j++) {
                    temp[j] = p->text[j];
                }
                temp[9] = '\0';
                drawString(win, offset_x + current_x * ICON_VIEW_SIZE + 5,
                           offset_y + current_y * ICON_VIEW_SIZE + ICON_IMG_SIZE, temp,
                           black, ICON_VIEW_SIZE - 2);
                for (j = 0; j < 9; j++) {
                    if (!p->text[j + 9]) {
                        break;
                    }
                    temp[j] = p->text[j + 9];
                }
                temp[j] = '\0';
                drawString(win, offset_x + current_x * ICON_VIEW_SIZE + (9 - strlen(temp)) * 9 / 2 + 5,
                           offset_y + current_y * ICON_VIEW_SIZE + ICON_IMG_SIZE + 16, temp,
                           black, ICON_VIEW_SIZE - 2);
            }
            current_y++;
            p = p->next;
            if (current_y >= max_num_y) {
                current_y = 0;
                current_x ++;
                if (current_x >= max_num_x) {
                    break;
                }
            }
        }
    }
}

void drawAllWidget(window *win) {
    int i;
    for (i = 0; i < win->widget_number; i++) {
        win->widgets[i].paint(win, i);
    }
    updatewindow(win->handler, 0, 0, win->width, win->height);
}

void UI_suffix(char *t, char *s)
{
    int point = 0;

    while (*s != 0)
    {
        if (*s == '.')
            point = 1;
        s++;
    }
    if (point == 0)
    {
        strcpy(t, "");
        return;
    }
    while (*s != '.')
        s--;
    s++;
    strcpy(t, s);
}

void fileListDoubleClickHandler(window *win, int index, message *msg) {
    Widget *w = &(win->widgets[index]);
    if (w->context.fileList->direction == 0) { // on the desktop
        int max_num_y = w->size.height / ICON_VIEW_SIZE;
        int max_num_x = w->size.width / ICON_VIEW_SIZE;
        int offset_y = w->size.height % ICON_VIEW_SIZE / max_num_y;
        int offset_x = w->size.width % ICON_VIEW_SIZE / max_num_x;
        int current_x = (msg->params[0] - offset_x) / ICON_VIEW_SIZE;
        int current_y = (msg->params[1] - offset_y) / ICON_VIEW_SIZE;
        int calcu_index = current_x * max_num_y + current_y;
        if (calcu_index < w->context.fileList->file_num) {
            IconView *p = w->context.fileList->file_list;
            int i;
            for (i = 0; i < calcu_index; i++) {
                p = p->next;
            }
            char t[20];
            UI_suffix(t, p->text);
            if (strcmp(t, "bmp") == 0) {
                if (fork() == 0)
                {
                    char *argv2[] = { "image_viewer", p->text, 0};
                    exec(argv2[0], argv2);
//                    exit();
                }
            }
        }
    }
}

void mainLoop(window *win) {
    message msg;
    while (1) {
        if (getmessage(win->handler, &msg)) {
            if (msg.msg_type == WM_WINDOW_CLOSE) {
                UI_destroyWindow(win);
                break;
            }
            if (msg.msg_type == M_MOUSE_DBCLICK) {
                int i;
                for (i = 0; i < win->widget_number; i++) {
                    if (win->widgets[i].type == FILE_LIST) {
                        win->widgets[i].context.fileList->onDoubleClick(win, i, &msg);
                        break;
                    }
                }
            }
        }
    }
}