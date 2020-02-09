#ifndef _Mikabox_mikabox_h_
#define _Mikabox_mikabox_h_

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

// Constants

#define BTN_U     (1 << 0)
#define BTN_D     (1 << 1)
#define BTN_L     (1 << 2)
#define BTN_R     (1 << 3)
#define BTN_A     (1 << 4)
#define BTN_B     (1 << 5)
#define BTN_X     (1 << 6)
#define BTN_Y     (1 << 7)
#define BTN_L1    (1 << 8)
#define BTN_R1    (1 << 9)
#define BTN_L2    (1 << 10)
#define BTN_R2    (1 << 11)
#define BTN_L3    (1 << 12)
#define BTN_R3    (1 << 13)
#define BTN_START (1 << 14)
#define BTN_OPTN  (1 << 15)
#define BTN_META  (1 << 16)
#define BTN_AUX   (1 << 17)

#define BTN_CRO   BTN_A
#define BTN_CIR   BTN_B
#define BTN_SQR   BTN_X
#define BTN_TRI   BTN_Y

enum {
  v3d_tex_fmt_rgb = 0,
  v3d_tex_fmt_bgr,
  v3d_tex_fmt_rgba,
  v3d_tex_fmt_bgra,
  v3d_tex_fmt_argb,
  v3d_tex_fmt_abgr,
};

#define v3d_wrap_s_repeat   (0 << 0)
#define v3d_wrap_s_clamp    (1 << 0)
#define v3d_wrap_s_mirror   (2 << 0)
#define v3d_wrap_s_border   (3 << 0)
#define v3d_wrap_t_repeat   (0 << 2)
#define v3d_wrap_t_clamp    (1 << 2)
#define v3d_wrap_t_mirror   (2 << 2)
#define v3d_wrap_t_border   (3 << 2)
#define v3d_minfilt_linear  (0 << 4)
#define v3d_minfilt_nearest (1 << 4)
#define v3d_minfilt_n_mip_n (2 << 4)
#define v3d_minfilt_n_mip_l (3 << 4)
#define v3d_minfilt_l_mip_n (4 << 4)
#define v3d_minfilt_l_mip_l (5 << 4)
#define v3d_magfilt_linear  (0 << 7)
#define v3d_magfilt_nearest (1 << 7)

#define FA_READ           0x01
#define FA_WRITE          0x02
#define FA_OPEN_EXISTING  0x00
#define FA_CREATE_NEW     0x04
#define FA_CREATE_ALWAYS  0x08
#define FA_OPEN_ALWAYS    0x10
#define FA_OPEN_APPEND    0x30

// System call fundamentals

