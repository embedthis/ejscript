/*
    Serialize Error
 */

try {
    throw new Error("Boom")
} catch (e) {
    s = serialize(e)
    assert(s.contains('"data":null'))
    assert(s.contains('"message":"boom"'))
    assert(s.contains('"timestamp":"'))
    assert(s.contains('"code":null'))
}

