#include "v3d_wrapper.h"

#define GLEW_STATIC
#include "GL/glew.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define v3d_printf(...)

// Make sure `float` is 32-bit
typedef char _ensure_float_32[sizeof(float) == 4 ? 1 : -1];

void v3d_init()
{
}

v3d_mem v3d_mem_create(uint32_t size, uint32_t align, uint32_t flags)
{
  return (v3d_mem){ .ptr = malloc(size) };
}

void v3d_mem_lock(v3d_mem *m)
{
}

void v3d_mem_unlock(v3d_mem *m)
{
}

void v3d_mem_close(v3d_mem *m)
{
  free(m->ptr);
}

void v3d_mem_copy(v3d_mem *m, uint32_t offs, void *ptr, uint32_t size)
{
  memcpy((uint8_t *)m->ptr + offs, ptr, size);
}

v3d_mem v3d_mem_indexbuf(uint32_t count)
{
  return v3d_mem_create(count * 2, 0, 0);
}

void v3d_mem_indexcopy(v3d_mem *m, uint32_t pos, void *ptr, uint32_t count)
{
  v3d_mem_copy(m, pos * 2, ptr, count * 2);
}

v3d_tex v3d_tex_screen(uint32_t buf)
{
  v3d_tex t = { .w = 800, .h = 480 };
  glGenTextures(1, &t.id);
  return t;
}

#define is_screen(__tex)  ((__tex).mem.ptr == NULL)

v3d_tex v3d_tex_create(uint16_t w, uint16_t h)
{
}

void v3d_tex_update(v3d_tex *t, uint8_t *buf, v3d_tex_fmt_t fmt)
{
}

void v3d_tex_close(v3d_tex *tex)
{
}

v3d_vertarr v3d_vertarr_create(uint16_t num, uint8_t num_varyings)
{
  v3d_vertarr a;
  a.num = num;
  a.num_varyings = num_varyings;
  return a;
}

void v3d_vertarr_put(
  v3d_vertarr *a, uint32_t start_index,
  const v3d_vert *verts, uint32_t num
) {
}

void v3d_vertarr_close(v3d_vertarr *a)
{
}

v3d_unifarr v3d_unifarr_create(uint8_t num)
{
}

void v3d_unifarr_putu32(v3d_unifarr *a, uint32_t index, uint32_t value)
{
}

void v3d_unifarr_putf32(v3d_unifarr *a, uint32_t index, float value)
{
}

void v3d_unifarr_puttex(v3d_unifarr *a, uint32_t index, v3d_tex tex, uint8_t cfg)
{
}

#define GLSL(__source) "#version 120\n" #__source

static const char *vs = GLSL(
  attribute vec2 screen_pos;
  void main() {
    gl_Position = vec4(screen_pos, 0.0, 1.0);
  }
);

static const char *fs = GLSL(
  void main() {
    gl_FragColor = vec4(0.5, 0.6, 1.0, 1.0);
  }
);

static inline GLuint load_shader(GLenum type, const char *source)
{
  GLuint shader_id = glCreateShader(type);
  glShaderSource(shader_id, 1, &source, NULL);
  glCompileShader(shader_id);

  GLint status;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
  char msg_buf[1024];
  glGetShaderInfoLog(shader_id, sizeof(msg_buf) - 1, NULL, msg_buf);
  fprintf(stderr, "OvO  Compilation log for %s shader\n",
    (type == GL_VERTEX_SHADER ? "vertex" :
     type == GL_FRAGMENT_SHADER ? "fragment" : "unknown (!)"));
  fputs(msg_buf, stderr);
  fprintf(stderr, "=v=  End\n");
  if (status != GL_TRUE) {
    fprintf(stderr, "> <  Shader compilation failed\n");
    return 0;
  }

  return shader_id;
}


v3d_shader v3d_shader_create(const char *code)
{
  v3d_shader s;
  s.vsid = load_shader(GL_VERTEX_SHADER, vs);
  s.fsid = load_shader(GL_FRAGMENT_SHADER, fs);
  return s;
}

void v3d_shader_close(v3d_shader *s)
{
  glDeleteShader(s->vsid);
  glDeleteShader(s->fsid);
}

v3d_batch v3d_batch_create(
  const v3d_vertarr vertarr,
  const v3d_unifarr unifarr,
  const v3d_shader shader
) {
  v3d_batch b;
  b.id = glCreateProgram();
  glAttachShader(b.id, shader.vsid);
  glAttachShader(b.id, shader.fsid);
  glLinkProgram(b.id);
/*
  GLint result;
  glGetProgramiv(b.id, GL_LINK_STATUS, &result);
  printf("!! %d\n", result);
*/
}

void v3d_batch_close(v3d_batch *b)
{
  glDeleteProgram(b->id);
}

v3d_ctx v3d_ctx_create()
{
}

void v3d_ctx_anew(v3d_ctx *c, v3d_tex target, uint32_t clear)
{
  c->target = target;
  c->clear = clear;
}

void v3d_ctx_use_batch(v3d_ctx *c, const v3d_batch *batch)
{
  glUseProgram(batch->id);
}

void v3d_ctx_add_call(v3d_ctx *c, const v3d_call *call)
{
}

void v3d_ctx_issue(v3d_ctx *c)
{
  glClearColor(
    ((c->clear >> 16) & 0xff) / 255.0f,
    ((c->clear >>  8) & 0xff) / 255.0f,
    ((c->clear >>  0) & 0xff) / 255.0f,
    ((c->clear >> 24) & 0xff) / 255.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void v3d_ctx_wait(v3d_ctx *c)
{
  glFlush();
}

void v3d_ctx_close(v3d_ctx *c)
{
}
