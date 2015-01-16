/**
    Database.es -- Database class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

# Config.DB
module ejs.db {

    /**
        SQL Database support. The Database class provides an interface over other database adapter classes such as 
        SQLite or MySQL. Not all the functionality expressed by this API may be implemented by a specific database adapter.
        @spec ejs
        @stability evolving
     */
    class Database {
        private static var defaultDb: Database

        private var adapter: Object
        private var options: Object

        use default namespace public

        /**
            Initialize a database connection using the supplied database connection string. The first opened database
            will also be defined as the default database.
            @param adapter Database adapter to use. E.g. "sqlite". Sqlite is currently the only supported adapter.
            @param options Connection options. This may be filename or an object hash of properties. If set to a filename,
             it should contain the filename of the database on the local system. If options is an object hash, it should 
             contain adapter specific properties that specify how to attach to the database. 
            @option name Database name
            @option username Database username
            @option password Database password
            @option trace Trace database commands to the log
            @option socket Database communications socket
            @option module Module name containing the database connector class. This is a bare module name without ".mod"
                or any leading path.
            @option class Class name containing the database backend.
         */
        function Database(adapter: String, options: Object) {
            Database.defaultDb ||= this
            if (options is String || options is Path) {
                let name = Path(options)
                options = { name: name }
            }
            options.trace ||= false
            this.options = options
            adapter ||= "sqlite"
            options.module ||= ("ejs.db." + adapter)
            let adapterClass = options["class"] || adapter.toPascal()
            //BUG - should be able to use (options.module) below
            let module = options.module
            if (!global.module::[adapterClass]) {
                load(module + ".mod", {reload: false})
                if (!global.module::[adapterClass]) {
                    throw "Cannot find database connector \"" + module + "::" + adapter + "\""
                }
            }
            this.adapter = new global.module::[adapterClass](options)
        }

        //  TODO - inconsistent naming. Add column,index vs create table
        /**
            Add a column to a table.
            @param table Name of the table
            @param column Name of the column to add
            @param datatype Database independant type of the column. Valid types are: binary, boolean, date,
                datetime, decimal, float, integer, number, string, text, time and timestamp.
            @param options Optional parameters
         */
        function addColumn(table: String, column: String, datatype: String, options = null): Void
            adapter.addColumn(table, column, datatype, options)

        /**
            Add an index on a column
            @param table Name of the table
            @param column Name of the column to add
            @param index Name of the index
         */
        function addIndex(table: String, column: String, index: String): Void
            adapter.addIndex(table, column, index)

        /**
            Change a column
            @param table Name of the table holding the column
            @param column Name of the column to change
            @param datatype Database independant type of the column. Valid types are: binary, boolean, date,
                datetime, decimal, float, integer, number, string, text, time and timestamp.
            @param options Optional parameters
         */
        function changeColumn(table: String, column: String, datatype: String, options: Object? = null): Void
            adapter.changeColumn(table, column, datatype, options)

        /**
            Close the database connection. Database connections should be closed when no longer needed rather than waiting
            for the garbage collector to automatically close the connection when disposing the database instance.
         */
        function close(): Void
            adapter.close()

        /**
            Commit a database transaction
            @hide
         */
        function commit(): Void
            adapter.commit()

        //  TODO - implement
        /**
            Reconnect to the database using new connection options
            @param connectionString See Database() for information about connection options.
            @hide
         */
        function connect(options: Object): Void
            adapter.connect(options)

        /**
            The database connection options
         */
        function get connectionOptions(): Object
            options

        /**
            Create a new database
            @param name Name of the database
            @options Optional parameters
         */
        function createDatabase(name: String, options: Object? = null): Void
            adapter.createDatabase(name, options)

        //  TODO - should be addTable for consistency
        /**
            Create a new table
            @param table Name of the table
            @param columns Array of column descriptor tuples consisting of name:datatype
         */
        function createTable(table: String, columns: Array? = null): Void
            adapter.createTable(table, columns)

        /**
            Map the database independant data type to a database dependant SQL data type
            @param dataType Data type to map
            @returns A string containing the name of the the corresponding SQL database type
         */
        function dataTypeToSqlType(dataType:String): String
            adapter.dataTypeToSqlType(dataType)

        /**
            The default database for the application.
         */
        static function get defaultDatabase(): Database
            defaultDb

        /**
            Set the default database for the application.
            @param db the default database to define
         */
        static function set defaultDatabase(db: Database): Void 
            defaultDb = db

        /**
            Destroy a database
            @param name Name of the database to remove
         */
        function destroyDatabase(name: String): Void
            adapter.destroyDatabase(name)

        //  TODO - should be removeTable for consistency
        /**
            Destroy a table
            @param table Name of the table to destroy
         */
        function destroyTable(table: String): Void
            adapter.destroyTable(table)

        /**
            End a transaction
            @hide
         */
        function endTransaction(): Void
            adapter.endTransaction()

        /**
            Get column information 
            @param table Name of the table to examine
            @return An array of column data. This is database specific content and will vary depending on the
                database connector in use.
         */
        function getColumns(table: String): Array
            adapter.getColumns(table)

        /**
            Return list of tables in a database
            @returns an array containing list of table names present in the currently opened database.
         */
        function getTables(): Array
            adapter.getTables()

        /**
            Return the number of rows in a table
            @returns the count of rows in a table in the currently opened database.
         */
        function getNumRows(table: String): Number
            adapter.getNumRows(table)

        /**
            The name of the database
         */
        function get name(): String
            options.name

        /**
            Execute a SQL command on the database.
            @param cmd SQL command string
            @param tag Debug tag to use when logging the command
            @param trace Set to true to eanble logging this command.
            @returns An array of row results where each row is represented by an Object hash containing the 
                column names and values
            @TODO Refactor logging when Log class implemented
         */
        function query(cmd: String, tag: String = "SQL", trace: Boolean~ = false): Array {
            let mark, size
            trace ||= options.trace
            if (trace) {
                App.log.activity(tag, cmd)
                mark = new Date
                size = Memory.resident
            }
            let result = adapter.sql(cmd)
            if (trace) {
                App.log.activity("Stats", "Elapsed %5.2f msec, memory %5.2f".format(mark.elapsed, 
                    (Memory.resident - size) / (1024 * 1024)))
            }
            return result
        }

        /**
            Remove columns from a table
            @param table Name of the table to modify
            @param columns Array of column names to remove
         */
        function removeColumns(table: String, columns: Array): Void
            adapter.removeColumns(table, columns)

        /**
            Remove an index
            @param table Name of the table to modify
            @param index Name of the index to remove
         */
        function removeIndex(table: String, index: String): Void
            adapter.removeIndex(table, index)

        /**
            Rename a column
            @param table Name of the table to modify
            @param oldColumn Old column name
            @param newColumn New column name
         */
        function renameColumn(table: String, oldColumn: String, newColumn: String): Void
            adapter.renameColumn(table, oldColumn, newColumn)

        /**
            Rename a table
            @param oldTable Old table name
            @param newTable New table name
         */
        function renameTable(oldTable: String, newTable: String): Void
            adapter.renameTable(oldTable, newTable)

        /**
            Rollback an uncommited database transaction
            @hide
         */
        function rollback(): Void
            adapter.rollback()

        /**
            Execute a SQL command on the database. This is a low level SQL command interface that bypasses logging.
                Use @query instead.
            @param cmd SQL command to issue. Note: "SELECT" is automatically prepended and ";" is appended for you.
            @returns An array of row results where each row is represented by an Object hash containing the column 
                names and values
         */
        function sql(cmd: String): Array
            adapter.sql(cmd)

        /**
            Map the SQL type to a database independant data type
            @param sqlType Data type to map
            @returns The corresponding database independant type
         */
        function sqlTypeToDataType(sqlType: String): String
            adapter.sqlTypeToDataType(sqlType)

        /**
            Map the SQL type to an Ejscript type class
            @param sqlType Data type to map
            @returns The corresponding type class
         */
        function sqlTypeToEjsType(sqlType: String): Type
            adapter.sqlTypeToEjsType(sqlType)

        /**
            Start a new database transaction
            @hide
         */
        function startTransaction(): Void
            adapter.startTransaction()

//  TODO -- should be setter/getter
        /**
            Trace all SQL statements on this database. Control whether trace is enabled for all SQL statements 
            issued against the database.
            @param on If true, display each SQL statement to the log
         */
        function trace(on: Boolean): void
            options.trace = on

        /**
            Execute a database transaction
            @param code Function to run inside a database transaction
            @hide
         */
        function transaction(code: Function): Void {
            startTransaction()
            try {
                code()
            } catch (e: Error) {
                rollback();
            } finally {
                endTransaction()
            }
        }

        /**
            Quote ", ', --, ;
            @hide
         */
        static function quote(str: String): String  {
            // str.replace(/'/g, "''").replace(/[#;\x00\x1a\r\n",;\\-]/g, "\\$0")
            // return str.replace(/'/g, "''").replace(/[#;",;\\-]/g, "\\$0")
            // return str.replace(/'/g, "''").replace(/[#";\\]/g, "\\$0")
            // return str.replace(/'/g, "''").replace(/[;\\]/g, "\\$0")
            return str.toString().replace(/'/g, "''")
        }
    }
}

/*
    @copy   default

    Copyright (c) Embedthis Software. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
