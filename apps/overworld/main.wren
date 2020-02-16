import "mikabox" for Mikabox
import "uwu" for App, synth, event
import "floue" for Floue
import "apps_list" for AppsList

var list = []
for (i in 0..3) list.add([0.15 + 0.05 * i, 1, 0.6, 0.2])
for (i in 0..3) list.add([0.17 + 0.05 * i, 1, 0.7, 0.1])

App.f = Floue.new(list)

var appsList = AppsList.new()

var draw = Fiber.new {
  var ctx = Mikabox.gfxCtxCreate()
  var t1 = Mikabox.tick()

  while (true) {
    var t2 = Mikabox.tick()
    var dt = (t2 - t1) / 1000000

    App.f.tick(dt)
    t1 = t2

    Mikabox.gfxCtxWait(ctx)
    Mikabox.gfxCtxReset(ctx)
    Mikabox.gfxCtxConfig(ctx,
      Mikabox.gfxTexScreen(),
      Mikabox.btns(0) == 0 ? 0xffffffdd : 0xffffddff)

    App.f.draw(ctx)

    Mikabox.gfxCtxIssue(ctx)
    Fiber.yield(true)
  }
}

var update = Fiber.new {
  while (true) {
    Fiber.yield(true)
  }
}
