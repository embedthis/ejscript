/*
    Http upload tests
 */

const HTTP = App.config.uris.http

var http: Http = new Http

http.upload(HTTP + "/upload.ejs", { myfile: "file.dat"} )

http.finalize()
assert(http.status == Http.Ok)
assert(http.response.contains('"clientFilename": "file.dat"'))
http.close()
