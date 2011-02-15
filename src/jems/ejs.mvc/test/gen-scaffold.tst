/*
    mvc generate scaffold
 */

require ejs.unix

let mvc = locate("mvc")
let ejs = locate("ejs")

//  Prepare
rmdir("junk", true)
assert(!exists("junk"))

//  Generate app and scaffold
sh(mvc + " generate app junk")
sh("cd junk ; " + mvc + " generate scaffold post title:string body:text")
assert(exists("junk/db/migrations") && isDir("junk/db/migrations"))
assert(exists("junk/controllers/Post.es"))
assert(exists("junk/models/Post.es"))
assert(exists("junk/views/Post/index.ejs"))
assert(exists("junk/views/Post/edit.ejs"))

//  Compile app
sh("cd junk ; " + mvc + " compile")
assert(exists("junk/cache/Post.mod"))

rmdir("junk", true)

