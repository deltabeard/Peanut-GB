#ifndef _AUDIO_H_
#define _AUDIO_H_
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

int audio_init(void);
uint8_t audio_read( uint16_t address );
void audio_write( uint16_t address, uint8_t data );
void audio_cleanup(void);
void audio_frame(void);

#ifdef __cplusplus
}
#endif

#endif // _AUDIO_H_

