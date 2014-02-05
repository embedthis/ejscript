/*
    Test RegExp.exec with bad start
 */
let re = RegExp('Any pattern')

//  Should not crash
assert(re.exec('', '$1') == null)
