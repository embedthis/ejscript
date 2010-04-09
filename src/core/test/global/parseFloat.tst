/*
    parseFloat
 */

// 

assert(parseFloat("0") == 0)
assert(parseFloat(" 1234") == 1234)

assert(parseFloat(".1") == .1)
assert(parseFloat("3.1415") == 3.1415)

assert(parseFloat("0.1234e+2") == 0.1234e+2)
assert(parseFloat("1.23Suffix") == 1.23)

assert(parseFloat("-1.1") == -1.1)
assert(isNaN(parseFloat("No Number")))
