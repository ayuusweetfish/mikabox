#include "v3d_wrapper.h"

void *syscall_dup_mem(uint32_t addr, uint32_t size);
void *syscall_dup_str(uint32_t addr);

#define GLEW_STATIC
#include "GL/glew.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define v3d_printf(...)

// Make sure `float` is 32-bit
typedef char _ensure_float_32[sizeof(float) == 4 ? 1 : -1];

#define vert_sz(__nvar) (8 + 4 * (uint32_t)(__nvar))

void v3d_init()
{
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
  v3d_tex t;
  t.w = w;
  t.h = h;
  glGenTextures(1, &t.id);
  glBindTexture(GL_TEXTURE_2D, t.id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h,
    0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  return t;
}

void v3d_tex_update(v3d_tex *t, uint32_t buf, v3d_tex_fmt_t fmt)
{
  uint8_t *p = syscall_dup_mem(buf, t->w * t->h * 4);
  glBindTexture(GL_TEXTURE_2D, t->id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t->w, t->h,
    GL_RGBA, GL_UNSIGNED_BYTE, p);
  free(p);
}

void v3d_tex_close(v3d_tex *t)
{
  glDeleteTextures(1, &t->id);
}

v3d_vertarr v3d_vertarr_create(uint16_t num, uint8_t num_varyings)
{
  v3d_vertarr a;
  a.num = num;
  a.num_varyings = num_varyings;

  glGenBuffers(1, &a.vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, a.vbo_id);
  glBufferData(GL_ARRAY_BUFFER, num * vert_sz(num_varyings),
    NULL, GL_STREAM_DRAW);
  printf("gen buffer %d\n", (int)a.vbo_id);

  return a;
}

void v3d_vertarr_put(
  v3d_vertarr *a, uint32_t start_index,
  uint32_t verts, uint32_t num
) {
  uint32_t sz = vert_sz(a->num_varyings);
  uint8_t *p = syscall_dup_mem(verts, num * sz);

  glBindBuffer(GL_ARRAY_BUFFER, a->vbo_id);
  glBufferSubData(GL_ARRAY_BUFFER, start_index * sz, num * sz, p);
  free(p);
}

void v3d_vertarr_close(v3d_vertarr *a)
{
  glDeleteBuffers(1, &a->vbo_id);
  printf("del buffer %d\n", (int)a->vbo_id);
}

v3d_unifarr v3d_unifarr_create(uint8_t num)
{
  return (v3d_unifarr){
    .num = num,
    .data = malloc(num * sizeof(struct v3d_unif))
  };
}

void v3d_unifarr_putu32(v3d_unifarr *a, uint32_t index, uint32_t value)
{
  a->data[index] = (struct v3d_unif){
    .is_tex = false,
    .u32 = value
  };
}

void v3d_unifarr_putf32(v3d_unifarr *a, uint32_t index, float value)
{
  a->data[index] = (struct v3d_unif){
    .is_tex = false,
    .f32 = value
  };
}

void v3d_unifarr_puttex(v3d_unifarr *a, uint32_t index, v3d_tex tex, uint8_t cfg)
{
  a->data[index] = (struct v3d_unif){
    .is_tex = true,
    .tex_id = tex.id
  };
  a->data[index + 1] = (struct v3d_unif){
    .is_tex = true,
    .tex_cfg = cfg
  };
}

void v3d_unifarr_close(v3d_unifarr *a)
{
  free(a->data);
}

#define GLSL(__source) "#version 330 core\n" #__source

static const char *shader_source[][2] = {
  // #C
  { GLSL(
    in vec2 screen_pos;
    in vec3 chroma;
    out vec3 chroma_f;
    void main() {
      gl_Position = vec4(
        (screen_pos.x - 400) / 400,
        (240 - screen_pos.y) / 240,
        0.0, 1.0);
      chroma_f = chroma;
    }
  ), GLSL(
    in vec3 chroma_f;
    out vec4 ooo;
    void main() {
      ooo = vec4(chroma_f, 1.0);
    }
  ) },
  // #CA
  { GLSL(
    in vec2 screen_pos;
    in vec4 chroma;
    out vec4 chroma_f;
    void main() {
      gl_Position = vec4(
        (screen_pos.x - 400) / 400,
        (240 - screen_pos.y) / 240,
        0.0, 1.0);
      chroma_f = chroma;
    }
  ), GLSL(
    in vec4 chroma_f;
    out vec4 ooo;
    void main() {
      ooo = chroma_f;
    }
  ) },
  // #T
  { GLSL(
    in vec2 screen_pos;
    in vec2 tex_pos;
    out vec2 tex_pos_f;
    void main() {
      gl_Position = vec4(
        (screen_pos.x - 400) / 400,
        (240 - screen_pos.y) / 240,
        0.0, 1.0);
      tex_pos_f = tex_pos;
    }
  ), GLSL(
    in vec2 tex_pos_f;
    out vec4 ooo;
    uniform sampler2D tex;
    void main() {
      ooo = texture(tex, tex_pos_f);
      ooo.a = 1.0;
    }
  ) },
  // #TCA
  { GLSL(
    in vec2 screen_pos;
    in vec2 tex_pos;
    in vec4 chroma;
    out vec2 tex_pos_f;
    out vec4 chroma_f;
    void main() {
      gl_Position = vec4(
        (screen_pos.x - 400) / 400,
        (240 - screen_pos.y) / 240,
        0.0, 1.0);
      tex_pos_f = tex_pos;
      chroma_f = chroma;
    }
  ), GLSL(
    in vec2 tex_pos_f;
    in vec4 chroma_f;
    out vec4 ooo;
    uniform sampler2D tex;
    void main() {
      ooo = texture(tex, tex_pos_f) * chroma_f;
    }
  ) },
};

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


v3d_shader v3d_shader_create(uint32_t code)
{
  v3d_shader s;
  char *c = syscall_dup_str(code);

  int idx = -1;
  if (strcmp(c, "#C") == 0) idx = 0;
  else if (strcmp(c, "#CA") == 0) idx = 1;
  else if (strcmp(c, "#T") == 0) idx = 2;
  else if (strcmp(c, "#TCA") == 0) idx = 3;

  if (idx != -1) {
    s.type = idx;
    s.vsid = load_shader(GL_VERTEX_SHADER, shader_source[idx][0]);
    s.fsid = load_shader(GL_FRAGMENT_SHADER, shader_source[idx][1]);
  } else {
    printf("> <\n");
    s.type = 0xff;
  }

  free(c);
  return s;
}

v3d_buf v3d_idxbuf_create(uint32_t count)
{
  v3d_buf m;
  glGenBuffers(1, &m.id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * 2,
    NULL, GL_STREAM_DRAW);
  printf("gen buffer %d\n", (int)m.id);
  return m;
}

void v3d_idxbuf_copy(v3d_buf *m, uint32_t pos, uint32_t ptr, uint32_t count)
{
  uint8_t *p = syscall_dup_mem(ptr, count * 2);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->id);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, pos * 2, count * 2, p);
  free(p);
}

