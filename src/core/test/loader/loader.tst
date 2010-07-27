/*
    Test CommonJS require() loader
 */

var add = require("math").add
assert(add(1, 2, 3) == 6)

var add = require("math.js").add
assert(add(1, 3) == 4)

var add = require("./math.js").add
assert(add(7, 3) == 10)
