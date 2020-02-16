import "mikabox" for Mikabox

var H = 480.0 / 800.0

var Rand = Fn.new {|a, b| Mikabox.rand() / 18446744073709551616 * (b - a) + a }
var Poly = 48

// A spotlight's vertices:
// - 0 ~ (Poly - 1): Polygon; even - inner, odd - outer
// - Poly: Centre

// A spotlight's triangles:
// - Inner: (Cen, i * 2, (i * 2 + 2) % Poly) for i in 0...(Poly / 2)
// - Outer: (i, (i + 1) % Poly, (i + 2) % Poly) for i in 0...Poly

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

    _varr = Mikabox.gfxVarrCreate(_n * (Poly * 2 + 1), 4)
    _uarr = Mikabox.gfxUarrCreate(0)
    _shad = Mikabox.gfxShadCreate("#CA")
    _bat = Mikabox.gfxBatCreate(_varr, _uarr, _shad)

    _iarr = Mikabox.gfxIarrCreate(_n * Poly * 4.5)

    var idxs = List.filled(_n * Poly * 4.5, 0)
    var base = 0
    for (i in 0..._n) {
      var vbase = i * (Poly + 1)
      for (j in 0...(Poly / 2)) {
        idxs[base + 0] = vbase + Poly
        idxs[base + 1] = vbase + j * 2
        idxs[base + 2] = vbase + (j * 2 + 2) % Poly
        base = base + 3
      }
      for (j in 0...Poly) {
        idxs[base + 0] = vbase + j
        idxs[base + 1] = vbase + (j + 1) % Poly
        idxs[base + 2] = vbase + (j + 2) % Poly
        base = base + 3
      }
    }
    Mikabox.gfxIarrPut(_iarr, 0, idxs, _n * Poly * 4.5)

    _vs = List.filled(_n * (Poly + 1) * 6, 0)

    base = 0
    for (i in 0..._n) {
      var s = _spots[i]
      var alpha
      for (j in 0..Poly) {
        if (j % 2 == 0) alpha = 0.3 else alpha = 0
        _vs[base + 2] = s.cr * alpha
        _vs[base + 3] = s.cg * alpha
        _vs[base + 4] = s.cb * alpha
        _vs[base + 5] = alpha
        base = base + 6
      }
    }

    _cosTable = []
    _sinTable = []
    for (i in 0...Poly) {
      _cosTable.add((Num.pi * 2 / Poly * i).cos)
      _sinTable.add((Num.pi * 2 / Poly * i).sin)
    }
  }

  tick(dt) {
    for (s in _spots) s.tick(dt)
  }

  draw(ctx) {
    for (i in 0..._n) {
      var s = _spots[i]
      // Draw a spotlight
      var base = (Poly + 1) * 6 * i
      var x = s.x * 800
      var y = s.y * 800
      var r
      for (j in 0...Poly) {
        if (j % 2 == 0) r = s.r * 800 - 50 else r = s.r * 800
        _vs[base + 0] = x + r * _cosTable[j]
        _vs[base + 1] = y + r * _sinTable[j]
        base = base + 6
      }
      _vs[base + 0] = x
      _vs[base + 1] = y
    }

    Mikabox.gfxVarrPut(_varr, 0, _vs, _n * (Poly + 1))
  }

  addCommands(ctx) {
    Mikabox.gfxCtxBatch(ctx, _bat)
    Mikabox.gfxCtxCall(ctx, 1, _n * Poly * 4.5, _iarr)
  }
}
