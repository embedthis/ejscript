/*
    ps.tst - Process list
 */

if (!Path("/bin").exists || Config.OS == 'windows' || Cmd.locate('busybox')) {
    test.skip("Only run on unix like systems, including cygwin and not busybox")
} else {
    if (Config.OS == "windows" || Config.OS == "cygwin") {
        program = "bash"
        re = /bash/
    } else if (Config.OS == "macosx") {
        program = "launchd"
        re = /launchd/
    } else {
        program = "sh"
        re = /sh/
    }

    //  Test with RE match
    let cmds = Cmd.ps(re)
    assert(cmds)

    assert(cmds.length > 0)
    assert(cmds[0].pid > 0)
    assert(cmds[0].command.contains(program))

    //  Test with string match
    let cmds = Cmd.ps(program)
    assert(cmds)
    assert(cmds.length > 0)
    assert(cmds[0].pid > 0)
    assert(cmds[0].command.contains(program))
}
