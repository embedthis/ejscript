/*
    Test glob
 */

//  General
assert(Path('abc.html').glob('**.html') == true)
assert(Path('abc.html').glob('**/*.html') == true)
assert(Path('abc.html').glob('*.xhtml') == false)

assert(Path('dir/a.tst').glob('**.tst') == true)
assert(Path('dir/a.tst').glob('**.es') == false)
assert(Path('dir/a.tst').glob('**') == true)

assert(Path('app').glob('**.tst') == false)

//  Array args

assert(Path('/a/b/c').glob(['/a/b/c']))
assert(Path('/a/b/c').glob(['/a', '/a/b/c']))
assert(!Path('/a/b/c').glob(['/a', '/a/b']))

//  Negated patterns
assert(Path('/a/b/c').glob(['/a**']))
assert(!Path('/a/b/c').glob(['/a**', '!/a/b/c']))

assert(!Path("abc").glob(null))

//  Empty args
assert(Path("").glob(""))
assert(!Path("abc").glob(""))
assert(!Path("").glob("abc"))

//  Substrings
assert(Path("abc").glob("abc"))
assert(!Path("abc").glob("abcd"))
assert(!Path("abc").glob("ab"))

//  Subpaths
assert(Path("/a/b").glob("/a/b"))
assert(!Path("/a/b").glob("/a/b/c"))
assert(!Path("/a/b").glob("/a/"))

//  Trailing separators
assert(Path("/a/b/c").glob("/a/b/c/"))
assert(Path("/a/b/c/").glob("/a/b/c"))

//  Wild 
assert(Path("abc").glob("*"))
assert(Path("abc").glob("a*"))
assert(Path("abc").glob("*c"))
assert(Path("abc").glob("a*c"))
assert(!Path("abc").glob("a*d"))

//  Single char
assert(Path("abc").glob("???"))
assert(!Path("abc").glob("??"))
assert(!Path("abc").glob("?"))
assert(Path("abc").glob("?*"))
assert(Path("abc").glob("*?"))

//  Double wild
assert(Path("a/b/c").glob("**"))
assert(Path("a/b/c").glob("**c"))
assert(Path("a/b/c").glob("**/c"))
assert(Path("a/b/c").glob("**/*c"))
assert(Path("a/b/c").glob("a/**c"))
assert(Path("a/b/c").glob("a/**/*c"))

assert(!Path("a/b/c").glob("a/**/d"))
assert(!Path("a/b/c").glob("b/**"))
assert(!Path("a/b/c").glob("**/x/c"))

//  Non-canonical separators
assert(Path("a////b").glob("a/b"))
assert(Path("a/b").glob("a/////b"))

//  Trailing separators
assert(Path("a/b/").glob("a/b"))
assert(Path("a/b").glob("a/b/"))

//  Quad wild
assert(Path("a/b/c").glob("****c"))
assert(Path("a/b/c").glob("**/**c"))

//  Pattern as directory
assert(Path('a.c').glob('**/a.c') == true)
assert(Path('a.c/a.c').glob('**/a.c') == true)
assert(Path('a.c/a.c/a.c').glob('**/a.c') == true)
assert(Path('a.c/a.c/a.c').glob('**/a.c/a.c') == true)
assert(Path('a.c').glob('**/a.c/a.c') == false)
assert(Path('a.c/a.c').glob('**/a.c/a.c') == true)

//  Stress
assert(Path("/a/b/c/d/e/f/g").glob("/a**"))
assert(Path("/a/b/c/d/e/f/g").glob("/a**/c/**/g"))
assert(!Path("/a/b/c/d/e/f/g").glob("/a**/c/**/h"))
assert(!Path("/a/b/c/d/e/f/g").glob("/a**/k/**/g"))

