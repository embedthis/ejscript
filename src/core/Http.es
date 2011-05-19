/**
    Http.es -- HTTP client side communications
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The Http object represents a Hypertext Transfer Protocol version 1.1 client connection. It is used to issue 
        HTTP requests and capture responses. It supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
        @spec ejs
        @stability evolving
     */
    class Http implements Stream {

        use default namespace public

        /** 
          HTTP Continue Status (100)
         */     
        static const Continue : Number = 100

        /** 
            HTTP Success Status (200) 
         */     
        static const Ok : Number = 200

        /** 
            HTTP Created Status (201) 
         */     
        static const Created : Number = 201

        /** 
            HTTP Accepted Status (202) 
         */     
        static const Accepted : Number = 202

        /** 
            HTTP Non-Authoritative Information Status (203) 
         */     
        static const NotAuthoritative : Number = 203

        /** 
            HTTP No Content Status (204)  
         */     
        static const NoContent : Number = 204

        /** 
            HTTP Reset Content Status (205) 
         */     
        static const Reset : Number = 205

        /** 
            HTTP Partial Content Status (206) 
         */     
        static const PartialContent : Number = 206

        /** 
            HTTP Multiple Choices Status (300) 
         */     
        static const MultipleChoice : Number = 300

        /** 
            HTTP Moved Permanently Status (301) 
         */     
        static const MovedPermanently : Number = 301

        /** 
            HTTP Found but Moved Temporily Status (302) 
         */     
        static const MovedTemporarily : Number = 302

        /** 
            HTTP See Other Status (303) 
         */     
        static const SeeOther : Number = 303

        /** 
            HTTP Not Modified Status (304)     
         */
        static const NotModified : Number = 304

        /** 
            HTTP Use Proxy Status (305) 
         */     
        static const UseProxy : Number = 305

        /** 
            HTTP Bad Request Status(400) 
         */     
        static const BadRequest : Number = 400

        /** 
            HTTP Unauthorized Status (401) 
         */     
        static const Unauthorized : Number = 401

        /** 
            HTTP Payment Required Status (402) 
         */     
        static const PaymentRequired : Number = 402

        /** 
            HTTP Forbidden Status (403)  
         */     
        static const Forbidden : Number = 403

        /** 
            HTTP Not Found Status (404) 
         */     
        static const NotFound : Number = 404

        /** 
            HTTP Method Not Allowed Status (405) 
         */     
        static const BadMethod : Number = 405

        /** 
            HTTP Not Acceptable Status (406) 
         */     
        static const NotAcceptable : Number = 406

        /** 
            HTTP ProxyAuthentication Required Status (407) 
         */     
        static const ProxyAuthRequired : Number = 407

        /** 
            HTTP Request Timeout Status (408) 
         */     
        static const RequestTimeout : Number = 408

        /** 
            HTTP Conflict Status (409) 
         */     
        static const Conflict : Number = 409

        /** 
            HTTP Gone Status (410) 
         */     
        static const Gone : Number = 410

        /** 
            HTTP Length Required Status (411) 
         */     
        static const LengthRequired : Number = 411
        
        /** 
            HTTP Precondition Failed Status (412) 
         */     
        static const PrecondFailed : Number = 412

        /** 
            HTTP Request Entity Too Large Status (413) 
         */     
        static const EntityTooLarge : Number = 413

        /** 
            HTTP Request URI Too Long Status (414)  
         */     
        static const UriTooLong : Number = 414

        /** 
            HTTP Unsupported Media Type (415) 
         */     
        static const UnsupportedMedia : Number = 415

        /** 
            HTTP Requested Range Not Satisfiable (416) 
         */     
        static const BadRange : Number = 416

        /** 
            HTTP Server Error Status (500) 
         */     
        static const ServerError : Number = 500

        /** 
            HTTP Not Implemented Status (501) 
         */     
        static const NotImplemented : Number = 501

        /** 
            HTTP Bad Gateway Status (502) 
         */     
        static const BadGateway : Number = 502

        /** 
            HTTP Service Unavailable Status (503) 
         */     
        static const ServiceUnavailable : Number = 503

        /** 
            HTTP Gateway Timeout Status (504) 
         */     
        static const GatewayTimeout : Number = 504

        /** 
            HTTP Http Version Not Supported Status (505) 
         */     
        static const VersionNotSupported: Number = 505

        /** 
            Create an Http object. The object is initialized with the Uri
            @param uri The (optional) Uri to initialize with.
            @throws IOError if the Uri is malformed.
         */
        native function Http(uri: Uri? = null)

        /** 
            @duplicate Stream.async
         */
        native function get async(): Boolean
        native function set async(enable: Boolean): Void

        /** 
            @duplicate Stream.close 
            This immediately closes any open network connection and resets the http object to be ready for another 
            connection.  Connections should be explicitly closed rather than relying on the garbage collector to 
            dispose of the Http object and automatically close the connection. If you have more requests that can 
            be issued on the same network connection, use reset() rather than close to prepare for a new request 
            on the same connection.
         */
        native function close(): Void 

        /** 
            Commence a HTTP request for the current method and uri. The HTTP method should be defined via the $method 
            property and Uri via the $uri property. This routine is typically not used. Rather it is invoked via one 
            of the Http methods $get(), $head(), $post() instead. This call, and the Http method calls  may not immediately
            initiate the connection. The Http class will delay connections until $finalize() is called explicitly or 
            implicitly reading $status or response content. This enables the request content length to be determined 
            automatically for smaller requests where the request body data can be buffered and measured before sending 
            the request headers.  
            @param method Http method. This is typically "GET" or "POST"
            @param uri New uri to use. This overrides any previously defined uri for the Http object.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server. Once the connection has been made, 
                exceptions will not be thrown and $status must be consulted for request status.
         */
        native function connect(method: String, uri: Uri? = null, ...data): Void

        /** 
            Filename of the certificate file for this HTTP object. The certificate is only used if secure
            communications are in use. Currently not implemented.
            @hide
         */
        native function get certificate(): Path
        native function set certificate(certFile: Path): Void

        /** 
            Response content body length. Set to the length of the response body in bytes or -1 if no body or not known.
            To set the request body Content-Length, use setHeader("Content-Length", Length)
         */
        native function get contentLength(): Number

        /** 
            Response content type derrived from the response Http Content-Type header.
            To set the request body Content-Type, use setHeader("Content-Type", MimeType)
         */
        native function get contentType(): String

        /** 
            When the response was generated. Response date derrived from the response Http Date header.
         */
        native function get date(): Date

        /** 
            Encoding scheme for serializing strings. The default encoding is UTF-8. Not yet implemented.
            @hide
         */
        function get encoding(): String
            "utf-8"

        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        /** 
            @hide
            Fetch a URL. This is a convenience method to asynchronously invoke an Http method without waiting. 
            It can be useful to wait for completion using App.waitForEvent(http, "close"))
            @param method Http method. This is typically "GET" or "POST"
            @param uri URL to fetch
            @param data Body data to send with the request. Set to null for no data. If set to null, the request
                will be finalized. If not set to null, $finalize() must be called after writing all data.
            @param callback Optional function to invoke on completion of the request.
          */
        function fetch(method: String, uri: Uri, data: *, callback: Function = null) {
            let xh = XMLHttp(this)
            xh.open(method, uri)
            xh.send(data)
            xh.onreadystatechange = function () {
                if (xh.readyState == XMLHttp.Loaded) {
                    response = xh.responseText
                    if (callback) {
                        if (callback.bound) {
                            callback()
                        } else {
                            callback.call(this)
                        }
                    }
                }
            }
        }

        /** 
            Signals the end of any write data and flushes any buffered write data to the client. 
         */
        native function finalize(): Void 

        /** 
            Has the request output been finalized. 
            @return True if the all the output has been written.
         */
        native function get finalized(): Boolean 

        /** 
            Flush request data. Calling $flush(Sream.WRITE) or $finalize() is required to ensure write data is sent to 
            the server.
            @duplicate Stream.flush
         */
        native function flush(dir: Number = Stream.WRITE): Void

        /** 
            Control whether redirects should be automatically followed by this Http object. When true, a redirected
            response will be followed and the redirected URL will be transparently re-fetched.  Default is false.
         */
        native function get followRedirects(): Boolean
        native function set followRedirects(flag: Boolean): Void

        /** 
            Commence a POST request with www-url encoded key=value data. See $connect() for connection details.
            This will encode each data objects as a string of "key=value" pairs separated by "&" characters.
            After writing data, $form() will call $finalize().
            @param uri Optional request uri. If non-null, this overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @param data Optional object hash of key value pairs to use as the post data. These are www-url-encoded and
                the content mime type is set to "application/x-www-form-urlencoded".
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function form(uri: Uri, data: Object): Void

        /**
FUTURE & KEEP
            Commence a POST request with form data the current uri. See $connect() for connection details.
            @param uri Optional request uri. If non-null, this overrides any previously defined uri for the Http object.
                If null, use the previously defined uri.
            @param data Data objects to pass with the POST request. The objects are json encoded and the Content-Type is
            set to "application/json". If you require "application/x-www-form-urlencoded" encoding, use publicForm().
            @throws IOError if the request cannot be issued to the remote server.
            @hide

            function publicForm(uri: Uri, ...data): Void
                connect("POST", uri, Uri.encodeObjects(data))
         */
        native function jsonForm(uri: Uri, ...data): Void

        /** 
            Commence a GET request for the current uri. See $connect() for connection details.
            This call initiates a GET request. It does not wait for the request to complete. 
            The $get() method will call finalize. If you need to send body content with a get request, use $connect(). 
            Once initiated, one of the $read or response routines  may be used to receive the response data.
            @param uri The uri to get. This overrides any previously defined uri for the Http object. If null, use
                a previously defined uri.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server. Once the connection has been made, 
                exceptions will not be thrown and $status must be consulted for request status.
         */
        native function get(uri: Uri? = null, ...data): Void

        /** 
            Get the (proposed) request headers that will be sent with the request. Use $headers to get the response
            headers or header(key) to get a single response header.
            @return The set of request headers that will be used when the request is sent.
         */
        native function getRequestHeaders(): Object

        /** 
            Commence a HEAD request for the current uri. See $connect() for connection details.
            @param uri The request uri. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function head(uri: Uri? = null): Void

        /** 
            Get the value of a single response header. This is a higher performance API than using response.headers["key"].
            @return The header field value as a string or null if not known.
         */
        native function header(key: String): String

        /** 
            Response headers. Use $header() to retrieve a single header value.
            Set to an object filled with all the response headers. If multiple headers of the same key value are
                defined, their contents will be catenated with a ", " separator as per the HTTP/1.1 specification.
         */
        native function get headers(): Object

        /** 
            Is the connection is utilizing SSL.
            @return True if the connection is using SSL.
         */
        native function get isSecure(): Boolean

        /** 
            Private key file for this Https object. NOT currently supported.
            @return The file name.
            @hide
         */
        native function get key(): Path
        native function set key(keyFile: Path): Void

        /** 
            When the response content was last modified. Set to the the value of the response Http Last-Modified header.
            Set to null if not known.
         */
        native function get lastModified(): Date

        /**
            Resource limits for requests.
            @param limits. Limits is an object hash with the following properties:
            @option chunk Maximum size of a chunk when using chunked transfer encoding.
            @option connReuse Maximum number of times to reuse a connection for requests (KeepAlive count).
            Chunked encoding will be used if the total body content length is unknown at the time the request headers 
            must be emitted. The Http class will typically buffer output until $flush is called and will often be able 
            to determine the content length even if a Content-Length header has not been explicitly defined. 
            @option headers Maximum number of headers in a response.
            @option header Maximum size of response headers.
            @option inactivityTimeout Maximum time in seconds to keep a connection open if idle. Set to zero for no timeout.
            @option receive Maximum size of incoming body data.
            @option requestTimeout Maximum time in seconds for a request to complete. Set to zero for no timeout.
            @option stageBuffer Maximum stage buffer size for each direction.
            @option transmission Maximum size of outgoing body data.
            @see setLimits
          */
        native function get limits(): Object

        /** 
            Http request method for this Http object. Default is "GET". Typical methods are: GET, POST, HEAD, OPTIONS, 
            PUT, DELETE and TRACE.
         */
        native function get method(): String
        native function set method(name: String)

        /** 
            @duplicate Stream.off
         */
        native function off(name, observer: Function): Void

        /** 
            @duplicate Stream.on
            All events are called with the following signature.  The "this" object will be set to the instance object
            if the callback is a method. Otherwise, "this" will be set to the Http instance. If Function.bind may also
            be used to define the "this" object and to inject additional callback arguments. 
                function (event: String, http: Http): Void
            @event headers Issued when the response headers have been fully received.
            @event readable Issued when some body content is available.
            @event writable Issued when the connection is writable to accept body data (PUT, POST).
            @event complete Issued when the request completes. Complete is always issued whether the request errors or not.
            @event error Issued if the request does not complete successfully. This is not issued if the request 
                ompletes successfully but with a non 200 Http status code.
         */
        native function on(name, observer: Function): Void

        /** 
            Initiate a POST request. This call initiates a POST request. It does not wait for the request to complete. 
            Posted data is NOT URL encoded. If you want to post data to a form, consider using the $form method instead 
            which automatically URL encodes the data. After writing data, $post() will call $finalize(). Post data may be 
            supplied may alternatively via $write. 
            @param uri Optional request uri. If non-null, this overrides any previously defined uri for the Http object. 
                If null, use a previously defined uri.
            @param data Data objects to send with the post request. Data is written raw and is not encoded or converted. 
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function post(uri: Uri, ...data): Void

        /** 
            Commence a PUT request for the current uri. See $connect() for connection details.
            If a contentLength has not been previously defined for this request, chunked transfer encoding will be enabled.
            @param uri The uri to put. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @param data Optional data objects to write to the request stream. Data is written raw and is not encoded 
                or converted.  However, put intelligently handles arrays such that, each element of the array will be 
                written. If encoding of put data is required, use the BinaryStream filter. If no putData is supplied,
                and the ContentLength header is non-zero you must call $write to supply the body data.
            @param data Optional object hash of key value pairs to use as the post data.
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function put(uri: Uri, ...data): Void

        /** 
            @duplicate Stream.read
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number

        /** 
            Read the response as a string. This call will block and should not be used in async mode.
            @param count of bytes to read. Returns the entire response contents if count is -1.
            @returns a string of $count characters beginning at the start of the response data.
            @throws IOError if an I/O error occurs.
         */
        native function readString(count: Number = -1): String

        /** 
            Read the request response as an array of lines. This call will block and should not be used in async mode.
            @param count of linese to read. Returns the entire response contents if count is -1.
            @returns an array of strings
            @throws IOError if an I/O error occurs.
         */
        function readLines(count: Number = -1): Array {
            let stream: TextStream = TextStream(this)
            result = stream.readLines()
            return result
        }

        /** 
            Read the request response as an XML document. This call will block and should not be used in async mode.
            @returns the response content as an XML object 
            @throws IOError if an I/O error occurs.
         */
        function readXml(): XML
            XML(response)

        /**
            Reset the Http object to prepare for a new request. This will discard existing headers and security 
            credentials. It will not close the connection.
         */
        native function reset(): Void

        /** 
            Response body content as a string. The first time this property is read, the response content will be read 
            and buffered.  Don't use this property in async mode as it will block. Set to the response as a string of 
            characters. If the response has no body content, the empty string will be returned.
            @throws IOError if an I/O error occurs.
         */
        native function get response(): String
        native function set response(data: String): Void

        /** 
            The maximum number of retries for a request. Retries are essential as the HTTP protocol permits a 
            server or network to be unreliable. The default retries is 2.
            @hide
         */
        native function get retries(): Number
        native function set retries(count: Number): Void

        /** 
            Get the ejs session cookie. This call extracts the ejs session cookie from the Http response headers.
            Ejscript sessions are identified by a client cookie which when transmitted with subsequent requests will 
            permit the server to locate the relevant session state store for the server-side application. 
            Use: setCookie(cookie) to transmit the cookie on subsquent requests.
         */
        function get sessionCookie() {
            let cookie = header("Set-Cookie")
            if (cookie) {
                return cookie.match(/(-ejs-session-=.*);/)[1]
            }
            return null
        }

        /**
            Set a "Cookie" header in the request headers. This is used to send a cookie to the server.
            @param cookie Cookie header value
         */
        function setCookie(cookie: String): Void
            setHeader("Cookie", cookie)

        /** 
            Set the user credentials to use if the request requires authentication.
            @param username String user name to use. If null, then reset the current credentials.
            @param password Un-encrypted string password to use 
         */
        native function setCredentials(username: String, password: String): Void

        /** 
            Set a request header. Use setHeaders() to set all the headers. Use $getRequestHeaders() to retrieve and examine
            the request header set.
            @param key The header keyword for the request, e.g. "accept".
            @param value The value to associate with the header, e.g. "yes"
            @param overwrite If the header is already defined and overwrite is true, then the new value will
                overwrite the old. If overwrite is false, the new value will be catenated to the old value with a ", "
                separator.
         */
        native function setHeader(key: String, value: String, overwrite: Boolean = true): Void

        /** 
            Set request headers. Use setHeader() to set a single header. Use $getRequestHeaders() to retrieve and examine 
            the request headers set.
            @param headers Object hash of headers to set.
            @param overwrite If true, the new set of headers completely replaces the existing set of request headers.
                If overwrite is false, a new header value will be catenated to the old value after a ", " separator.
         */
        native function setHeaders(headers: Object, overwrite: Boolean = true): Void

        /**
            Update the request resource limits. The supplied limit fields are updated.
            See $limit for limit field details.
            @param limits Object hash of limit fields and values
            @see limits
         */
        native function setLimits(limits: Object): Void

        /** 
            Http response status code from the Http response status line, e.g. 200. Set to null if unknown.
            This command will block until the request completes.
         */
        native function get status(): Number

        /** 
            Descriptive status message for the Http response. This message may come from either the HTTP response status
                line or from a local error message if the response fails to parse.
         */
        native function get statusMessage(): String

        /**
            @hide
         */
        function get success(): Boolean
            200 <= status && status < 300

        /**
            Configure request tracing for the request. The default is to trace the first line of requests and responses at
            level 2 and to trace headers at level 3. The options argument contains optional properties: rx and tx 
            (for receive and transmit tracing). The rx and tx properties may contain an object hash which describes 
            the tracing for that direction and includes any of the following fields:
            @param options. Set of trace options with properties "rx" and "tx" for receive and transmit direction tracing.
                The include property is an array of file extensions to include in tracing.
                The include property is an array of file extensions to exclude from tracing.
                The all property specifies that everything for this direction should be traced.
                The conn property specifies that new connections should be traced.
                The first property specifies that the first line of the request should be traced.
                The headers property specifies that the headers (including first line) of the request should be traced.
                The body property specifies that the body content of the request should be traced.
                The size property specifies a maximum body size in bytes that will be traced. Content beyond this limit 
                    will not be traced.
            @option transmit. Object hash with optional properties: include, exclude, first, headers, body, size.
            @option receive. Object hash with optional properties: include, exclude, conn, first, headers, body, size.
            @example:
                trace({
                    transmit: { exclude: ["gif", "png"], "headers": 3, "body": 4, size: 1000000 }
                    receive:  { "conn": 1, "headers": 2 , "body": 4, size: 1024 }
                })
          */
        native function trace(options: Object): Void

        /** 
            Upload files using multipart/mime. This routine initiates a POST request and sends the specified files
            and form fields using multipart mime encoding. This call is synchronous (blocks) until complete.
            @param uri The uri to upload to. This overrides any previously defined uri for the Http object.
            @param files Object hash of files to upload
            @param fields Object hash of of form fields to send
            @example
                fields = { name: "John Smith", address: "700 Park Avenue" }
                files = { file1: "a.txt, file2: "b.txt" }
                http.upload(URL, files, fields)
         */
        function upload(uri: String, files: Object, fields: Object? = null): Void {
            reset()
            let boundary = "<<Upload Boundary - " + md5(Date.now()) + ">>"
            setHeader("Content-Type", "multipart/form-data; boundary=" + boundary)
            post(uri)
            if (fields) {
                for (let [key,value] in fields) {
                    write('--' + boundary + "\r\n")
                    write('Content-Disposition: form-data; name=' + Uri.encode(key) + "\r\n")
                    write('Content-Type: application/x-www-form-urlencoded\r\n\r\n')
                    write(Uri.encode(value) + "\r\n")
                }
            }
            for (let [key,file] in files) {
                write('--' + boundary + "\r\n")
                write('Content-Disposition: form-data; name=' + key + '; filename=' + Path(file).basename + "\r\n")
                write('Content-Type: ' + Uri(file).mimeType + "\r\n\r\n")

                let f = File(file, "r")
                let data = new ByteArray(System.Bufsize, false)
                while (f.read(data) > 0) {
                    let n = write(data)
                }
                f.close()
                write("\r\n")
            }
            write('--' + boundary + "--\r\n\r\n")
            finalize()
        }

        /** 
            The current Uri for this Http object. The Uri is used for the request URL when making a $connect call.
         */
        native function get uri(): Uri
        native function set uri(newUri: Uri): Void

        /** 
            Wait for a request to complete. This will call $finalize() if in sync mode and the request is not already 
            finalized.
            @param timeout Timeout in milliseconds to wait for the request to complete. A timeout of zero means no 
            timeout, ie. wait forever. A timeout of < 0 (default), means use the default request timeout.
            @return True if the request successfully completes.
         */
        native function wait(timeout: Number = -1): Boolean

        /** 
            Write body data to the server. This will buffer the written data until either $flush() or $finalize() is called. 
            The Http "Content-Length" header should normally be set prior to writing any data for optimial data transfter.
            If the Content-Length header has not been defined, the data will be transferred using chunked transfers. 
            @duplicate Stream.write
         */
        native function write(...data): Number

        /* ***************************************** Legacy *******************************************/

        /** 
            @hide 
            @deprecated 1.0.0
         */
        # Config.Legacy
        function addHeader(key: String, value: String, overwrite: Boolean = true): Void
            setHeader(key, value, overwrite)

        /** 
            The number of response data bytes that are currently available for reading.
            @returns The number of available bytes.
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        native function get available(): Number 

        /** 
            @hide 
            @deprecated 1.0.0
         */
        # Config.Legacy
        function get bodyLength(): Void
            contentLength

        # Config.Legacy
        function set bodyLength(value: Number): Void
            setHeader("content-length", value)

        /** 
            @hide 
            @deprecated 1.0.0
         */
        # Config.Legacy
        function get chunked(): Boolean
            chunksize != 0

        # Config.Legacy
        function set chunked(enable: Boolean): Void
            chunkSize = (enable) ? 8192 : 0

        /** 
            @hide 
            @deprecated 1.0.0
         */
        # Config.Legacy
        function get code(): Number
            status

        /** 
            @hide 
            @deprecated 1.0.0
         */
        # Config.Legacy
        function get codeString(): String
            statusMessage

        /**
            Get the value of the content encoding of the response.
            @return A string with the content type or null if not known.
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        function get contentEncoding(): String
            header("content-encoding")

        /** 
            Commence a DELETE request for the current uri. See $connect() for connection details.
            @param uri The uri to delete. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server.
            @deprecated 1.0.0
            @hide
         */
        # Config.Legacy
        native function del(uri: Uri? = null, ...data): Void

        /** 
            When the response content expires. This is derrived from the response Http Expires header.
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        function get expires(): Date
            Date.parseUTCDate(header("expires"))

        /**
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        static function mimeType(path: String): String
            Uri(path)..mimeType

        /** 
            Commence an OPTIONS request for the current uri. See $connect() for connection details.
            @param uri New uri to use. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @throws IOError if the request cannot be issued to the remote server.
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        native function options(uri: Uri? = null): Void

        /**
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        function setCallback(eventMask: Number, cb: Function): Void {
            on("" + eventMask, cb);
        }

        /** 
            Commence a TRACE request for the current uri. See $connect() for connection details.
            @param uri New uri to use. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @throws IOError if the request cannot be issued to the remote server.
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        native function trace_old(uri: Uri? = null): Void

    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.
    
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
