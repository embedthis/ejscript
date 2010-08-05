/**
    ejsSession.c - Native code for the Session class.
    This provides an in-memory, server-local session state store. It is fast, non-durable, non-scalable.

    The Session class serializes objects that are stored to the session object so that they can be accessed safely 
    from multiple interpreters.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"
#include    "ejsWeb.h"
#include    "ejs.web.slots.h"

/*********************************** Forwards *********************************/

static void noteSessionActivity(Ejs *ejs, EjsSession *sp);
static void sessionTimer(EjsHttpServer *sp, MprEvent *event);

/************************************* Code ***********************************/

static EjsObj *getSessionProperty(Ejs *ejs, EjsSession *sp, int slotNum)
{
    EjsObj      *vp;
    Ejs         *master;

    master = ejs->master ? ejs->master : ejs;
    ejsLockVm(master);
    vp = ejs->objectType->helpers.getProperty(ejs, (EjsObj*) sp, slotNum);
    if (vp) {
        vp = ejsDeserialize(ejs, (EjsString*) vp);
    }
    if (vp == ejs->undefinedValue) {
        vp = (EjsObj*) ejs->emptyStringValue;
    }
    noteSessionActivity(ejs, sp);
    ejsUnlockVm(master);
    return vp;
}


static EjsObj *getSessionPropertyByName(Ejs *ejs, EjsSession *sp, EjsName *qname)
{
    EjsObj      *vp;
    Ejs         *master;
    int         slotNum;

    qname->space = EJS_EMPTY_NAMESPACE;
    master = ejs->master ? ejs->master : ejs;
    ejsLockVm(master);

    slotNum = ejs->objectType->helpers.lookupProperty(ejs, (EjsObj*) sp, qname);
    if (slotNum < 0) {
        /*  
            Return empty string so that web pages can access session values without having to test for null/undefined
         */
        vp = (EjsObj*) ejs->emptyStringValue;
    } else {
        vp = ejs->objectType->helpers.getProperty(ejs, (EjsObj*) sp, slotNum);
        if (vp) {
            vp = ejsDeserialize(ejs, (EjsString*) vp);
        }
    }
    noteSessionActivity(ejs, sp);
    ejsUnlockVm(master);
    return vp;
}


static int setSessionProperty(Ejs *ejs, EjsSession *sp, int slotNum, EjsObj *value)
{
    Ejs     *master;
    
    /*  
        Allocate the serialized object using the master interpreter
     */
    master = ejs->master ? ejs->master : ejs;
    ejsLockVm(master);

    value = (EjsObj*) ejsToJSON(master, value, NULL);
    slotNum = master->objectType->helpers.setProperty(master, (EjsObj*) sp, slotNum, value);
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


void ejsSetSessionTimeout(Ejs *ejs, EjsSession *sp, int timeout)
{
    sp->expire = mprGetTime(ejs) + timeout;
}


void ejsUpdateSessionLimits(Ejs *ejs, EjsHttpServer *server)
{
    EjsSession  *session;
    MprTime     now;
    int         i, count, timeout;

    if (server->sessions) {
        timeout = server->server->limits->sessionTimeout;
        now = mprGetTime(ejs);
        count = ejsGetPropertyCount(ejs, (EjsObj*) server->sessions);
        for (i = count - 1; i >= 0; i--) {
            session = ejsGetProperty(ejs, (EjsObj*) server->sessions, i);
            session->expire = now + timeout;
        }
    }
}


EjsSession *ejsGetSession(Ejs *ejs, EjsRequest *req)
{
    Ejs             *master;
    EjsName         qname;
    EjsSession      *session;
    EjsHttpServer   *server;
    cchar           *cookies, *cookie;
    char            *id, *cp, *value;
    int             quoted, len;

    master = ejs->master ? ejs->master : ejs;
    server = req->server;
    session = 0;

    if (server->sessions) {
        cookies = httpGetCookies(req->conn);
        for (cookie = cookies; cookie && (value = strstr(cookie, EJS_SESSION)) != 0; cookie = value) {
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
            session = ejsGetPropertyByName(master, server->sessions, ejsName(&qname, "", id));
            mprFree(id);
            break;
        }
    }
    return session;
}


/*  
    Create a new session object. This is created in the master interpreter and will persist past the life 
    of the current request. This will allocate a new session ID. Timeout is in seconds.
 */
EjsSession *ejsCreateSession(Ejs *ejs, EjsRequest *req, int timeout, bool secure)
{
    Ejs             *master;
    EjsSession      *session;
    EjsName         qname;
    EjsHttpServer   *server;
    HttpLimits      *limits;
    MprTime         now;
    char            idBuf[64], *id;
    int             count, slotNum, next;

    master = ejs->master ? ejs->master : ejs;
    server = req->server;
    limits = server->server->limits;

    if (timeout <= 0) {
        timeout = limits->sessionTimeout;
    }
    now = mprGetTime(ejs);

    ejsLockVm(master);
    session = (EjsSession*) ejsCreateObject(master, ejs->sessionType, 0);
    if (session == 0) {
        ejsUnlockVm(master);
        return 0;
    }
    session->timeout = timeout;
    session->expire = now + timeout;
    /*  
        Use an MD5 prefix of "x" to avoid the hash being interpreted as a numeric index.
     */
    next = ejs->nextSession++;
    mprSprintf(ejs, idBuf, sizeof(idBuf), "%08x%08x%d", PTOI(ejs) + PTOI(session->expire), (int) now, next);
    id = mprGetMD5Hash(session, idBuf, sizeof(idBuf), "x");
    if (id == 0) {
        mprFree(session);
        ejsUnlockVm(master);
        return 0;
    }
    session->id = mprStrdup(session, id);

    if (server->sessions == 0) {
        server->sessions = ejsCreateSimpleObject(ejs);
    }
    count = ejsGetPropertyCount(ejs, (EjsObj*) server->sessions);
    if (count >= limits->sessionCount) {
        ejsThrowResourceError(ejs, "Too many sessions: %d", count);
        mprFree(session);
        ejsUnlockVm(master);
        return 0;
    }
    slotNum = ejsSetPropertyByName(master, server->sessions, EN(&qname, session->id), session);
    if (slotNum < 0) {
        mprFree(session);
        ejsUnlockVm(master);
        return 0;
    }
    session->index = slotNum;

    if (server->sessionTimer == 0) {
        server->sessionTimer = mprCreateTimerEvent(ejs->dispatcher, "sessionTimer", EJS_TIMER_PERIOD, 
            (MprEventProc) sessionTimer, server, MPR_EVENT_CONTINUOUS);
    }
    ejsUnlockVm(master);

    mprLog(ejs, 3, "Created new session %s. Count %d/%d", id, slotNum + 1, limits->sessionCount);
    if (server->emitter) {
        ejsSendEvent(ejs, server->emitter, "createSession", NULL, (EjsObj*) ejsCreateString(ejs, id));
    }
    return session;
}


/*  
    Destroy a session. Return true if destroyed. Return 0 if cancelled. 
 */
int ejsDestroySession(Ejs *ejs, EjsHttpServer *server, EjsSession *session)
{
    Ejs         *master;
    EjsName     qname;
    int         slotNum;

    if (session && server->sessions) {
        master = ejs->master ? ejs->master : ejs;
        if (server) {
            ejsSendEvent(ejs, server->emitter, "destroySession", NULL, (EjsObj*) ejsCreateString(ejs, session->id));
        }
        if ((slotNum = ejsLookupProperty(master, server->sessions, EN(&qname, session->id))) >= 0) {
            ejsDeleteProperty(master, server->sessions, slotNum);
            return 1;
        }
    }
    return 0;
}


#if UNUSED
/*  
    function get count(): Number
 */
static EjsObj *sess_count(Ejs *ejs, EjsSession *sp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, ejsGetPropertyCount(ejs, ejs->sessions));
}
#endif


