// Test logger

/*
var app: Logger = new Logger("app", App.logger, 5)
var sub: Logger = new Logger("sub", app)

//  app.filter = function(log: Logger, level: Number, msg: String): Boolean {
//      return (log.name == "ejs.app") && level < 2
//  }
//  app.match = /(.app.sub)|(.app.sub2)/

App.logger.log(0, "Default level")
app.log(4, "App level")
sub.log(2, "Sub level")
sub.log(0, "Zero")
sub.warn(Logger.Warn, "Warn")
*/


stdout.write("Hello")
stderr.write("\n")
stderr.writeLine("Hello")
stderr.writeLine("abc", "def", "ghi")

while (line = stdin.readLine()) {
    print("LINE \"" + line + "\"")
}
