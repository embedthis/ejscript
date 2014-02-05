/*
    big.tst -- Test downloading file bigger than the server pipeline buffers
 */

const HTTP = App.config.uris.http

var http: Http = new Http

http.get(HTTP + "/big.txt")
assert(http.status == 200)
assert(http.response == Path("web/big.txt").readString())
http.close()
