/**
    ejsSession.c - Native code for the Session class.
    This provides an in-memory, server-local session state store. It is fast, non-durable, non-scalable.

    The Session class serializes objects that are stored to the session object.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"
#include    "ejsWeb.h"
#include    "ejs.web.slots.h"

/*********************************** Forwards *********************************/

static void noteSessionActivity(Ejs *ejs, EjsSession *sp);
static void sessionTimer(Ejs *ejs, MprEvent *event);

/************************************* Code ***********************************/

static EjsVar *getSessionProperty(Ejs *ejs, EjsSession *sp, int slotNum)
{
    EjsVar      *vp;
    Ejs         *master;

    master = ejs->master ? ejs->master : ejs;
    ejsLockVm(master);
    vp = ejs->objectType->helpers.getProperty(ejs, (EjsVar*) sp, slotNum);
    if (vp) {
        vp = ejsDeserialize(ejs, (EjsString*) vp);
    }
    if (vp == ejs->undefinedValue) {
        vp = (EjsVar*) ejs->emptyStringValue;
    }
    noteSessionActivity(ejs, sp);
    ejsUnlockVm(master);
    return vp;
}


static EjsVar *getSessionPropertyByName(Ejs *ejs, EjsSession *sp, EjsName *qname)
{
    EjsVar      *vp;
    Ejs         *master;
    int         slotNum;

    qname->space = EJS_EMPTY_NAMESPACE;
    master = ejs->master ? ejs->master : ejs;
    ejsLockVm(master);

    slotNum = ejs->objectType->helpers.lookupProperty(ejs, (EjsVar*) sp, qname);
    if (slotNum < 0) {
        /*  
            Return empty string so that web pages can access session values without having to test for null/undefined
         */
        vp = (EjsVar*) ejs->emptyStringValue;
    } else {
        vp = ejs->objectType->helpers.getProperty(ejs, (EjsVar*) sp, slotNum);
        if (vp) {
            vp = ejsDeserialize(ejs, (EjsString*) vp);
        }
    }
    noteSessionActivity(ejs, sp);
    ejsUnlockVm(master);
    return vp;
}


static int setSessionProperty(Ejs *ejs, EjsSession *sp, int slotNum, EjsVar *value)
{
    Ejs     *master;
    
    /*  
        Allocate the serialized object using the master interpreter
     */
    master = ejs->master ? ejs->master : ejs;
    ejsLockVm(master);

    value = (EjsVar*) ejsToJSON(master, value, NULL);
    slotNum = master->objectType->helpers.setProperty(master, (EjsVar*) sp, slotNum, value);
    noteSessionActivity(ejs, sp);
    ejsUnlockVm(master);
    return slotNum;
}


/*  
    Update the session expiration time due to activity
 */
static void noteSessionActivity(Ejs *ejs, EjsSession *sp)
{
    sp->expire = mprGetTime(ejs) + sp->timeout * MPR_TICKS_PER_SEC;
}


/*  
    Check for expired sessions
 */
static void sessionTimer(Ejs *ejs, MprEvent *event)
{
    Ejs             *master;
    EjsObj          *sessions;
    EjsSession      *session;
    MprTime         now;
    int             i, count, deleted;

    now = mprGetTime(ejs);

    sessions = ejs->sessions;
    master = ejs->master;
    if (master == 0) {
        mprAssert(master);
        return;
    }

    /*  
        This could be on the primary event thread. Can't block long.
     */
    if (mprTryLock(master->mutex)) {
        count = ejsGetPropertyCount(master, (EjsVar*) sessions);
        deleted = 0;
        for (i = count - 1; i >= 0; i--) {
            session = (EjsSession*) ejsGetProperty(master, (EjsVar*) sessions, i);
            if (session->obj.type == ejs->sessionType) {
                if (session && session->expire <= now) {
                    ejsDeleteProperty(master, (EjsVar*) sessions, i);
                    deleted++;
                }
            }
        }
        if (deleted) {
            ejsCollectGarbage(master, EJS_GEN_NEW);
        }
        if (count == 0) {
            ejs->sessionTimer = 0;
            mprFree(event);
        }
        mprUnlock(master->mutex);
    }
}


#if UNUSED
void ejsParseWebSessionCookie(EjsRequest *req)
{
    EjsName         qname;
    char            *cookie, *id, *cp, *value;
    int             quoted, len;

    cookie = req->cookie;
    while (cookie && (value = strstr(cookie, EJS_SESSION)) != 0) {
        value += strlen(EJS_SESSION);
        while (isspace((int) *value) || *value == '=') {
            value++;
        }
        quoted = 0;
        if (*value == '"') {
            value++;
            quoted++;
        }
        for (cp = value; *cp; cp++) {
            if (quoted) {
                if (*cp == '"' && cp[-1] != '\\') {
                    break;
                }
            } else {
                if ((*cp == ',' || *cp == ';') && cp[-1] != '\\') {
                    break;
                }
            }
        }
        len = cp - value;
        id = mprMemdup(req, value, len + 1);
        id[len] = '\0';

        if (ejs->master) {
            ejsName(&qname, "", id);
            req->session = (EjsSession*) ejsGetPropertyByName(ejs->master, (EjsVar*) ejs->sessions, &qname);
        }
        mprFree(id);
        cookie = value;
    }
}
#endif


