var http = new Http

/*
let URL = "https://secure.sbc.com/sbc/sbc_login.adp"
http.form(URL, {
    roamRelm: "attwifi.com",
    name: "michael.obrien",
    password: "saturn",
    aupAgree: "checked",
    AUPConfirmed: "1",
})

print("CODE " + http.code)
print("RESPONSE " + http.response)
*/

http.get("www.google.com/index.html")
data = http.response
print("GOOGLE found " + data.contains("<title>Google</title>"))
