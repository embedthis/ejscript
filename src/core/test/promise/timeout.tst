/*
    Promise.timeout
 */

var state

//  Timeout and wait
promise = Promise().onTimeout(function (event) {
    state = event
})
promise.timeout(5)
promise.wait(1000)
assert(state == "timeout")
