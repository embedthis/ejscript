/*
    helpers.tst
 */

let ejs = App.exePath

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Cmd.run
    assert(App.dir.same(Cmd.run("/bin/pwd").trim()))

    //  Cmd.sh
    assert(App.dir.same(Cmd.sh("pwd")))
    let response = Cmd.sh([ejs.portable, "./args", "a", "b", "c"])
    assert(deserialize(response) == "./args,a,b,c")

    //  sh quoting
    response = Cmd.sh(['./args', 'a b', 'c'])
    assert(deserialize(response) == "./args,a b,c")

    response = Cmd.sh('./args "a b" c')
    assert(deserialize(response) == "./args,a b,c")
}
