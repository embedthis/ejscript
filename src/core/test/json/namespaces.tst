/*
    Namespace display
 */

class Shape {
    public var width = 7
}
s = new Shape

str = serialize(s, {namespaces: true, hidden: true})
assert(str == '{"public"::"width":7}')
