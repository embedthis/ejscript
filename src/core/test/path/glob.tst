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
assert(dir.glob('*glob.tst') == 'glob.tst')

//  Directory matching
assert(Path('..').glob('*/*.tst').length > 200)
assert(Path('..').glob('http/w*/n*').length == 2)
assert(Path('../../..').glob('core/*.json') == '../../../core/package.json')

//  Double star (should not contain directories)
assert(Path('..').glob("**/*.tst").length > 200)
assert(!Path('..').glob("**/*.tst").toString().contains('"../app"'))
assert(Path('../..').glob("**/*.tst").length > 200)
assert(!Path('../..').glob("**/*.tst").toString().contains('"../app"'))

//  Start with a base path 
assert(Path('../../../core').glob('*.json') == '../../../core/package.json')

//  relative
assert(Path('..').glob('http/w*/n*', {relative: true}).sort() == 'http/web/numbers.html,http/web/numbers.txt')

//  No directories
assert(!Path('..').glob('*', {files: true}).sort().toString().contains('app,'))
assert(Path('..').glob('*', {files: false}).sort().toString().contains('app,'))

//  descend
assert(Path('..').glob('*.tst') == '')
assert(Path('..').glob('**/*.tst').length > 200)

//  exclude
assert(!dir.glob('*', {exclude: /file/}).toString().contains("file"))

//  include
assert(dir.glob('*', {include: /regress/}) == 'regress')
assert(dir.glob('*', {include: /file/}).toString().contains("file"))

//  Depth first
let files = Path('..').glob('**/')
assert(files.indexOf(Path('../path')) < files.indexOf(Path('../path/regress')))
files = Path('..').glob('**/', {depthFirst: true})
assert(files.indexOf(Path('../path')) > files.indexOf(Path('../path/regress')))

//  Full path

assert(Path('/anything').glob(App.dir.toString() + '/*').toString().contains('path-01.tst'))

//  files
if (FileSystem('.').separators[0] == '\\') {
    assert(Path('..').glob('*app*/*', {files: true, descend: true}).sort() == '..\\app\\09100-app.tst,..\\app\\io.tst')
} else {
    assert(Path('..').glob('*app*/*', {files: true, descend: true}).sort() == '../app/09100-app.tst,../app/io.tst')
}

/*
    hidden
    missing - throws
 */
