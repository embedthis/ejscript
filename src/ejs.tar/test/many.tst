/*
    many.tst - archive many files
 */

require ejs.tar

let name = Path('').temp().joinExt('tar')
tar = new Tar(name)
tar.create(Path('.').files('**.dat'))
assert(tar.list().length == 2)
// name.remove()
