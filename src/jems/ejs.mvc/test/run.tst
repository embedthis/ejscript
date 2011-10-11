/*
    mvc run
 */
//  TODO Generate minimum/flat app

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
Cmd.sh("cd junk ; " + mvc + " compile")

//  Start web server. Use ejs start.es so we can kill it. Change to mvc run when Cmd supports kill group
chdir("junk")
let pid = System.daemon(ejs + " start.es")
assert(pid)
chdir("..")
sleep(2000)

try {
    let http = new Http
    //  Get the home page
    http.get(HTTP + "/")
    assert(http.status == 200)
    assert(http.response.contains("Welcome to Ejscript"))

} finally {
    Cmd.kill(pid, 9)
    rmdir("junk", true)
}

