/*
    Test Zlib byte routines
 */

require ejs.zlib

let src = new ByteArray(100)
for (i in 100) {
    src.writeByte(i)
}
let dest = Zlib.compressBytes(src)
let result = Zlib.uncompressBytes(dest)
for (i in 100) {
    assert(result.readByte() == i)
}
