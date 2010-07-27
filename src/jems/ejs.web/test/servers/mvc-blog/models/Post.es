/*
    Post.es - Post Model Class
 */

require ejs.db.mapper

public dynamic class Post implements Record {

    function Post(fields: Object? = null) {
        initialize(fields)
    }
}
