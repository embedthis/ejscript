/*
    ejsUri.c - Uri class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Forwards ********************************/

static EjsUri *completeUri(Ejs *ejs, EjsUri *up, EjsObj *missing, int includeQuery);
static int same(Ejs *ejs, HttpUri *u1, HttpUri *u2, int exact);
static HttpUri *createHttpUriFromHash(Ejs *ejs, EjsObj *arg, int flags);
static HttpUri *toHttpUri(Ejs *ejs, EjsObj *arg, int dup);
static EjsUri *uri_join(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv);

/************************************ Helpers *********************************/
#if UNUSED && KEEP
//  keep this for when the cast helper is reversed
/*
    Convert an arg to a URI. Can handle strings, paths, URIs and object hashes. Will cast all else to strings and then
    parse.
 */
static EjsUri *castToUri(Ejs *ejs, EjsObj *arg)
{
    EjsUri  *up;

    up = ejsCreateObj(ejs, ESV(Uri), 0);
    if (ejsIs(ejs, arg, String)) {
        up->uri = httpCreateUri(up, ejsToMulti(ejs, arg), 0);

    } else if (ejsIs(ejs, arg, Uri)) {
        up->uri = httpCloneUri(((EjsUri*) arg)->uri, 0);

    } else if (ejsIs(ejs, arg, Path)) {
        ustr = ((EjsPath*) arg)->path;
        up->uri = httpCreateUri(up, ustr, 0);

    } else if (ejsGetLength(ejs, arg) > 0) {
        up->uri = createHttpUriFromHash(ejs, up, arg, 0);

    } else {
        arg = (EjsObj) ejsToString(ejs, arg);
        up->uri = httpCreateUri(up, ejsToMulti(ejs, arg), 0);
    }
    if (!up->uri) {
        ejsThrowArgError(ejs, "Invalid URI");
    }
    return up;
}
#endif


static EjsUri *cloneUri(Ejs *ejs, EjsUri *src, bool deep)
{
    EjsUri     *dest;

    if ((dest = ejsCreateObj(ejs, TYPE(src), 0)) == 0) {
        return 0;
    }
#if UNUSED
    /*  NOTE: a deep copy will complete the uri */
    dest->uri = httpCloneUri(src->uri, deep ? HTTP_COMPLETE_URI : 0);
#else
    dest->uri = httpCloneUri(src->uri, 0);
#endif
    return dest;
}


static EjsAny *coerceUriOperands(Ejs *ejs, EjsUri *lhs, int opcode,  EjsAny *rhs)
{
    HttpUri     *uri;
    char        *ustr;

    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        uri = lhs->uri;
        ustr = httpFormatUri(uri->scheme, uri->host, uri->port, uri->path, uri->reference, uri->query, 0);
        return ejsInvokeOperator(ejs, ejsCreateStringFromAsc(ejs, ustr), opcode, rhs);

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (!ejsIsDefined(ejs, rhs)) {
            return ((opcode == EJS_OP_COMPARE_EQ) ? ESV(false): ESV(true));
        }
        uri = lhs->uri;
        ustr = httpFormatUri(uri->scheme, uri->host, uri->port, uri->path, uri->reference, uri->query, 0);
        return ejsInvokeOperator(ejs, ejsCreateStringFromAsc(ejs, ustr), opcode, rhs);

    case EJS_OP_COMPARE_STRICTLY_NE:
        return ESV(true);

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return ESV(false);

    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_TRUE:
        return ESV(true);

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return ESV(false);

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return ESV(false);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return ESV(undefined);
    }
}


