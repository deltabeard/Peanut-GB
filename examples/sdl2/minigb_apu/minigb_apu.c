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
	uint8_t load;
	bool enabled;
	float counter;
	float inc;
};

struct chan_vol_env {
	uint8_t step;
	bool up;
	float counter;
	float inc;
};

struct chan_freq_sweep {
	uint_fast16_t freq;
	uint8_t rate;
	uint8_t shift;
	bool up;
	float counter;
	float inc;
};

static struct chan {
	bool enabled;
	bool powered;
	bool on_left;
	bool on_right;
	bool muted;

	uint8_t volume;
	uint8_t volume_init;

	uint16_t freq;
	float    freq_counter;
	float    freq_inc;

	int_fast8_t val;

	struct chan_len_ctr    len;
	struct chan_vol_env    env;
	struct chan_freq_sweep sweep;

	// square
	uint8_t duty;
	uint8_t duty_counter;

	// noise
	uint16_t lfsr_reg;
	uint8_t     lfsr_wide;
	uint8_t      lfsr_div;

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

static void set_note_freq_inc(struct chan *c, uint16_t freq)
{
	c->freq_inc = freq / AUDIO_SAMPLE_RATE;
}

static void set_note_freq(struct chan *c)
{
	// val = 4194304.0 / (double)((2048 - f) << 5);
	// Where f < 1536 and a multiple of 16 (right shift by 3 bits).
	static const uint_fast16_t low_lut[96] = {
		64, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 70, 70, 71, 71, 72,
		73, 73, 74, 75, 75, 76, 77, 78, 78, 79, 80, 81, 81, 82, 83, 84,
		85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
		101, 102, 103, 105, 106, 107, 109, 110, 112, 113, 115, 117,
		118, 120, 122, 124, 126, 128, 130, 132, 134, 136, 138, 141,
		143, 146, 148, 151, 154, 157, 160, 163, 167, 170, 174, 178,
		182, 186, 190, 195, 199, 204, 210, 215, 221, 227, 234, 240,
		248
	};
	// val = 4194304.0 / (double)((2048 - f) << 5);
	// Where 1536 <= f < 2048 and a multiple of 2 (right shift by 1 bit).
	static const uint_fast16_t hi_lut[256] = {
		256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267,
		268, 269, 270, 271, 273, 274, 275, 276, 277, 278, 280, 281,
		282, 283, 284, 286, 287, 288, 289, 291, 292, 293, 295, 296,
		297, 299, 300, 302, 303, 304, 306, 307, 309, 310, 312, 313,
		315, 316, 318, 319, 321, 322, 324, 326, 327, 329, 330, 332,
		334, 336, 337, 339, 341, 343, 344, 346, 348, 350, 352, 354,
		356, 358, 360, 362, 364, 366, 368, 370, 372, 374, 376, 378,
		381, 383, 385, 387, 390, 392, 394, 397, 399, 402, 404, 407,
		409, 412, 414, 417, 420, 422, 425, 428, 431, 434, 436, 439,
		442, 445, 448, 451, 455, 458, 461, 464, 468, 471, 474, 478,
		481, 485, 489, 492, 496, 500, 504, 508, 512, 516, 520, 524,
		528, 532, 537, 541, 546, 550, 555, 560, 564, 569, 574, 579,
		585, 590, 595, 601, 606, 612, 618, 624, 630, 636, 642, 648,
		655, 661, 668, 675, 682, 689, 697, 704, 712, 720, 728, 736,
		744, 753, 762, 771, 780, 789, 799, 809, 819, 829, 840, 851,
		862, 873, 885, 897, 910, 923, 936, 949, 963, 978, 992, 1008,
		1024, 1040, 1057, 1074, 1092, 1110, 1129, 1149, 1170, 1191,
		1213, 1236, 1260, 1285, 1310, 1337, 1365, 1394, 1424, 1456,
		1489, 1524, 1560, 1598, 1638, 1680, 1724, 1771, 1820, 1872,
		1927, 1985, 2048, 2114, 2184, 2259, 2340, 2427, 2520, 2621,
		2730, 2849, 2978, 3120, 3276, 3449, 3640, 3855, 4096, 4369,
		4681, 5041, 5461, 5957, 6553, 7281, 8192, 9362, 10922, 13107,
		16384, 21845, 32768, 65536
	};
	uint_fast16_t freq = c->freq;
	if(freq < 1536)
	{
		freq >>= 4;
		freq = low_lut[freq];
	}
	else
	{
		freq >>= 1;
		freq = hi_lut[freq];
	}
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
				set_note_freq(c);
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

	set_note_freq(c);
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

	set_note_freq(c);

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
	set_note_freq_inc(c, freq);

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
