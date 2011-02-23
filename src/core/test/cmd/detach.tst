/*
    detach.tst
 */

let ejs = App.exePath

if (!Path("/usr/bin/true").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Commands block by default
    let mark = new Date
    cmd = Cmd("/bin/sleep 2")
    assert(mark.elapsed > 1500)

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
