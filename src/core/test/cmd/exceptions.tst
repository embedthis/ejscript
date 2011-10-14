/*
    exceptions.tst
 */

let ejs = App.exePath.portable

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Throw exceptions on errors
    let caught = false
    try {
        cmd = Cmd("ls /asdf", {exception: true})
    } catch (e) {
        caught = true
        assert(e.toString().contains("asdf: No such file or directory"))
    }
    assert(caught)

    //  Should not throw
    Cmd("echo", {exception: true})
}
