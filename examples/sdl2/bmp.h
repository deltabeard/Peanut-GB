/* 24-bit BMP (Bitmap) ANSI C header library
 * This is free and unencumbered software released into the public domain.
 */
#ifndef BMP_H
#define BMP_H

#define BMP_SIZE(w, h) ((h) * ((w) + ((w) * 3) % 4) * 3 + 14 + 40)

static unsigned long
bmp_size(long width, long height)
{
    long pad = ((width % 4) * 3) % 4; /* Overflow-safe */
    if (width < 1 || height < 1) {
        return 0; /* Illegal size */
    } else if (width > (0x7fffffffL - 14 - 40) / 3 / height - pad) {
        return 0; /* Overflow */
    } else {
        return height * (width + pad) * 3 + 14 + 40;
    }
}

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
bmp_set(void *buf, long x, long y, unsigned long color)
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
    p[0]  = color >>  0;
    p[1]  = color >>  8;
    p[2]  = color >> 16;
}

static unsigned long
bmp_get(const void *buf, long x, long y)
{
    const unsigned char *p;
    const unsigned char *hdr = (unsigned char *)buf;
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
    return (unsigned long)p[0] <<  0 |
           (unsigned long)p[1] <<  8 |
           (unsigned long)p[2] << 16;
}

static unsigned long
bmp_encode(float r, float g, float b)
{
    unsigned long ur = r * 255.0f + 0.5f;
    unsigned long ug = g * 255.0f + 0.5f;
    unsigned long ub = b * 255.0f + 0.5f;
    return ub | ug << 8 | ur << 16;
}

static void
bmp_decode(unsigned long c, float *r, float *g, float *b)
{
    unsigned long ur = c >> 16 & 0xff;
    unsigned long ug = c >>  8 & 0xff;
    unsigned long ub = c >>  0 & 0xff;
    *r = ur / 255.0f;
    *g = ug / 255.0f;
    *b = ub / 255.0f;
}

#endif /* BMP_H */
