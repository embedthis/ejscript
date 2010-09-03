/*
  	Namespace variable references
 */

namespace blue = "bbb"
var x = 1
{
    assert(x == 1)
    blue var x = 2

    assert(x == 1)
    use namespace blue

    assert(x == 2)
    assert("bbb"::x == 2)
    assert(blue::x == 2)
    assert(blue::["x"] == 2)
    assert(blue::["x"] == 2)
    assert(("bbb")::["x"] == 2)
    assert(blue::["x"] == 2)
    assert(("bb" + "b")::["x"] == 2)
}

