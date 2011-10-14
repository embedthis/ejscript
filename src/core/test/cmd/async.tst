/*
    async.tst
 */

let ejs = App.exePath.portable

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Async I/O
    cmd = Cmd("echo Hello World", {detach: true})
    let data = new ByteArray
    cmd.on("readable", function(event, c) {
        assert(event == "readable")
        assert(typeOf(c) == "Cmd")
        assert(this == cmd)
        cmd.read(data, -1)
    })
    cmd.finalize()
    assert(cmd.wait(5000))
    assert(data.toString().trim() == "Hello World")
}
