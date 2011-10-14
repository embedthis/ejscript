/*
    detach.tst
 */

let ejs = App.exePath.portable

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Commands block by default
    let mark = new Date
    cmd = Cmd("sleep 1")
    assert(mark.elapsed > 250)

    //  Detached command
    let mark = new Date
    cmd = Cmd("sleep 5", {detach: true})
    assert(mark.elapsed < 5000)
    cmd.close()

    //  Kill and pid
    cmd = new Cmd
    cmd.start("sleep 20", {detach: true})
    assert(cmd.pid != 0)
    Cmd.kill(cmd.pid)

    //  Kill detached
    cmd = Cmd("sleep 20", { detach: true })
    assert(cmd.kill(cmd.pid))

    //  Stop 
    cmd = Cmd("sleep 20", { detach: true })
    assert(cmd.stop())
}
