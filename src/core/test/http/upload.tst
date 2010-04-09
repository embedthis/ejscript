/*
    Http upload tests
 */

const HTTP = (global.session && session["http"]) || ":6700"

var http: Http = new Http

http.upload(HTTP + "/upload.ejs", { myfile: "file.dat"} )
assert(http.response.contains('"clientFilename": "file.dat"'))
http.close()
