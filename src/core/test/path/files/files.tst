/*
    Path.files()
 */

// Parameter formats
assert(Path().files('**.dat').length > 5)

let numdat = Path().files('**.dat').length
assert(Path().files('**.dat').length == numdat)
assert(Path().files(['**.dat']).length == numdat)

assert(Path().files({ files: '**.dat' }).length == numdat)
assert(Path().files({ from: '**.dat' }).length == numdat)
assert(Path().files([{ from: '**.dat' }]).length == numdat)

//  Negate 
let count = Path().files(['**', '!**.dat', '!**.dat']).length
assert(count > 0 && count < numdat)
let count = Path().files(['**.dat', '!**.dat', '!**.dat'], {noneg: true}).length
assert(count == numdat)

//  No match returns zero
assert(Path().files('**abc').length == 0)

//  Missing - throws exceptions if pattern does not match
let caught 
try {
    Path().files('**abc', {missing: undefined})
} catch {
    caught = true
}
assert(caught)

//  Missing with replacement
assert(Path().files('**abc', {missing: 'xxxx'}) == 'xxxx')
assert(Path().files('**abc', {missing: null}) == '')

//  Relative when path is relative
assert(Path().files().every(function(e) !e.isAbsolute))

//  Absolute when path is absolute
let files = App.dir.files()
assert(files.every(function(e) e.isAbsolute))

//  Must return absolute paths
let files = Path().files(App.dir.name + '/**')
assert(files.every(function(e) e.isAbsolute))

//  Still absolute as relative means pattern relative to path. Pattern is absolute.
let files = Path().files(App.dir.name + '/**', {relative: true})
assert(files.every(function(e) e.isAbsolute))
assert(Path().files('mid/*.dat', {relative: true})[0].portable == 'mid/middle.dat')

assert(Path('../files').files('mid/*.dat') == '../files/mid/middle.dat')

//  Exclude
assert(Path().files('**.dat', {exclude: /.dat/}).length == 0)
assert(Path().files('**.dat', {exclude: /.xx.dat/}).length == numdat)
assert(!Path().files('*', {exclude: /\/$/}).toString().contains(',mid,'))
assert(!Path().files('*', {exclude: /file/}).toString().contains('file'))

//  Include 
assert(Path().files('**.dat', {include: /.dat/}).length == numdat)
assert(Path().files('**.dat', {include: /.xx.dat/}).length == 0)
assert(Path().files('*', {include: /\/$/}).toString() == 'mid')
assert(Path().files('*', {include: 'directories'}).toString() == 'mid')
assert(Path().files('*', {include: /file.dat/}).sort() == 'file.dat,pre-file.dat')
assert(Path().files('*', {include: /file/}).toString().contains('file.dat'))

//  Filter in all files and count
let count = 0
let files = Path().files('**.dat', {
    filter: function(file, base, options) {
        count++
        return true
}})
assert(count == numdat)
assert(files.length == numdat)

//  Filter out files
let files = Path().files('**.dat', {
    filter: function(file, base, options) false
})
assert(files.length == 0)

//  depthFirst: Directories before sub-directory contents
let files = Path().files('m**')
assert(files[0] == 'mid')
let files = Path().files('m**', {depthFirst: false})
assert(files[0] == 'mid')

//  depthFirst: Directories last
let files = Path().files('m**', {depthFirst: true})
assert(files.pop() == 'mid')

//  Expand
let files = Path().files('${what}/**.dat', {
    expand: { what: '.' }
})
assert(files.length == numdat)

//  Hidden
assert(Path().files('*').find(function(e) e.name == '.hidden') == null)
assert(Path().files('*', {hidden: true}).find(function(e) e.name == '.hidden') == '.hidden')

//  Special cases
//  Trailing / on pattern
assert(Path().files('mid/').length == 1)
assert(Path().files('mid/', {contents: true}).length > 1)

