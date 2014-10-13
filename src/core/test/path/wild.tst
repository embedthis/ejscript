/*
    Test wild card patterns via files()
 */

let d = Path('data')

//  Wild card patterns
assert(d.files('file.dat') == 'data/file.dat')
assert(d.files('*file.dat').sort() == 'data/file.dat,data/pre-file.dat')
assert(d.files('*le.dat').sort() == 'data/file.dat,data/pre-file.dat')
assert(d.files('*file.dat*').sort() == 'data/file.dat,data/pre-file.dat')
assert(d.files('*file.d*').sort() == 'data/file.dat,data/pre-file.dat')
assert(d.files('file*.dat').sort() == 'data/file-post.dat,data/file.dat')
assert(d.files('fi*.dat').sort() == 'data/file-post.dat,data/file.dat')
assert(d.files('file.*dat') == 'data/file.dat')
assert(d.files('file.*at') == 'data/file.dat')
assert(d.files('file.???').sort() == 'data/file.dat')
assert(d.files('f?l?.dat') == 'data/file.dat')

//  Must not match
assert(d.files('zle.dat') == '')
assert(d.files('file.datx') == '')
assert(d.files('file*.t') == '')

//  Directory matching
assert(d.files('*').length > 4)
assert(d.files('*').contains(Path('data/mid')))
assert(!d.files('*').contains(Path('data/mid/middle.dat')))
assert(!d.files('*').contains(Path('data/mid/sub1')))
assert(!d.files('*').contains(Path('data/mid/sub2')))

//  Directory wildcards
assert(d.files('mid/su*/lea*').length == 4)

//  Trailing slash matches directory contents
assert(d.files('*/').length > 1)

//  Double star 
assert(d.files("**/*.dat").length > 8)
assert(d.files("**").length > 8)
assert(d.files("**").contains(Path('data/mid')))
assert(d.files("**").contains(Path('data/file.dat')))

//  Directories only
assert(d.files("**", {include: 'directories'}).length == 3)
assert(d.files("**", {include: 'directories'}).contains(Path('data/mid')))
assert(!d.files("**", {include: 'directories'}).contains(Path('data/file.dat')))

//  Double star with suffix
assert(d.files("**.dat").length > 8)

//  Start with a base path 
assert(Path('../path').files('data/mid/*.dat') == '../path/data/mid/middle.dat')
