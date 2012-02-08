/*
    mvc generate app
 */

require ejs.unix

let mvc = Cmd.locate("mvc").portable

rm("junk", {descend: true})
assert(!exists("junk"))

//  Generate app
Cmd.sh(mvc + " generate app junk")
assert(exists("junk") && isDir("junk"))
assert(exists("junk/README"))
assert(exists("junk/cache") && isDir("junk/cache"))
assert(exists("junk/cache/App.mod"))
assert(exists("junk/controllers") && isDir("junk/controllers"))
assert(exists("junk/controllers/Base.es"))
assert(exists("junk/db") && isDir("junk/db"))
assert(exists("junk/db/junk.sdb"))
assert(exists("junk/ejsrc"))
assert(exists("junk/layouts") && isDir("junk/layouts"))
assert(exists("junk/layouts/default.ejs"))
assert(exists("junk/models") && isDir("junk/models"))
assert(exists("junk/src") && isDir("junk/src"))
assert(exists("junk/src/App.es"))
assert(exists("junk/start.es"))
assert(exists("junk/static") && isDir("junk/static"))
assert(exists("junk/views") && isDir("junk/views"))

rm("junk", {descend: true})
