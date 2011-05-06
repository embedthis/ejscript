/*
    PostMessage worker
 */

print("TOP")
breakpoint()
onmessage = function (e) {
    print("BEFORE EXIT " + e)
    exit()
}
print("BEFORE RUN")
App.run()
