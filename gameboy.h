#include <stdint.h>

/* Interrupt masks */
#define VBLANK_INTR		0x01
#define LCDC_INTR		0x02
#define TIMER_INTR		0x04
#define SERIAL_INTR		0x08
#define CONTROL_INTR	0x10
#define ANY_INTR		0x1F

/* Memory section sizes for DMG */
#define WRAM_SIZE		0x2000
#define VRAM_SIZE		0x2000
#define HRAM_SIZE		0x0100
#define OAM_SIZE		0x00A0

/* Memory addresses */
#define ROM_0_ADDR      0x0000
#define ROM_N_ADDR      0x4000
#define VRAM_ADDR       0x8000
#define CART_RAM_ADDR   0xA000
#define WRAM_0_ADDR     0xC000
#define WRAM_1_ADDR     0xD000
#define ECHO_ADDR       0xE000
#define OAM_ADDR        0xFE00
#define UNUSED_ADDR     0xFEA0
#define IO_ADDR         0xFF00
#define HRAM_ADDR       0xFF80 /* TODO: Check this. */
#define INTR_EN_ADDR    0xFFFF

/* Cart section sizes */
#define ROM_BANK_SIZE   0x4000
#define WRAM_BANK_SIZE  0x1000
#define CRAM_BANK_SIZE  0x2000
#define VRAM_BANK_SIZE  0x2000

/* TAC masks */
#define TAC_ENABLE          0x04
#define TAC_RATE            0x03

/* STAT register masks */
#define STAT_LYC_INTR       0x40
#define STAT_MODE_2_INTR    0x20
#define STAT_MODE_1_INTR    0x10
#define STAT_MODE_0_INTR    0x08
#define STAT_LYC_COINC      0x04
#define STAT_MODE           0x03
#define STAT_USER_BITS      0xF8

/* LCDC control masks */
#define LCDC_ENABLE         0x80
#define LCDC_WINDOW_MAP     0x40
#define LCDC_WINDOW_ENABLE  0x20
#define LCDC_TILE_SELECT    0x10
#define LCDC_BG_MAP         0x08
#define LCDC_OBJ_SIZE       0x04
#define LCDC_OBJ_ENABLE     0x02
#define LCDC_BG_ENABLE      0x01

/* Interrupt jump addresses */
#define VBLANK_INTR_ADDR    0x0040
#define LCDC_INTR_ADDR      0x0048
#define TIMER_INTR_ADDR     0x0050
#define SERIAL_INTR_ADDR    0x0058
#define CONTROL_INTR_ADDR   0x0060

struct cpu_registers_t
{
	struct {
		/* Combine A and F registers. */
		union {
			struct {
				/* Define specific bits of Flag register. */
				union {
					struct {
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
	};

	struct {
		union {
			struct {
				uint8_t c;
				uint8_t b;
			};
			uint16_t bc;
		};
	};

	struct {
		union {
			struct {
				uint8_t e;
				uint8_t d;
			};
			uint16_t de;
		};
	};

	struct {
		union {
			struct {
				unsigned char l;
				unsigned char h;
			};
			unsigned short hl;
		};
	};

	unsigned short sp; /* Stack pointer */
	unsigned short pc; /* Program counter */
};

struct timer_t
{
	//uint16_t cpu_count; /* Unused currently */
	uint16_t lcd_count;		/* LCD Timing */
	uint16_t div_count;		/* Divider Register Counter */
	uint16_t tima_count;	/* Timer Counter */
	uint8_t tac_enable;		/* Timer enable */
	uint8_t tac_rate;		/* Timer rate */
};

struct gb_registers_t
{
	/* TODO: Sort variables in address order. */
	/* Timing */
	uint8_t TIMA;	uint8_t TMA;	uint8_t TAC;	uint8_t DIV;
	/* Sound */
	uint8_t NR10;	uint8_t NR11;	uint8_t NR12;	uint8_t NR13;
	uint8_t NR14;
	uint8_t NR21;	uint8_t NR22;	uint8_t NR24;
	uint8_t NR30;	uint8_t NR31;	uint8_t NR32;	uint8_t NR33;
	uint8_t NR41;	uint8_t NR42;	uint8_t NR43;	uint8_t NR44;
	uint8_t NR50;	uint8_t NR51;	uint8_t NR52;
	/* LCD */
	uint8_t LCDC;	uint8_t STAT;	uint8_t SCY;	uint8_t SCX;
	uint8_t LY;		uint8_t LYC;	uint8_t DMA;	uint8_t BGP;
	uint8_t OBP0;	uint8_t OBP1;	uint8_t WY;		uint8_t WX;
	
