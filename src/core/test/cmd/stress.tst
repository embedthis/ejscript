/*
    Stress test
 */

if (Config.OS != 'WINDOWS') {
    cmd = Cmd("sh ./ls-to-stderr")
    assert(cmd.status == 0)
    assert(cmd.response == "")
    assert(cmd.error.length > 0)
    assert(cmd.error.contains("/bin/sh"))
} else {
    if (global.test) {
        test.skip("Test only valid on windows")
    }
}

