/*
    PostMessage Worker Tests
 */
var w: Worker

print("BEFORE NEW WORKER")
w = new Worker("postMessage.es")
print("BEFORE POST MESSAGE")
w.postMessage("Back to you")
print("AFTER POST MESSAGE")
Worker.join()
print("DONE")
