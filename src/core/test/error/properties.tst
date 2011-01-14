/*
    Test Error Properties
 */

try {
    throw new Error("Boom")

} catch (e) {
    assert(e.message == "Boom")
    assert(e.filename == "properties.tst")
    assert(e.lineno is Number)
    assert(e.timestamp is Date)
    assert(e.data == null)
    assert(e.code == null)
    assert(e.stack is Array)
    assert(e.stack[0].filename is Path)
    assert(e.stack[0].filename.basename == "properties.tst")
    assert(e.stack[0].lineno is Number)
    assert(e.stack[0].func == "default")
    assert(e.formatStack() is String)
}

