/*
    Test basic creation
 */

require ejs.tar

let name = Path('').temp().joinExt('tar')
tar = new Tar(name)
assert(tar.name == name)
name.remove()
