/**
 * MIT License
 *
 * Copyright (c) 2018 Mahyar Koshkouei
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Please note that at least three parts of source code within this project was
 * taken from the SameBoy project at https://github.com/LIJI32/SameBoy/ which at
 * the time of this writing is released under the MIT License. Occurrences of
 * this code is marked as being taken from SameBoy with a comment.
 * SameBoy, and code marked as being taken from SameBoy,
 * is Copyright (c) 2015-2019 Lior Halphon.
 */

#pragma once

#include <stdint.h> /* Required for int types */
#include <time.h>   /* Required for tm struct */

/**
 * Sound support must be provided by an external library. When audio_read() and
 * audio_write() functions are provided, define ENABLE_SOUND to a non-zero value
 * before including peanut_gb.h in order for these functions to be used.
 */
#ifndef ENABLE_SOUND
#define ENABLE_SOUND 0
#endif

/* Enable LCD drawing. On by default. May be turned off for testing purposes. */
#ifndef ENABLE_LCD
#define ENABLE_LCD 1
#endif

/* Interrupt masks */
#define VBLANK_INTR 0x01
#define LCDC_INTR 0x02
#define TIMER_INTR 0x04
#define SERIAL_INTR 0x08
#define CONTROL_INTR 0x10
#define ANY_INTR 0x1F

/* Memory section sizes for DMG */
#define SRAM_SIZE 0x20000
#define WRAM_SIZE 0x8000
#define VRAM_SIZE 0x4000
#define HRAM_SIZE 0x0100
#define OAM_SIZE 0x00A0

/* Memory addresses */
#define ROM_0_ADDR 0x0000
#define ROM_N_ADDR 0x4000
#define VRAM_ADDR 0x8000
#define CART_RAM_ADDR 0xA000
#define WRAM_0_ADDR 0xC000
#define WRAM_1_ADDR 0xD000
#define ECHO_ADDR 0xE000
#define OAM_ADDR 0xFE00
#define UNUSED_ADDR 0xFEA0
#define IO_ADDR 0xFF00
#define HRAM_ADDR 0xFF80
#define INTR_EN_ADDR 0xFFFF

/* Cart section sizes */
#define ROM_BANK_SIZE 0x4000
#define WRAM_BANK_SIZE 0x1000
#define CRAM_BANK_SIZE 0x2000
#define VRAM_BANK_SIZE 0x2000

/* DIV Register is incremented at rate of 16384Hz.
 * 4194304 / 16384 = 256 clock cycles for one increment. */
#define DIV_CYCLES 256

/* Serial clock locked to 8192Hz on DMG.
 * 4194304 / (8192 / 8) = 4096 clock cycles for sending 1 byte. */
#define SERIAL_CYCLES 4096

/* Calculating VSYNC. */
#define DMG_CLOCK_FREQ 4194304.0
#define SCREEN_REFRESH_CYCLES 70224.0
#define VERTICAL_SYNC (DMG_CLOCK_FREQ / SCREEN_REFRESH_CYCLES)

/* SERIAL SC register masks. */
#define SERIAL_SC_TX_START 0x80
#define SERIAL_SC_CLOCK_SRC 0x01

/* STAT register masks */
#define STAT_LYC_INTR 0x40
#define STAT_MODE_2_INTR 0x20
#define STAT_MODE_1_INTR 0x10
#define STAT_MODE_0_INTR 0x08
#define STAT_LYC_COINC 0x04
#define STAT_MODE 0x03
#define STAT_USER_BITS 0xF8

/* LCDC control masks */
#define LCDC_ENABLE 0x80
#define LCDC_WINDOW_MAP 0x40
#define LCDC_WINDOW_ENABLE 0x20
#define LCDC_TILE_SELECT 0x10
#define LCDC_BG_MAP 0x08
#define LCDC_OBJ_SIZE 0x04
#define LCDC_OBJ_ENABLE 0x02
#define LCDC_BG_ENABLE 0x01

/* LCD characteristics */
#define LCD_LINE_CYCLES 456
#define LCD_MODE_0_CYCLES 0
#define LCD_MODE_2_CYCLES 204
#define LCD_MODE_3_CYCLES 284
#define LCD_VERT_LINES 154
#define LCD_WIDTH 160
#define LCD_HEIGHT 144

/* VRAM Locations */
#define VRAM_TILES_1 (0x8000 - VRAM_ADDR)
#define VRAM_TILES_2 (0x8800 - VRAM_ADDR)
#define VRAM_BMAP_1 (0x9800 - VRAM_ADDR)
#define VRAM_BMAP_2 (0x9C00 - VRAM_ADDR)
#define VRAM_TILES_3 (0x8000 - VRAM_ADDR + VRAM_BANK_SIZE)
#define VRAM_TILES_4 (0x8800 - VRAM_ADDR + VRAM_BANK_SIZE)

/* Interrupt jump addresses */
#define VBLANK_INTR_ADDR 0x0040
#define LCDC_INTR_ADDR 0x0048
#define TIMER_INTR_ADDR 0x0050
#define SERIAL_INTR_ADDR 0x0058
#define CONTROL_INTR_ADDR 0x0060

/* SPRITE controls */
#define NUM_SPRITES 0x28
#define MAX_SPRITES_LINE 0x0A
#define OBJ_PRIORITY 0x80
#define OBJ_FLIP_Y 0x40
#define OBJ_FLIP_X 0x20
#define OBJ_PALETTE 0x10
#define OBJ_BANK 0x08
#define OBJ_CGB_PALETTE 0x07

#define ROM_HEADER_CHECKSUM_LOC 0x014D

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

struct cpu_registers_s {
    /* Combine A and F registers. */
    union {
        struct
        {
            /* Define specific bits of Flag register. */
            union {
                struct
                {
                    uint8_t unused : 4;
                    uint8_t c : 1; /* Carry flag. */
                    uint8_t h : 1; /* Half carry flag. */
                    uint8_t n : 1; /* Add/sub flag. */
                    uint8_t z : 1; /* Zero flag. */
                } f_bits;
                uint8_t f;
            };
            uint8_t a;
        };
        uint16_t af;
    };

    union {
        struct
        {
            uint8_t c;
            uint8_t b;
        };
        uint16_t bc;
    };

    union {
        struct
        {
            uint8_t e;
            uint8_t d;
        };
        uint16_t de;
    };

    union {
        struct
        {
            uint8_t l;
            uint8_t h;
        };
        uint16_t hl;
    };

    uint16_t sp; /* Stack pointer */
    uint16_t pc; /* Program counter */
};

struct count_s {
    uint_fast16_t lcd_count;    /* LCD Timing */
    uint_fast16_t div_count;    /* Divider Register Counter */
    uint_fast16_t tima_count;   /* Timer Counter */
    uint_fast16_t serial_count; /* Serial Counter */
};

struct gb_registers_s {
    /* TODO: Sort variables in address order. */
    /* Timing */
    uint8_t TIMA, TMA, DIV;
    union {
        struct
        {
            uint8_t tac_rate : 2;   /* Input clock select */
            uint8_t tac_enable : 1; /* Timer enable */
            uint8_t unused : 5;
        };
        uint8_t TAC;
    };

    /* LCD */
    uint8_t LCDC;
    uint8_t STAT;
    uint8_t SCY;
    uint8_t SCX;
    uint8_t LY;
    uint8_t LYC;
    uint8_t DMA;
    uint8_t BGP;
    uint8_t OBP0;
    uint8_t OBP1;
    uint8_t WY;
    uint8_t WX;

    /* Joypad info. */
    uint8_t P1;

    /* Serial data. */
    uint8_t SB;
    uint8_t SC;

    /* Interrupt flag. */
    uint8_t IF;

    /* Interrupt enable. */
    uint8_t IE;
};

#if ENABLE_LCD
/* Bit mask for the shade of pixel to display */
#define LCD_COLOUR 0x03
/**
    * Bit mask for whether a pixel is OBJ0, OBJ1, or BG. Each may have a different
    * palette when playing a DMG game on CGB.
    */
#define LCD_PALETTE_OBJ 0x10
#define LCD_PALETTE_BG 0x20
/**
    * Bit mask for the two bits listed above.
    * LCD_PALETTE_ALL == 0b00 --> OBJ0
    * LCD_PALETTE_ALL == 0b01 --> OBJ1
    * LCD_PALETTE_ALL == 0b10 --> BG
    * LCD_PALETTE_ALL == 0b11 --> NOT POSSIBLE
    */
#define LCD_PALETTE_ALL 0x30
#endif

/**
 * Errors that may occur during emulation.
 */
enum gb_error_e {
    GB_UNKNOWN_ERROR,
    GB_INVALID_OPCODE,
    GB_INVALID_READ,
    GB_INVALID_WRITE,

    GB_INVALID_MAX
};

/**
 * Errors that may occur during library initialisation.
 */
enum gb_init_error_e {
    GB_INIT_NO_ERROR,
    GB_INIT_CARTRIDGE_UNSUPPORTED,
    GB_INIT_INVALID_CHECKSUM
};

/**
 * Return codes for serial receive function, mainly for clarity.
 */
enum gb_serial_rx_ret_e {
    GB_SERIAL_RX_SUCCESS = 0,
    GB_SERIAL_RX_NO_CONNECTION = 1
};

/**
 * Emulator context.
 *
 * Only values within the `direct` struct may be modified directly by the
 * front-end implementation. Other variables must not be modified.
 */
struct gb_s {
    /**
     * Return byte from ROM at given address.
     *
     * \param gb_s    emulator context
     * \param addr    address
     * \return        byte at address in ROM
     */
    uint8_t (*gb_rom_read)(struct gb_s *, const uint_fast32_t addr);

    /**
     * Return byte from cart RAM at given address.
     *
     * \param gb_s    emulator context
     * \param addr    address
     * \return        byte at address in RAM
     */
    uint8_t (*gb_cart_ram_read)(struct gb_s *, const uint_fast32_t addr);

    /**
     * Write byte to cart RAM at given address.
     *
     * \param gb_s    emulator context
     * \param addr    address
     * \param val    value to write to address in RAM
     */
    void (*gb_cart_ram_write)(struct gb_s *, const uint_fast32_t addr,
                              const uint8_t val);

    /**
     * Notify front-end of error.
     *
     * \param gb_s            emulator context
     * \param gb_error_e    error code
     * \param val            arbitrary value related to error
     */
    void (*gb_error)(struct gb_s *, const enum gb_error_e, const uint16_t val);

    /* Transmit one byte and return the received byte. */
    void (*gb_serial_tx)(struct gb_s *, const uint8_t tx);
    enum gb_serial_rx_ret_e (*gb_serial_rx)(struct gb_s *, uint8_t *rx);

    struct
    {
        uint8_t gb_halt : 1;
        uint8_t gb_ime : 1;
        uint8_t gb_bios_enable : 1;
        uint8_t gb_frame : 1; /* New frame drawn. */

#define LCD_HBLANK 0
#define LCD_VBLANK 1
#define LCD_SEARCH_OAM 2
#define LCD_TRANSFER 3
        uint8_t lcd_mode : 2;
        uint8_t lcd_blank : 1;
    };

    /* Cartridge information:
     * Memory Bank Controller (MBC) type. */
    uint8_t mbc;
    /* Whether the MBC has internal RAM. */
    uint8_t cart_ram;
    /* Number of ROM banks in cartridge. */
    uint16_t num_rom_banks_mask;
    /* Number of RAM banks in cartridge. */
    uint8_t num_ram_banks;

    uint16_t selected_rom_bank;
    /* WRAM and VRAM bank selection not available. */
    uint8_t cart_ram_bank;
    uint16_t cart_ram_bank_offset;  //offset to subtract from the address to point to the right SRAM bank
    uint8_t enable_cart_ram;
    /* Cartridge ROM/RAM mode select. */
    uint8_t cart_mode_select;
    union {
        struct
        {
            uint8_t sec;
            uint8_t min;
            uint8_t hour;
            uint8_t yday;
            uint8_t high;
        } rtc_bits;
        uint8_t cart_rtc[5];
    };

    struct cpu_registers_s cpu_reg;
    struct gb_registers_s gb_reg;
    struct count_s counter;

    /* TODO: Allow implementation to allocate WRAM, VRAM and Frame Buffer. */
    uint8_t wram[WRAM_SIZE];
    uint8_t vram[VRAM_SIZE];
    uint8_t hram[HRAM_SIZE];
    uint8_t oam[OAM_SIZE];

    struct
    {
        /**
         * Draw line on screen.
         *
         * \param gb_s        emulator context
         * \param pixels    The 160 pixels to draw.
         *             Bits 1-0 are the colour to draw.
         *             Bits 5-4 are the palette, where:
         *                 OBJ0 = 0b00,
         *                 OBJ1 = 0b01,
         *                 BG = 0b10
         *             Other bits are undefined.
         *             Bits 5-4 are only required by front-ends
         *             which want to use a different colour for
         *             different object palettes. This is what
         *             the Game Boy Color (CGB) does to DMG
         *             games.
         * \param line        Line to draw pixels on. This is
         * guaranteed to be between 0-144 inclusive.
         */
        void (*lcd_draw_line)(struct gb_s *gb,
                              const uint8_t *pixels,
                              const uint_fast8_t line);

        /* Palettes */
        uint8_t bg_palette[4];
        uint8_t sp_palette[8];

        uint8_t window_clear;
        uint8_t WY;

        /* Only support 30fps frame skip. */
        uint8_t frame_skip_count : 1;
        uint8_t interlace_count : 1;
    } display;

    /* Game Boy Color Mode*/
    struct {
        uint8_t cgbMode;
        uint8_t doubleSpeed;
        uint8_t doubleSpeedPrep;
        uint8_t wramBank;
        uint16_t wramBankOffset;
        uint8_t vramBank;
        uint16_t vramBankOffset;
        uint16_t fixPalette[0x40];  //BG then OAM palettes fixed for the screen
        uint8_t OAMPalette[0x40];
        uint8_t BGPalette[0x40];
        uint8_t OAMPaletteID;
        uint8_t BGPaletteID;
        uint8_t OAMPaletteInc;
        uint8_t BGPaletteInc;
        uint8_t dmaActive;
        uint8_t dmaMode;
        uint8_t dmaSize;
        uint16_t dmaSource;
        uint16_t dmaDest;
    } cgb;
    /**
     * Variables that may be modified directly by the front-end.
     * This method seems to be easier and possibly less overhead than
     * calling a function to modify these variables each time.
     *
     * None of this is thread-safe.
     */
    struct
    {
        /* Set to enable interlacing. Interlacing will start immediately
         * (at the next line drawing).
         */
        uint8_t interlace : 1;
        uint8_t frame_skip : 1;

