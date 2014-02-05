/*
    Test Zlib string routines
 */

require ejs.zlib

let src = 'abcdef'
let dest = Zlib.compressString(src)
let result = Zlib.uncompressString(dest)
assert(src == result)

src = 'abcdef'.times(50)
dest = Zlib.compressString(src)
result = Zlib.uncompressString(dest)
assert(src == result)

src = 'abcdef'.times(500)
dest = Zlib.compressString(src)
result = Zlib.uncompressString(dest)
assert(src == result)
