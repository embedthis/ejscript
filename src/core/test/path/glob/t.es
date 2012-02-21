let d = Path(App.dir)

// dump(Path('/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d').glob('libssl*.dylib'))


/*
dump(d.glob('file.dat'))
dump(d.glob('*file.dat'))
dump(d.glob('*le.dat'))
dump(d.glob('file.dat*'))
dump(d.glob('file.d*'))

dump(d.glob('file*.dat'))
dump(d.glob('fi*.dat'))
dump(d.glob('file.*dat'))
dump(d.glob('file.*at'))
*/

//Must not match
// dump(d.glob('*zle.dat'))
// dump(d.glob('file.datx'))
// dump(d.glob('file*.t'))

// dump(d.glob('*'))
    //  sub only
// dump(d.glob('*/'))

// dump(d.glob('su*/'))
// dump(d.glob('su*'))

//  should equal sub only
// dump(d.glob('**/'))

//  match all files
// dump(d.glob('**/*'))

// dump(d.glob('**'))

// dump(d.glob('**', {include: /\/$/}))
// dump(d.glob('**', {exclude: /\/$/}))
// dump(d.glob('**', {include: /\./}))

// dump(d.glob('**', {hidden: true}))

dump(d.glob('**.dat'))

/*
    Should ** match files or directories or both

    - Doc Path.es, ejsPath.c with examples
 */