        union {
            struct
            {
                uint8_t a : 1;
                uint8_t b : 1;
                uint8_t select : 1;
                uint8_t start : 1;
                uint8_t right : 1;
                uint8_t left : 1;
                uint8_t up : 1;
                uint8_t down : 1;
            } joypad_bits;
            uint8_t joypad;
        };

        /* Implementation defined data. Set to NULL if not required. */
        void *priv;
    } direct;
};

/**
 * Tick the internal RTC by one second.
 * This was taken from SameBoy, which is released under MIT Licence.
 */
void gb_tick_rtc(struct gb_s *gb) {
    /* is timer running? */
    if ((gb->cart_rtc[4] & 0x40) == 0) {
        if (++gb->rtc_bits.sec == 60) {
            gb->rtc_bits.sec = 0;

            if (++gb->rtc_bits.min == 60) {
                gb->rtc_bits.min = 0;

                if (++gb->rtc_bits.hour == 24) {
                    gb->rtc_bits.hour = 0;

                    if (++gb->rtc_bits.yday == 0) {
                        if (gb->rtc_bits.high & 1) /* Bit 8 of days*/
                        {
                            gb->rtc_bits.high |= 0x80; /* Overflow bit */
                        }

                        gb->rtc_bits.high ^= 1;
                    }
                }
            }
        }
    }
}

/**
 * Set initial values in RTC.
 * Should be called after gb_init().
 */
void gb_set_rtc(struct gb_s *gb, const struct tm *const time) {
    gb->cart_rtc[0] = time->tm_sec;
    gb->cart_rtc[1] = time->tm_min;
    gb->cart_rtc[2] = time->tm_hour;
    gb->cart_rtc[3] = time->tm_yday & 0xFF; /* Low 8 bits of day counter. */
    gb->cart_rtc[4] = time->tm_yday >> 8;   /* High 1 bit of day counter. */
}

/**
 * Internal function used to read bytes.
 */
uint8_t __gb_read(struct gb_s *gb, const uint_fast16_t addr) {
    switch (addr >> 12) {
        case 0x0:

        /* TODO: BIOS support. */
        case 0x1:
        case 0x2:
        case 0x3:
            return gb->gb_rom_read(gb, addr);

        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
            if (gb->mbc == 1 && gb->cart_mode_select)
                return gb->gb_rom_read(gb,
                                       addr + ((gb->selected_rom_bank & 0x1F) - 1) * ROM_BANK_SIZE);
            else
                return gb->gb_rom_read(gb, addr + (gb->selected_rom_bank - 1) * ROM_BANK_SIZE);

        case 0x8:
        case 0x9:
            return gb->vram[addr - gb->cgb.vramBankOffset];

        case 0xA:
        case 0xB:
            if (gb->cart_ram && gb->enable_cart_ram) {
                if (gb->mbc == 3 && gb->cart_ram_bank >= 0x08)
                    return gb->cart_rtc[gb->cart_ram_bank - 0x08];
                else
                    return gb->gb_cart_ram_read(gb, addr - gb->cart_ram_bank_offset);
            }

            return 0xFF;

        case 0xC:
            return gb->wram[addr - WRAM_0_ADDR];

        case 0xD:
            return gb->wram[addr - gb->cgb.wramBankOffset];

        case 0xE:
            return gb->wram[addr - ECHO_ADDR];

        case 0xF:
            if (addr < OAM_ADDR)
                return gb->wram[(addr - 0x2000) - gb->cgb.wramBankOffset];

            if (addr < UNUSED_ADDR)
                return gb->oam[addr - OAM_ADDR];

            /* Unusable memory area. Reading from this area returns 0.*/
            if (addr < IO_ADDR)
                return 0xFF;

            /* HRAM */
            if (HRAM_ADDR <= addr && addr < INTR_EN_ADDR)
                return gb->hram[addr - IO_ADDR];

            if ((addr >= 0xFF10) && (addr <= 0xFF3F)) {
#if ENABLE_SOUND
                return audio_read(addr);
#else
                return 1;
#endif
            }

            /* IO and Interrupts. */
            switch (addr & 0xFF) {
                /* IO Registers */
                case 0x00:
                    return 0xC0 | gb->gb_reg.P1;

                case 0x01:
                    return gb->gb_reg.SB;

                case 0x02:
                    return gb->gb_reg.SC;

                /* Timer Registers */
                case 0x04:
                    return gb->gb_reg.DIV;

                case 0x05:
                    return gb->gb_reg.TIMA;

                case 0x06:
                    return gb->gb_reg.TMA;

                case 0x07:
                    return gb->gb_reg.TAC;

                /* Interrupt Flag Register */
                case 0x0F:
                    return gb->gb_reg.IF;

                /* LCD Registers */
                case 0x40:
                    return gb->gb_reg.LCDC;

                case 0x41:
                    return (gb->gb_reg.STAT & STAT_USER_BITS) |
                           (gb->gb_reg.LCDC & LCDC_ENABLE ? gb->lcd_mode : LCD_VBLANK);

                case 0x42:
                    return gb->gb_reg.SCY;

                case 0x43:
                    return gb->gb_reg.SCX;

                case 0x44:
                    return gb->gb_reg.LY;

                case 0x45:
                    return gb->gb_reg.LYC;

                /* DMA Register */
                case 0x46:
                    return gb->gb_reg.DMA;

                /* DMG Palette Registers */
                case 0x47:
                    return gb->gb_reg.BGP;

                case 0x48:
                    return gb->gb_reg.OBP0;

                case 0x49:
                    return gb->gb_reg.OBP1;

                /* Window Position Registers */
                case 0x4A:
                    return gb->gb_reg.WY;

                case 0x4B:
                    return gb->gb_reg.WX;

                /* Speed Switch*/
                case 0x4D:
                    return (gb->cgb.doubleSpeed << 7) + gb->cgb.doubleSpeedPrep;

                /* CGB VRAM Bank*/
                case 0x4F:
                    return gb->cgb.vramBank | 0xFE;

                /* CGB DMA*/
                case 0x51:
                    return (gb->cgb.dmaSource >> 8);
                case 0x52:
                    return (gb->cgb.dmaSource & 0xF0);
                case 0x53:
                    return (gb->cgb.dmaDest >> 8);
                case 0x54:
                    return (gb->cgb.dmaDest & 0xF0);
                case 0x55:
                    return (gb->cgb.dmaActive << 7) | (gb->cgb.dmaSize - 1);

                /* IR Register*/
                case 0x56:
                    return gb->hram[0x56];

                /* CGB BG Palette Index*/
                case 0x68:
                    return (gb->cgb.BGPaletteID & 0x3F) + (gb->cgb.BGPaletteInc << 7);

                /* CGB BG Palette*/
                case 0x69:
                    return gb->cgb.BGPalette[(gb->cgb.BGPaletteID & 0x3F)];

                /* CGB OAM Palette Index*/
                case 0x6A:
                    return (gb->cgb.OAMPaletteID & 0x3F) + (gb->cgb.OAMPaletteInc << 7);

                /* CGB OAM Palette*/
                case 0x6B:
                    return gb->cgb.OAMPalette[(gb->cgb.OAMPaletteID & 0x3F)];

                /* CGB WRAM Bank*/
                case 0x70:
                    return gb->cgb.wramBank;

                /* Interrupt Enable Register */
                case 0xFF:
                    return gb->gb_reg.IE;

                /* Unused registers return 1 */
                default:
                    return 0xFF;
            }
    }

    (gb->gb_error)(gb, GB_INVALID_READ, addr);
    return 0xFF;
}

/**
 * Internal function used to write bytes.
 */
