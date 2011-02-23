/*
    error.tst
 */

let ejs = App.exePath

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Error stream
    cmd = Cmd("/bin/ls /asdf", {detach: true})
    let msg = new ByteArray
    cmd.on("error", function(event, c) {
        cmd.error.read(msg, -1)
    })
    cmd.wait()
    assert(msg.available > 0)
    assert(msg.toString().contains("ls: /asdf: No such file or directory"))
}
