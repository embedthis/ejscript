/*
    wait.tst
 */

let ejs = App.exePath

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Wait for completion
    let mark = new Date
    cmd = Cmd("/bin/sleep 2", {detach: true})
    assert(cmd.wait())
    assert(mark.elapsed >= 2000)

    //  Timeout in Wait
    let mark = new Date
    cmd = Cmd("/bin/sleep 2", {detach: true})
    assert(!cmd.wait(200))
    assert(mark.elapsed < 2000)
}