void v3d_idxbuf_close(v3d_buf *m)
{
  glDeleteBuffers(1, &m->id);
  printf("del buffer %d\n", (int)m->id);
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
  b.unifarr = unifarr;

  // Program setup
  b.prog_id = glCreateProgram();
  glAttachShader(b.prog_id, shader.vsid);
  glAttachShader(b.prog_id, shader.fsid);
  glLinkProgram(b.prog_id);
  glBindFragDataLocation(b.prog_id, 0, "ooo");

  // VAO setup
  glGenVertexArrays(1, &b.vao_id);
  glBindVertexArray(b.vao_id);
  printf("gen vao %d\n", (int)b.vao_id);

  glBindBuffer(GL_ARRAY_BUFFER, vertarr.vbo_id);

  GLuint index;
  uintptr_t count = 0;

#define add_attr(__name, __type, __size) do { \
  index = glGetAttribLocation(b.prog_id, (__name)); \
  glEnableVertexAttribArray(index); \
  glVertexAttribPointer(index, (__size), (__type), \
    GL_FALSE, vert_sz(vertarr.num_varyings), \
    (GLvoid *)(count * 4)); \
  count += (__size); \
} while (0)

  add_attr("screen_pos", GL_FLOAT, 2);

  // TODO: Make this extensible
  if (shader.type >= 2)
    add_attr("tex_pos", GL_FLOAT, 2);
  if (shader.type <= 1 || shader.type == 3)
    add_attr("chroma", GL_FLOAT, (shader.type == 0 ? 3 : 4));

  glBindVertexArray(0);

  return b;
}

