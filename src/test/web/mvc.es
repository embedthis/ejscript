require ejs.web

/*
    Pure script View
class MyView extends View {
    function MyView(request) {
        super(request)
    }
    override function render() {
        write("Hello World")
    }
}
*/


exports.app = function(request: Request) {
    View(request).render(function(request: Request) {
        write("Hello World\r\n")
        print("URI " + makeUri("/logout.es"))
    })
}


exports.app = function(request: Request): Object {
    /*  Re-route
        router = new Router(Router.LegacyRoutes)
        router.route(request)
     */
print("CALLING MVC.INIT")
    Mvc.init(request)
print("CALLING CREATE CONTROLLER")
    let controller = Controller.create(request)
print("CALLING CONTROLLER.START")
    return controller.start(request)
}
