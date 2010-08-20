/*
    ejsUri.c - Uri class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Forwards ********************************/

static EjsObj *completeUri(Ejs *ejs, EjsUri *up, EjsObj *missing, int includeQuery);
static int same(Ejs *ejs, HttpUri *u1, HttpUri *u2, int exact);
static HttpUri *createHttpUriFromHash(Ejs *ejs, MprCtx ctx, EjsObj *arg, int complete);
static HttpUri *toHttpUri(Ejs *ejs, MprCtx ctx, EjsObj *arg, int dup);
static EjsObj *uri_join(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv);

/************************************ Helpers *********************************/

#if UNUSED && KEEP
//  MOB -- keep this for when the cast helper is reversed
/*
    Convert an arg to a URI. Can handle strings, paths, URIs and object hashes. Will cast all else to strings and then
    parse.
 */
static EjsUri *castToUri(Ejs *ejs, EjsObj *arg)
{
    EjsUri  *up;

    up = (EjsUri*) ejsCreate(ejs, ejs->uriType, 0);
    if (ejsIsString(arg)) {
        up->uri = httpCreateUri(up, ejsGetString(ejs, arg), 0);

    } else if (ejsIsUri(ejs, arg)) {
        up->uri = httpCloneUri(up, ((EjsUri*) arg)->uri, 0);

    } else if (ejsIsPath(ejs, arg)) {
        ustr = ((EjsPath*) arg)->path;
        up->uri = httpCreateUri(up, ustr, 0);

    } else if (ejsGetPropertyCount(ejs, arg) > 0) {
        up->uri = createHttpUriFromHash(ejs, up, arg, 0);

    } else {
        arg = (EjsObj) ejsToString(ejs, arg);
        up->uri = httpCreateUri(up, ejsGetString(ejs, arg), 0);
    }
    return up;
}
#endif


static EjsUri *cloneUri(Ejs *ejs, EjsUri *src, bool deep)
{
    EjsUri     *dest;

    dest = (EjsUri*) ejsCloneObject(ejs, (EjsObj*) src, deep);
    /*  Deep copy will complete the uri */
    dest->uri = httpCloneUri(dest, src->uri, deep);
    return dest;
}


static EjsObj *coerceUriOperands(Ejs *ejs, EjsUri *lhs, int opcode,  EjsObj *rhs)
{
    HttpUri     *uri;
    char        *ustr;

    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        uri = lhs->uri;
        ustr = httpFormatUri(lhs, uri->scheme, uri->host, uri->port, uri->path, uri->reference, uri->query, 0);
        return ejsInvokeOperator(ejs, (EjsObj*) ejsCreateString(ejs, ustr), opcode, rhs);

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIsNull(rhs) || ejsIsUndefined(rhs)) {
            return (EjsObj*) ((opcode == EJS_OP_COMPARE_EQ) ? ejs->falseValue: ejs->trueValue);
        }
        uri = lhs->uri;
        ustr = httpFormatUri(lhs, uri->scheme, uri->host, uri->port, uri->path, uri->reference, uri->query, 0);
        return ejsInvokeOperator(ejs, (EjsObj*) ejsCreateStringAndFree(ejs, ustr), opcode, rhs);

    case EJS_OP_COMPARE_STRICTLY_NE:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return (EjsObj*) ejs->falseValue;

    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_TRUE:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return (EjsObj*) ejs->falseValue;

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return (EjsObj*) ejs->falseValue;

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %s", opcode, lhs->obj.type->qname.name);
        return ejs->undefinedValue;
    }
    return 0;
}


