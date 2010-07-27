require ejs.web

exports.app = function(r: Request) {

    observe("error", function(event) {
        print("GOT ERROR EVENT " + errorMessage)
    })
    observe("close", function(event) {
        print("GOT CLOSE EVENT")
    })
    observe("readable", function(event) {
        buf = new ByteArray
        if (read(buf)) {
            print('READ returned ' + buf)
        } else {
            print("EOF")
            timer.stop()
            finalize()
        } 
    })

    let timer = Timer(5000 * 1000, function() {
print("TIMER WRITING")
try {
        write("SOME MORE " + new Date + " \r\n")
        print("BEFORE FLUSH")
        flush()
        print('AFTER FLUSH')
} catch {
    timer.stop()
    print("CATCH")
}
    })
    timer.repeat = true
    timer.start()
}
