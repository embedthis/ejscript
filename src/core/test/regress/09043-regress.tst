/*
    The then-clause was being executed regardless
 */
var o = {
    exe: 42,
}

// The then-clause was being executed regardless
if (false) delete o.exe
assert(o.exe)

