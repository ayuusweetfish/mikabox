System.print("uwu")

var F

class App {
  static f { F }
  static f=(value) { F = value }
}

var synth = Fiber.new {
  while (true) {
    Fiber.yield(true)
  }
}

var event = Fiber.new {
  while (true) Fiber.yield(true)
}
