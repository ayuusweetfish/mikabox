class Mikabox {
  foreign static btns(player)
}

class App {
  construct new() {
    System.print("world")
  }

  draw() {
    while (true) Fiber.yield(true)
  }

  synth() {
    while (true) Fiber.yield(true)
  }

  event() {
    while (true) Fiber.yield(true)
  }

  update() {
    var i = 0
    while (true) {
      i = i + 1
      System.print("update %(Mikabox.btns(0))")
      Fiber.yield(i % 3 == 0)
    }
  }
}

System.print("hello")

var app = App.new()
var draw = Fiber.new {app.draw()}
var synth = Fiber.new {app.synth()}
var event = Fiber.new {app.event()}
var update = Fiber.new {app.update()}
