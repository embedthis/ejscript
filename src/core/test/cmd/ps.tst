/*
    ps.tst - Process list
 */

if (!Path("/bin").exists) {
    test.skip("Only run on unix like systems, including cygwin")
} else {
    if (Config.OS == "WINDOWS" || Config.OS == "CYGWIN") {
        program = "bash"
        re = /bash/
    } else if (Config.OS == "MACOSX") {
        program = "launchd"
        re = /launchd/
    } else {
        /* On linux, may run from init and bash may not be running */
        program = "syslogd"
        re = /syslogd/
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
