/*
    kill.tst - Kill and killall
 */

if (!Path("/bin").exists) {
    test.skip("Only run on unix like systems")
} else {

    //  Simple kill by PID
    let pid = Cmd.daemon(["sleep", "60"])
    assert(pid > 0)
    Cmd.kill(pid)

    //  Killall by string name
    let pid1 = Cmd.daemon(["sleep", "60"])
    let pid2 = Cmd.daemon(["sleep", "60"])
    assert(pid1 > 0)
    assert(pid2 > 0)
    Cmd.killall("sleep", 2)

    //  Killall by RE name
    let pid1 = Cmd.daemon(["sleep", "60"])
    let pid2 = Cmd.daemon(["sleep", "60"])
    assert(pid1 > 0)
    assert(pid2 > 0)
    Cmd.killall(/sleep.*60/, 2)

    //  Killall with preserving self
    let pid = Cmd.daemon(["ejs", "-c", "App.run()"])
    assert(pid > 0)
    Cmd.killall("ejs", 2, App.pid)
}
