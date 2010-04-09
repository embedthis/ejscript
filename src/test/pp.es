function fun(x) {
    return function closure() {
        return x
    }
}

a = fun(1)
b = fun(2)
c = fun(3)

print(a())
print(b())
print(c())


