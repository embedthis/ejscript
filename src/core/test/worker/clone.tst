/*
    Test cloning worker interpreters
 */

//  Clone self
public var x = 1234
assert(x == 1234)
w = Worker.cloneSelf()
y = w.eval('assert(x == 1234), x')
assert(y == x)
Worker.join(w)


//  Clone new worker. Preload a module into the first worker, clone to the second and should already be loaded
w = new Worker
w.preload("ejs.web.mod")
w.preeval('assert("ejs.web"::Request != null)')
w2 = w.clone()
w2.eval('assert("ejs.web"::Request != null)')

Worker.join(w, w2)
