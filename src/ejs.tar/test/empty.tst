/*
    empty.tst - Test tar creation with empty file
 */

require ejs.tar

let name = Path('').temp().joinExt('tar')
tar = new Tar(name)

tar.create('empty.dat')
assert(name.exists)
assert(name.size > 0)
assert(tar.list() == 'empty.dat')
name.remove()
