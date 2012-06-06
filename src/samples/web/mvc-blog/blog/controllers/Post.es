
require ejs.web

public class PostController extends BaseController {

    public var post: Post

    function PostController() {
    }

    use namespace action

    action function index() { 
        renderView("list")
    }

    action function list() { 
    }

    action function edit() {
        post = Post.find(params.id)
    }

    action function create() {
        post = new Post
        renderView("edit")
    }

    action function update() {
        if (params.commit == "Cancel") {
            redirect("list")

        } else if (params.commit == "Delete") {
            destroy()

        } else if (params.id) {
            post = Post.find(params.id)
            if (post.saveUpdate(params.post)) {
                inform("Post updated successfully.")
                redirect("list")
            } else {
                /* Validation failed */
                renderView("edit")
            }

        } else {
            post = new Post(params.post)
            if (post.save()) {
                inform("New post created")
                redirect("list")
            } else {
                renderView("edit")
            }
        }
    }

    action function destroy() {
        Post.remove(params.id)
        inform("Post " + params.id + " removed")
        redirect("list")
    }

    
}
