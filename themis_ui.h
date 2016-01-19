#ifndef __ASSEMBLER__
struct RGB;
struct RGBA;
struct message;
struct window;

#define MAX_WIDTH 800
#define MAX_HEIGHT 600

#define MAX_WIDGET 10
#define MAX_SHORT_STRLEN 20
#define MAX_LONG_STRLEN 512

// @para: window widget_index message
typedef void(*Handler)(struct window *win, int index, message *msg);

// @para: window widget_index
typedef void(*painter)(struct window *win, int index);

typedef struct widget_size {
    int x;
    int y;
    int width;
    int height;
} widget_size;

#define IMAGE 0
#define LABEL 1
#define BUTTON 2
#define INPUT 3
#define TEXT_AREA 4
#define FILE_LIST 5

#define FILE_TYPE_NUM 6
#define EXPLORER_FILE 0
#define TEXT_FILE 1
#define BMP_FILE 2
#define EXEC_FILE 3
#define FOLDER_FILE 4
#define UNKNOWN_FILE 5

#define ICON_IMG_SIZE 64

typedef struct Image {
    struct RGB *image;
} Image;

typedef struct Label {
    struct RGBA color;
    char text[MAX_SHORT_STRLEN];
} Label;

typedef struct Button {
    struct RGBA color;
    struct RGBA bg_color;
    char text[MAX_SHORT_STRLEN];
    Handler onLeftClick;
} Button;

typedef struct Input {
    struct RGBA color;
    int current_pos;
    char text[MAX_SHORT_STRLEN];
    Handler onKeyDown;
    Handler onLeftClick;
} Input;

typedef struct TextArea {
    int begin_line;
    int current_line;
    int current_pos;
    struct RGBA color;
    char text[MAX_LONG_STRLEN];
    Handler onKeyDown;
    Handler onLeftClick;
} TextArea;

typedef struct IconView {
    RGBA *image;
    char text[MAX_SHORT_STRLEN];
    struct IconView *next;
} IconView;

// TODO: I don't know...
typedef struct FileList {
    char direction;
    char scrollable;
    IconView *file_list;
    int file_num;
    char path[MAX_LONG_STRLEN];
    RGBA* image[FILE_TYPE_NUM];
    Handler onDoubleClick;
    Handler onLeftClick;
    Handler onFileChange;
} FileList;

typedef union widget_base {
    Image *image;
    Label *label;
    Input *input;
    TextArea *textArea;
    Button *button;
    FileList *fileList;
} widget_base;

typedef struct Widget {
    widget_base context;
    int type;
    widget_size size;
    painter paint;
} Widget;

typedef struct window {
    RGB *window_buf;
    int width;
    int height;
    int handler;
    int widget_number;
    Widget widgets[MAX_WIDGET];
} window;

#endif
