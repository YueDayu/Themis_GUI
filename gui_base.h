#define GUI_BUF 0x9000
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#ifndef __ASSEMBLER__

typedef struct GUI_MODE_INFO {
    unsigned short ModeAttributes;
    unsigned char WinAAttributes;
    unsigned char WinBAttributes;
    unsigned short WinGranularity;
    unsigned short WinSize;
    unsigned short WinASegment;
    unsigned short WinBSegment;
    unsigned long WinFuncPtr;
    unsigned short BytesPerScanLine;
    unsigned short XResolution;
    unsigned short YResolution;
    unsigned char XCharSize;
    unsigned char YCharSize;
    unsigned char NumberOfPlanes;
    unsigned char BitsPerPixel;
    unsigned char NumberOfBanks;
    unsigned char MemoryModel;
    unsigned char BankSize;
    unsigned char NumberOfImagePages;
    unsigned char Reserved_page;
    unsigned char RedMaskSize;
    unsigned char RedMaskPos;
    unsigned char GreenMaskSize;
    unsigned char GreenMaskPos;
    unsigned char BlueMaskSize;
    unsigned char BlueMaskPos;
    unsigned char ReservedMaskSize;
    unsigned char ReservedMaskPos;
    unsigned char DirectColorModeInfo;
    unsigned long PhysBasePtr;
    unsigned long OffScreenMemOffset;
    unsigned short OffScreenMemSize;
    unsigned char Reserved[206];
} GUI_MODE_INFO;

#endif
