/*
    error.tst
 */

let ejs = Cmd.locate('ejs')

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  error response property
    cmd = Cmd("ls /asdf", {detach: true})
    let gotError
    cmd.on("error", function(event, c) {
        gotError = true
    })
    cmd.finalize()
    cmd.wait()
    assert(gotError)
    assert(cmd.error.contains("ls:"))
    assert(cmd.error.contains("No such file or directory"))

    //  ErrorStream
    cmd = Cmd("ls /asdf", {detach: true})
    let msg = new ByteArray
    cmd.on("error", function(event, c) {
        cmd.errorStream.read(msg, -1)
    })
    cmd.finalize()
    cmd.wait()
    assert(msg.length > 0)
    assert(msg.toString().contains("ls:"))
    assert(msg.toString().contains("No such file or directory"))
}
