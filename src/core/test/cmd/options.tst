/*
    options.tst
 */

let ejs = App.exePath.portable

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Set child directory
    cmd = new Cmd
    let parent = App.dir.parent
    cmd.start("pwd", {dir: parent})
    if (Config.OS != "WIN") {
        //  Windows with CYGWIN paths can't handle this
        assert(parent.same(cmd.response.trim()))
    }

    //  Set environment
    cmd = new Cmd
    cmd.env = { "WEATHER": "sunny", "PATH": "/bin:/usr/bin" }
    cmd.start("sh -c env")
    assert(cmd.response.contains("WEATHER=sunny"))
    assert(cmd.env.WEATHER == "sunny")
}
