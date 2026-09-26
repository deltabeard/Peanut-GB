#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string.h>

#define MINIFB_DEVICE_PATH "/dev/fb0"
#define MINIFB_GB_WIDTH 160
#define MINIFB_GB_HEIGHT 144
#define MINIFB_SCALE 3


static int fb_fd = -1;
static unsigned char *fb_mem = NULL;
static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static long screensize = 0;

int mfb_open(const char* title, int width, int height)
{
    fb_fd = open(MINIFB_DEVICE_PATH, O_RDWR);
    if (fb_fd == -1) {
        printf("Error opening %s\n", MINIFB_DEVICE_PATH);
        return -1;
    }

    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo)) {
        printf("Error getting finfo");
        return -1;
    }

    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("Error getting vinfo");
        return -1;
    }

    screensize = finfo.smem_len; 
    fb_mem = (unsigned char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    
    if (fb_mem == MAP_FAILED) {
        printf("mmap failed");
        return -1;
    }

    memset(fb_mem, 0, screensize);

    printf("FB Initialized: %dx%d, %dbpp, line_len: %d\n", 
            vinfo.xres, vinfo.yres, vinfo.bits_per_pixel, finfo.line_length);

    return 1;
}

int mfb_update(void* buffer)
{
    if (fb_mem == NULL || buffer == NULL) return -1;

    const int bpp = vinfo.bits_per_pixel / 8;
    const int line_len = finfo.line_length;
    const int GB_W = MINIFB_GB_WIDTH;
    const int GB_H = MINIFB_GB_HEIGHT;
    const int SCALE = MINIFB_SCALE; 

    int start_x = (vinfo.xres - (GB_W * SCALE)) / 2;
    int start_y = (vinfo.yres - (GB_H * SCALE)) / 2;
    if (start_x < 0) start_x = 0;
    if (start_y < 0) start_y = 0;

    uint32_t *src = (uint32_t *)buffer;

    if (bpp == 4) { // 32 bits
        uint32_t row_cache[GB_W * SCALE]; 

        for (int y = 0; y < GB_H; y++) {
            uint32_t *src_row = &src[y * GB_W];

            for (int x = 0; x < GB_W; x++) {
                uint32_t pixel = src_row[x];
                for (int j = 0; j < SCALE; j++) {
                    row_cache[x * SCALE + j] = pixel;
                }
            }

            unsigned char *dst_base = fb_mem + (y * SCALE + start_y) * line_len + (start_x * 4);
            for (int i = 0; i < SCALE; i++) {
                memcpy(dst_base + (i * line_len), row_cache, sizeof(row_cache));
            }
        }
    } else if (bpp == 2) { // 16 bits
        return -1; // TODO: Implement 16bpp support
    }
    return 0;
}

void mfb_close(void) {
    if (fb_mem && fb_mem != MAP_FAILED) munmap(fb_mem, screensize);
    if (fb_fd >= 0) close(fb_fd);
    fb_mem = NULL;
}
