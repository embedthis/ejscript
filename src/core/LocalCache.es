/*
    LocalCache.es -- Local in-memory key/value cache class 
 */

module ejs {

    /**
        Fast, non-scalable, non-durable, in-memory key/value cache class. The cache is in-process and is not 
        available to other processes on the same machine. Key/value data is not persisted. Use this cache if you require
        the fastest, non-scalable key/value cache alternative. There is one cache per Application. All uses of the LocalCache
        class connect to the one cache using one domain of keys.
        @stability prototype
     */
    class LocalCache {
        use default namespace public

        /**
            Create and connect to the local cache. An application has only one cache regardless of how many LocalCache
            instances are created. All LocalCache instances connect to the same underlying cache. Multiple interpreters
            can use the local cache to reliably share information.
            @param options Configuration options.
            @option shared Create or connect to a single, shared cache. If multiple interpreters in a single
                process create shared LocalCache instances they will be able to share key/value data.
            @option lifespan Default lifespan for key values in seconds. Set to zero for a default unlimited timeout.
            @option resolution Time in milliseconds to check for expired expired keys
            @option memory Maximum memory to use for keys and data
            @option trace Trace I/O operations for debug
         */
        native function LocalCache(options: Object = null)

        /**
            Destroy the cache
         */
        native function destroy(): Void

        /**
            Set a fixed expire date for a key. After defining an expiry, the key's lifespan will not be renewed via
            access to the key.
            @param key Key to modify.
            @param expires Date at which to expire the data. Set expires to null to cause the key to never expire.
            @return True if the key's expiry can be updated. 
         */
        native function expire(key: String~, expires: Date): Boolean

        /**
            Increment a key's value by a given amount. This operation is atomic.
            @param key Key value to read.
            @param amount Amount by which to increment the value. This amount can be negative to achieve a decrement.
            @return The new key value
         */ 
        native function inc(key: String~, amount: Number~ = 1): Number

        /**
            Resource limits for the server and for initial resource limits for requests.
            @param limits. Limits is an object hash with the following properties:
            @option keys Maximum number of keys in the cache.
            @option lifespan Default time to preserve key data in seconds. Set to zero for an unlimited default timeout.
            @option memory Maximum memory to use for keys and data.
            @see setLimits
          */
        native function get limits(): Object

        /**
            Read a key. Read will return the keys value or null if the key is not present or expired. If options.version
            is set to true, then the call will return an object with "data" and "version" properties where version is
            the key's unique update version ID. Each time the key is updated, the version ID is automatically changed to
            a new value. Versions IDs can be used used for conditional (CAS) writes.
            @param key Key value to read.
            @param options Read options
            @option version If set to true, the read will return an object hash containing the data and a unique version 
                identifier for the last update to this key. This version identifier can be specified to write to peform
                an atomic CAS (check and swap) operation.
            @return Null if the key is not present. Otherwise return key data as a string or if the options parameter 
                specified "version == true", return an object with the properties "data" for the key data and 
                "version" for the CAS version identifier.
         */
        native function read(key: String~, options: Object = null): String

        /**
            Remove the key and associated value from the cache
            @param key Key value to remove. If key is null, then all keys are removed.
            @return True if the key was removed.
         */
        native function remove(key: String): Boolean

        /**
            Update the cache cache resource limits. The supplied limit fields are updated.
            See the $limits property for limit field details.
            @param limits Object hash of limit fields and values
            @see limits
         */
        native function setLimits(limits: Object): Void

        /**
            Write the key and associated value to the cache. The value is written according to the optional mode option.  
            The key's expiry will be updated based on the defined lifespan from the current time.
            @param key Key to modify
            @param value String value to associate with the key
            @param options Options values
            @option expires When to expire the key. Takes precedence over lifetime. 
            @option lifespan Preservation time for the key in seconds. If zero, the key will never expire.
            @option mode Mode of writing: "set" is the default and means set a new value and create if required.
                "add" means set the value only if the key does not already exist. "append" means append to any existing
                value and create if required. "prepend" means prepend to any existing value and create if required.
                NOTE: only "set" is implemented.
            @option throw Throw an exception rather than returning null if the version id has been updated for the key. 
            @option version Required key version for the write to succeed. A version can be obtained via the $read 
                call. If set to true, the read will return an object hash containing the data and a unique version 
                identifier for the last update to this key. This version identifier can be specified to write to peform
                an atomic CAS (check and swap) operation.
            @return The number of bytes written, returns null if the write failed due to an updated version identifier for
                the key.
         */
        native function write(key: String~, value: String~, options: Object = null): Number
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
    
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

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
