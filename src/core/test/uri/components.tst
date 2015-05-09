/*
    Uri.components
 */

let uri = Uri('http://www.example.com/path/to/resource.gif#fragment?abc=def&ghi=jkl').components
assert(uri.scheme == 'http')
assert(uri.host == 'www.example.com')
assert(uri.port == null)
assert(uri.path == '/path/to/resource.gif')
assert(uri.reference == 'fragment')
assert(uri.query == 'abc=def&ghi=jkl')

//  With port and without path 
u = Uri('www.example.com:7000')
uri = u.components
assert(uri.host = 'www.example.com')
assert(uri.port = 7000)
assert(uri.path = '/')

function show(uri: Uri) {
    print('scheme: ', uri.scheme)
    print('host:   ', uri.host)
    print('port:   ', uri.port)
    print('path:   ', uri.path)
    print('ext:    ', uri.extension)
    print('ref:    ', uri.reference)
    print('query:  ', uri.query)
}

uri = Uri(null)
assert(uri.scheme == null)
assert(uri.host == null)
assert(uri.port == null)
assert(uri.path == null)
assert(uri.extension == null)
assert(uri.reference == null)
assert(uri.query == null)

uri = Uri('');
assert(uri.scheme == null)
assert(uri.host == null)
assert(uri.port == null)
assert(uri.path == null)
assert(uri.reference == null)
assert(uri.extension == null)
assert(uri.query == null)

uri = Uri('http');
assert(uri.scheme == null)
assert(uri.host == null)
assert(uri.port == null)
assert(uri.path == 'http')
assert(uri.reference == null)
assert(uri.extension == null)
assert(uri.query == null)

uri = Uri('https');
assert(uri.scheme == null)
assert(uri.path == 'https')

uri = Uri('http://');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.path == null)

uri = Uri('https://');
assert(uri.scheme == 'https')
assert(uri.port == null)
assert(uri.path == null)

uri = Uri('http://:8080/');
assert(uri.scheme == 'http')
assert(uri.port == 8080)
assert(uri.host == null)
assert(uri.path == '/')


uri = Uri('http://:8080');
assert(uri.scheme == 'http')
assert(uri.port == 8080)
assert(uri.host == null)
assert(uri.path == null)

uri = Uri('http:///');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == null)
assert(uri.path == '/')

uri = Uri('http://localhost');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == 'localhost')
assert(uri.path == null)

uri = Uri('http://localhost/');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == 'localhost')
assert(uri.path == '/')

uri = Uri('http://[::]');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == '::')
assert(uri.path == null)

uri = Uri('http://[::]/');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == '::')
assert(uri.path == '/')

uri = Uri('http://[::]:8080');
assert(uri.scheme == 'http')
assert(uri.port == 8080)
assert(uri.host == '::')
assert(uri.path == null)

uri = Uri('http://[::]:8080/');
assert(uri.scheme == 'http')
assert(uri.port == 8080)
assert(uri.host == '::')
assert(uri.path == '/')

uri = Uri('http://localhost/path');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == 'localhost')
assert(uri.path == '/path')
assert(uri.extension == null)

uri = Uri('http://localhost/path.txt');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == 'localhost')
assert(uri.extension == 'txt')
assert(uri.path == '/path.txt')
assert(uri.extension == 'txt')
assert(uri.reference == null)
assert(uri.query == null)
assert(uri.reference == null)

uri = Uri('http://localhost/path.txt?query');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == 'localhost')
assert(uri.path == '/path.txt')
assert(uri.extension == 'txt')
assert(uri.reference == null)
assert(uri.query == 'query')
assert(uri.extension == 'txt')

uri = Uri('http://localhost/path.txt?query#ref');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == 'localhost')
assert(uri.path == '/path.txt')
assert(uri.extension == 'txt')
assert(uri.reference == null)
assert(uri.query == 'query#ref')

uri = Uri('http://localhost/path.txt#ref?query');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == 'localhost')
assert(uri.path == '/path.txt')
assert(uri.extension == 'txt')
assert(uri.reference == 'ref')
assert(uri.query == 'query')

uri = Uri('http://localhost/path.txt#ref/extra');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == 'localhost')
assert(uri.path == '/path.txt')
assert(uri.extension == 'txt')
assert(uri.reference == 'ref/extra')
assert(uri.query == null)

uri = Uri('http://localhost/path.txt#ref/extra?query');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == 'localhost')
assert(uri.path == '/path.txt')
assert(uri.extension == 'txt')
assert(uri.reference == 'ref/extra')
assert(uri.query == 'query')

uri = Uri(':4100');
assert(uri.scheme == null)
assert(uri.port == 4100)
assert(uri.host == null)
assert(uri.path == null)
assert(uri.extension == null)
assert(uri.reference == null)
assert(uri.query == null)


uri = Uri(':4100/path');
assert(uri.scheme == null)
assert(uri.port == 4100)
assert(uri.host == null)
assert(uri.path == '/path')
assert(uri.extension == null)
assert(uri.reference == null)
assert(uri.query == null)

uri = Uri('http:/');
assert(uri.scheme == null)
assert(uri.port == null)
assert(uri.host == 'http')
assert(uri.path == '/')
assert(uri.extension == null)
assert(uri.reference == null)
assert(uri.query == null)

uri = Uri('http://:/');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == null)
assert(uri.path == '/')
assert(uri.extension == null)
assert(uri.reference == null)
assert(uri.query == null)

uri = Uri('http://:');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == null)
assert(uri.path == null)
assert(uri.extension == null)
assert(uri.reference == null)
assert(uri.query == null)

uri = Uri('http://localhost:');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == 'localhost')
assert(uri.path == null)
assert(uri.extension == null)
assert(uri.reference == null)
assert(uri.query == null)

uri = Uri('http://local#host/');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == 'local')
assert(uri.path == null)
assert(uri.extension == null)
assert(uri.reference == 'host/')
assert(uri.query == null)

uri = Uri('http://local?host/');
assert(uri.scheme == 'http')
assert(uri.port == null)
assert(uri.host == 'local')
assert(uri.path == null)
assert(uri.extension == null)
assert(uri.reference == null)
assert(uri.query == 'host/')

let caught
try {
    uri = Uri('http://local host/');
    assert(true)
} catch {
    caught = true
}
assert(caught)

caught = false
try {
    uri = Uri('http://localhost/long path');
    assert(true)
} catch {
    caught = true
}
assert(caught == true)

//  Fuzz
uri = Uri('://localhost');
assert(uri.scheme == '')
assert(uri.port == null)
assert(uri.host == 'localhost')
assert(uri.path == null)
assert(uri.extension == null)
assert(uri.reference == null)
assert(uri.query == null)

uri = Uri('../path');
assert(uri.scheme == null)
assert(uri.port == null)
assert(uri.host == null)
assert(uri.path == '../path')
assert(uri.extension == null)
assert(uri.reference == null)
assert(uri.query == null)
