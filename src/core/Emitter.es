/*
    Emitter.es -- Event emitter.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The emitter class supports the registration of listeners who want notification of events of interest.
        @example
            events.addListener(event, function (event, ...args) {
                //  Do something
            }
            events.emit("topic", 1, 2, 3)
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

        private function addOneListener(name: String, callback: Function): Void {
            let listeners : Array? = endpoints[name]
            if (listeners) {
                for each (var e: Endpoint in listeners) {
                    if (e.callback == callback && e.name == name) {
                        return
                    }
                }
            } else {
                listeners = endpoints[name] = new Array
            }
            if (callback) {
//MOB           callback.bind(this)
                listeners.append(new Endpoint(callback, name))
                emit("addListener", name, callback)
            }
        }

        /** 
            Add a listener function for events.
            @param name Event name to listen for. The listener will receive events of this event class or any of its 
            subclasses.  The name can be a string or an array of event strings.
            @param callback Function to call when the event is received.
         */
        function addListener(name: Object, callback: Function): Void {
            if (name is String) {
                addOneListener(name, callback)
            } else if (name is Array) {
                for each (n in name) {
                    addOneListener(n, callback)
                }
            } else {
                throw new Error("Bad name type for addListener")
            }
        }

        /** 
            Clear listeners for a given event name. 
            @param name Event name to clear. The name can be a string or an array of event strings. If null, listeners 
            for all event names are cleared.
         */
        function clearListeners(name: Object? = null): Void {
            if (name == null) {
                endpoints = new Object
            } else if (name is Array) {
                for each (n in name) {
                    listeners = endpoints[n] = new Array
                }
            } else {
                listeners = endpoints[name] = new Array
            }
        }

        /** 
            Determine if the emitter has any listeners.
            @return True if there are currently registered listeners
        */
        function hasListeners(): Boolean 
            endpoints.length > 0

        /** 
            Return the listeners for this emitter. 
            @param name Event name to send to the listeners.
            @return An array of listener endpoints. These are cloned and not the actual listener objects.
         */
        function listeners(name: String): Array
            endpoints[name].clone(true)
       
        //  MOB -- rename send() or fire()
        /** 
            Emit an event to the registered listeners.
            @param name Event name to send to the listeners.
            @param args Args to pass to the listener callback
         */
        function emit(name: String, ...args): Void {
            let listeners: Array? = endpoints[name]
            if (listeners) {
                for each (var e: Endpoint in listeners) {
                    if (name == e.name) {
                        if (!e.active) {
                            e.active = true
                            do {
                                e.again = false
                                try {
                                    e.callback.apply(null, [name] + args)
                                } catch (e) {
                                    App.errorStream.write("Exception in event on listener: " + name  + "\n" + e)
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

        private function removeOneListener(name: String, callback: Function): Void {
            var listeners: Array? = endpoints[name]
            for (let i in listeners) {
                var e: Endpoint = listeners[i]
                if (e.callback == callback && e.name == name) {
                    emit("removeListener", name, callback)
                    listeners.splice(i, 1)
                }
            }
        }

        /** 
            Remove a registered listener.
            @param name Event name used when adding the listener.
            @param callback Listener callback function used when adding the listener.
         */
        function removeListener(name: Object, callback: Function): Void {
            if (name is String) {
                removeOneListener(name, callback)
            } else if (name is Array) {
                for each (n in name) {
                    removeOneListener(n, callback)
                }
            } else {
                throw new Error("Bad name type for removeListener")
            }
        }
    }


    /* Listening endpoints */
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
