
/*OK 9
throw "BAD"
*/

/*OK 8
postMessage({ name: "Michael O'Brien", address: "7427 Orin Court N"})
print("DONE")
*/


/* OK - 7
GC.run()
print('SLEEPING')
App.sleep(60 * 1000)
GC.run()
print('AFTER SLEEPING')
GC.run()
*/

/* OK - 6
App.sleep(100)
print("In Worker " + self.name)
App.sleep(100)
*/

/* OK - 5
print("do nothing in SCRIPT: " + self.name)
App.sleep(100)
*/


/* OK - 4
function fun() {
    throw new Error("Can't continue")
}
fun()
*/

/*OK - 1
postMessage("Short Message")
App.sleep(1000);
*/

/* OK - 2
print("IN worker name " + self.name)
exit("Some Data")
print("AFTER EXIT")
*/

/* OK - 3
onmessage = function (e) {
    print("WORKER GOT " + e.data) 
    exit()
}
App.serviceEvents()
*/
