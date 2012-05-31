/**
    Record.es -- Record class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs.db.mapper {

    require ejs.db

    /**
        Database record class. A record instance corresponds to a row in the database. This class provides a low level 
        Object Relational Mapping (ORM) between the database and Ejscript objects. This class provides methods to create,
        read, update and delete rows in the database. When read or initialized object properties are dynamically created 
        in the Record instance for each column in the database table. Users should subclass the Record class for each 
        database table to manage. When users subclass Record to create models, they should use "implement" rather than
        extend.
        @example 
        public dynamic class MyModel implements Record {}
        @spec ejs
        @stability prototype
     */
    public class Record {
        

        static var  _assocName: String          //  Name for use in associations. Lower case class name
        static var  _belongsTo: Array = null    //  List of belonging associations

        /*
            Queries to cache. Indexed by model name and optionally query string. Contains cache options for this model.
            Created on demand if cache() is called.
         */
        static var _cacheOptions: Object = {}
        static var _caching: Boolean

        static var  _className: String          //  Model class name
        static var  _columns: Object            //  List of columns in this database table
        static var  _hasOne: Array = null       //  List of 1-1 containment associations
        static var  _hasMany: Array = null      //  List of 1-many containment  associations

        static var  _db: Database               //  Hosting database
        static var  _foreignId: String          //  Camel case class name with "Id". (userCartId))
        static var  _keyName: String            //  Name of the key column (typically "id")
        static var  _model: Type                //  Model class
        static var  _tableName: String          //  Name of the database table. Plural, PascalCase
        static var  _trace: Boolean             //  Trace database SQL statements
        static var  _validations: Array = null

        static var  _beforeFilters: Array = null//  Filters that run before saving data
        static var  _afterFilters: Array = null //  Filters that run after saving data
        static var  _wrapFilters: Array = null  //  Filters that run before and after saving data

        var _keyValue: Object                   //  Record key column value
        var _errors: Object                     //  Error message aggregation
        var _cacheAssoc: Object                 //  Cached association data
        var _imodel: Type                       //  Model class

        static var ErrorMessages = {
            accepted: "must be accepted",
            blank: "can't be blank",
            confirmation: "doesn't match confirmation",
            empty: "can't be empty",
            invalid: "is invalid",
            missing: "is missing",
            notNumber: "is not a number",
            notUnique: "is not unique",
            taken: "already taken",
            tooLong: "is too long",
            tooShort: "is too short",
            wrongLength: "wrong length",
            wrongFormat: "wrong format",
        }

        /*
            Initialize the model. This should be called by the model as its very first call.
         */
        if (Object.getName(this) != "Record") {
            _keyName = "id"
            _className = Object.getName(this)
            _model = this
            _assocName = _className.toCamel()
            _foreignId = _className.toCamel() + _keyName.toPascal()
            _tableName = plural(_className).toPascal()
        }

        use default namespace public

        /**
            Constructor for use when instantiating directly from Record. Typically, use models will implement this
            class and will provdie their own constructor which calls initialize().
         */
        function Record(fields: Object? = null) {
            initialize(fields)
        }

        /**
            Construct a new record instance. This is really a constructor function, because the Record class is 
            implemented by user models, no constructor will be invoked when a new user model is instantiated. 
            The record may be initialized by optionally supplying field data. However, the record will not be 
            written to the database until $save is called. To read data from the database into the record, use 
            one of the $find methods.
            @param fields An optional object set of field names and values may be supplied to initialize the record.
         */
        function initialize(fields: Object? = null): Void {
            _imodel = Object.getType(this)
            if (fields) {
                blend(this, fields, {deep: false, overwrite: true})
            }
        }

        /**
            Run filters after saving data
            @param fn Function to run
            @param options - reserved
         */
        static function afterFilter(fn, options: Object? = null): Void {
            _afterFilters ||= []
            _afterFilters.append([fn, options])
        }

        /**
            Run filters before saving data
            @param fn Function to run
            @param options - reserved
         */
        static function beforeFilter(fn, options: Object? = null): Void {
            _beforeFilters ||= []
            _beforeFilters.append([fn, options])
        }

        /**
            Define a belonging reference to another model class. When a model belongs to another, it has a foreign key
            reference to another class.
            @param owner Referenced model class that logically owns this model.
            @param options Optional options hash
            @option className Name of the class
            @option foreignKey Key name for the foreign key
            @option conditions SQL conditions for the relationship to be satisfied
         */
        static function belongsTo(owner, options: Object? = null): Void {
            _belongsTo ||= []
            _belongsTo.append(owner)
        }

        /**
            Database query caching. This caches controls the caching of database records. If enabled, the results
            of queries are cached with a given lifetime. If the lifetime has not expired, subsequent queries will
            be optimized by retrieving cached data. If the record is updated, the cached data will be removed so that
            the next query retrieves fresh data.
            Caching is disabled/enabled via the ejsrc config.cache.database.enable field. It is enabled by default.
            Caching may be used for any Database model, though typically it is most useful for state-less GET requests.
            @param model Model class. This can be a Model class object, "this" or a String model class name.
                You can specify "this" in static code or can also use "this" in class instance
                code and this routine will determine the underlying model class name.
            @param options Cache control options. Default options for all model caching can also be provided by the 
                ejsrc config.cache.database field.
            @option lifespan Time in seconds for the cached output to persist.
            @option query SQL query command to further differentiate cached content. If supplied, different cache data
                can be stored for each query that applies to the given model. If the URI is set to "*" all 
                URIs for the specified model will be uniquely cached. 
            @example 
                cache()
                cache("Store", {lifespan: 200})
                cache(this, {query: "SELECT * from Products"})
                cache(this, {query: "*"})
         */
        static function cache(model = null, options: Object = {}): Void {
            _caching = App.config.cache.database.enable
            if (!_caching) {
                return
            }
            let mname
            if (model == null) {
                mname = _className
            } else if (model is String) {
                mname = model
            } else if (!(model is Type)) {
                model = Object.getType(model)
                mname = Object.getName(model)
            } else {
                mname = Object.getName(model)
            }
            blend(options, App.config.cache.database, {overwrite: false})
            cacheIndex = getCacheIndex(mname)
            _cacheOptions[cacheIndex] = options
            if (options.lifespan is Number) {
                let cacheName = cacheIndex
                if (options.query) {
                    cacheName += "::" + options.query
                }
                /* Invalidate cache data for app reloads */
                App.cache.expire(cacheName, null)
                App.cache.expire(cacheName, Date().future(options.lifespan * 1000))
            }
        }

        private static function getCacheIndex(model: String): String
            "::ejs.db.mapper::" + model

        private static function getCacheName(name: String, options: Object, query: String): String {
            if (options && options.query && query) {
                name += "::" + query
            }
            return name
        }


        /*
            Read a single record of kind "model" by the given "key". Data is cached for subsequent reuse.
            Read into rec[field] from table[key]
         */
        private static function cachedRead(rec: Record, field: String, model, key: String, options: Object): Object {
            rec._cacheAssoc ||= {}
            if (rec._cacheAssoc[field] == null) {
                rec._cacheAssoc[field] =  model.readRecords(key, options); 
            }
            return rec._cacheAssoc[field]
        }

        private static function checkFormat(thisObj: Record, field: String, value, options: Object): Void {
            if (! RegExp(options.format).test(value)) {
                thisObj._errors[field] = (options && options.message) ? options.message : ErrorMessages.wrongFormat
            }
        }

        private static function checkNumber(thisObj: Record, field: String, value, options): Void {
            //  MOB - what about decimal, +-
            if (! RegExp(/^[0-9]+$/).test(value)) {
                thisObj._errors[field] = (options && options.message) ? options.message : ErrorMessages.notNumber
            }
        }

        private static function checkPresent(thisObj: Record, field: String, value, options): Void {
            if (value == undefined) {
                thisObj._errors[field] = (options && options.message) ? options.message : ErrorMessages.missing
            } else if (value.length == 0 || value.trim() == "" && thisObj._errors[field] == undefined) {
                thisObj._errors[field] = ErrorMessages.blank
            }
        }

        private static function checkUnique(thisObj: Record, field: String, value, options): Void {
            let grid: Array
            if (thisObj._keyValue) {
                grid = findWhere(field + ' = "' + value + '" AND id <> ' + thisObj._keyValue)
            } else {
                grid = findWhere(field + ' = "' + value + '"')
            }
            if (grid.length > 0) {
                thisObj._errors[field] = (options && options.message) ? options.message : ErrorMessages.notUnique
            }
        }

        /*
            Map types from SQL to ejs when reading from the database
         */
        private function coerceToEjsTypes(): Void {
            for (let field: String in this) {
                let col: Column = _imodel._columns[field]
                if (col == undefined) {
                    continue
                }
                if (col.ejsType == Object.getType(this[field])) {
                    continue
                }
                let value: String = this[field]
                switch (col.ejsType) {
                case Boolean:
                    if (value is String) {
                        this[field] = (value.trim().toLowerCase() == "true")
                    } else if (value is Number) {
                        this[field] = (value == 1)
                    } else {
                        this[field] = value cast Boolean
                    }
                    this[field] = (this[field]) ? true : false
                    break

                case Date:
                    this[field] = new Date(value)
                    break

                case Number:
                    this[field] = this[field] cast Number
                    break
                }
            }
        }

        /*
            Create associations for a record
         */
        private static function createAssociations(rec: Record, set: Array, preload, options): Void {
            for each (let model in set) {
                if (model is Array) {
                    model = model[0]
                }
                if (preload == true || (preload && preload.contains(model))) {
                    /*
                        Query did table join, so rec already has the data. Extract the fields for the referred model and
                        then remove from rec and replace with an association reference. 
                     */
                    let association = {}
                    if (!model._columns) {
                        model.getSchema()
                    }
                    for (let field: String in model._columns) {
                        let f: String = "_" + model._className + field.toPascal()
                        association[field] = rec[f]
                        delete rec.public::[f]
                    }
                    rec[model._assocName] = model.createRecord(association, options)

                } else {
                    let reader = makeLazyReader(rec, model._assocName, model, rec[model._foreignId])
                    Object.defineProperty(rec, model._assocName, { get: reader })
                    if (!model._columns) model.getSchema()
                    for (let field: String  in model._columns) {
                        let f: String = "_" + model._className + field.toPascal()
                        if (rec[f]) {
                            delete rec.public::[f]
                        }
                    }
                }
            }
        }

        /*
            Create a new record instance and apply the row data
            Process a sql result and add properties for each field in the row
         */
        private static function createRecord(data: Object, options: Object = {}) {
var before = Memory.resident
            let rec: Record = new global[_className]
            rec.initialize(data)
            rec._keyValue = data[_keyName]

            let subOptions = {}
            if (options.depth) {
                subOptions.depth = options.depth
                subOptions.depth--
            }
            if (options.include) {
                createAssociations(rec, options.include, true, subOptions)
            }
            if (options.depth != 0) {
                if (_belongsTo) {
                    createAssociations(rec, _belongsTo, options.preload, subOptions)
                }
                if (_hasOne) {
                    for each (model in _hasOne) {
                        if (!rec[model._assocName]) {
                            let reader = makeLazyReader(rec, model._assocName, model, null,
                                {conditions: rec._foreignId + " = " + data[_keyName] + " LIMIT 1"})
                            Object.defineProperty(rec, model._assocName, { get: reader })
                        }
                    }
                }
                if (_hasMany) {
                    for each (model in _hasMany) {
                        if (!rec[model._assocName]) {
                            let reader = makeLazyReader(rec, model._assocName, model, null,
                                {conditions: rec._foreignId + " = " + data[_keyName]})
                            Object.defineProperty(rec, model._assocName, { get: reader })
                        }
                    }
                }
            }
            rec.coerceToEjsTypes()
// print("Record.createRecord(" + _className + ") consumed " + (Memory.resident - before))
            return rec
        }

        /**
            Set an error message. This defines an error message for the given field in a record.
            @param field Name of the field to associate with the error message
            @param msg Error message
         */
        function error(field: String, msg: String): Void {
            field ||= ""
            _errors ||= {}
            _errors[field] = msg
        }

        /*
            Fetch cached data from the cache if present
            @return a response object
         */
        private static function fetchCachedResponse(query: String = null): Object {
            let cacheIndex = getCacheIndex(_className)
            let options = _cacheOptions[cacheIndex]
            if (options) {
                let cacheName = getCacheName(cacheIndex, options, query)
                if (!options.query || options.query == "*" || cacheName == (cacheIndex + "::" + options.query)) {
                    let item = App.cache.readObj(cacheName)
                    if (item) {
                        App.log.debug(6, "Use cached database query: " + cacheName)
                        return item
                    }
                }
                App.log.debug(6, "No cached database query for: " + cacheName)
            }
            return null
        }

        //  MOB - should not throw when a record is not found. 
        /**
            Find a record. Find and return a record identified by its primary key if supplied or by the specified options. 
            If more than one record matches, return the first matching record.
            @param key Key Optional key value. Set to null if selecting via the options 
            @param options Optional search option values
            @returns a model record or null if the record cannot be found.
            @throws IOError on internal SQL errors
            @option columns List of columns to retrieve
            @option conditions { field: value, ...}   or [ "SQL condition", "id == 23", ...]
            @option from Low level from clause (not fully implemented)
            @option keys [set of matching key values]
            @option order ORDER BY clause
            @option group GROUP BY clause
            @option include [Model, ...] Models to join in the query and create associations for. Always preloads.
                The include Model entry may also be an array of [Model, "Join Condition"]
            @option joins Low level join statement "LEFT JOIN vists on stockId = visits.id". Low level joins do not
                create association objects (or lazy loaders). The names of the joined columns are prefixed with the
                appropriate table name using camel case (tableColumn).
            @option limit LIMIT count
            @option depth Specify the depth for which to create associations for belongsTo, hasOne and hasMany relationships.
                 Depth of 1 creates associations only in the immediate fields of the result. Depth == 2 creates in the 
                 next level and so on. Defaults to one.
            @option offset OFFSET count
            @option preload [Model1, ...] Preload "belongsTo" model associations rather than creating lazy loaders. This can
                reduce the number of database queries if iterating through associations.
            @option readonly
            @option lock
         */
        static function find(key: Object, options: Object = {}): Object {
            let grid: Array = innerFind(key, 1, options)
            if (grid.length >= 1) {
                let results = createRecord(grid[0], options)
                if (options && options.debug) {
                    print("RESULTS: " + serialize(results))
                }
                return results
            } 
            return null
        }

        /**
            Find all the matching records
            @param options Optional set of options. See $find for list of possible options.
            @returns An array of model records. The array may be empty if no matching records are found
            @throws IOError on internal SQL errors
         */
        static function findAll(options: Object = {}): Array {
            let grid: Array = innerFind(null, null, options)
            // start = new Date
            for (let i = 0; i < grid.length; i++) {
                grid[i] = createRecord(grid[i], options)
            }
            // print("findAll - create records TIME: " + start.elapsed())
            if (options && options.debug) {
                print("RESULTS: " + serialize(grid))
            }
            return grid
        }

        /**
            Find the first record matching a condition. Select a record using a given SQL where clause.
            @param where SQL WHERE clause to use when selecting rows.
            @returns a model record or null if the record cannot be found.
            @throws IOError on internal SQL errors
            @example
                rec = findOneWhere("cost < 200")
         */
        static function findOneWhere(where: String): Object {
            let grid: Array = innerFind(null, 1, { conditions: [where]})
            if (grid.length >= 1) {
                return createRecord(grid[0])
            } 
            return null
        }

        /**
            Find records matching a condition. Select a set of records using a given SQL where clause
            @param where SQL WHERE clause to use when selecting rows.
            @returns An array of objects. Each object represents a matching row with fields for each column.
            @example
                list = findWhere("cost < 200")
         */
        static function findWhere(where: String, count: Number? = null): Array {
            let grid: Array = innerFind(null, null, { conditions: [where]})
            for (i in grid.length) {
                grid[i] = createRecord(grid[i])
            }
            return grid
        }

        /**
            Return the column names for the table
            @returns an array containing the names of the database columns. This corresponds to the set of properties
                that will be created when a row is read using $find.
         */
        static function getColumnNames(): Array { 
            if (!_columns) _model.getSchema()
            let result: Array = []
            for (let col: String in _columns) {
                result.append(col)
            }
            return result
        }

        /**
            Return the column names for the record
            @returns an array containing the Pascal case names of the database columns. The names have the first letter
                capitalized. 
         */
        static function getColumnTitles(): Array { 
            if (!_columns) _model.getSchema()
            let result: Array = []
            for (let col: String in _columns) {
                result.append(col.toPascal())
            }
            return result
        }

        /** 
            Get the type of a column
            @param field Name of the field to examine.
            @return A string with the data type of the column
         */
        static function getColumnType(field: String): String {
            if (!_columns) _model.getSchema()
            return _db.sqlTypeToDataType(_columns[field].sqlType)
        }

        /**
            Get the database connection for this record class
            @returns Database instance object created via new $Database
         */
        static function getDb(): Database {
            if (!_db) {
                _db = Database.defaultDatabase
            }
            return _db
        }

        /**
            Get the errors for the record. 
            @return The error message collection for the record.  
         */
        function getErrors(): Array
            _errors

        /**
            Get the key name for this record
         */
        static function getKeyName(): String
            _keyName

        /**
            Return the number of rows in the table
         */
        static function getNumRows(): Number {
            if (!_columns) _model.getSchema()
            let cmd: String = "SELECT COUNT(*) FROM " + _tableName + " WHERE " + _keyName + " <> '';"
            let grid: Array = _db.query(cmd, "numRows", _trace)
            return grid[0]["COUNT(*)"]
        }

        /*
            Read the table schema and return the column hash
         */
        private static function getSchema(): Void {
            if (!_db) {
                _db = Database.defaultDatabase
                if (!_db) {
                    throw new Error("Can't get schema, database connection has not yet been established")
                }
            }
            let sql: String = 'PRAGMA table_info("' + _tableName + '");'
            let grid: Array = _db.query(sql, "schema", _trace)
            _columns = {}
            for each (let row in grid) {
                let name = row["name"]
                let sqlType = row["type"].toLowerCase()
                let ejsType = mapSqlTypeToEjs(sqlType)
                _columns[name] = new Column(name, false, ejsType, sqlType)
            }
        }

        /**
            Get the associated name for this record
            @returns the database table name backing this record class. Normally this is simply a plural class name. 
         */
        static function getTableName(): String
            _tableName

        /**
            Define a containment relationship to another model class. When using "hasAndBelongsToMany" on another model, it 
            means that other models have a foreign key reference to this class and this class can "contain" many instances 
            of the other models.
            @param model Model. (TODO - not implemented).
            @param options Object hash of options. (TODO - not implemented).
            @option foreignKey Key name for the foreign key. (TODO - not implemented).
            @option through String Class name which mediates the many to many relationship. (TODO - not implemented).
            @option joinTable. (TODO - not implemented).
         */
        static function hasAndBelongsToMany(model: Object, options: Object = {}): Void {
            belongsTo(model, options)
            hasMany(model, options)
        }

        /**
            Check if the record has any errors.
            @return True if the record has errors.
         */
        function hasError(field: String? = null): Boolean {
            if (field) {
                return (_errors && _errors[field])
            }
            if (_errors) {
                return (Object.getOwnPropertyCount(_errors) > 0)
            } 
            return false
        }

        /**
            Define a containment relationship to another model class. When using "hasMany" on another model, it means 
            that other model has a foreign key reference to this class and this class can "contain" many instances of 
            the other.
            @param model Model class that is contained by this class. 
            @param options Options parameter
            @option things Model object that is posessed by this. (TODO - not implemented)
            @option through String Class name which mediates the many to many relationship. (TODO - not implemented)
            @option foreignKey Key name for the foreign key. (TODO - not implemented)
         */
        static function hasMany(model: Object, options: Object = {}): Void {
            _hasMany ||= []
            _hasMany.append(model)
        }

        /**
            Define a containment relationship to another model class. When using "hasOne" on another model, 
            it means that other model has a foreign key reference to this class and this class can "contain" 
            only one instance of the other.
            @param model Model class that is contained by this class. 
            @option thing Model that is posessed by this. (TODO - not implemented).
            @option foreignKey Key name for the foreign key (TODO - not implemented).
            @option as String  (TODO - not implemented).
         */
        static function hasOne(model: Object, options: Object? = null): Void {
            _hasOne ||= []
            _hasOne.append(model)
        }

        /*
            Common find implementation. See find/findAll for doc.
         */
        static private function innerFind(key: Object, limit: Number? = null, options: Object = {}): Array {
            let cmd: String
            let columns: Array
            let from: String
            let conditions: String
            let where: Boolean
            let results: Array

            if (!_columns) _model.getSchema()
            if (_caching && (results = fetchCachedResponse())) {
                return results
            }
            if (options == null) {
                options = {}
            }
            //  LEGACY 1.0.0-B2
            if (options.noassoc) {
                options.depth = 0
            }
            if (options.columns) {
                columns = options.columns
                /*
                    Qualify "id" so it won't clash when doing joins. If the "click" option is specified, must have an ID
                    TODO - Should not modify the parameter. This is actually modifying the options passed in.
                 */
                let index: Number = columns.indexOf("id")
                if (index >= 0) {
                    columns[index] = _tableName + ".id"
                } else if (!columns.contains(_tableName + ".id")) {
                    columns.insert(0, _tableName + ".id")
                }
            } else {
                columns = ["*"]
            }

            conditions = ""
            from = ""
            where = false

            if (options.from) {
                from = options.from
            } else {
                from = _tableName
            }

            if (options.include) {
                let model
                if (options.include is Array) {
                    for each (entry in options.include) {
                        if (entry is Array) {
                            model = entry[0]
                            from += " LEFT OUTER JOIN " + model._tableName
                            from += " ON " + entry[1]
                        } else {
                            model = entry
                            from += " LEFT OUTER JOIN " + model._tableName
                        }
                    }
                } else {
                    model = options.include
                    from += " LEFT OUTER JOIN " + model._tableName
                    // conditions = " ON " + model._tableName + ".id = " + _tableName + "." + model._assocName + "Id"
                }
            }

            if (options.depth != 0) {
                if (_belongsTo) {
                    conditions = " ON "
                    for each (let owner in _belongsTo) {
                        from += " INNER JOIN " + owner._tableName
                    }
                    for each (let owner in _belongsTo) {
                        let tname: String = Object.getName(owner)
                        tname = tname[0].toLowerCase() + tname.slice(1) + "Id"
                        conditions += _tableName + "." + tname + " = " + owner._tableName + "." + owner._keyName + " AND "
                    }
                    if (conditions == " ON ") {
                        conditions = ""
                    }
                }
            }

            if (options.joins) {
                if (conditions == "") {
                    conditions = " ON "
                }
                let parts: Array = options.joins.split(/ ON | on /)
                from += " " + parts[0]
                if (parts.length > 1) {
                    conditions += parts[1] + " AND "
                }
            }
            conditions = conditions.trim(" AND ")

            if (options.conditions) {
                let whereConditions: String = " WHERE "
                if (options.conditions is Array) {
                    for each (cond in options.conditions) {
                        whereConditions += cond + " " + " AND "
                    }
                    whereConditions = whereConditions.trim(" AND ")

                } else if (options.conditions is String) {
                    whereConditions += options.conditions + " " 

                } else if (options.conditions is Object) {
                    for (field in options.conditions) {
                        //  Remove quote from options.conditions[field]
                        whereConditions += field + " = '" + options.conditions[field] + "' " + " AND "
                    }
                }
                whereConditions = whereConditions.trim(" AND ")
                if (whereConditions != " WHERE ") {
                    where = true
                    conditions += whereConditions
                } else {
                    whereConditions = ""
                    from = from.trim(" AND ")
                }

            } else {
                from = from.trim(" AND ")
            }

            if (key || options.key) {
                if (!where) {
                    conditions += " WHERE "
                    where = true
                } else {
                    conditions += " AND "
                }
                conditions += (_tableName + "." + _keyName + " = ") + ((key) ? key : options.key)
            }

            //  Removed quote from "from"
            cmd = "SELECT " + Database.quote(columns) + " FROM " + from + conditions
            if (options.group) {
                cmd += " GROUP BY " + options.group
            }
            if (options.order) {
                cmd += " ORDER BY " + options.order
            }
            if (limit) {
                cmd += " LIMIT " + limit
            } else if (options.limit) {
                cmd += " LIMIT " + options.limit
            }
            if (options.offset) {
                cmd += " OFFSET " + options.offset
            }
            cmd += ";"

            if (_db == null) {
                throw new Error("Database connection has not yet been established")
            }
            if (_caching && (results = fetchCachedResponse(cmd))) {
                return results
            }
            if (results == null) {
                try {
                    if (_trace) {
                        let start = new Date
                        results = _db.query(cmd, "find", _trace)
                        App.log.activity("TIME", "Query Time:", start.elapsed)
                        App.log.info("Query Time:", start.elapsed)
                    } else {
                        results = _db.query(cmd, "find", _trace)
                    }
                    if (_caching) {
                        saveQuery(results, cmd)
                    }
                } 
                catch (e) {
                    throw e
                }
            }
            return results
        }

        /*
            Make a getter function to lazily (on-demand) read associated records (belongsTo)
         */
        private static function makeLazyReader(rec: Record, field: String, model, key: String, 
                options: Object = {}): Function {
            // print("Make lazy reader for " + _tableName + "[" + field + "] for " + model._tableName + "[" + key + "]")
            var lazyReader: Function = function(): Object {
                // print("Run reader for " + _tableName + "[" + field + "] for " + model._tableName + "[" + key + "]")
                return cachedRead(rec, field, model, key, options)
            }
            return lazyReader
        }

        private static function mapSqlTypeToEjs(sqlType: String): Type {
            sqlType = sqlType.replace(/\(.*/, "")
            let ejsType: Type = _db.sqlTypeToEjsType(sqlType)
            if (ejsType == undefined) {
                throw new Error("Unsupported SQL type: \"" + sqlType + "\"")
            }
            return ejsType
        }

        /*
            Prepare a value to be written to the database
         */
        private static function prepareValue(field: String, value: Object): String {
            let col: Column = _columns[field]
            if (col == undefined) {
                return undefined
            }
            if (value == undefined) {
                throw new Error("Field \"" + field + "\" is undefined")
            }
            if (value == null) {
                throw new Error("Field \"" + field + "\" is null")
            }
            switch (col.ejsType) {
            case Boolean:
                if (value is String) {
                    value = (value.toLowerCase() == "true")
                } else if (value is Number) {
                    value = (value == 1)
                } else {
                    value = value cast Boolean
                }
                return value

            case Date:
                return "%d".format((new Date(value)).time)

            case Number:
                return value cast Number
             
            case String:
                return Database.quote(value)
            }
            return Database.quote(value.toString())
        }

        /*
            Read records for an assocation. Will return one or an array of records matching the supplied key and options.
         */
        private static function readRecords(key: String, options: Object): Object {
            let data: Array = innerFind(key, null, options)
            if (data.length > 1) {
                let result: Array = new Array
                for each (row in data) {
                    result.append(createRecord(row))
                }
                return result

            } else if (data.length == 1) {
                return createRecord(data[0])
            }
            return null
        }

        /**
            Remove records from the database
            @param ids Set of keys identifying the records to remove
         */
        static function remove(...ids): Void {
            for each (let key: Object in ids) {
                let cmd: String = "DELETE FROM " + _tableName + " WHERE " + _keyName + " = " + key + ";"
                getDb().query(cmd, "remove", _trace)
            }
        }

        private function runFilters(filters): Void {
            for each (filter in filters) {
                let fn = filter[0]
                let options = filter[1]
                if (options) {
                    let only = options.only
/* TODO
                    if (only) {
                        if (only is String && actionName != only) {
                            continue
                        }
                        if (only is Array && !only.contains(actionName)) {
                            continue
                        }
                    } 
                    except = options.except
                    if (except) {
                        if (except is String && actionName == except) {
                            continue
                        }
                        if (except is Array && except.contains(actionName)) {
                            continue
                        }
                    }
*/
                }
                fn.call(this)
            }
        }

        /**
            Save the record to the database.
            @returns True if the record is validated and successfully written to the database
            @throws IOError Throws exception on sql errors
         */
        function save(): Boolean {
            var sql: String
            _imodel ||= Object.getType(this)
            if (!_imodel._columns) _imodel.getSchema()
            if (!validateRecord()) {
                return false
            }
            runFilters(_imodel._beforeFilters)
            
            if (_keyValue == null) {
                sql = "INSERT INTO " + _imodel._tableName + " ("
                for (let field: String in this) {
                    if (_imodel._columns[field]) {
                        sql += field + ", "
                    }
                }
                sql = sql.trim(', ')
                sql += ") VALUES("
                for (let field: String in this) {
                    if (_imodel._columns[field]) {
                        sql += "'" + prepareValue(field, this[field]) + "', "
                    }
                }
                sql = sql.trim(', ')
                sql += ")"

            } else {
                sql = "UPDATE " + _imodel._tableName + " SET "
                for (let field: String in this) {
                    if (_imodel._columns[field]) {
                        sql += field + " = '" + prepareValue(field, this[field]) + "', "
                    }
                }
                sql = sql.trim(', ')
                sql += " WHERE " + _imodel._keyName + " = " +  _keyValue
            }
            if (!_keyValue) {
                sql += "; SELECT last_insert_rowid();"
            } else {
                sql += ";"
            }

            let result: Array = _imodel._db.query(sql, "save", _imodel._trace)
            if (!_keyValue) {
                _keyValue = this["id"] = result[0]["last_insert_rowid()"] cast Number
            }
            runFilters(_imodel._afterFilters)
            if (_imodel._caching) {
                let cacheIndex = getCacheIndex(_imodel._className)
                let options = _cacheOptions[cacheIndex]
                if (options == null || options.query == null) {
                    let cacheName = getCacheName(cacheIndex, options)
                    App.log.debug(6, "Expire database query cache " + cacheName)
                    App.cache.writeObj(cacheName, null, options)
                } else {
                }
            }
            return true
        }

        /*
            Save the output from a database query for future reuse
         */
        private static function saveQuery(results, query: String = null): Void {
            let cacheIndex = getCacheIndex(_className)
            let options = _cacheOptions[cacheIndex]
            if (options) {
                let cacheName = getCacheName(cacheIndex, options, query)
                App.cache.writeObj(cacheName, results, options)
                App.log.debug(6, "Cache database query " + cacheName)
            }
        }

        /**
            Update a record based on the supplied fields and values.
            @param fields Hash of field/value pairs to use for the record update.
            @returns True if the database is successfully updated. Returns false if validation fails. In this case,
                the record is not saved.
            @throws IOError on database SQL errors
         */
        function saveUpdate(fields: Object): Boolean {
            for (field in fields) {
                if (this[field] != undefined) {
                    this[field] = fields[field]
                }
            }
            return save()
        }

        /**
            Set the database connection for this record class
            @param database Database instance object created via new $Database
         */
        static function setDb(database: Database) {
            _db = database
        }

        /**
            Set the key name for this record
         */
        static function setKeyName(name: String): Void {
            _keyName = name
        }

        /**
            Set the associated table name for this record
            @param name Name of the database table to backup the record class.
         */
        static function setTableName(name: String): Void {
            if (_tableName != name) {
                _tableName = name
                if (_db) {
                    _model.getSchema()
                }
            }
        }

        /**
            Run an SQL statement and return selected records.
            @param cmd SQL command to issue. Note: "SELECT" is automatically prepended and ";" is appended for you.
            @returns An array of objects. Each object represents a matching row with fields for each column.
         */
        static function sql(cmd: String, count: Number? = null): Array {
            cmd = "SELECT " + cmd + ";"
            return getDb().query(cmd, "select", _trace)
        }

        /**
            Trace SQL statements. Control whether trace is enabled for the actual SQL statements issued against the database.
            @param on If true, display each SQL statement to the log
         */
        static function trace(on: Boolean): void
            _trace = on

        /** @hide TODO */
        static function validateFormat(fields: Object, options = null) {
            if (_validations == null) {
                _validations = []
            }
            _validations.append([checkFormat, fields, options])
        }

        /** @hide TODO */
        static function validateNumber(fields: Object, options = null) {
            if (_validations == null) {
                _validations = []
            }
            _validations.append([checkNumber, fields, options])
        }

        /** @hide TODO */
        static function validatePresence(fields: Object, options = null) {
            if (_validations == null) {
                _validations = []
            }
            _validations.append([checkPresent, fields, options])
        }

        /**
            Validate a record. This call validates all the fields in a record.
            @returns True if the record has no errors.
         */
        function validateRecord(): Boolean {
            _imodel ||= Object.getType(this)
            if (!_imodel._columns) _imodel.getSchema()
            _errors = {}
            if (_imodel._validations) {
                for each (let validation: String in _imodel._validations) {
                    let check = validation[0]
                    let fields = validation[1]
                    let options = validation[2]
                    if (fields is Array) {
                        for each (let field in fields) {
                            if (_errors[field]) {
                                continue
                            }
                            check(this, field, this[field], options)
                        }
                    } else {
                        check(this, fields, this[fields], options)
                    }
                }
            }
            let thisType = Object.getType(this)
            if (thisType["validate"]) {
                thisType["validate"].call(this)
            }
            coerceToEjsTypes()
            return Object.getOwnPropertyCount(_errors) == 0
        }

        /** @hide TODO */
        static function validateUnique(fields: Object, option = null)
            _validations.append([checkUnique, fields, options])

        /**
            Run filters before and after saving data
            @param fn Function to run
            @param options - reserved
         */
        static function wrapFilter(fn, options: Object? = null): Void {
            _wrapFilters ||= []
            _wrapFilters.append([fn, options])
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        static function get columnNames(): Array {
            return getColumnNames()
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        static function get columnTitles(): Array {
            return getColumnTitles()
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        static function get db(): Datbase {
            return getDb()
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        static function get keyName(): String {
            return getKeyName()
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        static function get numRows(): String {
            return getNumRows()
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        static function get tableName(): String {
            return getTableName()
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        function constructor(fields: Object = null): Void {
            initialize(fields)
        }
    }


    /**
        Database column class. A database record is comprised of one or mor columns
        @hide
     */
    class Column {
        //  TODO - workaround. Make these public, see ticket 1227: ejsweb generate was not finding them when internal
        //  missing the internal namespace.
        public var ejsType: Object 
        public var sqlType: Object 

        function Column(name: String, accessor: Boolean = false, ejsType: Type? = null, sqlType: String? = null) {
            this.ejsType = ejsType
            this.sqlType = sqlType
        }
    }

    /** @hide */
    function plural(name: String): String
        name + "s"

    /** @hide */
    function singular(name: String): String {
        var s: String = name + "s"
        if (name.endsWith("ies")) {
            return name.slice(0,-3) + "y"
        } else if (name.endsWith("es")) {
            return name.slice(0,-2)
        } else if (name.endsWith("s")) {
            return name.slice(0,-1)
        }
        return name.toPascal()
    }

    /**
        Map a type assuming it is already of the correct ejs type for the schema
        @hide
     */
    function mapType(value: Object): String {
        if (value is Date) {
            return "%d".format((new Date(value)).time)
        } else if (value is Number) {
            return "%d".format(value)
        }
        return value
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
