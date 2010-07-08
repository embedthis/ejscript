/*
    Simple Promise testing
 */

var state

promise = Promise().onCancel(function (event, a, b, c) {
    state = event
    assert(event == "cancel")
    assert([a,b,c] == "1,2,3")
})
promise.cancel(1, 2, 3)
assert(state == "cancel")
