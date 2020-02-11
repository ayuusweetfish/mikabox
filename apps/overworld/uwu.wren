System.print("uwu")

class App {
  a {
    return _a
  }
}

var synth = Fiber.new {
  while (true) Fiber.yield(true)
}

var event = Fiber.new {
  while (true) Fiber.yield(true)
}

var update = Fiber.new {
  while (true) Fiber.yield(true)
}
