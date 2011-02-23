/*
    big.tst - Write big data with writable events
 */

let ejs = App.exePath

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Test write and writable events
    data = new ByteArray
    for (i in 1000) {
        data.write(i + ": aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
    }
    let count = data.available
    cmd = new Cmd
    cmd.start("/bin/cat", {detach: true})
    cmd.on("writable", function(event, cmd) {
        let len = cmd.write(data)
        data.readPosition += len
        if (data.available == 0) {
            cmd.finalize()
        }
    })
    cmd.wait()
    assert(cmd.response.length == count)
    assert(cmd.status == 0)
}
