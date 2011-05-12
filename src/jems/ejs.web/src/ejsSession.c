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

/*********************************** Locals ***********************************/

static MprMutex     *sessionLock;
static EjsPot       *sessions;             /**< In-memory session cache */
static MprEvent     *sessionEvent;         /**< Session expiry event timer */

/*********************************** Forwards *********************************/

static void sessionActivity(Ejs *ejs, EjsSession *sp);
static void sessionTimer(EjsHttpServer *sp, MprEvent *event);
static void startSessionTimer(Ejs *ejs, EjsHttpServer *server);

/************************************* Code ***********************************/

static EjsObj *getSessionProperty(Ejs *ejs, EjsSession *sp, int slotNum)
{
    EjsObj      *vp;

    mprLock(sessionLock);
    vp = ejs->potHelpers.getProperty(ejs, sp, slotNum);
    if (vp) {
        vp = ejsDeserialize(ejs, (EjsString*) vp);
    }
    if (ejsIs(ejs, vp, Void)) {
        vp = S(empty);
    }
    sessionActivity(ejs, sp);
    mprUnlock(sessionLock);
    return vp;
}


static EjsObj *getSessionPropertyByName(Ejs *ejs, EjsSession *sp, EjsName qname)
{
    EjsObj      *vp;
    int         slotNum;

    mprLock(sessionLock);
    qname.space = S(empty);
    slotNum = ejs->potHelpers.lookupProperty(ejs, sp, qname);
    if (slotNum < 0) {
        /*  
            Return empty string so that web pages can access session values without having to test for null/undefined
         */
        vp = S(empty);
    } else {
        vp = ejs->potHelpers.getProperty(ejs, sp, slotNum);
        if (vp) {
            vp = ejsDeserialize(ejs, (EjsString*) vp);
        }
    }
    sessionActivity(ejs, sp);
    mprUnlock(sessionLock);
    return vp;
}


static int setSessionProperty(Ejs *ejs, EjsSession *sp, int slotNum, EjsObj *value)
{
    mprLock(sessionLock);
    value = (EjsObj*) ejsToJSON(ejs, value, NULL);
    slotNum = ejs->potHelpers.setProperty(ejs, sp, slotNum, value);
    sessionActivity(ejs, sp);
    mprUnlock(sessionLock);
    return slotNum;
}


static void sessionActivity(Ejs *ejs, EjsSession *sp)
{
    /*  
        Update the session expiration time due to activity
     */
    sp->expire = mprGetTime() + sp->timeout;
}


void ejsSetSessionTimeout(Ejs *ejs, EjsSession *sp, int timeout)
{
    sp->expire = mprGetTime() + timeout;
}


void ejsUpdateSessionLimits(Ejs *ejs, EjsHttpServer *server)
{
    EjsSession  *session;
    MprTime     now;
    int         i, count, timeout;

    if (sessions && server->server) {
        timeout = server->server->limits->sessionTimeout;
        now = mprGetTime();
        mprLock(sessionLock);
        count = ejsGetPropertyCount(ejs, sessions);
        for (i = count - 1; i >= 0; i--) {
            session = ejsGetProperty(ejs, sessions, i);
            session->expire = now + timeout;
        }
        mprUnlock(sessionLock);
    }
}


/*
    Return the session object corresponding to a request cookie. Note the Session class on which session instances are
    based, defines helpers for all accesses to session data objects.
 */
EjsSession *ejsGetSession(Ejs *ejs, EjsRequest *req)
{
    EjsSession      *session;
    EjsHttpServer   *server;
    cchar           *cookies, *cookie;
    char            *id, *cp, *value;
    int             quoted, len;

    server = req->server;
    session = 0;

    mprLock(sessionLock);
    if (server && sessions) {
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
            len = (int) (cp - value);
            id = mprMemdup(value, len + 1);
            id[len] = '\0';
            session = ejsGetPropertyByName(ejs, sessions, EN(id));
            break;
        }
    }
    mprUnlock(sessionLock);
    return session;
}


