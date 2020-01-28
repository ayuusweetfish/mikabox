#ifndef _Mikabox_emu_h_
#define _Mikabox_emu_h_

#include "timer_lib/timer.h"

extern int8_t routine_id;
extern uint32_t routine_pc[8];

extern uint32_t req_flags;

extern uint64_t app_tick;
void update_tick();

#define MAX_PLAYERS 4
extern int num_players;
extern uint64_t player_btns[MAX_PLAYERS];
extern uint64_t player_axes[MAX_PLAYERS]; // Packed octal s8

void syscall_read_mem(uint32_t addr, uint32_t size, void *buf);
void *syscall_dup_mem(uint32_t addr, uint32_t size);
void *syscall_dup_str(uint32_t addr);
void syscall_write_mem(uint32_t addr, uint32_t size, void *buf);

#endif
