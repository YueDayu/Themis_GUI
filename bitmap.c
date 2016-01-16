#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "gui_base.h"
#include "bitmap.h"

void readBitmapHeader(int bmpFile, BITMAP_FILE_HEADER *bmpFileHeader, BITMAP_INFO_HEADER *bmpInfoHeader) {
    // Read Bitmap file header
    read(bmpFile, bmpFileHeader, sizeof(BITMAP_FILE_HEADER));
    // Read Bitmap info header
    read(bmpFile, bmpInfoHeader, sizeof(BITMAP_INFO_HEADER));
}

int readBitmapFile(char *fileName, RGBA *result, int *height, int *width) {
    int i;
    int bmpFile = open(fileName, 0);
    if (bmpFile < 0) {
        return -1;
    }

    BITMAP_FILE_HEADER bmpFileHeader;
    BITMAP_INFO_HEADER bmpInfoHeader;

    readBitmapHeader(bmpFile, &bmpFileHeader, &bmpInfoHeader);
    *width = bmpInfoHeader.biWidth;
    *height = bmpInfoHeader.biHeight;
    int column = bmpInfoHeader.biWidth;
    int row = bmpInfoHeader.biHeight;
    int bits = bmpInfoHeader.biBitCount;
    char tmpBytes[3];
    int rowBytes = column * bits / 8;
    char *buf = (char *) result;
    for (i = 0; i < row; i++) {
        if (bits == 32) {
            read(bmpFile, buf + i * rowBytes, rowBytes);
        } else {
            int j = 0;
            for (j = 0; j < column; j++) {
                read(bmpFile, buf + i * column * 4 + j * sizeof(RGBA), 3);
                *(buf + i * column * 4 + j * sizeof(RGBA) + 3) = 255;
            }
        }
        if (rowBytes % 4 > 0) {
            read(bmpFile, tmpBytes, 4 - (rowBytes % 4));
        }
    }

    close(bmpFile);
    return 0;
}

int read24BitmapFile(char *fileName, RGB *result, int *height, int *width) {
    int i;
    int bmpFile = open(fileName, 0);
    if (bmpFile < 0) {
        return -1;
    }

    BITMAP_FILE_HEADER bmpFileHeader;
    BITMAP_INFO_HEADER bmpInfoHeader;

    readBitmapHeader(bmpFile, &bmpFileHeader, &bmpInfoHeader);
    *width = bmpInfoHeader.biWidth;
    *height = bmpInfoHeader.biHeight;
    int column = bmpInfoHeader.biWidth;
    int row = bmpInfoHeader.biHeight;
    char tmpBytes[3];
    int rowBytes = column * 3;
    char *buf = (char *) result;
    for (i = 0; i < row; i++) {
        read(bmpFile, buf + i * rowBytes, rowBytes);
        if (rowBytes % 4 > 0) {
            read(bmpFile, tmpBytes, 4 - (rowBytes % 4));
        }
    }

    close(bmpFile);
    return 0;
}

