/*
    expand.tst - Test expand
 */

//  Basic usage
var s = "one ${two} three ${four}"
assert(s.expand({two: 2, four: 4}) == "one 2 three 4")

//  Throw for missing tokens
let caught
try {
    s = "one ${two} three ${four}"
    assert(s.expand({}) == "one 2 three 4")
} catch (e) {
    caught = true
}
assert caught = true


//  Throw for missing tokens
caught = false
try {
    s = "one ${two} three ${four}"
} catch (e) {
    caught = true
}


//  Replace missing tokens
s = "one ${two} three ${four}"
assert(s.expand({}, {missing: 'MISSING'}) == "one MISSING three MISSING")


//  Preserve tokens
s = "one ${two} three ${four}"
assert(s.expand({}, {missing: true}) == s)


//  Remove tokens
s = "one ${two} three ${four}"
assert(s.expand({}, {missing: false}) == 'one  three ')

