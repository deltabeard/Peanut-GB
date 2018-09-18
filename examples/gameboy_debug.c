#include <errno.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../gameboy.h"

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;
};

/**
 * Returns an byte from the ROM file at the given address.
 */
uint8_t gb_rom_read(struct gb_t *gb, const uint32_t addr)
{
    const struct priv_t * const p = gb->priv;
    return p->rom[addr];
}

uint8_t gb_cart_ram_read(struct gb_t *gb, const uint32_t addr)
{
	const struct priv_t * const p = gb->priv;
	return p->cart_ram[addr];
}

void gb_cart_ram_write(struct gb_t *gb, const uint32_t addr,
	const uint8_t val)
{
	const struct priv_t * const p = gb->priv;
	p->cart_ram[addr] = val;
}

void gb_error(struct gb_t *gb, const enum gb_error_e gb_err)
{
	switch(gb_err)
	{
		case GB_INVALID_OPCODE:
			printf("Invalid opcode %#04x", __gb_read(gb, gb->cpu_reg.pc));
			break;

		case GB_INVALID_WRITE:
			puts("Invalid write");
			return;

		case GB_INVALID_READ:
			puts("Invalid read");
			return;

		default:
			printf("Unknown error");
			break;
	}

	printf(" at PC: %#06x, SP: %#06x\n", gb->cpu_reg.pc, gb->cpu_reg.sp);

	abort();
}

/**
 * Returns a pointer to the allocated space containing the ROM. Must be freed.
 */
uint8_t *read_rom_to_ram(const char *file_name)
{
	FILE *rom_file = fopen(file_name, "rb");
	long rom_size;
	uint8_t *rom = NULL;

	if(rom_file == NULL)
        return NULL;

	fseek(rom_file, 0, SEEK_END);
	rom_size = ftell(rom_file);
	rewind(rom_file);
	rom = malloc(rom_size);

	if(fread(rom, sizeof(uint8_t), rom_size, rom_file) != rom_size)
    {
		free(rom);
		fclose(rom_file);
		return NULL;
	}

	fclose(rom_file);
	return rom;
}

int main(int argc, char **argv)
{
    struct gb_t gb;
	struct priv_t priv;
	uint32_t running = UINT32_MAX;

	if(argc != 2)
	{
		printf("Usage: %s FILE\n", argv[0]);
		return EXIT_FAILURE;
	}

	if((priv.rom = read_rom_to_ram(argv[1])) == NULL)
	{
		printf("%s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	/* TODO: Sanity check input GB file. */
    /* TODO: Init GB */
    gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write, &gb_error,
			&priv);

	/* TODO: Load Save File. */
	priv.cart_ram = malloc(gb_get_save_size(&gb));
	memset(priv.cart_ram, 0xFF, gb_get_save_size(&gb));

	while(running--)
	{
		__gb_step_cpu(&gb);
		/* Debugging */
		printf("OP: %#04X  PC: %#06X  SP: %#06X  A: %#04X  Stack: %#06X %#06X\n",
			(gb.gb_halt ? 0x00 : __gb_read(&gb, gb.cpu_reg.pc)),
			gb.cpu_reg.pc,
			gb.cpu_reg.sp,
			gb.cpu_reg.a,
			(__gb_read(&gb, 0xDFFF - 4) + (__gb_read(&gb, 0xDFFF - 3) << 8)),
			(__gb_read(&gb, 0xDFFF - 2) + (__gb_read(&gb, 0xDFFF - 1) << 8)));
	}

	puts("Timeout");

	free(priv.rom);
	free(priv.cart_ram);

	return EXIT_SUCCESS;
}
