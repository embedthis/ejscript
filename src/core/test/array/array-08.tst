/*
    Concat
 */

a = [1,2,3,4]
b = [5,6,7,8]

assert(a.concat(b) == "1,2,3,4,5,6,7,8")

a = [1,2,3]
b = []
assert(a.concat(b) == "1,2,3")
