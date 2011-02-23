/*
    args.tst
 */

let ejs = App.exePath

if (!Path("/usr/bin/true").exists) {
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

    //  Backquoting to pass in quotes
    cmd = Cmd(ejs + ' args \\"a\\" b c')
    let response = deserialize(cmd.response)
    assert(response.length == 4)
    assert(response == 'args,"a",b,c')
}
