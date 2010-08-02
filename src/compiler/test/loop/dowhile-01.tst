/*
  	Do/while construct
 */

//  Basic do {} while
count = 0
do {
	count++
} while (count < 10)
assert(count == 10)


//  Ensure result of functions are pushed 
function fun(i) {
    return i < 9
}
count = 0
do {
} while(fun(count++))
assert(count == 10)
