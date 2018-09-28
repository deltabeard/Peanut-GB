#include <SDL2/SDL.h>

#include "Basic_Gb_Apu.h"
#include "audio.h"
#include <stdio.h>

//#define debugprintf printf
#define debugprintf(...)

static Basic_Gb_Apu apu;
SDL_AudioDeviceID dev;

const long sample_rate = 16384;
void fill_audio( void *userdata, uint8_t *stream, int len );

int audio_init(void)
{
	SDL_AudioSpec want, have;

    debugprintf("audio was initialised\n");
    apu.set_sample_rate( sample_rate );

    want.freq = sample_rate;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 1024;
    want.callback = fill_audio;
    want.userdata = NULL;

	printf("Audio driver: %s\n", SDL_GetAudioDeviceName(0, 0));

	if((dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE)) == 0)
	{
		printf("SDL could not open audio device: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_PauseAudioDevice(dev, 0);

    return 0;
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

