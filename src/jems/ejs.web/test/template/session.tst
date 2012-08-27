/*
    session.tst -- Test session creation and management
 */

const HTTP = App.config.uris.http

var http: Http = new Http

//  Test a basic get first. Should not create a session
http.get(HTTP + "/session.ejs")
assert(http.status == 200)
assert(http.response.contains("Session ID: null"))
http.close()


//  Test POST to the form which will auto-create a sesion
http.form(HTTP + "/session.ejs", { submit: "OK" })
assert(http.status == 200)
let result = http.response
assert(http.response.contains("Session ID: SET to "))
let firstSession = http.response.match(/"(.*)"/)[1]
let cookie = http.header("Set-Cookie").match(/(-ejs-session-=[^;]*);/)[1]
assert(firstSession == cookie.match(/-ejs-session-=(.*)/)[1])
assert(firstSession != "")
http.close()


//  Re-post and check using the same session
http.setHeader("Cookie", cookie)
http.form(HTTP + "/session.ejs", { submit: "OK" })
let secondSession = http.response.match(/"(.*)"/)[1]
assert(http.status == 200)
assert(firstSession == secondSession)
http.close()


//  Test click on new Session
http.header("Cookie", cookie)
http.form(HTTP + "/session.ejs", { submit: "New Session" })
let thirdSession = http.response.match(/"(.*)"/)[1]
assert(thirdSession != firstSession)
assert(http.status == 200)
http.close()
