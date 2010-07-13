/*
    Indent formatting
 */

o = {
    something: 7,
}
str = serialize(o, { indent: "______________" })
assert(str == '{
______________"something": 7
}')


str = serialize(o, { indent: 20 })
assert(str = '{
                    "something": 7
}')
