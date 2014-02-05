/*
    blog.tst - Blog mini app
 */

require ejs.unix

if (Config.SQLITE) {

    const HTTP = App.config.uris.http
    let mvc = Cmd.locate("mvc").portable
    let ejs = Cmd.locate("ejs").portable

    //  Prepare
    rmdir("junk")
    assert(!exists("junk"))

    //  Generate app and scaffold
    Cmd.run([mvc, '--listen', HTTP, 'generate', 'app', 'junk'])

    Cmd.run([mvc, 'generate', 'scaffold', 'post', 'title:string', 'body:text'], {dir: 'junk'})
    Cmd.run([mvc, 'compile'], {dir: 'junk'})

    //  Start web server
    let pid

    try {
        pid = Cmd.daemon([ejs, 'start.es'], {dir: 'junk'})
        assert(pid)

        sleep(2000)
        let http = new Http

        //  Get the home page
        http.get(HTTP + "/")
        assert(http.status == 200)
        assert(http.response.contains("Welcome to Ejscript"))

        //  Get the post index
        http.get(HTTP + "/Post")
        assert(http.status == 200)
        assert(http.response.contains("No Data"))

        //  Get the form to create  a post
        http.get(HTTP + "/Post/init")
        assert(http.status == 200)
        assert(http.response.contains('<input name="post.title"'))
        assert(http.response.contains('<textarea name="post.body"'))
        assert(http.response.contains("Title"))
        assert(http.response.contains("Body"))
        assert(http.response.contains('<input name="commit"'))

        //  Post the form to create a post
        http.form(HTTP + "/Post", { "post.title": "Test Post 1", "post.body": "The quick brown fox", "commit": "OK" })
        assert(http.status == 302)
        assert(http.header("Location").contains("/Post"))

        //  Get the post index
        http.get(HTTP + "/Post")
        assert(http.status == 200)
        assert(http.response.contains("The quick brown fox"))

    } catch (e) {
        print("CATCH " + e)

    } finally {
        if (pid) {
            Cmd.kill(pid, 9)
        }
        rmdir("junk")
    }
} else {
    test.skip("SQLite is not enabled")
}
