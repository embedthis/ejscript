require ejs.web


//  TODO -- this not being used?
class MyView extends View {

    //  TODO -- is this required
    function MyView(request) {
        super(request)
    }
    //  TODO -- what to do here
}

exports.app = function(request: Request) {
    View(request).render(function(request: Request) {
        write("Hello World\r\n")
        write("URI " + link("/logout.es") + "\r\n")
    })
}
