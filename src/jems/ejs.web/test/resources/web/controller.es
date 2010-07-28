require ejs.web

class MyController extends Controller {
    function MyController(request) {
        super(request)
    }
    use namespace action

    action function index() {
        render("IN INDEX\r\n")
    }
}

exports.app = function(request: Request) {
    MyController(request).run(request)
}
