/*
    Test multiline matching
 */

s = 'one\ntwo\nthree'
assert(s.replace(/^/mg, '@') == '@one\n@two\n@three')

