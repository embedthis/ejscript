/*
    Was failing with an explicit -1 in copyIn
 */
source = new ByteArray
source.write("Hello")

dest = new ByteArray

//   This -1 was failing. Omitted was working

dest.copyIn(0, source, 0, -1)
dest.writePosition += source.length
assert(dest == "Hello")
