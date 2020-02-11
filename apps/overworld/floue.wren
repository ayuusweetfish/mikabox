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
    var vs = List.filled(48, 0)

    for (s in _spots) {
      // Draw a spotlight
      for (i in 0...48) {
        vs[i] = i
      }
    }
    Mikabox.gfxVarrPut(_varr, 0, vs, 48)

    Mikabox.gfxCtxBatch(ctx, _bat)
    Mikabox.gfxCtxCall(ctx, 0, 48, 0)
  }
}
