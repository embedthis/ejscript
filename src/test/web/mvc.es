require ejs.web

/*
    Pure script View
class MyView extends View {

    //  TODO -- if you don't have a constructor, could it use this constructor by default?
    function MyView(request) {
        super(request)
    }
    override function render() {
        write("Hello World")
    }
}
*/


exports.app = function(request: Request) {

    //  TODO -- why is render being called  with this as an arg. Why with request as an arg and not view
    View(request).render(function(request: Request) {
        //  TODO -- this == view
        write("Hello World\r\n")
        print("URI " + makeUri("/logout.es"))
    })
}


exports.app = function(request: Request): Object {
    /*  
        Re-route
        router = new Router(Router.LegacyRoutes)
        router.route(request)
     */
    Mvc.init(request)
    let controller = Controller.create(request)
    return controller.start(request)
}
