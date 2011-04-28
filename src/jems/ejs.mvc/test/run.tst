/*
    mvc run
 */
//  TODO Generate minimum app

// const HTTP = (global.test) ? test.config.http_port : ":4770"
const HTTP = ":4000"

print(HTTP)

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
    Cmd.kill(pid, 9)
    rmdir("junk", true)
}

