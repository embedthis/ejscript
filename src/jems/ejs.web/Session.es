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

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
