try {
    if (global.EJSCRIPT) { 
    }
} catch (e) {
    load("json.js")
    function dump(args) {
        s = "{\n"
        for (f in args) {
            s +=  "    \"" + f + "\": " + JSON.stringify(args[f]) + ",\n" 
        }
        s += "}"
        print(s)
    }
    function assert(cond) {
        if (!cond) {
            throw "Assertion failed " + cond
        } 
    }
    function breakpoint() {}
    function typeOf(a) {
        var type = typeof a
        return type[0].toUpperCase() + type.slice(1)
    }
}
