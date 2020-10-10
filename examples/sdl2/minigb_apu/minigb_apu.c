/**
 * minigb_apu is released under the terms listed within the LICENSE file.
 *
 * minigb_apu emulates the audio processing unit (APU) of the Game Boy. This
 * project is based on MiniGBS by Alex Baines: https://github.com/baines/MiniGBS
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "minigb_apu.h"

#define ENABLE_HIPASS 1

#define AUDIO_NSAMPLES ((unsigned)(AUDIO_SAMPLE_RATE / VERTICAL_SYNC) * 2)

#define AUDIO_MEM_SIZE (0xFF3F - 0xFF10 + 1)
#define AUDIO_ADDR_COMPENSATION 0xFF10

#define MAX(a, b) ( a > b ? a : b )
#define MIN(a, b) ( a <= b ? a : b )

/**
 * Memory holding audio registers between 0xFF10 and 0xFF3F inclusive.
 */
static uint8_t audio_mem[AUDIO_MEM_SIZE];

struct chan_len_ctr {
	unsigned load : 6;
	unsigned enabled : 1;
	float counter;
	float inc;
};

struct chan_vol_env {
	unsigned step : 3;
	unsigned up : 1;
	float counter;
	float inc;
};

struct chan_freq_sweep {
	uint_fast16_t freq;
	unsigned rate : 3;
	unsigned up : 1;
	unsigned shift : 3;
	float counter;
	float inc;
};

static struct chan {
	unsigned enabled : 1;
	unsigned powered : 1;
	unsigned on_left : 1;
	unsigned on_right : 1;
	unsigned muted : 1;

	unsigned volume : 4;
	unsigned volume_init : 4;

	uint16_t freq;
	float    freq_counter;
	float    freq_inc;

	int val;

	struct chan_len_ctr    len;
	struct chan_vol_env    env;
	struct chan_freq_sweep sweep;

	// square
	uint8_t duty;
	uint8_t duty_counter;

	// noise
	uint16_t lfsr_reg;
	bool     lfsr_wide;
	int      lfsr_div;

	// wave
	uint8_t sample;

#if ENABLE_HIPASS
	float capacitor;
#endif
} chans[4];

static float vol_l, vol_r;

static float hipass(struct chan *c, float sample)
{
#if ENABLE_HIPASS
	float out    = sample - c->capacitor;
	c->capacitor = sample - out * 0.996f;
	return out;
#else
	return sample;
#endif
}

static void set_note_freq(struct chan *c, const uint_fast16_t freq)
{
	c->freq_inc = freq / AUDIO_SAMPLE_RATE;
}

static void chan_enable(const uint_fast8_t i, const bool enable)
{
	chans[i].enabled = enable;

	uint8_t val = (audio_mem[0xFF26 - AUDIO_ADDR_COMPENSATION] & 0x80) |
		      (chans[3].enabled << 3) | (chans[2].enabled << 2) |
		      (chans[1].enabled << 1) | (chans[0].enabled << 0);

	audio_mem[0xFF26 - AUDIO_ADDR_COMPENSATION] = val;
}

static void update_env(struct chan *c)
{
	c->env.counter += c->env.inc;

	while (c->env.counter > 1.0f) {
		if (c->env.step) {
			c->volume += c->env.up ? 1 : -1;
			if (c->volume == 0 || c->volume == 15) {
				c->env.inc = 0;
			}
			c->volume = MAX(0, MIN(15, c->volume));
		}
		c->env.counter -= 1.0f;
	}
}

static void update_len(struct chan *c)
{
	if (c->len.enabled) {
		c->len.counter += c->len.inc;
		if (c->len.counter > 1.0f) {
			chan_enable(c - chans, 0);
			c->len.counter = 0.0f;
		}
	}
}

static bool update_freq(struct chan *c, float *pos)
{
	float inc = c->freq_inc - *pos;
	c->freq_counter += inc;

	if (c->freq_counter > 1.0f) {
		*pos		= c->freq_inc - (c->freq_counter - 1.0f);
		c->freq_counter = 0.0f;
		return true;
	} else {
		*pos = c->freq_inc;
		return false;
	}
}

