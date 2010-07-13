/*
    Depth control
 */

o = {
    one: {
        two: {
            three: {
            },
        },
    },
}
str = serialize(o, { depth: 2 })
assert(str == '{"one":{"two":{}}}')
