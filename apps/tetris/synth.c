#include "mikabox.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define M_PI  3.1415926535897932384626433832795
#define round(_x) ((int)floorf((_x) + 0.5f))
#define frac(_x)  ((_x) - floorf(_x))

#define ENV_A (2e-3f * 44100) // 2 ms = 88.2 samples
#define ENV_K (0.3f * 44100)  // Decays to 1/e every 0.3 s

// A crude sound emulator for retro consoles

static int block;
static short buf[4096][2];

// Square *2, Triangle, Noise

static float freq[4];   // Frequency (in Hz)
static int len[4];      // Note length (in samples)
static float vol[4];    // Volume (linear)
static bool env[4];     // Envelope
static int phase[4];    // Length played (in samples); negative denotes delay

void synth_note(int channel, float frequency,
  float length, float delay, float volume, bool use_envelope)
{
  freq[channel] = frequency;
  len[channel] = round(length * 44100);
  vol[channel] = volume;
  env[channel] = use_envelope;
  phase[channel] = -round(delay * 44100);
}

static void dododo()
{
  memset(buf, 0, sizeof buf);
  for (int ch = 0; ch < 4; ch++) if (phase[ch] < len[ch]) {
    int start = (phase[ch] < 0 ? -phase[ch] : 0);
    int end = len[ch] - phase[ch];
    if (start > block) start = block;
    if (end > block) end = block;

    float step = 1.0f / 44100 * freq[ch];
    float rate = frac(phase[ch] * step);

  #define _envelope(_i) (env[ch] ? \
    ((_i) < ENV_A ? ((float)(_i) / ENV_A) : expf(-(_i) / ENV_K)) : 1)
  #define envelope _envelope(phase[ch] + i)

    if (ch <= 1) {
      // Square
      if (env[ch]) {
        for (int i = start; i < end; i++, rate = frac(rate + step)) {
          buf[i][0] += (rate < 0.5f ? +1 : -1) *
            round(vol[ch] * envelope * 32767);
        }
      } else {
        short amp = round(vol[ch] * 32767);
        for (int i = start; i < end; i++, rate = frac(rate + step))
          buf[i][0] += (rate < 0.5f ? +amp : -amp);
      }
    } else if (ch == 2) {
      // Triangle
      for (int i = start; i < end; i++, rate = frac(rate + step)) {
        float val = (rate < 0.5f ? rate * 4 - 1 : 3 - rate * 4);
        buf[i][0] += round(val * vol[ch] * envelope * 32767);
      }
    } else if (ch == 3) {
      for (int i = start; i < end; i++) {
        buf[i][0] += round((float)rand() / (RAND_MAX - 1)
          * vol[ch] * envelope * 32767);
      }
    }

    phase[ch] += block;
  }
  for (int i = 0; i < block; i++) buf[i][1] = buf[i][0];
}

void synth()
{
  srand((unsigned)mika_rand());

  block = aud_blocksize();
  mika_printf("Audio block size %d\n", block);
  if (block > sizeof(buf) / sizeof(buf[0])) {
    mika_printf("Block size too large!\n");
    while (1) mika_yield(1);
  }

  while (1) {
    int drop = aud_dropped();
    if (drop) {
      mika_printf("%d frame%s of audio dropped",
        drop, drop == 1 ? "" : "s");
    }
    dododo();
    aud_write(buf);
    mika_yield(1);
  }
}

