/*
    Test ejsmvc
 */

/*
System.daemon("/bin/sleep 9999")
rmdir("junk", true)
assert(!exists("junk"))

//  Generate app
let mvc = locate("mvc")
sh(mvc + " generate app junk")
assert(exists("junk") && isDir("junk"))
assert(exists("junk/README"))
assert(exists("junk/cache") && isDir("junk/cache"))
assert(exists("junk/controllers") && isDir("junk/controllers"))
assert(exists("junk/db") && isDir("junk/db"))
assert(exists("junk/ejsrc"))
assert(exists("junk/layouts") && isDir("junk/layouts"))
assert(exists("junk/models") && isDir("junk/models"))
assert(exists("junk/src") && isDir("junk/src"))
assert(exists("junk/start.es"))
assert(exists("junk/static") && isDir("junk/static"))
assert(exists("junk/views") && isDir("junk/views"))

let pid = System.daemon("sh -c \"cd junk ; " + mvc + " run \"")
print(pid)
App.sleep(1000)

//  MOB -- need Cmd.kill(pid)
kill(pid)


//  Generate scaffold

sh("cd junk ; " + mvc + " generate scaffold post ")


//  MOB Generate flat app

//  Cleanup
rmdir("junk", true)
*/
