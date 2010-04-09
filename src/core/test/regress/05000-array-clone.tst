/*
    Confirm deep clone copies array elements
 */

sub = ['a','b']
original = [1,2,3,4,sub]
shallow = original.clone(false)
deep = original.clone(true)

//  Modify sub
sub.push('c')

assert(original.toString() == shallow.toString())
assert(original.toString() != deep.toString())