static void update_sweep(struct chan *c)
{
	c->sweep.counter += c->sweep.inc;

	while (c->sweep.counter > 1.0f) {
		if (c->sweep.shift) {
			uint16_t inc = (c->sweep.freq >> c->sweep.shift);
			if (!c->sweep.up)
				inc *= -1;

			c->freq += inc;
			if (c->freq > 2047) {
				c->enabled = 0;
			} else {
				set_note_freq(c,
					4194304 / ((2048 - c->freq)<< 5));
				c->freq_inc *= 8.0f;
			}
		} else if (c->sweep.rate) {
			c->enabled = 0;
		}
		c->sweep.counter -= 1.0f;
	}
}

static void update_square(float *restrict samples, const bool ch2)
{
	struct chan *c = chans + ch2;
	if (!c->powered)
		return;

	set_note_freq(c, 4194304.0f / ((2048 - c->freq) << 5));
	c->freq_inc *= 8.0f;

	for (uint_fast16_t i = 0; i < AUDIO_NSAMPLES; i += 2) {
		update_len(c);

		if (c->enabled) {
			update_env(c);
			if (!ch2)
				update_sweep(c);

			float pos      = 0.0f;
			float prev_pos = 0.0f;
			float sample   = 0.0f;

			while (update_freq(c, &pos)) {
				c->duty_counter = (c->duty_counter + 1) & 7;
				sample += ((pos - prev_pos) / c->freq_inc) *
					  (float)c->val;
				c->val = (c->duty & (1 << c->duty_counter)) ?
						 1 :
						 -1;
				prev_pos = pos;
			}
			sample += ((pos - prev_pos) / c->freq_inc) *
				  (float)c->val;
			sample = hipass(c, sample * (c->volume / 15.0f));

			if (!c->muted) {
				samples[i + 0] +=
					sample * 0.25f * c->on_left * vol_l;
				samples[i + 1] +=
					sample * 0.25f * c->on_right * vol_r;
			}
		}
	}
}

static uint8_t wave_sample(const unsigned int pos, const unsigned int volume)
{
	uint8_t sample =
		audio_mem[(0xFF30 + pos / 2) - AUDIO_ADDR_COMPENSATION];
	if (pos & 1) {
		sample &= 0xF;
	} else {
		sample >>= 4;
	}
	return volume ? (sample >> (volume - 1)) : 0;
}

static void update_wave(float *restrict samples)
{
	struct chan *c = chans + 2;
	if (!c->powered)
		return;

	uint_fast16_t freq = 4194304.0f / ((2048 - c->freq) << 5);
	set_note_freq(c, freq);

	c->freq_inc *= 16.0f;

	for (uint_fast16_t i = 0; i < AUDIO_NSAMPLES; i += 2) {
		update_len(c);

		if (c->enabled) {
			float pos      = 0.0f;
			float prev_pos = 0.0f;
			float sample   = 0.0f;

			c->sample = wave_sample(c->val, c->volume);

			while (update_freq(c, &pos)) {
				c->val = (c->val + 1) & 31;
				sample += ((pos - prev_pos) / c->freq_inc) *
					  (float)c->sample;
				c->sample = wave_sample(c->val, c->volume);
				prev_pos  = pos;
			}
			sample += ((pos - prev_pos) / c->freq_inc) *
				  (float)c->sample;

			if (c->volume > 0) {
				float diff = (float[]){ 7.5f, 3.75f,
							1.5f }[c->volume - 1];
				sample     = hipass(c, (sample - diff) / 7.5f);

				if (!c->muted) {
					samples[i + 0] += sample * 0.25f *
							  c->on_left * vol_l;
					samples[i + 1] += sample * 0.25f *
							  c->on_right * vol_r;
				}
			}
		}
	}
}

