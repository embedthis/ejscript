/**
    Session.es -- Session state management
 */

module ejs.web {
    /** 
        Stores session state information. The session array will be created automatically if SessionAutoCreate 
        is defined or if a session is started via the useSession() or createSession() functions. Sessions are 
        shared among requests that come from a single client. This may mean that multiple requests access the 
        same session concurrently. Ejscript ensures that such accesses are serialized. The elements are user defined.
        @spec ejs
        @stability prototype
     */
    var session: Object

    /** 
        Session state storage class. The Session class provides management over sessions supporting listeners that
        trigger when sessions are created or destroyed.
        @spec ejs
     */
    class Session { 
        /** 
            Add a listener to session state store.
            @param name Name of the event to listen for. The name may be an array of events.
            @param listener Callback listening function. The function is called with the following signature:
                function listener(event: String, ...args): Void
            @event createSession Issued when a new session is created
            @event destroySession Issued when a session is destroyed
         */
        native static function addListener(name, listener: Function): Void

        /** 
            Get the count of active sessions
            @return The number of active sessionss
         */
        native static function get count(): Number

        /** 
            Remove a listener
            @param name Event name previously used with addListener. The name may be an array of events.
            @param listener Listener function previously used with addListener.
         */
        native static function removeListener(name, listener: Function): Void

//  MOB -- how to identify the session
        /** 
            Set a session timeout
            @param timeout Timeout for the session in seconds. 
            A value of -1 means no timeout.
         */
        native function setSessionTimeout(timeout: Number): Object 
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
