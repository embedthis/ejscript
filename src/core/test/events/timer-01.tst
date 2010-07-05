/*
    Timer
 */

var fired

var timer = Timer(100, function (a, b, c) {
    assert(a == 1)
    assert(b == 2)
    assert(c == 3)
    fired = true
}, 1, 2, 3)

timer.start()

for (let i in 100) {
	if (fired) {
		break
	}
	App.sleep(100)
}
assert(fired)