	/* Joypad info. */
	uint8_t P1;

	/* Serial data. */
	uint8_t SB;		uint8_t SC;
	
	/* Interrupt flag. */
	uint8_t IF;

	/* Interrupt enable. */
	uint8_t IE;
};

struct gb_t
{
	uint8_t (*gb_rom_read)(struct gb_t*, const uint32_t);
	uint8_t (*gb_cart_ram_read)(struct gb_t*, const uint32_t);
	void (*gb_cart_ram_write)(struct gb_t*, const uint32_t, const uint8_t val);
	void *priv;

	struct
	{
		unsigned int	gb_halt : 1;
		unsigned int	gb_ime : 1;
		unsigned int	gb_bios_enable : 1;
		unsigned int	gb_frame : 1; /* New frame drawn. */
		enum
		{
				LCD_HBLANK = 0,
				LCD_VBLANK = 1,
				LCD_SEARCH_OAM = 2,
				LCD_TRANSFER = 3
		}				lcd_mode : 2;
	};

	/* Cartridge information:
	 * Memory Bank Controller (MBC) type. */
	uint8_t mbc;
	/* Whether the MBC has internal RAM. */
	uint8_t cart_ram;
	/* Number of ROM banks in cartridge. */
	uint8_t num_rom_banks;
	/* Number of RAM banks in cartridge. */
	uint8_t num_ram_banks;

	uint8_t selected_rom_bank;
	/* WRAM and VRAM bank selection not available. */
	uint8_t selected_cart_bank;
	uint8_t enable_cart_bank;
	/* Cartridge ROM/RAM mode select. */
	uint8_t cart_mode_select;
	uint8_t cart_rtc[5];

	struct cpu_registers_t cpu_reg;
	struct timer_t timer;
	struct gb_registers_t gb_reg;

