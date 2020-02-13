import "mikabox" for Mikabox

class AppsList {
  construct new() {
    var dir = Mikabox.filOpendir("/apps")
    while (1) {
      var ls = []
      var n = Mikabox.filReaddir(dir, ls)
      if (n == 0) break
      if (n == 2) {
        var name = ls[0]
        if (Mikabox.filStat("/apps/%(name)/a.out") == 1) {
          System.print(name)
        }
      }
    }
    Mikabox.filClosedir(dir)
  }
}
