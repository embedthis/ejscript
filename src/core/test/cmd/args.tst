/*
    args.tst
 */

let ejs = App.exePath.portable

//  MOB -- should run on windows too!
if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Command args
    cmd = Cmd(ejs + " args a b c")
    let response = deserialize(cmd.response)
    assert(response.length == 4)
    assert(response == "args,a,b,c")

    //  Command args as an array
    cmd = Cmd([ejs, "args", "a", "b", "c"])
    let response = deserialize(cmd.response)
    assert(response.length == 4)
    assert(response == "args,a,b,c")

    //  Quoted args
    cmd = Cmd(ejs + ' args "a b" c')
    let response = deserialize(cmd.response)
    assert(response.length == 3)
    assert(response == "args,a b,c")

    //  Single quoted args
    cmd = Cmd(ejs + " args 'a b' c")
    let response = deserialize(cmd.response)
    assert(response == "args,a b,c")
    assert(response.length == 3)

    //  Backquoting to pass in quotes. Won't work on windows as windows command exec strips quotes
    if (Config.OS != "WIN") {
        cmd = Cmd(ejs + ' args \\"a\\" b c')
        let response = deserialize(cmd.response)
        assert(response.length == 4)
        assert(response == 'args,"a",b,c')
    }
}