void __gb_write(struct gb_s *gb, const uint_fast16_t addr, const uint8_t val) {
    switch (addr >> 12) {
        case 0x0:
        case 0x1:
            if (gb->mbc == 2 && addr & 0x10)
                return;
            else if (gb->mbc > 0 && gb->cart_ram)
                gb->enable_cart_ram = ((val & 0x0F) == 0x0A);

            return;

        case 0x2:
            if (gb->mbc == 5) {
                gb->selected_rom_bank = (gb->selected_rom_bank & 0x100) | val;
                gb->selected_rom_bank =
                    gb->selected_rom_bank & gb->num_rom_banks_mask;
                return;
            }

            /* Intentional fall through. */

        case 0x3:
            if (gb->mbc == 1) {
                //selected_rom_bank = val & 0x7;
                gb->selected_rom_bank = (val & 0x1F) | (gb->selected_rom_bank & 0x60);

                if ((gb->selected_rom_bank & 0x1F) == 0x00)
                    gb->selected_rom_bank++;
            } else if (gb->mbc == 2 && addr & 0x10) {
                gb->selected_rom_bank = val & 0x0F;

                if (!gb->selected_rom_bank)
                    gb->selected_rom_bank++;
            } else if (gb->mbc == 3) {
                gb->selected_rom_bank = val & 0x7F;

                if (!gb->selected_rom_bank)
                    gb->selected_rom_bank++;
            } else if (gb->mbc == 5)
                gb->selected_rom_bank = (val & 0x01) << 8 | (gb->selected_rom_bank & 0xFF);

            gb->selected_rom_bank = gb->selected_rom_bank & gb->num_rom_banks_mask;
            return;

        case 0x4:
        case 0x5:
            if (gb->mbc == 1) {
                gb->cart_ram_bank = (val & 3);
                gb->cart_ram_bank_offset = 0xA000 - (gb->cart_ram_bank << 13);
                gb->selected_rom_bank = ((val & 3) << 5) | (gb->selected_rom_bank & 0x1F);
                gb->selected_rom_bank = gb->selected_rom_bank & gb->num_rom_banks_mask;
            } else if (gb->mbc == 3) {
                gb->cart_ram_bank = val;
                gb->cart_ram_bank_offset = 0xA000 - ((gb->cart_ram_bank & 3) << 13);
            } else if (gb->mbc == 5) {
                gb->cart_ram_bank = (val & 0x0F);
                gb->cart_ram_bank_offset = 0xA000 - (gb->cart_ram_bank << 13);
            }
            return;

        case 0x6:
        case 0x7:
            gb->cart_mode_select = (val & 1);
            return;

        case 0x8:
        case 0x9:
            gb->vram[addr - gb->cgb.vramBankOffset] = val;
            return;

        case 0xA:
        case 0xB:
            if (gb->cart_ram && gb->enable_cart_ram) {
                if (gb->mbc == 3 && gb->cart_ram_bank >= 0x08)
                    gb->cart_rtc[gb->cart_ram_bank - 0x08] = val;
                else if (gb->num_ram_banks)
                    gb->gb_cart_ram_write(gb, addr - gb->cart_ram_bank_offset, val);
            }

            return;

        case 0xC:
            gb->wram[addr - WRAM_0_ADDR] = val;
            return;

        case 0xD:
            gb->wram[addr - gb->cgb.wramBankOffset] = val;
            return;

        case 0xE:
            gb->wram[addr - ECHO_ADDR] = val;
            return;

        case 0xF:
            if (addr < OAM_ADDR) {
                gb->wram[(addr - 0x2000) - gb->cgb.wramBankOffset] = val;
                return;
            }

            if (addr < UNUSED_ADDR) {
                gb->oam[addr - OAM_ADDR] = val;
                return;
            }

            /* Unusable memory area. */
            if (addr < IO_ADDR)
                return;

            if (HRAM_ADDR <= addr && addr < INTR_EN_ADDR) {
                gb->hram[addr - IO_ADDR] = val;
                return;
            }

            if ((addr >= 0xFF10) && (addr <= 0xFF3F)) {
#if ENABLE_SOUND
                audio_write(addr, val);
#endif
                return;
            }
            uint16_t fixPaletteTemp;
            /* IO and Interrupts. */
            switch (addr & 0xFF) {
                /* Joypad */
                case 0x00:
                    /* Only bits 5 and 4 are R/W.
             * The lower bits are overwritten later, and the two most
             * significant bits are unused. */
                    gb->gb_reg.P1 = val;

                    /* Direction keys selected */
                    if ((gb->gb_reg.P1 & 0b010000) == 0)
                        gb->gb_reg.P1 |= (gb->direct.joypad >> 4);
                    /* Button keys selected */
                    else
                        gb->gb_reg.P1 |= (gb->direct.joypad & 0x0F);

                    return;

                /* Serial */
                case 0x01:
                    gb->gb_reg.SB = val;
                    return;

                case 0x02:
                    gb->gb_reg.SC = val;
                    return;

                /* Timer Registers */
                case 0x04:
                    gb->gb_reg.DIV = 0x00;
                    return;

                case 0x05:
                    gb->gb_reg.TIMA = val;
                    return;

                case 0x06:
                    gb->gb_reg.TMA = val;
                    return;

                case 0x07:
                    gb->gb_reg.TAC = val;
                    return;

                /* Interrupt Flag Register */
                case 0x0F:
                    gb->gb_reg.IF = (val | 0b11100000);
                    return;

                /* LCD Registers */
                case 0x40:
                    if (((gb->gb_reg.LCDC & LCDC_ENABLE) == 0) &&
                        (val & LCDC_ENABLE)) {
                        gb->counter.lcd_count = 0;
                        gb->lcd_blank = 1;
                    }

                    gb->gb_reg.LCDC = val;

                    /* LY fixed to 0 when LCD turned off. */
                    if ((gb->gb_reg.LCDC & LCDC_ENABLE) == 0) {
                        /* Do not turn off LCD outside of VBLANK. This may
                 * happen due to poor timing in this emulator. */
                        if (gb->lcd_mode != LCD_VBLANK) {
                            gb->gb_reg.LCDC |= LCDC_ENABLE;
                            return;
                        }

                        gb->gb_reg.STAT = (gb->gb_reg.STAT & ~0x03) | LCD_VBLANK;
                        gb->gb_reg.LY = 0;
                        gb->counter.lcd_count = 0;
                    }

                    return;

                case 0x41:
                    gb->gb_reg.STAT = (val & 0b01111000);
                    return;

                case 0x42:
                    gb->gb_reg.SCY = val;
                    return;

                case 0x43:
                    gb->gb_reg.SCX = val;
                    return;

                /* LY (0xFF44) is read only. */
                case 0x45:
                    gb->gb_reg.LYC = val;
                    return;

                /* DMA Register */
                case 0x46:
                    gb->gb_reg.DMA = (val % 0xF1);

                    for (uint8_t i = 0; i < OAM_SIZE; i++)
                        gb->oam[i] = __gb_read(gb, (gb->gb_reg.DMA << 8) + i);

                    return;

                /* DMG Palette Registers */
                case 0x47:
                    gb->gb_reg.BGP = val;
                    gb->display.bg_palette[0] = (gb->gb_reg.BGP & 0x03);
                    gb->display.bg_palette[1] = (gb->gb_reg.BGP >> 2) & 0x03;
                    gb->display.bg_palette[2] = (gb->gb_reg.BGP >> 4) & 0x03;
                    gb->display.bg_palette[3] = (gb->gb_reg.BGP >> 6) & 0x03;
                    return;

                case 0x48:
                    gb->gb_reg.OBP0 = val;
                    gb->display.sp_palette[0] = (gb->gb_reg.OBP0 & 0x03);
                    gb->display.sp_palette[1] = (gb->gb_reg.OBP0 >> 2) & 0x03;
                    gb->display.sp_palette[2] = (gb->gb_reg.OBP0 >> 4) & 0x03;
                    gb->display.sp_palette[3] = (gb->gb_reg.OBP0 >> 6) & 0x03;
                    return;

                case 0x49:
                    gb->gb_reg.OBP1 = val;
                    gb->display.sp_palette[4] = (gb->gb_reg.OBP1 & 0x03);
                    gb->display.sp_palette[5] = (gb->gb_reg.OBP1 >> 2) & 0x03;
                    gb->display.sp_palette[6] = (gb->gb_reg.OBP1 >> 4) & 0x03;
                    gb->display.sp_palette[7] = (gb->gb_reg.OBP1 >> 6) & 0x03;
                    return;

                /* Window Position Registers */
                case 0x4A:
                    gb->gb_reg.WY = val;
                    return;

                case 0x4B:
                    gb->gb_reg.WX = val;
                    return;

                /* Prepare Speed Switch*/
                case 0x4D:
                    gb->cgb.doubleSpeedPrep = val & 1;
                    return;

                /* CGB VRAM Bank*/
                case 0x4F:
                    gb->cgb.vramBank = val & 0x01;
                    if (gb->cgb.cgbMode) gb->cgb.vramBankOffset = VRAM_ADDR - (gb->cgb.vramBank << 13);
                    return;

                /* Turn off boot ROM */
                case 0x50:
                    gb->gb_bios_enable = 0;
                    return;

                /* DMA Register */
                case 0x51:
                    gb->cgb.dmaSource = (gb->cgb.dmaSource & 0xFF) + (val << 8);
                    return;
                case 0x52:
                    gb->cgb.dmaSource = (gb->cgb.dmaSource & 0xFF00) + val;
                    return;
                case 0x53:
                    gb->cgb.dmaDest = (gb->cgb.dmaDest & 0xFF) + (val << 8);
                    return;
                case 0x54:
                    gb->cgb.dmaDest = (gb->cgb.dmaDest & 0xFF00) + val;
                    return;

                /* DMA Register*/
                case 0x55:
                    gb->cgb.dmaSize = (val & 0x7F) + 1;
                    gb->cgb.dmaMode = val >> 7;
                    //DMA GBC
                    if (gb->cgb.dmaActive) {  // Only transfer if dma is not active (=1) otherwise treat it as a termination
                        if (gb->cgb.cgbMode && (!gb->cgb.dmaMode)) {
                            for (int i = 0; i < (gb->cgb.dmaSize << 4); i++) {
                                __gb_write(gb, ((gb->cgb.dmaDest & 0x1FF0) | 0x8000) + i, __gb_read(gb, (gb->cgb.dmaSource & 0xFFF0) + i));
                            }
                            gb->cgb.dmaSource += (gb->cgb.dmaSize << 4);
                            gb->cgb.dmaDest += (gb->cgb.dmaSize << 4);
                            gb->cgb.dmaSize = 0;
                        }
                    }
                    gb->cgb.dmaActive = gb->cgb.dmaMode ^ 1;  // set active if it's an HBlank DMA
                    return;

                /* IR Register*/
                case 0x56:
                    gb->hram[0x56] = val;
                    return;

                /* CGB BG Palette Index*/
                case 0x68:
                    gb->cgb.BGPaletteID = val & 0x3F;
                    gb->cgb.BGPaletteInc = val >> 7;
                    return;

                /* CGB BG Palette*/
                case 0x69:
                    gb->cgb.BGPalette[(gb->cgb.BGPaletteID & 0x3F)] = val;
                    fixPaletteTemp = (gb->cgb.BGPalette[(gb->cgb.BGPaletteID & 0x3E) + 1] << 8) + (gb->cgb.BGPalette[(gb->cgb.BGPaletteID & 0x3E)]);
                    gb->cgb.fixPalette[((gb->cgb.BGPaletteID & 0x3E) >> 1)] = ((fixPaletteTemp & 0x7C00) >> 10) | (fixPaletteTemp & 0x03E0) | ((fixPaletteTemp & 0x001F) << 10);  // swap Red and Blue
                    if (gb->cgb.BGPaletteInc) gb->cgb.BGPaletteID = (++gb->cgb.BGPaletteID) & 0x3F;
                    return;

                /* CGB OAM Palette Index*/
                case 0x6A:
                    gb->cgb.OAMPaletteID = val & 0x3F;
                    gb->cgb.OAMPaletteInc = val >> 7;
                    return;

                /* CGB OAM Palette*/
                case 0x6B:
                    gb->cgb.OAMPalette[(gb->cgb.OAMPaletteID & 0x3F)] = val;
                    fixPaletteTemp = (gb->cgb.OAMPalette[(gb->cgb.OAMPaletteID & 0x3E) + 1] << 8) + (gb->cgb.OAMPalette[(gb->cgb.OAMPaletteID & 0x3E)]);
                    gb->cgb.fixPalette[0x20 + ((gb->cgb.OAMPaletteID & 0x3E) >> 1)] = ((fixPaletteTemp & 0x7C00) >> 10) | (fixPaletteTemp & 0x03E0) | ((fixPaletteTemp & 0x001F) << 10);  // swap Red and Blue
                    if (gb->cgb.OAMPaletteInc) gb->cgb.OAMPaletteID = (++gb->cgb.OAMPaletteID) & 0x3F;
                    return;

                /* CGB WRAM Bank*/
                case 0x70:
                    gb->cgb.wramBank = val;
                    gb->cgb.wramBankOffset = WRAM_1_ADDR - (1 << 12);
                    if (gb->cgb.cgbMode && (gb->cgb.wramBank & 7) > 0) gb->cgb.wramBankOffset = WRAM_1_ADDR - ((gb->cgb.wramBank & 7) << 12);
                    return;

                /* Interrupt Enable Register */
                case 0xFF:
                    gb->gb_reg.IE = val;
                    return;
            }
    }

    (gb->gb_error)(gb, GB_INVALID_WRITE, addr);
}

uint8_t __gb_execute_cb(struct gb_s *gb) {
    uint8_t inst_cycles;
    uint8_t cbop = __gb_read(gb, gb->cpu_reg.pc++);
    uint8_t r = (cbop & 0x7);
    uint8_t b = (cbop >> 3) & 0x7;
    uint8_t d = (cbop >> 3) & 0x1;
    uint8_t val;
    uint8_t writeback = 1;

    inst_cycles = 8;
    /* Add an additional 8 cycles to these sets of instructions. */
    switch (cbop & 0xC7) {
        case 0x06:
        case 0x86:
        case 0xC6:
            inst_cycles += 8;
            break;
        case 0x46:
            inst_cycles += 4;
            break;
    }

    switch (r) {
        case 0:
            val = gb->cpu_reg.b;
            break;

        case 1:
            val = gb->cpu_reg.c;
            break;

        case 2:
            val = gb->cpu_reg.d;
            break;

        case 3:
            val = gb->cpu_reg.e;
            break;

        case 4:
            val = gb->cpu_reg.h;
            break;

        case 5:
            val = gb->cpu_reg.l;
            break;

        case 6:
            val = __gb_read(gb, gb->cpu_reg.hl);
            break;

        /* Only values 0-7 are possible here, so we make the final case
     * default to satisfy -Wmaybe-uninitialized warning. */
        default:
            val = gb->cpu_reg.a;
            break;
    }

    /* TODO: Find out WTF this is doing. */
    switch (cbop >> 6) {
        case 0x0:
            cbop = (cbop >> 4) & 0x3;

            switch (cbop) {
                case 0x0:  /* RdC R */
                case 0x1:  /* Rd R */
                    if (d) /* RRC R / RR R */
                    {
                        uint8_t temp = val;
                        val = (val >> 1);
                        val |= cbop ? (gb->cpu_reg.f_bits.c << 7) : (temp << 7);
                        gb->cpu_reg.f_bits.z = (val == 0x00);
                        gb->cpu_reg.f_bits.n = 0;
                        gb->cpu_reg.f_bits.h = 0;
                        gb->cpu_reg.f_bits.c = (temp & 0x01);
                    } else /* RLC R / RL R */
                    {
                        uint8_t temp = val;
                        val = (val << 1);
                        val |= cbop ? gb->cpu_reg.f_bits.c : (temp >> 7);
                        gb->cpu_reg.f_bits.z = (val == 0x00);
                        gb->cpu_reg.f_bits.n = 0;
                        gb->cpu_reg.f_bits.h = 0;
                        gb->cpu_reg.f_bits.c = (temp >> 7);
                    }

                    break;

                case 0x2:
                    if (d) /* SRA R */
                    {
                        gb->cpu_reg.f_bits.c = val & 0x01;
                        val = (val >> 1) | (val & 0x80);
                        gb->cpu_reg.f_bits.z = (val == 0x00);
                        gb->cpu_reg.f_bits.n = 0;
                        gb->cpu_reg.f_bits.h = 0;
                    } else /* SLA R */
                    {
                        gb->cpu_reg.f_bits.c = (val >> 7);
                        val = val << 1;
                        gb->cpu_reg.f_bits.z = (val == 0x00);
                        gb->cpu_reg.f_bits.n = 0;
                        gb->cpu_reg.f_bits.h = 0;
                    }

                    break;

                case 0x3:
                    if (d) /* SRL R */
                    {
                        gb->cpu_reg.f_bits.c = val & 0x01;
                        val = val >> 1;
                        gb->cpu_reg.f_bits.z = (val == 0x00);
                        gb->cpu_reg.f_bits.n = 0;
                        gb->cpu_reg.f_bits.h = 0;
                    } else /* SWAP R */
                    {
                        uint8_t temp = (val >> 4) & 0x0F;
                        temp |= (val << 4) & 0xF0;
                        val = temp;
                        gb->cpu_reg.f_bits.z = (val == 0x00);
                        gb->cpu_reg.f_bits.n = 0;
                        gb->cpu_reg.f_bits.h = 0;
                        gb->cpu_reg.f_bits.c = 0;
                    }

                    break;
            }

            break;

        case 0x1: /* BIT B, R */
            gb->cpu_reg.f_bits.z = !((val >> b) & 0x1);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 1;
            writeback = 0;
            break;

        case 0x2: /* RES B, R */
            val &= (0xFE << b) | (0xFF >> (8 - b));
            break;

        case 0x3: /* SET B, R */
            val |= (0x1 << b);
            break;
    }

    if (writeback) {
        switch (r) {
            case 0:
                gb->cpu_reg.b = val;
                break;

            case 1:
                gb->cpu_reg.c = val;
                break;

            case 2:
                gb->cpu_reg.d = val;
                break;

            case 3:
                gb->cpu_reg.e = val;
                break;

            case 4:
                gb->cpu_reg.h = val;
                break;

            case 5:
                gb->cpu_reg.l = val;
                break;

            case 6:
                __gb_write(gb, gb->cpu_reg.hl, val);
                break;

            case 7:
                gb->cpu_reg.a = val;
                break;
        }
    }
    return inst_cycles;
}

