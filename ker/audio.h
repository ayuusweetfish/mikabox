#ifndef _Mikabox_audio_h_
#define _Mikabox_audio_h_

#include <stdbool.h>
#include <stdint.h>

uint32_t audio_blocksize();
bool audio_pending();
uint32_t audio_dropped();
void *audio_write_pos();

unsigned audio_callback(int16_t *buf, unsigned chunk_size);

#endif
