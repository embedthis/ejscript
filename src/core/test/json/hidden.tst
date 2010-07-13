/*
    Hidden properties 
 */

class Shape {
    var width = 7
}
s = new Shape

str = serialize(s)
assert(str == "{}")
str = serialize(s, {hidden: false})
assert(str == "{}")
str = serialize(s, {hidden: true})
assert(str == '{"width":7}')
