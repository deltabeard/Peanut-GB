#include <SDL.h>

#include "Basic_Gb_Apu.h"
#include "audio.h"
#include <stdio.h>

//#define debugprintf printf
#define debugprintf(...)

static Basic_Gb_Apu apu;

void fill_audio( void *userdata, uint8_t *stream, int len );

void audio_init(SDL_AudioDeviceID *dev)
{
	const long default_sample_rate = 48000;
	SDL_AudioSpec want, have;

	debugprintf("audio was initialised\n");
	apu.set_sample_rate( default_sample_rate );

	want.freq = default_sample_rate;
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	want.samples = 1024;
	want.callback = fill_audio;
	want.userdata = NULL;

	debugprintf("Audio driver: %s\n", SDL_GetAudioDeviceName(0, 0));

	if((*dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0)) == 0)
	{
		printf("SDL could not open audio device: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_PauseAudioDevice(*dev, 0);
}

void fill_audio( void *userdata, uint8_t *stream, int len )
{
    debugprintf( "fill_audio( %p, %p, %d );\n", userdata, stream, len );
    memset( stream, 0, len );
    apu.read_samples( (blip_sample_t *)stream, len/2 );
}

void audio_cleanup(void)
{
    debugprintf("audio was cleaned up");
    SDL_CloseAudio();
}

uint8_t audio_read( uint16_t address )
{
    return apu.read_register( address );
}

void audio_write( uint16_t address, uint8_t data )
{
    apu.write_register(address, data);
}

void audio_frame(void)
{
    apu.end_frame();
}

int audio_length(void)
{
    return apu.samples_avail();
}

void audio_sample_rate_set(unsigned int sample_rate)
{
	apu.set_sample_rate( sample_rate );
}
