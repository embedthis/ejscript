/*
    helpers.tst
 */

let ejs = Cmd.locate('ejs')

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Cmd.run
    if (Config.OS != "WIN") {
        //  Windows with cygwin paths can't handle this
        assert(App.dir.same(Cmd.run("pwd").trim()))
        //  Cmd.sh
        assert(App.dir.same(Cmd.sh("pwd")))
    }

    let response = Cmd.sh([ejs, "./args", "a", "b", "c"])
    assert(deserialize(response) == "./args,a,b,c")

    //  sh quoting
    response = Cmd.sh([ejs, 'args', 'a b', 'c'])
    assert(deserialize(response) == "args,a b,c")

    /*
        WARNING: If starting a program compiled with Cygwin, Cygwin has a bug where embedded quotes are parsed
        incorrectly by the Cygwin crt runtime startup. If an arg starts with a drive spec, embedded backquoted 
        quotes will be stripped and the backquote will be passed in. Windows crt runtime handles this correctly.
        This happens because the entire command passed to Cmd.sh is wrapped with /bin/sh -c "command".
        For example:  ./args "c:/path \"a b\"
            Cygwin will parse as  argv[1] == c:/path \a \b
            Windows will parse as argv[1] == c:/path "a b"
        NOTE: so this command will fail as the ejs path below has an embedded drive spec
            response = Cmd.sh(ejs + ' ./args "a b" c')
        FIX: use single quotes or don't put a drive spec in the start of the arg.
     */
    response = Cmd.sh('ejs ./args "a b" c')
    assert(deserialize(response) == "./args,a b,c")

    response = Cmd.sh(ejs + " ./args 'a b' c")
    assert(deserialize(response) == "./args,a b,c")
}
