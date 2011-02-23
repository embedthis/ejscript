/*
    detach.tst
 */

let ejs = App.exePath

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Commands block by default
    let mark = new Date
    cmd = Cmd("/bin/sleep 1")
    assert(mark.elapsed > 250)

    //  Detached command
    let mark = new Date
    cmd = Cmd("/bin/sleep 5", {detach: true})
    assert(mark.elapsed < 5000)
    cmd.close()

    //  Kill and pid
    cmd = new Cmd
    cmd.start("/bin/sleep 20", {detach: true})
    assert(cmd.pid != 0)
    Cmd.kill(cmd.pid)

    //  Kill detached
    cmd = Cmd("/bin/sleep 20", { detach: true })
    assert(cmd.kill(cmd.pid))

    //  Stop 
    cmd = Cmd("/bin/sleep 20", { detach: true })
    assert(cmd.stop())
}
