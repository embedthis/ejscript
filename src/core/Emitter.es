/*
    Emitter.es -- Event emitter.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The emitter class provides a publish/subscribe model of communication. It supports the registration of observers
        who want to subscribe to events of interest. 
        @example
            events.observe(event, function (event, ...args) {
                //  Do something
            }
            events.fire("topic", 1, 2, 3)
        @stability prototype
     */
    class Emitter {
        use default namespace public

        private var endpoints: Object

        /** 
            Construct a new event Emitter object 
         */
        function Emitter()
            endpoints = new Object

        private function addOneObserver(name: String, callback: Function): Void {
            let observers : Array? = endpoints[name]
            if (observers) {
                for each (var e: Endpoint in observers) {
                    if (e.callback == callback && e.name == name) {
                        return
                    }
                }
            } else {
                observers = endpoints[name] = new Array
            }
            if (callback) {
                observers.append(new Endpoint(callback, name))
                fire("observe", name, callback)
            }
        }

        /** 
            Add an observer for a set of events.
            The callback will be invoked when the requested event is fired by calling Emitter.fire. When the callback 
            runs, it will be invoked with the value of "this" relevant to the context of the callback. If the callback
            is a class method, the value of "this" will be the object instance. Global functions will have "this" set to
            the global object. Use Function.bind to override the bound "this" value.
            @param name Event name to observe. The observer will receive events of this event class or any of its subclasses.
            The name can be a string or an array of event strings.
            @param callback Function to call when the event is received.
         */
        function observe(name: Object, callback: Function): Void {
            if (name is String) {
                addOneObserver(name, callback)
            } else if (name is Array) {
                for each (n in name) {
                    addOneObserver(n, callback)
                }
            } else {
                throw new Error("Bad name type for observe")
            }
        }

        /** 
            Clear observers for a given event name. 
            @param name Event name to clear. The name can be a string or an array of event strings. If null, observers 
            for all event names are cleared.
         */
        function clearObservers(name: Object? = null): Void {
            if (name == null) {
                endpoints = new Object
            } else if (name is Array) {
                for each (n in name) {
                    observers = endpoints[n] = new Array
                }
            } else {
                observers = endpoints[name] = new Array
            }
        }

        /** 
            Determine if the emitter has any observers.
            @return True if there are currently registered observers
        */
        function hasObservers(): Boolean 
            endpoints.length > 0

        /** 
            Return the observers for this emitter. 
            @param name Event name to fire to the observers.
            @return An array of observer endpoints. These are cloned and not the actual observer objects.
         */
        function getObservers(name: String): Array
            endpoints[name].clone(true)
       
        /** 
            Emit an event to the registered observers.
            @param name Event name to fire to the observers.
            @param args Args to pass to the observer callback
         */
        function fire(name: String, ...args): Void {
            let observers: Array? = endpoints[name]
            if (observers) {
                for each (var e: Endpoint in observers) {
                    if (name == e.name) {
                        if (!e.active) {
                            e.active = true
                            do {
                                e.again = false
                                try {
                                    /* This forces to use the bound this value */
                                    e.callback.apply(null, [name] + args)
                                } catch (e) {
                                    App.errorStream.write("Exception in event on observer: " + name  + "\n" + e)
                                }
                            } while (e.again)
                            e.active = false
                        } else {
                            e.again = true
                        }
                    }
                }
            }
        }

        /** @hide
            MOB -- complete
         */
        function delayedFire(name: String, delay: Number, ...args): Void {
            Timer(delay, function() {
                fire(name, ...args)
            })
        }

        private function removeOneObserver(name: String, callback: Function): Void {
            var observers: Array? = endpoints[name]
            for (let i in observers) {
                var e: Endpoint = observers[i]
                if (e.callback == callback && e.name == name) {
                    fire("removeObserver", name, callback)
                    observers.splice(i, 1)
                }
            }
        }

        /** 
            Remove a registered observer.
            @param name Event name used when the observer was added.
            @param callback Callback function used when the observer was added.
         */
        function removeObserver(name: Object, callback: Function): Void {
            if (name is String) {
                removeOneObserver(name, callback)
            } else if (name is Array) {
                for each (n in name) {
                    removeOneObserver(n, callback)
                }
            } else {
                throw new Error("Bad name type for removeObserver")
            }
        }

        //  LEGACY
        /** @deprecated
            @hide 
         */
        function addListener(name: Object, callback: Function): Void
            observe(name, callback)

        /** @deprecated
            @hide 
         */
        function emit(name: String, ...args): Void 
            fire(name, ...args)
    }


    /* Observing endpoints */
    internal class Endpoint {
        public var callback: Function
        public var name: String
        public var active: Boolean
        public var again: Boolean
        function Endpoint(callback: Function, name: String) {
            this.callback = callback
            this.name = name
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
