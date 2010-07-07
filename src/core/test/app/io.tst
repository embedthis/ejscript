/*
  	I/O tests for the App class
 */

assert(stdout is TextStream)
assert(stderr is TextStream)
assert(stdin is TextStream)

assert(App.outputStream is Stream)
assert(App.errorStream is Stream)
assert(App.inputStream is Stream)

/*
    stream = App.outputStream
    stream.write("Hello World\n")

    stream = App.errorStream
    stream.write("Error\n")

    ip = new TextStream(App.inputStream)
    while (s = ip.readLine()) {
        print("GOT \"" + s + "\"")
    }
    print(s)
    App.exit(1)
*/
