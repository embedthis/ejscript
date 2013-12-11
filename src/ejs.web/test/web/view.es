require ejs.web


//  MOB -- this not being used?
class MyView extends View {

    //  MOB -- is this required
    function MyView(request) {
        super(request)
    }
    //  MOB -- what to do here
}

exports.app = function(request: Request) {
    View(request).render(function(request: Request) {
        write("Hello World\r\n")
        write("URI " + link("/logout.es") + "\r\n")
    })
}
