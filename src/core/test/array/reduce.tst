/*
    Test reduce, reduceRight
 */


// Flatten 
var flat = [[0,1], [2,3], [4,5]].reduce(function(a,b) {  
          return a.concat(b);  
}, []); 
assert(flat == "0,1,2,3,4,5")


// Sum values
var total = [0, 1, 2, 3].reduce(function(a, b){ return a + b; }); 
assert(total == 6)


// Accumulate
let result = [0,1,2,3,4].reduce(function(previous, current, index, array){  
    // print(previous, current, index)
    return previous + current;  
}); 
assert(result == 10)


// With initial value
let result = [0,1,2,3,4].reduce(function(previous, current, index, array){  
    // print(previous, current, index)
    return previous + current;  
}, 10); 
assert(result == 20)


// reduceRight
elts = []
let result = [0,1,2,3,4].reduceRight(function(previous, current, index, array){  
    // print(previous, current, index)
    elts.append(current)
    return previous + current;  
}, 10); 
assert(elts == "4,3,2,1,0")
assert(result == 20)

