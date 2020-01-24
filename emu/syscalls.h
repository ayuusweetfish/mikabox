#include <stdint.h>

#define FN(__grp, __id)    \
  syscall_##__grp##_##__id

#define init(__fn)

#if !SYSCALLS_DECL && !SYSCALLS_IMPL && !SYSCALLS_TABLE && !SYSCALLS_INIT
#define SYSCALLS_DECL 1
#endif

#if SYSCALLS_DECL
  #define def(__grp, __id, __fn)  \
    uint64_t FN(__grp, __id)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);
#elif SYSCALLS_IMPL
  #define def(__grp, __id, __fn)  \
    uint64_t FN(__grp, __id)(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) \
    { __fn return 0; }
#elif SYSCALLS_TABLE
  #define def(__grp, __id, __fn)  \
    [SYSCALL_GRP_OFFS_##__grp + __id] = &FN(__grp, __id),
#elif SYSCALLS_INIT
  #define def(__grp, __id, __fn)
  #undef init
  #define init(__fn)  __fn
#else
  #define def(__grp, __id, __fn)
#endif

#define SYSCALL_GRP_OFFS_GEN  0
#define SYSCALL_GRP_OFFS_GFX  256
#define SYSCALL_GRP_OFFS_FIL  512
#define SYSCALL_GRP_OFFS_AUD  768

#if SYSCALLS_DECL
void syscalls_init();

#elif SYSCALLS_IMPL
#include "emu.h"
#include "v3d_wrapper.h"
#include "../sys/pool.h"
#include <stdio.h>
#include <string.h>

static pool_decl(v3d_ctx, 16, ctxs);
static pool_decl(v3d_tex, 4096, texs);
static pool_decl(v3d_vertarr, 4096, vas);
static pool_decl(v3d_unifarr, 4096, uas);
static pool_decl(v3d_shader, 256, shaders);
static pool_decl(v3d_batch, 4096, batches);
static pool_decl(v3d_buf, 4096, ias);

#define syscall_log(_fmt, ...) \
  printf("%s: " _fmt, __func__, ##__VA_ARGS__)

void syscall_read_mem(uint32_t addr, uint32_t size, void *buf);
void syscall_reinit_rng();
uint64_t syscalls_lcg;
static uint32_t rng_count = 0;

#endif

def(GEN, 0, {
  if (routine_id != -1) {
    syscall_log("Routines can only be chaned in initialization routine\n");
    return 0;
  }
  routine_pc[0] = r0;
  routine_pc[1] = r1;
  routine_pc[2] = r2;
})

def(GEN, 1, {
})

def(GEN, 2, {
  update_tick();
  return app_tick;
})

def(GEN, 3, {
  return num_players;
})

def(GEN, 4, {
  if (r0 < num_players)
    return player_btns[r0];
})

def(GEN, 5, {
  if (r0 < num_players)
    return player_axes[r0];
})

def(GEN, 6, {
  if (++rng_count >= 1e6) {
    rng_count = 0;
    syscall_reinit_rng();
  }
  // Newlib/Musl LCG implementation
  uint64_t ret;
  syscalls_lcg = (syscalls_lcg * 6364136223846793005LL + 1);
  ret = (syscalls_lcg >> 32) << 32;
  syscalls_lcg = (syscalls_lcg * 6364136223846793005LL + 1);
  ret = ret | (syscalls_lcg >> 32);
  return ret;
})

def(GEN, 7, {
  char last;
  char ch;
  while (1) {
    syscall_read_mem(r0++, 1, &ch);
    if (ch == '\0') break;
    putchar(last = ch);
  }
  if (last != '\n') putchar('\n');
})

def(GFX, 0, {
  size_t idx;
  v3d_ctx *c = pool_alloc(&ctxs, &idx);
  if (c != NULL) {
    *c = v3d_ctx_create();
    return idx;
  } else {
    return (uint32_t)-1;
  }
})

def(GFX, 1, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  v3d_tex *t = pool_elm(&texs, r1);
  if (c == NULL || t == NULL) return (uint32_t)-2;
  v3d_ctx_anew(c, *t, r2);
})

def(GFX, 2, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  v3d_batch *b = pool_elm(&batches, r1);
  if (c == NULL || b == NULL) return (uint32_t)-2;
  v3d_ctx_use_batch(c, b);
})

def(GFX, 3, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  if (c == NULL) return (uint32_t)-2;
  v3d_call call;
  call.is_indexed = !!r1;
  call.num_verts = r2;
  if (r1) { // Indexed
    v3d_buf *m = pool_elm(&ias, r3);
    if (m == NULL) return (uint32_t)-2;
    call.indices = *m;
  } else {
    call.start_index = r3;
  }
  v3d_ctx_add_call(c, &call);
})

def(GFX, 4, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  if (c == NULL) return (uint32_t)-2;
  v3d_ctx_issue(c);
})

def(GFX, 5, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  if (c == NULL) return (uint32_t)-2;
  v3d_ctx_wait(c);
})

