/*
    Test return in try
 */

function testReturn() {   
    try {   
        return "try"   
    } 
    catch (e: String) {   
        return "catch-string"   
    } 
    catch (e: Number) {   
        return "catch-number"   
    } 
    finally {   
        return "finally"  
    }
    return "normal"
}
assert(testReturn() == "finally")