uint64_t syscall0(uint32_t);
uint64_t syscall1(uint32_t, uint32_t);
uint64_t syscall2(uint32_t, uint32_t, uint32_t);
uint64_t syscall3(uint32_t, uint32_t, uint32_t, uint32_t);
uint64_t syscall4(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#define VA_GET(_0, _1, _2, _3, _4, _N, ...) (_N)
#define VA_ARGC(...) VA_GET(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define VA_SYSCALL(_argc, _0, _1, _2, _3, _4, ...) \
  ((_argc) == 0 ? syscall0((uint32_t)(_0)) : \
   (_argc) == 1 ? syscall1((uint32_t)(_1), (uint32_t)(_0)) : \
   (_argc) == 2 ? syscall2((uint32_t)(_1), (uint32_t)(_2), (uint32_t)(_0)) : \
   (_argc) == 3 ? syscall3((uint32_t)(_1), (uint32_t)(_2), (uint32_t)(_3), (uint32_t)(_0)) : \
   (_argc) == 4 ? syscall4((uint32_t)(_1), (uint32_t)(_2), (uint32_t)(_3), (uint32_t)(_4), (uint32_t)(_0)) : 0)
#define syscall(...) \
  VA_SYSCALL(VA_ARGC(__VA_ARGS__) - 1, __VA_ARGS__, 0, 0, 0, 0)

// System call definitions

#define offs_mika   0
#define offs_gfx  256
#define offs_fil  512
#define offs_aud  768
#define offs_ovw 3840

#define argc_(_, _1, _2, _3, _4, _N, ...) _N
#define fn__(_pfx, _argc) _pfx##_argc
#define fn_(_pfx, _argc) fn__(_pfx, _argc)
#define fn_argc(_pfx, ...) \
  fn_(_pfx, argc_(_, ##__VA_ARGS__, 4, 3, 2, 1, 0))

#define sep__  void,
#define sep_i  uint32_t,
#define sep_l  uint64_t,
#define sep_p  void *,
#define sep_cp const void *,

#define cdr_(_h, _t) _t
#define cdr(...) cdr_(__VA_ARGS__)
#define name(_arg) cdr(sep_##_arg)
#define decl__(_ty, _name) _ty _name
#define decl_(...) decl__(__VA_ARGS__)
#define decl(_arg) decl_(sep_##_arg)

#define def_sig_body_0()
#define def_sig_body_1(_arg) decl(_arg)
#define def_sig_body_2(_arg, ...) decl(_arg), def_sig_body_1(__VA_ARGS__)
#define def_sig_body_3(_arg, ...) decl(_arg), def_sig_body_2(__VA_ARGS__)
#define def_sig_body_4(_arg, ...) decl(_arg), def_sig_body_3(__VA_ARGS__)

#define def_argnames_0()
#define def_argnames_1(_arg) , name(_arg)
#define def_argnames_2(_arg, ...) , name(_arg) def_argnames_1(__VA_ARGS__)
#define def_argnames_3(_arg, ...) , name(_arg) def_argnames_2(__VA_ARGS__)
#define def_argnames_4(_arg, ...) , name(_arg) def_argnames_3(__VA_ARGS__)

#define def_sig_decl_(_group, _ty, _name) _ty _group##_##_name
#define def_sig_decl(...) def_sig_decl_(__VA_ARGS__)
#define def_ret_(_ty, _name) return (_ty) syscall
#define def_ret(...) def_ret_(__VA_ARGS__)

#define def__(_group, _num, _name, ...) \
  static inline def_sig_decl(_group, sep_##_name) ( \
    fn_argc(def_sig_body_, ##__VA_ARGS__) (__VA_ARGS__) \
  ) { \
    def_ret(sep_##_name) (offs_##_group + _num \
      fn_argc(def_argnames_, ##__VA_ARGS__) (__VA_ARGS__)); \
  }
#define def_(_group, ...) def__(_group, __VA_ARGS__)
#define def(...) def_(_group, __VA_ARGS__)

#define _group mika

def(0, _ rout, cp c1, cp c2, cp c3, cp c4)
def(1, _ yield, i clear)
def(2, l tick)
def(3, i party)
def(4, l btns, i player)
def(5, l axes, i player)
def(6, l rand)
def(7, _ log, i level, cp str)

def(128, l wall)

#undef _group
#define _group gfx

def( 0, i ctx_create)
def( 1, _ ctx_reset, i ctx, i target, i clear)
def( 2, _ ctx_batch, i ctx, i bat)
def( 3, _ ctx_call, i ctx, i is_indexed, i num_verts, i start_or_idxbuf)
def( 4, _ ctx_issue, i ctx)
def( 5, _ ctx_wait, i ctx)
def(15, _ ctx_close, i ctx)

def(16, i tex_create, i width, i height)
def(17, _ tex_update, i tex, cp pixels, i format)
def(18, i tex_screen)
def(31, _ tex_close, i tex)

def(32, i varr_create, i num_verts, i num_varyings)
def(33, _ varr_put, i varr, i start, cp verts, i num)
def(47, _ varr_close, i varr)

def(48, i uarr_create, i num_uniforms)
def(49, _ uarr_putu32, i uarr, i index, i value)
def(50, _ uarr_puttex, i uarr, i index, i tex, i config)
def(63, _ uarr_close, i uarr);

def(64, i shad_create, cp code)
def(79, _ shad_close, i shad)

def(80, i bat_create, i varr, i uarr, i shad)
def(95, _ bat_close, i bat)

def(96, i iarr_create, i num)
def(97, i iarr_put, i iarr, i start, cp idxs, i num)
def(111, i iarr_close, i iarr)

#undef _group
#define _group fil

def( 0, i open, cp path, i flags)
def( 1, _ close, i f)
def( 2, i read, i f, p buf, i len)
def( 3, i write, i f, cp buf, i len)
def( 4, _ seek, i f, i pos)
def( 5, _ trunc, i f)
def( 6, _ flush, i f)
def( 7, i tell, i f)
def( 8, i eof, i f)
def( 9, i size, i f)
def(10, i err, i f)
def(16, i opendir, cp path)
def(17, _ closedir, i d)
def(18, i readdir, i d, p name)
def(32, i stat, cp path)
def(33, _ unlink, cp path)
def(34, _ rename, cp path_old, cp path_new)
def(35, _ mkdir, cp path)

#undef _group
#define _group aud

def(0, i blocksize)
def(1, i dropped)
def(2, _ write, p buf)

#undef _group
#define _group ovw

def(0, i start, cp path)
def(1, _ stop)
def(2, i paused)
def(3, _ resume)

#undef _group

#undef offs_mika
#undef offs_gfx
#undef offs_fil
#undef offs_aud
#undef offs_ovw

#undef argc_
#undef fn__
#undef fn_
#undef fn_argc

#undef sep__
#undef sep_i
#undef sep_l
#undef sep_p
#undef sep_cp

#undef cdr_
#undef cdr
#undef name
#undef decl__
#undef decl_
#undef decl

#undef def_sig_body_0
#undef def_sig_body_1
#undef def_sig_body_2
#undef def_sig_body_3
#undef def_sig_body_4

#undef def_argnames_0
#undef def_argnames_1
#undef def_argnames_2
#undef def_argnames_3
#undef def_argnames_4

#undef def_sig_decl_
#undef def_sig_decl
#undef def_ret_
#undef def_ret

#undef def__
#undef def_
#undef def

// Utilities

static inline void mika_printf(const char *format, ...)
{
  char s[256];
  va_list arg;
  va_start(arg, format);
  vsnprintf(s, sizeof s, format, arg);
  va_end(arg);
  mika_log(0, s);
}

#define printf mika_printf

#endif