static EjsObj *invokeUriOperator(Ejs *ejs, EjsUri *lhs, int opcode,  EjsUri *rhs, void *data)
{
    EjsObj      *result;

    if (rhs == 0 || lhs->obj.type != rhs->obj.type) {
        if ((result = coerceUriOperands(ejs, lhs, opcode, (EjsObj*) rhs)) != 0) {
            return result;
        }
    }

    /*  Types now match, both Uris
     */
    switch (opcode) {
    case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_EQ:
        if (lhs == rhs || (lhs->uri == rhs->uri)) {
            return (EjsObj*) ejs->trueValue;
        }
        return (EjsObj*) ejsCreateBoolean(ejs,  same(ejs, lhs->uri, rhs->uri, 1));

    case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_STRICTLY_NE:
        return (EjsObj*) ejsCreateBoolean(ejs,  !same(ejs, lhs->uri, rhs->uri, 1));

    /*  NOTE: these only compare the paths */
    case EJS_OP_COMPARE_LT:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprStrcmp(lhs->uri->path, rhs->uri->path) < 0);

    case EJS_OP_COMPARE_LE:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprStrcmp(lhs->uri->path, rhs->uri->path) <= 0);

    case EJS_OP_COMPARE_GT:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprStrcmp(lhs->uri->path, rhs->uri->path) > 0);

    case EJS_OP_COMPARE_GE:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprStrcmp(lhs->uri->path, rhs->uri->path) >= 0);

    /*  
        Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return (EjsObj*) ((lhs->uri->path) ? ejs->trueValue: ejs->falseValue);

    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ((lhs->uri->path == 0) ? ejs->trueValue: ejs->falseValue);


    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return (EjsObj*) ejs->falseValue;

    /*  
        Binary operators
     */
    case EJS_OP_ADD:
        return uri_join(ejs, lhs, 1, (EjsObj**) &rhs);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %s", opcode, lhs->obj.type->qname.name);
        return 0;
    }
    mprAssert(0);
}


/************************************ Methods *********************************/

/*  
    Constructor
    function Uri(uri: Uri)
    function Uri(path: String)
    function Uri(parts: Object)
 */
static EjsObj *uri_constructor(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    if (argc >= 0) {
        up->uri = toHttpUri(ejs, up, argv[0], 1);
    }
    return (EjsObj*) up;
}


/*  
    Make an absolute reference for "this" URI.

    function absolute(base): Uri
 */
static EjsObj *uri_absolute(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *result;
    HttpUri     *uri, *baseUri;

    if (argc >= 1) {
        baseUri = toHttpUri(ejs, ejs, argv[0], 1);
        result = cloneUri(ejs, up, 0);
        uri = result->uri;
        if (uri->path && uri->path[0] != '/') {
            httpJoinUriPath(uri, baseUri, uri);
        }
        httpCompleteUri(result->uri, baseUri);
        mprFree(baseUri);
    } else {
        result = cloneUri(ejs, up, 0);
        httpCompleteUri(result->uri, NULL);
    }
    httpNormalizeUri(result->uri);
    return (EjsObj*) result;
}


/*  
    function get basename(): Uri
 */
static EjsObj *uri_basename(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *np;
    char        *path, *cp;
    int         len;

    np = cloneUri(ejs, up, 0);
    path = np->uri->path;
    if (path == 0) {
        return (EjsObj*) ejs->emptyStringValue;
    }
    len = strlen(path);
    if (path[len - 1] == '/') {
        *path = '\0';
    } else {
        if ((cp = strrchr(path, '/')) != 0) {
            np->uri->path = &cp[1];
        }
    }
    return (EjsObj*) np;
}


/*  
    function complete(missing = null): Uri
 */
static EjsObj *uri_complete(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri  *result;

    result = cloneUri(ejs, up, 0);
    return completeUri(ejs, result, (argc >= 1) ? argv[0] : 0, 1);
}


/*  
    Break a uri into components
  
    function components(): Object
 */
static EjsObj *uri_components(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsObj      *obj;
    HttpUri     *uri;
    EjsName     qname;

    uri = up->uri;
    obj = ejsCreateSimpleObject(ejs);

    if (uri->scheme) {
        ejsSetPropertyByName(ejs, obj, EN(&qname, "scheme"), ejsCreateString(ejs, uri->scheme));
    }
    if (uri->host) {
        ejsSetPropertyByName(ejs, obj, EN(&qname, "host"), ejsCreateString(ejs, uri->host));
    }
    if (uri->port > 0) {
        ejsSetPropertyByName(ejs, obj, EN(&qname, "port"), ejsCreateNumber(ejs, uri->port));
    }
    if (uri->path) {
        ejsSetPropertyByName(ejs, obj, EN(&qname, "path"), ejsCreateString(ejs, uri->path));
    }
    if (uri->reference) {
        ejsSetPropertyByName(ejs, obj, EN(&qname, "reference"), ejsCreateString(ejs, uri->reference));
    }
    if (uri->query) {
        ejsSetPropertyByName(ejs, obj, EN(&qname, "query"), ejsCreateString(ejs, uri->query));
    }
    return (EjsObj*) obj;
}


