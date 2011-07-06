/*
    Test blending objects
 */

dest = {
    session : {
        timeout: 4000,
        enable: true
    },
    top: 7,
}

src = {
    top: 8,
    session: {
        big: true,
        timeout: 5000,
    },
}

d = dest.clone()
blend(d, src, {overwrite: false})
assert(d.session.timeout == 4000)
assert(d.top == 7)
assert(d.session.enable == true)
assert(d.session.big == true)

d = dest.clone()
blend(d, src)
assert(d.session.timeout == 5000)
assert(d.top == 8)
assert(d.session.enable == true)
assert(d.session.big == true)
