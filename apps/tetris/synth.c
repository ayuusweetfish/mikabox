#include "mikabox.h"
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define M_PI  3.1415926535897932384626433832795
#define round(_x) ((int)floorf((_x) + 0.5f))
#define frac(_x)  ((_x) - floorf(_x))

#define ENV_A (2e-3f * 44100) // 2 ms = 88.2 samples
#define ENV_K (0.32f * 44100) // Decays to 1/e every 0.32 s

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

static void dododo(short buf[][2], int block)
{
  if (block == 0) return;
  memset(buf, 0, block * sizeof buf[0]);
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

// Sequence data

static inline unsigned read_int(char **_p)
{
  char *p = *_p;
  unsigned ret = 0;
  while (*p != '\0' && (*p < '0' || *p > '9')) *p++;
  while (*p >= '0' && *p <= '9') ret = ret * 10 + *(p++) - '0';
  *_p = p;
  return ret;
}

static inline void skip_nondigit(char **_p)
{
  char *p = *_p;
  while (*p != '\0' && (*p < '0' || *p > '9')) *p++;
  *_p = p;
}

static int seq_total, seq_count;  // Length in samples, number of notes
static int seq_index, seq_sample; // Current index and sample

static struct note {
  unsigned t, ch, env;
  float freq, len, vol;
} notes[4096];

// Advance sequencer by a given number of samples

static inline int seq_advance(int samples)
{
  int i = seq_index;
  int start = seq_sample, end = seq_sample + samples;
  int done = 0;

  // Fails for very very short sequences (less than a block in total)
  // but who needs such ones anyway?
  while (1) {
    int delay;

    if (notes[i].t >= start && notes[i].t < end) {
      delay = notes[i].t - start;
      start = notes[i].t;
    } else if (notes[i].t + seq_total >= start && notes[i].t + seq_total < end) {
      delay = notes[i].t + seq_total - start;
      start = notes[i].t + seq_total;
    } else break;

    // Previous time
    dododo(buf + done, delay);
    done += delay;

    // Onset
    synth_note(notes[i].ch, notes[i].freq,
      notes[i].len, 0, notes[i].vol, notes[i].env);

    i = (i + 1 == seq_count ? 0 : i + 1);
  }

  dododo(buf + done, samples - done);
  seq_index = i;
  seq_sample = end % seq_total;
}

static inline void seq_read()
{
  int f = fil_open("korobeiniki.txt", FA_READ);
  char buf[524288];
  int len = fil_size(f);
  if (len > sizeof buf) {
    mika_printf("File too long (%u bytes)\n", len);
    goto done;
  }
  if (fil_read(f, buf, len) < len) {
    mika_printf("File read incomplete\n");
    goto done;
  }

  // Parse data
  // <milliseconds for each tick>
  // <length in ticks>
  // Each note following:
  //   <time in ticks> <channel> <envelope on/off (1/0)>
  //   <MIDI note number> <length in ticks> <volume in percent>
  char *p = &buf[0];
  float secs_per_tick = read_int(&p) * 1e-6f;
  float samples_per_tick = secs_per_tick * 44100;
  seq_total = round(read_int(&p) * samples_per_tick);

  int count = 0;
  do {
    notes[count].t = round(read_int(&p) * samples_per_tick);
    notes[count].ch = read_int(&p);
    notes[count].env = read_int(&p);
    int pitch = read_int(&p);
    notes[count].freq = 440.0 * powf(2, (pitch - 69) / 12.0f);
    notes[count].len = read_int(&p) * secs_per_tick;
    notes[count].vol = read_int(&p) * 0.01f;
    count++;
    skip_nondigit(&p);
  } while (*p != '\0');

  seq_count = count;
  seq_index = seq_sample = 0;

  mika_printf("length: %u s\n", seq_total / 44100);
  mika_printf("note count: %d\n", count);

done:
  fil_close(f);
  return;
}

// Synth routine

void synth()
{
  srand((unsigned)mika_rand());

  seq_read();

  block = aud_blocksize();
  mika_printf("Audio block size %d\n", block);
  if (block > sizeof(buf) / sizeof(buf[0])) {
    mika_printf("Block size too large!\n");
    while (1) mika_yield(1);
  }

  int wait_update = 0;

  while (1) {
    int drop = aud_dropped();
    if (drop) {
      mika_printf("%d frame%s of audio dropped",
        drop, drop == 1 ? "" : "s");
    }

    seq_advance(block);

    aud_write(buf);
    mika_yield(1);
  }
}

