/*
    Server for ipv6.tst

    ejs ipv6.es PORT
    Echo back received data. Terminate on "\r\n\r\n".
 */

let PORT = App.args[1] || "[::]:6701"
server = new Socket
server.listen(PORT)

while (sock = server.accept()) {
print("ACCEPT")
    data = new ByteArray
    while (sock.read(data)) {
print("GOT " + data)
        sock.write("ECHO " + data.toString().trim())
        if (data.toString().contains("\r\n\r\n")) {
            sock.close()
print("DONE")
            App.exit()
            break
        }
        data.flush()
    }
    sock.close()
}
App.run()
