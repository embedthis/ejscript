/*
    mvc run
 */
//  MOB Generate minimum app

//  MOB -- should use the test http port for this
const HTTP = ":4000"

require ejs.unix

let mvc = locate("mvc")
let ejs = locate("ejs")

//  Prepare
rmdir("junk", true)
assert(!exists("junk"))

//  Generate app and scaffold
sh(mvc + " generate app junk")
sh("cd junk ; " + mvc + " compile")

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
    kill(pid)
    rmdir("junk", true)
}

