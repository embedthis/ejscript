/*
    Path.files testing
 */

//  files

files = Path(".").files(true)
assert(files is Array)
assert(files.length >= 2)
assert(files.find(function (e) { return e.same("file.dat"); }) != null)
