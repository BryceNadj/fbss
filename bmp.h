#ifndef BMP_H
#define BMP_H

#define PX_DATA_OFFSET 54 // 14 (header) + 40 (dib header)

// header field offsets
#define OFF_ID_1 0x00
#define OFF_ID_2 0x01
#define OFF_FILE_SIZE 0x02
#define OFF_PX_ARR 0x0A

// DIB header field offsets
#define OFF_DIB_SIZE 0x0E
#define OFF_WIDTH 0x12
#define OFF_HEIGHT 0x16
#define OFF_COLPLANE 0x1A
#define OFF_BPP 0x1C
#define OFF_COMPRESSION 0x1E
#define OFF_DATA_SIZE 0x22
#define OFF_PRINT_RES_X 0x26
#define OFF_PRINT_RES_Y 0x2A
#define OFF_PALETTE_COLS 0x2E
#define OFF_IMPORTANT_COLS 0x32

#define DIR_MODE 0775
#define FILE_MODE 0666
#define MAX_FNAME_SIZE 255

typedef struct dirent dirent;

#endif // BMP_H
