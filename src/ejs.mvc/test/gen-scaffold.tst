/*
    mvc generate scaffold
 */

require ejs.unix

if (Config.SQLITE) {
    let mvc = Cmd.locate(App.exeDir.join('mvc')).portable
    rmdir("junk")
    assert(!exists("junk"))

    //  Generate app and scaffold
    Cmd.sh([mvc, 'generate', 'app', 'junk'])
    Cmd.sh([mvc, 'generate', 'scaffold', 'post', 'title:string', 'body:text'], {dir: 'junk'})
    assert(exists("junk/db/migrations") && isDir("junk/db/migrations"))
    assert(exists("junk/controllers/Post.es"))
    assert(exists("junk/models/Post.es"))
    assert(exists("junk/views/Post/index.ejs"))
    assert(exists("junk/views/Post/edit.ejs"))

    //  Compile app
    Cmd.sh([mvc, 'compile'], {dir: 'junk'})
    assert(exists("junk/cache/Post.mod"))

    rmdir("junk")

} else {
    test.skip("SQLite is not enabled")
}
