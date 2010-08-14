
require ejs.web

class MyFilter extends Filter {


}

filter = new Filter(stream)

filter.observe("readable", function(request) {
    assert(this == filter)
    buf = new ByteArray
    filter.write(read(buf))
})

filter.observe("writable", function(request) {
    assert(this == filter)
    buf = new ByteArray
    filter.write(read(buf))
})

filter.observe("close", function(request) {
    assert(this == filter)
})

filter.observe("error", function(request) {
    assert(this == filter)
})
