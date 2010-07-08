/*
    Error stack capture
 */

function fun() {
    function nested() {
        try {
            error = new Error("Boom")
            /* Strip off two levels of stack (nested, fun) */
            error.capture(2)
            throw error
        } catch (e) {

            assert(e.stack.length == 1)
            assert(e.stack[0].func == "default")
        }
    }
    nested()
}
fun()
