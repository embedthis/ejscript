/*
    Test nested classes
 */
class Outer {
    function fun() {
        class Shape {
            static var a = 1
            public var b = 2
            function Shape() {
                assert(a == 1)
                assert(b == 2)
                b = 3
                assert(b == 3)
            }
        }
        s = new Shape
        assert(s.b == 3)
    }
}

o = new Outer
o.fun()
