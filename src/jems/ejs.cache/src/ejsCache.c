/**
    ejsCache.c - Native code for the Cache class.
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"
#include    "ejs.cache.slots.h"

/************************************ Locals **********************************/

typedef struct EjsCache
{
    EjsObj   obj;                /* Base object */
    EjsObj   *cache;             /* Cache object */
    int         timeout;            /* Session inactivity lifespan */
} EjsCache;

/*********************************** Forwards *********************************/

#if FUTURE
static void cacheActivity(Ejs *ejs, EjsCache *sp);
static void cacheTimer(EjsWebControl *control, MprEvent *event);
#endif

/************************************* Code ***********************************/
 
static EjsVar *cacheConstructor(Ejs *ejs, EjsCache *cp, int argc, EjsVar **argv)
{
    Ejs         *master;
    EjsType     *cacheType;

    master = ejs->master ? ejs->master : ejs;

    cacheType = ejsGetTypeByName(master, "ejs.cache", "Cache");
    if (cacheType == 0) {
        ejsThrowTypeError(ejs, "Can't find Cache type");
        return 0;
    }
#if ES_ejs_cache_Cache_Cache
    cp->cache = ejsGetProperty(master, (EjsVar*) cacheType, ES_ejs_cache_Cache_cache);
#endif
    return 0;
}


static EjsVar *readCache(Ejs *ejs, EjsCache *cp, int argc, EjsVar **argv)
{
    Ejs         *master;
    EjsName     qname;
    EjsVar      *vp;
    cchar       *domain, *key;

    master = ejs->master ? ejs->master : ejs;
    domain = ejsGetString(ejs, argv[0]);
    key = ejsGetString(ejs, argv[1]);
    ejsLockVm(master);

    vp = ejsGetPropertyByName(master, cp->cache, ejsName(&qname, domain, key));
    if (vp == 0) {
        ejsUnlockVm(master);
        return ejs->nullValue;
    }
    vp = ejsDeserialize(ejs, (EjsString*) vp);
    if (vp == ejs->undefinedValue) {
        vp = (EjsVar*) ejs->emptyStringValue;
    }
    ejsUnlockVm(master);
    return vp;
}


static EjsVar *removeCache(Ejs *ejs, EjsCache *cp, int argc, EjsVar **argv)
{
    Ejs         *master;
    EjsName     qname;
    cchar       *domain, *key;

    master = ejs->master ? ejs->master : ejs;
    domain = ejsGetString(ejs, argv[0]);
    key = ejsGetString(ejs, argv[1]);

    ejsLockVm(master);
    ejsDeletePropertyByName(master, cp->cache, ejsName(&qname, domain, key));
    ejsUnlockVm(master);
    return 0;
}


static EjsVar *writeCache(Ejs *ejs, EjsCache *cp, int argc, EjsVar **argv)
{
    Ejs         *master;
    EjsName     qname;
    EjsVar      *value;
    cchar       *domain, *key;

    master = ejs->master ? ejs->master : ejs;
    domain = ejsGetString(ejs, argv[0]);
    key = ejsGetString(ejs, argv[1]);
    ejsLockVm(master);

    value = (EjsVar*) ejsToJSON(master, argv[2], NULL);
    ejsSetPropertyByName(master, cp->cache, ejsName(&qname, domain, key), value);
#if FUTURE
    if (argc == 4) {
        //  TODO - store the timeout somehow.
    }
#endif
    ejsUnlockVm(master);
    return 0;
}


#if FUTURE
static void cacheActivity(Ejs *ejs, EjsCache *sp)
{
    sp->expire = mprGetTime(ejs) + sp->timeout   MPR_TICKS_PER_SEC;
}


/*
    Check for expired caches
 */
static void cacheTimer(EjsWebControl *control, MprEvent *event)
{
    Ejs             *master;
    EjsObj       *caches;
    EjsCache   *cache;
    MprTime         now;
    int             i, count, deleted;

    now = mprGetTime(control);

    caches = control->caches;
    master = control->master;
    if (master == 0) {
        mprAssert(master);
        return;
    }

    /*
        This could be on the primary event thread. Can't block long.
     */
    if (mprTryLock(master->mutex)) {
        count = ejsGetPropertyCount(master, (EjsVar*) caches);
        deleted = 0;
        for (i = count - 1; i >= 0; i--) {
            cache = (EjsCache*) ejsGetProperty(master, (EjsVar*) caches, i);
            if (cache->obj.var.type == control->cacheType) {
                if (cache && cache->expire <= now) {
                    ejsDeleteProperty(master, (EjsVar*) caches, i);
                    deleted++;
                }
            }
        }
        if (deleted) {
            ejsCollectGarbage(master, EJS_GEN_NEW);
        }
        if (count == 0) {
            control->cacheTimer = 0;
            mprFree(event);
        }
        mprUnlock(master->mutex);
    }
}
#endif


static int configureCacheType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *prototype;

    type = ejsConfigureNativeType(ejs, "ejs.cache", "Cache", sizeof(EjsCache));

    ejsBindConstructor(ejs, type, (EjsProc) cacheConstructor);
    prototype = type->prototype;
    ejsBindMethod(ejs, prototype, ES_ejs_cache_Cache_read, (EjsProc) readCache);
    ejsBindMethod(ejs, prototype, ES_ejs_cache_Cache_write, (EjsProc) writeCache);
    ejsBindMethod(ejs, prototype, ES_ejs_cache_Cache_remove, (EjsProc) removeCache);
    return 0;
}


/*
    Module load entry point
 */
int ejs_cache_Init(MprCtx ctx)
{
    return ejsAddNativeModule(ctx, "ejs.cache", configureCacheType, _ES_CHECKSUM_ejs_cache, EJS_LOADER_ETERNAL);
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
