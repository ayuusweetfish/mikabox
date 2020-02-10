class Mikabox {
  foreign static rout(c1, c2, c3, c4)
  foreign static yield(clear)
  foreign static tick()
  foreign static party()
  foreign static btns(player)
  foreign static axes(player)
  foreign static rand()
  foreign static log(level, str)
  foreign static test()
  foreign static wall()
  foreign static gfxCtxCreate()
  foreign static gfxCtxReset(ctx, target, clear)
  foreign static gfxCtxBatch(ctx, bat)
  foreign static gfxCtxCall(ctx, is_indexed, num_verts, start_or_idxbuf)
  foreign static gfxCtxIssue(ctx)
  foreign static gfxCtxWait(ctx)
  foreign static gfxCtxClose(ctx)
  foreign static gfxTexCreate(width, height)
  foreign static gfxTexUpdate(tex, pixels, format)
  foreign static gfxTexScreen()
  foreign static gfxTexClose(tex)
  foreign static gfxVarrCreate(num_verts, num_varyings)
  foreign static gfxVarrPut(varr, start, verts, num)
  foreign static gfxVarrClose(varr)
  foreign static gfxUarrCreate(num_uniforms)
  foreign static gfxUarrPutu32(uarr, index, value)
  foreign static gfxUarrPuttex(uarr, index, tex, config)
  foreign static gfxUarrClose(uarr)
  foreign static gfxShadCreate(code)
  foreign static gfxShadClose(shad)
  foreign static gfxBatCreate(varr, uarr, shad)
  foreign static gfxBatClose(bat)
  foreign static gfxIarrCreate(num)
  foreign static gfxIarrPut(iarr, start, idxs, num)
  foreign static gfxIarrClose(iarr)
  foreign static filOpen(path, flags)
  foreign static filClose(f)
  foreign static filRead(f, buf, len)
  foreign static filWrite(f, buf, len)
  foreign static filSeek(f, pos)
  foreign static filTrunc(f)
  foreign static filFlush(f)
  foreign static filTell(f)
  foreign static filEof(f)
  foreign static filSize(f)
  foreign static filErr(f)
  foreign static filOpendir(path)
  foreign static filClosedir(d)
  foreign static filReaddir(d, name)
  foreign static filStat(path)
  foreign static filUnlink(path)
  foreign static filRename(path_old, path_new)
  foreign static filMkdir(path)
  foreign static audBlocksize()
  foreign static audDropped()
  foreign static audWrite(buf)
  foreign static ovwStart(path)
  foreign static ovwStop()
  foreign static ovwPaused()
  foreign static ovwResume()
}

class App {
  construct new() {
    System.print("world")
  }

  draw() {
    while (true) Fiber.yield(true)
  }

  synth() {
    while (true) Fiber.yield(true)
  }

  event() {
    while (true) Fiber.yield(true)
  }

  update() {
    var i = 0
    while (true) {
      i = i + 1
      System.print("update %(Mikabox.btns(0))")
      Fiber.yield(i % 3 == 0)
    }
  }
}

System.print("hello")

var app = App.new()
var draw = Fiber.new {app.draw()}
var synth = Fiber.new {app.synth()}
var event = Fiber.new {app.event()}
var update = Fiber.new {app.update()}
