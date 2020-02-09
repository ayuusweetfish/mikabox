class App {
  construct new() {
    System.print("world")
  }

  update() {
    var i = 0
    while (true) {
      i = i + 1
      System.print("hahaha %(i)")
      Fiber.yield(true)
    }
  }
}

System.print("hello")

var update = Fiber.new {|a| a.update()}
