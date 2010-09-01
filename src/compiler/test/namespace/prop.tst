/*
    Test namespace qualified property references
 */
namespace blue = "bbb"

class Shape {
    var x = "BLACK"
    blue var x = "BLUE"

    function fun() {
        assert(this.x == "BLACK")
        assert(x == "BLACK")

        use namespace blue
        assert(this.x == "BLUE")
        assert(x == "BLUE")

        assert(this.blue::["x"] == "BLUE")
        assert(this.("bbb")::["x"] == "BLUE")
        let o = this
        assert(o.("bbb")::["x"] == "BLUE")
        assert(o.blue::["x"] == "BLUE")
    }
}

s = new Shape
s.fun()


