/*
    FileSystem testing
 */

fs = new FileSystem("/")

//  newline

assert(fs.newline is String)
assert(fs.newline.length >= 1)

//  root

assert(fs.root.toString() == Path("/").absolute)
assert(FileSystem("/a/b/c").root == Path("/").absolute)

//  separator

if (Config.OS == "windows") {
    assert(fs.separators == "\\/")
} else if (Config.OS == "cygwin") {
    assert(fs.separators == "/\\")
} else {
    assert(fs.separators == "/")
}

//  FUTURE assert(fs.isReady)
//  FUTURE assert(fs.isWritable)
//  FUTURE assert(fs.size > 0);
//  FUTURE assert(fs.space > 0);

