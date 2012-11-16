/**
    Session.es -- Session state management
 */

module ejs.web {
    /** 
        Session state storage class. 
        The session state objects can be used to share state between requests. The $ejs.web::Request object has
        a $ejs.web::Request.session property that stores a Session object instance. Values stored into the session 
        state are serialized and deserialized upon retrieval.
        @spec ejs
    */
    dynamic class Session { 
        use default namespace public

        /**
            Create a new session state object. This uses the Store in-memory caching service.
            @param key Optional key to provide. If omitted or null, a new key will be generated. If the given key has
                expired, a new key will be generated.
            @param options
            @option lifespan Lifespan in seconds for the session
         */
        native function Session(key, options: Object? = null)

        /**
            Get the session ID key for a session. This is a static method so as to not clash with session properties.
            @param session Session to examine
            @return The session ID key.
         */
        native static function key(session: Session!): String

        /**
            Destroy a session
            @param session Session to destroy
         */
        native static function destorySession(session: Session!): Void
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