#if ENABLE_LCD
void __gb_draw_line(struct gb_s *gb) {
    uint8_t pixels[160] = {0};

    /* If LCD not initialised by front-end, don't render anything. */
    if (gb->display.lcd_draw_line == NULL)
        return;

    if (gb->direct.frame_skip && !gb->display.frame_skip_count)
        return;

    uint8_t pixelsPrio[160] = {0};  //do these pixels have priority over OAM?
    /* If interlaced mode is activated, check if we need to draw the current
    * line. */
    if (gb->direct.interlace) {
        if ((gb->display.interlace_count == 0 && (gb->gb_reg.LY & 1) == 0) || (gb->display.interlace_count == 1 && (gb->gb_reg.LY & 1) == 1)) {
            /* Compensate for missing window draw if required. */
            if (gb->gb_reg.LCDC & LCDC_WINDOW_ENABLE && gb->gb_reg.LY >= gb->display.WY && gb->gb_reg.WX <= 166)
                gb->display.window_clear++;

            return;
        }
    }

    /* If background is enabled, draw it. */
    if (gb->gb_reg.LCDC & LCDC_BG_ENABLE) {
        /* Calculate current background line to draw. Constant because
        * this function draws only this one line each time it is
        * called. */
        const uint8_t bg_y = gb->gb_reg.LY + gb->gb_reg.SCY;

        /* Get selected background map address for first tile
        * corresponding to current line.
        * 0x20 (32) is the width of a background tile, and the bit
        * shift is to calculate the address. */
        const uint16_t bg_map =
            ((gb->gb_reg.LCDC & LCDC_BG_MAP) ? VRAM_BMAP_2 : VRAM_BMAP_1) + (bg_y >> 3) * 0x20;

        /* The displays (what the player sees) X coordinate, drawn right
        * to left. */
        uint8_t disp_x = LCD_WIDTH - 1;

        /* The X coordinate to begin drawing the background at. */
        uint8_t bg_x = disp_x + gb->gb_reg.SCX;

        /* Get tile index for current background tile. */
        uint8_t idx = gb->vram[bg_map + (bg_x >> 3)];
        uint8_t idxAtt = gb->vram[bg_map + (bg_x >> 3) + 0x2000];
        /* Y coordinate of tile pixel to draw. */
        const uint8_t py = (bg_y & 0x07);
        /* X coordinate of tile pixel to draw. */
        uint8_t px = 7 - (bg_x & 0x07);

        uint16_t tile;

        /* Select addressing mode. */
        if (gb->gb_reg.LCDC & LCDC_TILE_SELECT)
            tile = VRAM_TILES_1 + idx * 0x10;
        else
            tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;

        if (gb->cgb.cgbMode && (idxAtt & 0x08)) tile += 0x2000;  //VRAM bank 2
        if (gb->cgb.cgbMode && (idxAtt & 0x40)) tile += 2 * (7 - py);
        if (!gb->cgb.cgbMode || !(idxAtt & 0x40)) tile += 2 * py;

        /* fetch first tile */
        uint8_t t1, t2;
        if (gb->cgb.cgbMode && (idxAtt & 0x20)) {  //Horizantal Flip
            t1 = gb->vram[tile] << px;
            t2 = gb->vram[tile + 1] << px;
        } else {
            t1 = gb->vram[tile] >> px;
            t2 = gb->vram[tile + 1] >> px;
        }
        for (; disp_x != 0xFF; disp_x--) {
            if (px == 8) {
                /* fetch next tile */
                px = 0;
                bg_x = disp_x + gb->gb_reg.SCX;
                idx = gb->vram[bg_map + (bg_x >> 3)];
                idxAtt = gb->vram[bg_map + (bg_x >> 3) + 0x2000];

                if (gb->gb_reg.LCDC & LCDC_TILE_SELECT)
                    tile = VRAM_TILES_1 + idx * 0x10;
                else
                    tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;

                if (gb->cgb.cgbMode && (idxAtt & 0x08)) tile += 0x2000;  //VRAM bank 2
                if (gb->cgb.cgbMode && (idxAtt & 0x40)) tile += 2 * (7 - py);
                if (!gb->cgb.cgbMode || !(idxAtt & 0x40)) tile += 2 * py;
                t1 = gb->vram[tile];
                t2 = gb->vram[tile + 1];
            }

            /* copy background */
            if (gb->cgb.cgbMode && (idxAtt & 0x20)) {  //Horizantal Flip
                uint8_t c = (((t1 & 0x80) >> 1) | (t2 & 0x80)) >> 6;
                pixels[disp_x] = ((idxAtt & 0x07) << 2) + c;
                pixelsPrio[disp_x] = (idxAtt >> 7);
                t1 = t1 << 1;
                t2 = t2 << 1;
            } else {
                uint8_t c = (t1 & 0x1) | ((t2 & 0x1) << 1);
                if (gb->cgb.cgbMode) {
                    pixels[disp_x] = ((idxAtt & 0x07) << 2) + c;
                    pixelsPrio[disp_x] = (idxAtt >> 7);
                } else {
                    pixels[disp_x] = gb->display.bg_palette[c];
                    pixels[disp_x] |= LCD_PALETTE_BG;
                }
                t1 = t1 >> 1;
                t2 = t2 >> 1;
            }
            px++;
        }
    }

    /* draw window */
    if (gb->gb_reg.LCDC & LCDC_WINDOW_ENABLE && gb->gb_reg.LY >= gb->display.WY && gb->gb_reg.WX <= 166) {
        /* Calculate Window Map Address. */
        uint16_t win_line = (gb->gb_reg.LCDC & LCDC_WINDOW_MAP) ? VRAM_BMAP_2 : VRAM_BMAP_1;
        win_line += (gb->display.window_clear >> 3) * 0x20;

        uint8_t disp_x = LCD_WIDTH - 1;
        uint8_t win_x = disp_x - gb->gb_reg.WX + 7;

        // look up tile
        uint8_t py = gb->display.window_clear & 0x07;
        uint8_t px = 7 - (win_x & 0x07);
        uint8_t idx = gb->vram[win_line + (win_x >> 3)];
        uint8_t idxAtt = gb->vram[win_line + (win_x >> 3) + 0x2000];
        uint16_t tile;

        if (gb->gb_reg.LCDC & LCDC_TILE_SELECT)
            tile = VRAM_TILES_1 + idx * 0x10;
        else
            tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;

        if (gb->cgb.cgbMode && (idxAtt & 0x08)) tile += 0x2000;  //VRAM bank 2
        if (gb->cgb.cgbMode && (idxAtt & 0x40)) tile += 2 * (7 - py);
        if (!gb->cgb.cgbMode || !(idxAtt & 0x40)) tile += 2 * py;

        // fetch first tile
        uint8_t t1, t2;
        if (gb->cgb.cgbMode && (idxAtt & 0x20)) {  //Horizantal Flip
            t1 = gb->vram[tile] << px;
            t2 = gb->vram[tile + 1] << px;
        } else {
            t1 = gb->vram[tile] >> px;
            t2 = gb->vram[tile + 1] >> px;
        }
        // loop & copy window
        uint8_t end = (gb->gb_reg.WX < 7 ? 0 : gb->gb_reg.WX - 7) - 1;

        for (; disp_x != end; disp_x--) {
            if (px == 8) {
                // fetch next tile
                px = 0;
                win_x = disp_x - gb->gb_reg.WX + 7;
                idx = gb->vram[win_line + (win_x >> 3)];
                idxAtt = gb->vram[win_line + (win_x >> 3) + 0x2000];

                if (gb->gb_reg.LCDC & LCDC_TILE_SELECT)
                    tile = VRAM_TILES_1 + idx * 0x10;
                else
                    tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;

                if (gb->cgb.cgbMode && (idxAtt & 0x08)) tile += 0x2000;  //VRAM bank 2
                if (gb->cgb.cgbMode && (idxAtt & 0x40)) tile += 2 * (7 - py);
                if (!gb->cgb.cgbMode || !(idxAtt & 0x40)) tile += 2 * py;
                t1 = gb->vram[tile];
                t2 = gb->vram[tile + 1];
            }

            // copy window
            if (idxAtt & 0x20) {  //Horizantal Flip
                uint8_t c = (((t1 & 0x80) >> 1) | (t2 & 0x80)) >> 6;
                pixels[disp_x] = ((idxAtt & 0x07) << 2) + c;
                pixelsPrio[disp_x] = (idxAtt >> 7);
                t1 = t1 << 1;
                t2 = t2 << 1;
            } else {
                uint8_t c = (t1 & 0x1) | ((t2 & 0x1) << 1);
                if (gb->cgb.cgbMode) {
                    pixels[disp_x] = ((idxAtt & 0x07) << 2) + c;
                    pixelsPrio[disp_x] = (idxAtt >> 7);
                } else {
                    pixels[disp_x] = gb->display.bg_palette[c];
                    pixels[disp_x] |= LCD_PALETTE_BG;
                }
                t1 = t1 >> 1;
                t2 = t2 >> 1;
            }
            px++;
        }

        gb->display.window_clear++;  // advance window line
    }

    // draw sprites
    if (gb->gb_reg.LCDC & LCDC_OBJ_ENABLE) {
        uint8_t count = 0;

        for (uint8_t s = NUM_SPRITES - 1;
             s != 0xFF /* && count < MAX_SPRITES_LINE */;
             s--) {
            /* Sprite Y position. */
            uint8_t OY = gb->oam[4 * s + 0];
            /* Sprite X position. */
            uint8_t OX = gb->oam[4 * s + 1];
            /* Sprite Tile/Pattern Number. */
            uint8_t OT = gb->oam[4 * s + 2] & (gb->gb_reg.LCDC & LCDC_OBJ_SIZE ? 0xFE : 0xFF);
            /* Additional attributes. */
            uint8_t OF = gb->oam[4 * s + 3];

            /* If sprite isn't on this line, continue. */
            if (gb->gb_reg.LY +
                        (gb->gb_reg.LCDC & LCDC_OBJ_SIZE ? 0 : 8) >=
                    OY ||
                gb->gb_reg.LY + 16 < OY)
                continue;

            count++;

            /* Continue if sprite not visible. */
            if (OX == 0 || OX >= 168)
                continue;

            // y flip
            uint8_t py = gb->gb_reg.LY - OY + 16;

            if (OF & OBJ_FLIP_Y)
                py = (gb->gb_reg.LCDC & LCDC_OBJ_SIZE ? 15 : 7) - py;

            // fetch the tile
            uint8_t t1, t2;
            if (gb->cgb.cgbMode) {
                t1 = gb->vram[((OF & OBJ_BANK) << 10) + VRAM_TILES_1 + OT * 0x10 + 2 * py];
                t2 = gb->vram[((OF & OBJ_BANK) << 10) + VRAM_TILES_1 + OT * 0x10 + 2 * py + 1];
            } else {
                t1 = gb->vram[VRAM_TILES_1 + OT * 0x10 + 2 * py];
                t2 = gb->vram[VRAM_TILES_1 + OT * 0x10 + 2 * py + 1];
            }

            // handle x flip
            uint8_t dir, start, end, shift;

            if (OF & OBJ_FLIP_X) {
                dir = 1;
                start = (OX < 8 ? 0 : OX - 8);
                end = MIN(OX, LCD_WIDTH);
                shift = 8 - OX + start;
            } else {
                dir = -1;
                start = MIN(OX, LCD_WIDTH) - 1;
                end = (OX < 8 ? 0 : OX - 8) - 1;
                shift = OX - (start + 1);
            }

            // copy tile
            t1 >>= shift;
            t2 >>= shift;

            for (uint8_t disp_x = start; disp_x != end; disp_x += dir) {
                uint8_t c = (t1 & 0x1) | ((t2 & 0x1) << 1);
                // check transparency / sprite overlap / background overlap
                if (gb->cgb.cgbMode && (c && !(pixelsPrio[disp_x] && (pixels[disp_x] & 0x3)) && !((OF & OBJ_PRIORITY) && (pixels[disp_x] & 0x3)))) {
                    /* Set pixel colour. */
                    pixels[disp_x] = ((OF & OBJ_CGB_PALETTE) << 2) + c + 0x20;  // add 0x20 to differentiate from BG
                } else if (c && !(OF & OBJ_PRIORITY && pixels[disp_x] & 0x3)) {
                    /* Set pixel colour. */
                    pixels[disp_x] = (OF & OBJ_PALETTE)
                                         ? gb->display.sp_palette[c + 4]
                                         : gb->display.sp_palette[c];
                    /* Set pixel palette (OBJ0 or OBJ1). */
                    pixels[disp_x] |= (OF & OBJ_PALETTE);
                    /* Deselect BG palette. */
                    pixels[disp_x] &= ~LCD_PALETTE_BG;
                }

                t1 = t1 >> 1;
                t2 = t2 >> 1;
            }
        }
    }

    gb->display.lcd_draw_line(gb, pixels, gb->gb_reg.LY);
}
#endif

/**
 * Internal function used to step the CPU.
 */
