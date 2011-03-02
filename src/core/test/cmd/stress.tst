/*
    Stress test
 */

cmd = Cmd("stderrOutput")
assert(cmd.status == 0)
assert(cmd.response == "")
assert(cmd.error.length > 0)
assert(cmd.error.contains("/bin/sh"))

