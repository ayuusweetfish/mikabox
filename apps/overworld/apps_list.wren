import "mikabox" for Mikabox
import "stb" for Stb

class AppRecord {
  construct new(name) {
    _tex = null
    for (ext in ["png", "jpg", "gif"]) {
      var path = "/apps/%(name)/icon.%(ext)"
      if (Mikabox.filStat(path) == 1) {
        var image = Stb.loadImage(path)
        if (image != null) {
          _tex = image[0]
          break
        }
      }
    }
    if (_tex == null) {
      var image = Stb.loadImage("/res/default_icon.png")
      _tex = image[0]
    }

    _varr = Mikabox.gfxVarrCreate(4, 2)
    _uarr = Mikabox.gfxUarrCreate(2)
    _shad = Mikabox.gfxShadCreate("#T")
    _bat = Mikabox.gfxBatCreate(_varr, _uarr, _shad)

    Mikabox.gfxUarrPuttex(_uarr, 0, _tex, 0)

    _vs = List.filled(4 * 4, 0)
    for (i in 0..1) {
      for (j in 0..1) {
        var base = (i * 2 + j) * 4
        _vs[base + 2] = i
        _vs[base + 3] = j
      }
    }

    _iarr = Mikabox.gfxIarrCreate(6)
    Mikabox.gfxIarrPut(_iarr, 0, [0, 1, 2, 1, 2, 3], 6)
  }

  draw(ctx, x, y) {
    for (i in 0..1) {
      for (j in 0..1) {
        var base = (i * 2 + j) * 4
        _vs[base + 0] = x + 120 * i
        _vs[base + 1] = y + 120 * j
      }
    }
    Mikabox.gfxVarrPut(_varr, 0, _vs, 4)

    Mikabox.gfxCtxBatch(ctx, _bat)
    Mikabox.gfxCtxCall(ctx, 1, 6, _iarr)
  }
}

class AppsList {
  construct new() {
    _apps = []

    var dir = Mikabox.filOpendir("/apps")
    while (1) {
      var ls = []
      var n = Mikabox.filReaddir(dir, ls)
      if (n == 0) break
      if (n == 2) {
        var name = ls[0]
        if (Mikabox.filStat("/apps/%(name)/a.out") == 1) {
          System.print(name)
          _apps.add(AppRecord.new(name))
        }
      }
    }
    Mikabox.filClosedir(dir)
  }

  draw(ctx) {
    for (i in 0..._apps.count) {
      var a = _apps[i]
      a.draw(ctx, 0, i * 180)
    }
  }
}
