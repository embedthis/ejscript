/*
    Server for blocking.tst
 */
let PORT = App.args[1] || 6701
server = new Socket
server.async = true

server.on("accept", function(event, listenSock) {
    let sock = listenSock.accept()
    sock.on("readable", function(event, sock) {
        data = new ByteArray
        sock.read(data)
        sock.write("ECHO " + data.toString().trim())
        if (data.toString().contains("\r\n\r\n")) {
            App.exit()
        }
    })
})
server.listen(PORT)
App.run()
