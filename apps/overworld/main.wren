import "mikabox" for Mikabox
import "uwu" for App, synth, event, update
import "floue" for Floue

var draw = Fiber.new {
  var ctx = Mikabox.gfxCtxCreate()
  var f = Floue.new(1)
  while (true) {
    Mikabox.gfxCtxWait(ctx)
    Mikabox.gfxCtxReset(ctx, Mikabox.gfxTexScreen(), 0xffffffdd)

    f.tick(0.016)
    f.draw(ctx)

    Mikabox.gfxCtxIssue(ctx)
    Fiber.yield(true)
  }
}
