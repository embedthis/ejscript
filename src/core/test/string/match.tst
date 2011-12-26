/*
    match.tst - test empty match
 */

let matches = "".match(/^.*$/mg)
assert(matches.length == 1)
assert(matches[0] == '')
