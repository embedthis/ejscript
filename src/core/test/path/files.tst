/*
    Path.files()
 */

/*
    Convert an array of Paths to a string of filenames in portable format
 */
function portable(array: Array): String array.transform(function(path: Path) path.portable.name)

let base = Path('data')

// Parameter formats
assert(base.files('**.dat').length > 5)

let numdat = base.files('**.dat').length
assert(base.files('**.dat').length == numdat)
assert(base.files(['**.dat']).length == numdat)

assert(base.files({ files: '**.dat' }).length == numdat)
assert(base.files({ from: '**.dat' }).length == numdat)
assert(base.files([{ from: '**.dat' }]).length == numdat)

//  Negate 
let count = base.files(['**', '!**.dat', '!**.dat']).length
assert(count > 0 && count < numdat)
let count = base.files(['**.dat', '!**.dat', '!**.dat'], {noneg: true}).length
assert(count == numdat)

//  No match returns zero
assert(base.files('**abc').length == 0)

//  Missing - throws exceptions if pattern does not match
let caught 
try {
    base.files('**abc', {missing: undefined})
} catch {
    caught = true
}
assert(caught)

//  Missing with replacement
assert(base.files('**abc', {missing: 'xxxx'}) == 'xxxx')
assert(base.files('**abc', {missing: null}) == '')

//  Relative when path is relative
assert(base.files().every(function(e) !e.isAbsolute))

//  Absolute when path is absolute
let files = App.dir.files()
assert(files.every(function(e) e.isAbsolute))

//  Must return absolute paths
let files = base.files(App.dir.name + '/**')
assert(files.every(function(e) e.isAbsolute))

//  Relative paths to current directory
let files = base.files(App.dir.name + '/**', {relative: true})
assert(files.every(function(e) !e.isAbsolute))
assert(base.files('mid/*.dat', {relative: true})[0].portable == 'mid/middle.dat')

//  Relative to a path
let files = base.files('**', {relative: '..'})
assert(files.every(function(e) e.startsWith('path/')))

//  Result should include the path itself by default
assert(Path('../path').files('data/mid/*.dat') == '../path/data/mid/middle.dat')

//  Exclude RegExp
assert(base.files('**.dat', {exclude: /.dat/}).length == 0)
assert(base.files('**.dat', {exclude: /.xx.dat/}).length == numdat)
assert(!base.files('*', {exclude: /\/$/}).toString().contains(',mid,'))
assert(!base.files('*', {exclude: /file/}).toString().contains('file'))

//  Exclude string directories
assert(!portable(base.files('*', {exclude: 'directories'})).contains('data/mid'))

//  Exclude function
let count = 0
let files = base.files('**.dat', {
    exclude: function(file, options) {
        count++
        return false
    }})
assert(count == numdat)
assert(files.length == numdat)

//  Include RegExp
assert(base.files('**.dat', {include: /.dat/}).length == numdat)
assert(base.files('**.dat', {include: /.xx.dat/}).length == 0)
assert(portable(base.files('*', {include: /\/$/})).contains('data/mid'))
assert(portable(base.files('*', {include: /file.dat/}).sort()).contains('data/file.dat,data/pre-file.dat'))
assert(base.files('*', {include: /file/}).toString().contains('file.dat'))

//  Include string 'directories'
assert(portable(base.files('*', {include: 'directories'})).contains('data/mid'))

//  Include function
let count = 0
let files = base.files('**.dat', {
    include: function(file, options) {
        count++
        return true
    }})
assert(count == numdat)
assert(files.length == numdat)

//  depthFirst: Directories before sub-directory contents
let files = base.files('m**')
assert(files[0].portable == 'data/mid')
let files = base.files('m**', {depthFirst: false})
assert(files[0].portable == 'data/mid')

//  depthFirst: Directories last
let files = base.files('m**', {depthFirst: true})
assert(files.pop().portable == 'data/mid')

//  Expand by object
let files = base.files('${what}/**.dat', {
    expand: { what: '.' }
})
assert(files.length == numdat)

//  Expand by function
let files = base.files('${what}/**.dat', {
    expand: function(file) {
        if (file.startsWith('${what}')) {
            return file.replace('${what}', '.')
        }
        return file
    }
})
assert(files.length == numdat)

//  Hidden
assert(base.files('*').find(function(e) e.name == '.hidden') == null)
assert(base.files('*', {hidden: true}).find(function(e) e.portable.name == 'data/.hidden').portable == 'data/.hidden')

//  Special cases
//  Trailing / on pattern implies contents
assert(base.files('mid/').length > 1)

//  Regressions - test trailing '/' on directory
assert(App.dir.files('data/mid') == App.dir.join('data/mid'))
assert(App.dir.files('data/mid/').length > 1)
assert(App.dir.files(App.dir.join('data/mid')) == App.dir.join('data/mid'))
assert(App.dir.files(App.dir.join('data/mid/').length > 1))

