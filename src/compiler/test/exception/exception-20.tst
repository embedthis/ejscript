/*
    Test break in try still runs finally
 */

var finallyRan

for (i in 3) { 
    try {
        // Should break on first iteration
        break
    } catch (e) {
        break
    } finally {
        // Save iteration count
        finallyRan = i
    }
}
assert(finallyRan == 0)
