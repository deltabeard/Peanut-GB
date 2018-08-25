#include <stdint.h>

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

struct gb_sound_registers_t
{
	uint8_t NR10;	uint8_t NR11;	uint8_t NR12;	uint8_t NR14;
	uint8_t NR21;	uint8_t NR22;	uint8_t NR24;
	uint8_t NR30;	uint8_t NR31;	uint8_t NR32;	uint8_t NR33;
	uint8_t NR41;	uint8_t NR42;	uint8_t NR43;	uint8_t NR44;
	uint8_t NR50;	uint8_t NR51;	uint8_t NR52;
};

struct gb_registers_t
{
	/* TODO: Sort variables in address order. */
	/* Timing */
	uint8_t TIMA;	uint8_t TMA;	uint8_t TAC;	uint8_t DIV;
	/* Sound */
	struct gb_sound_registers_t gb_snd_reg;
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
	uint8_t (*gb_rom_read)(const uint32_t, struct gb_t*);
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
	uint8_t cartridge_mode_select;

	struct cpu_registers_t cpu_reg;
	struct timer_t timer;
	struct gb_registers_t gb_reg;
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

	gb->mbc = cart_mbc[gb->gb_rom_read(mbc_location, gb)];
	gb->cart_ram = cart_ram[gb->gb_rom_read(mbc_location, gb)];
	gb->num_rom_banks = num_rom_banks[gb->gb_rom_read(bank_count_location, gb)];
	gb->num_ram_banks = num_ram_banks[gb->gb_rom_read(ram_size_location, gb)];
}

void __gb_step_cpu(struct gb_t *gb)
{
	return;
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
	uint32_t ram_size = gb->gb_rom_read(ram_size_location, gb);
	return ram_sizes[ram_size];
}

struct gb_t gb_init(uint8_t (*gb_rom_read)(uint32_t, struct gb_t*), void *priv)
{
	struct gb_t gb;
	gb.gb_rom_read = gb_rom_read;
	gb.priv = priv;

	__gb_init_rom_type(&gb);
	__gb_power_on(&gb);
	return gb;
}
