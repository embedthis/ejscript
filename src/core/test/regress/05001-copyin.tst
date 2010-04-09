/*
    ByteArray.copyIn was not handling a negative count
 */

var src = new ByteArray
src.write("Helo World")
var dest = new ByteArray
var len = dest.copyIn(0, src, 0, -1)
assert(len > 0)
