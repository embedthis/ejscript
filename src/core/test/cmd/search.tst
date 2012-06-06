/*
    search.tst - Use search paths to locate executable
 */

let ejs = Cmd.locate('ejs')

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Simple command
    cmd = Cmd("echo")
    assert(cmd.status == 0)

    //  Command with failing status
    cmd = Cmd("ls /asdfasdf")
    assert(cmd.status != 0)
}
