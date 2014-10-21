/*
    Test wild card patterns via files()
 */

/*
    Convert an array of Paths to a string of filenames in portable format
 */
function portable(array: Array): String array.transform(function(path: Path) path.portable.name)

let d = Path('data')

//  Wild card patterns
assert(portable(d.files('file.dat')) == 'data/file.dat')
assert(portable(d.files('*file.dat').sort()) == 'data/file.dat,data/pre-file.dat')
assert(portable(d.files('*le.dat').sort()) == 'data/file.dat,data/pre-file.dat')
assert(portable(d.files('*file.dat*').sort()) == 'data/file.dat,data/pre-file.dat')
assert(portable(d.files('*file.d*').sort()) == 'data/file.dat,data/pre-file.dat')
assert(portable(d.files('file*.dat').sort()) == 'data/file-post.dat,data/file.dat')
assert(portable(d.files('fi*.dat').sort()) == 'data/file-post.dat,data/file.dat')
assert(portable(d.files('file.*dat')) == 'data/file.dat')
assert(portable(d.files('file.*at')) == 'data/file.dat')
assert(portable(d.files('file.???').sort()) == 'data/file.dat')
assert(portable(d.files('f?l?.dat')) == 'data/file.dat')

//  Must not match
assert(d.files('zle.dat') == '')
assert(d.files('file.datx') == '')
assert(d.files('file*.t') == '')

//  Directory matching
assert(d.files('*').length > 4)
portable(d.files('*'))
assert(portable(d.files('*')).contains('data/mid'))
assert(!portable(d.files('*')).contains('data/mid/middle.dat'))
assert(!portable(d.files('*')).contains('data/mid/sub1'))
assert(!portable(d.files('*')).contains('data/mid/sub2'))

//  Directory wildcards
assert(d.files('mid/su*/lea*').length == 4)

//  Trailing slash matches directory contents
assert(d.files('*/').length > 1)

//  Double star 
assert(d.files("**/*.dat").length > 8)
assert(d.files("**").length > 8)
assert(portable(d.files("**")).contains('data/mid'))
assert(portable(d.files("**")).contains('data/file.dat'))

//  Directories only
assert(d.files("**", {include: 'directories'}).length == 3)
assert(portable(d.files("**", {include: 'directories'})).contains('data/mid'))
assert(!portable(d.files("**", {include: 'directories'})).contains('data/file.dat'))

//  Double star with suffix
assert(d.files("**.dat").length > 8)

//  Start with a base path 
assert(Path('../path').files('data/mid/*.dat') == '../path/data/mid/middle.dat')
