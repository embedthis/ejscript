/*
    Simple Promise testing
 */

var state

// Simple success
state = null
promise = Promise().onSuccess(function (event, a, b, c) {
    assert(event == "success")
    state = event
    assert([a,b,c] == "1,2,3")
})
promise.emitSuccess(1, 2, 3)
assert(state == "success")


// Simple cancel
state = null
promise = Promise().onSuccess(function (event) {
    // Not called
    assert(event == "success")
    state = event
}).onCancel(function (event) {
    assert(event == "cancel")
    state = event
})
promise.emitCancel()
assert(state == "cancel")


// Simple error
state = null
promise = Promise().onSuccess(function (event) {
    // Not called
    assert(event == "success")
    state = event
}).onCancel(function (event) {
    assert(event == "cancel")
    state = event
}).onError(function (event) {
    assert(event == "error")
    state = event
})
promise.emitError()
assert(state == "error")
