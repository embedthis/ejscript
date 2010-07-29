/*
    Create a controller with action methods
 */
require ejs.web

class MyController extends Controller {
    use namespace action

    //  MOB -- how is the index selected?
    action function index() {
        render("Hello Index\r\n")
    }
}

exports.app = function(request: Request) {
    MyController.create(request).run(request)
}
