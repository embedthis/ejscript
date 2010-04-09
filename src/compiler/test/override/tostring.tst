/*
    Test override tostring
 */

/* MOB - String is now final

class Font extends String {
    var thickness: String

    function Font(value: String, thickness: String) {
        super(value)
        this.thickness = thickness
    }
    public override function toString(): String {
        return thickness + ":" + this
    }
}

var f = new Font("Hello World", "bold")
assert(f.toString() == "bold:Hello World")
*/