/*  
    Decode a Uri
    static function decode(str: String): String
 */
static EjsObj *uri_decode(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringAndFree(ejs, mprUriDecode(ejs, ejsGetString(ejs, argv[0])));
}


/*  
    Decode a Uri component
    static function decodeComponent(str: String): String
 */
static EjsObj *uri_decodeComponent(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringAndFree(ejs, mprUriDecode(ejs, ejsGetString(ejs, argv[0])));
}


/*  
    function get dirname(): Uri
 */
static EjsObj *uri_dirname(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *np;
    char        *path, *cp;
    int         len;

    np = cloneUri(ejs, up, 0);
    path = np->uri->path;
    if (path == 0) {
        return (EjsObj*) ejs->emptyStringValue;
    }
    len = strlen(path);
    if (path[len - 1] == '/') {
        if (len > 1) {
            path[len - 1] = '\0';
        }
    } else {
        if ((cp = strrchr(path, '/')) != 0) {
            if (cp > path) {
                *cp = '\0';
            } else {
                cp[1] = '\0';
            }
        }
    }
    return (EjsObj*) np;
}


/*  
    Uri Encode a string
    function encode(str: String): String
 */
static EjsObj *uri_encode(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringAndFree(ejs, mprUriEncode(ejs, ejsGetString(ejs, argv[0]), MPR_ENCODE_URI));
}


/*  
    Encode a Uri component
    static function encodeComponent(str: String): String
 */
static EjsObj *uri_encodeComponent(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringAndFree(ejs, mprUriEncode(ejs, ejsGetString(ejs, argv[0]), MPR_ENCODE_URI_COMPONENT));
}


/*  
    Get the Uri extension
    static function get extension(): String
 */
static EjsObj *uri_extension(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, up->uri->ext);
}


/*  
    Set the Uri extension
    static function set extension(ext: String): Void
 */
static EjsObj *uri_set_extension(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    HttpUri     *uri;

    uri = up->uri;
    uri->ext = mprStrdup(uri, ejsGetString(ejs, argv[0]));
    uri->path = mprStrcat(uri, -1, mprTrimPathExtension(uri, uri->path), uri->ext, NULL);
    return 0;
}


/*  
    Determine if the uri has an extension
    static function get hasExtension(): Boolean
 */
static EjsObj *uri_hasExtension(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return (EjsObj*) ((up->uri->ext) ? ejs->trueValue : ejs->falseValue);
}


/*  
    Determine if the uri has a host
    static function get hasHost(): Boolean
 */
static EjsObj *uri_hasHost(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return (EjsObj*) ((up->uri->host) ? ejs->trueValue : ejs->falseValue);
}


/*  
    Determine if the uri has a port
    static function get hasPort(): Boolean
 */
static EjsObj *uri_hasPort(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return (EjsObj*) ((up->uri->port > 0) ? ejs->trueValue : ejs->falseValue);
}


/*  
    Determine if the uri has a query
    static function get hasQuery(): Boolean
 */
static EjsObj *uri_hasQuery(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return (EjsObj*) ((up->uri->query) ? ejs->trueValue : ejs->falseValue);
}


/*  
    Determine if the uri has a reference
    static function get hasReference(): Boolean
 */
static EjsObj *uri_hasReference(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return (EjsObj*) ((up->uri->reference) ? ejs->trueValue : ejs->falseValue);
}


/*  
    Determine if the uri has a scheme
    static function get hasScheme(): Boolean
 */
static EjsObj *uri_hasScheme(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return (EjsObj*) ((up->uri->scheme) ? ejs->trueValue : ejs->falseValue);
}


/*  
    Get the host portion
    static function get host(): String
 */
static EjsObj *uri_host(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    if (up->uri->host == 0) {
        return (EjsObj*) ejsCreateString(ejs, "localhost");
    }    
    return (EjsObj*) ejsCreateString(ejs, up->uri->host);
}


/*  
    Set the host portion
    static function set host(name: String): Void
 */
static EjsObj *uri_set_host(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    up->uri->host = mprStrdup(up->uri, ejsGetString(ejs, argv[0]));
    return 0;
}


/*  
    function get isAbsolute(): Boolean
 */
static EjsObj *uri_isAbsolute(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return (EjsObj*) ((up->uri->path[0] == '/') ? ejs->trueValue : ejs->falseValue);
}


/*  
    Determine if the file name is a directory. This 
    function get isDir(): Boolean
 */