static void update_noise(float *restrict samples)
{
	struct chan *c = chans + 3;
	if (!c->powered)
		return;

	uint_fast16_t freq = 4194304 / ((uint_fast8_t[]){
			8, 16, 32, 48, 64, 80, 96, 112
		}[c->lfsr_div] << c->freq);
	set_note_freq(c, freq);

	if (c->freq >= 14)
		c->enabled = 0;

	for (uint_fast16_t i = 0; i < AUDIO_NSAMPLES; i += 2) {
		update_len(c);

		if (c->enabled) {
			update_env(c);

			float pos      = 0.0f;
			float prev_pos = 0.0f;
			float sample   = 0.0f;

			while (update_freq(c, &pos)) {
				c->lfsr_reg = (c->lfsr_reg << 1) |
					      (c->val == 1);

				if (c->lfsr_wide) {
					c->val = !(((c->lfsr_reg >> 14) & 1) ^
						   ((c->lfsr_reg >> 13) & 1)) ?
							 1 :
							 -1;
				} else {
					c->val = !(((c->lfsr_reg >> 6) & 1) ^
						   ((c->lfsr_reg >> 5) & 1)) ?
							 1 :
							 -1;
				}
				sample += ((pos - prev_pos) / c->freq_inc) *
					  c->val;
				prev_pos = pos;
			}
			sample += ((pos - prev_pos) / c->freq_inc) * c->val;
			sample = hipass(c, sample * (c->volume / 15.0f));

			if (!c->muted) {
				samples[i + 0] +=
					sample * 0.25f * c->on_left * vol_l;
				samples[i + 1] +=
					sample * 0.25f * c->on_right * vol_r;
			}
		}
	}
}

/**
 * SDL2 style audio callback function.
 */
void audio_callback(void *userdata, uint8_t *restrict stream, int len)
{
	float *samples = (float *)stream;

	/* Appease unused variable warning. */
	(void)userdata;

	memset(stream, 0, len);

	update_square(samples, 0);
	update_square(samples, 1);
	update_wave(samples);
	update_noise(samples);
}

static void chan_trigger(uint_fast8_t i)
{
	struct chan *c = chans + i;

	chan_enable(i, 1);
	c->volume = c->volume_init;

	// volume envelope
	{
		uint8_t val =
			audio_mem[(0xFF12 + (i * 5)) - AUDIO_ADDR_COMPENSATION];

		c->env.step = val & 0x07;
		c->env.up   = val & 0x08 ? 1 : 0;
		c->env.inc  = c->env.step ? (64.0f / (float)c->env.step) /
						   AUDIO_SAMPLE_RATE :
					   8.0f / AUDIO_SAMPLE_RATE;
		c->env.counter = 0.0f;
	}

	// freq sweep
	if (i == 0) {
		uint8_t val = audio_mem[0xFF10 - AUDIO_ADDR_COMPENSATION];

		c->sweep.freq  = c->freq;
		c->sweep.rate  = (val >> 4) & 0x07;
		c->sweep.up    = !(val & 0x08);
		c->sweep.shift = (val & 0x07);
		c->sweep.inc   = c->sweep.rate ?
				       (128.0f / (float)(c->sweep.rate)) /
					       AUDIO_SAMPLE_RATE :
				       0;
		c->sweep.counter = nexttowardf(1.0f, 1.1f);
	}

	int len_max = 64;

	if (i == 2) { // wave
		len_max = 256;
		c->val  = 0;
	} else if (i == 3) { // noise
		c->lfsr_reg = 0xFFFF;
		c->val      = -1;
	}

	c->len.inc =
		(256.0f / (float)(len_max - c->len.load)) / AUDIO_SAMPLE_RATE;
	c->len.counter = 0.0f;
}

/**
 * Read audio register.
 * \param addr	Address of audio register. Must be 0xFF10 <= addr <= 0xFF3F.
 *				This is not checked in this function.
 * \return		Byte at address.
 */
uint8_t audio_read(const uint16_t addr)
{
	static uint8_t ortab[] = { 0x80, 0x3f, 0x00, 0xff, 0xbf, 0xff,
				   0x3f, 0x00, 0xff, 0xbf, 0x7f, 0xff,
				   0x9f, 0xff, 0xbf, 0xff, 0xff, 0x00,
				   0x00, 0xbf, 0x00, 0x00, 0x70 };

	if (addr > 0xFF26)
		return audio_mem[addr - AUDIO_ADDR_COMPENSATION];
	
	return audio_mem[addr - AUDIO_ADDR_COMPENSATION] | ortab[addr - 0xFF10];
}