/*  
    Check for expired sessions
 */
static void sessionTimer(EjsHttpServer *server, MprEvent *event)
{
    Ejs             *ejs;
    EjsObj          *sessions;
    EjsSession      *session;
    MprTime         now;
    HttpLimits      *limits;
    int             i, count, soon, redline;

    sessions = server->sessions;
    ejs = server->ejs;

    /*  
        This could be on the primary event thread. Can't block long.  MOB -- is this lock really needed
     */
    if (sessions && mprTryLock(ejs->mutex)) {
        limits = server->server->limits;
        count = ejsGetPropertyCount(ejs, (EjsObj*) sessions);
        mprLog(ejs, 6, "Check for sessions count %d/%d", count, limits->sessionCount);
        now = mprGetTime(server);

        /*
            Start pruning at 80% of the max
         */
        redline = limits->sessionCount * 8 / 10;
        if (count > redline) {
            /* Expire oldest sessions */
            /*
                One quick swipe to find sessions that are 80% expired
             */
            soon = limits->sessionTimeout / 5;
            for (i = count - 1; i >= 0; i--) {
                if ((session = ejsGetProperty(ejs, (EjsObj*) sessions, i)) == 0) {
                    continue;
                }
                printf("CMP %d with %d\n", (int) ((session->expire - now) / 1000), ((int) soon / 1000));
                if ((session->expire - now) < soon) {
                    mprLog(ejs, 3, "Too many sessions. Pruning session %s", session->id);
                    ejsDeleteProperty(ejs, (EjsObj*) sessions, i);
                    count--;
                }
            }
        }
        for (i = count - 1; i >= 0; i--) {
            if ((session = ejsGetProperty(ejs, (EjsObj*) sessions, i)) == 0) {
                continue;
            }
            if (session->obj.type == ejs->sessionType) {
                mprLog(ejs, 7, "Check session %s timeout %d, expires %d secs", session->id, 
                   session->timeout / MPR_TICKS_PER_SEC,
                   (int) (session->expire - now) / MPR_TICKS_PER_SEC);
                if (count > limits->sessionCount) {
                    mprLog(ejs, 3, "Too many sessions. Pruning session %s", session->id);
                    ejsDeleteProperty(ejs, (EjsObj*) sessions, i);
                    count--;
                }  
                if (session->expire <= now) {
                    mprLog(ejs, 3, "Session expired: %s (timeout %d secs)", 
                        session->id, session->timeout / MPR_TICKS_PER_SEC);
                    ejsDeleteProperty(ejs, (EjsObj*) sessions, i);
                }
            }
        }
        count = ejsCompactObject(ejs, sessions);
        if (count == 0) {
            server->sessionTimer = 0;
            mprFree(event);
        }
        mprUnlock(ejs->mutex);
    }
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

#if UNUSED
    ejsBindMethod(ejs, type, ES_ejs_web_Session_count, (EjsFun) sess_count);
#endif
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
