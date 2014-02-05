/*
    DatabaseConnector.es -- Database Connector interface

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

# Config.DB
module ejs.db {

    /**
        Database Connector interface. The database connector interface is the contract implemented by 
        concrete database implementations. This interface is implemented by database connectors such as SQLite and MYSQL.
        @spec ejs
        @stability evolving
     */
    interface DatabaseConnector {

        use default namespace public

        // function DatabaseConnector(connectionString: String)

        /** @duplicate ejs.db::Database.addColumn */
        function addColumn(table: String, column: String, datatype: String, options: Object? = null): Void

        /** @duplicate ejs.db::Database.addIndex */
        function addIndex(table: String, column: String, index: String): Void

        /** @duplicate ejs.db::Database.changeColumn */
        function changeColumn(table: String, column: String, datatype: String, options: Object? = null): Void

        /** @duplicate ejs.db::Database.close */
        function close(): Void

        /** @duplicate ejs.db::Database.commit 
            @hide
         */
        function commit(): Void

        /** @duplicate ejs.db::Database.connect 
            @hide
         */
        function connect(connectionString: String): Void

        /** @duplicate ejs.db::Database.createDatabase */
        function createDatabase(name: String, options: Object? = null): Void

        /** @duplicate ejs.db::Database.createTable */
        function createTable(table: String, columns: Array? = null): Void

        /** @duplicate ejs.db::Database.dataTypeToSqlType */
        function dataTypeToSqlType(dataType:String): String

        /** @duplicate ejs.db::Database.destroyDatabase */
        function destroyDatabase(name: String): Void

        /** @duplicate ejs.db::Database.destroyTable */
        function destroyTable(table: String): Void

        /** @duplicate ejs.db::Database.getColumns */
        function getColumns(table: String): Array

        /** @duplicate ejs.db::Database.getTables */
        function getTables(): Array

        /** @duplicate ejs.db::Database.removeColumns */
        function removeColumns(table: String, columns: Array): Void 

        /** @duplicate ejs.db::Database.removeIndex */
        function removeIndex(table: String, index: String): Void

        /** @duplicate ejs.db::Database.renameColumn */
        function renameColumn(table: String, oldColumn: String, newColumn: String): Void

        /** @duplicate ejs.db::Database.renameTable */
        function renameTable(oldTable: String, newTable: String): Void

        /** @duplicate ejs.db::Database.rollback 
            @hide
         */
        function rollback(): Void

        /** @duplicate ejs.db::Database.sql */
        function sql(cmd: String): Array

        /** @duplicate ejs.db::Database.sqlTypeToDataType */
        function sqlTypeToDataType(sqlType: String): String

        /** @duplicate ejs.db::Database.sqlTypeToEjsType */
        function sqlTypeToEjsType(sqlType: String): String

        /** @duplicate ejs.db::Database.startTransaction 
            @hide */
        function startTransaction(): Void
    }
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
