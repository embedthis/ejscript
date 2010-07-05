/*
    Uri encoding tests
 */

s= "http://[127.0.0.1]:4000:/some $p@th/special+!()*,;#ext?a=b&c=d"

if (this.Uri) {
    assert(Uri.encode(s) == "http://[127.0.0.1]:4000:/some%20$p@th/special+%21%28%29%2A,;#ext?a=b&c=d")
    assert(Uri.encodeComponent(s) == "http%3A%2F%2F%5B127.0.0.1%5D%3A4000%3A%2Fsome+%24p%40th%2Fspecial%2B%21%28%29%2A%2C%3B%23ext%3Fa%3Db%26c%3Dd")
}
assert(encodeURI(s) == "http://%5B127.0.0.1%5D:4000:/some%20$p@th/special+!()*,;#ext?a=b&c=d")
assert(encodeURIComponent(s) == "http%3A%2F%2F%5B127.0.0.1%5D%3A4000%3A%2Fsome%20%24p%40th%2Fspecial%2B!()*%2C%3B%23ext%3Fa%3Db%26c%3Dd")
