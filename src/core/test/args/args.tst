/*
    Test Args
*/

var used
function usage() { used = true }

let template = {
    options: {
        verbose: { alias: 'v' },
        quiet:   { value: false },
        log:     { range: String, value: 'stdout:4' },
        mode:    { range: [ 'low', 'med', 'high' ], value: 'high' },
    },
    silent: true,
}
let caught

//  usage and silent
used = false
args = Args({usage: usage, silent: true} , ['test', '-xyz'])

assert(used)
used = false
assert(args.rest == "")
assert(serialize(args.options) == '{}')

//  onerror
caught = false
try {
    args = Args({onerror: 'throw'} , ['test', '-xyz'])
} catch(e) {
    caught = true
}
assert(caught)
caught = false
assert(args.rest == "")


//  Default values
args = Args(template, ['test'])
assert(args.rest == "")
assert(args.options.verbose == undefined)
assert(args.options.quiet == false)
assert(args.options.log == "stdout:4")
assert(args.options.mode == "high")

//  Parse rest
args = Args(template, ["test", "a", "b", "c"])
assert(args.rest == "a,b,c")

//  Parse switches and options
args = Args(template, ["test", "--quiet", "a", "b", "c"])
assert(args.options.quiet == true)

//  Full command line
args = Args(template, ["test", "--verbose", "--log", "trace.out:4", "--quiet", "a", "b", "c"])
assert(args.options.quiet == true)
assert(args.options.verbose == true)
assert(args.options.log == "trace.out:4")
assert(args.options.mode == "high")

//  Alias
args = Args(template, ["test", "-v", "--mode", "low"])
assert(args.options.verbose == true)

//  Test with "-" as the option leadin
args = Args(template, ["test", "-mode", "low"])
assert(args.options.mode == "low")

//  Test with "=" as the option leadin
args = Args(template, ["test", "-mode=low", "-verbose"])
assert(args.options.mode == "low")
assert(args.options.verbose)

//  Test with "-" as the option leadin
args = Args(template, ["test", "-verbose", "-log", "trace.out:4", "-quiet", "a", "b", "c"])
assert(args.options.quiet == true)
assert(args.options.verbose == true)
assert(args.options.log == "trace.out:4")

//  Test bad arg
caught = false
let t2 = blend(template, {onerror: 'throw'})
try { Args(t2, ["test", "-badarg"]); } catch { caught = true; }
assert(caught)

//  Test bad arg value
caught = false  
try { Args(t2, ["test", "-mode", "extreme"]); } catch { caught = true; }
assert(caught)