static EjsObj *uri_isDir(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    HttpUri     *uri;

    uri = up->uri;
    return (EjsObj*) ((uri->path[strlen(uri->path) - 1] == '/') ? ejs->trueValue : ejs->falseValue);
}


/*  
    Join uri segments
    function join(...others): Uri
 */
static EjsObj *uri_join(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *result;
    EjsObj      *arg;
    EjsArray    *args;
    HttpUri     *uri, *other, *oldUri;
    int         i;

    args = (EjsArray*) argv[0];
    result = cloneUri(ejs, up, 0);
    uri = result->uri;
    for (i = 0; i < args->length; i++) {
        arg = ejsGetProperty(ejs, (EjsObj*) args, i);
        if ((other = toHttpUri(ejs, result, arg, 0)) == NULL) {
            return 0;
        }
        oldUri = uri;
        uri = httpJoinUri(result, oldUri, 1, &other);
        mprFree(oldUri);
        if (!ejsIsUri(ejs, arg)) {
            mprFree(other);
        }
    }
    result->uri = uri;
    return (EjsObj*) result;
}


/*  
    Join extension
    function joinExt(ext: String): Uri
 */
static EjsObj *uri_joinExt(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *np;
    HttpUri     *nuri;
    char        *ext;

    np = cloneUri(ejs, up, 1);
    nuri = np->uri;
    ext = mprStrdup(nuri, ejsGetString(ejs, argv[0]));
    if (ext && *ext == '.') {
        ext++;
    }
    nuri->ext = ext;
    nuri->path = mprStrcat(nuri, -1, mprTrimPathExtension(nuri, nuri->path), ".", nuri->ext, NULL);
    return (EjsObj*) np;
}


/*  
    Get the mimeType
    function mimeType(): String
 */
static EjsObj *uri_mimeType(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, mprLookupMimeType(ejs, up->uri->ext));
}


/*  
    function get normalize(): Uri
 */
static EjsObj *uri_normalize(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *np;

    np = cloneUri(ejs, up, 0);
    np->uri->path = httpNormalizeUriPath(np, up->uri->path);
    return (EjsObj*) np;
}


/*  
    Get the path portion
    static function get path(): String
 */
static EjsObj *uri_path(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, up->uri->path);
}


/*  
    Set the path portion
    static function set path(path: String): Void
 */
static EjsObj *uri_set_path(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    up->uri->path = httpNormalizeUriPath(up, ejsGetString(ejs, argv[0]));
    up->uri->ext = (char*) mprGetPathExtension(up->uri, up->uri->path);
    return 0;
}


/*  
    Get the port portion
    static function get port(): Number
 */
static EjsObj *uri_port(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    HttpUri     *uri;
    
    uri = up->uri;
    if (uri->port == 0) {
        //  MOB -- push this down into http
        if (uri->scheme == 0 || strcmp(uri->scheme, "http") == 0) {
            return (EjsObj*) ejsCreateNumber(ejs, 80);
        } else if (uri->scheme && strcmp(uri->scheme, "https") == 0) {
            return (EjsObj*) ejsCreateNumber(ejs, 443);
        }
    }
    return (EjsObj*) ejsCreateNumber(ejs, up->uri->port);
}


/*  
    Set the port portion
    static function set port(port: Number): Void
 */
static EjsObj *uri_set_port(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    up->uri->port = ejsGetInt(ejs, argv[0]);
    return 0;
}


/*  
    Replace the Uri extension
    static function set replaceExt(ext: String): Uri
 */
static EjsObj *uri_replaceExtension(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *np;
    HttpUri     *nuri;
    char        *ext;

    np = cloneUri(ejs, up, 1);
    nuri = np->uri;
    nuri->path = mprTrimPathExtension(nuri, nuri->path);
    ext = mprStrdup(np, ejsGetString(ejs, argv[0]));
    if (ext && *ext == '.') {
        ext++;
    }
    nuri->ext = ext;
    nuri->path = mprStrcat(nuri, -1, mprTrimPathExtension(nuri, nuri->path), ".", nuri->ext, NULL);
    return (EjsObj*) np;
}


/*  
    function resolve(target, relative: Boolean = true): Uri
 */