static EjsAny *invokeUriOperator(Ejs *ejs, EjsUri *lhs, int opcode,  EjsUri *rhs, void *data)
{
    EjsAny      *result;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if ((result = coerceUriOperands(ejs, lhs, opcode, rhs)) != 0) {
            return result;
        }
    }

    /*  Types now match, both Uris
     */
    switch (opcode) {
    case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_EQ:
        if (lhs == rhs || (lhs->uri == rhs->uri)) {
            return ESV(true);
        }
        return ejsCreateBoolean(ejs,  same(ejs, lhs->uri, rhs->uri, 1));

    case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_STRICTLY_NE:
        return ejsCreateBoolean(ejs,  !same(ejs, lhs->uri, rhs->uri, 1));

    /*  NOTE: these only compare the paths */
    case EJS_OP_COMPARE_LT:
        return ejsCreateBoolean(ejs,  scmp(lhs->uri->path, rhs->uri->path) < 0);

    case EJS_OP_COMPARE_LE:
        return ejsCreateBoolean(ejs,  scmp(lhs->uri->path, rhs->uri->path) <= 0);

    case EJS_OP_COMPARE_GT:
        return ejsCreateBoolean(ejs,  scmp(lhs->uri->path, rhs->uri->path) > 0);

    case EJS_OP_COMPARE_GE:
        return ejsCreateBoolean(ejs,  scmp(lhs->uri->path, rhs->uri->path) >= 0);

    /*  
        Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return ((lhs->uri->path) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_ZERO:
        return ((lhs->uri->path == 0) ? ESV(true): ESV(false));


    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return ESV(false);

    /*  
        Binary operators
     */
    case EJS_OP_ADD:
        return uri_join(ejs, lhs, 1, (EjsObj**) (void*) &rhs);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
    assert(0);
}


/************************************ Methods *********************************/

/*  
    Constructor
    function Uri(uri: Uri)
    function Uri(path: String)
    function Uri(parts: Object)
 */
static EjsUri *uri_constructor(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    if (argc >= 0) {
        up->uri = toHttpUri(ejs, argv[0], 1);
    }
    return up;
}


/*  
    Make an absolute reference for "this" URI.

    function absolute(base): Uri
 */
static EjsUri *uri_absolute(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *result;
    HttpUri     *uri, *baseUri;

    if (argc >= 1) {
        baseUri = toHttpUri(ejs, argv[0], 0);
        result = cloneUri(ejs, up, 0);
        uri = result->uri;
        if (uri->path && uri->path[0] != '/') {
            httpJoinUriPath(uri, baseUri, uri);
        }
        httpCompleteUri(result->uri, baseUri);
    } else {
        result = cloneUri(ejs, up, 0);
        httpCompleteUri(result->uri, NULL);
    }
    httpNormalizeUri(result->uri);
    return result;
}


/*  
    function get basename(): Uri
 */
static EjsUri *uri_basename(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *np;
    char        *path, *cp;
    int         len;

    np = cloneUri(ejs, up, 0);
    path = np->uri->path;
    if (path == 0) {
        return ESV(null);
    }
    len = (int) strlen(path);
    if (path[len - 1] == '/') {
        *path = '\0';
    } else {
        if ((cp = strrchr(path, '/')) != 0) {
            np->uri->path = &cp[1];
        }
    }
    return np;
}


/*  
    function complete(missing = null): Uri
 */
static EjsUri *uri_complete(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
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

    uri = up->uri;
    obj = ejsCreateEmptyPot(ejs);

    if (uri->scheme) {
        ejsSetPropertyByName(ejs, obj, EN("scheme"), ejsCreateStringFromAsc(ejs, uri->scheme));
    }
    if (uri->host) {
        ejsSetPropertyByName(ejs, obj, EN("host"), ejsCreateStringFromAsc(ejs, uri->host));
    }
    if (uri->port > 0) {
        ejsSetPropertyByName(ejs, obj, EN("port"), ejsCreateNumber(ejs, uri->port));
    }
    if (uri->path) {
        ejsSetPropertyByName(ejs, obj, EN("path"), ejsCreateStringFromAsc(ejs, uri->path));
    }
    if (uri->reference) {
        ejsSetPropertyByName(ejs, obj, EN("reference"), ejsCreateStringFromAsc(ejs, uri->reference));
    }
    if (uri->query) {
        ejsSetPropertyByName(ejs, obj, EN("query"), ejsCreateStringFromAsc(ejs, uri->query));
    }
    return obj;
}


/*  
    Decode a Uri
    static function decode(str: String): String
 */
static EjsString *uri_decode(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, mprUriDecode(ejsToMulti(ejs, argv[0])));
}


/*  
    Decode a Uri component
    static function decodeComponent(str: String): String
 */
