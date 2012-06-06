/*
    Test that modifying a collection during iteration does not spin forever
 */
var bit = {
    ext: {
        exe: "",
        lib: "a",
        o: "o",
        res: "res",
        shlib: "dylib",
        shobj: "dylib",
        dotexe: "",
        dotlib: ".a",
        doto: ".o",
        dotres: ".res",
        dotshlib: ".dylib",
        dotshobj: ".dylib",
    }
}

for (let [key,value] in bit.ext) {
    if (value) {
        bit.ext['dot' + key] = '.' + value
    } else {
        bit.ext['dot' + key] = value
    }
}

