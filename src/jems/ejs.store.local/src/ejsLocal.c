/**
    ejsEjsLocalStore.c - Native code for the Local store class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"
#include    "ejs.store.local.slots.h"

/************************************ Locals **********************************/

#define STORE_TIMER_PERIOD  (60 * MPR_TICKS_PER_SEC)
#define STORE_HASH_SIZE     257

typedef struct EjsLocalStore
{
    EjsPot          pot;                    /* Object base */
    MprHashTable    *cache;                 /* Key store */
    MprMutex        *mutex;                 /* Store lock*/
    MprEvent        *timer;
    MprTime         lifespan;               /* Default lifespan */
    ssize           usedMem;                /* Memory in use for keys and data */
    ssize           maxKeys;                /* Max number of keys */
    ssize           maxMem;                 /* Max memory for session data */
} EjsLocalStore;

typedef struct StoreItem
{
    EjsString   *key;                   /* Original key */
    EjsString   *data;                  /* Store data */
    MprTime     expires;                /* Fixed expiry date. If zero, key is imortal. */
    MprTime     lifespan;               /* Lifespan after each access to key */
    int64       version;
} StoreItem;

/*********************************** Forwards *********************************/

static void localTimer(EjsLocalStore *store, MprEvent *event);
static void manageLocalStore(EjsLocalStore *store, int flags);
static void manageStoreItem(StoreItem *item, int flags);
static void setLocalLimits(Ejs *ejs, EjsLocalStore *store, EjsPot *limits);

/************************************* Code ***********************************/
/*
   function Local(options)
 */
static EjsLocalStore *localConstructor(Ejs *ejs, EjsLocalStore *store, int argc, EjsAny **argv)
{
    store->mutex = mprCreateLock();
    store->cache = mprCreateHash(STORE_HASH_SIZE, 0);
    store->maxMem = MAXSSIZE;
    store->maxKeys = MAXSSIZE;
    if (argc >= 1) {
        setLocalLimits(ejs, store, argv[0]);
    }
    return store;
}


static EjsVoid *sl_destroy(Ejs *ejs, EjsLocalStore *store, int argc, EjsObj **argv)
{
    if (store->timer) {
        mprRemoveEvent(store->timer);
        store->timer = 0;
    }
    return 0;
}


/*
    function expire(key: String, expires: Date): Boolean
 */
static EjsAny *sl_expire(Ejs *ejs, EjsLocalStore *store, int argc, EjsAny **argv)
{
    EjsString   *key;
    EjsDate     *expires;
    StoreItem   *item;

    key = argv[0];
    expires = argv[1];

    lock(store);
    //  UNICODE
    if ((item = mprLookupHash(store->cache, key->value)) == 0) {
        unlock(store);
        return S(false);
    }
    item->lifespan = 0;
    if (expires == S(null)) {
        item->expires = 0;
    } else {
        item->expires = ejsGetDate(ejs, expires);
    }
    unlock(store);
    return S(true);
}


/*
    function get limits(): Object
 */
static EjsPot *sl_limits(Ejs *ejs, EjsLocalStore *store, int argc, EjsObj **argv)
{
    EjsPot      *result;

    result = ejsCreateEmptyPot(ejs);
    ejsSetPropertyByName(ejs, result, EN("keys"), ejsCreateNumber(ejs, store->maxKeys == MAXSSIZE ? 0 : store->maxKeys));
    ejsSetPropertyByName(ejs, result, EN("lifespan"), ejsCreateNumber(ejs, store->lifespan));
    ejsSetPropertyByName(ejs, result, EN("memory"), ejsCreateNumber(ejs, store->maxMem == MAXSSIZE ? 0 : store->maxMem));
    return result;
}


/*
    function read(key: String, options: Object = null): Object
 */
