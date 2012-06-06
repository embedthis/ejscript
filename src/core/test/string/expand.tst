/*
    expand.tst - Test expand
 */

var s = "one ${two} three ${four}"
assert(s.expand({two: 2, four: 4}) == "one 2 three 4")