void v3d_batch_close(v3d_batch *b)
{
  glDeleteVertexArrays(1, &b->vao_id);
  glDeleteProgram(b->prog_id);
  printf("del vao %d\n", (int)b->vao_id);
}

v3d_ctx v3d_ctx_create()
{
}

void v3d_ctx_anew(v3d_ctx *c, v3d_tex target, uint32_t clear)
{
  c->target = target;

  glClearColor(
    ((clear >> 16) & 0xff) / 255.0f,
    ((clear >>  8) & 0xff) / 255.0f,
    ((clear >>  0) & 0xff) / 255.0f,
    ((clear >> 24) & 0xff) / 255.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void v3d_ctx_use_batch(v3d_ctx *c, const v3d_batch *batch)
{
  // Use vertex and shader states
  glBindVertexArray(batch->vao_id);
  glUseProgram(batch->prog_id);

  // Activate textures
  for (int i = 0; i < batch->unifarr.num; i++) {
    printf("%d %s %u\n", i,
      batch->unifarr.data[i].is_tex ? "tex" : "norm",
      batch->unifarr.data[i].u32);
    if (batch->unifarr.data[i].is_tex) {
      // TODO: Support multiple textures
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, batch->unifarr.data[i].tex_id);
      glUniform1i(glGetUniformLocation(batch->prog_id, "tex"), 0);
      i++;
      // TODO: Border colour for clamp-to-border warp mode
      uint8_t cfg = batch->unifarr.data[i].tex_cfg;
      GLint arg;
      switch (cfg & 3) {
        case 0: arg = GL_REPEAT; break;
        case 1: arg = GL_CLAMP_TO_EDGE; break;
        case 2: arg = GL_MIRRORED_REPEAT; break;
        case 3: arg = GL_CLAMP_TO_BORDER; break;
      }
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, arg);
      switch ((cfg >> 2) & 3) {
        case 0: arg = GL_REPEAT; break;
        case 1: arg = GL_CLAMP_TO_EDGE; break;
        case 2: arg = GL_MIRRORED_REPEAT; break;
        case 3: arg = GL_CLAMP_TO_BORDER; break;
      }
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, arg);
      switch ((cfg >> 4) & 7) {
        case 0: default: arg = GL_LINEAR; break;
        case 1: arg = GL_NEAREST; break;
        case 2: arg = GL_NEAREST_MIPMAP_NEAREST; break;
        case 3: arg = GL_NEAREST_MIPMAP_LINEAR; break;
        case 4: arg = GL_LINEAR_MIPMAP_NEAREST; break;
        case 5: arg = GL_LINEAR_MIPMAP_LINEAR; break;
      }
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, arg);
      switch ((cfg >> 7) & 7) {
        case 0: default: arg = GL_LINEAR; break;
        case 1: arg = GL_NEAREST; break;
      }
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, arg);
    } else {
    }
  }
}

void v3d_ctx_add_call(v3d_ctx *c, const v3d_call *call)
{
  if (call->is_indexed) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, call->indices.id);
    glDrawElements(GL_TRIANGLES, call->num_verts, GL_UNSIGNED_SHORT, NULL);
  } else {
    glDrawArrays(GL_TRIANGLES, call->start_index, call->num_verts);
  }
}

void v3d_ctx_issue(v3d_ctx *c)
{
}

void v3d_ctx_wait(v3d_ctx *c)
{
  glFlush();
}

void v3d_ctx_close(v3d_ctx *c)
{
}
