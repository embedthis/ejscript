//OK  Blocking client HTTP request
var client: Socket = new Socket
client.connect(4100)
client.write("POST /index.html HTTP/1.1\r\nConnection: Keep-Alive\r\nContent-Length: 10\r\n\r\n0123456789POST /index.html HTTP/1.1\r\nConnection: Keep-Alive\r\nContent-Length: 10\r\n\r\nabcdefghij")
data = new ByteArray
while ((len = client.read(data))) {
    print("LEN " + len)
    print(data)
}
print("BEFORE CLOSE")
client.close()
print("AFTER CLOSE")
