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

static EjsSession *initSession(Ejs *ejs, EjsSession *sp, EjsString *key, MprTime timeout)
{
    EjsObj      *app;

    app = ejsGetPropertyByName(ejs, ejs->global, N("ejs", "App"));
    sp->store = ejsGetProperty(ejs, app, ES_App_store);

    sp->lifespan = timeout;
    sp->key = key;
    return sp;
}


static EjsString *makeKey(Ejs *ejs, EjsSession *sp)
{
    char        idBuf[64];
    static int  nextSession = 0;

    /* Thread race here on nextSession++ not critical */
    mprSprintf(idBuf, sizeof(idBuf), "%08x%08x%d", PTOI(ejs) + PTOI(sp), (int) mprGetTime(), nextSession++);
    return ejsCreateStringFromAsc(ejs, mprGetMD5Hash(idBuf, sizeof(idBuf), "::ejs.web.session::"));
}


/*
    Get (create) a session object using the supplied key. If the key has expired or is NULL, then generate a new key if
    create is true.
 */
EjsSession *ejsGetSession(Ejs *ejs, EjsString *key, MprTime timeout, int create)
{
    EjsSession  *sp;

    if ((sp = ejsCreateObj(ejs, ST(Session), 0)) == 0) {
        return 0;
    }
    mprSetName(sp, "session");
    initSession(ejs, sp, key, timeout);
    if (!getSessionState(ejs, sp) && create) {
        sp->key = makeKey(ejs, sp);
    }
    return sp;
}


int ejsDestroySession(Ejs *ejs, EjsSession *sp)
{
    if (sp) {
        ejsStoreRemove(ejs, sp->store, sp->key);
    }
    return 0;
}


static void manageSession(EjsSession *sp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ejsManagePot(sp, flags);
        mprMark(sp->key);
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
    if (sp->key && (src = ejsStoreReadObj(ejs, sp->store, sp->key, 0)) != 0) {
        sp->pot.numProp = 0;
        count = ejsGetPropertyCount(ejs, src);
        for (i = 0; i < count; i++) {
            if ((vp = ejsGetProperty(ejs, src, i)) == 0) {
                continue;
            }
            qname = ejsGetPropertyName(ejs, src, i);
            ejs->potHelpers.setProperty(ejs, sp, i, vp);
            ejs->potHelpers.setPropertyName(ejs, sp, i, qname);
        }
        return 1;
    }
    return 0;
}


static EjsObj *getSessionProperty(Ejs *ejs, EjsSession *sp, int slotNum)
{
    EjsObj  *value;

    getSessionState(ejs, sp);
    value = ejs->potHelpers.getProperty(ejs, sp, slotNum);
    if (ejsIs(ejs, value, Void)) {
        /*  Return empty string so that web pages can access session values without having to test for null/undefined */
        value = S(empty);
    }
    return value;
}


static EjsObj *getSessionPropertyByName(Ejs *ejs, EjsSession *sp, EjsName qname)
{
    int     slotNum;

    getSessionState(ejs, sp);
    slotNum = ejs->potHelpers.lookupProperty(ejs, sp, qname);
    return (slotNum < 0) ? S(empty) : ejs->potHelpers.getProperty(ejs, sp, slotNum);
}


static int lookupSessionProperty(Ejs *ejs, EjsSession *sp, EjsName qname)
{
    getSessionState(ejs, sp);
    return ejs->potHelpers.lookupProperty(ejs, sp, qname);
}


/*
    Set a session property with write-through to the key/value store
 */
static int setSessionProperty(Ejs *ejs, EjsSession *sp, int slotNum, EjsAny *value)
{
    if (ejs->potHelpers.setProperty(ejs, sp, slotNum, value) != slotNum) {
        return EJS_ERR;
    }
    if (sp->options == 0) {
        sp->options = ejsCreateEmptyPot(ejs);
        ejsSetPropertyByName(ejs, sp->options, EN("lifespan"), ejsCreateNumber(ejs, sp->lifespan));
    }
    if (ejsStoreWriteObj(ejs, sp->store, sp->key, sp, sp->options) == 0) {
        return EJS_ERR;
    }
    return 0;
}


void ejsSetSessionTimeout(Ejs *ejs, EjsSession *sp, int timeout)
{
    ejsStoreExpire(ejs, sp->store, sp->key, ejsCreateDate(ejs, mprGetTime() + timeout));
}


/*
    function Session(key: String, options: Object)

    The constructor is bypassed when ejsGetSession is called from Request.
 */
static EjsSession *sess_constructor(Ejs *ejs, EjsSession *sp, int argc, EjsAny **argv)
{
    EjsAny      *vp;
    EjsPot      *options;
    MprTime     lifespan;

    lifespan = 0;
    if (argc > 0) {
        options = argv[0];
        vp = ejsGetPropertyByName(ejs, options, EN("lifespan"));
        lifespan = ejsGetInt(ejs, vp);
    }
    return initSession(ejs, sp, sp->key, lifespan);
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

    type = ejsConfigureNativeType(ejs, N("ejs.web", "Session"), sizeof(EjsSession), manageSession, EJS_POT_HELPERS);
    ejsSetSpecialType(ejs, S_Session, type);
    mprAssert(type->mutex == 0);

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
