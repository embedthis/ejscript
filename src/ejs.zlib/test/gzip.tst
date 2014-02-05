/*
    Test Zlib.compress/uncompress
 */

require ejs.zlib

let orig = Path('orig.dat')
let dest = Path('').temp().joinExt('gz')

Zlib.compress(orig, dest)
assert(dest.exists)

let dest2 = Path('').temp().joinExt('gz2')
Zlib.uncompress(dest, dest2)
assert(dest2.exists)

assert(orig.readString() == dest2.readString())
dest.remove()
dest2.remove()