def(GFX, 15, {
  v3d_ctx *c = pool_elm(&ctxs, r0);
  if (c == NULL) return (uint32_t)-2;
  v3d_ctx_wait(c);
  v3d_ctx_close(c);
  pool_release(&ctxs, r0);
})

def(GFX, 16, {
  size_t idx;
  v3d_tex *t = pool_alloc(&texs, &idx);
  if (t == NULL) return (uint32_t)-1;
  *t = v3d_tex_create(r0, r1);
  return idx;
})

def(GFX, 17, {
  v3d_tex *t = pool_elm(&texs, r0);
  if (t == NULL) return (uint32_t)-2;
  v3d_tex_update(t, (uint8_t *)r1, (v3d_tex_fmt_t)r2);  // !
})

init({
  size_t idx;
  v3d_tex *t = pool_alloc(&texs, &idx);
  assert(t != NULL && idx == 0);
})

def(GFX, 18, {
  v3d_tex *t = pool_elm(&texs, 0);
  if (t == NULL) return (uint32_t)-2;
  *t = v3d_tex_screen(r0);
  return 0;
})

def(GFX, 31, {
  if (r0 == 0) return (uint32_t)-3;
  v3d_tex *t = pool_elm(&texs, r0);
  if (t == NULL) return (uint32_t)-2;
  v3d_tex_close(t);
  pool_release(&texs, r0);
})

def(GFX, 32, {
  size_t idx;
  v3d_vertarr *a = pool_alloc(&vas, &idx);
  if (a == NULL) return (uint32_t)-1;
  *a = v3d_vertarr_create(r0, r1);
  return idx;
})

def(GFX, 33, {
  v3d_vertarr *a = pool_elm(&vas, r0);
  if (a == NULL) return (uint32_t)-2;
  v3d_vertarr_put(a, r1, r2, r3);
})

def(GFX, 47, {
  v3d_vertarr *a = pool_elm(&vas, r0);
  if (a == NULL) return (uint32_t)-2;
  v3d_vertarr_close(a);
  pool_release(&vas, r0);
})

def(GFX, 48, {
  size_t idx;
  v3d_unifarr *a = pool_alloc(&uas, &idx);
  if (a == NULL) return (uint32_t)-1;
  *a = v3d_unifarr_create(r0);
  return idx;
})

def(GFX, 49, {
  v3d_unifarr *a = pool_elm(&uas, r0);
  if (a == NULL) return (uint32_t)-2;
  v3d_unifarr_putu32(a, r1, r2);
})

def(GFX, 50, {
  v3d_unifarr *a = pool_elm(&uas, r0);
  v3d_tex *t = pool_elm(&texs, r2);
  if (a == NULL || t == NULL) return (uint32_t)-2;
  v3d_unifarr_puttex(a, r1, *t, r3);
})

def(GFX, 63, {
  v3d_unifarr *a = pool_elm(&uas, r0);
  if (a == NULL) return (uint32_t)-2;
  v3d_unifarr_close(a);
  pool_release(&uas, r0);
})

def(GFX, 64, {
  size_t idx;
  v3d_shader *s = pool_alloc(&shaders, &idx);
  if (s == NULL) return (uint32_t)-1;
  *s = v3d_shader_create((const char *)r0); // !
  return idx;
})

def(GFX, 79, {
  v3d_shader *s = pool_elm(&shaders, r0);
  if (s == NULL) return (uint32_t)-2;
  v3d_shader_close(s);
  pool_release(&shaders, r0);
})

def(GFX, 80, {
  v3d_vertarr *va = pool_elm(&vas, r0);
  v3d_unifarr *ua = pool_elm(&uas, r1);
  v3d_shader *s = pool_elm(&shaders, r2);
  if (va == NULL || ua == NULL || s == NULL)
    return (uint32_t)-2;

  size_t idx;
  v3d_batch *b = pool_alloc(&batches, &idx);
  if (b == NULL) return (uint32_t)-1;
  *b = v3d_batch_create(*va, *ua, *s);
  return idx;
})

def(GFX, 95, {
  v3d_batch *b = pool_elm(&batches, r0);
  if (b == NULL) return (uint32_t)-2;
  v3d_batch_close(b);
  pool_release(&batches, r0);
})

def(GFX, 96, {
  size_t idx;
  v3d_buf *m = pool_alloc(&ias, &idx);
  if (m == NULL) return (uint32_t)-1;
  *m = v3d_idxbuf_create(r0);
  return idx;
})

def(GFX, 97, {
  v3d_buf *m = pool_elm(&ias, r0);
  if (m == NULL) return (uint32_t)-2;
  v3d_idxbuf_copy(m, r1, r2, r3);
})

def(GFX, 111, {
  v3d_buf *m = pool_elm(&ias, r0);
  if (m == NULL) return (uint32_t)-2;
  v3d_idxbuf_close(m);
  pool_release(&ias, r0);
})

#undef def
#undef init
#undef FN

#undef SYSCALLS_DECL
#undef SYSCALLS_IMPL
#undef SYSCALLS_TABLE
#undef SYSCALLS_INIT
