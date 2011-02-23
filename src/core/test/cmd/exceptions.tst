/*
    exceptions.tst
 */

let ejs = App.exePath

if (!Path("/usr/bin/true").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Throw exceptions on errors
    let caught = false
    try {
        cmd = Cmd("/bin/ls /asdf", {exception: true})
    } catch (e) {
        caught = true
        assert(e.toString().contains("asdf: No such file or directory"))
    }
    assert(caught)

    //  Should not throw
    Cmd("/usr/bin/true", {exception: true})
}
