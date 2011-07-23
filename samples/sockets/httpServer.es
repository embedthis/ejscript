/*
    Demonstrate the Socket API. This is a mini (non-production) HTTP server.
    NOTE: Real web servers support Keep-Alive, Chunking, Ranged requests, PUT, DELETE etc.
    This web server just responds to simple GET requests.
 */

module HttpServer {

    var methods = { "DELETE", "GET", "OPTIONS", "POST", "PUT", "TRACE" }

    class Response {
        var conn: Socket
        var code: Number = 200
        var headers: Object

        var codeMessages = {
             "100": "Continue", "200": "OK", "201": "Created", "202": "Accepted", "204": "No Content", 
             "205": "Reset Content", "206": "Partial Content", "301": "Moved Permanently", "302": "Moved Temporarily", 
             "304": "Not Modified", "305": "Use Proxy", "307": "Temporary Redirect", "400": "Bad Request", 
             "401": "Unauthorized", "402": "Payment Required", "403": "Forbidden", "404": "Not Found", 
             "405": "Method Not Allowed", "406": "Not Acceptable", "408": "Request Time-out", "409": "Conflict", 
             "410": "Length Required", "411": "Length Required", "413": "Request Entity Too Large", 
             "414": "Request-URI Too Large", "415": "Unsupported Media Type", "416": "Requested Range Not Satisfiable", 
             "417": "Expectation Failed", "500": "Internal Server Error", "501": "Not Implemented", "502": "Bad Gateway", 
             "503": "Service Unavailable", "504": "Gateway Time-out", "505": "Http Version Not Supported", 
             "507": "Insufficient Storage",
        }

        function Response(conn: Socket) {
            this.conn = conn
        }

        function writeHeaders() {
            msg = codeMessages[code] || "Error"
            conn.write(protocol + " " + code + " " + msg + "\r\n")
            headers["ContentLength"] ||= 0
            for (header in headers) {
                conn.write(header + ":" + headers[header] + "\r\n")
            }
            conn.write("\r\n")
        }

        function sendFile(uri: String) {
            path = documents.join(uri)
            if (!exists(path)) {
                throw "Can't open: " + path
            }
            file = path.open()
            while (file.read(data) >= 0) {
                conn.write(data)
            }
        }
    }

    class Request {
        var conn: Socket
        var contentLength: Number
        var headers: Object = {}
        var keepAlive: Number = Number.MaxValue
        var method: String
        var protocol: String
        var resp: Response
        var uri: String

        function Request(conn: Socket) {
            this.conn = conn
        }

        function serve(conn: Socket) {
            ts = new TextStream(conn)
            parseFirst(ts.readLine())
            while ((line = ts.readLine() != "") {
                parseHeader(line)
            }
            parseContent()
            validate()
            respond()
        }

        function parseFirst(line: String) {
            //  TODO - better way to tokenize (destructuring assignment)
            parts = line.split(" ")
            method = parts[0].toUpper()
            uri = parts[1]
            protocol = parts[2]

            if (protocol == "HTTP/1.0") {
                req.keepAlive = 0
            } else if (protocol != "HTTP/1.1") {
                throw "Bad protocol"
            }
            if (methods[method] == undefined) {
                throw "Bad method"
            }
        }

        function parseHeader(line: String) {
            parts = line.split(":")
            key = parts[0].toUpper()
            value = parts[1].trim()
            headers[key] = value
            switch (key) {
            case "CONTENT_LENGTH":
                contentLength = value cast Number
            }
        }

        function parseContent() {
            //  TODO
        }

        function validate() {
            //  TODO - validate URL and headers
        }

        function respond() {
            resp = new Response(conn)
            //  TODO - set mime type
            resp.setHeader("ContentLength", path.size())
            resp.writeHeaders()
            resp.sendFile(path)
        }
    }

    class Server {
        private var documents: Path
        private var endpoint: Socket
        private var port: Number
        private var ip: String

        function Server(path: String, port: Number, ip: String = "") {
            documents = Path(path)
            this.port = port
            this.ip = ip
            endpoint = new Socket
        }

        function run() {
            while ((conn = endpoint.listen(port, ip)) != null) {
                do {
                    req = new Request(conn)
                    try {
                        req.serve(line)
                    } catch (e) {
                        error("Error serving request: " + e)
                    }
                } while (conn.keepAlive-- > 0);
            }
        }

    }
}

require HttpServer

//  TODO - command line args for documentRoot, port and IP address
var server = new Server(".", port, "")
server.run()
