/*
    String replace with function map
 */

function map(matched, sub1, offset, orig) {  
    /*
        print("orig " + orig)
        print("matched " + matched)
        print("sub1 " + sub1)
        print("offset " + offset)
    */
    assert(orig == "43 Hello 23")
    assert(matched == "ello")
    assert(sub1 == "ello")
    assert(offset == 4)
    return matched.toUpperCase()
}  
assert("43 Hello 23".replace(/([a-z]+)/g, map) == "43 HELLO 23")
assert("43 Hello 23".replace("ello", map) == "43 HELLO 23")