/**
 * Write audio register.
 * \param addr	Address of audio register. Must be 0xFF10 <= addr <= 0xFF3F.
 *				This is not checked in this function.
 * \param val	Byte to write at address.
 */
void audio_write(const uint16_t addr, const uint8_t val)
{
	/* Find sound channel corresponding to register address. */
	uint_fast8_t i = (addr - 0xFF10) / 5;
	audio_mem[addr - AUDIO_ADDR_COMPENSATION] = val;

	switch (addr) {
	case 0xFF12:
	case 0xFF17:
	case 0xFF21: {
		chans[i].volume_init = val >> 4;
		chans[i].powered     = (val >> 3) != 0;

		// "zombie mode" stuff, needed for Prehistorik Man and probably
		// others
		if (chans[i].powered && chans[i].enabled) {
			if ((chans[i].env.step == 0 && chans[i].env.inc != 0)) {
				if (val & 0x08) {
					chans[i].volume++;
				} else {
					chans[i].volume += 2;
				}
			} else {
				chans[i].volume = 16 - chans[i].volume;
			}

			chans[i].volume &= 0x0F;
			chans[i].env.step = val & 0x07;
		}
	} break;

	case 0xFF1C:
		chans[i].volume = chans[i].volume_init = (val >> 5) & 0x03;
		break;

	case 0xFF11:
	case 0xFF16:
	case 0xFF20: {
		const uint8_t duty_lookup[] = { 0x10, 0x30, 0x3C, 0xCF };
		chans[i].len.load	   = val & 0x3f;
		chans[i].duty		    = duty_lookup[val >> 6];
		break;
	}

	case 0xFF1B:
		chans[i].len.load = val;
		break;

	case 0xFF13:
	case 0xFF18:
	case 0xFF1D:
		chans[i].freq &= 0xFF00;
		chans[i].freq |= val;
		break;

	case 0xFF1A:
		chans[i].powered = (val & 0x80) != 0;
		chan_enable(i, val & 0x80);
		break;

	case 0xFF14:
	case 0xFF19:
	case 0xFF1E:
		chans[i].freq &= 0x00FF;
		chans[i].freq |= ((val & 0x07) << 8);
		/* Intentional fall-through. */
	case 0xFF23:
		chans[i].len.enabled = val & 0x40 ? 1 : 0;
		if (val & 0x80)
			chan_trigger(i);

		break;

	case 0xFF22:
		chans[3].freq      = val >> 4;
		chans[3].lfsr_wide = !(val & 0x08);
		chans[3].lfsr_div  = val & 0x07;
		break;

	case 0xFF24:
		vol_l = ((val >> 4) & 0x07) / 7.0f;
		vol_r = (val & 0x07) / 7.0f;
		break;

	case 0xFF25:
		for (uint_fast8_t i = 0; i < 4; ++i) {
			chans[i].on_left  = (val >> (4 + i)) & 1;
			chans[i].on_right = (val >> i) & 1;
		}
		break;
	}
}

void audio_init(void)
{
	/* Initialise channels and samples. */
	memset(chans, 0, sizeof(chans));
	chans[0].val = chans[1].val = -1;

	/* Initialise IO registers. */
	{
		const uint8_t regs_init[] = { 0x80, 0xBF, 0xF3, 0xFF, 0x3F,
					      0xFF, 0x3F, 0x00, 0xFF, 0x3F,
					      0x7F, 0xFF, 0x9F, 0xFF, 0x3F,
					      0xFF, 0xFF, 0x00, 0x00, 0x3F,
					      0x77, 0xF3, 0xF1 };

		for(uint_fast8_t i = 0; i < sizeof(regs_init); ++i)
			audio_write(0xFF10 + i, regs_init[i]);
	}

	/* Initialise Wave Pattern RAM. */
	{
		const uint8_t wave_init[] = { 0xac, 0xdd, 0xda, 0x48,
					      0x36, 0x02, 0xcf, 0x16,
					      0x2c, 0x04, 0xe5, 0x2c,
					      0xac, 0xdd, 0xda, 0x48 };

		for(uint_fast8_t i = 0; i < sizeof(wave_init); ++i)
			audio_write(0xFF30 + i, wave_init[i]);
	}
}
