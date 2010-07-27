
exports.add = function(...args) {
    let sum = 0
    for each (num in args) {
        sum += num
    }
    return sum
}
