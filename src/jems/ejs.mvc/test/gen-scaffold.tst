/*
    mvc generate scaffold
 */

require ejs.unix

let mvc = Cmd.locate("mvc").portable

rm("junk", {descend: true})
assert(!exists("junk"))

//  Generate app and scaffold
Cmd.sh(mvc + " generate app junk")
Cmd.sh("cd junk ; " + mvc + " generate scaffold post title:string body:text")
assert(exists("junk/db/migrations") && isDir("junk/db/migrations"))
assert(exists("junk/controllers/Post.es"))
assert(exists("junk/models/Post.es"))
assert(exists("junk/views/Post/index.ejs"))
assert(exists("junk/views/Post/edit.ejs"))

//  Compile app
Cmd.sh("cd junk ; " + mvc + " compile")
assert(exists("junk/cache/Post.mod"))

rm("junk", {descend: true})
