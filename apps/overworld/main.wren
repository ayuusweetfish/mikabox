class App {
  construct new() {
    System.print("hello")
  }

  init() {
    System.print("world")
  }

  update() {
    while (true) {
      System.print("hahaha")
      Fiber.yield(true)
    }
  }
}

System.print("heya")
