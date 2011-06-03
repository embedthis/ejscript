/*
    Cache.es -- Cache class providing key/value storage.

    Usage Tutorial:

        cache = new Cache("local", {lifespan: 86400, timeout: 5000, memory: 200000000, keys: 10000000})

        session = cache.read(key)
        cache.write(key, value, {lifespan: 3600})
        cache.remove(key)
        cache.destroy()

        cache = new Cache("memcached", {addresses: ["127.0.0.1:11211"], debug: false})
        cache = new Cache("file", {dir: "/tmp"})

        Ejs internally uses the key naming convention:  ::module::key
 */
module ejs {

    /**
        Cache meta class to manage in-memory storage of key-value data. The Cache class provides an abstraction over
        various in-memory and disk-based caching cache backends.
        @stability prototype
     */
    class Cache {
        use default namespace public

        private var adapter: Object

        /**
            Cache constructor.
            @param adapter Adapter for the cache cache. E.g. "local". The Local cache is the only currently supported
                cache backend. 
            @param options Adapter options. The common options are described below, other options are passed through
            to the relevant caching backend.
            @option lifespan Default lifespan for key values
            @option resolution Time in milliseconds to check for expired expired keys
            @option timeout Timeout on cache I/O operations
            @option trace Trace I/O operations for debug
            @option module Module name containing the cache connector class. This is a bare module name without ".mod"
                or any leading path. If module is not present, a module name is derrived using "ejs.cache" + adapter.
            @option class Class name containing the cache backend. If the class property is not present, the 
                class is derived from the adapter name with "Cache" appended. The first letter of the adapter is converted
                to uppercase. For example, if the adapter was "mem", the class would be inferred to be "MemCache".
         */
        function Cache(adapter: String = null, options: Object = {}) {
            let adapterClass, modname
            if (adapter == null || adapter == "local") {
                options = blend({shared: true}, options, true)
                adapter = "local"
                modname = "ejs"
                adapterClass = "LocalCache"
            } else {
                adapterClass ||= options["class"] || (adapter.toPascal() + "Cache")
                modname ||= options.module || ("ejs.cache." + adapter)
                if (!global.modname::[adapterClass]) {
                    load(modname + ".mod", {reload: false})
                    if (!global.modname::[adapterClass]) {
                        throw "Can't find cache adapter: \"" + modname + "::" + adapter + "\""
                    }
                }
            }
            this.adapter = new global.modname::[adapterClass](options)
        }

        /**
            Destroy the cache
         */
        function destroy(): Void
            adapter.destroy()

        /**
            Set a new expire date for a key
            @param key Key to modify
            @param when Date at which to expire the data. Set to null to never expire.
         */
        function expire(key: String, when: Date): Void
            adapter.expire(key, when)

        /**
            Increment a key's value by a given amount. This operation is atomic.
            @param key Key value to read.
            @param amount Amount by which to increment the value. This amount can be negative to achieve a decrement.
            @return The new key value. If the key does not exist, it is initialized to the amount value.
         */ 
        function inc(key: String, amount: Number = 1): Number
            adapter.inc(key, amount)

        /**
            Resource limits for the server and for initial resource limits for requests.
            @param limits. Limits is an object hash. Depending on the cache backend in-use, the limits object may have
                some of the following properties. Consult the documentation for the actual cache backend for which properties
                are supported by the backend.
            @option keys Maximum number of keys in the cache. Set to zero for no limit.
            @option lifespan Default time to preserve key data. Set to zero for no timeout.
            @option memory Total memory to allocate for cache keys and data. Set to zero for no limit.
            @option retries Maximum number of times to retry I/O operations with cache backends.
            @option timeout Maximum time to transact I/O operations with cache backends. Set to zero for no timeout.
            @see setLimits
          */
        function get limits(): Object
            adapter.limits

        /**
            Read a key. 
            @param key Key value to read.
            @param options Read options
            @option version If set to true, the read will return an object hash containing the data and a unique version 
                identifier for the last update to this key. This version identifier can be specified to write to peform
                an atomic CAS (check and swap) operation.
            @return Null if the key is not present. Otherwise return key data as a string or if the options parameter 
                specified "version == true", return an object with the properties "data" for the key data and 
                "version" for the CAS version identifier.
         */
        function read(key: String, options: Object = null): String
            adapter.read(key, options)

        /**
            Read a key and return an object 
            This will read the data for a key and then deserialize. This assumes that $writeObj was used to store the
            key value.
            @param key Key value to read.
            @param options Read options
            @option version If set to true, the read will return an object hash containing the data and a unique version 
                identifier for the last update to this key. This version identifier can be specified to write to peform
                an atomic CAS (check and swap) operation.
            @return Null if the key is not present. Otherwise return key data as an object.
         */
        function readObj(key: String, options: Object = null): Object
            deserialize(adapter.read(key, options))

        /**
            Remove the key and associated value from the cache
            @param key Key value to remove. If key is null, then all keys are removed.
            @return true if the key was removed
         */
        function remove(key: String): Boolean
            adapter.remove(key)

        /**
            Update the cache cache resource limits. The supplied limit fields are updated.
            See the $limits property for limit field details.
            @param limits Object hash of limit fields and values
            @see limits
         */
        function setLimits(limits: Object): Void
            adapter.setLimits(limits)

        /**
            Write the key and associated value to the cache. The value is written according to the optional mode option.  
            @param key Key to modify
            @param value String value to associate with the key
            @param options Options values
            @option lifespan Preservation time for the key in seconds 
            @option expire When to expire the key. Takes precedence over lifetime.
            @option mode Mode of writing: "set" is the default and means set a new value and create if required.
                "add" means set the value only if the key does not already exist. "append" means append to any existing
                value and create if required. "prepend" means prepend to any existing value and create if required.
            @option version Unique version identifier to be used for a conditional write. The write will only be 
                performed if the version id for the key has not changed. This implements an atomic compare and swap.
                See $read.
            @option throw Throw an exception rather than returning null if the version id has been updated for the key.
            @return The number of bytes written, returns null if the write failed due to an updated version identifier for
                the key.
         */
        function write(key: String, value: String, options: Object = null): Number
            adapter.write(key, value, options)

        /**
            Write the key and associated object value to the cache. The object value is serialized using JSON notation and
            written according to the optional mode option.  
            @param key Key to modify
            @param value Object to associate with the key
            @param options Options values
            @option lifespan Preservation time for the key in seconds 
            @option expire When to expire the key. Takes precedence over lifetime.
            @option mode Mode of writing: "set" is the default and means set a new value and create if required.
                "add" means set the value only if the key does not already exist. "append" means append to any existing
                value and create if required. "prepend" means prepend to any existing value and create if required.
            @option version Unique version identifier to be used for a conditional write. The write will only be 
                performed if the version id for the key has not changed. This implements an atomic compare and swap.
                See $read.
            @option throw Throw an exception rather than returning null if the version id has been updated for the key.
            @return The number of bytes written, returns null if the write failed due to an updated version identifier for
                the key.
         */
        function writeObj(key: String, value: Object, options: Object = null): Number
            adapter.write(key, serialize(value), options)
    }

}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.
    
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