/*  
    Create a new session object. This is created in the master interpreter and will persist past the life 
    of the current request. This will allocate a new session ID. Timeout is in seconds.
 */
EjsSession *ejsCreateSession(Ejs *ejs, int timeout, bool secure)
{
    Ejs             *master;
    EjsSession      *session;
    EjsName         qname;
    MprTime         now, expire;
    char            idBuf[64], *id;
    int             slotNum, next;

    master = ejs->master;
    if (master == 0) {
        return 0;
    }
    if (timeout <= 0) {
        timeout = ejs->sessionTimeout;
    }
    now = mprGetTime(ejs);
    expire = now + timeout * MPR_TICKS_PER_SEC;

    ejsLockVm(master);
    session = (EjsSession*) ejsCreateObject(master, ejs->sessionType, 0);
    if (session == 0) {
        ejsUnlockVm(master);
        return 0;
    }
    session->timeout = timeout;
    session->expire = mprGetTime(ejs) + timeout * MPR_TICKS_PER_SEC;
    /*  
        Use an MD5 prefix of "x" to avoid the hash being interpreted as a numeric index.
     */
    next = ejs->nextSession++;
    mprSprintf(idBuf, sizeof(idBuf), "%08x%08x%d", PTOI(ejs) + + PTOI(expire), (int) now, next);
    id = mprGetMD5Hash(session, idBuf, sizeof(idBuf), "x");
    if (id == 0) {
        mprFree(session);
        ejsUnlockVm(master);
        return 0;
    }
    session->id = mprStrdup(session, id);

    slotNum = ejsSetPropertyByName(ejs->master, (EjsVar*) ejs->sessions, EN(&qname, session->id), (EjsVar*) session);
    if (slotNum < 0) {
        mprFree(session);
        ejsUnlockVm(master);
        return 0;
    }
    session->index = slotNum;

    if (ejs->sessionTimer == 0) {
        ejs->sessionTimer = mprCreateTimerEvent(mprGetDispatcher(ejs), "sessionTimer", EJS_TIMER_PERIOD, 
            (MprEventProc) sessionTimer, ejs, MPR_EVENT_CONTINUOUS);
    }
    ejsUnlockVm(master);

    mprLog(ejs, 3, "Created new session %s", id);

#if TODO
    //  TODO - need to set out of band 
    /*  
        Create a cookie that will only live while the browser is not exited. (Set timeout to zero).
     */
    ejsSetCookie(ejs, EJS_SESSION, id, "/", NULL, 0, secure);
#endif
    ejsSendEvent(ejs, ejs->emitter, "createSession", (EjsObj*) session);
    return session;
}


/*  
    Destroy a session. Return true if destroyed. Return 0 if cancelled. 
 */
int ejsDestroySession(Ejs *ejs, EjsSession *session)
{
    EjsName     qname;
    MprTime     now;

    if (session) {
        //  MOB -- but users can't do anything with a bare session. They really need the request object.
        ejsSendEvent(ejs, ejs->emitter, "destroySession", (EjsObj*) session);
        now = mprGetTime(ejs);
        if (session->expire <= now) {
            ejsDeletePropertyByName(ejs->master, (EjsVar*) ejs->sessions, EN(&qname, session->id));
            return 1;
        }
    }
    return 0;
}


/*  
    function addListener(name: [String|Array], listener: Function): Void
 */
static EjsObj *sess_addListener(Ejs *ejs, EjsSession *sp, int argc, EjsObj **argv)
{
    ejsAddListener(ejs, &ejs->emitter, argv[0], argv[1]);
    return 0;
}


/*  
    function get count(): Number
 */
static EjsObj *sess_count(Ejs *ejs, EjsSession *sp, int argc, EjsObj **argv)
{
    return 0;
}


/*  
    function removeListener(name: [String|Array], listener: Function): Void
 */
static EjsObj *sess_removeListener(Ejs *ejs, EjsSession *sp, int argc, EjsObj **argv)
{
    ejsRemoveListener(ejs, ejs->emitter, argv[0], argv[1]);
    return 0;
}


void ejsConfigureSessionType(Ejs *ejs)
{
    EjsType         *type;
    EjsTypeHelpers  *helpers;

    type = ejs->sessionType = ejsConfigureNativeType(ejs, "ejs.web", "Session", sizeof(EjsSession));
    helpers = &type->helpers;
    helpers->getProperty = (EjsGetPropertyHelper) getSessionProperty;
    helpers->getPropertyByName = (EjsGetPropertyByNameHelper) getSessionPropertyByName;
    helpers->setProperty = (EjsSetPropertyHelper) setSessionProperty;

    ejsBindMethod(ejs, type, ES_ejs_web_Session_addListener, (EjsFun) sess_addListener);
    ejsBindMethod(ejs, type, ES_ejs_web_Session_count, (EjsFun) sess_count);
    ejsBindMethod(ejs, type, ES_ejs_web_Session_removeListener, (EjsFun) sess_removeListener);
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
