/*
    Test ejs.sys::CmdArgs
*/

let template = [
    [ "verbose" ],
    [ "quiet", null, false ],
    [ "log", String, "stdout:4" ],
    [ "mode", [ "low", "med", "high" ], "high" ],
]
let caught

cmd = CmdArgs(template, [])
assert(cmd.args == "")
assert(cmd.options.verbose == undefined)
assert(cmd.options.quiet == false)
assert(cmd.options.log == "stdout:4")
assert(cmd.options.mode == "high")

cmd = CmdArgs(template, ["a", "b", "c"])
assert(cmd.args == "a,b,c")

cmd = CmdArgs(template, ["--quiet", "a", "b", "c"])
assert(cmd.options.quiet == true)

cmd = CmdArgs(template, ["--verbose", "--log", "trace.out:4", "--quiet", "a", "b", "c"])
assert(cmd.options.quiet == true)
assert(cmd.options.verbose == true)
assert(cmd.options.log == "trace.out:4")
assert(cmd.options.mode == "high")

//  Test with "-" as the option leadin
cmd = CmdArgs(template, ["-mode", "low"])
assert(cmd.options.mode == "low")

//  Test with "=" as the option leadin
cmd = CmdArgs(template, ["-mode=low", "-verbose"])
assert(cmd.options.mode == "low")
assert(cmd.options.verbose)

//  Test with "-" as the option leadin
cmd = CmdArgs(template, ["-verbose", "-log", "trace.out:4", "-quiet", "a", "b", "c"])
assert(cmd.options.quiet == true)
assert(cmd.options.verbose == true)
assert(cmd.options.log == "trace.out:4")

//  Test bad arg
caught = false
try { CmdArgs(template, ["-badarg"]); } catch { caught = true; }
assert(caught)

//  Test bad arg value
caught = false  
try { CmdArgs(template, ["-mode", "extreme"]); } catch { caught = true; }
assert(caught)
