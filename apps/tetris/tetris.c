#include "tetris.h"

#include <string.h>

tetro_type TETRO[7] = {
  {
    // O
    .bbsize = 2,
    .spawn = 0,
    .mino = {{{0, 0}, {0, 1}, {1, 0}, {1, 1}}},
    .rot = {
      {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
      {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
      {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
      {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
    }
  }, {
    // I
    .bbsize = 4,
    .spawn = -2,
    .mino = {{{2, 0}, {2, 1}, {2, 2}, {2, 3}}},
    .rot = {
      {{2, 1}, {2, 0}, {2, 3}, {2, 0}, {2, 3}},
      {{2, 1}, {2, 2}, {2, 2}, {3, 2}, {0, 2}},
      {{2, 1}, {2, 3}, {2, 0}, {1, 3}, {1, 0}},
      {{2, 1}, {2, 1}, {2, 1}, {0, 1}, {3, 1}},
    }
  }, {
    // T
    .bbsize = 3,
    .spawn = -1,
    .mino = {{{1, 0}, {1, 1}, {1, 2}, {2, 1}}},
    .rot = {
      {{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}},
      {{1, 1}, {1, 2}, {0, 2}, {3, 1}, {3, 2}},
      {{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}},
      {{1, 1}, {1, 0}, {0, 0}, {3, 1}, {3, 0}},
    }
  }, {
    // L
    .bbsize = 3,
    .spawn = -1,
    .mino = {{{1, 0}, {1, 1}, {1, 2}, {2, 2}}},
    .rot = {
      {{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}},
      {{1, 1}, {1, 2}, {0, 2}, {3, 1}, {3, 2}},
      {{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}},
      {{1, 1}, {1, 0}, {0, 0}, {3, 1}, {3, 0}},
    }
  }, {
    // J
    .bbsize = 3,
    .spawn = -1,
    .mino = {{{1, 0}, {1, 1}, {1, 2}, {2, 0}}},
    .rot = {
      {{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}},
      {{1, 1}, {1, 2}, {0, 2}, {3, 1}, {3, 2}},
      {{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}},
      {{1, 1}, {1, 0}, {0, 0}, {3, 1}, {3, 0}},
    }
  }, {
    // S
    .bbsize = 3,
    .spawn = -1,
    .mino = {{{1, 0}, {1, 1}, {2, 1}, {2, 2}}},
    .rot = {
      {{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}},
      {{1, 1}, {1, 2}, {0, 2}, {3, 1}, {3, 2}},
      {{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}},
      {{1, 1}, {1, 0}, {0, 0}, {3, 1}, {3, 0}},
    }
  }, {
    // Z
    .bbsize = 3,
    .spawn = -1,
    .mino = {{{1, 1}, {1, 2}, {2, 0}, {2, 1}}},
    .rot = {
      {{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}},
      {{1, 1}, {1, 2}, {0, 2}, {3, 1}, {3, 2}},
      {{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}},
      {{1, 1}, {1, 0}, {0, 0}, {3, 1}, {3, 0}},
    }
  }
};

uint8_t matrix[MATRIX_H][MATRIX_W];

uint32_t clear_count;
uint8_t recent_clear[4][MATRIX_W];

uint8_t drop_next[14];
uint8_t drop_pointer;
uint8_t hold_type;
bool hold_used;

uint8_t drop_type;
uint8_t drop_ori;
int8_t drop_pos[2];
uint8_t drop_interval;
uint8_t drop_countdown;
int8_t drop_lowest;
uint8_t epld_counter;
uint8_t epld_countdown;

void tetris_init()
{
  for (uint8_t i = 0; i < 7; i++) {
    uint8_t s = TETRO[i].bbsize;
#define m   TETRO[i].mino
    for (uint8_t j = 0; j < 4; j++) {
      uint8_t x = m[0][j][0];
      uint8_t y = m[0][j][1];
      m[1][j][0] = s - 1 - y;
      m[1][j][1] = x;
      m[2][j][0] = s - 1 - x;
      m[2][j][1] = s - 1 - y;
      m[3][j][0] = y;
      m[3][j][1] = s - 1 - x;
    }
    /*for (uint8_t j = 0; j < 4; j++) {
      printf("Tetrimino %u rotation %u:", i, j);
      for (uint8_t k = 0; k < 4; k++)
        printf(" (%u %u)", m[j][k][0], m[j][k][1]);
      putchar('\n');
    }*/
#undef m
  }

  memset(matrix, MINO_NONE, sizeof matrix);
  tetris_refill(0);
  tetris_refill(7);
  clear_count = 0;
  drop_pointer = 0;
  hold_type = MINO_NONE;
  drop_interval = 30;
};

static inline uint32_t mrand()
{
  static uint32_t seed = 20200203;
  return (seed = ((seed * 1103515245) + 12345) & 0x7fffffff);
}

void tetris_refill(uint8_t start)
{
  for (uint8_t i = 0; i < 7; i++) drop_next[start + i] = i;
  for (uint8_t i = 1; i < 7; i++) {
    uint8_t j = mrand() % (i + 1);
    uint8_t t = drop_next[start + j];
    drop_next[start + j] = drop_next[start + i];
    drop_next[start + i] = t;
  }
}

void tetris_resetpos()
{
  drop_ori = 0;
  drop_pos[0] = MATRIX_HV + TETRO[drop_type].spawn;
  drop_pos[1] = (MATRIX_W - TETRO[drop_type].bbsize) / 2;
  drop_lowest = drop_pos[0] + 1;
  epld_countdown = 0;
  tetris_drop();
}

void tetris_spawn()
{
  drop_type = drop_next[drop_pointer];
  if ((drop_pointer = (drop_pointer + 1) % 14) % 7 == 0)
    tetris_refill(7 - drop_pointer);
  tetris_resetpos();
  hold_used = false;
}

bool tetris_check(uint8_t check_lowest)
{
  uint8_t lowest = MATRIX_H;
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t r = drop_pos[0] + TETRO[drop_type].mino[drop_ori][i][0];
    uint8_t c = drop_pos[1] + TETRO[drop_type].mino[drop_ori][i][1];
    if (r >= MATRIX_H || c >= MATRIX_W || matrix[r][c] != MINO_NONE)
      return false;
    if (lowest > r) lowest = r;
  }
  return (!check_lowest || lowest < MATRIX_HV);
}

void tetris_lockdown()
{
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t r = drop_pos[0] + TETRO[drop_type].mino[drop_ori][i][0];
    uint8_t c = drop_pos[1] + TETRO[drop_type].mino[drop_ori][i][1];
    if (r < MATRIX_H || c < MATRIX_W) matrix[r][c] = drop_type;
  }
  drop_type = MINO_NONE;
}

bool tetris_drop()
{
  drop_pos[0]--;
  if (!tetris_check(0)) {
    drop_pos[0]++;
    return false;
  }
  drop_countdown = drop_interval;
  if (drop_pos[0] < drop_lowest) {
    drop_lowest = drop_pos[0];
    epld_counter = 15;
    epld_countdown = 30;
  }
  return true;
}

bool tetris_hor(int8_t dx)
{
  drop_pos[1] += dx;
  if (!tetris_check(0)) {
    drop_pos[1] -= dx;
    return false;
  }
  if (epld_counter > 0) {
    epld_counter--;
    epld_countdown = 30;
  }
  return true;
}

bool tetris_rotate(int8_t dir)
{
  uint8_t od = drop_ori;
  int8_t ox = drop_pos[0], oy = drop_pos[1];
  drop_ori = (drop_ori + dir + 4) % 4;
  // Try 5 rotation points in sequential order
  for (uint8_t i = 0; i < 5; i++) {
#define r   TETRO[drop_type].rot
    int8_t dx = (int8_t)r[od][i][0] - r[drop_ori][i][0];
    int8_t dy = (int8_t)r[od][i][1] - r[drop_ori][i][1];
    drop_pos[0] = ox + dx;
    drop_pos[1] = oy + dy;
    if (tetris_check(0)) {
      if (epld_counter > 0) {
        epld_counter--;
        epld_countdown = 30;
      }
      return true;
    }
#undef m
  }
  drop_ori = od;
  drop_pos[0] = ox;
  drop_pos[1] = oy;
  return false;
}

void tetris_harddrop()
{
  while (tetris_drop()) { }
  drop_countdown = 1;   // Lock down will happen at the following tick()
  epld_countdown = 1;
}

bool tetris_hold()
{
  if (hold_used) return false;
  hold_used = true;
  if (hold_type == MINO_NONE) {
    hold_type = drop_type;
    tetris_spawn();
  } else {
    uint8_t t = hold_type;
    hold_type = drop_type;
    drop_type = t;
    tetris_resetpos();
  }
  return true;
}

int8_t tetris_ghost()
{
  int8_t ox = drop_pos[0];
  while (tetris_check(0)) drop_pos[0]--;
  int8_t ret = drop_pos[0] + 1;
  drop_pos[0] = ox;
  return ret;
}

static inline uint32_t tetris_clearlines()
{
  uint32_t ret = 0;
  for (uint8_t i = 0, j = 0, k = 0; i < MATRIX_H && j < MATRIX_HV; i++) {
    // i = current row to be checked
    // j = next row to copy in the matrix
    bool line_cleared = true;
    for (uint8_t c = 0; c < MATRIX_W; c++)
      if (matrix[i][c] == MINO_NONE) { line_cleared = false; break; }
    if (line_cleared) {
      ret |= (1 << i);
      clear_count++;
      for (uint8_t c = 0; c < MATRIX_W; c++) recent_clear[k][c] = matrix[i][c];
      k++;
    } else {
      // Copy a line
      if (i != j)
        for (uint8_t c = 0; c < MATRIX_W; c++) matrix[j][c] = matrix[i][c];
      j++;
    }
  }
  return ret;
}

uint32_t tetris_tick()
{
  if (drop_countdown != 0) drop_countdown--;
  if (epld_countdown != 0) epld_countdown--;
  if (drop_countdown == 0) {
    if (tetris_drop()) {
    } else if (epld_countdown == 0) {
      if (!tetris_check(1)) return TETRIS_GAMEOVER;
      tetris_lockdown();
      return TETRIS_LOCKDOWN | tetris_clearlines();
    }
  }
  return 0;
}
