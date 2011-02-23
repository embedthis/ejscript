/*
    helpers.tst
 */

let ejs = App.exePath

if (!Path("/bin/echo").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Static helpers
    assert(Cmd.sh("pwd") == App.dir)
    assert(Cmd.run("/bin/pwd").trim() == App.dir)
    let response = Cmd.sh(["args", "a", "b", "c"])
    assert(deserialize(response) == "./args,a,b,c")
}
