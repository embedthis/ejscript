/**
    Couch.es -- CouchDB Database class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*
    TODO:
    - Can this extend / implement database? If not, should not be ejs.db.couch but rather ejs.couch
    - Create one line helper to do put|get and deserialize of the result
 */
/*
"total_rows": 3,
    "offset": 0,
    "rows": [{
        "id": "64ACF01B05F53ACFEC48C062A5D01D89",
        "key": null,
        "value": {
          "Name": "Bill",
          "Address": "123 Park Lane",
          "Phone": 1234
        }
      },
    ]
 }

 PUT response
    Code 201 {"ok":true,"id":"0","rev":"525-892b10489a466d3d8fa9c1dcc3e81c42"}
 */

module ejs.db.couch {

    /** 
        CouchDB database support
        @hide
        @spec ejs
        @stability prototype
     */
    class Couch implements Stream {
        use default namespace public

        /* Name of the database */
        private var db: String

        /* */
        private var uri: Uri
        private var hp: Http

        /** 
            Couch("http://127.0.0.1:5984", "test") 
            @hide
         */
        function Couch(uri: Uri, db: String) {
            this.db = db
            this.uri = uri
            hp = new Http
            hp.async = true
        }

        /**
            @hide
         */
        function get async(): Boolean
            hp.async

        /**
            @hide
         */
        function set async(enabled: Boolean): Void {
            hp.async = enabled
        }

        /** 
            @duplicate Stream.close 
            This closes any open network connection and resets the http object to be ready for another connection. 
            This option blocks and should only be used in sync mode. If false, the current request will be aborted.
            @hide
         */
        function close(): Void 
            hp.close

        /**
            @hide
         */
        function cmd(method: String, url: String, data: String? = null): Object {
            hp.method = method
            if (data) {
                hp.setHeader("Content-Length", data.length)
            }
            hp.connect(uri.join(url))
            if (data) {
//  MOB -- this is sync and blocking
                hp.write(data)
            }
            hp.finalize()
            if (!hp.async) {
                return deserialize(hp.response)
            }
        }

        /** 
            @duplicate Stream.flush
            @hide 
         */
        function flush(dir: Number = Stream.BOTH): Void {}

        /**
            @hide
         */
        function get http(): Http
            hp

        /**
            @hide
         */
        function on(event: String, observer: Function): Void
            hp.on(event, observer)

        /**
            @hide
         */
        function removeObserver(event: String, observer: Function): Void
            hp.removeObserver(event, observer)

        /**
            @hide
         */
        function get status(): Number
            hp.status

        /** @hide */
        function get success(): Boolean
            200 <= hp.status && hp.status < 300

        /** 
            @duplicate Http.read
            @hide
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number
            hp.read(buffer, offset, count)

        /** 
            @duplicate Http.write
         */
        function write(...data): Void
            hp.write(data)

        /**
            Server level
         */
        function getDatabases(): Object 
            cmd("GET", "/_all_dbs")

        /*
            Database level commands
         */

        /**
            @hide
         */
        function bulkUpdate(data): Object
            cmd("POST", "/" + db + "/_bulk_docs", data)

        /**
            @hide
         */
        function compact(): Object
            cmd("POST", db + "/_compact")

        /**
            @hide
         */
        function createDatabase(): Object
            cmd("PUT", "/" + db)

        /**
            @hide
         */
        function destroyDatabase(): Object
            cmd("DELETE", "/" + db)

        /**
            @hide
         */
        function cleanupView(): Object
            cmd("POST", url + "/_view_cleanup/")

        /**
            @hide
         */
        function getDocuments(): Object
            cmd("GET", db + "/_all_docs")

        /*
            Document level
         */

        /**
            @hide
         */
        function createDoc(data): Object
            cmd("POST", db, data)

        /**
            @hide
         */
        function deleteDoc(name: String, ref: String): Object
            cmd("DELETE", db + "/" + name + "?ref=" + ref)

        /**
            @hide
         */
        function getDoc(id: String): Object
            cmd("GET", db + "/" + id)

        /**
            @hide
         */
        function updateDoc(name: String, data): Object
            cmd("PUT", db + "/" + name, data)

        /*
            Views
            @hide
         */
        function tempView(viewCode: String): Object 
            cmd("POST", db + "/_temp_view", viewCode)

        /**
            @hide
         */
        function view(name: String, options = null): Object {
            let parts = name.split(":")
            let view = parts[0]
            let design = parts[1] || "master"
            let uri = db + "/_design/" + design + "/_view/" + view
            if (options) {
                uri += "?" + options
            }
            return cmd("GET", uri)
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
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
    
    @end
 */
