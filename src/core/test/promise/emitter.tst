/*
    Extending emitter
 */

//  Use base emitter
var events = []
promise = Promise().onSuccess(function (event) {
    events += [event]
})
promise.observe("success", function(event) {
    events += [event]
})
promise.emitSuccess()
assert(events.length == 2)
