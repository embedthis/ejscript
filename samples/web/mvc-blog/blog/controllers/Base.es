/*
    BaseController.es - Base class for all controllers
 */
require ejs.web

public class BaseController extends Controller {

    public var title: String = "blog"
    public var style: String

    function BaseController() {
        style = request.home.join("web/style.css")
    }

    action function home() {}
}
