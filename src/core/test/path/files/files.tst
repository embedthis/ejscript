/*
    Path.files
 */

let d = Path('.')

//  Wild card patterns
assert(d.files('file.dat') == 'file.dat')
assert(d.files('*file.dat').sort() == 'file.dat,pre-file.dat')
assert(d.files('*le.dat').sort() == 'file.dat,pre-file.dat')
assert(d.files('*file.dat*').sort() == 'file.dat,pre-file.dat')
assert(d.files('*file.d*').sort() == 'file.dat,pre-file.dat')
assert(d.files('file*.dat').sort() == 'file-post.dat,file.dat')
assert(d.files('fi*.dat').sort() == 'file-post.dat,file.dat')
assert(d.files('file.*dat') == 'file.dat')
assert(d.files('file.*at') == 'file.dat')
assert(d.files('file.???').sort() == 'file.dat')
assert(d.files('f?l?.dat') == 'file.dat')

//  Must not match
assert(d.files('zle.dat') == '')
assert(d.files('file.datx') == '')
assert(d.files('file*.t') == '')

//  Directory matching
assert(d.files('*').length > 4)
assert(d.files('*').contains(Path('mid')))
assert(!d.files('*').contains(Path('mid/middle.dat')))
assert(!d.files('*').contains(Path('mid/sub1')))
assert(!d.files('*').contains(Path('mid/sub2')))

//  Directory wildcards
assert(d.files('mid/su*/lea*').length == 4)

//  Trailing slash matches only directories
assert(d.files('*/') == 'mid')

//  Double star 
assert(d.files("**/*.dat").length == 8)
assert(d.files("**").length > 8)
assert(d.files("**").contains(Path('mid')))
assert(d.files("**").contains(Path('file.dat')))

//  Directories only
assert(d.files("**/").length == 3)
assert(d.files("**/").contains(Path('mid')))
assert(!d.files("**/").contains(Path('file.dat')))

//  Double star with suffix
assert(d.files("**.dat").length == 8)

//  Start with a base path 
assert(Path('../files').files('mid/*.dat') == '../files/mid/middle.dat')

//  relative
assert(d.files('mid/*.dat', {relative: true})[0].portable == 'mid/middle.dat')

//  exclude
assert(!d.files('*', {exclude: /\/$/}).contains(Path('mid')))
assert(!d.files('*', {exclude: /file/}).toString().contains("file"))

//  include
assert(d.files('*', {include: /\/$/}) == 'mid')
assert(d.files('*', {include: /file.dat/}).sort() == 'file.dat,pre-file.dat')
assert(d.files('*', {include: /file/}).contains(Path("file.dat")))

//  descend
assert(d.files('*.tst') == 'files.tst')
assert(d.parent.parent.files('**/*.tst').length > 200)

//  Depth first
let files = d.files('**/').transform(function(p) p.portable)
assert(files.indexOf(Path('mid')) < files.indexOf(Path('mid/sub1')))
files = d.files('**/', {depthFirst: true})
assert(files.indexOf(Path('mid')) > files.indexOf(Path('mid/sub1')))

//  Full path
assert(Path('/anything').files(d.absolute.toString() + '/*').toString().contains(/files.tst/))
