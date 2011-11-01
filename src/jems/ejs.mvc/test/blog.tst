/*
    blog.tst - Blog mini app
 */

//  TODO -- should use the test http port for this
const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

require ejs.unix

let mvc = Cmd.locate("mvc").portable
let ejs = Cmd.locate("ejs").portable

//  Prepare
rmdir("junk", true)
assert(!exists("junk"))

//  Generate app and scaffold
Cmd.sh(mvc + " --listen " + HTTP + " generate app junk")

Cmd.sh("cd junk ; " + mvc + " generate scaffold post title:string body:text")
Cmd.sh("cd junk ; " + mvc + " compile")

//  Start web server
let pid

try {
    chdir("junk")
    pid = System.daemon(ejs + " start.es")
    assert(pid)
    chdir("..")

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
    rmdir("junk", true)
}

