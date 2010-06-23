/*
    Uri normalize tests
 */

function check(s, expected) {
    let norm = Uri(s).normalize
    assert(norm == expected)
}

check("", "")
check(" ", " ")
check(".", "")
check("..", "")
check("/", "/")
check("abc", "abc")
check("/abc", "/abc")
check("/abc/", "/abc/")


//  Dot prefixes and suffixes
check("/.", "/")
check("abc/.", "abc/")
check("./", "")


//  Dot-Dot
check("/abc/../def", "/def")
check("../abc", "abc")
check("abc/..", "")
check("/abc/..", "/")
check("/..", "")
check("../", "")
check("/../", "")


//  Trailing slash forms
check("abc/", "abc/")
check("/abc/", "/abc/")
check("////", "/")