	uint8_t wram[WRAM_SIZE];
	uint8_t vram[VRAM_SIZE];
	uint8_t hram[HRAM_SIZE];
	uint8_t oam[OAM_SIZE];
};

/**
 * Initialises startup values.
 */
void __gb_power_on(struct gb_t *gb)
{
	gb->gb_halt = 0;
	gb->gb_ime = 1;
	gb->gb_bios_enable = 1;
	gb->lcd_mode = LCD_HBLANK;

	/* Initialise MBC values. */
	gb->selected_rom_bank = 1;
	gb->selected_cart_bank = 0;
	gb->enable_cart_bank = 0;
	gb->cart_mode_select = 0;

	/* Initialise CPU registers as though a DMG. */
	gb->cpu_reg.af = 0x01B0;
	gb->cpu_reg.bc = 0x0013;
	gb->cpu_reg.de = 0x00D8;
	gb->cpu_reg.hl = 0x014D;
	gb->cpu_reg.sp = 0xFFFE;
	/* TODO: Add BIOS support. */
	gb->cpu_reg.pc = 0x0100;

	//gb->timer.cpu_count = 0; TODO
	gb->timer.lcd_count = 0;
	gb->timer.div_count = 0;
	gb->timer.tima_count = 0;
	gb->timer.tac_enable = 0;
	gb->timer.tac_rate = 0;

	/* TODO: Set to correct values instead of zero. */
	memset(&gb->gb_reg, 0, sizeof(struct gb_registers_t));
}

void __gb_init_rom_type(struct gb_t *gb)
{
	const unsigned int mbc_location = 0x0147;
	const unsigned int bank_count_location = 0x0148;
	const unsigned int ram_size_location = 0x0149;
	const uint8_t cart_mbc[] = {
		0, 1, 1, 1,-1, 2, 2,-1, 0, 0,-1, 0, 0, 0,-1, 3,
		3, 3, 3, 3,-1,-1,-1,-1,-1, 5, 5, 5, 5, 5, 5, 0
	};
	const uint8_t cart_ram[] = {
		0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0
	};
	const uint8_t num_rom_banks[] = {
		2, 4, 8,16,32,64,128, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0,72,80,96, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	const uint8_t num_ram_banks[] = { 0, 1, 1, 4, 16 };

	gb->mbc = cart_mbc[gb->gb_rom_read(gb, mbc_location)];
	gb->cart_ram = cart_ram[gb->gb_rom_read(gb, mbc_location)];
	gb->num_rom_banks = num_rom_banks[gb->gb_rom_read(gb, bank_count_location)];
	gb->num_ram_banks = num_ram_banks[gb->gb_rom_read(gb, ram_size_location)];
}

uint8_t __gb_read(struct gb_t *gb, const uint16_t addr)
{
	switch ((addr >> 12) & 0xF)
	{
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
			if(gb->mbc == 1 && gb->cart_mode_select)
				return gb->gb_rom_read(gb, addr + ((gb->selected_rom_bank & 0x1F) - 1) * ROM_BANK_SIZE);
			else
				return gb->gb_rom_read(gb, addr + (gb->selected_rom_bank - 1) * ROM_BANK_SIZE);

		case 0x8:
		case 0x9:
			return gb->vram[addr - VRAM_ADDR];

		case 0xA:
		case 0xB:
			if(gb->cart_ram && gb->enable_cart_bank)
			{
				if(gb->mbc == 3 && gb->selected_cart_bank >= 0x08)
					return gb->cart_rtc[gb->selected_cart_bank - 0x08];
				else if((gb->cart_mode_select || gb->mbc != 1) && gb->selected_cart_bank < gb->num_ram_banks)
					return gb->gb_cart_ram_read(gb, addr - CART_RAM_ADDR + gb->selected_cart_bank * CRAM_BANK_SIZE);
				else
					return gb->gb_cart_ram_read(gb, addr - CART_RAM_ADDR);
			}
			return 0;

		case 0xC:
			return gb->wram[addr - WRAM_0_ADDR];

		case 0xD:
			return gb->wram[addr - WRAM_1_ADDR + WRAM_BANK_SIZE];

		case 0xE:
			return gb->wram[addr - ECHO_ADDR];

		case 0xF:
			if(addr < OAM_ADDR)
				return gb->wram[addr - ECHO_ADDR];
			if(addr < UNUSED_ADDR)
				return gb->oam[addr - OAM_ADDR];
			/* Unusable memory area. */
			if(addr < IO_ADDR)
				return 0;
			/* IO, HRAM, and Interrupts. */
			switch (addr & 0xFF)
			{
				/* IO Registers */
				case 0x00: return 0xC0 | gb->gb_reg.P1;
				case 0x01: return gb->gb_reg.SB;
				case 0x02: return gb->gb_reg.SC;

				/* Timer Registers */
				case 0x04: return gb->gb_reg.DIV;
				case 0x05: return gb->gb_reg.TIMA;
				case 0x06: return gb->gb_reg.TMA;
				case 0x07: return gb->gb_reg.TAC;

				/* Interrupt Flag Register */
				case 0x0F: return gb->gb_reg.IF;

				/* Sound registers */
				case 0x10: return gb->gb_reg.NR10;
				case 0x11: return gb->gb_reg.NR11;
				case 0x12: return gb->gb_reg.NR12;
				case 0x13: return gb->gb_reg.NR13;
				case 0x14: return gb->gb_reg.NR14;
				case 0x16: return gb->gb_reg.NR21;
				case 0x17: return gb->gb_reg.NR22;
				case 0x19: return gb->gb_reg.NR24;
				case 0x1A: return gb->gb_reg.NR30;
				case 0x1B: return gb->gb_reg.NR31;
				case 0x1C: return gb->gb_reg.NR32;
				case 0x1E: return gb->gb_reg.NR33;
				case 0x20: return gb->gb_reg.NR41;
				case 0x21: return gb->gb_reg.NR42;
				case 0x22: return gb->gb_reg.NR43;
				case 0x23: return gb->gb_reg.NR44;
				case 0x24: return gb->gb_reg.NR50;
				case 0x25: return gb->gb_reg.NR51;
				case 0x26: return gb->gb_reg.NR52;

				/* LCD Registers */
				case 0x40: return gb->gb_reg.LCDC;
				case 0x41:
						   return (gb->gb_reg.STAT & STAT_USER_BITS) |
							   (gb->gb_reg.LCDC & LCDC_ENABLE ? gb->lcd_mode : LCD_VBLANK);
				case 0x42: return gb->gb_reg.SCY;
				case 0x43: return gb->gb_reg.SCX;
				case 0x44: return gb->gb_reg.LY;
				case 0x45: return gb->gb_reg.LYC;

				/* DMA Register */
				case 0x46: return gb->gb_reg.DMA;

				/* DMG Palette Registers */
				case 0x47: return gb->gb_reg.BGP;
				case 0x48: return gb->gb_reg.OBP0;
				case 0x49: return gb->gb_reg.OBP1;

				/* Window Position Registers */
				case 0x4A: return gb->gb_reg.WY;
				case 0x4B: return gb->gb_reg.WX;

				/* Interrupt Enable Register */
				case 0xFF: return gb->gb_reg.IE;

				/* HRAM */
				default:
					if(addr >= HRAM_ADDR)
						return gb->hram[addr - HRAM_ADDR];
			}
	}

	return 0;
}

void __gb_write(struct gb_t *gb, const uint16_t addr, const uint8_t val)
{
	switch(addr >> 12)
	{
		case 0x0:
		case 0x1:
			if(gb->mbc == 2 && addr & 0x10)
				return;
			else if(gb->mbc > 0 && gb->cart_ram)
				gb->enable_cart_bank = ((val & 0x0F) == 0x0A);
			return;

		case 0x2:
			if(gb->mbc == 5)
			{
				gb->selected_rom_bank = (gb->selected_rom_bank & 0x100) | val;
				gb->selected_rom_bank =
					gb->selected_rom_bank % gb->num_rom_banks;
				return;
			}
			/* TODO: Check if this should continue to next case. */

		case 0x3:
			if(gb->mbc == 1)
			{
				//selected_rom_bank = val & 0x7;
				gb->selected_rom_bank = (val & 0x1F) | (gb->selected_rom_bank & 0x60);
				if ((gb->selected_rom_bank & 0x1F) == 0x00)
					gb->selected_rom_bank++;
			}
			else if(gb->mbc == 2 && addr & 0x10)
			{
				gb->selected_rom_bank = val & 0x0F;
				if (!gb->selected_rom_bank)
					gb->selected_rom_bank++;
			}
			else if(gb->mbc == 3)
			{
				gb->selected_rom_bank = val & 0x7F;
				if (!gb->selected_rom_bank)
					gb->selected_rom_bank++;
			}
			else if(gb->mbc == 5)
				gb->selected_rom_bank = (val & 0x01) << 8 | (gb->selected_rom_bank & 0xFF);

			gb->selected_rom_bank = gb->selected_rom_bank % gb->num_rom_banks;
			return;

		case 0x4:
		case 0x5:
			if(gb->mbc == 1)
			{
				gb->selected_cart_bank = (val & 3);
				gb->selected_rom_bank = ((val & 3) << 5) | (gb->selected_rom_bank & 0x1F);
				gb->selected_rom_bank = gb->selected_rom_bank % gb->num_rom_banks;
			}
			else if(gb->mbc == 3)
				gb->selected_cart_bank = val;
			else if(gb->mbc == 5)
				gb->selected_cart_bank = (val & 0x0F);
			return;

		case 0x6:
		case 0x7:
			gb->cart_mode_select = (val & 1);
			return;
		
		case 0x8:
		case 0x9:
			gb->vram[addr - VRAM_ADDR] = val;
			return;
			
		case 0xA:
		case 0xB:
			if(gb->cart_ram && gb->enable_cart_bank)
			{
				if(gb->mbc == 3 && gb->selected_cart_bank >= 0x08)
					gb->cart_rtc[gb->selected_cart_bank - 0x08] = val;
				else if(gb->cart_mode_select &&
						gb->selected_cart_bank < gb->num_ram_banks)
				{
					gb->gb_cart_ram_write(gb, addr - CART_RAM_ADDR + gb->selected_cart_bank * CRAM_BANK_SIZE, val);
				}
				else
					gb->gb_cart_ram_write(gb, addr - CART_RAM_ADDR, val);
			}
			return;

		case 0xC:
			gb->wram[addr - WRAM_0_ADDR] = val;
			return;

		case 0xD:
			gb->wram[addr - WRAM_1_ADDR] = val;
			return;

		case 0xE:
			gb->wram[addr - ECHO_ADDR] = val;
			return;
		
		case 0xF:
			if(addr < OAM_ADDR)
			{
				gb->wram[addr - ECHO_ADDR] = val;
				return;
			}
			if(addr < UNUSED_ADDR)
			{
				gb->oam[addr - OAM_ADDR] = val;
				return;
			}
			/* Unusable memory area. */
			if(addr < IO_ADDR)
				return;
			/* IO, HRAM, and Interrupts. */
			switch(addr & 0xFF)
			{
				/* IO Registers */
				case 0x00:
					gb->gb_reg.P1 = val & 0x30;
					/* TODO: Controls. */
					return;
				case 0x01: gb->gb_reg.SB = val;		return;
				case 0x02: gb->gb_reg.SC = val;		return;

				/* Timer Registers */
				case 0x04: gb->gb_reg.DIV = 0x00;	return;
				case 0x05: gb->gb_reg.TIMA = val;	return;
				case 0x06: gb->gb_reg.TMA = val;	return;
				case 0x07:
					gb->gb_reg.TAC = val;
					gb->timer.tac_enable = gb->gb_reg.TAC & TAC_ENABLE;
					gb->timer.tac_rate = gb->gb_reg.TAC & TAC_RATE;
					return;

				/* Interrupt Flag Register */
				case 0x0F:
					gb->gb_reg.IF = val;
					return;
				
				/* Sound registers */
				case 0x10: gb->gb_reg.NR10 = val;	return;
				case 0x11: gb->gb_reg.NR11 = val;	return;
				case 0x12: gb->gb_reg.NR12 = val;	return;
				case 0x13: gb->gb_reg.NR13 = val;	return;
				case 0x14: gb->gb_reg.NR14 = val;	return;
				case 0x16: gb->gb_reg.NR21 = val;	return;
				case 0x17: gb->gb_reg.NR22 = val;	return;
				case 0x19: gb->gb_reg.NR24 = val;	return;
				case 0x1A: gb->gb_reg.NR30 = val;	return;
				case 0x1B: gb->gb_reg.NR31 = val;	return;
				case 0x1C: gb->gb_reg.NR32 = val;	return;
				case 0x1E: gb->gb_reg.NR33 = val;	return;
				case 0x20: gb->gb_reg.NR41 = val;	return;
				case 0x21: gb->gb_reg.NR42 = val;	return;
				case 0x22: gb->gb_reg.NR43 = val;	return;
				case 0x23: gb->gb_reg.NR44 = val;	return;
				case 0x24: gb->gb_reg.NR50 = val;	return;
				case 0x25: gb->gb_reg.NR51 = val;	return;
				case 0x26: gb->gb_reg.NR52 = val;	return;

				/* LCD Registers */
				case 0x40: gb->gb_reg.LCDC = val;	return;
				case 0x41: gb->gb_reg.STAT = val;	return;
				case 0x42: gb->gb_reg.SCY = val;	return;
				case 0x43: gb->gb_reg.SCX = val;	return;
				case 0x44: gb->gb_reg.LY = val;		return;
				case 0x45: gb->gb_reg.LYC = val;	return;

				/* DMA Register */
				case 0x46:
					gb->gb_reg.DMA = (val % 0xF1);
					/* TODO: Check the 8 bit shift. */
					for (uint8_t i = 0; i < OAM_SIZE; i++)
						gb->oam[i] = __gb_read(gb, (gb->gb_reg.DMA << 8) + i);

					return;
				
				/* DMG Palette Registers */
				case 0x47: gb->gb_reg.BGP = val;	return;
				case 0x48: gb->gb_reg.OBP0 = val;	return;
				case 0x49: gb->gb_reg.OBP1 = val;	return;

				/* Window Position Registers */
				case 0x4A: gb->gb_reg.WY = val;		return;
				case 0x4B: gb->gb_reg.WX = val;		return;

				/* Turn off boot ROM */
				case 0x50: gb->gb_bios_enable = 0;	return;

				/* Interrupt Enable Register */
				case 0xFF: gb->gb_reg.IE = val;		return;

				/* HRAM */
				default:
					if(addr >= HRAM_ADDR)
						gb->hram[addr - HRAM_ADDR] = val;
					
					return;
			}
	}
}

void __gb_step_cpu(struct gb_t *gb)
{
	uint8_t opcode, inst_cycles;
	const uint8_t op_cycles[0x100] = {
	/*  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F	*/
		4,12, 8, 8, 4, 4, 8, 4,20, 8, 8, 8, 4, 4, 8, 4,		/* 0x00 */
		4,12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4,		/* 0x10 */
		8,12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4,		/* 0x20 */
		8,12, 8, 8,12,12,12, 4, 8, 8, 8, 8, 4, 4, 8, 4,		/* 0x30 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,		/* 0x40 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,		/* 0x50 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,		/* 0x60 */
		8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4,		/* 0x70 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,		/* 0x80 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,		/* 0x90 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,		/* 0xA0 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,		/* 0xB0 */
		8,12,12,12,12,16, 8,32, 8, 8,12, 8,12,12, 8,32,		/* 0xC0 */
		8,12,12, 0,12,16, 8,32, 8, 8,12, 0,12, 0, 8,32,		/* 0xD0 */
		12,12,8, 0, 0,16, 8,32,16, 4,16, 0, 0, 0, 8,32,		/* 0xE0 */
		12,12,8, 4, 0,16, 8,32,12, 8,16, 4, 0, 0, 8,32		/* 0xF0 */
	};

	/* Handle interrupts */
	if((gb->gb_ime || gb->gb_halt) &&
		(gb->gb_reg.IF & gb->gb_reg.IE & ANY_INTR))
	{
		gb->gb_halt = 0;

		if(gb->gb_ime)
		{
			/* Disable interrupts */
			gb->gb_ime = 0;

			/* Push Program Counter */
			__gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc >> 8);
			__gb_write(gb, --gb->cpu_reg.sp, gb->cpu_reg.pc & 0xFF);

			/* Call interrupt handler if required. */
			if(gb->gb_reg.IF & gb->gb_reg.IE & VBLANK_INTR)
			{
				gb->cpu_reg.pc = VBLANK_INTR_ADDR;
				gb->gb_reg.IF ^= VBLANK_INTR;
			}
			else if(gb->gb_reg.IF & gb->gb_reg.IE & LCDC_INTR)
			{
				gb->cpu_reg.pc = LCDC_INTR_ADDR;
				gb->gb_reg.IF ^= LCDC_INTR;
			}
			else if(gb->gb_reg.IF & gb->gb_reg.IE & TIMER_INTR)
			{
				gb->cpu_reg.pc = TIMER_INTR_ADDR;
				gb->gb_reg.IF ^= TIMER_INTR;
			}
			else if(gb->gb_reg.IF & gb->gb_reg.IE & SERIAL_INTR)
			{
				gb->cpu_reg.pc = SERIAL_INTR_ADDR;
				gb->gb_reg.IF ^= SERIAL_INTR;
			}
			else if(gb->gb_reg.IF & gb->gb_reg.IE & CONTROL_INTR)
			{
				gb->cpu_reg.pc = CONTROL_INTR_ADDR;
				gb->gb_reg.IF ^= CONTROL_INTR;
			}
		}
	}

	/* Obtain opcode */
	opcode = (gb->gb_halt ? 0x00 : __gb_read(gb, gb->cpu_reg.pc++));
	inst_cycles = op_cycles[opcode];

	/* Execute opcode */
	switch(opcode)
	{
		case 0x00: /* NOP */
			break;
	}
}

void gb_run_frame(struct gb_t *gb)
{
	gb->gb_frame = 0;
	while(!gb->gb_frame)
		__gb_step_cpu(gb);
}

/**
 * Gets the size of the save file required for the ROM.
 */
uint32_t gb_get_save_size(struct gb_t *gb)
{
	const unsigned int ram_size_location = 0x0149;
	const uint32_t ram_sizes[] = {
		0x00, 0x800, 0x2000, 0x8000, 0x20000
	};
	uint32_t ram_size = gb->gb_rom_read(gb, ram_size_location);
	return ram_sizes[ram_size];
}

struct gb_t gb_init(uint8_t (*gb_rom_read)(struct gb_t*, const uint32_t),
	uint8_t (*gb_cart_ram_read)(struct gb_t*, const uint32_t),
	void (*gb_cart_ram_write)(struct gb_t*, const uint32_t, const uint8_t val),
	void *priv)
{
	struct gb_t gb;
	gb.gb_rom_read = gb_rom_read;
	gb.gb_cart_ram_read = gb_cart_ram_read;
	gb.gb_cart_ram_write = gb_cart_ram_write;
	gb.priv = priv;

	__gb_init_rom_type(&gb);
	__gb_power_on(&gb);
	return gb;
}
