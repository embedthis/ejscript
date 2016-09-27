/*
    exceptions.tst
 */

let ejs = Cmd.locate('ejs')

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Throw exceptions on errors
    let caught = false
    try {
        cmd = Cmd("ls /asdf", {exceptions: true})
    } catch (e) {
        caught = true
        assert(e.toString().contains("No such file or directory"))
    }
    assert(caught)

    //  Should not throw
    Cmd("echo", {exceptions: true})
}
