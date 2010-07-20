/**
    ejsWeb.h -- Header for the Ejscript Web Framework
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#ifndef _h_EJS_WEB_h
#define _h_EJS_WEB_h 1

#include    "ejs.web.slots.h"

/*********************************** Defines **********************************/

#define EJS_SESSION             "-ejs-session-"

#ifdef  __cplusplus
extern "C" {
#endif

/*********************************** Types ************************************/

#ifndef EJS_HTTPSERVER_NAME
#define EJS_HTTPSERVER_NAME "ejs-http/" BLD_VERSION
#endif

/** 
    HttpServer Class
    @description
        HttpServer objects represents a Hypertext Transfer Protocol version 1.1 client connection and are used 
        HTTP requests and capture responses. This class supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
    @stability Prototype
    @defgroup EjsHttpServer EjsHttpServer
    @see EjsHttpServer
 */
typedef struct EjsHttpServer {
    EjsObj          obj;                        /**< Extends Object */
    EjsObj          *emitter;                   /**< Event emitter */
    Ejs             *ejs;                       /**< Ejscript interpreter handle */
    HttpServer      *server;                    /**< Http server object */
    struct MprSsl   *ssl;                       /**< SSL configuration */
    cchar           *dir;                       /**< Directory containing web documents */
    char            *keyFile;                   /**< SSL key file */
    char            *certFile;                  /**< SSL certificate file */
    char            *protocols;                 /**< SSL protocols */
    char            *ciphers;                   /**< SSL ciphers */
    char            *ip;                        /**< Listening address */
    char            *name;                      /**< Server name */
    int             port;                       /**< Listening port */
    int             async;                      /**< Async mode */
} EjsHttpServer;


/** 
    Request Class
    @description
        Request objects represent a single Http request.
    @stability Prototype
    @defgroup EjsRequest EjsRequest
    @see EjsRequest ejsCreateRequest
 */
typedef struct EjsRequest {
    EjsObj          obj;            /**< Base object storage */
    EjsObj          *cookies;       /**< Cached cookies */
    HttpConn        *conn;          /**< Underlying Http connection object */
    EjsHttpServer   *server;        /**< Owning server */
    EjsObj          *emitter;       /**< Event emitter */
    struct EjsSession *session;     /**< Current session */
    EjsObj          *files;         /**< Cached files object */
    EjsObj          *headers;       /**< Cached headers object */
    EjsObj          *params;        /**< Form variables */
    EjsObj          *env;           /**< Request.env */
    EjsUri          *fullUri;       /**< Cached full uri */
    Ejs             *ejs;           /**< Ejscript interpreter handle */
    cchar           *dir;           /**< Home directory containing the application */
    cchar           *home;          /**< Relative URI to the home of the application from this request */
    cchar           *absHome;       /**< Absolute URI to the home of the application from this request */
    int             dontFinalize;   /**< Don't auto-finalize. Must call finalize(force) */
    int             probedSession;  /**< Determined if a session exists */
    int             running;        /**< Request has started */
} EjsRequest;


/** 
    Create a new request. Create a new request object associated with the given Http connection.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param server EjsHttpServer owning this request
    @param conn Http connection object
    @param dir Default directory containing web documents
    @return A new request object.
*/
extern EjsRequest *ejsCreateRequest(Ejs *ejs, EjsHttpServer *server, HttpConn *conn, cchar *dir);

/** 
    Clone a new request. This is used to clone a request from one interpreter into another.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param req Original request to copy
    @param deep Ignored
    @return A new request object.
*/
extern EjsRequest *ejsCloneRequest(Ejs *ejs, EjsRequest *req, bool deep);

/** 
    Session Class
    @description
        Session objects represent a shared session state object into which Http Requests and store and retrieve data
        that persists beyond a single request.
    @stability Prototype
    @defgroup EjsSession EjsSession
    @see EjsSession ejsCreateSession ejsDestroySession
 */
typedef struct EjsSession
{
    EjsObj      obj;
    MprTime     expire;             /* When the session should expire */
    cchar       *id;                /* Session ID */
    int         timeout;            /* Session inactivity lifespan */
    int         index;              /* Index in sesssions[] */
} EjsSession;

/** 
    Create a session object
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param req Request object creating the session
    @param timeout Timeout in milliseconds
    @param secure If the session is to be given a cookie that is designated as only for secure sessions (SSL)
    @returns A new session object.
*/
extern EjsSession *ejsCreateSession(Ejs *ejs, struct EjsRequest *req, int timeout, bool secure);
extern EjsSession *ejsGetSession(Ejs *ejs, struct EjsRequest *req);

/** 
    Destroy as session. This destroys the session object so that subsequent requests will need to establish a new session.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param server Server object owning the session.
    @param session Session object created via ejsCreateSession()
*/
extern int ejsDestroySession(Ejs *ejs, EjsHttpServer *server, EjsSession *session);

/******************************* Internal APIs ********************************/

extern void ejsConfigureHttpServerType(Ejs *ejs);
extern void ejsConfigureRequestType(Ejs *ejs);
extern void ejsConfigureSessionType(Ejs *ejs);
extern void ejsConfigureWebTypes(Ejs *ejs);

#ifdef  __cplusplus
}
#endif
#endif /* _h_EJS_WEB_h */

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
