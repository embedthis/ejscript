/*
    Test require() loader
 */

/*  WARNING: require() may not be supported in its current form in future releases */

var add = require("math").add
assert(add(1, 2, 3) == 6)

var add = require("math.js").add
assert(add(1, 3) == 4)

var add = require("./math.js").add
assert(add(7, 3) == 10)