void __gb_step_cpu(struct gb_s *gb) {
    uint8_t opcode, inst_cycles;
    static const uint8_t op_cycles[0x100] =
        {
            /* *INDENT-OFF* */
            /*0 1 2  3  4  5  6  7  8  9  A  B  C  D  E  F    */
            4, 12, 8, 8, 4, 4, 8, 4, 20, 8, 8, 8, 4, 4, 8, 4,          /* 0x00 */
            4, 12, 8, 8, 4, 4, 8, 4, 12, 8, 8, 8, 4, 4, 8, 4,          /* 0x10 */
            8, 12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4,           /* 0x20 */
            8, 12, 8, 8, 12, 12, 12, 4, 8, 8, 8, 8, 4, 4, 8, 4,        /* 0x30 */
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,            /* 0x40 */
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,            /* 0x50 */
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,            /* 0x60 */
            8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4,            /* 0x70 */
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,            /* 0x80 */
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,            /* 0x90 */
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,            /* 0xA0 */
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,            /* 0xB0 */
            8, 12, 12, 16, 12, 16, 8, 16, 8, 16, 12, 8, 12, 24, 8, 16, /* 0xC0 */
            8, 12, 12, 0, 12, 16, 8, 16, 8, 16, 12, 0, 12, 0, 8, 16,   /* 0xD0 */
            12, 12, 8, 0, 0, 16, 8, 16, 16, 4, 16, 0, 0, 0, 8, 16,     /* 0xE0 */
            12, 12, 8, 4, 0, 16, 8, 16, 12, 8, 16, 4, 0, 0, 8, 16      /* 0xF0 */
                                                                       /* *INDENT-ON* */
        };

    /* Handle interrupts */
    if ((gb->gb_ime || gb->gb_halt) &&
        (gb->gb_reg.IF & gb->gb_reg.IE & ANY_INTR)) {
        gb->gb_halt = 0;

        if (gb->gb_ime) {
            /* Disable interrupts */
            gb->gb_ime = 0;

            /* Push Program Counter */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);

            /* Call interrupt handler if required. */
            if (gb->gb_reg.IF & gb->gb_reg.IE & VBLANK_INTR) {
                gb->cpu_reg.pc = VBLANK_INTR_ADDR;
                gb->gb_reg.IF ^= VBLANK_INTR;
            } else if (gb->gb_reg.IF & gb->gb_reg.IE & LCDC_INTR) {
                gb->cpu_reg.pc = LCDC_INTR_ADDR;
                gb->gb_reg.IF ^= LCDC_INTR;
            } else if (gb->gb_reg.IF & gb->gb_reg.IE & TIMER_INTR) {
                gb->cpu_reg.pc = TIMER_INTR_ADDR;
                gb->gb_reg.IF ^= TIMER_INTR;
            } else if (gb->gb_reg.IF & gb->gb_reg.IE & SERIAL_INTR) {
                gb->cpu_reg.pc = SERIAL_INTR_ADDR;
                gb->gb_reg.IF ^= SERIAL_INTR;
            } else if (gb->gb_reg.IF & gb->gb_reg.IE & CONTROL_INTR) {
                gb->cpu_reg.pc = CONTROL_INTR_ADDR;
                gb->gb_reg.IF ^= CONTROL_INTR;
            }
        }
    }

    /* Obtain opcode */
    opcode = (gb->gb_halt ? 0x00 : __gb_read(gb, gb->cpu_reg.pc++));
    inst_cycles = op_cycles[opcode];

    /* Execute opcode */
    switch (opcode) {
        case 0x00: /* NOP */
            break;

        case 0x01: /* LD BC, imm */
            gb->cpu_reg.c = __gb_read(gb, gb->cpu_reg.pc++);
            gb->cpu_reg.b = __gb_read(gb, gb->cpu_reg.pc++);
            break;

        case 0x02: /* LD (BC), A */
            __gb_write(gb, gb->cpu_reg.bc, gb->cpu_reg.a);
            break;

        case 0x03: /* INC BC */
            gb->cpu_reg.bc++;
            break;

        case 0x04: /* INC B */
            gb->cpu_reg.b++;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.b == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.b & 0x0F) == 0x00);
            break;

        case 0x05: /* DEC B */
            gb->cpu_reg.b--;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.b == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.b & 0x0F) == 0x0F);
            break;

        case 0x06: /* LD B, imm */
            gb->cpu_reg.b = __gb_read(gb, gb->cpu_reg.pc++);
            break;

        case 0x07: /* RLCA */
            gb->cpu_reg.a = (gb->cpu_reg.a << 1) | (gb->cpu_reg.a >> 7);
            gb->cpu_reg.f_bits.z = 0;
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = (gb->cpu_reg.a & 0x01);
            break;

        case 0x08: /* LD (imm), SP */
        {
            uint16_t temp = __gb_read(gb, gb->cpu_reg.pc++);
            temp |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
            __gb_write(gb, temp++, gb->cpu_reg.sp & 0xFF);
            __gb_write(gb, temp, gb->cpu_reg.sp >> 8);
            break;
        }

        case 0x09: /* ADD HL, BC */
        {
            uint_fast32_t temp = gb->cpu_reg.hl + gb->cpu_reg.bc;
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (temp ^ gb->cpu_reg.hl ^ gb->cpu_reg.bc) & 0x1000 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFFFF0000) ? 1 : 0;
            gb->cpu_reg.hl = (temp & 0x0000FFFF);
            break;
        }

        case 0x0A: /* LD A, (BC) */
            gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.bc);
            break;

        case 0x0B: /* DEC BC */
            gb->cpu_reg.bc--;
            break;

        case 0x0C: /* INC C */
            gb->cpu_reg.c++;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.c == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.c & 0x0F) == 0x00);
            break;

        case 0x0D: /* DEC C */
            gb->cpu_reg.c--;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.c == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.c & 0x0F) == 0x0F);
            break;

        case 0x0E: /* LD C, imm */
            gb->cpu_reg.c = __gb_read(gb, gb->cpu_reg.pc++);
            break;

        case 0x0F: /* RRCA */
            gb->cpu_reg.f_bits.c = gb->cpu_reg.a & 0x01;
            gb->cpu_reg.a = (gb->cpu_reg.a >> 1) | (gb->cpu_reg.a << 7);
            gb->cpu_reg.f_bits.z = 0;
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            break;

        case 0x10: /* STOP */
            //gb->gb_halt = 1;
            if (gb->cgb.cgbMode & gb->cgb.doubleSpeedPrep) {
                gb->cgb.doubleSpeedPrep = 0;
                gb->cgb.doubleSpeed ^= 1;
            }
            break;

        case 0x11: /* LD DE, imm */
            gb->cpu_reg.e = __gb_read(gb, gb->cpu_reg.pc++);
            gb->cpu_reg.d = __gb_read(gb, gb->cpu_reg.pc++);
            break;

        case 0x12: /* LD (DE), A */
            __gb_write(gb, gb->cpu_reg.de, gb->cpu_reg.a);
            break;

        case 0x13: /* INC DE */
            gb->cpu_reg.de++;
            break;

        case 0x14: /* INC D */
            gb->cpu_reg.d++;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.d == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.d & 0x0F) == 0x00);
            break;

        case 0x15: /* DEC D */
            gb->cpu_reg.d--;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.d == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.d & 0x0F) == 0x0F);
            break;

        case 0x16: /* LD D, imm */
            gb->cpu_reg.d = __gb_read(gb, gb->cpu_reg.pc++);
            break;

        case 0x17: /* RLA */
        {
            uint8_t temp = gb->cpu_reg.a;
            gb->cpu_reg.a = (gb->cpu_reg.a << 1) | gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = 0;
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = (temp >> 7) & 0x01;
            break;
        }

        case 0x18: /* JR imm */
        {
            int8_t temp = (int8_t)__gb_read(gb, gb->cpu_reg.pc++);
            gb->cpu_reg.pc += temp;
            break;
        }

        case 0x19: /* ADD HL, DE */
        {
            uint_fast32_t temp = gb->cpu_reg.hl + gb->cpu_reg.de;
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (temp ^ gb->cpu_reg.hl ^ gb->cpu_reg.de) & 0x1000 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFFFF0000) ? 1 : 0;
            gb->cpu_reg.hl = (temp & 0x0000FFFF);
            break;
        }

        case 0x1A: /* LD A, (DE) */
            gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.de);
            break;

        case 0x1B: /* DEC DE */
            gb->cpu_reg.de--;
            break;

        case 0x1C: /* INC E */
            gb->cpu_reg.e++;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.e == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.e & 0x0F) == 0x00);
            break;

        case 0x1D: /* DEC E */
            gb->cpu_reg.e--;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.e == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.e & 0x0F) == 0x0F);
            break;

        case 0x1E: /* LD E, imm */
            gb->cpu_reg.e = __gb_read(gb, gb->cpu_reg.pc++);
            break;

        case 0x1F: /* RRA */
        {
            uint8_t temp = gb->cpu_reg.a;
            gb->cpu_reg.a = gb->cpu_reg.a >> 1 | (gb->cpu_reg.f_bits.c << 7);
            gb->cpu_reg.f_bits.z = 0;
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = temp & 0x1;
            break;
        }

        case 0x20: /* JP NZ, imm */
            if (!gb->cpu_reg.f_bits.z) {
                int8_t temp = (int8_t)__gb_read(gb, gb->cpu_reg.pc++);
                gb->cpu_reg.pc += temp;
                inst_cycles += 4;
            } else
                gb->cpu_reg.pc++;

            break;

        case 0x21: /* LD HL, imm */
            gb->cpu_reg.l = __gb_read(gb, gb->cpu_reg.pc++);
            gb->cpu_reg.h = __gb_read(gb, gb->cpu_reg.pc++);
            break;

        case 0x22: /* LDI (HL), A */
            __gb_write(gb, gb->cpu_reg.hl, gb->cpu_reg.a);
            gb->cpu_reg.hl++;
            break;

        case 0x23: /* INC HL */
            gb->cpu_reg.hl++;
            break;

        case 0x24: /* INC H */
            gb->cpu_reg.h++;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.h == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.h & 0x0F) == 0x00);
            break;

        case 0x25: /* DEC H */
            gb->cpu_reg.h--;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.h == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.h & 0x0F) == 0x0F);
            break;

        case 0x26: /* LD H, imm */
            gb->cpu_reg.h = __gb_read(gb, gb->cpu_reg.pc++);
            break;

        case 0x27: /* DAA */
        {
            uint16_t a = gb->cpu_reg.a;

            if (gb->cpu_reg.f_bits.n) {
                if (gb->cpu_reg.f_bits.h)
                    a = (a - 0x06) & 0xFF;

                if (gb->cpu_reg.f_bits.c)
                    a -= 0x60;
            } else {
                if (gb->cpu_reg.f_bits.h || (a & 0x0F) > 9)
                    a += 0x06;

                if (gb->cpu_reg.f_bits.c || a > 0x9F)
                    a += 0x60;
            }

            if ((a & 0x100) == 0x100)
                gb->cpu_reg.f_bits.c = 1;

            gb->cpu_reg.a = a;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0);
            gb->cpu_reg.f_bits.h = 0;

            break;
        }

        case 0x28: /* JP Z, imm */
            if (gb->cpu_reg.f_bits.z) {
                int8_t temp = (int8_t)__gb_read(gb, gb->cpu_reg.pc++);
                gb->cpu_reg.pc += temp;
                inst_cycles += 4;
            } else
                gb->cpu_reg.pc++;

            break;

        case 0x29: /* ADD HL, HL */
        {
            uint_fast32_t temp = gb->cpu_reg.hl + gb->cpu_reg.hl;
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = (temp & 0x1000) ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFFFF0000) ? 1 : 0;
            gb->cpu_reg.hl = (temp & 0x0000FFFF);
            break;
        }

        case 0x2A: /* LD A, (HL+) */
            gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.hl++);
            break;

        case 0x2B: /* DEC HL */
            gb->cpu_reg.hl--;
            break;

        case 0x2C: /* INC L */
            gb->cpu_reg.l++;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.l == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.l & 0x0F) == 0x00);
            break;

        case 0x2D: /* DEC L */
            gb->cpu_reg.l--;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.l == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.l & 0x0F) == 0x0F);
            break;

        case 0x2E: /* LD L, imm */
            gb->cpu_reg.l = __gb_read(gb, gb->cpu_reg.pc++);
            break;

        case 0x2F: /* CPL */
            gb->cpu_reg.a = ~gb->cpu_reg.a;
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = 1;
            break;

        case 0x30: /* JP NC, imm */
            if (!gb->cpu_reg.f_bits.c) {
                int8_t temp = (int8_t)__gb_read(gb, gb->cpu_reg.pc++);
                gb->cpu_reg.pc += temp;
                inst_cycles += 4;
            } else
                gb->cpu_reg.pc++;

            break;

        case 0x31: /* LD SP, imm */
            gb->cpu_reg.sp = __gb_read(gb, gb->cpu_reg.pc++);
            gb->cpu_reg.sp |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
            break;

        case 0x32: /* LD (HL), A */
            __gb_write(gb, gb->cpu_reg.hl, gb->cpu_reg.a);
            gb->cpu_reg.hl--;
            break;

        case 0x33: /* INC SP */
            gb->cpu_reg.sp++;
            break;

        case 0x34: /* INC (HL) */
        {
            uint8_t temp = __gb_read(gb, gb->cpu_reg.hl) + 1;
            gb->cpu_reg.f_bits.z = (temp == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = ((temp & 0x0F) == 0x00);
            __gb_write(gb, gb->cpu_reg.hl, temp);
            break;
        }

        case 0x35: /* DEC (HL) */
        {
            uint8_t temp = __gb_read(gb, gb->cpu_reg.hl) - 1;
            gb->cpu_reg.f_bits.z = (temp == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = ((temp & 0x0F) == 0x0F);
            __gb_write(gb, gb->cpu_reg.hl, temp);
            break;
        }

        case 0x36: /* LD (HL), imm */
            __gb_write(gb, gb->cpu_reg.hl, __gb_read(gb, gb->cpu_reg.pc++));
            break;

        case 0x37: /* SCF */
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 1;
            break;

        case 0x38: /* JP C, imm */
            if (gb->cpu_reg.f_bits.c) {
                int8_t temp = (int8_t)__gb_read(gb, gb->cpu_reg.pc++);
                gb->cpu_reg.pc += temp;
                inst_cycles += 4;
            } else
                gb->cpu_reg.pc++;

            break;

        case 0x39: /* ADD HL, SP */
        {
            uint_fast32_t temp = gb->cpu_reg.hl + gb->cpu_reg.sp;
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                ((gb->cpu_reg.hl & 0xFFF) + (gb->cpu_reg.sp & 0xFFF)) & 0x1000 ? 1 : 0;
            gb->cpu_reg.f_bits.c = temp & 0x10000 ? 1 : 0;
            gb->cpu_reg.hl = (uint16_t)temp;
            break;
        }

        case 0x3A: /* LD A, (HL--) */
            gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.hl--);
            break;

        case 0x3B: /* DEC SP */
            gb->cpu_reg.sp--;
            break;

        case 0x3C: /* INC A */
            gb->cpu_reg.a++;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.a & 0x0F) == 0x00);
            break;

        case 0x3D: /* DEC A */
            gb->cpu_reg.a--;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.a & 0x0F) == 0x0F);
            break;

        case 0x3E: /* LD A, imm */
            gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.pc++);
            break;

        case 0x3F: /* CCF */
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = ~gb->cpu_reg.f_bits.c;
            break;

        case 0x40: /* LD B, B */
            break;

        case 0x41: /* LD B, C */
            gb->cpu_reg.b = gb->cpu_reg.c;
            break;

        case 0x42: /* LD B, D */
            gb->cpu_reg.b = gb->cpu_reg.d;
            break;

        case 0x43: /* LD B, E */
            gb->cpu_reg.b = gb->cpu_reg.e;
            break;

        case 0x44: /* LD B, H */
            gb->cpu_reg.b = gb->cpu_reg.h;
            break;

        case 0x45: /* LD B, L */
            gb->cpu_reg.b = gb->cpu_reg.l;
            break;

        case 0x46: /* LD B, (HL) */
            gb->cpu_reg.b = __gb_read(gb, gb->cpu_reg.hl);
            break;

        case 0x47: /* LD B, A */
            gb->cpu_reg.b = gb->cpu_reg.a;
            break;

        case 0x48: /* LD C, B */
            gb->cpu_reg.c = gb->cpu_reg.b;
            break;

        case 0x49: /* LD C, C */
            break;

        case 0x4A: /* LD C, D */
            gb->cpu_reg.c = gb->cpu_reg.d;
            break;

        case 0x4B: /* LD C, E */
            gb->cpu_reg.c = gb->cpu_reg.e;
            break;

        case 0x4C: /* LD C, H */
            gb->cpu_reg.c = gb->cpu_reg.h;
            break;

        case 0x4D: /* LD C, L */
            gb->cpu_reg.c = gb->cpu_reg.l;
            break;

        case 0x4E: /* LD C, (HL) */
            gb->cpu_reg.c = __gb_read(gb, gb->cpu_reg.hl);
            break;

        case 0x4F: /* LD C, A */
            gb->cpu_reg.c = gb->cpu_reg.a;
            break;

        case 0x50: /* LD D, B */
            gb->cpu_reg.d = gb->cpu_reg.b;
            break;

        case 0x51: /* LD D, C */
            gb->cpu_reg.d = gb->cpu_reg.c;
            break;

        case 0x52: /* LD D, D */
            break;

        case 0x53: /* LD D, E */
            gb->cpu_reg.d = gb->cpu_reg.e;
            break;

        case 0x54: /* LD D, H */
            gb->cpu_reg.d = gb->cpu_reg.h;
            break;

        case 0x55: /* LD D, L */
            gb->cpu_reg.d = gb->cpu_reg.l;
            break;

        case 0x56: /* LD D, (HL) */
            gb->cpu_reg.d = __gb_read(gb, gb->cpu_reg.hl);
            break;

        case 0x57: /* LD D, A */
            gb->cpu_reg.d = gb->cpu_reg.a;
            break;

        case 0x58: /* LD E, B */
            gb->cpu_reg.e = gb->cpu_reg.b;
            break;

        case 0x59: /* LD E, C */
            gb->cpu_reg.e = gb->cpu_reg.c;
            break;

        case 0x5A: /* LD E, D */
            gb->cpu_reg.e = gb->cpu_reg.d;
            break;

        case 0x5B: /* LD E, E */
            break;

        case 0x5C: /* LD E, H */
            gb->cpu_reg.e = gb->cpu_reg.h;
            break;

        case 0x5D: /* LD E, L */
            gb->cpu_reg.e = gb->cpu_reg.l;
            break;

        case 0x5E: /* LD E, (HL) */
            gb->cpu_reg.e = __gb_read(gb, gb->cpu_reg.hl);
            break;

        case 0x5F: /* LD E, A */
            gb->cpu_reg.e = gb->cpu_reg.a;
            break;

        case 0x60: /* LD H, B */
            gb->cpu_reg.h = gb->cpu_reg.b;
            break;

        case 0x61: /* LD H, C */
            gb->cpu_reg.h = gb->cpu_reg.c;
            break;

        case 0x62: /* LD H, D */
            gb->cpu_reg.h = gb->cpu_reg.d;
            break;

        case 0x63: /* LD H, E */
            gb->cpu_reg.h = gb->cpu_reg.e;
            break;

        case 0x64: /* LD H, H */
            break;

        case 0x65: /* LD H, L */
            gb->cpu_reg.h = gb->cpu_reg.l;
            break;

        case 0x66: /* LD H, (HL) */
            gb->cpu_reg.h = __gb_read(gb, gb->cpu_reg.hl);
            break;

        case 0x67: /* LD H, A */
            gb->cpu_reg.h = gb->cpu_reg.a;
            break;

        case 0x68: /* LD L, B */
            gb->cpu_reg.l = gb->cpu_reg.b;
            break;

        case 0x69: /* LD L, C */
            gb->cpu_reg.l = gb->cpu_reg.c;
            break;

        case 0x6A: /* LD L, D */
            gb->cpu_reg.l = gb->cpu_reg.d;
            break;

        case 0x6B: /* LD L, E */
            gb->cpu_reg.l = gb->cpu_reg.e;
            break;

        case 0x6C: /* LD L, H */
            gb->cpu_reg.l = gb->cpu_reg.h;
            break;

        case 0x6D: /* LD L, L */
            break;

        case 0x6E: /* LD L, (HL) */
            gb->cpu_reg.l = __gb_read(gb, gb->cpu_reg.hl);
            break;

        case 0x6F: /* LD L, A */
            gb->cpu_reg.l = gb->cpu_reg.a;
            break;

        case 0x70: /* LD (HL), B */
            __gb_write(gb, gb->cpu_reg.hl, gb->cpu_reg.b);
            break;

        case 0x71: /* LD (HL), C */
            __gb_write(gb, gb->cpu_reg.hl, gb->cpu_reg.c);
            break;

        case 0x72: /* LD (HL), D */
            __gb_write(gb, gb->cpu_reg.hl, gb->cpu_reg.d);
            break;

        case 0x73: /* LD (HL), E */
            __gb_write(gb, gb->cpu_reg.hl, gb->cpu_reg.e);
            break;

        case 0x74: /* LD (HL), H */
            __gb_write(gb, gb->cpu_reg.hl, gb->cpu_reg.h);
            break;

        case 0x75: /* LD (HL), L */
            __gb_write(gb, gb->cpu_reg.hl, gb->cpu_reg.l);
            break;

        case 0x76: /* HALT */
            /* TODO: Emulate HALT bug? */
            gb->gb_halt = 1;
            break;

        case 0x77: /* LD (HL), A */
            __gb_write(gb, gb->cpu_reg.hl, gb->cpu_reg.a);
            break;

        case 0x78: /* LD A, B */
            gb->cpu_reg.a = gb->cpu_reg.b;
            break;

        case 0x79: /* LD A, C */
            gb->cpu_reg.a = gb->cpu_reg.c;
            break;

        case 0x7A: /* LD A, D */
            gb->cpu_reg.a = gb->cpu_reg.d;
            break;

        case 0x7B: /* LD A, E */
            gb->cpu_reg.a = gb->cpu_reg.e;
            break;

        case 0x7C: /* LD A, H */
            gb->cpu_reg.a = gb->cpu_reg.h;
            break;

        case 0x7D: /* LD A, L */
            gb->cpu_reg.a = gb->cpu_reg.l;
            break;

        case 0x7E: /* LD A, (HL) */
            gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.hl);
            break;

        case 0x7F: /* LD A, A */
            break;

        case 0x80: /* ADD A, B */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.b;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.b ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x81: /* ADD A, C */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.c ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x82: /* ADD A, D */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.d;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.d ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x83: /* ADD A, E */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.e;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.e ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x84: /* ADD A, H */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.h;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.h ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x85: /* ADD A, L */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.l;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.l ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x86: /* ADD A, (HL) */
        {
            uint8_t hl = __gb_read(gb, gb->cpu_reg.hl);
            uint16_t temp = gb->cpu_reg.a + hl;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ hl ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x87: /* ADD A, A */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.a;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = temp & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x88: /* ADC A, B */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.b + gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.b ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x89: /* ADC A, C */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.c + gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.c ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x8A: /* ADC A, D */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.d + gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.d ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x8B: /* ADC A, E */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.e + gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.e ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x8C: /* ADC A, H */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.h + gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.h ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x8D: /* ADC A, L */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.l + gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.l ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x8E: /* ADC A, (HL) */
        {
            uint8_t val = __gb_read(gb, gb->cpu_reg.hl);
            uint16_t temp = gb->cpu_reg.a + val + gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ val ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x8F: /* ADC A, A */
        {
            uint16_t temp = gb->cpu_reg.a + gb->cpu_reg.a + gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            /* TODO: Optimisation here? */
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.a ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x90: /* SUB B */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.b;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.b ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x91: /* SUB C */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.c ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x92: /* SUB D */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.d;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.d ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x93: /* SUB E */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.e;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.e ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x94: /* SUB H */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.h;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.h ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x95: /* SUB L */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.l;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.l ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x96: /* SUB (HL) */
        {
            uint8_t val = __gb_read(gb, gb->cpu_reg.hl);
            uint16_t temp = gb->cpu_reg.a - val;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ val ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x97: /* SUB A */
            gb->cpu_reg.a = 0;
            gb->cpu_reg.f_bits.z = 1;
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0x98: /* SBC A, B */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.b - gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.b ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x99: /* SBC A, C */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.c - gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.c ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x9A: /* SBC A, D */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.d - gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.d ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x9B: /* SBC A, E */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.e - gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.e ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x9C: /* SBC A, H */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.h - gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.h ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x9D: /* SBC A, L */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.l - gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.l ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x9E: /* SBC A, (HL) */
        {
            uint8_t val = __gb_read(gb, gb->cpu_reg.hl);
            uint16_t temp = gb->cpu_reg.a - val - gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ val ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0x9F: /* SBC A, A */
            gb->cpu_reg.a = gb->cpu_reg.f_bits.c ? 0xFF : 0x00;
            gb->cpu_reg.f_bits.z = gb->cpu_reg.f_bits.c ? 0x00 : 0x01;
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = gb->cpu_reg.f_bits.c;
            break;

        case 0xA0: /* AND B */
            gb->cpu_reg.a = gb->cpu_reg.a & gb->cpu_reg.b;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 1;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xA1: /* AND C */
            gb->cpu_reg.a = gb->cpu_reg.a & gb->cpu_reg.c;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 1;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xA2: /* AND D */
            gb->cpu_reg.a = gb->cpu_reg.a & gb->cpu_reg.d;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 1;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xA3: /* AND E */
            gb->cpu_reg.a = gb->cpu_reg.a & gb->cpu_reg.e;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 1;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xA4: /* AND H */
            gb->cpu_reg.a = gb->cpu_reg.a & gb->cpu_reg.h;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 1;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xA5: /* AND L */
            gb->cpu_reg.a = gb->cpu_reg.a & gb->cpu_reg.l;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 1;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xA6: /* AND (HL) */
            gb->cpu_reg.a = gb->cpu_reg.a & __gb_read(gb, gb->cpu_reg.hl);
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 1;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xA7: /* AND A */
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 1;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xA8: /* XOR B */
            gb->cpu_reg.a = gb->cpu_reg.a ^ gb->cpu_reg.b;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xA9: /* XOR C */
            gb->cpu_reg.a = gb->cpu_reg.a ^ gb->cpu_reg.c;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xAA: /* XOR D */
            gb->cpu_reg.a = gb->cpu_reg.a ^ gb->cpu_reg.d;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xAB: /* XOR E */
            gb->cpu_reg.a = gb->cpu_reg.a ^ gb->cpu_reg.e;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xAC: /* XOR H */
            gb->cpu_reg.a = gb->cpu_reg.a ^ gb->cpu_reg.h;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xAD: /* XOR L */
            gb->cpu_reg.a = gb->cpu_reg.a ^ gb->cpu_reg.l;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xAE: /* XOR (HL) */
            gb->cpu_reg.a = gb->cpu_reg.a ^ __gb_read(gb, gb->cpu_reg.hl);
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xAF: /* XOR A */
            gb->cpu_reg.a = 0x00;
            gb->cpu_reg.f_bits.z = 1;
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xB0: /* OR B */
            gb->cpu_reg.a = gb->cpu_reg.a | gb->cpu_reg.b;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xB1: /* OR C */
            gb->cpu_reg.a = gb->cpu_reg.a | gb->cpu_reg.c;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xB2: /* OR D */
            gb->cpu_reg.a = gb->cpu_reg.a | gb->cpu_reg.d;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xB3: /* OR E */
            gb->cpu_reg.a = gb->cpu_reg.a | gb->cpu_reg.e;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xB4: /* OR H */
            gb->cpu_reg.a = gb->cpu_reg.a | gb->cpu_reg.h;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xB5: /* OR L */
            gb->cpu_reg.a = gb->cpu_reg.a | gb->cpu_reg.l;
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xB6: /* OR (HL) */
            gb->cpu_reg.a = gb->cpu_reg.a | __gb_read(gb, gb->cpu_reg.hl);
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xB7: /* OR A */
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xB8: /* CP B */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.b;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.b ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            break;
        }

        case 0xB9: /* CP C */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.c;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.c ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            break;
        }

        case 0xBA: /* CP D */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.d;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.d ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            break;
        }

        case 0xBB: /* CP E */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.e;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.e ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            break;
        }

        case 0xBC: /* CP H */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.h;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.h ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            break;
        }

        case 0xBD: /* CP L */
        {
            uint16_t temp = gb->cpu_reg.a - gb->cpu_reg.l;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ gb->cpu_reg.l ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            break;
        }

        /* TODO: Optimsation by combining similar opcode routines. */
        case 0xBE: /* CP (HL) */
        {
            uint8_t val = __gb_read(gb, gb->cpu_reg.hl);
            uint16_t temp = gb->cpu_reg.a - val;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ val ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            break;
        }

        case 0xBF: /* CP A */
            gb->cpu_reg.f_bits.z = 1;
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xC0: /* RET NZ */
            if (!gb->cpu_reg.f_bits.z) {
                gb->cpu_reg.pc = __gb_read(gb, gb->cpu_reg.sp++);
                gb->cpu_reg.pc |= __gb_read(gb, gb->cpu_reg.sp++) << 8;
                inst_cycles += 12;
            }

            break;

        case 0xC1: /* POP BC */
            gb->cpu_reg.c = __gb_read(gb, gb->cpu_reg.sp++);
            gb->cpu_reg.b = __gb_read(gb, gb->cpu_reg.sp++);
            break;

        case 0xC2: /* JP NZ, imm */
            if (!gb->cpu_reg.f_bits.z) {
                uint16_t temp = __gb_read(gb, gb->cpu_reg.pc++);
                temp |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
                gb->cpu_reg.pc = temp;
                inst_cycles += 4;
            } else
                gb->cpu_reg.pc += 2;

            break;

        case 0xC3: /* JP imm */
        {
            uint16_t temp = __gb_read(gb, gb->cpu_reg.pc++);
            temp |= __gb_read(gb, gb->cpu_reg.pc) << 8;
            gb->cpu_reg.pc = temp;
            break;
        }

        case 0xC4: /* CALL NZ imm */
            if (!gb->cpu_reg.f_bits.z) {
                uint16_t temp = __gb_read(gb, gb->cpu_reg.pc++);
                temp |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
                __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
                __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
                gb->cpu_reg.pc = temp;
                inst_cycles += 12;
            } else
                gb->cpu_reg.pc += 2;

            break;

        case 0xC5: /* PUSH BC */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.b);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.c);
            break;

        case 0xC6: /* ADD A, imm */
        {
            /* Taken from SameBoy, which is released under MIT Licence. */
            uint8_t value = __gb_read(gb, gb->cpu_reg.pc++);
            uint16_t calc = gb->cpu_reg.a + value;
            gb->cpu_reg.f_bits.z = ((uint8_t)calc == 0) ? 1 : 0;
            gb->cpu_reg.f_bits.h =
                ((gb->cpu_reg.a & 0xF) + (value & 0xF) > 0x0F) ? 1 : 0;
            gb->cpu_reg.f_bits.c = calc > 0xFF ? 1 : 0;
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.a = (uint8_t)calc;
            break;
        }

        case 0xC7: /* RST 0x0000 */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
            gb->cpu_reg.pc = 0x0000;
            break;

        case 0xC8: /* RET Z */
            if (gb->cpu_reg.f_bits.z) {
                uint16_t temp = __gb_read(gb, gb->cpu_reg.sp++);
                temp |= __gb_read(gb, gb->cpu_reg.sp++) << 8;
                gb->cpu_reg.pc = temp;
                inst_cycles += 12;
            }

            break;

        case 0xC9: /* RET */
        {
            uint16_t temp = __gb_read(gb, gb->cpu_reg.sp++);
            temp |= __gb_read(gb, gb->cpu_reg.sp++) << 8;
            gb->cpu_reg.pc = temp;
            break;
        }

        case 0xCA: /* JP Z, imm */
            if (gb->cpu_reg.f_bits.z) {
                uint16_t temp = __gb_read(gb, gb->cpu_reg.pc++);
                temp |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
                gb->cpu_reg.pc = temp;
                inst_cycles += 4;
            } else
                gb->cpu_reg.pc += 2;

            break;

        case 0xCB: /* CB INST */
            inst_cycles = __gb_execute_cb(gb);
            break;

        case 0xCC: /* CALL Z, imm */
            if (gb->cpu_reg.f_bits.z) {
                uint16_t temp = __gb_read(gb, gb->cpu_reg.pc++);
                temp |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
                __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
                __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
                gb->cpu_reg.pc = temp;
                inst_cycles += 12;
            } else
                gb->cpu_reg.pc += 2;

            break;

        case 0xCD: /* CALL imm */
        {
            uint16_t addr = __gb_read(gb, gb->cpu_reg.pc++);
            addr |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
            gb->cpu_reg.pc = addr;
        } break;

        case 0xCE: /* ADC A, imm */
        {
            uint8_t value, a, carry;
            value = __gb_read(gb, gb->cpu_reg.pc++);
            a = gb->cpu_reg.a;
            carry = gb->cpu_reg.f_bits.c;
            gb->cpu_reg.a = a + value + carry;

            gb->cpu_reg.f_bits.z = gb->cpu_reg.a == 0 ? 1 : 0;
            gb->cpu_reg.f_bits.h =
                ((a & 0xF) + (value & 0xF) + carry > 0x0F) ? 1 : 0;
            gb->cpu_reg.f_bits.c =
                (((uint16_t)a) + ((uint16_t)value) + carry > 0xFF) ? 1 : 0;
            gb->cpu_reg.f_bits.n = 0;
            break;
        }

        case 0xCF: /* RST 0x0008 */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
            gb->cpu_reg.pc = 0x0008;
            break;

        case 0xD0: /* RET NC */
            if (!gb->cpu_reg.f_bits.c) {
                uint16_t temp = __gb_read(gb, gb->cpu_reg.sp++);
                temp |= __gb_read(gb, gb->cpu_reg.sp++) << 8;
                gb->cpu_reg.pc = temp;
                inst_cycles += 12;
            }

            break;

        case 0xD1: /* POP DE */
            gb->cpu_reg.e = __gb_read(gb, gb->cpu_reg.sp++);
            gb->cpu_reg.d = __gb_read(gb, gb->cpu_reg.sp++);
            break;

        case 0xD2: /* JP NC, imm */
            if (!gb->cpu_reg.f_bits.c) {
                uint16_t temp = __gb_read(gb, gb->cpu_reg.pc++);
                temp |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
                gb->cpu_reg.pc = temp;
                inst_cycles += 4;
            } else
                gb->cpu_reg.pc += 2;

            break;

        case 0xD4: /* CALL NC, imm */
            if (!gb->cpu_reg.f_bits.c) {
                uint16_t temp = __gb_read(gb, gb->cpu_reg.pc++);
                temp |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
                __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
                __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
                gb->cpu_reg.pc = temp;
                inst_cycles += 12;
            } else
                gb->cpu_reg.pc += 2;

            break;

        case 0xD5: /* PUSH DE */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.d);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.e);
            break;

        case 0xD6: /* SUB imm */
        {
            uint8_t val = __gb_read(gb, gb->cpu_reg.pc++);
            uint16_t temp = gb->cpu_reg.a - val;
            gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ val ^ temp) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp & 0xFF);
            break;
        }

        case 0xD7: /* RST 0x0010 */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
            gb->cpu_reg.pc = 0x0010;
            break;

        case 0xD8: /* RET C */
            if (gb->cpu_reg.f_bits.c) {
                uint16_t temp = __gb_read(gb, gb->cpu_reg.sp++);
                temp |= __gb_read(gb, gb->cpu_reg.sp++) << 8;
                gb->cpu_reg.pc = temp;
                inst_cycles += 12;
            }

            break;

        case 0xD9: /* RETI */
        {
            uint16_t temp = __gb_read(gb, gb->cpu_reg.sp++);
            temp |= __gb_read(gb, gb->cpu_reg.sp++) << 8;
            gb->cpu_reg.pc = temp;
            gb->gb_ime = 1;
        } break;

        case 0xDA: /* JP C, imm */
            if (gb->cpu_reg.f_bits.c) {
                uint16_t addr = __gb_read(gb, gb->cpu_reg.pc++);
                addr |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
                gb->cpu_reg.pc = addr;
                inst_cycles += 4;
            } else
                gb->cpu_reg.pc += 2;

            break;

        case 0xDC: /* CALL C, imm */
            if (gb->cpu_reg.f_bits.c) {
                uint16_t temp = __gb_read(gb, gb->cpu_reg.pc++);
                temp |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
                __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
                __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
                gb->cpu_reg.pc = temp;
                inst_cycles += 12;
            } else
                gb->cpu_reg.pc += 2;

            break;

        case 0xDE: /* SBC A, imm */
        {
            uint8_t temp_8 = __gb_read(gb, gb->cpu_reg.pc++);
            uint16_t temp_16 = gb->cpu_reg.a - temp_8 - gb->cpu_reg.f_bits.c;
            gb->cpu_reg.f_bits.z = ((temp_16 & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h =
                (gb->cpu_reg.a ^ temp_8 ^ temp_16) & 0x10 ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp_16 & 0xFF00) ? 1 : 0;
            gb->cpu_reg.a = (temp_16 & 0xFF);
            break;
        }

        case 0xDF: /* RST 0x0018 */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
            gb->cpu_reg.pc = 0x0018;
            break;

        case 0xE0: /* LD (0xFF00+imm), A */
            __gb_write(gb, 0xFF00 | __gb_read(gb, gb->cpu_reg.pc++),
                       gb->cpu_reg.a);
            break;

        case 0xE1: /* POP HL */
            gb->cpu_reg.l = __gb_read(gb, gb->cpu_reg.sp++);
            gb->cpu_reg.h = __gb_read(gb, gb->cpu_reg.sp++);
            break;

        case 0xE2: /* LD (C), A */
            __gb_write(gb, 0xFF00 | gb->cpu_reg.c, gb->cpu_reg.a);
            break;

        case 0xE5: /* PUSH HL */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.h);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.l);
            break;

        case 0xE6: /* AND imm */
            /* TODO: Optimisation? */
            gb->cpu_reg.a = gb->cpu_reg.a & __gb_read(gb, gb->cpu_reg.pc++);
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 1;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xE7: /* RST 0x0020 */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
            gb->cpu_reg.pc = 0x0020;
            break;

        case 0xE8: /* ADD SP, imm */
        {
            int8_t offset = (int8_t)__gb_read(gb, gb->cpu_reg.pc++);
            /* TODO: Move flag assignments for optimisation. */
            gb->cpu_reg.f_bits.z = 0;
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.sp & 0xF) + (offset & 0xF) > 0xF) ? 1 : 0;
            gb->cpu_reg.f_bits.c = ((gb->cpu_reg.sp & 0xFF) + (offset & 0xFF) > 0xFF);
            gb->cpu_reg.sp += offset;
            break;
        }

        case 0xE9: /* JP HL */
            gb->cpu_reg.pc = gb->cpu_reg.hl;
            break;

        case 0xEA: /* LD (imm), A */
        {
            uint16_t addr = __gb_read(gb, gb->cpu_reg.pc++);
            addr |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
            __gb_write(gb, addr, gb->cpu_reg.a);
            break;
        }

        case 0xEE: /* XOR imm */
            gb->cpu_reg.a = gb->cpu_reg.a ^ __gb_read(gb, gb->cpu_reg.pc++);
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xEF: /* RST 0x0028 */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
            gb->cpu_reg.pc = 0x0028;
            break;

        case 0xF0: /* LD A, (0xFF00+imm) */
            gb->cpu_reg.a =
                __gb_read(gb, 0xFF00 | __gb_read(gb, gb->cpu_reg.pc++));
            break;

        case 0xF1: /* POP AF */
        {
            uint8_t temp_8 = __gb_read(gb, gb->cpu_reg.sp++);
            gb->cpu_reg.f_bits.z = (temp_8 >> 7) & 1;
            gb->cpu_reg.f_bits.n = (temp_8 >> 6) & 1;
            gb->cpu_reg.f_bits.h = (temp_8 >> 5) & 1;
            gb->cpu_reg.f_bits.c = (temp_8 >> 4) & 1;
            gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.sp++);
            break;
        }

        case 0xF2: /* LD A, (C) */
            gb->cpu_reg.a = __gb_read(gb, 0xFF00 | gb->cpu_reg.c);
            break;

        case 0xF3: /* DI */
            gb->gb_ime = 0;
            break;

        case 0xF5: /* PUSH AF */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.a);
            __gb_write(gb, --gb->cpu_reg.sp,
                       gb->cpu_reg.f_bits.z << 7 | gb->cpu_reg.f_bits.n << 6 |
                           gb->cpu_reg.f_bits.h << 5 | gb->cpu_reg.f_bits.c << 4);
            break;

        case 0xF6: /* OR imm */
            gb->cpu_reg.a = gb->cpu_reg.a | __gb_read(gb, gb->cpu_reg.pc++);
            gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = 0;
            gb->cpu_reg.f_bits.c = 0;
            break;

        case 0xF7: /* RST 0x0030 */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
            gb->cpu_reg.pc = 0x0030;
            break;

        case 0xF8: /* LD HL, SP+/-imm */
        {
            /* Taken from SameBoy, which is released under MIT Licence. */
            int8_t offset = (int8_t)__gb_read(gb, gb->cpu_reg.pc++);
            gb->cpu_reg.hl = gb->cpu_reg.sp + offset;
            gb->cpu_reg.f_bits.z = 0;
            gb->cpu_reg.f_bits.n = 0;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.sp & 0xF) + (offset & 0xF) > 0xF) ? 1 : 0;
            gb->cpu_reg.f_bits.c = ((gb->cpu_reg.sp & 0xFF) + (offset & 0xFF) > 0xFF) ? 1 : 0;
            break;
        }

        case 0xF9: /* LD SP, HL */
            gb->cpu_reg.sp = gb->cpu_reg.hl;
            break;

        case 0xFA: /* LD A, (imm) */
        {
            uint16_t addr = __gb_read(gb, gb->cpu_reg.pc++);
            addr |= __gb_read(gb, gb->cpu_reg.pc++) << 8;
            gb->cpu_reg.a = __gb_read(gb, addr);
            break;
        }

        case 0xFB: /* EI */
            gb->gb_ime = 1;
            break;

        case 0xFE: /* CP imm */
        {
            uint8_t temp_8 = __gb_read(gb, gb->cpu_reg.pc++);
            uint16_t temp_16 = gb->cpu_reg.a - temp_8;
            gb->cpu_reg.f_bits.z = ((temp_16 & 0xFF) == 0x00);
            gb->cpu_reg.f_bits.n = 1;
            gb->cpu_reg.f_bits.h = ((gb->cpu_reg.a ^ temp_8 ^ temp_16) & 0x10) ? 1 : 0;
            gb->cpu_reg.f_bits.c = (temp_16 & 0xFF00) ? 1 : 0;
            break;
        }

        case 0xFF: /* RST 0x0038 */
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
            __gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);
            gb->cpu_reg.pc = 0x0038;
            break;

        default:
            (gb->gb_error)(gb, GB_INVALID_OPCODE, opcode);
    }

    /* DIV register timing */
    gb->counter.div_count += inst_cycles;

    if (gb->counter.div_count >= DIV_CYCLES) {
        gb->gb_reg.DIV++;
        gb->counter.div_count -= DIV_CYCLES;
    }

    /* Check serial transmission. */
    if (gb->gb_reg.SC & SERIAL_SC_TX_START) {
        /* If new transfer, call TX function. */
        if (gb->counter.serial_count == 0 && gb->gb_serial_tx != NULL)
            (gb->gb_serial_tx)(gb, gb->gb_reg.SB);

        gb->counter.serial_count += inst_cycles;

        /* If it's time to receive byte, call RX function. */
        if (gb->counter.serial_count >= SERIAL_CYCLES) {
            /* If RX can be done, do it. */
            /* If RX failed, do not change SB if using external
             * clock, or set to 0xFF if using internal clock. */
            uint8_t rx;

            if (gb->gb_serial_rx != NULL &&
                (gb->gb_serial_rx(gb, &rx) ==
                 GB_SERIAL_RX_SUCCESS)) {
                gb->gb_reg.SB = rx;

                /* Inform game of serial TX/RX completion. */
                gb->gb_reg.SC &= 0x01;
                gb->gb_reg.IF |= SERIAL_INTR;
            } else if (gb->gb_reg.SC & SERIAL_SC_CLOCK_SRC) {
                /* If using internal clock, and console is not
                 * attached to any external peripheral, shifted
                 * bits are replaced with logic 1. */
                gb->gb_reg.SB = 0xFF;

                /* Inform game of serial TX/RX completion. */
                gb->gb_reg.SC &= 0x01;
                gb->gb_reg.IF |= SERIAL_INTR;
            } else {
                /* If using external clock, and console is not
                 * attached to any external peripheral, bits are
                 * not shifted, so SB is not modified. */
            }

            gb->counter.serial_count = 0;
        }
    }

    /* TIMA register timing */
    /* TODO: Change tac_enable to struct of TAC timer control bits. */
    if (gb->gb_reg.tac_enable) {
        static const uint_fast16_t TAC_CYCLES[4] = {1024, 16, 64, 256};

        gb->counter.tima_count += inst_cycles;

        while (gb->counter.tima_count >= TAC_CYCLES[gb->gb_reg.tac_rate]) {
            gb->counter.tima_count -= TAC_CYCLES[gb->gb_reg.tac_rate];

            if (++gb->gb_reg.TIMA == 0) {
                gb->gb_reg.IF |= TIMER_INTR;
                /* On overflow, set TMA to TIMA. */
                gb->gb_reg.TIMA = gb->gb_reg.TMA;
            }
        }
    }

    /* TODO Check behaviour of LCD during LCD power off state. */
    /* If LCD is off, don't update LCD state. */
    if ((gb->gb_reg.LCDC & LCDC_ENABLE) == 0)
        return;

    /* LCD Timing */
    gb->counter.lcd_count += (inst_cycles >> gb->cgb.doubleSpeed);

    /* New Scanline */
    if (gb->counter.lcd_count > LCD_LINE_CYCLES) {
        gb->counter.lcd_count -= LCD_LINE_CYCLES;

        /* LYC Update */
        if (gb->gb_reg.LY == gb->gb_reg.LYC) {
            gb->gb_reg.STAT |= STAT_LYC_COINC;

            if (gb->gb_reg.STAT & STAT_LYC_INTR)
                gb->gb_reg.IF |= LCDC_INTR;
        } else
            gb->gb_reg.STAT &= 0xFB;

        /* Next line */
        gb->gb_reg.LY = (gb->gb_reg.LY + 1) % LCD_VERT_LINES;

        /* VBLANK Start */
        if (gb->gb_reg.LY == LCD_HEIGHT) {
            gb->lcd_mode = LCD_VBLANK;
            gb->gb_frame = 1;
            gb->gb_reg.IF |= VBLANK_INTR;
            gb->lcd_blank = 0;

            if (gb->gb_reg.STAT & STAT_MODE_1_INTR)
                gb->gb_reg.IF |= LCDC_INTR;

#if ENABLE_LCD

            /* If frame skip is activated, check if we need to draw
             * the frame or skip it. */
            if (gb->direct.frame_skip) {
                gb->display.frame_skip_count =
                    !gb->display.frame_skip_count;
            }

            /* If interlaced is activated, change which lines get
             * updated. Also, only update lines on frames that are
             * actually drawn when frame skip is enabled. */
            if (gb->direct.interlace &&
                (!gb->direct.frame_skip ||
                 gb->display.frame_skip_count)) {
                gb->display.interlace_count =
                    !gb->display.interlace_count;
            }

#endif
        }
        /* Normal Line */
        else if (gb->gb_reg.LY < LCD_HEIGHT) {
            if (gb->gb_reg.LY == 0) {
                /* Clear Screen */
                gb->display.WY = gb->gb_reg.WY;
                gb->display.window_clear = 0;
            }

            gb->lcd_mode = LCD_HBLANK;

            //DMA GBC
            if (gb->cgb.cgbMode && (!gb->cgb.dmaActive) && gb->cgb.dmaMode) {
                for (uint8_t i = 0; i < 0x10; i++) {
                    __gb_write(gb, ((gb->cgb.dmaDest & 0x1FF0) | 0x8000) + i, __gb_read(gb, (gb->cgb.dmaSource & 0xFFF0) + i));
                }
                gb->cgb.dmaSource += 0x10;
                gb->cgb.dmaDest += 0x10;
                if (!(--gb->cgb.dmaSize)) gb->cgb.dmaActive = 1;
            }

            if (gb->gb_reg.STAT & STAT_MODE_0_INTR)
                gb->gb_reg.IF |= LCDC_INTR;
        }
    }
    /* OAM access */
    else if (gb->lcd_mode == LCD_HBLANK && gb->counter.lcd_count >= LCD_MODE_2_CYCLES) {
        gb->lcd_mode = LCD_SEARCH_OAM;

        if (gb->gb_reg.STAT & STAT_MODE_2_INTR)
            gb->gb_reg.IF |= LCDC_INTR;
    }
    /* Update LCD */
    else if (gb->lcd_mode == LCD_SEARCH_OAM && gb->counter.lcd_count >= LCD_MODE_3_CYCLES) {
        gb->lcd_mode = LCD_TRANSFER;
#if ENABLE_LCD
        if (!gb->lcd_blank)
            __gb_draw_line(gb);
#endif
    }
}

