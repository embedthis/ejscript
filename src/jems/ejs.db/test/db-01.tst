/*
  	Unit test for ejs.db
 */

require ejs.db
require ejs.db.mapper
require ejs.db.sqlite

let db = Database.defaultDatabase = new Database("sqlite", "test.sdb")
// db.trace(true)

assert(db.name  == "test.sdb")
assert(db.connection == "test.sdb")
assert(db.getTables() == "History,Portfolio,Stocks,Trades")
assert(db.getColumns("Trades") == "id,date,stockId,quantity,price")

db.close()
