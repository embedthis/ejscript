/*
  	Tests for the App class
 */

// exeDir 
assert(App.exeDir.name.length > 0)
assert(App.exeDir.exists)
assert(App.exeDir.name.search("%") < 0)


//	args
/* TODO set in workers
print("ARGS " + App.args)
print("LEN " + App.args.length)
assert(App.args != "")
assert(App.args.length > 0)
*/


//	sleep
d = new Date
App.sleep(100)
elapsed = new Date - d


//  getenv
assert(App.getenv("PATH"))


//  env
assert(App.env["PATH"])
assert(Object.getOwnPropertyCount(App.env) > 0)