static EjsObj *uri_resolve(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *result;
    HttpUri     *uri, *target;
    int         relative;

    relative = (argc >= 2 && argv[1] == ejs->falseValue) ? 0 : 1;
    uri = up->uri;
    target = toHttpUri(ejs, ejs, argv[0], 0);
    result = (EjsUri*) ejsCreate(ejs, ejs->uriType, 0);
    uri = httpResolveUri(result, uri, 1, &target, relative);
    if (!ejsIsUri(ejs, target)) {
        mprFree(target);
    }
    if (up->uri == uri) {
        uri = httpCloneUri(result, uri, 0);
    }
    result->uri = uri;
    return (EjsObj*) result;
}


/*  
    function relative(base): Uri
 */
static EjsObj *uri_relative(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *result;
    HttpUri     *baseUri;

    baseUri = toHttpUri(ejs, up, argv[0], 0);
    result = (EjsUri*) ejsCreate(ejs, ejs->uriType, 0);
    result->uri = httpGetRelativeUri(result, baseUri, up->uri, 1);
    return (EjsObj*) result;
}


/*  
    Get the scheme portion
    static function get scheme(): String
 */
static EjsObj *uri_scheme(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    if (up->uri->scheme == 0) {
        return (EjsObj*) ejsCreateString(ejs, "http");
    }
    return (EjsObj*) ejsCreateString(ejs, up->uri->scheme);
}


/*  
    Set the scheme portion
    static function set scheme(scheme: String): Void
 */
static EjsObj *uri_set_scheme(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    up->uri->scheme = mprStrdup(up, ejsGetString(ejs, argv[0]));
    return 0;
}


/*  
    Get the query portion
    static function get query(): String
 */
static EjsObj *uri_query(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, up->uri->query);
}


/*  
    Set the query portion
    static function set query(query: String): Void
 */
static EjsObj *uri_set_query(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    cchar    *value;

    value = (argv[0] == ejs->nullValue) ? "" : ejsGetString(ejs, argv[0]);
    up->uri->query = mprStrdup(up, value);
    return 0;
}


/*  
    Get the reference portion
    static function get reference(): String
 */
static EjsObj *uri_reference(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, up->uri->reference);
}


/*  
    Set the reference portion
    static function set reference(reference: String): Void
 */
static EjsObj *uri_set_reference(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    cchar    *value;

    value = (argv[0] == ejs->nullValue) ? "" : ejsGetString(ejs, argv[0]);
    up->uri->reference = mprStrdup(up, value);
    return 0;
}


/*  
    Compare two Uris
    function same(other: String, exact: Boolean = false): Boolean
 */
static EjsObj *uri_same(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri  *other;
    int     exact;

    other = (EjsUri*) argv[0];
    exact = (argc == 2 && argv[1] == (EjsObj*) ejs->trueValue);
    return (EjsObj*) (same(ejs, up->uri, other->uri, exact) ? ejs->trueValue: ejs->falseValue);
}


/* 
   function toString(): String
 */
static EjsObj *uri_toString(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    HttpUri     *uri;

    uri = up->uri;
    return (EjsObj*) ejsCreateStringAndFree(ejs, httpUriToString(up, uri, 0));
}


/* 
   function toLocalString(): String
 */
static EjsObj *uri_toLocalString(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    HttpUri     *uri;

    uri = up->uri;
    return (EjsObj*) ejsCreateStringAndFree(ejs, 
        httpFormatUri(ejs, NULL, NULL, 0, uri->path, uri->reference, uri->query, 0));
}


/*  
    function trimExt(): Uri
 */
static EjsObj *uri_trimExt(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *np;
    HttpUri     *nuri;

    np = cloneUri(ejs, up, 1);
    nuri = np->uri;
    nuri->ext = 0;
    nuri->path = mprTrimPathExtension(nuri, nuri->path);
    return (EjsObj*) np;
}


/*  
    function set uri(value: String): Void
 */
static EjsObj *uri_set_uri(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    mprFree(up->uri);
    up->uri = httpCreateUri(up, ejsGetString(ejs, argv[0]), 0);
    return 0;
}

/*********************************** Helpers **********************************/

#if UNUSED && KEEP
static char *uriToString(Ejs *ejs, EjsUri *up)
{
    HttpUri     *uri;

    uri = up->uri;
    return httpFormatUri(ejs, uri->scheme, uri->host, uri->port, uri->path, uri->reference, uri->query, 0);
}
#endif