static EjsString *uri_decodeComponent(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, mprUriDecode(ejsToMulti(ejs, argv[0])));
}


/*  
    function get dirname(): Uri
 */
static EjsUri *uri_dirname(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *np;
    char        *path, *cp;
    int         len;

    np = cloneUri(ejs, up, 0);
    path = np->uri->path;
    if (path == 0) {
        return ESV(null);
    }
    len = (int) strlen(path);
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
    return np;
}


/*  
    Uri Encode a string
    function encode(str: String): String
 */
static EjsString *uri_encode(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, mprUriEncode(ejsToMulti(ejs, argv[0]), MPR_ENCODE_URI));
}


/*  
    Encode a Uri component
    static function encodeComponent(str: String): String
 */
static EjsString *uri_encodeComponent(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, mprUriEncode(ejsToMulti(ejs, argv[0]), MPR_ENCODE_URI_COMPONENT));
}


/*  
    Get the Uri extension
    static function get extension(): String?
 */
static EjsString *uri_extension(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    if (up->uri->ext == 0) {
        return ESV(null);
    }    
    return ejsCreateStringFromAsc(ejs, up->uri->ext);
}


/*  
    Set the Uri extension
    static function set extension(ext: String?): Void
 */
static EjsObj *uri_set_extension(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    HttpUri     *uri;

    uri = up->uri;
    if (argv[0] == ESV(null)) {
        uri->ext = 0;
        uri->path = mprTrimPathExt(uri->path);
    } else {
        uri->ext = ejsToMulti(ejs, argv[0]);
        uri->path = sjoin(mprTrimPathExt(uri->path), uri->ext, NULL);
    }
    return 0;
}


/*  
    Determine if the uri has an extension
    static function get hasExtension(): Boolean
 */
static EjsBoolean *uri_hasExtension(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, up->uri->ext);
}


/*  
    Determine if the uri has a host
    static function get hasHost(): Boolean
 */
static EjsBoolean *uri_hasHost(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, up->uri->host);
}


/*  
    Determine if the uri has a port
    static function get hasPort(): Boolean
 */
static EjsBoolean *uri_hasPort(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, up->uri->port > 0);
}


/*  
    Determine if the uri has a query
    static function get hasQuery(): Boolean
 */
static EjsBoolean *uri_hasQuery(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, up->uri->query);
}


/*  
    Determine if the uri has a reference
    static function get hasReference(): Boolean
 */
static EjsBoolean *uri_hasReference(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, up->uri->reference);
}


/*  
    Determine if the uri has a scheme
    static function get hasScheme(): Boolean
 */
static EjsBoolean *uri_hasScheme(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, up->uri->scheme);
}


/*  
    Get the host portion
    static function get host(): String?
 */
static EjsString *uri_host(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    if (up->uri->host == 0) {
        return ESV(null);
    }    
    return ejsCreateStringFromAsc(ejs, up->uri->host);
}


/*  
    Set the host portion
    static function set host(name: String?): Void
 */
static EjsObj *uri_set_host(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    up->uri->host = (argv[0] == ESV(null)) ? 0 : ejsToMulti(ejs, argv[0]);
    return 0;
}


/*  
    function get isAbsolute(): Boolean
 */
static EjsBoolean *uri_isAbsolute(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, up->uri->path[0] == '/');
}


/*  
    Determine if the file name is a directory. This 
    function get isDir(): Boolean
 */
static EjsBoolean *uri_isDir(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    HttpUri     *uri;

    uri = up->uri;
    return ejsCreateBoolean(ejs, uri->path[strlen(uri->path) - 1] == '/');
}


/*  
    Join uri segments
    function join(...others): Uri
 */
static EjsUri *uri_join(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
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
        arg = ejsGetProperty(ejs, args, i);
        if ((other = toHttpUri(ejs, arg, 0)) == NULL) {
            return 0;
        }
        oldUri = uri;
        uri = httpJoinUri(oldUri, 1, &other);
    }
    result->uri = uri;
    return result;
}


/*  
    Join extension
    function joinExt(ext: String): Uri
 */
