/*
    basic.tst
 */

let ejs = App.exePath.portable

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Simple command
    cmd = Cmd("/bin/echo")
    assert(cmd.status == 0)

    //  Command with failing status
    cmd = Cmd("/bin/ls /asdfasdf")
    assert(cmd.status != 0)
}
