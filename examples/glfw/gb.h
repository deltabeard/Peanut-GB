#pragma once

#ifndef GB_H
#define GB_H

#define PEANUT_GB_12_COLOUR 1
#define PEANUT_GB_USE_NIBBLE_FOR_PALETTE 1
#include "../../peanut_gb.h"

struct priv
{
        struct gb_s gb;

        /* Pointer to allocated memory holding GB file. */
        uint8_t *rom;
        /* Pointer to allocated memory holding save file. */
        uint8_t *cart_ram;

        /* Framebuffer. */
        uint8_t fb[LCD_HEIGHT][LCD_WIDTH];
};

int gb_init_file(struct priv *p, const char *rom_file_name);

#endif //GB_H
