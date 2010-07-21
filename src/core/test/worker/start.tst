/*
    Start Worker Tests
 */

w = new Worker
w.load("worker.es")
let exitCount = 0
w.onclose = function (e) {
    exitCount++
}
Worker.join(w)
assert(exitCount == 1)

//  Test start with a name

w = new Worker(null, { name: "funny-worker" })
exitCount = 0
w.load("worker.es")
w.onclose = function (e) {
    exitCount++
    assert(this.name == "inside-funny-worker")
}
Worker.join(w)
assert(exitCount == 1)

//  Test receiving a message from the started script

w = new Worker("worker.es")
w.onmessage = function (e) {
    let o = deserialize(e.data)
    assert(o.name == "Mary")
    assert(o.address == "123 Park Ave")
}
Worker.join(w)