void gb_run_frame(struct gb_s *gb) {
    gb->gb_frame = 0;

    while (!gb->gb_frame)
        __gb_step_cpu(gb);
}

/**
 * Gets the size of the save file required for the ROM.
 */
uint_fast32_t gb_get_save_size(struct gb_s *gb) {
    const uint_fast16_t ram_size_location = 0x0149;
    const uint_fast32_t ram_sizes[] =
        {
            0x00, 0x800, 0x2000, 0x8000, 0x20000};
    uint8_t ram_size = gb->gb_rom_read(gb, ram_size_location);
    return ram_sizes[ram_size];
}

/**
 * Set the function used to handle serial transfer in the front-end. This is
 * optional.
 * gb_serial_transfer takes a byte to transmit and returns the received byte. If
 * no cable is connected to the console, return 0xFF.
 */
void gb_init_serial(struct gb_s *gb,
                    void (*gb_serial_tx)(struct gb_s *, const uint8_t),
                    enum gb_serial_rx_ret_e (*gb_serial_rx)(struct gb_s *,
                                                            uint8_t *)) {
    gb->gb_serial_tx = gb_serial_tx;
    gb->gb_serial_rx = gb_serial_rx;
}

uint8_t gb_colour_hash(struct gb_s *gb) {
#define ROM_TITLE_START_ADDR 0x0134
#define ROM_TITLE_END_ADDR 0x0143

    uint8_t x = 0;

    for (uint16_t i = ROM_TITLE_START_ADDR; i <= ROM_TITLE_END_ADDR; i++)
        x += gb->gb_rom_read(gb, i);

    return x;
}