static EjsObj *completeUri(Ejs *ejs, EjsUri *up, EjsObj *missing, int includeQuery)
{
    EjsUri      *missingUri;

    if (missing == 0 || ejsIsNull(missing) || ejsIsUndefined(missing)) {
        missingUri = 0;
    } else if (ejsGetPropertyCount(ejs, missing) > 0) {
        missingUri = (EjsUri*) ejsCreate(ejs, ejs->uriType, 0);
        missingUri->uri = createHttpUriFromHash(ejs, missingUri, missing, 1);
    } else {
        missingUri = ejsToUri(ejs, missing);
    }
    if (missingUri == 0) {
        if (!includeQuery) {
            up->uri->query = NULL;
        }
        httpCompleteUri(up->uri, NULL);
    } else {
        httpCompleteUri(up->uri, missingUri->uri);
    }
    return (EjsObj*) up;
}


#if UNUSED
static char *getUriString(Ejs *ejs, EjsObj *vp)
{
    if (ejsIsString(vp)) {
        //  MOB query dup
        return mprStrdup(ejs, ejsGetString(ejs, vp));

    } else if (ejsIsUri(ejs, vp)) {
        return uriToString(ejs, ((EjsUri*) vp));
    }
    ejsThrowIOError(ejs, "Bad ");
    return NULL;
}
#endif


static HttpUri *toHttpUri(Ejs *ejs, MprCtx ctx, EjsObj *arg, int dup)
{
    HttpUri     *uri;

    if (ejsIsString(arg)) {
        uri = httpCreateUri(ctx, ejsGetString(ejs, arg), 0);

    } else if (ejsIsUri(ejs, arg)) {
        if (dup) {
            uri = httpCloneUri(ctx, ((EjsUri*) arg)->uri, 0);
        } else {
            uri = ((EjsUri*) arg)->uri;
        }

    } else if (ejsIsPath(ejs, arg)) {
        uri = httpCreateUri(ctx, ((EjsPath*) arg)->path, 0);

    } else if (ejsGetPropertyCount(ejs, arg) > 0) {
        uri = createHttpUriFromHash(ejs, ctx, arg, 0);

    } else {
        arg = (EjsObj*) ejsToString(ejs, arg);
        uri = httpCreateUri(ctx, ejsGetString(ejs, arg), 0);
    }
    return uri;
}


static int same(Ejs *ejs, HttpUri *u1, HttpUri *u2, int exact)
{
    if (u1 == u2) {
        return 1;
    }
    if ((u1->scheme && !u2->scheme) || (!u1->scheme && u2->scheme)) {
        return 0;
    }
    if (u1->scheme && strcmp(u1->scheme, u2->scheme) != 0) {
        return 0;
    }
    if ((u1->host && !u2->host) || (!u1->host && u2->host)) {
        return 0;
    }
    if (u1->host && strcmp(u1->host, u2->host) != 0) {
        return 0;
    }
    if ((u1->path && !u2->path) || (!u1->path && u2->path)) {
        return 0;
    }
    if (u1->path && strcmp(u1->path, u2->path) != 0) {
        return 0;
    }
    if (u1->port != u2->port) {
        return 0;
    }
    if (exact) {
        if ((u1->reference && !u2->reference) || (!u1->reference && u2->reference)) {
            return 0;
        }
        if (u1->reference && strcmp(u1->reference, u2->reference) != 0) {
            return 0;
        }
        if ((u1->query && !u2->query) || (!u1->query && u2->query)) {
            return 0;
        }
        if (u1->query && strcmp(u1->query, u2->query) != 0) {
            return 0;
        }
    }
    return 1;
}


