#ifndef _Mikabox_syscalls_h_
#define _Mikabox_syscalls_h_

#define offs_mika   0
#define offs_gfx  256
#define offs_fil  512
#define offs_aud  768

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
#define def_ret_(_ty, _name) return (_ty) syscall64
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

def(0, _ rout, i c1, i c2, i c3, i c4)
def(1, _ yield, i clear)
def(2, _ tick)
def(3, i party)
def(4, l btns, i player)
def(5, l axes, i player)
def(6, l rand)
def(7, _ log, i level, cp str)

#undef _group
#define _group gfx

def( 0, i ctx_create)
def( 1, _ ctx_reset, i ctx, i target, i clear)
def( 2, _ ctx_batch, i ctx, i bat)
def( 3, _ ctx_call, i ctx, i is_indexed, i num_verts, i start_or_idxbuf)
def( 4, _ ctx_issue)
def( 5, _ ctx_wait)
def(15, _ ctx_close)

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

#undef offs_mika
#undef offs_gfx
#undef offs_fil
#undef offs_aud

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

#endif
