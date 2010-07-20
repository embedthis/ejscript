/*
    Extending emitter
 */

//  Use base emitter
var events = []
promise = Promise()
promise.onSuccess(function osc(event) {
    events += [event]
})
promise.observe("success", function obs(event) {
    events += [event]
})
promise.emitSuccess()
assert(events.length == 2)
