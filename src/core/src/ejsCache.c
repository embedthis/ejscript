/**
    ejsCache.c - API for the Cache class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************* Code ***********************************/

PUBLIC EjsAny *ejsCacheExpire(Ejs *ejs, EjsObj *cache, EjsString *key, EjsDate *when)
{
    EjsAny  *argv[3];
    int     prior;

    prior = ejsBlockGC(ejs);
    argv[0] = key;
    argv[1] = when;
    ejsRunFunctionBySlot(ejs, cache, ES_Cache_expire, 2, argv);
    ejsUnblockGC(ejs, prior);
    return 0;
}


PUBLIC EjsAny *ejsCacheRead(Ejs *ejs, EjsObj *cache, EjsString *key, EjsObj *options)
{
    EjsAny  *argv[3], *result;
    int     prior;

    prior = ejsBlockGC(ejs);
    argv[0] = key;
    argv[1] = (options) ? options : ESV(null);
    result = ejsRunFunctionBySlot(ejs, cache, ES_Cache_read, 2, argv);
    ejsUnblockGC(ejs, prior);
    return result;
}


PUBLIC EjsAny *ejsCacheReadObj(Ejs *ejs, EjsObj *cache, EjsString *key, EjsObj *options)
{
    EjsAny  *value, *argv[3], *result;
    int     prior;

    prior = ejsBlockGC(ejs);
    argv[0] = key;
    argv[1] = (options) ? options : ESV(null);
    if ((value = ejsRunFunctionBySlot(ejs, cache, ES_Cache_read, 2, argv)) == 0 || value == ESV(null)) {
        return 0;
    }
    result = ejsDeserialize(ejs, value);
    ejsUnblockGC(ejs, prior);
    return result;
}


PUBLIC EjsBoolean *ejsCacheRemove(Ejs *ejs, EjsObj *cache, EjsString *key)
{
    EjsAny  *argv[3], *result;
    int     prior;

    prior = ejsBlockGC(ejs);
    argv[0] = key;
    result = ejsRunFunctionBySlot(ejs, cache, ES_Cache_remove, 1, argv);
    ejsUnblockGC(ejs, prior);
    return result;
}


PUBLIC EjsAny *ejsCacheSetLimits(Ejs *ejs, EjsObj *cache, EjsObj *limits)
{
    EjsAny  *argv[2], *result;
    int     prior;

    prior = ejsBlockGC(ejs);
    argv[0] = limits;
    result = ejsRunFunctionBySlot(ejs, cache, ES_Cache_setLimits, 2, argv);
    ejsUnblockGC(ejs, prior);
    return result;
}


PUBLIC EjsNumber *ejsCacheWrite(Ejs *ejs, EjsObj *cache, EjsString *key, EjsString *value, EjsObj *options)
{
    EjsAny  *argv[3], *result;
    int     prior;

    prior = ejsBlockGC(ejs);
    argv[0] = key;
    argv[1] = value;
    argv[2] = (options) ? options : ESV(null);
    result = ejsRunFunctionBySlot(ejs, cache, ES_Cache_write, 3, argv);
    ejsUnblockGC(ejs, prior);
    return result;
}


PUBLIC EjsNumber *ejsCacheWriteObj(Ejs *ejs, EjsObj *cache, EjsString *key, EjsAny *value, EjsObj *options)
{
    EjsAny  *argv[3], *result;
    int     prior;

    prior = ejsBlockGC(ejs);
    argv[0] = key;
    argv[1] = ejsSerialize(ejs, value, 0);
    argv[2] = (options) ? options : ESV(null);
    result = ejsRunFunctionBySlot(ejs, cache, ES_Cache_write, 3, argv);
    ejsUnblockGC(ejs, prior);
    return result;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
