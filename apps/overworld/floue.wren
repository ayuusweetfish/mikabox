import "mikabox" for Mikabox

var H = 480.0 / 800.0

var Rand = Fn.new {|a, b| Mikabox.rand() / 18446744073709551616 * (b - a) + a }

class FloueSpotlight {
  // x, y - coordinate
  // r - radius
  // c - chroma
  construct new(x, y, r, cr, cg, cb) {
    _x = x
    _y = y
    _r = r
    _cr = cr
    _cg = cg
    _cb = cb
    _v = Rand.call(1 / 128, 1 / 32)
    _a = Rand.call(-Num.pi, Num.pi)
    _b = 0
  }

  tick(dt) {
    _b = _b + Rand.call(-0.3, 0.3) * dt
    if (_b < -0.5) _b = -0.5
    if (_b > 0.5) _b = 0.5
    _a = _a + _b * dt

    _x = _x + _v * _a.cos * dt
    _y = _y + _v * _a.sin * dt
    if (_x < -_r) _x = _x + (1 + _r * 2)
    if (_x > 1 + _r) _x = _x - (1 + _r * 2)
    if (_y < -_r) _y = _y + (H + _r * 2)
    if (_y > H + _r) _y = _y - (H + _r * 2)
  }

  x { _x }
  y { _y }
  r { _r }
  cr { _cr }
  cg { _cg }
  cb { _cb }
}

class Floue {
  construct new(list) {
    _n = list.count

    _spots = []
    for (s in list) {
      _spots.add(FloueSpotlight.new(
        Rand.call(0, 1), Rand.call(0, 1),
        s[0], s[1], s[2], s[3]
      ))
    }

    _varr = Mikabox.gfxVarrCreate(_n * 49, 4)
    _uarr = Mikabox.gfxUarrCreate(0)
    _shad = Mikabox.gfxShadCreate("#CA")
    _bat = Mikabox.gfxBatCreate(_varr, _uarr, _shad)

    _iarr = Mikabox.gfxIarrCreate(_n * 48 * 3)

    var idxs = List.filled(_n * 48 * 3, 0)
    for (i in 0..._n) {
      var base = i * 48 * 3
      var vbase = i * 49
      for (j in 0..47) {
        idxs[base + j * 3 + 0] = vbase + 0
        idxs[base + j * 3 + 1] = vbase + j + 1
        idxs[base + j * 3 + 2] = vbase + (j + 1) % 48 + 1
      }
    }
    Mikabox.gfxIarrPut(_iarr, 0, idxs, _n * 48 * 3)
  }

  tick(dt) {
    for (s in _spots) s.tick(dt)
  }

  draw(ctx) {
    var vs = List.filled(49 * 6, 0)

    for (i in 0..._n) {
      var s = _spots[i]
      // Draw a spotlight
      var x = s.x * 800
      var y = s.y * 800
      var r = s.r * 800
      var cr = s.cr * 0.5
      var cg = s.cg * 0.5
      var cb = s.cb * 0.5
      vs[0] = x
      vs[1] = y
      vs[2] = cr
      vs[3] = cg
      vs[4] = cb
      vs[5] = 0.5
      for (j in 1..48) {
        var a = Num.pi * 2 / 48 * j
        vs[j * 6 + 0] = x + r * a.cos
        vs[j * 6 + 1] = y + r * a.sin
        vs[j * 6 + 2] = cr
        vs[j * 6 + 3] = cg
        vs[j * 6 + 4] = cb
        vs[j * 6 + 5] = 0.5
      }
      Mikabox.gfxVarrPut(_varr, i * 49, vs, 49)
    }

    Mikabox.gfxCtxBatch(ctx, _bat)
    Mikabox.gfxCtxCall(ctx, 1, _n * 48 * 3, _iarr)
  }
}
