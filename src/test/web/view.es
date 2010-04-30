require ejs.web

class MyView extends View {
    function MyView(request) {
        super(request)
    }
    //  MOB -- what to do here
}


exports.app = function(request: Request) {
    View(request).render(function(request: Request) {
        write("Hello World\r\n")
        write("URI " + makeUri("/logout.es") + "\r\n")
    })
}
