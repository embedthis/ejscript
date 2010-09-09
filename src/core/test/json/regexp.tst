/*
    Test encoding of regular expressions
 */

o = { 
    r: /^[abc\/]*$/ 
}

s = serialize(o)
o = deserialize(s)
assert(o.r == "/^[abc\\/]*$/")
