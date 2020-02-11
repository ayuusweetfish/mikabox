import "mikabox" for Mikabox

var H = 480.0 / 800.0

var Rand = Fn.new {|a, b| Mikabox.rand() / 18446744073709551616 * (b - a) + a }

class FloueSpotlight {
  // x, y - coordinate
  // r - radius
  // c - chroma
  construct new(x, y, r, c) {
    _x = x
    _y = y
    _r = r
    _c = c
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
    if (_y < -_r) _y = _y + (1 + _r * 2)
    if (_y > H + _r) _y = _y - (H + _r * 2)
  }

  x { _x }
  y { _y }
  r { _r }
}

class Floue {
  construct new(n) {
    _spots = []
    for (i in 0...n) {
      _spots.add(FloueSpotlight.new(0.5, 0.5, 0.1, 0xffffffff))
    }

    _varr = Mikabox.gfxVarrCreate(48, 4)
    _uarr = Mikabox.gfxUarrCreate(0)
    _shad = Mikabox.gfxShadCreate("#CA")
    _bat = Mikabox.gfxBatCreate(_varr, _uarr, _shad)
  }

  tick(dt) {
    for (s in _spots) s.tick(dt)
  }

  draw(ctx) {
    var vs = List.filled(49 * 6, 0)

    for (s in _spots) {
      // Draw a spotlight
      var x = s.x
      var y = s.y
      var r = s.r * 800
      vs[0] = x * 800
      vs[1] = y * 480
      vs[2] = 0
      vs[3] = 0
      vs[4] = 0
      vs[5] = 1
      for (i in 1..48) {
        var a = Num.pi * 2 / 48 * i
        vs[i * 6 + 0] = x * 800 + r * a.cos
        vs[i * 6 + 1] = y * 480 + r * a.sin
        vs[i * 6 + 2] = 0
        vs[i * 6 + 3] = 0
        vs[i * 6 + 4] = 0
        vs[i * 6 + 5] = 1
      }
    }
    Mikabox.gfxVarrPut(_varr, 0, vs, 48)

    Mikabox.gfxCtxBatch(ctx, _bat)
    Mikabox.gfxCtxCall(ctx, 0, 48, 0)
  }
}
