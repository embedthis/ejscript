/*
    unix.tst

    Usage:
        cmd = Cmd("command line with quoted args")
        cmd.env = { "PATH": "/usr/bin" }
        cmd.on("readable", function ()) {})
        cmd.timeout = 1000

        cmd.start("command line with quoted args", {detach: true})
        cmd.start()
        cmd.wait()

        cmd.write(buf)
        cmd.finalize()
        cmd.read(buf)
        data = cmd.readString()
        status = cmd.status
        cmd.error.read(buf)
        print(cmd.response)

- MOB - is newline a command terminator
        cmd('
            ls
            date
        ')

- MOB - backquote for shell?
`        pwd ; ls .....
 */

let ejs = App.exePath

if (Config.OS == "WIN") {
    test.skip("Only run on unix systems")
} else {

    //  Simple command
    cmd = Cmd("/usr/bin/true")
    assert(cmd.status == 0)

    //  Command with failing status
    cmd = Cmd("/usr/bin/false")
    assert(cmd.status == 1)
    
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

    //  Read response as a string
    cmd = Cmd("/bin/echo Hello World")
    assert(cmd.readString().trim() == "Hello World")

    //  Can also read via the response property (multiple times)
    cmd = Cmd("/bin/echo Hello World")
    assert(cmd.response.trim() == "Hello World")
    assert(cmd.response.trim() == "Hello World")

    //  read() into a byte array
    cmd = Cmd("/bin/echo Hello World")
    ba = new ByteArray
    assert(cmd.read(ba) >= 11)
    assert(ba.toString().trim() == "Hello World")

    //  readLines()
    cmd = Cmd("/bin/ls -1 ..")
    assert(cmd.readLines().length > 30)

    //  Commands block by default
    let mark = new Date
    cmd = Cmd("/bin/sleep 2")
    assert(mark.elapsed > 1500)

    //  Detached command
    let mark = new Date
    cmd = Cmd("/bin/sleep 5", {detach: true})
    assert(mark.elapsed < 5000)
    cmd.close()

    //  Kill and pid
    cmd = new Cmd
    cmd.start("/bin/sleep 60", {detach: true})
    assert(cmd.pid != 0)
    Cmd.kill(cmd.pid)


    //  Set child directory
    cmd = new Cmd
    let parent = App.dir.parent
    cmd.start("/bin/pwd", {dir: parent})
    assert(cmd.response.trim() == parent)

    //  Static helpers
    assert(Cmd.sh("pwd") == App.dir)
    assert(Cmd.run("/bin/pwd").trim() == App.dir)
    assert(sh("pwd") == App.dir)
    let response = sh(["args", "a", "b", "c"])
    assert(deserialize(response) == "./args,a,b,c")


    /* Async
    cmd = Cmd("/bin/echo Hello World")
    print("HERE")
    cmd.on("readable", function(event, c) {
        dump("EVENT", event)
        print("CMD", typeOf(c))
        let data = new ByteArray
        print("GOT " + cmd.read(data))
        print(data)
    })
    App.run()
    */


    /*
    let cmd = new Cmd

    cmd.env = { "PATH": ".:/usr/bin" }
    cmd.timeout = 1000

    //  Test pipes and other shell features

    //  Throw exceptions
    try {
        cmd = Cmd("/bin/sh -c 'echo Hello World >&2'", { exception: true })
    } catch (e) {
        assert(e == 'Hello World')
    }

    cmd = Cmd("/bin/sleep 9999", { detach: true })
    print(cmd.pid)
    cmd.kill(cmd.pid)
    cmd.kill(0, 9)

    */
}
