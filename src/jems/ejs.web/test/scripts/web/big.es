/*
    Big output
 */

exports.app = function() {
    setHeader("Content-Type", "text/plain")
    for (i in 1000) {
        write(" Line: %05d %s".format(i, "aaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccccddddddd<br/>\r\n"))
    } 
    finalize()
}
