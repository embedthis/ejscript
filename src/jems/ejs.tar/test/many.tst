/*
    many.tst - archive many files
 */

require ejs.tar

let name = Path('').temp().joinExt('tar')
let name = Path('a.tar')
tar = new Tar(name)
tar.create(Path('.').glob('**.dat'))
assert(tar.list().length == 3)
// name.remove()
