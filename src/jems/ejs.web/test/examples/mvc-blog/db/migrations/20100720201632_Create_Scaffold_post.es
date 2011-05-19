/*
    Create Scaffold post
 */
require App
require ejs.db
require ejs.db.mapper

public class Migration {

    public function forward(db) {
        db.createTable("Posts", ["title:string", "body:text"])    }

    public function backward(db) {
        db.destroyTable("Posts")
    }
}
