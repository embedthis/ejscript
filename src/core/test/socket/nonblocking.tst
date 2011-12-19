/*
    Test client and server in blocking mode
 */
const PORT: Number = ((App.config.test.http_port cast Number) || 6700) + 1
const TIMEOUT = 5000

if (!App.getenv("NOSERVER")) {
    var server = Cmd("ejs nonblocking.es " + PORT, {detach: true})
    if (server.wait(100)) {
        App.log.error("Can't start nonblocking.es " + server.error)
    }
}
var client: Socket = new Socket

for (i in TIMEOUT/100) {
    try {
        client.connect(PORT)
        break
    } catch { App.sleep(100) }
}
let count = client.write("ABCDEF\r\n\r\n")
assert(count == 10)

data = new ByteArray
while (client.read(data)) {
    assert(data == "ECHO ABCDEF")
}
assert(data == "ECHO ABCDEF")
client.close()

if (server) {
    if (!server.wait(TIMEOUT)) {
        Cmd.kill(server.pid)
        assert(server.wait(0))
    }
    assert(server.status == 0)
}
