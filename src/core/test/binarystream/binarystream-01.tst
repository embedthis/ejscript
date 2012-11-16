/*
    Test basic BinaryStream
 */
const TestFile: String = "file.dat"
const TestLength = 500

/*
//  Test using loopback so we can read, written data
    
var ba = new ByteArray
var s = BinaryStream(ba)

s.writeInteger(4000)
assert(ba.length == 4 || ba.length == 8)
v = s.readInteger()
assert(v == 4000)


//	Read binary data

bs = Path(TestFile).openBinaryStream("r")
assert(bs.readString(5) == "12345")
assert(String.fromCharCode(bs.readByte()) == "6")
assert(String.fromCharCode(bs.readByte()) == "7")
assert(String.fromCharCode(bs.readByte()) == "8")
assert(String.fromCharCode(bs.readByte()) == "9")
assert(String.fromCharCode(bs.readByte()) == " ")

data = new ByteArray(500)
assert(bs.read(data, 0, data.size) == 490)
assert((data.readString(-1)).size == 490)
*/


//	Write binary data

/* MOB BUG
bs = Path("temp.dat").openBinaryStream("w")
bs.write("Hello world")
bs.flush()
bs.close()
f = new File("temp.dat", "r")
assert(f.size == 11)
f.close()
*/

/*XXX
//	endian encodings. TODO - this is not really testing much
//  MOB -- more here

bs = Path("temp.dat").openBinaryStream("w")
bs.endian = BinaryStream.BigEndian
for (i in 1024) {
	bs.writeInteger(i)
}
bs.close()
f = new File("temp.dat", "r")
assert(f.size == 4096 || f.size == 8192)
f.close()
XXX*/

Path("temp.dat").remove()
