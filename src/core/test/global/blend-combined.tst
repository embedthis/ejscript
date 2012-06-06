let a = {
    '+colors': 'red',
    '+numbers': 'one two three',
    '=weather': 'sunny',
    '-ignore': 'anything',

}

let b = {
    '+colors': 'blue',
    '-numbers': 'two',
    '=weather': 'cloudy',
}

//  Blend with empty using add, subtract and assign
let c = blend({}, a, {combine: true})
assert(serialize(c) == '{"colors":"red","numbers":"one two three","weather":"sunny"}')

//  Blend new with existing using empty using add, subtract and assign
let d = blend(c, b, {combine: true})
assert(serialize(d) == '{"colors":"red blue","numbers":"one  three","weather":"cloudy"}')


a = {
    numbers: ["one", "two", "three"],
}

b = {
    '+numbers': ["four", "five", "six"],
    '-numbers': ["three", "two"],
}

//  Blend new with existing using empty using add, subtract and assign
let c = blend(a, b, {combine: true})
assert(serialize(c) == '{"numbers":["one","four","five","six"]}')


a = {}
b = {
    '+settings': {
        debug: true,
		charlen: 1,
	},
}


//  Blend with empty using add, subtract and assign
let c = blend(a, b, {combine: true})
assert(serialize(c) == '{"settings":{"debug":true,"charlen":1}}')


//  Blend with duplicates
a = {
    colors: ['red', 'blue', 'green']
}

b = {
    '+colors': ['red', 'yellow']
}
let c = blend(a, b, {combine: true})
assert(serialize(c) == '{"colors":["red","blue","green","yellow"]}')
