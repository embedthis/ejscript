/**
    ejsEjsLocalCache.c - Native code for the Local cache class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Locals **********************************/

#define CACHE_TIMER_PERIOD  (60 * MPR_TICKS_PER_SEC)
#define CACHE_TIMER_PERIOD  (60 * MPR_TICKS_PER_SEC)
#define CACHE_HASH_SIZE     257
#define CACHE_LIFESPAN      (86400 * MPR_TICKS_PER_SEC)

typedef struct EjsLocalCache
{
    EjsObj          obj;                /* Object base */
    MprHashTable    *store;             /* Key/value store */
    MprMutex        *mutex;             /* Cache lock*/
    MprEvent        *timer;             /* Pruning timer */
    MprTime         lifespan;           /* Default lifespan */
    int             resolution;         /* Frequence for pruner */
    ssize           usedMem;            /* Memory in use for keys and data */
    ssize           maxKeys;            /* Max number of keys */
    ssize           maxMem;             /* Max memory for session data */
    struct EjsLocalCache *shared;       /* Shared common cache */
} EjsLocalCache;

static EjsLocalCache *shared;           /* Singleton shared cache */

typedef struct CacheItem
{
    EjsString   *key;                   /* Original key */
    EjsString   *data;                  /* Cache data */
    MprTime     expires;                /* Fixed expiry date. If zero, key is imortal. */
    MprTime     lifespan;               /* Lifespan after each access to key */
    int64       version;
} CacheItem;

/*********************************** Forwards *********************************/

static void localPruner(EjsLocalCache *cache, MprEvent *event);
static void manageLocalCache(EjsLocalCache *cache, int flags);
static void manageCacheItem(CacheItem *item, int flags);
static void setLocalLimits(Ejs *ejs, EjsLocalCache *cache, EjsPot *options);

/************************************* Code ***********************************/
/*
   function Local(options)
 */
static EjsLocalCache *localConstructor(Ejs *ejs, EjsLocalCache *cache, int argc, EjsAny **argv)
{
    EjsPot      *options;
    int         wantShared;

    options = (argc >= 1 && ejsIsDefined(ejs, argv[0])) ? argv[0] : 0;
    wantShared = (options && ejsGetPropertyByName(ejs, options, EN("shared")) == S(true));

    if (wantShared && shared) {
        cache->shared = shared;
    } else {
        cache->mutex = mprCreateLock();
        cache->store = mprCreateHash(CACHE_HASH_SIZE, 0);
        cache->maxMem = MAXSSIZE;
        cache->maxKeys = MAXSSIZE;
        cache->resolution = CACHE_TIMER_PERIOD;
        cache->lifespan = CACHE_LIFESPAN;
        setLocalLimits(ejs, cache, options);
        if (wantShared) {
            shared = cache;
        }
    }
    return cache;
}


static EjsVoid *sl_destroy(Ejs *ejs, EjsLocalCache *cache, int argc, EjsObj **argv)
{
    if (cache->timer && cache != shared) {
        mprRemoveEvent(cache->timer);
        cache->timer = 0;
    }
    //  MOB - race here
    if (cache == shared) {
        shared = 0;
    }
    return 0;
}


/*
    function expire(key: String, expires: Date): Boolean
 */
static EjsAny *sl_expire(Ejs *ejs, EjsLocalCache *cache, int argc, EjsAny **argv)
{
    EjsString   *key;
    EjsDate     *expires;
    CacheItem   *item;

    if (cache->shared) {
        cache = cache->shared;
        mprAssert(cache == shared);
    }
    key = argv[0];
    expires = argv[1];

    lock(cache);
    //  UNICODE
    if ((item = mprLookupKey(cache->store, key->value)) == 0) {
        unlock(cache);
        return S(false);
    }
    item->lifespan = 0;
    if (expires == S(null)) {
        item->expires = 0;
    } else {
        item->expires = ejsGetDate(ejs, expires);
    }
    unlock(cache);
    return S(true);
}


/*
    function inc(key: String, amount: Number): Number
 */