static EjsUri *uri_joinExt(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *np;
    HttpUri     *nuri;
    char        *ext;

    np = cloneUri(ejs, up, 1);
    nuri = np->uri;
    ext = ejsToMulti(ejs, argv[0]);
    if (ext && *ext == '.') {
        ext++;
    }
    nuri->ext = ext;
    nuri->path = sjoin(mprTrimPathExt(nuri->path), ".", nuri->ext, NULL);
    return np;
}


/*  
    function local(): Uri
 */
static EjsUri *uri_local(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *result;

    if ((result = cloneUri(ejs, up, 0)) != 0) {
        httpMakeUriLocal(result->uri);
    }
    return result;
}


/*  
    Get the mimeType
    function mimeType(): String?
 */
static EjsString *uri_mimeType(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    if (up->uri->ext == 0) {
        return ESV(null);
    }
    return ejsCreateStringFromAsc(ejs, mprLookupMime(NULL, up->uri->ext));
}


/*  
    function get normalize(): Uri
 */
static EjsUri *uri_normalize(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *np;

    np = cloneUri(ejs, up, 0);
    np->uri->path = httpNormalizeUriPath(up->uri->path);
    return np;
}


/*  
    Get the path portion
    static function get path(): String?
 */
static EjsString *uri_path(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    if (up->uri->path == 0) {
        return ESV(null);
    }    
    return ejsCreateStringFromAsc(ejs, up->uri->path);
}


/*  
    Set the path portion
    static function set path(path: String?): Void
 */
static EjsObj *uri_set_path(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    if (argv[0] == ESV(null)) {
        up->uri->path = 0;
        up->uri->ext = 0;
    } else {
        up->uri->path = httpNormalizeUriPath(ejsToMulti(ejs, argv[0]));
        up->uri->ext = mprGetPathExt(up->uri->path);
    }
    return 0;
}


/*  
    Get the port portion
    static function get port(): Number?
 */
static EjsNumber *uri_port(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    HttpUri     *uri;
    
    uri = up->uri;
    if (uri->port <= 0) {
        return ESV(null);
#if KEEP
        if (uri->host == 0) {
            return ESV(null);
        }
        if (uri->scheme == 0 || strcmp(uri->scheme, "http") == 0) {
            return ejsCreateNumber(ejs, 80);
        } else if (uri->scheme && strcmp(uri->scheme, "https") == 0) {
            return ejsCreateNumber(ejs, 443);
        }
#endif
    }
    return ejsCreateNumber(ejs, up->uri->port);
}


/*  
    Set the port portion
    static function set port(port: Number?): Void
 */
static EjsObj *uri_set_port(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    up->uri->port = (argv[0] == ESV(null)) ? 0 : ejsGetInt(ejs, argv[0]);
    return 0;
}


/*
    Get the reference portion
    static function get reference(): String?
 */
static EjsString *uri_reference(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    if (up->uri->reference == 0) {
        return ESV(null);
    }    
    return ejsCreateStringFromAsc(ejs, up->uri->reference);
}


/*  
    Set the reference portion
    static function set reference(reference: String?): Void
 */
static EjsObj *uri_set_reference(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    up->uri->reference = (argv[0] == ESV(null)) ? 0 : ejsToMulti(ejs, argv[0]);
    return 0;
}


/*  
    Replace the Uri extension
    static function set replaceExt(ext: String): Uri
 */
static EjsUri *uri_replaceExtension(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *np;
    HttpUri     *nuri;
    char        *ext;

    np = cloneUri(ejs, up, 1);
    nuri = np->uri;
    nuri->path = mprTrimPathExt(nuri->path);
    ext = ejsToMulti(ejs, argv[0]);
    if (ext && *ext == '.') {
        ext++;
    }
    nuri->ext = ext;
    nuri->path = sjoin(mprTrimPathExt(nuri->path), ".", nuri->ext, NULL);
    return np;
}


//  TODO - Uri should get a cast helper. Then this API and others can be typed.
/*  
    function resolve(target): Uri
 */
static EjsUri *uri_resolve(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *result;
    HttpUri     *uri, *target;

    uri = up->uri;
    target = toHttpUri(ejs, argv[0], 0);
    result = ejsCreateObj(ejs, ESV(Uri), 0);
    uri = httpResolveUri(uri, 1, &target, 0);
    if (up->uri == uri) {
        uri = httpCloneUri(uri, 0);
    }
    result->uri = uri;
    return result;
}


