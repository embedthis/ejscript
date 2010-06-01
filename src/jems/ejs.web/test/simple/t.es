// function fun(a,b,c,d) {
function fun() {
    for (i in 10) {
        print("TRY " + i)
        try { throw "Boom" } catch (e) { print("CATCH");}
        App.sleep(100)
    }
}

fun()
// fun(1,2,3,4)
