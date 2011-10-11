/*
    write.tst
 */

let ejs = App.exePath.portable

if (!Path("/bin").exists) {
    test.skip("Only run on unix systems")
} else {

    //  Test write and finalize
    cmd = new Cmd
    cmd.start("/bin/cat", {detach: true})
    cmd.write("Hello World")
    cmd.finalize()
    assert(cmd.response == "Hello World")
    assert(cmd.status == 0)

    //  Test various write data types
    cmd = new Cmd
    cmd.start("/bin/cat", {detach: true})
    cmd.write(1, 2, 3, " ")
    cmd.write(true)
    cmd.write(" ")
    cmd.write(new Date)
    cmd.finalize()
print(new Date)
    assert(cmd.response.contains("123 true "))
    assert(cmd.response.contains("GMT"))
    assert(cmd.status == 0)
}
