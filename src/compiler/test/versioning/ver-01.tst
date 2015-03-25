/*
    Test module versioning
 */

var geometry1 = '
    module geometry {
        var x = 1
    }
'

var geometry2 = '
    module geometry {
        var x = 1
        var y = 1
    }
'

//  Create the geometry module and compile drawing which requires it
let bin = App.test.bin
let ejs = bin.join('ejs')
let ejsc = bin.join('ejsc')

Path("geometry.es").write(geometry1)
Cmd.run(ejsc + " geometry.es")
Cmd.run(ejsc + " drawing.es")
Cmd.run(ejs + " drawing.mod")

//  Modify geometry module and re-run drawing. Should fail to run.
Path("geometry.es").write(geometry2)
Cmd.run(ejsc + " geometry.es")

let caught = false
//  Don't recompile drawing.es
try {
    //  This should fail as the required geometry module has changed
    Cmd.run(ejs + " drawing.mod")
} catch {
    caught = true
}
assert(caught)
