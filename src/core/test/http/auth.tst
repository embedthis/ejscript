/*
    Authentication tests
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)

/*
    Basic/Digest auth is not supported in Ejscript. Use Appweb for this functionality

    var http: Http = new Http
    http.get(HTTP + "/basic/basic.html")
    assert(http.status == 401)

    http = new Http
    http.setCredentials("joshua", "pass1")
    http.get(HTTP + "/basic/basic.html")
    assert(http.status == 200)
    http.close()
*/
