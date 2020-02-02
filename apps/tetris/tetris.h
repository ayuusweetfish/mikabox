#ifndef _Tetris_tetris_h_
#define _Tetris_tetris_h_

#include <stdbool.h>
#include <stdint.h>

#define MATRIX_W  10
#define MATRIX_H  30
#define MATRIX_HV   20

#define MINO_NONE   255
#define MINO_O    0
#define MINO_I    1
#define MINO_T    2
#define MINO_L    3
#define MINO_J    4
#define MINO_S    5
#define MINO_Z    6

typedef struct tetro_type {
  uint8_t bbsize;         // Side length of bounding box
  int8_t spawn;           // Number of rows above the skyline at spawn
  uint8_t mino[4][4][2];  // The four blocks; [orientation][block index][x/y]
  uint8_t rot[4][5][2];   // Rotation points; [orientation][point index][x/y]
                          // Orientation is NESW
} tetro_type;

extern tetro_type TETRO[7];

// Playfield

extern uint8_t matrix[MATRIX_H][MATRIX_W];

extern uint32_t clear_count;
extern uint8_t recent_clear[4][MATRIX_W];

extern uint8_t drop_next[14];
extern uint8_t drop_pointer;
extern uint8_t hold_type;
extern bool hold_used;

extern uint8_t drop_type;
extern uint8_t drop_ori;
extern int8_t drop_pos[2];    // {row, column}
extern uint8_t drop_interval;
extern uint8_t drop_countdown;
extern int8_t drop_lowest;    // Lowest row the bounding box has ever reached
extern uint8_t epld_counter;  // Extended placement lock down
extern uint8_t epld_countdown;

void tetris_init();

void tetris_refill(uint8_t start);
void tetris_spawn();
bool tetris_check(uint8_t check_lowest);
void tetris_lockdown();
bool tetris_drop();
bool tetris_hor(int8_t dx);
bool tetris_rotate(int8_t dir);
void tetris_harddrop();
bool tetris_hold();
int8_t tetris_ghost();

#define TETRIS_LOCKDOWN (1 << 30)
#define TETRIS_GAMEOVER (1 << 31)

uint32_t tetris_tick();

#endif
