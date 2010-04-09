/*
    parseInt
 */



assert(parseInt("0xe") == 14)
assert(parseInt("0xE") == 14)
assert(parseInt(" 0xE") == 14)

assert(parseInt("0xe", 16) == 14)
assert(parseInt("0xE", 16) == 14)
assert(parseInt(" 0xE", 16) == 14)

assert(parseInt("14") == 14)
assert(parseInt(" 14") == 14)
assert(parseInt("1110", 2) == 14)
assert(parseInt("14suffix") == 14)

assert(parseInt("-0xe", 16) == -14)
assert(parseInt("-14") == -14)

assert(isNaN(parseInt("No Number")))
assert(isNaN(parseInt("899", 8)))
