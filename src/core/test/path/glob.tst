/*
    Test glob
 */

assert(Path('abc.html').glob('**.html') == true)
assert(Path('abc.html').glob('**/*.html') == true)
assert(Path('abc.html').glob('*.xhtml') == false)

assert(Path('dir/a.tst').glob('**.tst') == true)
assert(Path('dir/a.tst').glob('**.es') == false)

//  Where 'app' is a directory that exists
assert(Path('app').glob('**.tst') == false)

