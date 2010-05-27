/*
 *	Basic core language benchmarks
 *
 *  Others
 *      Array, RegExp, E4X, SQLite
 *  Big objects, small objects
 */

var results = ""

function report(msg, count, when) {
    d = new Date
    elapsed = ((d - when) / 1000).toFixed(2)
    results += elapsed + ", "
    print(msg + " " + elapsed + " " + count)
}


var i
var start = new Date
var mark
var count

//
//	TEST: Traditional for loops
//
mark = new Date
count = 10000000
for (i = 0; i < count; i++) {
	;
}
report("Traditional Loop", count, mark)

// App.sleep(30 * 1024)
// App.exit()


//
//	TEST: For/In loops
//
start = mark = new Date
count = 20000000
for (i = 0; i < count; i++) {
	;
}
report("For in loop", count, mark)


//
//	TEST: Function calls
//
count = 4000000
function bench(a, b)
{
	return a
}
mark = new Date
for (i = 0; i < count; i++) {
	bench(i, i)
}
report("Function calls", count, mark)

//
//	TEST: If / else creation
//
mark = new Date
var j
count = 3000000
for (i = 0; i < count; i++) {
	if (i % 2) {
		if (i / 2) {
			j = i
		} else {
			j = i
		}
	} else {
		if (i / 2) {
			j = i
		} else {
			j = i
		}
	}
}
report("If/else", count, mark)


//
//	TEST: Dynamic Object creation
//
mark = new Date
var x
count = 6000000
for (i = 0; i < count; i++) {
	x = new Object
}
report("New Objects", count, mark)


//
//	TEST: Dynamic Property creation
//
var john = new Object
mark = new Date
count = 200000
for (i = 0; i < count; i++) {
	john.age = 32
	john.address = "22 Park Ave"
	john.married = true
	john.smoker = false
	john.dependents = 4
	john.driver = true
	john.height = 72
	john.weight = 190
}
report("Dynamic Properties", count, mark)


//
//	Property access
//
var user = new Object
user.peter = new Object
user.peter.age = 32
mark = new Date
count = 800000
for (i = 0; i < count; i++) {
	user.peter.age = 24
	x = user.peter.age
}
report("Property Access", count, mark)


function Shape() {
    /*
    this.height = 0
    this. width = 0
    */
}


//
//	TEST: Class Object creation
//
mark = new Date
count = 6000000
for (i = 0; i < count; i++) {
	x = new Shape()
}
report("Instance creation", count, mark)


//
//	TEST: Class Property access
//
var s = new Shape
mark = new Date
count = 5000000
for (i = 0; i < count; i++) {
	s.height = 24
	s.width = s.height
}
report("Class property access", count, mark)
report("Total", 0, start)

//
//  Sleep to allow ../Makefile to snapshot the memory size then print results.
//
print("\nTest Complete -- waiting for 10 secs")
// App.sleep(10000)
print("\nForLoop, ForIn, Function, If, NewObject, NewProperty, GetProperty, NewClass, ClassAccess, Total")
print(results)
// Memory.stats()

for (i = 0; ; i++) {
	;
}
