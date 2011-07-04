/*
    ps.tst - Process list
 */

if (!Path("/bin").exists) {
    test.skip("Only run on unix like systems")
} else {

    //  Test with RE match
    let cmds = Cmd.ps(/init/)
    assert(cmds)
    assert(cmds.length > 0)
    assert(cmds[0].pid > 0)
    assert(cmds[0].command.contains("init"))

    //  Test with string match
    let cmds = Cmd.ps("init")
    assert(cmds)
    assert(cmds.length > 0)
    assert(cmds[0].pid > 0)
    assert(cmds[0].command.contains("init"))
}