/**
 * Resets the context, and initialises startup values.
 */
void gb_reset(struct gb_s *gb) {
    gb->gb_halt = 0;
    gb->gb_ime = 1;
    gb->gb_bios_enable = 0;
    gb->lcd_mode = LCD_HBLANK;

    /* Initialise MBC values. */
    gb->selected_rom_bank = 1;
    gb->cart_ram_bank = 0;
    gb->enable_cart_ram = 0;
    gb->cart_mode_select = 0;

    /* Initialise CPU registers as though a DMG or CGB. */
    gb->cpu_reg.af = 0x01B0;
    if (gb->cgb.cgbMode) gb->cpu_reg.af = 0x1180;
    gb->cpu_reg.bc = 0x0013;
    if (gb->cgb.cgbMode) gb->cpu_reg.bc = 0x0000;
    gb->cpu_reg.de = 0x00D8;
    if (gb->cgb.cgbMode) gb->cpu_reg.de = 0x0008;
    gb->cpu_reg.hl = 0x014D;
    if (gb->cgb.cgbMode) gb->cpu_reg.hl = 0x007C;
    gb->cpu_reg.sp = 0xFFFE;
    /* TODO: Add BIOS support. */
    gb->cpu_reg.pc = 0x0100;

    gb->counter.lcd_count = 0;
    gb->counter.div_count = 0;
    gb->counter.tima_count = 0;
    gb->counter.serial_count = 0;

    gb->gb_reg.TIMA = 0x00;
    gb->gb_reg.TMA = 0x00;
    gb->gb_reg.TAC = 0xF8;
    gb->gb_reg.DIV = 0xAB;
    if (gb->cgb.cgbMode) gb->gb_reg.DIV = 0xFF;

    gb->gb_reg.IF = 0xE1;

    gb->gb_reg.LCDC = 0x91;
    gb->gb_reg.SCY = 0x00;
    gb->gb_reg.SCX = 0x00;
    gb->gb_reg.LYC = 0x00;

    /* Appease valgrind for invalid reads and unconditional jumps. */
    gb->gb_reg.SC = 0x7E;
    if (gb->cgb.cgbMode) gb->gb_reg.SC = 0x7F;
    gb->gb_reg.STAT = 0x85;
    gb->gb_reg.LY = 0;

    /* Initialize some CGB registers */
    gb->cgb.doubleSpeed = 0;
    gb->cgb.doubleSpeedPrep = 0;
    gb->cgb.wramBank = 1;
    gb->cgb.wramBankOffset = WRAM_0_ADDR;
    gb->cgb.vramBank = 0;
    gb->cgb.vramBankOffset = VRAM_ADDR;
    for (int i = 0; i < 0x20; i++) {
        gb->cgb.OAMPalette[(i << 1)] = gb->cgb.BGPalette[(i << 1)] = 0x7F;
        gb->cgb.OAMPalette[(i << 1) + 1] = gb->cgb.BGPalette[(i << 1) + 1] = 0xFF;
    }
    gb->cgb.OAMPaletteID = 0;
    gb->cgb.BGPaletteID = 0;
    gb->cgb.OAMPaletteInc = 0;
    gb->cgb.BGPaletteInc = 0;
    gb->cgb.dmaActive = 1;  // Not active
    gb->cgb.dmaMode = 0;
    gb->cgb.dmaSize = 0;
    gb->cgb.dmaSource = 0;
    gb->cgb.dmaDest = 0;

    __gb_write(gb, 0xFF47, 0xFC);  // BGP
    __gb_write(gb, 0xFF48, 0xFF);  // OBJP0
    __gb_write(gb, 0xFF49, 0x0F);  // OBJP1
    gb->gb_reg.WY = 0x00;
    gb->gb_reg.WX = 0x00;
    gb->gb_reg.IE = 0x00;

    gb->direct.joypad = 0xFF;
    gb->gb_reg.P1 = 0xCF;

    memset(gb->vram, 0x00, VRAM_SIZE);
}

