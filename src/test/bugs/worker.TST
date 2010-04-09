print(find("sub", "*", true))


/* TESTING extending NATIVE  types
class Font extends String {
    var thickness: String

    function Font(value: String, thickness: String) {
        super(value)
        this.thickness = thickness
    }
    public override function toString(): String {
        // super.toString()
        // let n: Number = new Number(super.abc)
        return "XX: " + thickness + " value " + this
    }
}

var f = new Font("Hello World", "bold")
print("TITLE "  + f.toString())

class Num extends Number {
    var flavor: String
    function Num(value: Number, flavor: String) {
        super(value)
        this.flavor = flavor
    }
    public override function toString(): String {
        // super.toString()
        // let n: Number = new Number(super.abc)
        return "Num: " + flavor + " value " + super.toString()
    }
}

var n = new Num(77, "red")
print(7 + n)
print("VALUE: " + n.toString())
*/

/*OK
print(Path('tmp').find())
Path('tmp').removeAll()
*/

/*OK
f =  Path('.').find('*')
print(f)
*/


var w: Worker

/*OK 9 - receive just a plain string
w = new Worker("script.es")
w.onerror = function (e) {
    assert(e is String)
    assert(e == "BAD")
}
Worker.join()
*/

/* OK - 8
w = new Worker("script.es")
w.onmessage = function (e) {
    o = deserialize(e.data)
    print(o.name)
    print(o.address)
}
Worker.join()
*/

/* OK - 5  testing start() module
w = new Worker("script.mod")
w.join()
*/


/* OK - 5  testing start() with name
w = new Worker(null, { name: "funny-worker" })
print("STARTING " + w.name)
w.start("script.es")
print("AFTER START")
w.onclose = function (e) {
    print("@@@@ Worker " + this.name + " exited")
}
w.join()
*/

/* OK - 5 testing start()
print("DONE")
w = new Worker
w.start("script.es")
print("AFTER START")
w.onclose = function (e) {
    print("@@@@ Worker " + this.name + " exited")
}
w.join()
print("DONE")
*/

/* OK - 7 
w = new Worker("script.es")
w.onclose = function (e) {
    print("@@@@ Worker " + this.name + " exited")
}
App.sleep(100)
w.terminate()
w.join()
*/

/* OK - 6
workers = []
for (i in 4) {
    w = new Worker("script.es")
    workers.append(w)
    print("Start " + w.name)
    w.onclose = function (e) {
        print("@@@@ Worker " + this.name + " exited")
    }
}
Worker.join(workers)
*/

/* OK - 5 
App.noexit()
w = new Worker("script.es")
w.onclose = function (e) {
    print("Worker " + this.name + " exited")
    App.exit()
}
*/

/*OK -4 
w = new Worker("script.es")
w.onerror = function (e) {
    // dump("ERROR IN WORKER:", e)
    print("Error: " + e.message + " in " + e.filename + "@" + e.lineno)
    print("Stack " + e.stack)
}
Worker.join()
*/

/*
 *  terminate
 *  names & lookup
 *  CREATE UNIT TESTS
 *  start.script
 *  start.function & class
 *  Join more threads
 */

/* OK - 3
w = new Worker("script.es")
w.postMessage("Back to you")
Worker.join()
*/

/*OK - 1
w = new Worker("script.es")
w.onmessage = function (e) {
    asser(e.data == "Short Message")
}
Worker.join()
*/

/*OK - 2
w = new Worker("script.es")
Worker.join()
*/

/*OK - 2 
w = new Worker("script.es")
for (i = 0; i < 10; i++) {
    if (Worker.join(null, 1000)) {
        break
    }
}
assert(i < 10)
*/

/*OK - 2
w = new Worker("script.es")
Worker.join()
 */

/*OK - 2
w = new Worker
w.load("script.es")
*/


/* UNIT TEST NEEDED
require ejs.web

cache = new Cache

cache.write("farm", "one", 77)

data = cache.read("farm", "one")
assert(data == 77)
print(data)
*/
