/*
    Event.es -- Event class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The Event class encapsulates information pertaining to a system or application event. Applications typically
        subclass Event to add custom event data if required. 
        @stability prototype
        @example 
            class UpdateEvent extends Event { }
            events.emit("topic", new UpdateEvent)
     */
    class Event {
        use default namespace public

        /** 
            Event data associated with the Event. When Events are created, the constructor optionally takes an arbitrary 
            object data reference.
         */
        var data: Object

        /** 
            Time the event was created. The Event constructor will automatically set the timestamp to the current time.  
         */
        var timestamp: Date

        /** 
            Constructor for Event. Create a new Event object.
            @param data Arbitrary object to associate with the event.
         */
        function Event(data: Object? = null) {
            this.timestamp = new Date
            this.data = data
        }

        override function toString(): String
            "[Event: " +  typeOf(this) + "]"
    }

    /** 
        Event for exceptions
        @spec WebWorker
     */
    class ErrorEvent extends Event {
        use default namespace public

        //  MOB -- should have methods to capture the call stack
        /** 
            Event message
         */
        var message: String

        /** 
            Source filename of the script that issued the error
         */
        var filename: String

        /** 
            Source line number in the script that issued the error
         */
        var lineno: String

        /** 
            Callback stack at the point of the error
         */
        var stack: String
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
