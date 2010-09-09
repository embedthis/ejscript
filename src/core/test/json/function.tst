/*
    Test encoding of functions
 */
o = { 
    f: function fun() {return 2}
}

s = serialize(o)
o = deserialize(s)
assert(o.f == "[function Function]")