/*  
    Create a new session object.  This will also allocate a new session ID. Timeout is in msec.
 */
EjsSession *ejsCreateSession(Ejs *ejs, EjsRequest *req, int timeout, bool secure)
{
    EjsSession      *session;
    EjsHttpServer   *server;
    HttpLimits      *limits;
    MprTime         now;
    char            idBuf[64], *id;
    int             count, slotNum, next;
    static int      nextSession = 0;

    if ((server = req->server) == 0) {
        return 0;
    }
    if (server->cloned) {
        server = server->cloned;
        ejs = server->ejs;
    }
    now = mprGetTime();

    mprLock(sessionLock);
    limits = server->server->limits;
    if (timeout <= 0) {
        timeout = limits->sessionTimeout;
    }
    if ((session = ejsCreateObj(ejs, ST(Session), 0)) == 0) {
        mprUnlock(sessionLock);
        return 0;
    }
    session->timeout = timeout;
    session->expire = now + timeout;

    /*  
        Use an MD5 prefix of "x" to avoid the hash being interpreted as a numeric index.
     */
    next = nextSession++;
    mprSprintf(idBuf, sizeof(idBuf), "%08x%08x%d", PTOI(ejs) + PTOI(session->expire), (int) now, next);
    id = mprGetMD5Hash(idBuf, sizeof(idBuf), "x");
    if (id == 0) {
        mprUnlock(sessionLock);
        return 0;
    }
    session->id = sclone(id);

    if (sessions == 0) {
        sessions = ejsCreateEmptyPot(ejs);
#if UNUSED
        ejsSetProperty(ejs, server, ES_ejs_web_HttpServer_sessions, sessions);
#endif
    }
    count = ejsGetPropertyCount(ejs, sessions);
    if (count >= limits->sessionCount) {
        mprWarn("Too many sessions: %d, limit %d", count, limits->sessionCount);
    }
    slotNum = ejsSetPropertyByName(ejs, sessions, EN(session->id), session);
    if (slotNum < 0) {
        mprUnlock(sessionLock);
        return 0;
    }
    session->index = slotNum;
    startSessionTimer(ejs, server);
    mprUnlock(sessionLock);

    /*
        Session created event issued on the non-cloned vm
     */
    mprLog(3, "Created new session %s. Count %d/%d", id, slotNum + 1, limits->sessionCount);
    if (server->emitter) {
        ejsSendEvent(ejs, server->emitter, "createSession", NULL, ejsCreateStringFromAsc(ejs, id));
    }
    return session;
}


int ejsDestroySession(Ejs *ejs, EjsHttpServer *server, EjsSession *session)
{
    int     slotNum;

    mprLock(sessionLock);
    if (session && sessions) {
        if (server) {
            ejsSendEvent(ejs, server->emitter, "destroySession", NULL, ejsCreateStringFromAsc(ejs, session->id));
        }
        if ((slotNum = ejsLookupProperty(ejs, sessions, EN(session->id))) >= 0) {
            ejsDeleteProperty(ejs, sessions, slotNum);
            mprUnlock(sessionLock);
            return 1;
        }
    }
    mprUnlock(sessionLock);
    return 0;
}


static void startSessionTimer(Ejs *ejs, EjsHttpServer *server)
{
    mprLock(sessionLock);
#if UNUSED
    if (server->sessionTimer == 0) {
        server->sessionTimer = mprCreateTimerEvent(ejs->dispatcher, "sessionTimer", EJS_TIMER_PERIOD, 
            sessionTimer, server, MPR_EVENT_STATIC_DATA); 
    }
#else
    if (sessionEvent == 0) {
        sessionEvent = mprCreateTimerEvent(ejs->dispatcher, "sessionTimer", EJS_TIMER_PERIOD, 
            sessionTimer, server, MPR_EVENT_STATIC_DATA); 
    }
#endif
    mprUnlock(sessionLock);
}


#if UNUSED
//  MOB - should not take server as arg
void ejsCheckSessionTimer(EjsHttpServer *server)
{
    mprLock(sessionLock);
    if (server->sessionTimer) {
        mprRemoveEvent(server->sessionTimer);
        server->sessionTimer = 0;
    }
    mprUnlock(sessionLock);
}
#endif


