import "mikabox" for Mikabox
import "uwu" for App, synth, event, update
import "floue" for Floue

var draw = Fiber.new {
  var ctx = Mikabox.gfxCtxCreate()

  var list = []
  for (i in 0..3) list.add([0.15 + 0.05 * i, 1, 0.6, 0.2])
  for (i in 0..3) list.add([0.17 + 0.05 * i, 1, 0.7, 0.1])
  var f = Floue.new(list)

  while (true) {
    Mikabox.gfxCtxWait(ctx)
    Mikabox.gfxCtxReset(ctx, Mikabox.gfxTexScreen(), 0xffffffdd)

    f.tick(0.016)
    f.draw(ctx)

    Mikabox.gfxCtxIssue(ctx)
    Fiber.yield(true)
  }
}
