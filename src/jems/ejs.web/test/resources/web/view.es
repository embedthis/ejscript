require ejs.web

class MyView extends View {
    function MyView(request) {
        super(request)
    }
    //  MOB -- what to do here
}

exports.app = function(request: Request) {
    MyView(request).render(function(request: Request) {
            //  MOB - put controls here
        image("logo.png")
        write("Hello World\r\n")
        write("URI " + makeUri("/logout.es") + "\r\n")
    })
}
