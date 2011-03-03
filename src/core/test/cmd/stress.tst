/*
    Stress test
 */

cmd = Cmd("stderr-response")
assert(cmd.status == 0)
assert(cmd.response == "")
assert(cmd.error.length > 0)
assert(cmd.error.contains("/bin/sh"))