/**
 * Initialise the emulator context. gb_reset() is also called to initialise
 * the CPU.
 */
enum gb_init_error_e gb_init(struct gb_s *gb,
                             uint8_t (*gb_rom_read)(struct gb_s *, const uint_fast32_t),
                             uint8_t (*gb_cart_ram_read)(struct gb_s *, const uint_fast32_t),
                             void (*gb_cart_ram_write)(struct gb_s *, const uint_fast32_t, const uint8_t),
                             void (*gb_error)(struct gb_s *, const enum gb_error_e, const uint16_t),
                             void *priv) {
    const uint16_t cgb_flag = 0x0143;
    const uint16_t mbc_location = 0x0147;
    const uint16_t bank_count_location = 0x0148;
    const uint16_t ram_size_location = 0x0149;
    /**
     * Table for cartridge type (MBC). -1 if invalid.
     * TODO: MMM01 is untested.
     * TODO: MBC6 is untested.
     * TODO: MBC7 is unsupported.
     * TODO: POCKET CAMERA is unsupported.
     * TODO: BANDAI TAMA5 is unsupported.
     * TODO: HuC3 is unsupported.
     * TODO: HuC1 is unsupported.
     **/
    const uint8_t cart_mbc[] =
        {
            0, 1, 1, 1, -1, 2, 2, -1, 0, 0, -1, 0, 0, 0, -1, 3,
            3, 3, 3, 3, -1, -1, -1, -1, -1, 5, 5, 5, 5, 5, 5, -1};
    const uint8_t cart_ram[] =
        {
            0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
            1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0};
    const uint16_t num_rom_banks_mask[] =
        {
            2, 4, 8, 16, 32, 64, 128, 256, 512, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 72, 80, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    const uint8_t num_ram_banks[] = {0, 1, 1, 4, 16, 8};

    gb->gb_rom_read = gb_rom_read;
    gb->gb_cart_ram_read = gb_cart_ram_read;
    gb->gb_cart_ram_write = gb_cart_ram_write;
    gb->gb_error = gb_error;
    gb->direct.priv = priv;

    /* Initialise serial transfer function to NULL. If the front-end does
     * not provide serial support, Peanut-GB will emulate no cable connected
     * automatically. */
    gb->gb_serial_tx = NULL;
    gb->gb_serial_rx = NULL;

    /* Check valid ROM using checksum value. */
    {
        uint8_t x = 0;

        for (uint16_t i = 0x0134; i <= 0x014C; i++)
            x = x - gb->gb_rom_read(gb, i) - 1;

        if (x != gb->gb_rom_read(gb, ROM_HEADER_CHECKSUM_LOC))
            return GB_INIT_INVALID_CHECKSUM;
    }

    /* Check if cartridge type is supported, and set MBC type. */
    {
        gb->cgb.cgbMode = (gb->gb_rom_read(gb, cgb_flag) & 0x80) >> 7;
        const uint8_t mbc_value = gb->gb_rom_read(gb, mbc_location);

        if (mbc_value > sizeof(cart_mbc) - 1 ||
            (gb->mbc = cart_mbc[mbc_value]) == 255u)
            return GB_INIT_CARTRIDGE_UNSUPPORTED;
    }

    gb->cart_ram = cart_ram[gb->gb_rom_read(gb, mbc_location)];
    gb->num_rom_banks_mask = num_rom_banks_mask[gb->gb_rom_read(gb, bank_count_location)] - 1;
    gb->num_ram_banks = num_ram_banks[gb->gb_rom_read(gb, ram_size_location)];

    gb->lcd_blank = 0;
    gb->display.lcd_draw_line = NULL;

    gb_reset(gb);

    return GB_INIT_NO_ERROR;
}

/**
 * Returns the title of ROM.
 *
 * \param gb        Initialised context.
 * \param title_str    Allocated string at least 16 characters.
 * \returns        Pointer to start of string, null terminated.
 */
const char *gb_get_rom_name(struct gb_s *gb, char *title_str) {
    uint_fast16_t title_loc = 0x134;
    /* End of title may be 0x13E for newer games. */
    const uint_fast16_t title_end = 0x143;
    const char *title_start = title_str;

    for (; title_loc <= title_end; title_loc++) {
        const char title_char = gb->gb_rom_read(gb, title_loc);

        if (title_char >= ' ' && title_char <= '_') {
            *title_str = title_char;
            title_str++;
        } else
            break;
    }

    *title_str = '\0';
    return title_start;
}

#if ENABLE_LCD
void gb_init_lcd(struct gb_s *gb,
                 void (*lcd_draw_line)(struct gb_s *gb,
                                       const uint8_t *pixels,
                                       const uint_fast8_t line)) {
    gb->display.lcd_draw_line = lcd_draw_line;

    gb->direct.interlace = 0;
    gb->display.interlace_count = 0;
    gb->direct.frame_skip = 0;
    gb->display.frame_skip_count = 0;

    gb->display.window_clear = 0;
    gb->display.WY = 0;

    return;
}
#endif
