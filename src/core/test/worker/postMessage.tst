/*
    PostMessage Worker Tests
 */
var w: Worker

w = new Worker("postMessage.es")
w.postMessage("Back to you")
Worker.join()
