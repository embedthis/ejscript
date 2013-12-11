/*
    JSGI style app with middleware layers
 */
require ejs.web

app = function layer(request) { 
    return {
        status: 200, 
        headers: {"Content-Type": "text/html"}, 
        body: "Hello Cruel World\n"
    } 
}

/*
    Wrap the app with the Monitor middleware
 */
//  MOB -- add CommonLog

exports.app = Monitor(app)