/*  
    Check for expired sessions
 */
//  MOB -- remove server?
static void sessionTimer(EjsHttpServer *sp, MprEvent *event)
{
    Ejs             *ejs;
    EjsSession      *session;
    MprTime         now;
    HttpLimits      *limits;
    int             i, count, removed, soon, redline;

    mprAssert(!sp->ejs->destroying);
    mprAssert(sp->ejs->name);

#if UNUSED
    EjsPot     *sessions;
    sessions = sp->sessions;
#endif
    ejs = sp->ejs;
    mprAssert(!ejs->destroying);
    mprAssert(ejs->name);

    /*  
        This could be on the primary event thread. Can't block long.
     */
    if (sessions && sp->server && mprTryLock(sessionLock)) {
        removed = 0;
        limits = sp->server->limits;
        count = ejsGetPropertyCount(ejs, sessions);
        mprLog(7, "Check for sessions count %d/%d", count, limits->sessionCount);
        now = mprGetTime();

        /*
            Start pruning at 80% of the max session count
         */
        redline = limits->sessionCount * 8 / 10;
        if (count > redline) {
            /*
                Over redline. Must prune some sessions. Expire the oldest sessions.
                One quick swipe to find sessions that are 80% expired.
             */
            soon = limits->sessionTimeout / 5;
            for (i = count - 1; soon > 0 && i >= 0; i--) {
                if ((session = ejsGetProperty(ejs, sessions, i)) == 0) {
                    continue;
                }
                if ((session->expire - now) < soon) {
                    mprLog(3, "Too many sessions. Pruning session %s", session->id);
                    ejsDeleteProperty(ejs, sessions, i);
                    removed++;
                    count--;
                }
            }
        }
        for (i = count - 1; i >= 0; i--) {
            if ((session = ejsGetProperty(ejs, sessions, i)) == 0) {
                continue;
            }
            mprAssert(TYPE(session) == ST(session));
            if (TYPE(session) == ST(Session)) {
                mprLog(7, "Check session %s timeout %d, expires %d secs", session->id, 
                                    session->timeout / MPR_TICKS_PER_SEC,
                                   (int) (session->expire - now) / MPR_TICKS_PER_SEC);
                if (count > limits->sessionCount) {
                    mprLog(3, "Too many sessions. Pruning session %s", session->id);
                    ejsDeleteProperty(ejs, sessions, i);
                    removed++;
                    count--;
                }  
                if (session->expire <= now) {
                    mprLog(3, "Session expired: %s (timeout %d secs)", 
                        session->id, session->timeout / MPR_TICKS_PER_SEC);
                    ejsDeleteProperty(ejs, sessions, i);
                    removed++;
                    count--;
                }
            }
        }
        if (removed) {
            count = ejsCompactPot(ejs, sessions);
        }
        if (count == 0) {
            sessionEvent = 0;
            mprRemoveEvent(event);
        }
        mprUnlock(sessionLock);
    }
}


static void manageSession(EjsSession *sp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ejsManagePot(sp, flags);
        mprMark(sp->id);
    }
}


void ejsConfigureSessionType(Ejs *ejs)
{
    EjsType         *type;
    EjsHelpers      *helpers;

    type = ejsConfigureNativeType(ejs, N("ejs.web", "Session"), sizeof(EjsSession), manageSession, EJS_POT_HELPERS);
    ejsSetSpecialType(ejs, S_Session, type);
    mprAssert(type->mutex == 0);
    if (sessionLock == 0) {
        sessionLock = type->mutex = mprCreateLock();
        mprHold(sessionLock);
    }
    helpers = &type->helpers;
    helpers->getProperty = (EjsGetPropertyHelper) getSessionProperty;
    helpers->getPropertyByName = (EjsGetPropertyByNameHelper) getSessionPropertyByName;
    helpers->setProperty = (EjsSetPropertyHelper) setSessionProperty;
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
