/*
    Simple Promise testing
 */

GC.enabled = false

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


// Then
state = null
promise = Promise().then(function success(event) {
    state = event
}, function error(event) {
    state = event
}, function cancel(event) {
    state = event
}, function progress(event) {
    state = event
})
promise.emitSuccess()
assert(state == "success")


//  Use base emitter
var events = []
promise = Promise().onSuccess(function (event) {
    events += [event]
})
promise.on("success", function(event) {
    events += [event]
})
promise.emitSuccess()
// assert(events.length == 2)


//  Cancel
state = null
promise = Promise().onCancel(function (event, a, b, c) {
    state = event
    assert(event == "cancel")
    assert([a,b,c] == "1,2,3")
})
promise.cancel(1, 2, 3)
assert(state == "cancel")


//  Timeout and wait
state = null
promise = Promise().onTimeout(function (event) {
    state = event
})
promise.timeout(5)
promise.wait(1000)
assert(state == "timeout")
