/**
    ejsSession.c - Native code for the Session class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"
#include    "ejsWeb.h"

/********************************** Forwards  *********************************/

static int getSessionState(Ejs *ejs, EjsSession *sp);

/************************************* Code ***********************************/

static EjsSession *initSession(Ejs *ejs, EjsSession *sp, EjsString *key, MprTicks timeout)
{
    EjsObj      *app;

    app = ejsGetPropertyByName(ejs, ejs->global, N("ejs", "App"));
    if ((sp->cache = ejsGetProperty(ejs, app, ES_App_cache)) == ESV(null)) {
        ejsThrowStateError(ejs, "App.cache is null");
        sp->cache = 0;
        return 0;
    }
    sp->timeout = timeout;
    sp->key = key;
    return sp;
}


static EjsString *makeKey(Ejs *ejs, EjsSession *sp)
{
    char        idBuf[64];
    static int  nextSession = 0;

    /* Thread race here on nextSession++ not critical */
    fmt(idBuf, sizeof(idBuf), "%08x%08x%d", PTOI(ejs) + PTOI(sp), (int) mprGetTime(), nextSession++);
    return ejsCreateStringFromAsc(ejs, mprGetMD5WithPrefix(idBuf, sizeof(idBuf), "::ejs.web.session::"));
}


/*
    Get (create) a session object using the supplied key. If the key has expired or is NULL, then generate a new key if
    create is true. The timeout is in msec.
 */
EjsSession *ejsGetSession(Ejs *ejs, EjsString *key, MprTicks timeout, int create)
{
    EjsSession  *sp;
    EjsType     *type;

    type = ejsGetTypeByName(ejs, N("ejs.web", "Session"));
    if ((sp = ejsCreateObj(ejs, type, 0)) == 0) {
        return 0;
    }
    mprSetName(sp, "session");
    if ((sp = initSession(ejs, sp, key, timeout)) == 0) {
        return 0;
    }
    if (!getSessionState(ejs, sp) && create) {
        sp->key = makeKey(ejs, sp);
    }
    return sp;
}


int ejsDestroySession(Ejs *ejs, EjsSession *sp)
{
    if (sp) {
        ejsCacheRemove(ejs, sp->cache, sp->key);
    }
    return 0;
}


static void manageSession(EjsSession *sp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ejsManagePot(sp, flags);
        mprMark(sp->key);
        mprMark(sp->cache);
        mprMark(sp->options);
    }
}


/*
    Session state is read once and cached. Writes to session properties are cached with write-through
 */
static int getSessionState(Ejs *ejs, EjsSession *sp) 
{
    EjsName     qname;
    EjsObj      *src, *vp;
    int         i, count;

    if (sp->ready) {
        return 1;
    }
    sp->ready = 1;
    if (sp->key && (src = ejsCacheReadObj(ejs, sp->cache, sp->key, 0)) != 0) {
        sp->pot.numProp = 0;
        count = ejsGetLength(ejs, src);
        for (i = 0; i < count; i++) {
            if ((vp = ejsGetProperty(ejs, src, i)) == 0) {
                continue;
            }
            qname = ejsGetPropertyName(ejs, src, i);
            ejs->service->potHelpers.setProperty(ejs, sp, i, vp);
            ejs->service->potHelpers.setPropertyName(ejs, sp, i, qname);
        }
        return 1;
    }
    return 0;
}


static EjsObj *getSessionProperty(Ejs *ejs, EjsSession *sp, int slotNum)
{
    EjsObj  *value;

    getSessionState(ejs, sp);
    value = ejs->service->potHelpers.getProperty(ejs, sp, slotNum);
    if (ejsIs(ejs, value, Void)) {
        /*  Return empty string so that web pages can access session values without having to test for null/undefined */
        value = ESV(empty);
    }
    return value;
}


