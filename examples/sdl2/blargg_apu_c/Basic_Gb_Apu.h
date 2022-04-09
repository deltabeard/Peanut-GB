
// Simplified Nintendo Game Boy PAPU sound chip emulator

// Gb_Snd_Emu 0.1.4. Copyright (C) 2003-2005 Shay Green. GNU LGPL license.

#pragma once

#include "Gb_Apu.h"
#include "Multi_Buffer.h"

typedef enum {
	NO_ERROR = 0
} blargg_err_t;

// Set output sample rate
blargg_err_t set_sample_rate(long rate);

// Pass reads and writes in the range 0xff10-0xff3f
void write_register(gb_addr_t, int data );
int read_register(gb_addr_t);

// End a 1/60 sound frame and add samples to buffer
void end_frame();

// Read at most 'count' samples out of buffer and return number actually read
typedef blip_sample_t sample_t;
long read_samples( sample_t* out, long count );

#if 0
class Basic_Gb_Apu {
public:
	Basic_Gb_Apu();
	~Basic_Gb_Apu();
	
	// Set output sample rate
	blargg_err_t set_sample_rate( long rate );
	
	// Pass reads and writes in the range 0xff10-0xff3f
	void write_register( gb_addr_t, int data );
	int read_register( gb_addr_t );
	
	// End a 1/60 sound frame and add samples to buffer
	void end_frame();
	
	// Samples are generated in stereo, left first. Sample counts are always
	// a multiple of 2.
	
	// Number of samples in buffer
	long samples_avail() const;
	
	// Read at most 'count' samples out of buffer and return number actually read
	typedef blip_sample_t sample_t;
	long read_samples( sample_t* out, long count );
	
private:
	Gb_Apu apu;
	Stereo_Buffer buf;
	blip_time_t time;
	
	// faked CPU timing
	blip_time_t clock() { return time += 4; }
};

#endif

