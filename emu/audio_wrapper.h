#ifndef _Mikabox_audio_wrapper_h_
#define _Mikabox_audio_wrapper_h_

#include "miniaudio/miniaudio.h"
#include <stdbool.h>

extern ma_device audio_device;

uint32_t audio_blocksize();
bool audio_pending();
void audio_clear_pending();
uint32_t audio_dropped();
void *audio_write_pos();

void audio_callback(ma_device *device, int16_t *output, const int16_t *_input, ma_uint32 frames);

#endif
