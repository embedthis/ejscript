/*
    JSON with quotes in keys and values
 */
o = {}

o['a"b'] = 'c"d'

s = serialize(o)
o2 = deserialize(s)
assert(serialize(o2) == s)
