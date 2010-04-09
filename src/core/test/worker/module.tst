/*
    Module Worker Tests
 */

w = new Worker("module.mod")
let msg = null
w.onmessage = function (e) {
    msg = e.data
}
Worker.join()
assert(msg.contains("Module Loaded"))
