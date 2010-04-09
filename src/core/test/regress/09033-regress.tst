/*
 *  Using requrie as a function
 */

var done = false
function require() {
    done = true
}

require()

assert(done == true)
