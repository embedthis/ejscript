/*
    Indent formatting
 */

o = {
    something: 7,
}
str = serialize(o, { pretty: true, indent: "______________" })
assert(str == '{
______________"something": 7
}')


str = serialize(o, { pretty: true, indent: 20 })
assert(str = '{
                    "something": 7
}')
