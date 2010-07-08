/*
    Error format stack
 */

function fun() {
    function nested() {
        try {
            throw new Error("Boom")
        } catch (e) {
            /*
                [00] format.tst, line 8, nested, throw new Error("Boom")
                [01] format.tst, line 13, fun, nested()
                [02] format.tst, line 15, default, fun()
            */
            s = e.formatStack()
            assert(s.contains('[00]'))
            assert(s.contains('[01]'))
            assert(s.contains('[02]'))
            assert(s.contains('nested()'))
            assert(s.contains('fun()'))
            assert(s.contains('Error("Boom")'))
        }
    }
    nested()
}
fun()