/*  
    function relative(base): Uri
 */
static EjsUri *uri_relative(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *result;
    HttpUri     *baseUri;

    baseUri = toHttpUri(ejs, argv[0], 0);
    result = ejsCreateObj(ejs, ESV(Uri), 0);
    result->uri = httpGetRelativeUri(baseUri, up->uri, 1);
    return result;
}


/*  
    Get the scheme portion
    static function get scheme(): String?
 */
static EjsString *uri_scheme(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    if (up->uri->scheme == 0) {
        return ESV(null);
    }
    return ejsCreateStringFromAsc(ejs, up->uri->scheme);
}


/*  
    Set the scheme portion
    static function set scheme(scheme: String?): Void
 */
static EjsObj *uri_set_scheme(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    up->uri->scheme = (argv[0] == ESV(null)) ? 0 : ejsToMulti(ejs, argv[0]);
    return 0;
}


/*  
    Get the query portion
    static function get query(): String?
 */
static EjsString *uri_query(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    if (up->uri->query == 0) {
        return ESV(null);
    }    
    return ejsCreateStringFromAsc(ejs, up->uri->query);
}


/*  
    Set the query portion
    static function set query(query: String): Void
 */
static EjsObj *uri_set_query(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    up->uri->query = (argv[0] == ESV(null)) ? 0 : ejsToMulti(ejs, argv[0]);
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
    exact = (argc == 2 && argv[1] == ESV(true));
    return ejsCreateBoolean(ejs, same(ejs, up->uri, other->uri, exact));
}


/*  
    Expand a template with {word} tokens from the given options objects

    function templateString(pattern: String, ...options): String
 */
static EjsString *uri_templateString(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsArray    *options;
    EjsObj      *obj, *value;
    MprBuf      *buf;
    cchar       *pattern, *cp, *ep, *str;
    char        *token;
    int         i, len;

    pattern = ejsToMulti(ejs, argv[0]);
    options = (EjsArray*) argv[1];

    buf = mprCreateBuf(-1, -1);
    for (cp = pattern; *cp; cp++) {
        if (*cp == '~' && (cp == pattern || cp[-1] != '\\')) {
            for (i = 0; i < options->length; i++) {
                obj = options->data[i];
                if ((value = ejsGetPropertyByName(ejs, obj, N(NULL, "scriptName"))) != 0 && ejsIsDefined(ejs, value)) {
                    str = ejsToMulti(ejs, value);
                    if (str && *str) {
                        mprPutStringToBuf(buf, str);
                        break;
                    } else {
                        value = 0;
                    }
                }
            }
        } else if (*cp == '{' && (cp == pattern || cp[-1] != '\\')) {
            if ((ep = strchr(++cp, '}')) != 0) {
                len = (int) (ep - cp);
                token = mprMemdup(cp, len + 1);
                token[len] = '\0';
                value = 0;
                for (i = 0; i < options->length; i++) {
                    obj = options->data[i];
                    if ((value = ejsGetPropertyByName(ejs, obj, N(NULL, token))) != 0 && ejsIsDefined(ejs, value)) {
                        str = ejsToMulti(ejs, value);
                        if (str && *str) {
                            mprPutStringToBuf(buf, str);
                            break;
                        } else {
                            value = 0;
                        }
                    }
                }
                if (!ejsIsDefined(ejs, value)) {
                    //  TODO - remove this. Should not be erasing the prior "/"
                    if (cp >= &pattern[2] && cp[-2] == '/') {
                        mprAdjustBufEnd(buf, -1);
                    }
                }
                cp = ep;
            }
        } else {
            mprPutCharToBuf(buf, *cp);
        }
    }
    mprAddNullToBuf(buf);
    return ejsCreateStringFromAsc(ejs, mprGetBufStart(buf));
}


static EjsUri *uri_template(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsString   *sp;

    if ((sp = uri_templateString(ejs, up, argc, argv)) != 0) {
        return ejsCreateUriFromAsc(ejs, ejsToMulti(ejs, sp));
    }
    return 0;
}


