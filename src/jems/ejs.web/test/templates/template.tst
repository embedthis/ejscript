/*
    template.tst -- Test template expansion
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

//  Test a basic get first
http.get(HTTP + "/template.ejs")
assert(http.status == 200)

var expected = "Hello World
i is 7 less than 8


    Line 0

    Line 1

    Line 2


Some included text
dynamic write from include

"
assert(http.response == expected)
http.close()
