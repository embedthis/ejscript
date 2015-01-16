/**
    ejsEjsLocalCache.c - Native code for the Local cache class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Locals **********************************/

//  TODO - should this be refactored to use MprCache?

#define CACHE_TIMER_PERIOD  (60 * MPR_TICKS_PER_SEC)
#define CACHE_HASH_SIZE     257
#define CACHE_LIFESPAN      (86400 * MPR_TICKS_PER_SEC)

typedef struct EjsLocalCache
{
    EjsObj          obj;                /* Object base */
    MprHash         *store;             /* Key/value store */
    MprMutex        *mutex;             /* Cache lock */
    MprEvent        *timer;             /* Pruning timer */
    MprTicks        lifespan;           /* Default lifespan (msec) */
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
    MprTicks    lifespan;               /* Lifespan after each access to key (msec) */
    int64       version;
} CacheItem;

/*********************************** Forwards *********************************/

static void localPruner(EjsLocalCache *cache, MprEvent *event);
static void manageLocalCache(EjsLocalCache *cache, int flags);
static void manageCacheItem(CacheItem *item, int flags);
static void removeItem(EjsLocalCache *cache, CacheItem *item);
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
    wantShared = (options && ejsGetPropertyByName(ejs, options, EN("shared")) == ESV(true));

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
        assert(cache == shared);
    }
    key = argv[0];
    expires = argv[1];

    lock(cache);
    //  UNICODE
    if ((item = mprLookupKey(cache->store, key->value)) == 0) {
        unlock(cache);
        return ESV(false);
    }
    item->lifespan = 0;
    if (expires == ESV(null)) {
        removeItem(cache, item);
    } else {
        item->expires = ejsGetDate(ejs, expires);
    }
    unlock(cache);
    return ESV(true);
}


/*
    function inc(key: String, amount: Number): Number
 */
static EjsAny *sl_inc(Ejs *ejs, EjsLocalCache *cache, int argc, EjsAny **argv)
{
    EjsString   *key;
    CacheItem   *item;
    int64       amount;

    if (cache->shared) {
        cache = cache->shared;
        assert(cache == shared);
    }
    key = argv[0];
    amount = (argc >= 2) ? ejsGetInt(ejs, argv[1]) : 1;

    lock(cache);
    //  UNICODE
    if ((item = mprLookupKey(cache->store, key->value)) == 0) {
        if ((item = mprAllocObj(CacheItem, manageCacheItem)) == 0) {
            unlock(cache);
            ejsThrowMemoryError(ejs);
            return 0;
        }
    } else {
        amount += stoi(item->data->value);
    }
    if (item->data) {
        cache->usedMem -= item->data->length;
    }
    item->data = ejsCreateStringFromAsc(ejs, itos(amount));
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
        assert(cache == shared);
    }
    result = ejsCreateEmptyPot(ejs);
    ejsSetPropertyByName(ejs, result, EN("keys"), 
        ejsCreateNumber(ejs, (MprNumber) (cache->maxKeys == MAXSSIZE ? 0 : cache->maxKeys)));
    ejsSetPropertyByName(ejs, result, EN("lifespan"), 
        ejsCreateNumber(ejs, (MprNumber) (cache->lifespan / MPR_TICKS_PER_SEC)));
    ejsSetPropertyByName(ejs, result, EN("memory"), 
        ejsCreateNumber(ejs, (MprNumber) (cache->maxMem == MAXSSIZE ? 0 : cache->maxMem)));
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
        assert(cache == shared);
    }
    key = argv[0];
    getVersion = 0;

    if (argc >= 2 && ejsIsDefined(ejs, argv[1])) {
        options = argv[1];
        getVersion = ejsGetPropertyByName(ejs, options, EN("version")) == ESV(true);
    }
    lock(cache);
    //  UNICODE
    if ((item = mprLookupKey(cache->store, key->value)) == 0) {
        unlock(cache);
        return ESV(null);
    }
    if (item->expires && item->expires <= mprGetTime()) {
        unlock(cache);
        return ESV(null);
    }
#if UNUSED && FUTURE
    //  TODO - should reading refresh cache
    //  Perhaps option "read-refresh"
    if (item->lifespan) {
        item->expires = mprGetTime() + item->lifespan;
    }
