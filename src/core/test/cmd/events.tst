/*
    events.tst - Test I/O events: readable, writable, complete, error
 */

let ejs = App.exePath

if (!Path("/bin/echo").exists) {
    test.skip("Only run on unix systems")
} else {

    data = new ByteArray
    for (i in 1000) {
        data.write(i + ": aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
    }
    let count = data.available

    cmd = new Cmd
    cmd.start("/bin/cat", {detach: true})
    let response = new ByteArray
    cmd.on("readable", function(event, cmd) {
        cmd.read(response, -1)
    })
    cmd.on("writable", function(event, cmd) {
        let len = cmd.write(data)
        data.readPosition += len
        if (data.available == 0) {
            cmd.finalize()
        }
    })
    let gotEvent
    cmd.on("complete", function(event, cmd) {
        gotEvent = event
    })
    cmd.wait()
    assert(response.available == count)
    assert(cmd.status == 0)
    assert(gotEvent == "complete")


    //  Error event
    cmd = Cmd("/bin/ls /asdf", {detach: true})
    let gotEvent = false
    cmd.on("error", function(event, c) {
        gotEvent = event
    })
    cmd.wait()
    assert(gotEvent == "error")
}
}
