import "mikabox" for Mikabox
import "stb" for Stb

class AppsList {
  construct new() {
    var dir = Mikabox.filOpendir("/apps")
    while (1) {
      var ls = []
      var n = Mikabox.filReaddir(dir, ls)
      if (n == 0) break
      if (n == 2) {
        var name = ls[0]
        if (Mikabox.filStat("/apps/%(name)/a.out") == 1) {
          System.print(name)
        }
      }
    }
    Mikabox.filClosedir(dir)

    _varr = Mikabox.gfxVarrCreate(4, 2)
    _uarr = Mikabox.gfxUarrCreate(2)
    _shad = Mikabox.gfxShadCreate("#T")
    _bat = Mikabox.gfxBatCreate(_varr, _uarr, _shad)

    var image = Stb.loadImage("/apps/tetris/1.jpg")
    _tex = image[0]
    var w = image[1]
    var h = image[2]
    Mikabox.gfxUarrPuttex(_uarr, 0, _tex, 0)

    _vs = List.filled(4 * 4, 0)
    for (i in 0..1) {
      for (j in 0..1) {
        var base = (i * 2 + j) * 4
        _vs[base + 0] = 100 + w * i * 0.5
        _vs[base + 1] = 100 + h * j * 0.5
        _vs[base + 2] = i
        _vs[base + 3] = j
      }
    }

    _iarr = Mikabox.gfxIarrCreate(6)
    Mikabox.gfxIarrPut(_iarr, 0, [0, 1, 2, 1, 2, 3], 6)
  }

  draw(ctx) {
    Mikabox.gfxVarrPut(_varr, 0, _vs, 4)

    Mikabox.gfxCtxBatch(ctx, _bat)
    Mikabox.gfxCtxCall(ctx, 1, 6, _iarr)
  }
}
