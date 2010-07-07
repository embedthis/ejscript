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
        private var timer: Timer
        private var fired: Boolean

        /** 
            Add a callback listener for the "success" event. Returns this promise object.
            @param listener Callback function
            @return Returns this promise
         */
        function addCallback(listener: Function): Promise {
            observe("success", listener)
            return this
        }

        /** 
            Add a cancel callback listener for the "cancel" event. Returns this promise object.
            @param listener Callback function
            @return Returns this promise
         */
        function addCancelback(listener: Function): Promise {
            observe("cancel", listener)
            return this
        }

        /** 
            Add an error callback listener for the "error" event. Returns this promise object.
            @param listener Callback function
            @return Returns this promise
         */
        function addErrback(listener: Function): Promise {
            observe("error", listener)
            return this
        }

        /** 
            Issue a "success" event with the given arguments. Once a result for the promise has been emitted via emitSucces,
            emitError or emitCancel, the Promise in completed and will not emit further events.
            @param args Args to pass to the listener
         */
        function emitSuccess(...args): Void {
            if (fired) {
                return
            }
            fired = true
            try {
                issue("success", ...args)
            } catch (e) {
print("CATCH", e)
                emitError(e)
            }
        }

        /** 
            Issue an "error" event with the given arguments. Once a result for the promise has been emitted via emitSucces,
            emitError or emitCancel, the Promise in completed and will not emit further events.
            @param args Args to pass to the listener
         */
        function emitError(...args): Void {
            if (fired) {
                return
            }
            fired = true
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
            @param args Args to pass to the listener
         */
        function emitCancel(...args): Void
            issue("cancel", ...args)

        /** 
            Cancels the promise and removes "success" and "error" and listeners then issues a cancel event.
            @param args Args to pass to the "cancel" event listener
         */
        function cancel(...args): Void {
            cancel = true
            fired = true
            if (timer) {
                timer.stop()
            }
            clearObservers(["success", "error"])
            issue("cancel", ...args)
        }

        /** 
            Convenience function to register callbacks. 
            @param success Success callback passed to addCallback
            @param error error callback passed to addErrback
            @param success Success callback passed to addCallback
            @param progress Currently ignored
            @return this promise
         */
        function then(success: Function, error: Function? = null, progress: Function? = null): Promise {
            observe("success", success)
            if (error) {
                observe("error", error)
            }
            if (progress) {
                observe("progress", progress)
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
            let done
            function awake(arg) {
                done = true
                if (timer) {
                    timer.stop()
                }
            }
            observe("success", awake)
            observe("error", awake)
            observe("cancel", awake)
            timer = new Timer(msec, function() {
                if (fired || done) {
                    return;
                }
                done = true
                timer = null
                issue("timeout")
                issue("error")
            })
            return this
        }
        
        /** 
            Wait for the promise to complete for a given period. This blocks execution until the promise completes or 
            is cancelled.
            @param timeout Time to wait in milliseconds
            @return The arguments array provided to emitSuccess
         */
        function wait(timeout: Number = -1): Object {
            let done = false
            let result
            function awake(event, ...args) {
                done = true
                result = args
            }
            observe(["cancel", "error", "success"], awake)
            timer = new Timer(timeout, awake)
            timer.start()
            while (!done && !fired) {
                App.serviceEvents(timeout, true)
            }
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
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
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
    
    @end
 */
