/*
    Promise.then
 */

var state

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

