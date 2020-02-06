#ifndef _Tetris_main_h_
#define _Tetris_main_h_

#include "mikabox.h"
#include <stdbool.h>

void draw();
void synth();
void event();
void update();

extern uint64_t last_btns;
extern uint64_t cur_btns;
static inline bool btnp(uint64_t b)
{
  return !(last_btns & b) && (cur_btns & b);
}
static inline bool btnr(uint64_t b)
{
  return (last_btns & b) && !(cur_btns & b);
}

// synth

void synth_note(int channel, float frequency,
  float length, float delay, float volume, bool use_envelope);

#endif