static EjsAny *sl_read(Ejs *ejs, EjsLocalStore *store, int argc, EjsAny **argv)
{
    EjsString   *key;
    EjsAny      *result;
    EjsPot      *options;
    StoreItem   *item;
    int         getVersion;

    key = argv[0];
    getVersion = 0;

    if (argc >= 2 && ejsIsDefined(ejs, argv[1])) {
        options = argv[1];
        getVersion = ejsGetPropertyByName(ejs, options, EN("version")) == S(true);
    }
    lock(store);
    //  UNICODE
    if ((item = mprLookupHash(store->cache, key->value)) == 0) {
        unlock(store);
        return S(null);
    }
    if (item->lifespan) {
        item->expires = mprGetTime() + item->lifespan;
    }
    if (getVersion) {
        result = ejsCreatePot(ejs, S(Object), 2);
        ejsSetPropertyByName(ejs, result, EN("version"), ejsCreateNumber(ejs, item->version));
        ejsSetPropertyByName(ejs, result, EN("data"), item->data);
    } else {
        result = item->data;
    }
    unlock(store);
    return result;
}


/*
    native function remove(key: String): Boolean
 */
static EjsBoolean *sl_remove(Ejs *ejs, EjsLocalStore *store, int argc, EjsAny **argv)
{
    EjsString   *key;
    EjsAny      *result;
    StoreItem   *item;

    key = argv[0];
    lock(store);
    if (ejsIsDefined(ejs, key)) {
        //  UNICODE
        if ((item = mprLookupHash(store->cache, key->value)) != 0) {
            store->usedMem -= (key->length + item->data->length);
            mprRemoveHash(store->cache, key->value);
            result = S(true);
        } else {
            result = S(false);
        }

    } else {
        /* Remove all keys */
        result = mprGetHashLength(store->cache) ? S(true) : S(false);
        store->cache = mprCreateHash(257, 0);
        store->usedMem = 0;
    }
    unlock(store);
    return result;
}


static void setLocalLimits(Ejs *ejs, EjsLocalStore *store, EjsPot *limits)
{
    EjsAny      *vp;

    if ((vp = ejsGetPropertyByName(ejs, limits, EN("keys"))) != 0) {
        store->maxKeys = (ssize) ejsGetInt64(ejs, vp);
        if (store->maxKeys <= 0) {
            store->maxKeys = MAXSSIZE;
        }
    }
    if ((vp = ejsGetPropertyByName(ejs, limits, EN("lifespan"))) != 0) {
        store->lifespan = (ssize) ejsGetInt(ejs, vp);
    }
    if ((vp = ejsGetPropertyByName(ejs, limits, EN("memory"))) != 0) {
        store->maxMem = (ssize) ejsGetInt64(ejs, vp);
        if (store->maxMem <= 0) {
            store->maxMem = MAXSSIZE;
        }
    }
}


/*
    function setLimits(limits: Object): Void
 */
static EjsVoid *sl_setLimits(Ejs *ejs, EjsLocalStore *store, int argc, EjsAny **argv)
{
    setLocalLimits(ejs, store, argv[0]);
    return 0;
}


/*
    function write(key: String~, value: String~, options: Object = null): Number
 */
