#include "audio_wrapper.h"
#include "emu.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ma_device audio_device;

#define MAX_BLOCK_SIZE  8192
#define N_BUFS  2
static uint32_t block_size = 512;

static uint16_t buf[N_BUFS][MAX_BLOCK_SIZE][2] = {{{ 0 }}};

// Read position
// Writes to (bufid + 1) % N_BUFS
static uint32_t bufid = 0, ptr = 0;
static bool request_pending = true;

// Dropped count
static uint32_t dropped = 0;

uint32_t audio_blocksize()
{
  return block_size;
}

bool audio_pending()
{
  return request_pending;
}

uint32_t audio_dropped()
{
  ma_mutex_lock(&audio_device.lock);

  uint32_t ret = dropped;
  dropped = 0;

  ma_mutex_unlock(&audio_device.lock);
  return ret;
}

void *audio_write_pos()
{
  ma_mutex_lock(&audio_device.lock);

  if (!request_pending) {
    printf("Already written\n");
    exit(1);
  }
  request_pending = false;
  void *ret = buf[(bufid + 1) % N_BUFS];

  ma_mutex_unlock(&audio_device.lock);
  return ret;
}

void audio_callback(ma_device *device, int16_t *output, const int16_t *_input, ma_uint32 frames)
{
/*
  static uint32_t p = 0;
  for (uint32_t i = 0; i < frames; i++, p++) {
    output[i * 2] = output[i * 2 + 1] =
      (int16_t)(sinf((float)p / 44100.0 * 440 * 2 * acosf(-1)) * 32767.0f);
  }
*/
  ma_mutex_lock(&device->lock);
  for (uint32_t i = 0; i < frames; i++) {
    if (ptr == block_size) {
      bufid = (bufid + 1) % N_BUFS;
      ptr = 0;
      if (request_pending) {
        if (!(headless && program_paused)) dropped++;
        memset(buf[bufid], 0, sizeof(int16_t) * 2 * block_size);
      } else {
        request_pending = true;
      }
    }
    output[i * 2 + 0] = buf[bufid][ptr][0];
    output[i * 2 + 1] = buf[bufid][ptr][1];
    ptr++;
  }
  ma_mutex_unlock(&device->lock);
}