static EjsAny *sl_inc(Ejs *ejs, EjsLocalCache *cache, int argc, EjsAny **argv)
{
    EjsString   *key;
    CacheItem   *item;
    int64       amount;
    char        nbuf[32];

    if (cache->shared) {
        cache = cache->shared;
        mprAssert(cache == shared);
    }
    key = argv[0];
    amount = (argc >= 2) ? ejsGetInt(ejs, argv[1]) : 1;

    lock(cache);
    //  UNICODE
    if ((item = mprLookupKey(cache->store, key->value)) == 0) {
        if ((item = mprAllocObj(CacheItem, manageCacheItem)) == 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
    } else {
        amount += stoi(item->data->value, 10, 0);
    }
    if (item->data) {
        cache->usedMem -= item->data->length;
    }
    item->data = ejsCreateStringFromAsc(ejs, itos(nbuf, sizeof(nbuf), amount, 10));
    cache->usedMem += item->data->length;
    item->expires = mprGetTime() + item->lifespan;
    item->version++;
    unlock(cache);
    return item->data;
}


/*
    function get limits(): Object
 */
static EjsPot *sl_limits(Ejs *ejs, EjsLocalCache *cache, int argc, EjsObj **argv)
{
    EjsPot      *result;

    if (cache->shared) {
        cache = cache->shared;
        mprAssert(cache == shared);
    }
    result = ejsCreateEmptyPot(ejs);
    ejsSetPropertyByName(ejs, result, EN("keys"), ejsCreateNumber(ejs, cache->maxKeys == MAXSSIZE ? 0 : cache->maxKeys));
    ejsSetPropertyByName(ejs, result, EN("lifespan"), 
        ejsCreateNumber(ejs, (MprNumber) (cache->lifespan / MPR_TICKS_PER_SEC)));
    ejsSetPropertyByName(ejs, result, EN("memory"), ejsCreateNumber(ejs, cache->maxMem == MAXSSIZE ? 0 : cache->maxMem));
    return result;
}


/*
    function read(key: String, options: Object = null): Object
 */
static EjsAny *sl_read(Ejs *ejs, EjsLocalCache *cache, int argc, EjsAny **argv)
{
    EjsString   *key;
    EjsAny      *result;
    EjsPot      *options;
    CacheItem   *item;
    int         getVersion;

    if (cache->shared) {
        cache = cache->shared;
        mprAssert(cache == shared);
    }
    key = argv[0];
    getVersion = 0;

    if (argc >= 2 && ejsIsDefined(ejs, argv[1])) {
        options = argv[1];
        getVersion = ejsGetPropertyByName(ejs, options, EN("version")) == S(true);
    }
    lock(cache);
    //  UNICODE
    if ((item = mprLookupKey(cache->store, key->value)) == 0) {
        unlock(cache);
        return S(null);
    }
    if (item->lifespan) {
        item->expires = mprGetTime() + item->lifespan;
    }
    if (getVersion) {
        result = ejsCreatePot(ejs, S(Object), 2);
        ejsSetPropertyByName(ejs, result, EN("version"), ejsCreateNumber(ejs, (MprNumber) item->version));
        ejsSetPropertyByName(ejs, result, EN("data"), item->data);
    } else {
        result = item->data;
    }
    unlock(cache);
    return result;
}


/*
    native function remove(key: String): Boolean
 */
static EjsBoolean *sl_remove(Ejs *ejs, EjsLocalCache *cache, int argc, EjsAny **argv)
{
    EjsString   *key;
    EjsAny      *result;
    CacheItem   *item;

    if (cache->shared) {
        cache = cache->shared;
        mprAssert(cache == shared);
    }
    key = argv[0];
    lock(cache);
    if (ejsIsDefined(ejs, key)) {
        //  UNICODE
        if ((item = mprLookupKey(cache->store, key->value)) != 0) {
            cache->usedMem -= (key->length + item->data->length);
            mprRemoveKey(cache->store, key->value);
            result = S(true);
        } else {
            result = S(false);
        }

    } else {
        /* Remove all keys */
        result = mprGetHashLength(cache->store) ? S(true) : S(false);
        cache->store = mprCreateHash(CACHE_HASH_SIZE, 0);
        cache->usedMem = 0;
    }
    unlock(cache);
    return result;
}


static void setLocalLimits(Ejs *ejs, EjsLocalCache *cache, EjsPot *options)
{
    EjsAny      *vp;

    if (!options) {
        return;
    }
    if (cache->shared) {
        cache = cache->shared;
        mprAssert(cache == shared);
    }
    if ((vp = ejsGetPropertyByName(ejs, options, EN("keys"))) != 0) {
        cache->maxKeys = (ssize) ejsGetInt64(ejs, vp);
        if (cache->maxKeys <= 0) {
            cache->maxKeys = MAXSSIZE;
        }
    }
    if ((vp = ejsGetPropertyByName(ejs, options, EN("lifespan"))) != 0) {
        cache->lifespan = (ssize) ejsGetInt64(ejs, vp) * MPR_TICKS_PER_SEC;
    }
    if ((vp = ejsGetPropertyByName(ejs, options, EN("memory"))) != 0) {
        cache->maxMem = (ssize) ejsGetInt64(ejs, vp);
        if (cache->maxMem <= 0) {
            cache->maxMem = MAXSSIZE;
        }
    }
    if ((vp = ejsGetPropertyByName(ejs, options, EN("resolution"))) != 0) {
        cache->resolution = ejsGetInt(ejs, vp);
        if (cache->resolution <= 0) {
            cache->resolution = CACHE_TIMER_PERIOD;
        }
    }
}


/*
    function setLimits(limits: Object): Void
 */
static EjsVoid *sl_setLimits(Ejs *ejs, EjsLocalCache *cache, int argc, EjsAny **argv)
{
    if (cache->shared) {
        cache = cache->shared;
        mprAssert(cache == shared);
    }
    setLocalLimits(ejs, cache, argv[0]);
    return 0;
}


/*
    function write(key: String~, value: String~, options: Object = null): Number
 */
static EjsNumber *sl_write(Ejs *ejs, EjsLocalCache *cache, int argc, EjsAny **argv)
{
    CacheItem   *item;
    EjsString   *key, *value, *sp;
    EjsPot      *options;
    EjsAny      *vp;
    MprTime     expires;
    MprHash     *hp;
    ssize       len, oldLen;
    int64       lifespan, version;
    int         checkVersion, exists, add, set, prepend, append, throw;

    if (cache->shared) {
        cache = cache->shared;
        mprAssert(cache == shared);
    }
    checkVersion = exists = add = prepend = append = throw = 0;
    set = 1;
    expires = 0;
    lifespan = cache->lifespan;
    key = argv[0];
    value = argv[1];

    if (argc >= 3 && argv[2] != S(null)) {
        options = argv[2];
        if ((vp = ejsGetPropertyByName(ejs, options, EN("lifespan"))) != 0) {
            lifespan = ejsGetInt64(ejs, vp) * MPR_TICKS_PER_SEC;
        }
        if ((vp = ejsGetPropertyByName(ejs, options, EN("expires"))) != 0 && ejsIs(ejs, vp, Date)) {
            expires = ejsGetDate(ejs, vp);
        }
        if ((sp = ejsGetPropertyByName(ejs, options, EN("mode"))) != 0) {
            set = 0;
            //  UNICODE
            if (scmp(sp->value, "add") == 0) {
                add = 1;
            } else if (scmp(sp->value, "append") == 0) {
                append = 1;
            } else if (scmp(sp->value, "prepend") == 0) {
                prepend = 1;
            } else if (scmp(sp->value, "set") == 0) {
                set = 1;
            }
        }
        throw = (vp = ejsGetPropertyByName(ejs, options, EN("throw"))) == S(true);
        if ((vp = ejsGetPropertyByName(ejs, options, EN("version"))) != 0) {
            version = ejsGetInt64(ejs, vp);
            checkVersion = 1;
        }
    }
    lock(cache);
    if ((hp = mprLookupKeyEntry(cache->store, key->value)) != 0) {
        exists++;
        item = (CacheItem*) hp->data;
        if (checkVersion) {
            if (item->version != version) {
                if (throw) {
                    ejsThrowStateError(ejs, "Key version does not match");
                } else {
                    return S(null);
                }
                unlock(cache);
            }
        }
    } else {
        if ((item = mprAllocObj(CacheItem, manageCacheItem)) == 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
        //  UNICODE
        mprAddKey(cache->store, key->value, item);
        set = 1;
        item->key = key;
    }
    oldLen = (item->data) ? (item->key->length + item->data->length) : 0;
    if (set) {
        item->data = value;
    } else if (add) {
        if (exists) {
            return S(null);
        }
        item->data = value;
    } else if (append) {
        item->data = ejsCatString(ejs, item->data, value);
    } else if (prepend) {
        item->data = ejsCatString(ejs, value, item->data);
    }
    if (expires) {
        /* Expires takes precedence over lifespan */
        item->expires = expires;
        item->lifespan = 0;
    } else if (lifespan == 0) {
        /* Never expires */
        item->expires = 0;
    } else {
        item->lifespan = lifespan;
        item->expires = mprGetTime() + item->lifespan;
    }
    item->version++;
    len = item->key->length + item->data->length;

    cache->usedMem += (len - oldLen);

    if (cache->timer == 0) {
        mprLog(5, "Start LocalCache pruner with resolution %d", cache->resolution);
        /* 
            Use the MPR dispatcher incase this VM is destroyed 
         */
        cache->timer = mprCreateTimerEvent(MPR->dispatcher, "localCacheTimer", cache->resolution, localPruner, cache, 
            MPR_EVENT_STATIC_DATA); 
    }
    unlock(cache);
    //  UNICODE
    return ejsCreateNumber(ejs, len);
}


/*
    Check for expired keys
 */
static void localPruner(EjsLocalCache *cache, MprEvent *event)
{
    MprTime         when, factor;
    MprHash         *hp;
    CacheItem       *item;
    ssize           excessKeys;

    if (mprTryLock(cache->mutex)) {
        when = mprGetTime();
        for (hp = 0; (hp = mprGetNextKey(cache->store, hp)) != 0; ) {
            item = (CacheItem*) hp->data;
            if (item->expires && item->expires <= when) {
                mprLog(5, "LocalCache prune key %s", hp->key);
                mprRemoveKey(cache->store, hp->key);
                cache->usedMem -= (item->key->length + item->data->length);
            }
        }
        mprAssert(cache->usedMem >= 0);

        /*
            If too many keys or too much memory used, prune keys expiring first.
         */
        if (cache->maxKeys < MAXSSIZE || cache->maxMem < MAXSSIZE) {
            excessKeys = mprGetHashLength(cache->store) - cache->maxKeys;
            while (excessKeys > 0 || cache->usedMem > cache->maxMem) {
                for (factor = 3600; excessKeys > 0; factor *= 2) {
                    for (hp = 0; (hp = mprGetNextKey(cache->store, hp)) != 0; ) {
                        if (item->expires && item->expires <= when) {
                            mprLog(5, "LocalCache prune key %s", hp->key);
                            mprRemoveKey(cache->store, hp->key);
                            cache->usedMem -= (item->key->length + item->data->length);
                        }
                    }
                    when += factor;
                }
                if (factor < 0) {
                    mprAssert(factor > 0);
                    break;
                }
            }
        }
        mprAssert(cache->usedMem >= 0);

        if (mprGetHashLength(cache->store) == 0) {
            mprRemoveEvent(event);
            cache->timer = 0;
        }
        unlock(cache);
    }
}


static void manageLocalCache(EjsLocalCache *cache, int flags) 
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(cache->store);
        mprMark(cache->mutex);
        mprMark(cache->timer);
        mprMark(cache->shared);

    } else if (flags & MPR_MANAGE_FREE) {
        if (cache == shared) {
            shared = 0;
        }
    }
}


