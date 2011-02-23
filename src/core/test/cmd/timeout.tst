/*
    timeout.tst
 */

let ejs = App.exePath

if (!Path("/bin/echo").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Set timeout
    let caught = false
    let mark = new Date
    try {
        cmd = new Cmd
        cmd.timeout = 250
        assert(cmd.timeout == 250)
        cmd.start("/bin/sleep 5")
    } catch (e) {
        caught = true
    }
    assert(caught)
    assert(mark.elapsed < 4000)

    //  Set timeout via options
    let caught = false
    let mark = new Date
    try {
        cmd = new Cmd
        cmd.start("/bin/sleep 5", {timeout: 250})
    } catch (e) {
        caught = true
    }
    assert(caught)
    assert(mark.elapsed < 4000)
}
