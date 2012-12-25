/*
    Test regular expression parsing, deserializing and serializing
 */


let r = /.html/
assert(r == '/.html/')

r = /\.html/
assert(r == '/\\.html/')

r = /\/html/
assert(r == '/\\/html/')

let o = deserialize('{ value: /.html/ }')
assert(o.value == '/.html/')

o = deserialize('{ value: /\\.html/ }')
assert(o.value == '/\\.html/')

o = deserialize('{ value: /\\/html/ }')
assert(o.value == '/\\/html/')

