/*
    Timer with error capture via onerror
 */

var fired

var timer = Timer(100, function () {
    fired = true
    throw new Error("Boom")
})

var caughtError
timer.onerror = function (e) {
    assert(e is Error)
    assert(e.message == "Boom")
    caughtError = true
}
timer.start()

for (let i in 100) {
	if (fired) {
		break
	}
	App.sleep(100)
}
assert(fired)
