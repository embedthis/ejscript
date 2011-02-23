/*
    helpers.tst
 */

let ejs = App.exePath

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Static helpers
    assert(App.dir.same(Cmd.sh("pwd")))
    assert(App.dir.same(Cmd.run("/bin/pwd").trim()))
    let response = Cmd.sh([ejs.portable, "./args", "a", "b", "c"])
    assert(deserialize(response) == "./args,a,b,c")
}
