
require ejs.web

class MyFilter extends Filter {


}

filter = new Filter(stream)

filter.on("readable", function(request) {
    assert(this == filter)
    buf = new ByteArray
    filter.write(read(buf))
})

filter.on("writable", function(request) {
    assert(this == filter)
    buf = new ByteArray
    filter.write(read(buf))
})

filter.on("close", function(request) {
    assert(this == filter)
})

filter.on("error", function(request) {
    assert(this == filter)
})
