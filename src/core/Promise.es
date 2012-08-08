/*
    Promise.es -- Promise keeper for deferred execution of async APIs.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The Promise class permits deferred processing for async APIs. A Promise encapsulates callbacks and state for an API
        that will take some time to execute. The API can return the promise and the caller can register callbacks for events
        of interest.
        WARNING: The CommonJS spec for promises is still changing 
        @spec commonjs
        @stability prototype
        @hide
     */
    dynamic class Promise extends Emitter {
        private var timer: Timer?
        private var complete: Boolean

        use default namespace public 

        /** 
            Add a callback observer for the "success" event. Returns this promise object.
            @param observer Callback function
            @return Returns this promise
         */
        function onSuccess(observer: Function): Promise {
            on("success", observer)
            return this
        }

        /** 
            Add a cancel callback observer for the "cancel" event. Returns this promise object.
            @param observer Callback function
            @return Returns this promise
         */
        function onCancel(observer: Function): Promise {
            on("cancel", observer)
            return this
        }

        /** 
            Add an error callback observer for the "error" event. Returns this promise object.
            @param observer Callback function
            @return Returns this promise
         */
        function onError(observer: Function): Promise {
            on("error", observer)
            return this
        }

        /** 
            Add a progress callback observer for the "progress" event. Returns this promise object.
            @param observer Callback function
            @return Returns this promise
         */
        function onProgress(observer: Function): Promise {
            on("progress", observer)
            return this
        }

        /** 
            Add a timeout callback observer for the "timeout" event. Returns this promise object.
            @param observer Callback function
            @return Returns this promise
         */
        function onTimeout(observer: Function): Promise {
            on("timeout", observer)
            return this
        }

        /** 
            Issue a "success" event with the given arguments. Once a result for the promise has been emitted via emitSucces,
            emitError or emitCancel, the Promise in completed and will not emit further events.
            @param args Args to pass to the observer
         */
        function emitSuccess(...args): Void {
            if (complete) {
                return
            }
            complete = true
            try {
                issue("success", ...args)
            } catch (e) {
                //  MOB
                print("CATCH", e)
                emitError(e)
            }
        }

        /** 
            Issue an "error" event with the given arguments. Once a result for the promise has been emitted via emitSucces,
            emitError or emitCancel, the Promise in completed and will not emit further events.
            @param args Args to pass to the observer
         */
        function emitError(...args): Void {
            if (complete) {
                return
            }
            complete = true
            try {
                issue("error", ...args)
            } catch (e) {
                //  MOB -- use logging
                print("EmitError CATCH", e)
            }
        }

        /** 
            Issue an "cancel" event with the given arguments. Once a result for the promise has been emitted via emitSucces,
            emitError or emitCancel, the Promise in completed and will not emit further events.
            @param args Args to pass to the observer
         */
        function emitCancel(...args): Void
            issue("cancel", ...args)

//  MOB -- why have cancel and emitCancel
        /** 
            Cancels the promise and removes "success" and "error" observers then issues a cancel event.
            @param args Args to pass to the "cancel" event observer
         */
        function cancel(...args): Void {
            complete = true
            if (timer) {
                timer.stop()
            }
            clearObservers(["success", "error"])
            issue("cancel", ...args)
        }

//  MOB -- what about cancel?
        /** 
            Convenience function to register callbacks. 
            @param success Success callback passed to onSuccess
            @param error error callback passed to onError
            @param cancel Cancel callback passed to onCancel
            @param progress Progress callback passed to onProgress
            @return this promise
         */
        function then(success: Function, error: Function? = null, cancel: Function? = null, 
                progress: Function? = null): Promise {
            on("success", success)
            if (error) {
                on("error", error)
            }
            if (cancel) {
                on("cancel", cancel)
            }
            if (progress) {
                on("progress", progress)
            }
            return this
        }

        /** 
            Create a timeout for the current promise. If the timeout expires before the promise completes or is cancelled, a
            "timeout" event and then an "error" event will be issued.
            @param msec Timeout in milliseconds
            @return this promise 
         */
        function timeout(msec: Number): Promise {
            if (timer) {
                timer.stop()
            }
            let timeoutComplete
            function wakeup(arg) {
                timeoutComplete = true
                if (timer) {
                    timer.stop()
                }
            }
            on("success", wakeup)
            on("error", wakeup)
            on("cancel", wakeup)
            timer = new Timer(msec, function() {
                if (complete || timeoutComplete) {
                    return;
                }
                timeoutComplete = true
                timer = null
                issue("timeout")
                issue("error")
            })
            timer.start()
            return this
        }
        
        //  MOB - fix MaxInt
        /** 
            Wait for the promise to complete for a given period. This blocks execution until the promise completes or 
            is cancelled.
            @param timeout Time to wait in milliseconds
            @return The arguments array provided to emitSuccess
         */
        function wait(timeout: Number = Number.MaxInt32): Object {
            let waitComplete = false
            let result
            function wakeup(event, ...args) {
                waitComplete = true
                result = args
            }
            on(["cancel", "error", "success"], wakeup)
            let t = new Timer(timeout, wakeup)
            t.start()
            while (!waitComplete && !complete) {
                App.run(timeout, true)
            }
            t.stop()
            return result
        }

        private function issue(name: String, ...args): Void {
            if (timer) {
                timer.stop()
            }
            if (args) {
                fire(name, ...args)
            } else {
                fire(name)
            }
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.md distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
