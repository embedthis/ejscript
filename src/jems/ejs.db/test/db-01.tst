/*
  	Unit test for ejs.db
 */

// require ejs.db
// require ejs.db.mapper

if (Config.DB && Config.SQLITE) {
    load('ejs.db.mod')
    load('ejs.db.mapper.mod')
    load('ejs.db.sqlite.mod')
    use namespace 'ejs.db'

    db = Database.defaultDatabase = new Database("sqlite", "test.sdb")
    assert(db.name  == "test.sdb")
    assert(db.connectionOptions.name == "test.sdb")
    assert(db.getTables() == "History,Portfolio,Stocks,Trades")
    assert(db.getColumns("Trades") == "id,date,stockId,quantity,price")
    db.close()


    let db = Database.defaultDatabase = new Database("sqlite", {name: "test.sdb", trace: true})
    assert(db.name  == "test.sdb")
    assert(db.connectionOptions.name == "test.sdb")
    assert(db.getTables() == "History,Portfolio,Stocks,Trades")
    assert(db.getColumns("Trades") == "id,date,stockId,quantity,price")
    db.close()


    let db = Database.defaultDatabase = new Database("sqlite", {name: "file://test.sdb"})
    assert(db.name  == "file://test.sdb")
    assert(db.connectionOptions.name == "file://test.sdb")
    assert(db.getTables() == "History,Portfolio,Stocks,Trades")
    assert(db.getColumns("Trades") == "id,date,stockId,quantity,price")
    db.close()
} else {
    test.skip("DB or SQLite not enabled")
}
