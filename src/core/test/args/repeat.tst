/*
    Args with repeated options
*/

let template = {
    options: {
        'enable': { range: String, separator: Array },
        'include': { range: String, separator: ',' },
    },
}

//  Array
let args = Args(template, ["test", '--enable', 'one', '--enable', 'two', '--enable', 'three'])
assert(args.rest == "")
assert(args.options.enable is Array)
assert(args.options.enable.length == 3)
assert(args.options.enable[0] == 'one')
assert(args.options.enable[1] == 'two')
assert(args.options.enable[2] == 'three')

//  String
args = Args(template, ["test", '--include', 'one', '--include', 'two', '--include', 'three'])
assert(args.rest == "")
assert(args.options.include is String)
assert(args.options.include == 'one,two,three')