static HttpUri *createHttpUriFromHash(Ejs *ejs, MprCtx ctx, EjsObj *arg, int complete)
{
    EjsObj      *schemeObj, *hostObj, *portObj, *pathObj, *referenceObj, *queryObj, *uriObj;
    EjsName     qname;
    cchar       *scheme, *host, *path, *reference, *query;
    int         port;

    /*
        This permits a uri property override. Used in ejs.web::View
     */
    uriObj = ejsGetPropertyByName(ejs, arg, EN(&qname, "uri"));
    if (uriObj) {
        return toHttpUri(ejs, ctx, uriObj, 1);
    }

    schemeObj = ejsGetPropertyByName(ejs, arg, EN(&qname, "scheme"));
    scheme = (schemeObj && ejsIsString(schemeObj)) ? ejsGetString(ejs, schemeObj) : 0;

    hostObj = ejsGetPropertyByName(ejs, arg, EN(&qname, "host"));
    host = (hostObj && ejsIsString(hostObj)) ? ejsGetString(ejs, hostObj) : 0;

    port = 0;
    if ((portObj = ejsGetPropertyByName(ejs, arg, EN(&qname, "port"))) != 0) {
        if (ejsIsNumber(portObj)) {
            port = ejsGetInt(ejs, portObj);
        } else if (ejsIsString(portObj)) {
            port = (int) mprAtoi(ejsGetString(ejs, portObj), 10);
        }
    }
    pathObj = ejsGetPropertyByName(ejs, arg, EN(&qname, "path"));
    path = (pathObj && ejsIsString(pathObj)) ? ejsGetString(ejs, pathObj) : 0;

    referenceObj = ejsGetPropertyByName(ejs, arg, EN(&qname, "reference"));
    reference = (referenceObj && ejsIsString(referenceObj)) ? ejsGetString(ejs, referenceObj) : 0;

    queryObj = ejsGetPropertyByName(ejs, arg, EN(&qname, "query"));
    query = (queryObj && ejsIsString(queryObj)) ? ejsGetString(ejs, queryObj) : 0;

    return httpCreateUriFromParts(ctx, scheme, host, port, path, reference, query, complete);
}


/*  
    Decode a Uri (ECMA Standard)

    function decodeURI(str: String): String
 */
static EjsObj *decodeURI(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringAndFree(ejs, mprUriDecode(ejs, ejsGetString(ejs, argv[0])));
}


/*  
    Decode a Uri component (ECMA Standard)
    function decodeURIComponent(str: String): String
 */
static EjsObj *decodeURIComponent(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringAndFree(ejs, mprUriDecode(ejs, ejsGetString(ejs, argv[0])));
}


/*  
    Uri Encode a string (ECMA Standard)
    function encodeURI(str: String): String
 */
static EjsObj *encodeURI(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringAndFree(ejs, mprUriEncode(ejs, ejsGetString(ejs, argv[0]), MPR_ENCODE_JS_URI));
}


/*  
    Encode a Uri component (ECMA Standard)
    static function encodeComponent(str: String): String
 */
static EjsObj *encodeURIComponent(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringAndFree(ejs, mprUriEncode(ejs, ejsGetString(ejs, argv[0]), MPR_ENCODE_JS_URI_COMPONENT));
}


/*********************************** Factory **********************************/

EjsUri *ejsCreateUri(Ejs *ejs, cchar *path)
{
    EjsUri      *up;
    EjsObj      *arg;

    up = (EjsUri*) ejsCreate(ejs, ejs->uriType, 0);
    if (up == 0) {
        return 0;
    }
    arg = (EjsObj*) ejsCreateString(ejs, path);
    uri_constructor(ejs, up, 1, (EjsObj**) &arg);
    return up;
}


EjsUri *ejsCreateFullUri(Ejs *ejs, cchar *scheme, cchar *host, int port, cchar *path, cchar *query, cchar *reference, 
    int complete)
{
    EjsUri      *up;

    up = (EjsUri*) ejsCreate(ejs, ejs->uriType, 0);
    if (up == 0) {
        return 0;
    }
    up->uri = httpCreateUriFromParts(up, scheme, host, port, path, reference, query, complete);
    return up;
}


EjsUri *ejsCreateUriAndFree(Ejs *ejs, char *value)
{
    EjsUri     *up;

    up = ejsCreateUri(ejs, value);
    mprFree(value);
    return up;
}


