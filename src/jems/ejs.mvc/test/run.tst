/*
    mvc run
 */
//  TODO Generate minimum/flat app

require ejs.unix

const HTTP = App.config.uris.http

let mvc = Cmd.locate("mvc").portable
let ejs = Cmd.locate("ejs").portable

//  Prepare
rmdir("junk")
assert(!exists("junk"))

//  Generate app and scaffold
Cmd.sh([mvc, '--listen', HTTP, 'generate', 'app', 'junk'])
Cmd.sh([mvc, 'compile'], {dir: 'junk'})

//  Start web server. Use ejs start.es so we can kill it. Change to mvc run when Cmd supports kill group
let pid = Cmd.daemon([ejs, 'start.es'], {dir: 'junk'})
assert(pid)
sleep(2000)

try {
    let http = new Http
    //  Get the home page
    http.get(HTTP + "/")
    assert(http.status == 200)
    assert(http.response.contains("Welcome to Ejscript"))

} finally {
    Cmd.kill(pid, 9)
    rmdir("junk")
}

