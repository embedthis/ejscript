let a = {
    entry: null
}

blend(a, { entry: { number: 42}})
assert(a.entry.number == 42)
