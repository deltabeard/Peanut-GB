#include <stdint.h>

enum lcd_mode_e
{
	lcd_hblank,
	lcd_vblank,
	lcd_search_oam,
	lcd_transfer
};

struct gb_t
{
	uint8_t (*gb_rom_read)(const uint32_t, struct gb_t*);
	void *priv;

	struct
	{
		unsigned int gb_halt : 1;
		unsigned int gb_ime : 1;
		unsigned int gb_bios_enable : 1;
		enum lcd_mode_e lcd_mode : 2;
	};
};

/**
 * Initialises startup values.
 */
void gb_power_on(struct gb_t *gb)
{
	gb->gb_halt = 0;
	gb->gb_ime = 1;
	gb->gb_bios_enable = 1;
	gb->lcd_mode = 0;
}

struct gb_t gb_init(uint8_t (*gb_rom_read)(uint32_t, struct gb_t*), void *priv)
{
	struct gb_t gb;
	gb.gb_rom_read = gb_rom_read;
	gb.priv = priv;
	gb_power_on(&gb);
	return gb;
}