import "mikabox" for Mikabox
import "uwu"

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
    Mikabox.log(2, "hi")
    while (true) {
      i = i + 1
      Mikabox.log(3, "update %(Mikabox.btns(0))")
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
