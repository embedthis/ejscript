/* TESTING extending NATIVE  types
class Font extends String {
    var thickness: String

    function Font(value: String, thickness: String) {
        super(value)
        this.thickness = thickness
    }
    public override function toString(): String {
        // super.toString()
        // let n: Number = new Number(super.abc)
        return "XX: " + thickness + " value " + this
    }
}

var f = new Font("Hello World", "bold")
print("TITLE "  + f.toString())

class Num extends Number {
    var flavor: String
    function Num(value: Number, flavor: String) {
        super(value)
        this.flavor = flavor
    }
    public override function toString(): String {
        // super.toString()
        // let n: Number = new Number(super.abc)
        return "Num: " + flavor + " value " + super.toString()
    }
}

var n = new Num(77, "red")
print(7 + n)
print("VALUE: " + n.toString())
*/
