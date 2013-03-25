/*
    Test regular expression parsing, deserializing and serializing

    Notes:
        Assert comparisions to strings will convert the RegExp to strings
        Ejs will correctly backquote / and \
 */

let r = /.html/
assert(r == '/.html/')

r = /\.html/
assert(r == '/\\.html/')

r = /\/html/
assert(r == '/\\/html/')

r = /"html"/
assert(r == '/"html"/')

let o = deserialize('{ value: /.html/ }')
assert(o.value == '/.html/')

o = deserialize('{ value: /\\.html/ }')
assert(o.value == '/\\.html/')

o = deserialize('{ value: /\\/html/ }')
assert(o.value == '/\\/html/')

assert(serialize({pattern: /42/}) == '{"pattern":"/42/"}')
assert(serialize({pattern: /42/}, {regexp: true}) == '{"pattern":/42/}')
