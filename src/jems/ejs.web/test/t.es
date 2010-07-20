/*
    Http upload tests
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)

var http: Http = new Http

http.upload(HTTP + "/upload.ejs", { myfile: "file.dat"} )
assert(http.response.contains('"clientFilename": "file.dat"'))
http.finalize()
print(http.status)
http.close()
