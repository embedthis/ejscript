/*
    events.tst - Test I/O events: readable, writable, complete, error
 */

let ejs = Cmd.locate('ejs')

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    let out = new ByteArray
    for (i in 1000) {
        out.write(i + ": aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
    }
    let count = out.length

    cmd = new Cmd
    cmd.start("cat", {detach: true})
    let input = new ByteArray
    cmd.on("readable", function(event, cmd) {
        cmd.read(input, -1)
    })
    cmd.on("writable", function(event, cmd) {
        let len = cmd.write(out)
        out.readPosition += len
        if (out.length == 0) {
            cmd.finalize()
        }
    })
    let gotEvent
    cmd.on("complete", function(event, cmd) {
        gotEvent = event
    })
    cmd.wait()
    assert(input.length > 0)
    assert(input.length == count)
    assert(cmd.status == 0)
    assert(gotEvent == "complete")

    //  Error event
    cmd = Cmd("ls /asdf", {detach: true})
    let gotEvent = false
    cmd.on("error", function(event, c) {
        gotEvent = event
    })
    cmd.finalize()
    cmd.wait()
    assert(gotEvent == "error")
}
