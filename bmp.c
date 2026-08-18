#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "bmp.h"
// #include "shm.h"

#define HEADER_SIZE 14
#define DIB_HEADER_SIZE 40

typedef struct fb_var_screeninfo fb_var_screeninfo;
typedef struct fb_fix_screeninfo fb_fix_screeninfo;

typedef struct fb_data {
    int fd;
    fb_var_screeninfo *vinfo;
    fb_fix_screeninfo *finfo;
    unsigned width;
    unsigned height;
    unsigned bpp;
    unsigned line_length;
    unsigned screensize;
} fb_data_t;

void *map_shm_object(int fd, int arrSize);
void unmap_close(void *data, int arrSize, int fd);

char *fb_init(fb_data_t *fb_data);
void write_bmp_header(fb_data_t *fb_data, char out[HEADER_SIZE]);
void write_dib_header(fb_data_t *fb_data, char out[DIB_HEADER_SIZE]);
void write_px_data(fb_data_t *fb_data, char *buf, char *out);
void write_bytes(char *buf, int offset, int data, int num_bytes);

int create_write_file(char *data, int size);

int main(int argc, const char *argv[]) {
    puts("Opening framebuffer...");
    fb_data_t fb_data;
    char *fbp = fb_init(&fb_data);
    if (fbp == NULL) {
        puts("Oopsie");
        return 1;
    }
    puts("Framebuffer open");

    int total_size = HEADER_SIZE + DIB_HEADER_SIZE + fb_data.screensize;
    char *bmp = malloc(total_size);

    puts("Writing header...");
    write_bmp_header(&fb_data, bmp);
    printf("bmp data: %s\n", bmp);

    puts("Writing DIB Header...");
    write_dib_header(&fb_data, bmp);

    puts("Writing PX data...");
    write_px_data(&fb_data, fbp, bmp);

    puts("Creating file...");
    create_write_file(bmp, total_size);
    printf("File has been created/written to: %d bytes\n", total_size);

    free(bmp);
    unmap_close(fbp, fb_data.screensize, fb_data.fd);
    return 0;
}

void write_bmp_header(fb_data_t *fb_data, char out[HEADER_SIZE]) {
    // necessary first two bytes of a bmp header
    out[OFF_ID_1] = 0x42; // B
    out[OFF_ID_2] = 0x4D; // M

    // size of file =
    // 14 - header +
    // 40 - DIB header +
    // screensize
    // (get addr of out[2] -> cast to int ptr -> deref)
    write_bytes(out, OFF_FILE_SIZE, PX_DATA_OFFSET + fb_data->screensize, 4);
    write_bytes(out, OFF_PX_ARR, PX_DATA_OFFSET, 4);
}

void write_dib_header(fb_data_t *fb_data, char out[DIB_HEADER_SIZE]) {
    write_bytes(out, OFF_DIB_SIZE, DIB_HEADER_SIZE, 4);

    write_bytes(out, OFF_WIDTH, fb_data->width, 4);
    write_bytes(out, OFF_HEIGHT, fb_data->height, 4);

    // these (2) bytes refer to color plane and should always be set to 1
    // apparently
    write_bytes(out, OFF_COLPLANE, 1, 2);

    write_bytes(out, OFF_BPP, fb_data->bpp, 4);
    write_bytes(out, OFF_DATA_SIZE, fb_data->screensize, 4);

    // taken from wikipedia: 72 dpi * 39.3701 metres per inch = 2834.6472
    int print_res = 2835;
    write_bytes(out, OFF_PRINT_RES_X, print_res, 4);
    write_bytes(out, OFF_PRINT_RES_Y, print_res, 4);
}

void write_px_data(fb_data_t *fb_data, char *fbp, char *out) {
    int height = fb_data->height;
    int width = fb_data->line_length;
    int cur_ptr;
    for (int i = 0; i < height; i++) {
        cur_ptr = width * (height - i) - width;
        memcpy(out + PX_DATA_OFFSET + cur_ptr, fbp + (i * width), width);
    }
}

int create_write_file(char *bmp, int size) {
    // rx-rx-rx-
    // mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    char path[] = "./output/img.bmp";

    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd == -1) {
        perror("Error creating file");
    }

    ssize_t bytes_written = write(fd, bmp, size);
    if (bytes_written == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        return 1;
    }
    return 0;
}

char *fb_init(fb_data_t *fb_data) {
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    int fd = open("/dev/fb0", O_RDWR);
    if (fd == -1) {
        perror("Failed to open framebuffer device");
        return NULL;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Failed to get variable screen info");
        close(fd);
        return NULL;
    }

    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Failed to get fixed screen info");
        close(fd);
        return NULL;
    }

    fb_data->vinfo = &vinfo;
    fb_data->finfo = &finfo;
    fb_data->width = vinfo.xres;
    fb_data->height = vinfo.yres;
    fb_data->bpp = vinfo.bits_per_pixel;
    fb_data->line_length = finfo.line_length;
    fb_data->screensize = finfo.line_length * vinfo.yres;

    char *fbp =
        (char *)mmap(0, fb_data->screensize, PROT_READ, MAP_SHARED, fd, 0);

    if (fbp == MAP_FAILED) {
        perror("Failed to mmap framebuffer device");
        close(fb_data->fd);
        return NULL;
    }
    return fbp;
}

void write_bytes(char *buf, int offset, int data, int num_bytes) {
    for (int i = 0; i < num_bytes; i++) {
        buf[offset + i] = (data >> (i * 8)) & 0xFF;
    }
}

void *map_shm_object(int fd, int arrSize) {
    void *mem = mmap(NULL, arrSize * sizeof(int), PROT_WRITE | PROT_READ,
                     MAP_SHARED, fd, 0);
    if (mem == NULL) {
        perror("Error mapping memory");
    }
    return mem;
}

void unmap_close(void *data, int size, int fd) {
    munmap(data, size);
    close(fd);
}
