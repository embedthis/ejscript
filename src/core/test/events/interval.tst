/*
    Interval Timers
 */

var count = 0
var complete = false

setInterval(function (a, b, c) {
print(a,b,c)
    assert(a == 1)
    assert(b == 2)
    assert(c == 3)
    if (++count == 3) {
        timer.stop()
        complete = true
    }
}, 100, 1, 2, 3)

for (let i in 100) {
	if (complete) {
		break
	}
	App.sleep(100)
}
assert(complete)
