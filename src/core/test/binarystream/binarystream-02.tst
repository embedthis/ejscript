/*
    Test binary stream with loopback
 */

var ba = new ByteArray
var s = BinaryStream(ba)

s.writeInteger(4000)
assert(ba.length == 4 || ba.length == 8)
v = s.readInteger()
assert(v == 4000)
