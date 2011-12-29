/*
    Server for ipv6.tst

    ejs ipv6.es ADDRESS
    Echo back received data. Terminate on "\r\n\r\n".
 */

let ADDRESS = App.args[1] || "[::]:6701"
server = new Socket
server.listen(ADDRESS)

while (sock = server.accept()) {
    data = new ByteArray
    while (sock.read(data)) {
        sock.write("ECHO " + data.toString().trim())
        if (data.toString().contains("\r\n\r\n")) {
            sock.close()
            App.exit()
            break
        }
        data.flush()
    }
    sock.close()
}
App.run()