void ejsConfigureUriType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *prototype;

    type = ejs->uriType = ejsConfigureNativeType(ejs, EJS_EJS_NAMESPACE, "Uri", sizeof(EjsUri));
    prototype = type->prototype;

    type->helpers.clone = (EjsCloneHelper) cloneUri;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeUriOperator;

    ejsBindMethod(ejs, type, ES_Uri_decode, (EjsProc) uri_decode);
    ejsBindMethod(ejs, type, ES_Uri_decodeComponent, (EjsProc) uri_decodeComponent);
    ejsBindMethod(ejs, type, ES_Uri_encode, (EjsProc) uri_encode);
    ejsBindMethod(ejs, type, ES_Uri_encodeComponent, (EjsProc) uri_encodeComponent);

    ejsBindConstructor(ejs, type, (EjsProc) uri_constructor);
    ejsBindMethod(ejs, prototype, ES_Uri_absolute, (EjsProc) uri_absolute);
    ejsBindMethod(ejs, prototype, ES_Uri_basename, (EjsProc) uri_basename);
    ejsBindMethod(ejs, prototype, ES_Uri_complete, (EjsProc) uri_complete);
    ejsBindMethod(ejs, prototype, ES_Uri_components, (EjsProc) uri_components);
    ejsBindMethod(ejs, prototype, ES_Uri_dirname, (EjsProc) uri_dirname);
    ejsBindAccess(ejs, prototype, ES_Uri_extension, (EjsProc) uri_extension, (EjsProc) uri_set_extension);
    ejsBindMethod(ejs, prototype, ES_Uri_hasExtension, (EjsProc) uri_hasExtension);
    ejsBindMethod(ejs, prototype, ES_Uri_hasHost, (EjsProc) uri_hasHost);
    ejsBindMethod(ejs, prototype, ES_Uri_hasPort, (EjsProc) uri_hasPort);
    ejsBindMethod(ejs, prototype, ES_Uri_hasQuery, (EjsProc) uri_hasQuery);
    ejsBindMethod(ejs, prototype, ES_Uri_hasReference, (EjsProc) uri_hasReference);
    ejsBindMethod(ejs, prototype, ES_Uri_hasScheme, (EjsProc) uri_hasScheme);
    ejsBindAccess(ejs, prototype, ES_Uri_host, (EjsProc) uri_host, (EjsProc) uri_set_host);
    ejsBindMethod(ejs, prototype, ES_Uri_isAbsolute, (EjsProc) uri_isAbsolute);
    ejsBindMethod(ejs, prototype, ES_Uri_isDir, (EjsProc) uri_isDir);
    ejsBindMethod(ejs, prototype, ES_Uri_join, (EjsProc) uri_join);
    ejsBindMethod(ejs, prototype, ES_Uri_joinExt, (EjsProc) uri_joinExt);
    ejsBindMethod(ejs, prototype, ES_Uri_mimeType, (EjsProc) uri_mimeType);
    ejsBindMethod(ejs, prototype, ES_Uri_normalize, (EjsProc) uri_normalize);
    ejsBindAccess(ejs, prototype, ES_Uri_path, (EjsProc) uri_path, (EjsProc) uri_set_path);
    ejsBindAccess(ejs, prototype, ES_Uri_port, (EjsProc) uri_port, (EjsProc) uri_set_port);
    ejsBindAccess(ejs, prototype, ES_Uri_scheme, (EjsProc) uri_scheme, (EjsProc) uri_set_scheme);
    ejsBindAccess(ejs, prototype, ES_Uri_query, (EjsProc) uri_query, (EjsProc) uri_set_query);
    ejsBindAccess(ejs, prototype, ES_Uri_reference, (EjsProc) uri_reference, (EjsProc) uri_set_reference);
    ejsBindMethod(ejs, prototype, ES_Uri_replaceExt, (EjsProc) uri_replaceExtension);
    ejsBindMethod(ejs, prototype, ES_Uri_relative, (EjsProc) uri_relative);
    ejsBindMethod(ejs, prototype, ES_Uri_resolve, (EjsProc) uri_resolve);
    ejsBindMethod(ejs, prototype, ES_Uri_same, (EjsProc) uri_same);
    ejsBindMethod(ejs, prototype, ES_Uri_toString, (EjsProc) uri_toString);
    ejsBindMethod(ejs, prototype, ES_Uri_toLocalString, (EjsProc) uri_toLocalString);
    ejsBindMethod(ejs, prototype, ES_Uri_trimExt, (EjsProc) uri_trimExt);
    ejsBindAccess(ejs, prototype, ES_Uri_uri, (EjsProc) uri_toString, (EjsProc) uri_set_uri);

    ejsBindMethod(ejs, ejs->global, ES_decodeURI, (EjsProc) decodeURI);
    ejsBindMethod(ejs, ejs->global, ES_decodeURIComponent, (EjsProc) decodeURIComponent);
    ejsBindMethod(ejs, ejs->global, ES_encodeURI, (EjsProc) encodeURI);
    ejsBindMethod(ejs, ejs->global, ES_encodeURIComponent, (EjsProc) encodeURIComponent);
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
