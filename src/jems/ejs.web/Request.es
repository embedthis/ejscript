/**
    Request.es -- Ejscript web request object. Request objects support the JSGI protocol specification. They can manage
    JSGI apps with a ".es" extension, templated web pages with a ".ejs" extension and are used as a foundation for
    MVC web applications.
 */
module ejs.web {

    /**
        Web request class. Request objects manage the state and execution of a web request. The HttpServer class creates
        instances of Request in response to incoming client requests. The Request object holds the client request state
        including the request URL and headers. It is also a Stream and by calling the read() method, request body 
        content can be read.

        The response to send back to the client can be defined by setting status and calling setHeaders() and write() to 
        set the response status, headers and body content respectively.
        @spec ejs
        @stability prototype
     */
    dynamic enumerable class Request implements Stream {
        use default namespace public

        /** 
            Absolute Uri for the top-level of the application. This returns a absolute Uri (includes scheme and host) 
            for the top most application Uri. See $home to get a relative Uri.
         */ 
        native var absHome: Uri

        /** 
            Authentication group. This property is set to the value of the authentication group header. 
         */
        native var authGroup: String

        /** 
            Authentication method if authorization is being used (basic or digest). Set to null if not using authentication. 
         */
        native var authType: String

        /** 
            Authentication user name. This property is set to the value of the authentication user header. Set to null if
            not yet defined.
         */
        native var authUser: String

        /** 
            Preferred response chunk size for transfer chunk encoding. Chunked encoding is used when an explicit request 
            content length is unknown at the time the response headers must be emitted.  Chunked encoding is automatically 
            enabled if the chunkFilter is configured and a contentLength has not been defined.
         */
        native var chunkSize: Number

        /** 
            Request configuration. Initially refers to App.config which is filled with the aggregated "ejsrc" content.
            Middleware may modify to refer to a request local configuration object.
         */
        var config: Object

        /** 
            Get the request content length. This property is readonly and is set to the length of the request content 
            body in bytes or -1 if not known.
         */
        native var contentLength: Number

        /** 
            The request content type as specified by the "Content-Type" Http request header. This is set to null 
            if not defined.
         */
        native var contentType: String

        /** 
            Cookie headers. Cookies are sent by the client browser via the Set-Cookie Http header. They are typically 
            used used to specify the session state. If sessions are being used, an Ejscript session cookie will be 
            sent to and from the browser with each request. 
         */
        native var cookies: Object

        /** 
            Application web document directory on the local file system. This is set to the directory containing the
            application. For MVC applications, this is set to the base directory of the application. For non-MVC apps, 
            it is set to the directory containing the application startup script.
         */
        native var dir: Path

        /** 
            Get the encoding scheme for serializing strings. Not yet implemented.
         */
        native var encoding: String

        /** 
            Files uploaded as part of the request. For each uploaded file, an instance of UploadFile is created in files. 
            Each element is named by the file upload HTML input element ID in the HTML page form. 
         */
        native var files: Object

        /** 
            Request Http headers. This is an object hash filled with the Http request headers. If multiple headers of 
            the same key value are defined, their contents will be catenated with a ", " separator as per the HTTP/1.1 
            specification. Use the header() method if you want to retrieve a single header.
         */
        native var headers: Object

        /** 
            Relative Uri for the top-level of the application. This returns a relative Uri from the current request
            up to the top most application Uri.
         */ 
        native var home: Uri

        /** 
            Client requested Host. This is the Http request "Host" header value.
         */
        native var host: String

        /** 
            Logger object. Set to App.logger. This is configured from the "log" section of the "ejsrc" config file.
         */
        function get log(): Logger 
            App.logger

        /** 
            Request HTTP method. String containing the Http method (DELETE | GET | POST | PUT | OPTIONS | TRACE)
         */
        native var method: String

        /** 
            The request form parameters. Object hash of user url-encoded post data parameters.
         */
        native var params: Object

        /** 
            Portion of the request URL after the scriptName. This is the location of the request within the application.
         */
        native var pathInfo: String

        /** 
            Request query string. This is the portion of the Uri after the "?". Set to null if there is no query.
         */
        native var query: String

        /** 
            Name of the referring URL. This comes from the request "Referrer" Http header. Set to null if there is
            no defined referrer.
         */
        native var referrer: String

        /** 
            IP address of the client issuing the request. 
         */
        native var remoteAddress: String

        /** 
            Route used for the request. The route is the matching entry in the route table for the request.
            The route has properties two properties of particular interest: "name" which is the name of the route and
            and "type" which classifies the type of request. 
         */
        var route: Route

        /** 
            Http request protocol scheme (http | https)
         */
        native var scheme: String

        /** 
            Script name for the current application serving the request. This is typically the leading Uri portion 
            corresponding to the application, but middleware may modify this to be an arbitrary string representing 
            the application.  The script name is typically determined by the Router as it parses the request using 
            the routing tables.
         */
        native var scriptName: String

        /** 
            Flag indicating if the request is using secure communications. This means that TLS/SSL is the underlying
            protocol scheme.
         */
        native var secure: Boolean

        /** 
            Owning server for the request. This is the HttpServer object that created this request.
         */
        native var server: HttpServer

        /** 
            Session state object. The session state object can be used to share state between requests.
            If a session has not already been created, this call creates a session and sets the $sessionID property. 
            A cookie containing a session ID is automatically created and sent to the client on the first 
            response after creating the session. Objects stored in the session state must be serializable using JSON.
         */
        native var session: Object 

        /** 
            Current session ID. Index into the $sessions object. Set to null if no session is defined.
         */
        native var sessionID: String

        /** 
            Set to the (proposed) Http response status code.
         */
        native var status: Number

        /** 
            Request timeout. Number of milliseconds for requests to block while processing the request.
            A value of -1 means no timeout.
         */
        native var timeout: Number

        /**
            The request URL as a parsed Uri. This is the original Uri and may not reflect changes to pathInfo or
            scriptName.
         */
        native var uri: Uri

        /** 
            Get the name of the client browser software set in the "User-Agent" Http header 
         */
        native var userAgent: String

        /* ************************************* Methods ******************************************/

        /** 
            @duplicate Stream.addListener
            @event readable Issued when some body content is available.
            @event writable Issued when the connection is writable to accept body data (PUT, POST).
            @event complete Issued when the request completes
            @event error Issued if the request does not complete
            All events are called with the signature:
            function (event: String, http: Http): Void
         */
        native function addListener(name, listener: Function): Void

        /** 
            @duplicate Stream.async
         */
        native function get async(): Boolean

        /** 
            @duplicate Stream.async
         */
        native function set async(enable: Boolean): Void

        /**
            Control the caching of the response content. Setting cacheable to false will add a Cache-Control: no-cache
            header to the output
            @param enable Set to false (default) to disable caching of the response content.
         */
        function cachable(enable: Boolean = false): Void {
            //  MOB -- need more control over this. Consult the spec
            if (!cache) {
                setHeader("Cache-Control", "no-cache", false)
            }
        }

        /** 
            @duplicate Stream.close
            This closes the current request.
         */
        native function close(): Void

        /** 
            Dump objects for debugging
            @param args List of arguments to print.
         */
        function dump(...args): Void {
            for each (var e: Object in args) {
                write(serialize(e, {pretty: true}) + "\r\n")
            }
        }

        /** 
            Destroy a session. This call destroys the session state store that is being used for the current client. 
            If no session exists, this call has no effect.
         */
        native function destroySession(): Void

        /** 
            Get the file extension of the script corresponding to the request 
         */
        function get extension() 
            Path(pathInfo).extension

        /** 
            Signals the end of any write data. If using chunked writes (no content length specified), finalize() must
            be called to properly signify the end of write data. This causes the chunk filter to write a chunk trailer.
         */
        native function finalize(): Void 

        /** 
            @duplicate Stream.flush
            @hide 
         */
        function flush(): Void {}

        /** 
            Get the (proposed) response headers
            @return The set of response headers that will be used when the response is sent.
         */
        native function getResponseHeaders(): Object

//  MOB -- will this work case insensitive?
        /** 
            Get a request header by keyword. Header properties are lower case. ie. "content_length". This is higher 
            performance than using request.headers["key"].
            @return The header value
         */
        native function header(key: String): String

        /** 
            @duplicate Stream.read
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number 

        /** 
            Redirect the client to a new URL. This call redirects the client's browser to a new location specified 
            by the @url.  Optionally, a redirection code may be provided. Normally this code is set to be the HTTP 
            code 302 which means a temporary redirect. A 301, permanent redirect code may be explicitly set.
            @param url Url to redirect the client to
            @param status Optional HTTP redirection status
         */
        function redirect(url: String, status: Number = Http.MovedTemporarily): Void {
            if (!url.contains("://")) {
                if (url[0] == '/') {
                    uri = Url({ scheme: scheme, host: serverName, port: port, path: url})
                } else {
                    url = Url({ scheme: scheme, host: serverName, port: port, path: scriptName + pathInfo + url})
                }
                url = uri.normalize.toString()
            }
            this.status = status
            setHeader("Location", url)
            write("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n" +
                   "<html><head><title>Redirect (" + status + ")</title></head>\r\n" +
                    "<body><h1>Redirect (" + status + ")</h1>\r\n</H1>\r\n" + 
                    "<p>The document has moved <a href=\"" + url + 
                    "\">here</a>.</p>\r\n" +
                    "<address>" + server.software + " at " + serverName + " Port " + server.port + 
                    "</address></body>\r\n</html>\r\n")
        }

        /** 
            @duplicate Stream.removeListener 
         */
        native function removeListener(name, listener: Function): Void

        //    MOB - doc
        /** @hide */
        function makeUri(parts: Object): Uri {
            if (route) {
                return route.makeUri(this, blend(params.clone(), parts))
            }
            let uri = request.absHome.components()
            for each (part in parts) {
                blend(uri, part)
            }
            return Uri(uri)
        }

        /**
            Convenience routine to define an application at a given Uri prefix and directory location. This is typically
                called from routing tables.
            @param prefix The leading Uri prefix for the application. This prefix is removed from the pathInfo and the
                $scriptName property is set to the prefix after removing the leading "/".
            @param location Path to where the application home directory is. This sets the $dir property to the $location
                argument.
        */
        function setLocation(prefix: String, location: Path): Void {
            dir = location
            pathInfo = pathInfo.trimStart(prefix)
            scriptName = prefix.trimStart("/")
        }

        /** 
            Send a response to the client. This can be used instead of setting status and calling setHeaders() and write(). 
            The $response argument is an object hash containing status, headers and
            body properties. The respond method replaces previously defined status and headers.
            @option status Numeric Http status code (e.g. 200 for a successful response)
            @option header Object hash of Http headers
            @option body Body content
        */
        function sendResponse(response: Object): Void {
            status = response.status || 200
            if (response.headers)
                setHeaders(response.headers)
            if (response.body)
                write(body)
            finalize()
        }

        /** 
            Define a cookie header to send with the response. Path, domain and lifetime can be set to null for 
                default values.
            @param name Cookie name
            @param options Cookie field options
            @options value Cookie value
            @options path Uri path to which the cookie applies
            @options domain Domain in which the cookie applies. Must have 2-3 dots.
            @options lifetime Duration for the cookie to persist in seconds
            @options secure Set to true if the cookie only applies for SSL based connections
         */
        function setCookie(name: String, options: Object) {
            options.path ||= "/"
            let value = encodeUri(name) + "="
            value += "; path=" + options.path
            if (options.domain)
                value += "domain=" + options.domain
            if (options.expires)
                value += "; expires= " + options.expires.toUTCString()
            if (options.secure)
                value += "; secure"
            setHeader("Set-Cookie", value)
            setHeader("Cache-control", "no-cache=\"set-cookie\"")
        }

        /** 
            Set a header. If a header has already been defined and $overwrite is true, the header will be overwritten.
            NOTE: case does not matter in the header keyword.
            @param key The header keyword for the request, e.g. "accept".
            @param value The value to associate with the header, e.g. "yes"
            @param overwrite If the header is already defined and overwrite is true, then the new value will
                overwrite the old. If overwrite is false, the new value will be catenated to the old value with a ", "
                separator.
         */
        native function setHeader(key: String, value: String, overwrite: Boolean = true): Void

        /**
            Set the HTTP response headers. Use getHeaders to inspect the response headers.
            @param headers Set of headers to use
            @param overwrite If the header is already defined and overwrite is true, then the new value will
                overwrite the old. If overwrite is false, the new value will be catenated to the old value with a ", "
                separator.
         */
        function setHeaders(headers: Object, overwrite: Boolean = true): Void {
            for (key in headers) {
                setHeader(key, headers[key], overwrite)
            }
        }

        /** 
            Set to the (proposed) Http response status code. This is equivalent to assigning to the $status property.
         */
        function setStatus(status: Number): Void {
            this.status = status
        }

        /** 
            @duplicate Stream.write
            Write data to the client
         */
        native function write(...data): Number

        /** 
            Write an error message back to the user. The status is set to Http.ServerError (500) and the content type
            is set to text/html. The output is html escaped for security. Output is finalized.
            @param msg Message to send. The message may be modified for readability if it contains an exception backtrace.
         */
        function writeError(msg: String, code: Number = Http.ServerError): Void {
            let text
            status = code
            msg = msg.replace(/.*Error Exception: /, "")
            if (config.log.showClient) {
                setHeader("Content-Type", "text/html")
                text = "<h1>Request error for \"" + pathInfo + "\"</h1>\r\n"
                text += "<pre>" + escapeHtml(msg) + "</pre>\r\n"
                text += '<p>To prevent errors being displayed in the "browser, ' + 
                    'set <b>errors.showClient</b> to false in the ejsrc file.</p>\r\n'
            } else {
                text = "<h1>Request error for \"" + pathInfo + "\"</h1>\r\n"
            }
            try {
                write(text)
            } catch {}
            finalize()
            log.error(status + ". " + msg)
        }

        /** 
            Send text back to the client which is first HTML escaped.
            @param args Objects to emit
         */
        function writeHtml(...args): Void
            write(html(args))

        /* ******************************************** JSGI  ********************************************************/
        /** 
            JSGI specification configuration object.
            @spec jsgi-0.3
         */
        static var jsgi: Object = {
            errors: App.logger,
            version: [0,3],
            multithread: true,
            multiprocess: false,
            runonce: false,
        }

        /** 
            Storage for middleware specific state. Added for JSGI compliance.
            @spec jsgi-0.3
         */
        native var env: Object

        /**
            Request content stream. This is equivalent to using "this" as Request objects are streams connected to the
            input content. Added for JSGI compliance.
            @spec jsgi-0.3
            @returns Stream object equal to the value of "this" request instance.
        */
        function get input(): Stream {
            return this
        }

        /** 
            Decoded query string (URL query string). Eqivalent to the $query property. Added for JSGI compliance
            @spec jsgi-0.3
            @return A string containing the request query. Returns an empty string if there is no query.
         */
        function get queryString(): String
            query

        /**
            Name of the server responding to the request. This is the portion of the URL that follows the scheme.
            If a "Host" header is provided, it is used in preference.
            @returns A string containing the server name.
         */
        function get serverName(): String {
            return (host) ? host : uri.host
        }

        /**
            Listening port number for the server
            @returns A number set to the TCP/IP port for the listening socket.
         */
        function get serverPort(): Number
            server.port
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
