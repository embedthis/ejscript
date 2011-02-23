/*
    basic.tst
 */

let ejs = App.exePath

if (!Path("/usr/bin/true").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Simple command
    cmd = Cmd("/usr/bin/true")
    assert(cmd.status == 0)

    //  Command with failing status
    cmd = Cmd("/usr/bin/false")
    assert(cmd.status == 1)
}
