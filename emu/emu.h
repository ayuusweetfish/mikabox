#ifndef _Mikabox_emu_h_
#define _Mikabox_emu_h_

#include "timer_lib/timer.h"

extern int8_t routine_id;
extern uint32_t routine_pc[3];

extern uint64_t app_tick;
void update_tick();

#define MAX_PLAYERS 4
extern int num_players;
extern uint64_t player_btns[MAX_PLAYERS];
extern uint64_t player_axes[MAX_PLAYERS]; // Packed quad s8

#endif