static EjsNumber *sl_write(Ejs *ejs, EjsLocalStore *store, int argc, EjsAny **argv)
{
    StoreItem   *item;
    EjsString   *key, *value, *sp;
    EjsPot      *options;
    EjsAny      *vp;
    MprTime     expires;
    MprHash     *hp;
    ssize       len, oldLen;
    int64       lifespan, version;
    int         checkVersion, exists, add, set, prepend, append, throw;

    checkVersion = exists = add = prepend = append = throw = 0;
    set = 1;
    expires = 0;
    lifespan = store->lifespan;
    key = argv[0];
    value = argv[1];

    if (argc >= 3 && argv[2] != S(null)) {
        options = argv[2];
        if ((vp = ejsGetPropertyByName(ejs, options, EN("lifespan"))) != 0) {
            lifespan = ejsGetInt64(ejs, vp);
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
    lock(store);
    if ((hp = mprLookupHashEntry(store->cache, key->value)) != 0) {
        exists++;
        item = (StoreItem*) hp->data;
        if (checkVersion) {
            if (item->version != version) {
                if (throw) {
                    ejsThrowStateError(ejs, "Key version does not match");
                } else {
                    return S(null);
                }
                unlock(store);
            }
        }
    } else {
        if ((item = mprAllocObj(StoreItem, manageStoreItem)) == 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
        //  UNICODE
        mprAddKey(store->cache, key->value, item);
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

    store->usedMem += (len - oldLen);

    if (store->timer == 0) {
        store->timer = mprCreateTimerEvent(ejs->dispatcher, "localStoreTimer", STORE_TIMER_PERIOD, localTimer, store, 
            MPR_EVENT_STATIC_DATA); 
    }
    unlock(store);
    //  UNICODE
    return ejsCreateNumber(ejs, len);
}


/*
    Check for expired keys
 */
static void localTimer(EjsLocalStore *store, MprEvent *event)
{
    MprTime         when, factor;
    MprHash         *hp;
    StoreItem       *item;
    ssize           excessKeys;

    if (mprTryLock(store->mutex)) {
        when = mprGetTime();
        for (hp = 0; (hp = mprGetNextHash(store->cache, hp)) != 0; ) {
            item = (StoreItem*) hp->data;
            if (item->expires && item->expires <= when) {
                mprRemoveHash(store->cache, hp->key);
                store->usedMem -= (item->key->length + item->data->length);
            }
        }
        mprAssert(store->usedMem >= 0);

        /*
            If too many keys or too much memory used, prune keys expiring first.
         */
        if (store->maxKeys < MAXSSIZE || store->maxMem < MAXSSIZE) {
            excessKeys = mprGetHashLength(store->cache) - store->maxKeys;
            while (excessKeys > 0 || store->usedMem > store->maxMem) {
                for (factor = 3600; excessKeys > 0; factor *= 2) {
                    for (hp = 0; (hp = mprGetNextHash(store->cache, hp)) != 0; ) {
                        if (item->expires && item->expires <= when) {
                            mprRemoveHash(store->cache, hp->key);
                            store->usedMem -= (item->key->length + item->data->length);
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
        mprAssert(store->usedMem >= 0);

        if (mprGetHashLength(store->cache) == 0) {
            mprRemoveEvent(event);
            store->timer = 0;
        }
        unlock(store);
    }
}


static void manageLocalStore(EjsLocalStore *store, int flags) 
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(store->cache);
        mprMark(store->mutex);
        mprMark(store->timer);
    }
}


static void manageStoreItem(StoreItem *item, int flags) 
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(item->key);
        mprMark(item->data);
    }
}


static int configureLocalTypes(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    type = ejsGetTypeByName(ejs, N("ejs.store.local", "Local"));
    mprAssert(type);
    type->instanceSize = sizeof(EjsLocalStore);
    type->mutableInstances = 1;
    type->manager = (MprManager) manageLocalStore;

    ejsBindConstructor(ejs, type, localConstructor);
    prototype = type->prototype;

    ejsBindMethod(ejs, prototype, ES_ejs_store_local_Local_expire, sl_expire);
    ejsBindMethod(ejs, prototype, ES_ejs_store_local_Local_destroy, sl_destroy);
    ejsBindAccess(ejs, prototype, ES_ejs_store_local_Local_limits, sl_limits, 0);
    ejsBindMethod(ejs, prototype, ES_ejs_store_local_Local_read, sl_read);
    ejsBindMethod(ejs, prototype, ES_ejs_store_local_Local_remove, sl_remove);
    ejsBindMethod(ejs, prototype, ES_ejs_store_local_Local_setLimits, sl_setLimits);
    ejsBindMethod(ejs, prototype, ES_ejs_store_local_Local_write, sl_write);
    return 0;
}


/*
    Module load entry point
 */
int ejs_store_local_Init(Ejs *ejs, MprModule *mp)
{
    return ejsAddNativeModule(ejs, "ejs.store.local", configureLocalTypes, _ES_CHECKSUM_ejs_store_local, EJS_LOADER_ETERNAL);
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
