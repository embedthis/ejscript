/*
    Test Path.tree()
 */

/*
    Setup
 */
const out = Path('out')
out.removeAll()

let path = Path('files')
let bdat = Path('b.dat')
let sep = path.separator

//  from, to 
path.tree(Path('a.dat'), bdat)
assert(bdat.exists)
bdat.remove()
assert(!bdat.exists)

//  from, to with String args
path.tree('a.dat', bdat.name)
assert(bdat.exists)
bdat.remove()
assert(!bdat.exists)

//  Object form
path.tree({ from: 'a.dat', to: bdat})
assert(bdat.exists)
bdat.remove()

//  Array of sources
path.tree({ from: ['a.dat'], to: 'b.dat'})
assert(bdat.exists)
bdat.remove()

//  Array of objects
path.tree([{ from: 'a.dat', to: 'b.dat'}])
assert(bdat.exists)
bdat.remove()

//  Array of files to a directory
assert(!out.exists)
path.tree({ from: ['a.dat', 'b.dat'], to: out})
assert(out.exists)
assert(out.isDir)
assert(Path('out/a.dat').exists)
assert(Path('out/b.dat').exists)
out.removeAll()

//  Negation
out.removeAll()
path.tree({ from: [ '*.dat', '!a.dat' ], to: 'out/'})
assert(out.exists)
assert(out.isDir)
assert(!Path('out/a.dat').exists)
assert(Path('out/b.dat').exists)
out.removeAll()

//  Trailing slash
path.tree('a.dat', 'out/')
assert(out.isDir)
assert(Path('out/a.dat').exists)
out.removeAll()

//  Multiple inputs
path.tree('*.dat', 'out')        //  Multiples without dir
assert(out.isDir)

//  Relative to a directory
path.tree('?.dat', 'out/', {relative: path.parent})
assert(Path('out/files/a.dat').exists)
assert(Path('out/files/b.dat').exists)
out.removeAll()

//  Trim components to a directory
path.tree('mid/sub1/*.dat', 'out/', {trim: 2})
assert(Path('out/sub1/leaf1.dat').exists)
assert(Path('out/sub1/leaf2.dat').exists)
out.removeAll()

//  Flatten
path.tree('**.dat', out, {flatten: true})
assert(Path('out/a.dat').exists)
assert(Path('out/leaf1.dat').exists)
assert(Path('out/leaf2.dat').exists)
out.removeAll()

//  Missing pattern
let count = path.tree('xa.dat', 'out/', {exceptions: false})
assert(count == 0)
assert(!out.isDir)

//  Extension mapping
path.tree('*.dat', out, {extension: 'min.dat'})
assert(Path('out/a.min.dat').exists)
assert(Path('out/b.min.dat').exists)
assert(!Path('out/a.dat').exists)
out.removeAll()

//  Callbacks
let called = 0
path.tree('*.dat', out, {post: function(src, dest, options) {
    called++
}})
assert(called > 2)
out.removeAll()

//  Result files
let files = []
path.tree('*.dat', out, {files: files})
assert(files.length > 2)
out.removeAll()

let instructions = []
path.tree('*.dat', 'out/', {instructions: instructions})
assert(instructions.length > 2)
out.removeAll()

//  Convert "mid/" to "mid/**"
path.tree('mid/', 'out/')
assert(Path('out/mid/middle.dat').exists)
assert(Path('out/mid/sub1/leaf1.dat').exists)
out.removeAll()
