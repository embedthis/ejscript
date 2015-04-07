/*
    Interval Timers
 */

var count = 0
var complete = false

setInterval(function (a, b, c) {
    assert(a == 1)
    assert(b == 2)
    assert(c == 3)
    if (++count == 3) {
        stop()
        complete = true
    }
}, 50, 1, 2, 3)

for (i in 100) {
    App.run(200, 1);
	if (complete) {
		break
	}
}
assert(complete)
