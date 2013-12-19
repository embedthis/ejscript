/*
    Test cloning worker interpreters
 */

//  Clone self (fork)
public var x = 1234
assert(x == 1234)
w = Worker.fork()
y = w.eval('assert(x == 1234), x')
assert(y == x)
Worker.join(w)


/*
    //  Clone new worker. Preload a module into the first worker, clone to the second and should already be loaded
    //  TODO - should not use ejs.web here - creates a forward dependency
    w = new Worker
    w.preload("ejs.web.mod")
    w.preeval('assert("ejs.web"::Request != null)')
    w2 = w.clone()
    w2.eval('assert("ejs.web"::Request != null)')
    Worker.join(w, w2)
*/
