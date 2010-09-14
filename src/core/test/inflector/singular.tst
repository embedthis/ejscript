/*
    Test toSingular
 */

assert(toSingular("cars") == "car")
assert(toSingular("waters") == "water")
assert(toSingular("sheep") == "sheep")
assert(toSingular("fish") == "fish")
assert(toSingular("money") == "money")
assert(toSingular("vertices") == "vertex")
assert(toSingular("buses") == "bus")
assert(toSingular("tomatoes") == "tomato")
assert(toSingular("hives") == "hive")

//  Already singular
assert(toSingular("vertex") == "vertex")
