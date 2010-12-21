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
    int      timeout;            /* Session inactivity lifespan */
} EjsCache;

/*********************************** Forwards *********************************/

#if FUTURE
static void cacheActivity(Ejs *ejs, EjsCache *sp);
static void cacheTimer(EjsWebControl *control, MprEvent *event);
#endif

/************************************* Code ***********************************/
 
static EjsObj *cacheConstructor(Ejs *ejs, EjsCache *cp, int argc, EjsObj **argv)
{
    EjsType     *cacheType;

    cacheType = ejsGetTypeByName(ejs, "ejs.cache", "Cache");
    if (cacheType == 0) {
        ejsThrowTypeError(ejs, "Can't find Cache type");
        return 0;
    }
    cp->cache = ejsGetProperty(ejs, (EjsObj*) cacheType, ES_ejs_cache_Cache_cache);
    return 0;
}


static EjsObj *readCache(Ejs *ejs, EjsCache *cp, int argc, EjsObj **argv)
{
    EjsName     qname;
    EjsObj      *vp;
    cchar       *domain, *key;

    domain = ejsGetString(ejs, argv[0]);
    key = ejsGetString(ejs, argv[1]);
    ejsLockVm(ejs);

    vp = ejsGetPropertyByName(ejs, cp->cache, ejsName(&qname, domain, key));
    if (vp == 0) {
        ejsUnlockVm(ejs);
        return ejs->nullValue;
    }
    vp = ejsDeserialize(ejs, (EjsString*) vp);
    if (vp == ejs->undefinedValue) {
        vp = (EjsObj*) ejs->emptyStringValue;
    }
    ejsUnlockVm(ejs);
    return vp;
}


static EjsObj *removeCache(Ejs *ejs, EjsCache *cp, int argc, EjsObj **argv)
{
    EjsName     qname;
    cchar       *domain, *key;

    domain = ejsGetString(ejs, argv[0]);
    key = ejsGetString(ejs, argv[1]);

    ejsLockVm(ejs);
    ejsDeletePropertyByName(ejs, cp->cache, ejsName(&qname, domain, key));
    ejsUnlockVm(ejs);
    return 0;
}


static EjsObj *writeCache(Ejs *ejs, EjsCache *cp, int argc, EjsObj **argv)
{
    EjsName     qname;
    EjsObj      *value;
    cchar       *domain, *key;

    domain = ejsGetString(ejs, argv[0]);
    key = ejsGetString(ejs, argv[1]);
    ejsLockVm(ejs);

    value = (EjsObj*) ejsToJSON(ejs, argv[2], NULL);
    ejsSetPropertyByName(ejs, cp->cache, ejsName(&qname, domain, key), value);
#if FUTURE
    if (argc == 4) {
        //  TODO - store the timeout somehow.
    }
#endif
    ejsUnlockVm(ejs);
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
    EjsObj     *caches;
    EjsCache   *cache;
    MprTime    now;
    int        i, count, deleted;

    now = mprGetTime(control);

    caches = control->caches;
    ejs = control->master;
    if (master == 0) {
        mprAssert(master);
        return;
    }

    /*
        This could be on the primary event thread. Can't block long.
     */
    if (mprTryLock(master->mutex)) {
        count = ejsGetPropertyCount(master, (EjsObj*) caches);
        deleted = 0;
        for (i = count - 1; i >= 0; i--) {
            cache = ejsGetProperty(master, (EjsObj*) caches, i);
            if (cache->obj.var.type == control->cacheType) {
                if (cache && cache->expire <= now) {
                    ejsDeleteProperty(master, (EjsObj*) caches, i);
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

    type = ejsConfigureNativeType(ejs, "ejs.cache", "Cache", sizeof(EjsCache), manageCache, EJS_DEFAULT_HELPERS);

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
