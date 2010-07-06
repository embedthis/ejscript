/*
  	Lambda expressions
 */

function square(x) x * x
assert(square(4) == 16)

function truth() true
assert(truth() == true)

var total = function() 1 + 5
assert(total() == 6)
