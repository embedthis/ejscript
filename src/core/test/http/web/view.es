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
    })
}
