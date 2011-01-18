/* 
     Test Logger class
 */

//  Verbosity constants
assert(Logger.Off == -1)
assert(Logger.Warn == 1)
assert(Logger.Info == 2)
assert(Logger.Config == 3)
assert(Logger.All == 9)


//  Simple log construction
let buf = new ByteArray
var log = new Logger("top", buf, 5)
assert(log.level == 5)
assert(log.name == "top")
assert(log.outStream == buf)


//  Test output routines
log.debug(4, "--debug--")
assert(buf == "top: 4: --debug--\n")

buf.flush()
log.config("--config--")
assert(buf == "top: CONFIG: --config--\n")

buf.flush()
log.error("--error--")
assert(buf == "top: ERROR: --error--\n")

buf.flush()
log.warn("--warn--")
assert(buf == "top: WARN: --warn--\n")

buf.flush()
log.info("--info--")
assert(buf == "top: INFO: --info--\n")

buf.flush()
log.write("--write--\n")
assert(buf == "--write--\n")


//  Test sub logger
var sub: Logger = new Logger("sub", log, 4)
assert(sub.level == 4)
assert(sub.name == "top.sub")
assert(sub.outStream == log)

buf.flush()
sub.debug(0, "--debug--")
assert(buf == "top.sub: 0: --debug--\n")


//  Test output filtering
sub.filter = function(log: Logger, name: String, level: Number, kind: String, msg: String): Boolean {
    return (name == "top.sub") && level < 2
}
buf.flush()
sub.debug(1, "--msg-a--")
assert(buf == "top.sub: 1: --msg-a--\n")
sub.filter = null

let buf2 = new ByteArray
log.redirect(buf2)
sub.debug(1, "--msg-a--")
assert(buf2 == "top.sub: 1: --msg-a--\n")
log.redirect(buf)


// Redirect to a file at level 2
let tmpfile = Path(".").temp()
log.redirect(tmpfile + ":2")
log.debug(1, "one")
log.debug(2, "two")
log.debug(3, "three")       // Should not be logged
log.close()

let data = tmpfile.readString()
assert(data == "top: 1: one\ntop: 2: two\n")
tmpfile.remove()


// Test match
sub.match = /^top.sub$/
buf = new ByteArray
log.redirect(buf)
sub.debug(0, "--msg-b--")
assert(buf == "top.sub: 0: --msg-b--\n")
sub.match = null

//  Filter out message (with flush called on the logger)
log.flush()
log.match = /^top$/
sub.debug(0, "--msg-c--")