#endif
    if (getVersion) {
        result = ejsCreatePot(ejs, ESV(Object), 2);
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
        assert(cache == shared);
    }
    key = argv[0];
    lock(cache);
    if (ejsIsDefined(ejs, key)) {
        //  UNICODE
        if ((item = mprLookupKey(cache->store, key->value)) != 0) {
            cache->usedMem -= (key->length + item->data->length);
            mprRemoveKey(cache->store, key->value);
            result = ESV(true);
        } else {
            result = ESV(false);
        }

    } else {
        /* Remove all keys */
        result = mprGetHashLength(cache->store) ? ESV(true) : ESV(false);
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
        assert(cache == shared);
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
        assert(cache == shared);
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
    MprKey      *kp;
    MprTime     expires;
    ssize       len, oldLen;
    int64       lifespan, version;
    int         checkVersion, exists, add, set, prepend, append, throw;

    if (cache->shared) {
        cache = cache->shared;
        assert(cache == shared);
    }
    checkVersion = exists = add = prepend = append = throw = 0;
    set = 1;
    expires = 0;
    lifespan = cache->lifespan;
    version = 0;
    key = argv[0];
    value = argv[1];

    if (argc >= 3 && argv[2] != ESV(null)) {
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
        throw = (vp = ejsGetPropertyByName(ejs, options, EN("throw"))) == ESV(true);
        if ((vp = ejsGetPropertyByName(ejs, options, EN("version"))) != 0) {
            version = ejsGetInt64(ejs, vp);
            checkVersion = 1;
        }
    }
    lock(cache);
    if ((kp = mprLookupKeyEntry(cache->store, key->value)) != 0) {
        exists++;
        item = (CacheItem*) kp->data;
        if (checkVersion) {
            if (item->version != version) {
                unlock(cache);
                if (throw) {
                    ejsThrowStateError(ejs, "Key version does not match");
                }
                return ESV(null);
            }
        }
    } else {
        if ((item = mprAllocObj(CacheItem, manageCacheItem)) == 0) {
            unlock(cache);
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
            return ESV(null);
        }
        item->data = value;
    } else if (append) {
        item->data = ejsJoinString(ejs, item->data, value);
    } else if (prepend) {
        item->data = ejsJoinString(ejs, value, item->data);
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
        mprDebug("ejs cache", 5, "Start LocalCache pruner with resolution %d", cache->resolution);
        /* 
            Use the MPR dispatcher incase this VM is destroyed 
         */
        cache->timer = mprCreateTimerEvent(MPR->dispatcher, "localCacheTimer", cache->resolution, localPruner, cache, 
            MPR_EVENT_STATIC_DATA); 
    }
    unlock(cache);
    //  UNICODE
    return ejsCreateNumber(ejs, (MprNumber) len);
}


static void removeItem(EjsLocalCache *cache, CacheItem *item)
{
    lock(cache);
    //UNICODE
    mprRemoveKey(cache->store, item->key->value);
    cache->usedMem -= (item->key->length + item->data->length);
    unlock(cache);
}


/*
    Check for expired keys
 */
static void localPruner(EjsLocalCache *cache, MprEvent *event)
{
    MprTime         when, factor;
    MprKey          *kp;
    CacheItem       *item;
    ssize           excessKeys;

    if (mprTryLock(cache->mutex)) {
        when = mprGetTime();
        /*
            Prune keys expiring first.
         */
        for (kp = 0; (kp = mprGetNextKey(cache->store, kp)) != 0; ) {
            item = (CacheItem*) kp->data;
#if KEEP
            mprDebug("ejs cache", 6, "LocalCache: \"%@\" lifespan %d, expires in %d secs", item->key, 
                    item->lifespan / 1000, (item->expires - when) / 1000);
#endif
            if (item->expires && item->expires <= when) {
                mprDebug("ejs cache", 5, "LocalCache prune expired key %s", kp->key);
                removeItem(cache, item);
            }
        }
        assert(cache->usedMem >= 0);

        /*
            If too many keys or too much memory used. Prune oldest first.
         */
        if (cache->maxKeys < MAXSSIZE || cache->maxMem < MAXSSIZE) {
            excessKeys = mprGetHashLength(cache->store) - cache->maxKeys;
            for (factor = 3600; factor < (86400 * 1000); factor *= 4) {
                if (excessKeys > 0 || cache->usedMem > cache->maxMem) {
                    for (kp = 0; (kp = mprGetNextKey(cache->store, kp)) != 0; ) {
                        item = (CacheItem*) kp->data;
                        if (item->expires && item->expires <= when) {
                            mprDebug("ejs cache", 5, "LocalCache too big execess keys %Ld, mem %Ld, prune key %s", 
                                    excessKeys, (cache->maxMem - cache->usedMem), kp->key);
                            removeItem(cache, item);
                            excessKeys--;
                        }
                    }
                    when += factor;
                } else {
                    break;
                }
            }
        }
        assert(cache->usedMem >= 0);

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


PUBLIC void ejsConfigureLocalCacheType(Ejs *ejs)
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

    Copyright (c) Embedthis Software. All Rights Reserved.

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