/* 
   function toString(): String
 */
static EjsString *uri_toString(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    HttpUri     *uri;

    uri = up->uri;
    return ejsCreateStringFromAsc(ejs, httpUriToString(uri, 0));
}


/* 
   function toLocalString(): String
 */
static EjsString *uri_toLocalString(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    HttpUri     *uri;

    uri = up->uri;
    return ejsCreateStringFromAsc(ejs, httpFormatUri(NULL, NULL, 0, uri->path, uri->reference, uri->query, 0));
}


/*  
    function trimExt(): Uri
 */
static EjsUri *uri_trimExt(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    EjsUri      *np;
    HttpUri     *nuri;

    np = cloneUri(ejs, up, 1);
    nuri = np->uri;
    nuri->ext = 0;
    nuri->path = mprTrimPathExt(nuri->path);
    return np;
}


/*  
    function set uri(value: String): Void
 */
static EjsObj *uri_set_uri(Ejs *ejs, EjsUri *up, int argc, EjsObj **argv)
{
    up->uri = httpCreateUri(ejsToMulti(ejs, argv[0]), 0);
    if (!up->uri) {
        ejsThrowArgError(ejs, "Invalid URI");
    }
    return 0;
}

/*********************************** Helpers **********************************/

#if UNUSED && KEEP
static char *uriToString(Ejs *ejs, EjsUri *up)
{
    HttpUri     *uri;

    uri = up->uri;
    return httpFormatUri(uri->scheme, uri->host, uri->port, uri->path, uri->reference, uri->query, 0);
}
#endif


