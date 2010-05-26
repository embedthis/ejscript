/*
    Unicode literals
 */

s = "The æ fox"
ps="The \u00C3\u00A6 fox"
assert(s == ps)
assert(s.printable() == "The \\u00C3\\u00A6 fox")

