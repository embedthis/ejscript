/*
    layout.tst -- Template expansion using a layout
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

//  Test a basic get first
http.get(HTTP + "/template-with-layout.ejs")
assert(http.status == 200)

var expected = '<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head></head>
<body>

Hello World


</body>
</html>
'
assert(http.response == expected)
http.close()
