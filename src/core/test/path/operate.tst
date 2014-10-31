/*
    Test Path.operate()
 */

/*
    Setup
 */
const out = Path('out')
out.removeAll()

let path = Path('data')
let bdat = Path('b.dat')
let sep = path.separator

path.operate({ from: 'a.dat', to: bdat })
assert(bdat.exists)
bdat.remove()

//  from, to 
path.operate(Path('a.dat'), bdat)
assert(bdat.exists)
bdat.remove()
assert(!bdat.exists)

//  from, to with String args
path.operate('a.dat', bdat.name)
assert(bdat.exists)
bdat.remove()
assert(!bdat.exists)

//  Object form
path.operate({ from: 'a.dat', to: bdat })
assert(bdat.exists)
bdat.remove()

//  Array of sources
path.operate({ from: ['a.dat'], to: 'b.dat' })
assert(bdat.exists)
bdat.remove()

//  Array of objects
path.operate([{ from: 'a.dat', to: 'b.dat' }])
assert(bdat.exists)
bdat.remove()

//  Array of files to a directory
assert(!out.exists)
path.operate({ from: ['a.dat', 'b.dat'], to: out })
assert(out.exists)
assert(out.isDir)
assert(Path('out/a.dat').exists)
assert(Path('out/b.dat').exists)
out.removeAll()

//  Negation
out.removeAll()
path.operate({ from: [ '*.dat', '!a.dat' ], to: 'out/' })
assert(out.exists)
assert(out.isDir)
assert(!Path('out/a.dat').exists)
assert(Path('out/b.dat').exists)
out.removeAll()

//  Trailing slash
path.operate('a.dat', 'out/')
assert(out.isDir)
assert(Path('out/a.dat').exists)
out.removeAll()

//  Multiple inputs
path.operate('*.dat', 'out')        //  Multiples without dir
assert(out.isDir)

//  Relative to a directory
path.operate('?.dat', 'out/', {relative: path.parent})
assert(Path('out/data/a.dat').exists)
assert(Path('out/data/b.dat').exists)
out.removeAll()

//  Trim components to a directory
path.operate('mid/sub1/*.dat', 'out/', {trim: 1})
assert(Path('out/sub1/leaf1.dat').exists)
assert(Path('out/sub1/leaf2.dat').exists)
out.removeAll()

//  Flatten is default
path.operate('**.dat', out)
assert(Path('out/a.dat').exists)
assert(Path('out/leaf1.dat').exists)
assert(Path('out/leaf2.dat').exists)
out.removeAll()

//  Missing pattern
let count = path.operate('xa.dat', 'out/', {exceptions: false})
assert(count == 0)
assert(!out.isDir)

//  Extension mapping
path.operate('*.dat', out, {extension: 'min.dat'})
assert(Path('out/a.min.dat').exists)
assert(Path('out/b.min.dat').exists)
assert(!Path('out/a.dat').exists)
out.removeAll()

//  Callbacks
let called = 0
path.operate('*.dat', out, {postPerform: function(src, dest, options) {
    called++
}})
assert(called > 2)
out.removeAll()

//  List of files
let list = path.operate('*.dat', out, {operation: 'list'})
assert(list.length > 2)
out.removeAll()

//  Convert "mid/" to "mid/**"
path.operate('mid/', 'out/', {flatten: false})
assert(Path('out/mid/middle.dat').exists)
assert(Path('out/mid/sub1/leaf1.dat').exists)
out.removeAll()

//  MOB - symlink
//  MOB - move
//  MOB - append with header|footer|separator
//  MOB - patch (with expand)
//  MOB - perform, rename, pre, post callbacks


