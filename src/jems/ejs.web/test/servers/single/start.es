/*
    Start http server
 */
require ejs.web

let address = App.args[1] || App.config.test.http_port || ":6700"
Web.run(address, "web")