static void manageCacheItem(CacheItem *item, int flags) 
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(item->key);
        mprMark(item->data);
    }
}


static EjsLocalCache *cloneLocalCache(Ejs *ejs, EjsLocalCache *src, bool deep)
{
    EjsLocalCache   *dest;

    if ((dest = ejsCreateObj(ejs, TYPE(src), 0)) == 0) {
        return 0;
    }
    if (src->shared) {
        dest->shared = src->shared;
    } else if (src == shared) {
        dest->shared = src;
    } else {
        dest->store = mprCreateHash(CACHE_HASH_SIZE, 0);
        dest->mutex = mprCreateLock();
        dest->timer = 0;
        dest->lifespan = src->lifespan;
        dest->resolution = src->resolution;
        dest->usedMem = src->usedMem;
        dest->maxMem = src->maxMem;
        dest->maxKeys = src->maxKeys;
        dest->shared = src->shared;
    }
    return dest;
}


void ejsConfigureLocalCacheType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "LocalCache"), sizeof(EjsLocalCache), 
            manageLocalCache, EJS_TYPE_OBJ | EJS_TYPE_MUTABLE_INSTANCES)) == 0) {
        return;
    }
    ejsAddImmutable(ejs, S_LocalCache, type->qname, type);
    type->helpers.clone = (EjsCloneHelper) cloneLocalCache;
    ejsBindConstructor(ejs, type, localConstructor);

    prototype = type->prototype;
    ejsBindMethod(ejs, prototype, ES_LocalCache_destroy, sl_destroy);
    ejsBindMethod(ejs, prototype, ES_LocalCache_expire, sl_expire);
    ejsBindMethod(ejs, prototype, ES_LocalCache_inc, sl_inc);
    ejsBindAccess(ejs, prototype, ES_LocalCache_limits, sl_limits, 0);
    ejsBindMethod(ejs, prototype, ES_LocalCache_read, sl_read);
    ejsBindMethod(ejs, prototype, ES_LocalCache_remove, sl_remove);
    ejsBindMethod(ejs, prototype, ES_LocalCache_setLimits, sl_setLimits);
    ejsBindMethod(ejs, prototype, ES_LocalCache_write, sl_write);
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
