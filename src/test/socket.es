
/*
var server: Socket = new Socket

//OK  Blocking server
server.listen("localhost:9999")
while (sock = server.accept()) {
    print("\nNew connection")
    data = new ByteArray
    while (sock.read(data)) {
        print(data)
    }
    sock.close()
}

//OK  Blocking client HTTP request
var client: Socket = new Socket
client.connect(9999)
client.write("GET / HTTP/1.1\r\nConnection: close\r\n\r\n")
data = new ByteArray
while (client.read(data)) {
    print(data)
}


//OK  Non-blocking server with events
server = new Socket
server.async = true
server.addListener("readable", function(event, listenSock) {
    //  TODO - or pass in the accepted socket and then no requirement to do accept
    let sock = listenSock.accept()
    sock.addListener("read", function(event, sock) {
        data = new ByteArray
        if (sock.read(data)) {
            print(data)
        } else {
            sock.close()
        }
    })
})
server.listen(9999)
*/


/*
//  Non-blocking client with events
client = new Socket
client.connect(9999)
client.addListener("connect", function(event, sock) {
    //  TODO - should get write event at the start
    client.addListener("write", function(event, sock) {
        print("GOT WRITE EVENT")
    }
    sock.write(Path("data.txt").readString())
})
App.serviceEvents()
*/

/*
    //  Non-blocking server with promises
    server = new Socket
    promise = server.listen(9999, true)
    promise.addCallback(function(event, sock) {
        sock.accept()
        sock.read(data).addCallback(function(event, sock) {
            if (sock.read(data)) {
                print(data)
            } else {
                sock.close()
            }
        })
    }).addCallback(function() { print("AFTER"); })


    //  Non-blocking client with promises
    client = Socket
    client.async = true
    promise = client.connect(9999)
    promise.addCallback(function(event, sock) {
        sock.write("GET / HTTP/1.1\r\n\r\n", 0, -1, true).addCallback(function() {
            data = new ByteArray
            sock.read(data).addCallback(function(event, sock) {
                if (sock.read(data)) {
                    print(data)
                } else {
                    sock.close()
                }
            })
        })
    })

 */



var http = new Http
const URL = "127.0.0.1:4100"

/*OK1 - NON blocking
http.get(URL + "/index.html")
assert(http.status == 200)
buf = new ByteArray 
let gotData = false
while (http.read(buf)) {
    print("GOT " + buf.available)
    gotData = true
}
assert(gotData == true)
print("DONE")
*/


let path = Path("data.dat")
let file = path.open("r")
http.async = true

//  TODO - should issue writable event in any case. Even if content length not set. 
//  If write done without content length, should auto set chunked

// http.chunked = true
// http.contentLength = path.size

http.addListener("writable", function (event, http) {
    p("WRITABLE event")
    let buf = new ByteArray
    if (file.read(buf)) {
        p("WRITE ", buf.available)
        http.write(buf)
    } else {
        p("FINALIZE writing")
        http.finalize()
    }
})

http.addListener("headers", function (event, http) {
    p("GOT HEADERS status", http.status)
    print("GOT  content length " + http.contentLength)
})

http.addListener("readable", function (event, http) {
    print("READABLE CODE ")
    let data = new ByteArray
    http.read(data)
    print("GOT " + data)
})

http.addListener("complete", function (event, http) {
    print("Complete event")
    //  MOB -- should close - close the connection?
    // http.close()
})

http.addListener("error", function (event, http) {
    print("Error event")
})

http.post(URL + "/form.ejs")
// http.put(URL + "/tmp/test.dat")
// http.get(URL + "/index.html")
// http.wait()
App.serviceEvents()
