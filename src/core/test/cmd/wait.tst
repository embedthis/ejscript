/*
    wait.tst
 */

let ejs = App.exePath.portable

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Wait for completion
    let mark = new Date
    cmd = Cmd("/bin/sleep 1", {detach: true})
    cmd.finalize()
    assert(cmd.wait(10000))
    assert(mark.elapsed >= 900)

    //  Timeout in Wait
    let mark = new Date
    cmd = Cmd("/bin/sleep 2", {detach: true})
    cmd.finalize()
    assert(!cmd.wait(200))
    assert(mark.elapsed < 2000)
}
