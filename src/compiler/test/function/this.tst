/*
    Test "this" bindings
 */


// Test "this" values
function global_fun() {
    assert(this == global)
}
global_fun()


// Test "this" inside an instance class
class Shape {
    function ifun() {
        assert(this == ss)
        function nested() {
            assert(this == ss)
        }
        nested()
    }
    static function sfun() {
        assert(this == Shape)
        function nested() {
            assert(this == Shape)
        }
        nested()
    }
}

var ss = new Shape
ss.ifun()
Shape.sfun()
