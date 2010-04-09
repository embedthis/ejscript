/*
 *  Test promises
 */

/*

var promise: Promise
var result, state

//  Simple promise with callback
promise = new Promise
promise.addCallback(function(event, value) {
    result = value
})
result = null
promise.emitSuccess(77)
assert(result == 77)


//  Chaining success callbacks
promise = new Promise
promise.addCallback(function(event, value){
    state = 1
})
promise.addCallback(function(event, value){
    assert(state == 1)
    state = 2
})
state = 0
promise.emitSuccess(22)
assert(state == 2)

//  Error callback
promise = new Promise
promise.addCallback(function(event){
    //  Should not get here
    assert(0)
    state = 1
})
promise.addErrback(function(event){
    assert(state == 0)
    state = 99
})
state = 0
promise.emitError()
assert(state == 99)


//  Then chaining
promise = new Promise
promise.then(function(event){
    assert(state == 0)
    state = 1
}).then(function(event){
    assert(state == 1)
    state = 2
})
state = 0
promise.emitSuccess()
assert(state == 2)


//  Timeout and wait
state = 0
promise = new Promise
promise.addCancelback(function() {
    state = 1
})
promise.addErrback(function() {
    state = 2
})
promise.timeout(250)
promise.wait(60000)
assert(state == 2)


//  Timeout and wait with a result (timeout not taken)
state = 0
promise = new Promise
promise.then(function() {
    state = 1
})
promise.addErrback(function() {
    state = 2
})
promise.timeout(60000)
Timer(100, function() {
    promise.emitSuccess("Did it")
})
result = promise.wait(250)
assert(result == "Did it")
assert(state == 1)


//  Cancel
state = 0
promise = new Promise
promise.addCallback(function() {
    //  Should never get here
    assert(0)
    state = 1
})
promise.addCancelback(function() {
    state = 2
})
promise.cancel()
promise.emitSuccess()
assert(state == 2)


//  Test storing dynamic properties in the promise object
state = 0
promise = new Promise
promise.color = "blue"
promise.then(function(event, value){
    assert(this.color == "blue")
    this.color = value
    assert(state == 0)
    state = 1
})
assert(promise.color == "blue")
promise.emitSuccess("red")
assert(promise.color == "red")
assert(state == 1)
*/