static EjsObj *getSessionPropertyByName(Ejs *ejs, EjsSession *sp, EjsName qname)
{
    int     slotNum;

    getSessionState(ejs, sp);
    slotNum = ejs->service->potHelpers.lookupProperty(ejs, sp, qname);
    return (slotNum < 0) ? ESV(empty) : ejs->service->potHelpers.getProperty(ejs, sp, slotNum);
}


static int lookupSessionProperty(Ejs *ejs, EjsSession *sp, EjsName qname)
{
    getSessionState(ejs, sp);
    return ejs->service->potHelpers.lookupProperty(ejs, sp, qname);
}


/*
    Set a session property with write-through to the key/value cache
 */
static int setSessionProperty(Ejs *ejs, EjsSession *sp, int slotNum, EjsAny *value)
{
    if (ejs->service->potHelpers.setProperty(ejs, sp, slotNum, value) != slotNum) {
        return EJS_ERR;
    }
    if (sp->options == 0) {
        sp->options = ejsCreateEmptyPot(ejs);
        ejsSetPropertyByName(ejs, sp->options, EN("lifespan"), 
            ejsCreateNumber(ejs, (MprNumber) (sp->timeout / MPR_TICKS_PER_SEC)));
    }
    if (ejsCacheWriteObj(ejs, sp->cache, sp->key, sp, sp->options) == 0) {
        return EJS_ERR;
    }
    return 0;
}


/*
    The timeout arg is a number of ticks to add to the current time
 */
void ejsSetSessionTimeout(Ejs *ejs, EjsSession *sp, MprTicks timeout)
{
    ejsCacheExpire(ejs, sp->cache, sp->key, ejsCreateDate(ejs, mprGetTime() + timeout));
}


/*
    function Session(key: String, options: Object)

    The constructor is bypassed when ejsGetSession is called from Request.
 */
static EjsSession *sess_constructor(Ejs *ejs, EjsSession *sp, int argc, EjsAny **argv)
{
    EjsAny      *vp;
    EjsPot      *options;
    MprTicks    timeout;

    timeout = 0;
    if (argc > 0) {
        options = argv[0];
        vp = ejsGetPropertyByName(ejs, options, EN("lifespan"));
        timeout = ejsGetInt(ejs, vp) * MPR_TICKS_PER_SEC;
    }
    return initSession(ejs, sp, sp->key, timeout);
}


/*
    static function destroySession(session: Session)
 */
static EjsVoid *sess_destroySession(Ejs *ejs, EjsType *Session, int argc, EjsAny **argv)
{
    ejsDestroySession(ejs, argv[0]);
    return 0;
}


/*
    static function key(session: Session): String
 */
static EjsString *sess_key(Ejs *ejs, EjsType *Session, int argc, EjsAny **argv)
{
    EjsSession  *sp;

    sp = argv[0];
    return sp->key;
}


void ejsConfigureSessionType(Ejs *ejs)
{
    EjsType         *type;
    EjsHelpers      *helpers;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs.web", "Session"), sizeof(EjsSession), manageSession, 
            EJS_TYPE_POT | EJS_TYPE_DYNAMIC_INSTANCES)) == 0) {
        return;
    }
    /*
        Sessions are created indirectly by accessing Request.session[] which uses ejsGetSession.
     */
    helpers = &type->helpers;
    helpers->getProperty = (EjsGetPropertyHelper) getSessionProperty;
    helpers->getPropertyByName = (EjsGetPropertyByNameHelper) getSessionPropertyByName;
    helpers->setProperty = (EjsSetPropertyHelper) setSessionProperty;
    helpers->lookupProperty = (EjsLookupPropertyHelper) lookupSessionProperty;

    ejsBindConstructor(ejs, type, sess_constructor);
    ejsBindAccess(ejs, type, ES_ejs_web_Session_destorySession, sess_destroySession, 0);
    ejsBindAccess(ejs, type, ES_ejs_web_Session_key, sess_key, 0);
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