static EjsUri *completeUri(Ejs *ejs, EjsUri *up, EjsObj *missing, int includeQuery)
{
    EjsUri      *missingUri;

    if (!ejsIsDefined(ejs, missing)) {
        missingUri = 0;
    } else if (ejsGetLength(ejs, missing) > 0) {
        missingUri = ejsCreateObj(ejs, ESV(Uri), 0);
        missingUri->uri = createHttpUriFromHash(ejs, missing, HTTP_COMPLETE_URI);
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
    return up;
}


static HttpUri *toHttpUri(Ejs *ejs, EjsObj *arg, int dup)
{
    HttpUri     *uri;

    if (!ejsIsDefined(ejs, arg)) {
        arg = ESV(empty);
    } 
    if (ejsIs(ejs, arg, String)) {
        uri = httpCreateUri(ejsToMulti(ejs, arg), 0);

    } else if (ejsIs(ejs, arg, Uri)) {
        if (dup) {
            uri = httpCloneUri(((EjsUri*) arg)->uri, 0);
        } else {
            uri = ((EjsUri*) arg)->uri;
        }

    } else if (ejsIs(ejs, arg, Path)) {
        uri = httpCreateUri(((EjsPath*) arg)->value, 0);

    } else if (ejsGetLength(ejs, arg) > 0) {
        uri = createHttpUriFromHash(ejs, arg, 0);

    } else {
        arg = (EjsObj*) ejsToString(ejs, arg);
        uri = httpCreateUri(ejsToMulti(ejs, arg), 0);
    }
    if (!uri) {
        ejsThrowArgError(ejs, "Invalid URI");
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


static HttpUri *createHttpUriFromHash(Ejs *ejs, EjsObj *arg, int flags)
{
    EjsObj      *schemeObj, *hostObj, *portObj, *pathObj, *referenceObj, *queryObj, *uriObj;
    cchar       *scheme, *host, *path, *reference, *query;
    int         port;

    /*
        This permits a uri property override. Used in ejs.web::View.getOptions()
     */
    uriObj = ejsGetPropertyByName(ejs, arg, EN("uri"));
    if (uriObj) {
        return toHttpUri(ejs, uriObj, 1);
    }
    schemeObj = ejsGetPropertyByName(ejs, arg, EN("scheme"));
    scheme = ejsIs(ejs, schemeObj, String) ? ejsToMulti(ejs, schemeObj) : 0;

    hostObj = ejsGetPropertyByName(ejs, arg, EN("host"));
    host = ejsIs(ejs, hostObj, String) ? ejsToMulti(ejs, hostObj) : 0;

    port = 0;
    if ((portObj = ejsGetPropertyByName(ejs, arg, EN("port"))) != 0) {
        if (ejsIs(ejs, portObj, Number)) {
            port = ejsGetInt(ejs, portObj);
        } else if (ejsIs(ejs, portObj, String)) {
            port = (int) stoi(ejsToMulti(ejs, portObj));
        }
    }
    pathObj = ejsGetPropertyByName(ejs, arg, EN("path"));
    path = ejsIs(ejs, pathObj, String) ? ejsToMulti(ejs, pathObj) : 0;

    referenceObj = ejsGetPropertyByName(ejs, arg, EN("reference"));
    reference = ejsIs(ejs, referenceObj, String) ? ejsToMulti(ejs, referenceObj) : 0;

    queryObj = ejsGetPropertyByName(ejs, arg, EN("query"));
    query = ejsIs(ejs, queryObj, String) ? ejsToMulti(ejs, queryObj) : 0;

    return httpCreateUriFromParts(scheme, host, port, path, reference, query, flags);
}


/*  
    Decode a Uri (ECMA Standard)

    function decodeURI(str: String): String
 */
static EjsObj *decodeURI(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringFromAsc(ejs, mprUriDecode(ejsToMulti(ejs, argv[0])));
}


/*  
    Decode a Uri component (ECMA Standard)
    function decodeURIComponent(str: String): String
 */
static EjsObj *decodeURIComponent(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringFromAsc(ejs, mprUriDecode(ejsToMulti(ejs, argv[0])));
}


/*  
    Uri Encode a string (ECMA Standard)
    function encodeURI(str: String): String
 */
static EjsObj *encodeURI(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringFromAsc(ejs, mprUriEncode(ejsToMulti(ejs, argv[0]), MPR_ENCODE_JS_URI));
}


/*  
    Encode a Uri component (ECMA Standard)
    static function encodeComponent(str: String): String
 */
static EjsObj *encodeURIComponent(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    char    *encoded;

    encoded = mprUriEncode(ejsToMulti(ejs, argv[0]), MPR_ENCODE_JS_URI_COMPONENT);
    return (EjsObj*) ejsCreateStringFromAsc(ejs, encoded);
}


/*********************************** Factory **********************************/

PUBLIC EjsUri *ejsCreateUri(Ejs *ejs, EjsString *path)
{
    EjsUri      *up;

    if ((up = ejsCreateObj(ejs, ESV(Uri), 0)) == NULL) {
        return 0;
    }
    uri_constructor(ejs, up, 1, (EjsObj**) (void*) &path);
    return up;
}


PUBLIC EjsUri *ejsCreateUriFromAsc(Ejs *ejs, cchar *path)
{
    EjsUri      *up;
    EjsObj      *arg;

    if ((up = ejsCreateObj(ejs, ESV(Uri), 0)) == 0) {
        return 0;
    }
    arg = (EjsObj*) ejsCreateStringFromAsc(ejs, path);
    uri_constructor(ejs, up, 1, (EjsObj**) &arg);
    return up;
}


PUBLIC EjsUri *ejsCreateUriFromParts(Ejs *ejs, cchar *scheme, cchar *host, int port, cchar *path, cchar *query, cchar *reference, 
    int flags)
{
    EjsUri      *up;

    if ((up = ejsCreateObj(ejs, ESV(Uri), 0)) == 0) {
        return 0;
    }
    up->uri = httpCreateUriFromParts(scheme, host, port, path, reference, query, flags);
    return up;
}


static void manageUri(EjsUri *up, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(up->uri);
    }
}


PUBLIC void ejsConfigureUriType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "Uri"), sizeof(EjsUri), manageUri,
            EJS_TYPE_OBJ | EJS_TYPE_MUTABLE_INSTANCES)) != 0) {
        type->helpers.clone = (EjsCloneHelper) cloneUri;
        //  TODO - Add cast helper to cast from Strings, Paths etc.
        type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeUriOperator;

        ejsBindMethod(ejs, type, ES_Uri_decode, uri_decode);
        ejsBindMethod(ejs, type, ES_Uri_decodeComponent, uri_decodeComponent);
        ejsBindMethod(ejs, type, ES_Uri_encode, uri_encode);
        ejsBindMethod(ejs, type, ES_Uri_encodeComponent, uri_encodeComponent);
        ejsBindMethod(ejs, type, ES_Uri_template, uri_template);
#if ES_Uri_templateString
        ejsBindMethod(ejs, type, ES_Uri_templateString, uri_templateString);
#endif

        prototype = type->prototype;
        ejsBindConstructor(ejs, type, uri_constructor);
        ejsBindMethod(ejs, prototype, ES_Uri_absolute, uri_absolute);
        ejsBindMethod(ejs, prototype, ES_Uri_basename, uri_basename);
        ejsBindMethod(ejs, prototype, ES_Uri_complete, uri_complete);
        ejsBindMethod(ejs, prototype, ES_Uri_components, uri_components);
        ejsBindMethod(ejs, prototype, ES_Uri_dirname, uri_dirname);
        ejsBindAccess(ejs, prototype, ES_Uri_extension, uri_extension, uri_set_extension);
        ejsBindMethod(ejs, prototype, ES_Uri_hasExtension, uri_hasExtension);
        ejsBindMethod(ejs, prototype, ES_Uri_hasHost, uri_hasHost);
        ejsBindMethod(ejs, prototype, ES_Uri_hasPort, uri_hasPort);
        ejsBindMethod(ejs, prototype, ES_Uri_hasQuery, uri_hasQuery);
        ejsBindMethod(ejs, prototype, ES_Uri_hasReference, uri_hasReference);
        ejsBindMethod(ejs, prototype, ES_Uri_hasScheme, uri_hasScheme);
        ejsBindAccess(ejs, prototype, ES_Uri_host, uri_host, uri_set_host);
        ejsBindMethod(ejs, prototype, ES_Uri_isAbsolute, uri_isAbsolute);
        ejsBindMethod(ejs, prototype, ES_Uri_isDir, uri_isDir);
        ejsBindMethod(ejs, prototype, ES_Uri_join, uri_join);
        ejsBindMethod(ejs, prototype, ES_Uri_joinExt, uri_joinExt);
        ejsBindMethod(ejs, prototype, ES_Uri_local, uri_local);
        ejsBindMethod(ejs, prototype, ES_Uri_mimeType, uri_mimeType);
        ejsBindMethod(ejs, prototype, ES_Uri_normalize, uri_normalize);
        ejsBindAccess(ejs, prototype, ES_Uri_path, uri_path, uri_set_path);
        ejsBindAccess(ejs, prototype, ES_Uri_port, uri_port, uri_set_port);
        ejsBindAccess(ejs, prototype, ES_Uri_scheme, uri_scheme, uri_set_scheme);
        ejsBindAccess(ejs, prototype, ES_Uri_query, uri_query, uri_set_query);
        ejsBindAccess(ejs, prototype, ES_Uri_reference, uri_reference, uri_set_reference);
        ejsBindMethod(ejs, prototype, ES_Uri_replaceExt, uri_replaceExtension);
        ejsBindMethod(ejs, prototype, ES_Uri_relative, uri_relative);
        ejsBindMethod(ejs, prototype, ES_Uri_resolve, uri_resolve);
        ejsBindMethod(ejs, prototype, ES_Uri_same, uri_same);
        ejsBindMethod(ejs, prototype, ES_Uri_toString, uri_toString);
        ejsBindMethod(ejs, prototype, ES_Uri_toLocalString, uri_toLocalString);
        ejsBindMethod(ejs, prototype, ES_Uri_trimExt, uri_trimExt);
        ejsBindAccess(ejs, prototype, ES_Uri_uri, uri_toString, uri_set_uri);
    }
    ejsBindMethod(ejs, ejs->global, ES_decodeURI, decodeURI);
    ejsBindMethod(ejs, ejs->global, ES_decodeURIComponent, decodeURIComponent);
    ejsBindMethod(ejs, ejs->global, ES_encodeURI, encodeURI);
    ejsBindMethod(ejs, ejs->global, ES_encodeURIComponent, encodeURIComponent);
}

/*
    @copy   default

    Copyright (c) Embedthis Software. All Rights Reserved.

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
