/*
    Script to replicate missing end-exception instruction
 */
function fun() {
    let idle = false
    do {
       let x = 7
       for each (p in [1,2,3,4,5,6,7,8,9]) {
           do {
               try {
                    assert(x == 7)
                   throw "Bad"
                } catch {
                    assert(x == 7)
                    idle = true
                }
            } while (!idle)
       }
    } while (!idle)
}
fun()
assert(true)
