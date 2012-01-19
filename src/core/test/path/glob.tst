/*
    Path.glob
 */

let dir = Path('.')

//  Wild card patterns
assert(dir.glob('file.xml*') == 'file.xml')
assert(dir.glob('*.xml') == 'file.xml')
assert(dir.glob('file.*').sort() == 'file.dat,file.xml')
assert(dir.glob('file*').sort() == 'file.dat,file.xml,files.tst')
assert(dir.glob('file.???').sort() == 'file.dat,file.xml')
assert(dir.glob('f*.???').sort() == 'file.dat,file.xml,files.tst')
assert(dir.glob('g?o?.tst') == 'glob.tst')

//  Directory matching
assert(dir.glob('../*/*.tst').length > 200)
assert(dir.glob('../http/w*/n*').length == 2)
assert(dir.glob('../../../core/*.json') == '../../../core/package.json')

//  Double star (should not contain directories)
assert(Path().glob("../**/*.tst").length > 200)
assert(!Path().glob("../**/*.tst").toString().contains('"../app"'))
assert(Path('../..').glob("**.tst").length > 200)
assert(!Path('../..').glob("**.tst").toString().contains('"../app"'))

//  Start with a base path 
assert(Path('../../../core').glob('*.json') == '../../../core/package.json')

//  relative
assert(dir.glob('../http/w*/n*', {relative: true}).sort() == 'numbers.html,numbers.txt')

//  No directories
assert(!dir.glob('../*', {nodirs: true}).sort().toString().contains('app,'))
assert(dir.glob('../*', {nodirs: false}).sort().toString().contains('app,'))

//  descend
assert(Path('..').glob('*.tst') == '')
assert(Path('..').glob('*.tst', {descend: true}).length > 200)

//  exclude
assert(!dir.glob('*', {exclude: /file/}).toString().contains("file"))

//  include
assert(dir.glob('*', {include: /regress/}) == 'regress')
assert(dir.glob('*', {include: /file/}).toString().contains("file"))

//  Depth first
let app = Path('../app').natural
assert(Path('..').glob('app', {descend: true, depthFirst: true}).toString().endsWith(app))
assert(Path('..').glob('app', {descend: true, depthFirst: false}).toString().startsWith(app))

//  nodirs
if (FileSystem('.').separators[0] == '\\') {
    assert(Path('..').glob('app', {nodirs: true, descend: true}).sort() == '..\\app\\09100-app.tst,..\\app\\io.tst')
} else {
    assert(Path('..').glob('app', {nodirs: true, descend: true}).sort() == '../app/09100-app.tst,../app/io.tst')
}

/*
    hidden
    missing - throws
 */
