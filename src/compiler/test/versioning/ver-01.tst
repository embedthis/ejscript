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
Path("geometry.es").write(geometry1)
sh(locate("ejsc") + " geometry.es")
sh(locate("ejsc") + " drawing.es")
sh(locate("ejs") + " drawing.mod")

//  Modify gemoetry module and re-run drawing. Should fail to run.
Path("geometry.es").write(geometry2)
sh(locate("ejsc") + " geometry.es")
let caught = false
//  Don't recompile drawing.es
try {
    //  This should fail as the required geometry module has changed
    sh(locate("ejs") + " drawing.mod")
} catch {
    caught = true
}
assert(caught)
