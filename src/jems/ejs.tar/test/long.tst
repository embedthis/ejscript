/*
    Test long names
 */

require ejs.tar

let name = Path('').temp().joinExt('tar')
tar = new Tar(name)

tar.create(Path('.').glob('a_very*/*.dat'))
assert(name.length > 0)
let sep = FileSystem('/').separators[0]
assert(tar.list() =='a_very_long_directory__________________________________________________________________________________________' + 
sep + 'a_very_long_but_less_than_100_chars_filename_________________________________.dat')
name.remove()
