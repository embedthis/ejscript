/*
    async.tst
 */

let ejs = App.exePath

if (!Path("/usr/bin/true").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Async I/O
    cmd = Cmd("/bin/echo Hello World", {detach: true})
    response = new ByteArray
    cmd.on("readable", function(event, c) {
        assert(event == "readable")
        assert(typeOf(c) == "Cmd")
        assert(this == global)
        cmd.read(response, -1)
    })
    cmd.wait()
    assert(response.toString().trim() == "Hello World")
}
