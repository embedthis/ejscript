/*
    Path.glob
 */

let d = Path('.')

//  Wild card patterns
assert(d.glob('file.dat') == 'file.dat')
assert(d.glob('*file.dat').sort() == 'file.dat,pre-file.dat')
assert(d.glob('*le.dat').sort() == 'file.dat,pre-file.dat')
assert(d.glob('*file.dat*').sort() == 'file.dat,pre-file.dat')
assert(d.glob('*file.d*').sort() == 'file.dat,pre-file.dat')
assert(d.glob('file*.dat').sort() == 'file-post.dat,file.dat')
assert(d.glob('fi*.dat').sort() == 'file-post.dat,file.dat')
assert(d.glob('file.*dat') == 'file.dat')
assert(d.glob('file.*at') == 'file.dat')
assert(d.glob('file.???').sort() == 'file.dat')
assert(d.glob('f?l?.dat') == 'file.dat')

//  Must not match
assert(d.glob('zle.dat') == '')
assert(d.glob('file.datx') == '')
assert(d.glob('file*.t') == '')

//  Directory matching
assert(d.glob('*').length > 4)
assert(d.glob('*').contains(Path('mid')))
assert(!d.glob('*').contains(Path('mid/middle.dat')))
assert(!d.glob('*').contains(Path('mid/sub1')))
assert(!d.glob('*').contains(Path('mid/sub2')))

//  Directory wildcards
assert(d.glob('mid/su*/lea*').length == 4)

//  Trailing slash matches only directories
assert(d.glob('*/') == 'mid')

//  Double star 
assert(d.glob("**/*.dat").length == 8)
assert(d.glob("**").length > 8)
assert(d.glob("**").contains(Path('mid')))
assert(d.glob("**").contains(Path('file.dat')))

//  Directories only
assert(d.glob("**/").length == 3)
assert(d.glob("**/").contains(Path('mid')))
assert(!d.glob("**/").contains(Path('file.dat')))

//  Double star with suffix
assert(d.glob("**.dat").length == 8)

//  Start with a base path 
assert(Path('../glob').glob('mid/*.dat') == '../glob/mid/middle.dat')

//  relative
assert(d.glob('mid/*.dat', {relative: true}).sort() == 'mid/middle.dat')

//  exclude
assert(!d.glob('*', {exclude: /\/$/}).contains(Path('mid')))
assert(!d.glob('*', {exclude: /file/}).toString().contains("file"))

//  include
assert(d.glob('*', {include: /\/$/}) == 'mid')
assert(d.glob('*', {include: /file.dat/}) == 'file.dat,pre-file.dat')
assert(d.glob('*', {include: /file/}).contains(Path("file.dat")))

//  descend
assert(d.glob('*.tst') == 'glob.tst')
assert(d.parent.parent.glob('**/*.tst').length > 200)

//  Depth first
let files = d.glob('**/')
assert(files.indexOf(Path('mid')) < files.indexOf(Path('mid/sub1')))
files = d.glob('**/', {depthFirst: true})
assert(files.indexOf(Path('mid')) > files.indexOf(Path('mid/sub1')))

//  Full path
assert(Path('/anything').glob(d.absolute.toString() + '/*').toString().contains(/glob.tst/))
