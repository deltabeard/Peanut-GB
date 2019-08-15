/* 24-bit BMP (Bitmap) ANSI C header library
 * This is free and unencumbered software released into the public domain.
 */
#ifndef BMP_H
#define BMP_H

#define BMP_SIZE(w, h) ((h) * ((w) + ((w) * 3) % 4) * 3 + 14 + 40)

static void
bmp_init(void *buf, long width, long height)
{
    long pad;
    unsigned long size;
    unsigned long uw = width;
    unsigned long uh = -height;
    unsigned char *p = (unsigned char *)buf;

#ifdef BMP_COMPAT
    uh = height;
#endif

    /* bfType */
    *p++ = 0x42;
    *p++ = 0x4D;

    /* bfSize */
    pad = (width * 3) % 4;
    size = height * (width + pad) * 3 + 14 + 40;
    *p++ = size >>  0;
    *p++ = size >>  8;
    *p++ = size >> 16;
    *p++ = size >> 24;

    /* bfReserved1 + bfReserved2 */
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    /* bfOffBits */
    *p++ = 0x36;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    /* biSize */
    *p++ = 0x28;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    /* biWidth */
    *p++ = uw >>  0;
    *p++ = uw >>  8;
    *p++ = uw >> 16;
    *p++ = uw >> 24;

    /* biHeight */
    *p++ = uh >>  0;
    *p++ = uh >>  8;
    *p++ = uh >> 16;
    *p++ = uh >> 24;

    /* biPlanes */
    *p++ = 0x01;
    *p++ = 0x00;

    /* biBitCount */
    *p++ = 0x18;
    *p++ = 0x00;

    /* biCompression */
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    /* biSizeImage */
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    /* biXPelsPerMeter */
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    /* biYPelsPerMeter */
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    /* biClrUsed */
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    /* biClrImportant */
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    /* biClrImportant */
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p   = 0x00;
}

static void
bmp_set_rgb(void *buf, long x, long y,
		unsigned char r, unsigned char g, unsigned char b)
{
    unsigned char *p;
    unsigned char *hdr = (unsigned char *)buf;
    unsigned long width =
        (unsigned long)hdr[18] <<  0 |
        (unsigned long)hdr[19] <<  8 |
        (unsigned long)hdr[20] << 16 |
        (unsigned long)hdr[21] << 24;
    long pad = (width * 3) % 4;
#ifdef BMP_COMPAT
    unsigned long height =
        (unsigned long)hdr[22] <<  0 |
        (unsigned long)hdr[23] <<  8 |
        (unsigned long)hdr[24] << 16 |
        (unsigned long)hdr[25] << 24;
    y = height - y - 1;
#endif
    p = hdr + 14 + 40 + y * (width + pad) * 3 + x * 3;
    p[0]  = b;
    p[1]  = g;
    p[2]  = r;
}

#endif /* BMP_H */
