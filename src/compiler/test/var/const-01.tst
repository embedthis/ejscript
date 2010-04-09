/*
    Test const declarations

 */

const x = 1
caught = false ; try { x = 2 } catch { caught = true} 
assert(x == 1)
assert(caught)

class Shape {
    const x = 1
}

Shape()
Shape()
