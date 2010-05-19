/*
    Yahoo style inheritance
 */

var Shape = function(h, w) {
    /* private vars */
    var height = h, width = w

    /* private method */
    function calcArea() {
        return height * width
    }

    return {
        marker: "Hello",
        area: function () {
            return calcArea.call(this)
        }
    }
}


//  Create new object (note: not using new)

s = Shape(100, 200)
assert(Object.getOwnPropertyNames(s) == "marker,area")
assert(s.marker == "Hello")
assert(s.area() == 20000)

