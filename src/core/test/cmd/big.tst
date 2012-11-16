/*
    big.tst - Write big data with writable events
 */

let ejs = Cmd.locate('ejs')

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Test write and writable events
    data = new ByteArray
    for (i in 1000) {
        data.write(i + ": aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
    }
    let finalized
    let count = data.length
    cmd = new Cmd
    cmd.start("cat", {detach: true})
    cmd.on("writable", function(event, cmd) {
        let len = cmd.write(data)
        data.readPosition += len
        if (data.length == 0) {
            cmd.finalize()
            finalized = true
        }
    })
    let completed = cmd.wait(5000)
    assert(completed)
    assert(finalized)
    if (completed) {
        assert(cmd.response.length == count)
        assert(cmd.status == 0)
    }
}
