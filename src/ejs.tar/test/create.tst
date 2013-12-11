/*
    create.tst - Test tar creation
 */

require ejs.tar

let name = Path('').temp().joinExt('tar')
tar = new Tar(name)

tar.create('short_name.dat')
assert(name.exists)
assert(name.size > 0)
name.remove()
