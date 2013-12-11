/**
    ejsWeb.h -- Header for the Ejscript Web Framework
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#ifndef _h_EJS_WEB_h
#define _h_EJS_WEB_h 1

#include    "http.h"
#include    "ejs.web.slots.h"

/*********************************** Defines **********************************/

#define EJS_SESSION "-ejs-session-"             /**< Default session cookie string */

#ifdef  __cplusplus
extern "C" {
#endif

/*********************************** Types ************************************/

#ifndef EJS_HTTPSERVER_NAME
#define EJS_HTTPSERVER_NAME "ejs-http"
#endif

/** 
    HttpServer Class
    @description
        HttpServer objects represents a Hypertext Transfer Protocol version 1.1 client connection and are used 
        HTTP requests and capture responses. This class supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
    @stability Prototype
    @defgroup EjsHttpServer EjsHttpServer
    @see EjsHttpServer ejsCloneHttpServer
 */
typedef struct EjsHttpServer {
    EjsPot          pot;                        /**< Extends Object */
    Ejs             *ejs;                       /**< Ejscript interpreter handle */
    HttpEndpoint    *endpoint;                  /**< Http endpoint object */
    struct MprSsl   *ssl;                       /**< SSL configuration */
    HttpTrace       trace[2];                   /**< Default tracing for requests */
    cchar           *connector;                 /**< Pipeline connector */
    char            *keyFile;                   /**< SSL key file */
    char            *certFile;                  /**< SSL certificate file */
    char            *protocols;                 /**< SSL protocols */
    char            *ciphers;                   /**< SSL ciphers */
    char            *ip;                        /**< Listening address */
    char            *name;                      /**< Server name */
    int             async;                      /**< Async mode */
    int             port;                       /**< Listening port */
    int             hosted;                     /**< Server being hosted inside a web server */
    struct EjsHttpServer *cloned;               /**< Server that was cloned */
    EjsObj          *emitter;                   /**< Event emitter */
    EjsObj          *limits;                    /**< Limits object */
    EjsArray        *incomingStages;            /**< Incoming Http pipeline stages */
    EjsArray        *outgoingStages;            /**< Outgoing Http pipeline stages */
} EjsHttpServer;

/** 
    Clone a http server
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param server HttpServer object
    @param deep Ignored
    @returns A new server object.
    @ingroup EjsHttpServer
*/
extern EjsHttpServer *ejsCloneHttpServer(Ejs *ejs, EjsHttpServer *server, bool deep);

/** 
    Request Class
    @description
        Request objects represent a single Http request.
    @stability Prototype
    @defgroup EjsRequest EjsRequest
    @see EjsRequest ejsCloneRequest ejsCreateRequest 
 */
typedef struct EjsRequest {
    EjsPot          pot;                /**< Base object storage */
    EjsObj          *absHome;           /**< Absolute URI to the home of the application from this request */
    struct EjsRequest *cloned;          /**< Request that was cloned */
    EjsObj          *config;            /**< Request config environment */
    HttpConn        *conn;              /**< Underlying Http connection object */
    EjsObj          *cookies;           /**< Cached cookies */
    EjsPath         *dir;               /**< Home directory containing the application */
    EjsObj          *emitter;           /**< Event emitter */
    EjsObj          *env;               /**< Request.env */
    EjsPath         *filename;          /**< Physical resource filename */
    EjsObj          *files;             /**< Files object */
    EjsString       *formData;          /**< Form data as a stable, sorted string */
    EjsObj          *headers;           /**< Headers object */
    EjsUri          *home;              /**< Relative URI to the home of the application from this request */
    EjsString       *host;              /**< Host property */
    EjsObj          *limits;            /**< Limits object */
    EjsObj          *log;               /**< Log object */
    EjsObj          *originalUri;       /**< Saved original URI */
    EjsObj          *params;            /**< Form variables + routing variables */
    EjsString       *pathInfo;          /**< PathInfo property */
    EjsNumber       *port;              /**< Port property */
    EjsString       *query;             /**< Query property */
    EjsString       *reference;         /**< Reference property */
    EjsObj          *responseHeaders;   /**< Headers object */
    EjsObj          *route;             /**< Matching route in route table */
    EjsString       *scheme;            /**< Scheme property */
    EjsString       *scriptName;        /**< ScriptName property */
    EjsHttpServer   *server;            /**< Owning server */
    EjsUri          *uri;               /**< Complete uri */
    EjsByteArray    *writeBuffer;       /**< Write buffer for capturing output */

    Ejs             *ejs;               /**< Ejscript interpreter handle */
    struct EjsSession *session;         /**< Current session */

    //  OPT - make bit fields
    int             dontAutoFinalize;   /**< Suppress auto-finalization */
    int             probedSession;      /**< Determined if a session exists */
    int             closed;             /**< Request closed and "close" event has been issued */
    int             error;              /**< Request errored and "error" event has been issued */
    int             finalized;          /**< Request has written all output data */
    int             running;            /**< Request has started */
    ssize           written;            /**< Count of data bytes written to the client */
} EjsRequest;

/** 
    Clone a request into another interpreter.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param req Original request to copy
    @param deep Ignored
    @return A new request object.
    @ingroup EjsRequest
*/
extern EjsRequest *ejsCloneRequest(Ejs *ejs, EjsRequest *req, bool deep);

/** 
    Create a new request. Create a new request object associated with the given Http connection.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param server EjsHttpServer owning this request
    @param conn Http connection object
    @param dir Default directory containing web documents
    @return A new request object.
    @ingroup EjsRequest
*/
extern EjsRequest *ejsCreateRequest(Ejs *ejs, EjsHttpServer *server, HttpConn *conn, cchar *dir);


/** 
    Session Class. Requests can access to session state storage via the Session class.
    @description
        Session objects represent a shared session state object into which Http Requests and store and retrieve data
        that persists beyond a single request.
    @stability Prototype
    @defgroup EjsSession EjsSession
    @see EjsSession ejsGetSession ejsDestroySession
 */
typedef struct EjsSession {
    EjsPot      pot;                /* Session properties */
    EjsString   *key;               /* Session ID key */
    EjsObj      *cache;             /* Cache store reference */
    EjsObj      *options;           /* Default write options */
    MprTicks    timeout;            /* Session inactivity timeout (msecs) */
    int         ready;              /* Data cached from store into pot */
} EjsSession;

/** 
    Get a session object for a given key. This will create a session if the given key is NULL or has expired.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param key String containing the session ID
    @param timeout Timeout to use for the session if one is created
    @param create Create a new session if an existing session cannot be found or it has expired.
    @returns A new session object.
    @ingroup EjsSession
*/
extern EjsSession *ejsGetSession(Ejs *ejs, EjsString *key, MprTicks timeout, int create);

/** 
    Destroy as session. This destroys the session object so that subsequent requests will need to establish a new session.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param session Session object created via ejsGetSession()
    @ingroup EjsSession
 */
extern int ejsDestroySession(Ejs *ejs, EjsSession *session);

/** 
    Set a session timeout
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param sp Session object
    @param lifespan Lifespan in milliseconds
    @ingroup EjsSession
*/
extern void ejsSetSessionTimeout(Ejs *ejs, EjsSession *sp, MprTicks lifespan);

/******************************* Internal APIs ********************************/

extern void ejsConfigureHttpServerType(Ejs *ejs);
extern void ejsConfigureRequestType(Ejs *ejs);
extern void ejsConfigureSessionType(Ejs *ejs);
extern void ejsConfigureWebTypes(Ejs *ejs);
extern void ejsSendRequestCloseEvent(Ejs *ejs, EjsRequest *req);
extern void ejsSendRequestErrorEvent(Ejs *ejs, EjsRequest *req);

#ifdef  __cplusplus
}
#endif
#endif /* _h_EJS_WEB_h */

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
