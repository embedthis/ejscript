/**
    ejsString.c - Ejscript string class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"
#include    "pcre.h"

/*********************************** Locals ***********************************/

//  TODO - should not be fixed
#if !defined(ME_MAX_REGEX_MATCHES)
    #define ME_MAX_REGEX_MATCHES 128
#endif

static int internHashSizes[] = {
     389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 0
};

/***************************** Forward Declarations ***************************/

static EjsString *buildString(Ejs *ejs, EjsString *result, wchar *str, ssize len);
static ssize indexof(wchar *str, ssize len, EjsString *pattern, ssize patternLength, int dir);
static void linkString(EjsString *head, EjsString *sp);
static void manageIntern(EjsIntern *intern, int flags);
static int rebuildIntern(EjsIntern *intern);
static void unlinkString(EjsString *sp);

/************************************* Code ***********************************/
/*
    Cast the string operand to a primitive type
 */
static EjsAny *castString(Ejs *ejs, EjsString *sp, EjsType *type)
{
    assert(sp);
    assert(type);

    switch (type->sid) {
    case S_Boolean:
        if (sp != ESV(empty)) {
            return ESV(true);
        }
        return ESV(false);

    case S_Number:
        return ejsParse(ejs, sp->value, S_Number);

    case S_Path:
        return ejsCreatePath(ejs, sp);

    case S_RegExp:
        return ejsParseRegExp(ejs, sp);

    case S_String:
        return sp;

    case S_Uri:
        return ejsCreateUri(ejs, sp);

    default:
        ejsThrowTypeError(ejs, "Cannot cast to required type");
        return 0;
    }
}


static EjsString *cloneString(Ejs *ejs, EjsString *sp, bool deep)
{
    /* Strings are immutable, interned and shared across all interps */
    return sp;
}


/*
    Get a string element. Slot numbers correspond to character indicies.
 */
static EjsString *getStringProperty(Ejs *ejs, EjsString *sp, int index)
{
    if (index < 0 || index >= sp->length) {
        return ESV(empty);
    }
    return ejsCreateString(ejs, &sp->value[index], 1);
}


static EjsAny *coerceStringOperands(Ejs *ejs, EjsAny *lhs, int opcode,  EjsAny *rhs)
{
    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsInvokeOperator(ejs, lhs, opcode, ejsToString(ejs, rhs));

    /*
        Overloaded operators
     */
    case EJS_OP_MUL:
        if (ejsIs(ejs, rhs, Number)) {
            return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, lhs, opcode, ejsToNumber(ejs, rhs));

    case EJS_OP_REM:
        return 0;

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_OR:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        return ejsInvokeOperator(ejs, lhs, opcode, ejsToString(ejs, rhs));

    case EJS_OP_COMPARE_STRICTLY_NE:
        return ESV(true);

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return ESV(false);

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT:
        return ejsInvokeOperator(ejs, ejsToBoolean(ejs, lhs), opcode, rhs);

    case EJS_OP_NOT:
    case EJS_OP_NEG:
        return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_TRUE:
        return (((EjsString*) lhs) ? ESV(true) : ESV(false));

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return (((EjsString*) lhs) ? ESV(false): ESV(true));

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return ESV(false);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return ESV(undefined);
    }
}


static EjsAny *invokeStringOperator(Ejs *ejs, EjsString *lhs, int opcode, EjsString *rhs, void *data)
{
    EjsAny  *result, *arg;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if (!ejsIs(ejs, lhs, String) || !ejsIs(ejs, rhs, String)) {
            if ((result = coerceStringOperands(ejs, lhs, opcode, rhs)) != 0) {
                return result;
            }
        }
    }
    /*
        Types now match, both strings
     */
    switch (opcode) {
    case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_EQ:
        //  OPT -- should use lhs == rhs
        if (lhs->value == rhs->value) {
            assert(lhs == rhs);
            return ESV(true);
        }
        return ESV(false);

    case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_STRICTLY_NE:
        //  OPT -- should use lhs == rhs
        if (lhs->value != rhs->value) {
            assert(lhs != rhs);
            return ESV(true);
        }
        return ESV(false);

    case EJS_OP_COMPARE_LT:
        return ejsCreateBoolean(ejs, 
            mprMemcmp(lhs->value, lhs->length * sizeof(wchar), rhs->value, rhs->length * sizeof(wchar)) < 0);

    case EJS_OP_COMPARE_LE:
        return ejsCreateBoolean(ejs, 
            mprMemcmp(lhs->value, lhs->length * sizeof(wchar), rhs->value, rhs->length * sizeof(wchar)) <= 0);

    case EJS_OP_COMPARE_GT:
        return ejsCreateBoolean(ejs, 
            mprMemcmp(lhs->value, lhs->length * sizeof(wchar), rhs->value, rhs->length * sizeof(wchar)) > 0);

    case EJS_OP_COMPARE_GE:
        return ejsCreateBoolean(ejs, 
                mprMemcmp(lhs->value, lhs->length * sizeof(wchar), rhs->value, rhs->length * sizeof(wchar)) >= 0);

    /*
        Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return ((lhs) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_ZERO:
        return ((lhs == 0) ? ESV(true): ESV(false));


    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return ESV(false);

    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsJoinString(ejs, lhs, rhs);

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_OR:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);

#if EXTENSIONS || 1
    /*
        Overloaded
     */
    case EJS_OP_SUB:
        arg = rhs;
        return ejsRunFunctionBySlot(ejs, lhs, ES_String_MINUS, 1, &arg);

    case EJS_OP_REM:
        arg = rhs;
        return ejsRunFunctionBySlot(ejs, lhs, ES_String_MOD, 1, &arg);
#endif

    case EJS_OP_NEG:
    case EJS_OP_LOGICAL_NOT:
    case EJS_OP_NOT:
        /* Already handled in coerceStringOperands */
    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
    assert(0);
}


/*
    Lookup an string index.
 */
static int lookupStringProperty(Ejs *ejs, EjsString *sp, EjsName qname)
{
    int     index;

    //  TODO UNICODE
    if (!isdigit((uchar) qname.name->value[0])) {
        return EJS_ERR;
    }
    index = ejsAtoi(ejs, qname.name, 10);
    if (index < sp->length) {
        return index;
    }
    return EJS_ERR;
}


/************************************ Methods *********************************/
/*
    String constructor

        function String()
        function String(str: String)
 */
static EjsString *stringConstructor(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsArray    *args;

    assert(argc == 0 || (argc == 1 && ejsIs(ejs, argv[0], Array)));
    sp->length = 0;

    if (argc == 1) {
        args = (EjsArray*) argv[0];
        if (args->length > 0) {
            return ejsToString(ejs, ejsGetProperty(ejs, args, 0));
        }
    }
    return ejsInternString(sp);
}


/*
    Do a case sensitive comparison between this string and another.

    function caseCompare(compare: String): Number
 */
static EjsNumber *caseCompare(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    int     result;

    assert(argc == 1 && ejsIs(ejs, argv[0], String));

    result = wcmp(sp->value, ((EjsString*) argv[0])->value);
    return ejsCreateNumber(ejs, result);
}


/*
    Do a case insensitive comparison between this string and another.

    function caselessCompare(compare: String): Number
 */
static EjsNumber *caselessCompare(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    int     result;

    assert(argc == 1 && ejsIs(ejs, argv[0], String));

    result = wcasecmp(sp->value, ((EjsString*) argv[0])->value);
    return ejsCreateNumber(ejs, result);
}


/*
    Return a string containing the character at a given index

    function charAt(index: Number): String
 */
static EjsString *charAt(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    int     index;

    assert(argc == 1 && ejsIs(ejs, argv[0], Number));
    index = ejsGetInt(ejs, argv[0]);
    if (index < 0 || index >= sp->length) {
        return ESV(empty);
    }
    return ejsCreateString(ejs, &sp->value[index], 1);
}


/*
    Return an integer containing the character at a given index

    function charCodeAt(index: Number = 0): Number
 */

static EjsNumber *charCodeAt(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    int     index;

    index = (argc == 1) ? ejsGetInt(ejs, argv[0]) : 0;
    if (index < 0) {
        index = (int) sp->length -1 ;
    }
    if (index < 0 || index >= sp->length) {
        return ESV(nan);
    }
    return ejsCreateNumber(ejs, (uchar) sp->value[index]);
}


/*
    Catenate args to a string and return a new string.

    function concat(...args): String
 */
static EjsString *concatString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsArray    *args;
    EjsString   *result, *str;
    int         i;

    assert(argc == 1 && ejsIs(ejs, argv[0], Array));
    args = (EjsArray*) argv[0];

    result = (EjsString*) ejsClone(ejs, sp, 0);
    for (i = 0; i < args->length; i++) {
        str = ejsToString(ejs, ejsGetProperty(ejs, args, i));
        if ((result = ejsJoinString(ejs, result, str)) == NULL) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
    }
    return result;
}


/**
    Check if a string contains the pattern (string or regexp)

    function contains(pattern: Object): Boolean
 */
static EjsBoolean *containsString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsObj      *pat;
    EjsString   *spat;

    pat = argv[0];

    if (ejsIs(ejs, pat, String)) {
        spat = (EjsString*) pat;
        return ejsCreateBoolean(ejs, wcontains(sp->value, spat->value) != 0);

    } else if (ejsIs(ejs, pat, RegExp)) {
        EjsRegExp   *rp;
        int         count;
        rp = (EjsRegExp*) argv[0];
        count = pcre_exec(rp->compiled, NULL, sp->value, (int) sp->length, 0, 0, 0, 0);
        return ejsCreateBoolean(ejs, count >= 0);
    }
    ejsThrowTypeError(ejs, "Wrong argument type");
    return 0;
}


/**
    Check if a string ends with a given pattern

    function endsWith(pattern: String): Boolean
 */
static EjsBoolean *endsWith(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString   *pattern;
    ssize       len;

    assert(argc == 1 && ejsIs(ejs, argv[0], String));

    pattern = (EjsString*) argv[0];
    len = pattern->length;
    if (len > sp->length) {
        return ESV(false);
    }
    return ejsCreateBoolean(ejs, wncmp(&sp->value[sp->length - len], pattern->value, len) == 0);
}


/*
    Lookup a token value. This will be called recursively for each name portion of a token. i.e. ${part.part.part}
 */
static int getTokenValue(Ejs *ejs, EjsObj *obj, cchar *fullToken, cchar *token, MprBuf *buf, EjsAny *missing, 
    EjsString *join)
{
    EjsAny      *vp;
    EjsString   *svalue;
    EjsName     qname;
    char        *rest, *first, *str;

    rest = (char*) (schr(token, '.') ? sclone(token) : token);
    first = ssplit(rest, ".", &rest);
    
    qname.name = ejsCreateStringFromAsc(ejs, first);
    qname.space = 0;
    if ((vp = ejsGetPropertyByName(ejs, obj, qname)) != 0) {
        if (rest && *rest && ejsIsPot(ejs, vp)) {
            return getTokenValue(ejs, vp, fullToken, rest, buf, missing, join);
        } else {
            if (ejsIs(ejs, vp, Array)) {
                svalue = ejsJoinArray(ejs, vp, join); 
            } else {
                svalue = ejsToString(ejs, vp);
            }
            mprPutStringToBuf(buf, svalue->value);
        }
    } else if (!missing || (vp == ESV(null) || vp == ESV(undefined))) {
        ejsThrowReferenceError(ejs, "Missing property %s", fullToken);
        return 0;
    } else if (missing == ESV(true)) {
        mprPutStringToBuf(buf, "${");
        mprPutStringToBuf(buf, fullToken);
        mprPutCharToBuf(buf, '}');
    } else if (missing == ESV(false)) {
        /* Omit */;
    } else {
        str = ejsToMulti(ejs, missing);
        //  DEPRECATE LEGACY
        if (smatch(str, "${}")) {
            mprPutStringToBuf(buf, "${");
            mprPutStringToBuf(buf, fullToken);
            mprPutCharToBuf(buf, '}');
        } else {
            mprPutStringToBuf(buf, str);
        }
    }
    return 1;
}


/*
    function expand(obj: Object, options: Object): String

    Options:
        missing: String to use for missing properties. 
            undefined - throw exception 
            true        Keep original token pattern ${token}
            false       Delete original token pattern
            string      Replace with string value

    Expand ${token} references. Tokens are be simple properties that may include "." references.

 */
static EjsString *expandString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    MprBuf      *buf;
    EjsAny      *missing, *obj, *options;
    EjsString   *join;
    char        *src, *cp, *tok;

    if (!ejsIs(ejs, sp, String)) {
        sp = ejsToString(ejs, sp);
    }
    missing = 0;
    join = ejsCreateStringFromAsc(ejs, " ");
    if (argc < 1) {
        ejsThrowArgError(ejs, "Missing object argument");
        return 0;
    }
    obj = argv[0];
    options = (argc >= 2) ? argv[1] : 0;
    if (options) {
        missing = ejsGetPropertyByName(ejs, options, EN("missing"));
        if (!missing) {
            //  LEGACY DEPRECATED
            missing = ejsGetPropertyByName(ejs, options, EN("fill"));
        }
    }
    if (sp->length == 0) {
        return ESV(empty);
    }
    //  UNICODE
    if (schr(sp->value, '$') == 0) {
        return sp;
    }
    buf = mprCreateBuf(0, 0);
    for (src = (char*) sp->value; src < &sp->value[sp->length]; ) {
        if (*src == '$' && src[1] == '{') {
            if (src > sp->value && src[-1] == '$') {
                for (++src; *src != '}' && src < &sp->value[sp->length]; ) {
                    mprPutCharToBuf(buf, *src++);
                }
            } else {
                src += 2;
                for (cp = src; *cp != '}' && cp < &sp->value[sp->length]; cp++) ;
                tok = snclone(src, cp - src);
                if (!getTokenValue(ejs, obj, tok, tok, buf, missing, join)) {
                    return 0;
                }
                src = cp + 1;
            }
        } else {
            mprPutCharToBuf(buf, *src++);
        }
    }
    mprAddNullToBuf(buf);
    return ejsCreateString(ejs, mprGetBufStart(buf), mprGetBufLength(buf));
}


PUBLIC EjsString *ejsExpandString(Ejs *ejs, EjsString *sp, EjsObj *tokens)
{
    return expandString(ejs, sp, 1, &tokens);
}


/**
    Format the arguments

    function format(...args): String

    Format:         %[modifier][width][precision][type]
    Modifiers:      +- #,
 */
static EjsString *formatString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsArray    *args, *inner;
    EjsString   *result;
    EjsAny      *value;
    wchar       *buf, fmt[32];
    ssize       i, flen, start, len, last;
    int         c, nextArg, kind;

    assert(argc == 1 && ejsIs(ejs, argv[0], Array));

    args = (EjsArray*) argv[0];

    /*
        Flatten the args if there is only one element and it is itself an array. This happens when invoked
        via the overloaded operator '%' which in turn invokes format()
     */
    if (args->length == 1) {
        inner = ejsGetProperty(ejs, args, 0);
        if (ejsIs(ejs, inner, Array)) {
            args = inner;
        }
    }
    if ((result = ejsCreateBareString(ejs, ME_MAX_BUFFER)) == NULL) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    result->length = 0;

    /*
        Parse the format string and extract one specifier at a time.
     */
    last = 0;
    for (i = 0, nextArg = 0; i < sp->length; i++) {
        c = sp->value[i];
        if (c != '%') {
            continue;
        }
        if (i > last) {
            result = buildString(ejs, result, &sp->value[last], i - last);
        }
        /*
            Find the end of the format specifier and determine the format type (kind)
         */
        start = i++;
        i += mspn(&sp->value[i], "-+ #,0*123456789.");
        kind = sp->value[i];

        if (strchr("cdefginopsSuxX", kind)) {
            len = i - start + 1;
            wncopy(fmt, (sizeof(fmt) / sizeof(wchar)) - 4, &sp->value[start], len);

            if (nextArg < args->length) {
                value = ejsGetProperty(ejs, args, nextArg);
            } else {
                value = ESV(null);
            }
            buf = 0;
            //  OPT
            switch (kind) {
            case 'o':
                value = ejsToNumber(ejs, value);
                flen = sizeof(fmt) - len + 1;
                mtow(&fmt[len - 1], flen, "Lo", 2);
                buf = wfmt(fmt, (int64) ejsGetNumber(ejs, value));
                break;
            case 'd': case 'i': case 'u':
                value = ejsToNumber(ejs, value);
                flen = sizeof(fmt) - len + 1;
                mtow(&fmt[len - 1], flen, ".0f", 3);
                buf = wfmt(fmt, ejsGetNumber(ejs, value));
                break;

            case 'e': case 'g': case 'f':
                value = ejsToNumber(ejs, value);
                buf = wfmt(fmt, ejsGetNumber(ejs, value));
                break;

            case 's':
                value = ejsToString(ejs, value);
                buf = wfmt(fmt, ejsToMulti(ejs, value));
                break;

            case 'X': case 'x':
                buf = wfmt(fmt, (int64) ejsGetNumber(ejs, value));
                break;

            case 'n':
                buf = wfmt(fmt, 0);
                break;
                    
            case 'c':
                buf = wfmt(fmt, (int) ejsGetNumber(ejs, value));
                break;

            default:
                ejsThrowArgError(ejs, "Bad format specifier");
                return 0;
            }
            result = buildString(ejs, result, buf, wlen(buf));
            last = i + 1;
            nextArg++;

        } else if (kind == '%') {
            wchar percent[1];
            percent[0] = '%';
            result = buildString(ejs, result, percent, 1);
            last = i + 1;
        }
    }
    if (sp->length > last) {
        result = buildString(ejs, result, &sp->value[last], sp->length - last);
    }
    return ejsInternString(result);
}


/*
    Create a string from character codes

    static function fromCharCode(...codes): String
 */
static EjsString *fromCharCode(Ejs *ejs, EjsString *unused, int argc, EjsObj **argv)
{
    EjsString   *result;
    EjsArray    *args;
    EjsObj      *vp;
    int         i;

    assert(argc == 1 && ejsIs(ejs, argv[0], Array));
    args = (EjsArray*) argv[0];

    if ((result = (EjsString*) ejsCreateBareString(ejs, argc + 1)) == NULL) {
        return 0;
    }
    for (i = 0; i < args->length; i++) {
        vp = ejsGetProperty(ejs, args, i);
        result->value[i] = ejsGetInt(ejs, ejsToNumber(ejs, vp));
    }
    result->value[i] = '\0';
    result->length = args->length;
    return ejsInternString(result);
}


/*
    Function to iterate and return the next character code.
    NOTE: this is not a method of String. Rather, it is a callback function for Iterator
 */
static EjsNumber *nextStringKey(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsString   *sp;

    sp = (EjsString*) ip->target;

    if (!ejsIs(ejs, sp, String)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }
    if (ip->index < sp->length) {
        return ejsCreateNumber(ejs, ip->index++);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return the default iterator. This returns the array index names.

    iterator function get(): Iterator
 */
static EjsIterator *getStringIterator(Ejs *ejs, EjsObj *sp, int argc, EjsObj **argv)
{
    return ejsCreateIterator(ejs, sp, -1, nextStringKey, 0, NULL);
}


/*
    Function to iterate and return the next string character (as a string).
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsString *nextStringValue(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsString   *sp;

    sp = (EjsString*) ip->target;
    if (!ejsIs(ejs, sp, String)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }

    if (ip->index < sp->length) {
        return ejsCreateString(ejs, &sp->value[ip->index++], 1);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return an iterator to return the next array element value.

    iterator function getValues(): Iterator
 */
static EjsIterator *getStringValues(Ejs *ejs, EjsObj *sp, int argc, EjsObj **argv)
{
    return ejsCreateIterator(ejs, sp, -1, nextStringValue, 0, NULL);
}


/*
    Get the length of a string.
    @return Returns the number of characters in the string

    override function get length(): Number
 */

static EjsNumber *stringLength(Ejs *ejs, EjsString *ap, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) ap->length);
}


/*
    Return the position of the first occurance of a substring

    function indexOf(pattern: String, startIndex: Number = 0): Number
 */
static EjsNumber *indexOf(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    EjsString   *pattern;
    ssize       start, index, patternLength;

    assert(1 <= argc && argc <= 2);
    assert(ejsIs(ejs, argv[0], String));

    pattern = (EjsString*) argv[0];
    patternLength = (int) pattern->length;

    if (argc == 2) {
        start = ejsGetInt(ejs, argv[1]);
        if (start > sp->length) {
            start = sp->length;
        }
        if (start < 0) {
            start = 0;
        }
    } else {
        start = 0;
    }
    index = indexof(&sp->value[start], sp->length - start, pattern, patternLength, 1);
    if (index < 0) {
        return ESV(minusOne);
    }
    return ejsCreateNumber(ejs, (MprNumber) (index + start));
}


static EjsBoolean *isAlpha(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    wchar       *cp;

    if (sp->length == 0) {
        return ESV(false);
    }
    for (cp = sp->value; cp < &sp->value[sp->length]; cp++) {
        if (*cp & 0x80 || !isalpha((uchar) *cp)) {
            return ESV(false);
        }
    }
    return ESV(true);
}


static EjsBoolean *isAlphaNum(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    wchar       *cp;

    if (sp->length == 0) {
        return ESV(false);
    }
    for (cp = sp->value; cp < &sp->value[sp->length]; cp++) {
        if (*cp & 0x80 || !isalnum((uchar) *cp)) {
            return ESV(false);
        }
    }
    return ESV(true);
}


static EjsBoolean *isDigit(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    wchar       *cp;

    if (sp->length == 0) {
        return ESV(false);
    }
    for (cp = sp->value; cp < &sp->value[sp->length]; cp++) {
        if (*cp & 0x80 || !isdigit((uchar) *cp)) {
            return ESV(false);
        }
    }
    return ESV(true);
}


static EjsBoolean *isLower(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    wchar       *cp;

    if (sp->length == 0) {
        return ESV(false);
    }
    for (cp = sp->value; cp < &sp->value[sp->length]; cp++) {
        if (!islower((int) *cp)) {
            return ESV(false);
        }
    }
    return ESV(true);
}


static EjsBoolean *isSpace(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    wchar       *cp;

    if (sp->length == 0) {
        return ESV(false);
    }
    for (cp = sp->value; cp < &sp->value[sp->length]; cp++) {
        if (*cp & 0x80 || !isspace((uchar) *cp)) {
            return ESV(false);
        }
    }
    return ESV(true);
}


static EjsBoolean *isUpper(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    wchar       *cp;

    if (sp->length == 0) {
        return ESV(false);
    }
    for (cp = sp->value; cp < &sp->value[sp->length]; cp++) {
        if (!isupper((int) *cp)) {
            return ESV(false);
        }
    }
    return ESV(true);
}


/*
    Return the position of the last occurance of a substring

    function lastIndexOf(pattern: String, start: Number = -1): Number
 */
static EjsNumber *lastIndexOf(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    EjsString   *pattern;
    ssize       start, patternLength, index;

    assert(1 <= argc && argc <= 2);

    pattern = (EjsString*) argv[0];
    patternLength = (int) pattern->length;

    if (argc == 2) {
        start = ejsGetInt(ejs, argv[1]);
        if (start >= (int) sp->length) {
            start = (int) sp->length - 1;
        }
        if (start < 0) {
            start = 0;
        }
    } else {
        start = 0;
    }
    index = indexof(sp->value, sp->length, pattern, patternLength, -1);
    if (index < 0) {
        return ESV(minusOne);
    }
    return ejsCreateNumber(ejs, (MprNumber) index);
}


/*
    Match a pattern

    function match(pattern: RegExp): Array
 */
static EjsArray *match(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsRegExp   *rp;
    EjsArray    *results;
    EjsString   *match;
    int         matches[ME_MAX_REGEX_MATCHES * 3];
    int         i, count, len, resultCount;

    rp = (EjsRegExp*) argv[0];
    rp->endLastMatch = 0;
    results = NULL;
    resultCount = 0;

    do {
        count = pcre_exec(rp->compiled, NULL, sp->value, (int) sp->length, rp->endLastMatch, 0, matches, 
            sizeof(matches) / sizeof(int));
        if (count <= 0) {
            break;
        }
        if (results == 0) {
            results = ejsCreateArray(ejs, count);
        }
        for (i = 0; i < count * 2; i += 2) {
            len = matches[i + 1] - matches[i];
            match = ejsCreateString(ejs, &sp->value[matches[i]], len);
            ejsSetProperty(ejs, results, resultCount++, match);
            if (rp->global) {
                if (matches[1] == rp->endLastMatch) {
                    rp->endLastMatch++;
                } else {
                    rp->endLastMatch = matches[1];
                }
                break;
            }
        }
    } while (rp->global);
    if (results == NULL) {
        return ESV(null);
    }
    return results;
}


static EjsString *printable(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString       *result;
    wchar           buf[16];
    ssize           len;
    int             i, j, k, nonprint;

    nonprint = 0;
    for (i = 0; i < sp->length; i++)  {
        if (!isprint((uchar) sp->value[i])) {
            nonprint++;
        }
    }
    if (nonprint == 0) {
        return sp;
    }
    if ((result = ejsCreateBareString(ejs, sp->length + (nonprint * 6) + 1)) == NULL) {
        return 0;
    }
    for (i = 0, j = 0; i < sp->length; i++)  {
        if (isprint((uchar) sp->value[i])) {
            result->value[j++] = sp->value[i];
        } else {
            result->value[j++] = '\\';
            result->value[j++] = 'u';
            itosbuf(buf, 4, (uchar) sp->value[i], 16);
            len = wlen(buf);
            for (k = (int) len; k < 4; k++) {
                result->value[j++] = '0';
            }
            for (k = 0; buf[k]; k++) {
                result->value[j++] = buf[k];
            }
        }
    }
    result->value[j] = '\0';
    result->length = j;
    return ejsInternString(result);
}


static EjsString *quote(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString       *result;

    if ((result = ejsCreateBareString(ejs, sp->length + 2)) == NULL) {
        return 0;
    }
    memcpy(&result->value[1], sp->value, sp->length);
    result->value[0] = '"';
    result->value[sp->length + 1] = '"';
    result->value[sp->length + 2] = '\0';
    result->length = sp->length + 2;
    return ejsInternString(result);
}


/*
    Remove characters and return a new string.

    function remove(start: Number, end: Number = -1): String

 */
static EjsString *removeCharsFromString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString       *result;
    ssize           start, end, i, j;

    assert(1 <= argc && argc <= 2);

    start = ejsGetInt(ejs, argv[0]);
    end = ejsGetInt(ejs, argv[1]);

    if (start < 0) {
        start += sp->length;
    }
    if (start < 0) {
        start = 0;
    }
    if (end < 0) {
        end += sp->length;
    }
    if (end < 0) {
        end = 0;
    }
    if (start >= sp->length) {
        start = sp->length - 1;
    }
    if (end > sp->length) {
        end = sp->length;
    }
    if ((result = ejsCreateBareString(ejs, sp->length - (end - start))) == NULL) {
        return 0;
    }
    for (j = i = 0; i < start; i++, j++) {
        result->value[j] = sp->value[i];
    }
    for (i = end; i < sp->length; i++, j++) {
        result->value[j] = sp->value[i];
    }
    result->value[j] = '\0';
    return ejsInternString(result);
}


static EjsString *getReplacementText(Ejs *ejs, EjsFunction *fn, int count, int *matches, EjsString *sp)
{
    EjsAny  *result, *argv[ME_MAX_REGEX_MATCHES * 3];
    int     i, offset, argc;

    assert(fn);
    
    argc = 0;
    argv[argc++] = ejsCreateString(ejs, &sp->value[matches[0]], matches[1] - matches[0]);
    for (i = 1; i < count; i++) {
        offset = i * 2;
        argv[argc++] = ejsCreateString(ejs, &sp->value[matches[offset]], matches[offset + 1] - matches[offset]);
    }
    argv[argc++] = ejsCreateNumber(ejs, matches[0]);
    argv[argc++] = sp;
    if ((result = ejsRunFunction(ejs, fn, ejs->global, argc, argv)) == 0) {
        return 0;
    }
    return (EjsString*) ejsToString(ejs, result);
}


/*
    Search and replace

    function replace(pattern: (String|Regexp), replacement: (String|Function)): String
 */
static EjsString *replace(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString   *result, *replacement, *pattern;
    EjsFunction *replacementFunction;
    wchar       cbuf[1];
    ssize       patternLength, index;
    int         matches[ME_MAX_REGEX_MATCHES * 3], enabled;

    result = 0;
    if (argc == 1) {
        replacement = ESV(empty);
        replacementFunction = 0;
    } else if (ejsIsFunction(ejs, argv[1])) {
        replacementFunction = (EjsFunction*) argv[1];
        replacement = 0;
    } else {
        replacement = (EjsString*) ejsToString(ejs, argv[1]);
        replacementFunction = 0;
    }
    if (ejsIs(ejs, argv[0], RegExp)) {
        EjsRegExp   *rp;
        wchar       *cp, *lastReplace, *end;
        ssize       endLastMatch, startNextMatch, submatch;
        int         count;

        rp = (EjsRegExp*) argv[0];

        result = ejsCreateBareString(ejs, ME_MAX_BUFFER);
        result->length = 0;
        startNextMatch = endLastMatch = 0;
        do {
            if (startNextMatch > sp->length) {
                break;
            }
            count = pcre_exec(rp->compiled, NULL, sp->value, (int) sp->length, (int) startNextMatch, 0, matches,
                    sizeof(matches) / sizeof(int));
            if (count <= 0) {
                break;
            }
            if (endLastMatch < matches[0]) {
                /* Append prior string text */
                result = buildString(ejs, result, &sp->value[endLastMatch], matches[0] - endLastMatch);
            }
            /*
                Process the replacement template
             */
            if (replacementFunction) {
                enabled = mprEnableGC(0);
                replacement = getReplacementText(ejs, replacementFunction, count, matches, sp);
                mprEnableGC(enabled);
            }
            end = &replacement->value[replacement->length];
            lastReplace = replacement->value;

            for (cp = replacement->value; cp < end; ) {
                if (*cp == '$') {
                    if (lastReplace < cp) {
                        result = buildString(ejs, result, lastReplace, (int) (cp - lastReplace));
                    }
                    switch (*++cp) {
                    case '$':
                        cbuf[0] = '$';
                        result = buildString(ejs, result, cbuf, 1);
                        break;
                    case '&':
                        /* Replace the matched string */
                        result = buildString(ejs, result, &sp->value[matches[0]], matches[1] - matches[0]);
                        break;
                    case '`':
                        /* Insert the portion that preceeds the matched string */
                        result = buildString(ejs, result, sp->value, matches[0]);
                        break;
                    case '\'':
                        /* Insert the portion that follows the matched string */
                        result = buildString(ejs, result, &sp->value[matches[1]], sp->length - matches[1]);
                        break;
                    default:
                        //  BUG should handle backquoted $
                        /* Insert the nth submatch */
                        if (isdigit((uchar) *cp)) {
                            submatch = (int) wtoi(cp);
                            while (isdigit((uchar) *++cp))
                                ;
                            cp--;
                            if (submatch < count) {
                                submatch *= 2;
                                result = buildString(ejs, result, 
                                    &sp->value[matches[submatch]], matches[submatch + 1] - matches[submatch]);
                            }
                        } else {
                            ejsThrowArgError(ejs, "Bad replacement $ specification");
                            return 0;
                        }
                    }
                    lastReplace = cp + 1;
                }
                cp++;
            }
            if (lastReplace < cp && lastReplace < end) {
                result = buildString(ejs, result, lastReplace, (int) (cp - lastReplace));
            }
            endLastMatch = matches[1];
            startNextMatch = endLastMatch;
            if (matches[0] == matches[1]) {
                /* 
                    A pattern may match but have no length. Step over the position always.
                    E.g. multline matching ^ or $, or an optional pattern
                 */
                startNextMatch++;
            }
        } while (rp->global);

        if (endLastMatch < sp->length) {
            /* Append remaining string text */
            result = buildString(ejs, result, &sp->value[endLastMatch], sp->length - endLastMatch);
        }
    } else {
        pattern = ejsToString(ejs, argv[0]);
        patternLength = pattern->length;
        index = indexof(sp->value, sp->length, pattern, patternLength, 1);
        if (index >= 0) {
            if ((result = ejsCreateBareString(ejs, ME_MAX_BUFFER)) == NULL) {
                return 0;
            }
            result->length = 0;
            result = buildString(ejs, result, sp->value, index);
            if (replacementFunction) {
                matches[0] = matches[2] = (int) index;
                matches[1] = matches[3] = (int) (index + patternLength);
                enabled = mprEnableGC(0);
                replacement = getReplacementText(ejs, replacementFunction, 2, matches, sp);
                mprEnableGC(enabled);
            }
            result = buildString(ejs, result, replacement->value, replacement->length);
            index += patternLength;
            if (index < sp->length) {
                result = buildString(ejs, result, &sp->value[index], sp->length - index);
            }
        } else {
            result = ejsClone(ejs, sp, 0);
        }
    }
    return ejsInternString(result);
}


static EjsString *reverseString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString   *rp;
    wchar       *cp;
    ssize       i;

    if (sp->length <= 1) {
        return sp;
    }
    if ((rp = ejsCreateBareString(ejs, sp->length)) == 0) {
        return 0;
    }
    cp = rp->value;
    for (i = sp->length - 1; i >= 0; i--) {
        *cp++ = sp->value[i];
    }
    return ejsInternString(rp);
}


/*
    Search for a pattern
    function search(pattern: (String | RegExp)): Number
 */
static EjsNumber *searchString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsRegExp   *rp;
    EjsString   *pattern;
    ssize       index, patternLength;
    int         count, matches[ME_MAX_REGEX_MATCHES * 3];

    if (ejsIs(ejs, argv[0], String)) {
        pattern = (EjsString*) argv[0];
        patternLength = (int) pattern->length;
        index = indexof(sp->value, sp->length, pattern, patternLength, 1);
        return ejsCreateNumber(ejs, (MprNumber) index);

    } else if (ejsIs(ejs, argv[0], RegExp)) {
        rp = (EjsRegExp*) argv[0];
        count = pcre_exec(rp->compiled, NULL, sp->value, (int) sp->length, 0, 0, matches, sizeof(matches) / sizeof(int));
        if (count < 0) {
            return ESV(minusOne);
        }
        return ejsCreateNumber(ejs, matches[0]);

    } else {
        ejsThrowTypeError(ejs, "Wrong argument type");
    }
    return 0;
}


/*
    Return a substring. End is one past the last character.

    function slice(start: Number, end: Number = -1, step: Number = 1): String
 */
static EjsString *sliceString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString       *result;
    ssize           start, end, step, i, j, size;

    assert(1 <= argc && argc <= 3);

    start = ejsGetInt(ejs, argv[0]);
    if (argc >= 2) {
        end = ejsGetInt(ejs, argv[1]);
    } else {
        end = sp->length;
    }
    if (argc == 3) {
        step = ejsGetInt(ejs, argv[2]);
    } else {
        step = 1;
    }
    if (start < 0) {
        start += sp->length;
    }
    if (start < 0) {
        start = 0;
    } else if (start >= sp->length) {
        start = sp->length;
    }
    if (end < 0) {
        end += sp->length;
    }
    if (end < 0) {
        end = 0;
    } if (end >= sp->length) {
        end = sp->length;
    }
    if (step == 0) {
        step = 1;
    }
    size = (start < end) ? end - start : start - end;
    if ((result = ejsCreateBareString(ejs, ((size / (ssize) abs(((int) step))) + 1))) == NULL) {
        return 0;
    }
    if (step > 0) {
        for (i = start, j = 0; i < end; i += step) {
            result->value[j++] = sp->value[i];
        }
    } else {
        for (i = start, j = 0; i > end; i += step) {
            result->value[j++] = sp->value[i];
        }
    }
    result->value[j] = '\0';
    result->length = j;
    return ejsInternString(result);
}


/*
    Split a string

    function split(delimiter: (String | RegExp), limit: Number = -1): Array
 */
static EjsArray *split(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsArray    *results;
    EjsString   *elt, *delim;
    wchar       *cp, *mark, *end;
    ssize      limit;

    assert(1 <= argc && argc <= 2);

    limit = (argc == 2) ? ejsGetInt(ejs, argv[1]): MAXSSIZE;
    results = ejsCreateArray(ejs, 0);

    if (ejsIs(ejs, argv[0], String)) {
        delim = (EjsString*) argv[0];
        if (delim->length == 0) {
            for (cp = sp->value; (--limit != -1) && *cp; cp++) {
                ejsSetProperty(ejs, results, -1, ejsCreateString(ejs, cp, 1));
            }
        } else {
            end = &sp->value[sp->length];
            for (mark = cp = sp->value; limit > 0 && mark < end; mark++, limit--) {
                if (wncmp(mark, delim->value, delim->length) == 0) {
                    elt = ejsCreateString(ejs, cp, (int) (mark - cp));
                    ejsSetProperty(ejs, results, -1, elt);
                    cp = mark + delim->length;
                    mark = cp - 1;
                }
            }
            elt = ejsCreateString(ejs, cp, (int) (mark - cp));
            ejsSetProperty(ejs, results, -1, elt);
        }
        return results;

    } else if (ejsIs(ejs, argv[0], RegExp)) {
        EjsRegExp   *rp;
        EjsString   *match;
        int         matches[ME_MAX_REGEX_MATCHES * 3], count, resultCount;
        
        rp = (EjsRegExp*) argv[0];
        rp->endLastMatch = 0;
        resultCount = 0;
        while (1) {
            count = (int) pcre_exec(rp->compiled, NULL, sp->value, (int) sp->length, rp->endLastMatch, 0, matches, 
                sizeof(matches) / sizeof(int));
            if (count <= 0) {
                break;
            }
            match = ejsCreateString(ejs, &sp->value[rp->endLastMatch], matches[0] - rp->endLastMatch);
            ejsSetProperty(ejs, results, resultCount++, match);
            if (matches[1] == rp->endLastMatch) {
                /* Catch patterns that match the empty string - prevents infinite loop */
                rp->endLastMatch = matches[1] + 1;
            } else {
                rp->endLastMatch = matches[1];
            }
        }
        if (rp->endLastMatch < sp->length) {
            match = ejsCreateString(ejs, &sp->value[rp->endLastMatch], sp->length - rp->endLastMatch);
            ejsSetProperty(ejs, results, resultCount, match);
        } else {
            ejsSetProperty(ejs, results, resultCount, ESV(empty));
        }
        return results;
    }

    ejsThrowTypeError(ejs, "Wrong argument type");
    return 0;
}


/**
    Check if a string starts with a given pattern

    function startsWith(pattern: String): Boolean
 */
static EjsBoolean *startsWith(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString   *pattern;

    assert(argc == 1 && ejsIs(ejs, argv[0], String));

    pattern = (EjsString*) argv[0];
    return ejsCreateBoolean(ejs, wncmp(&sp->value[0], pattern->value, pattern->length) == 0);
}


/*
    Extract a substring. Simple routine with positive indicies.

    function substring(start: Number, end: Number = -1): String
 */
static EjsString *substring(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    ssize   start, end, tmp;

    start = ejsGetInt(ejs, argv[0]);
    if (argc == 2) {
        end = ejsGetInt(ejs, argv[1]);
    } else {
        end = sp->length;
    }
    if (start < 0) {
        start = 0;
    }
    if (start >= sp->length) {
        start = sp->length - 1;
    }
    if (end < 0) {
        end = sp->length;
    }
    if (end > sp->length) {
        end = sp->length;
    }
    /*
        Swap if start is bigger than end
     */
    if (start > end) {
        tmp = start;
        start = end;
        end = tmp;
    }
    return ejsSubstring(ejs, sp, start, (int) (end - start));
}


/*
    Convert the string to camelCase. Return a new string.

    function toCamel(): String
 */
static EjsString *toCamel(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString   *result;

    if ((result = ejsCreateBareString(ejs, sp->length)) == 0) {
        return 0;
    }
    memcpy(result->value, sp->value, sp->length * sizeof(wchar));
    result->value[0] = tolower((uchar) sp->value[0]);
    return ejsInternString(result);
}


/*
    Convert a string to a literal string representation. Wrap in quotes and backquote any quotes or backquotes.
 */
PUBLIC EjsString *ejsToLiteralString(Ejs *ejs, EjsObj *vp)
{
    EjsString   *sp;
    MprBuf      *buf;
    int         i, c;

    if (ejsIs(ejs, vp, String)) {
        sp = (EjsString*) vp;
    } else {
        sp = ejsToString(ejs, vp);
    }
    buf = mprCreateBuf(0, 0);
    mprPutCharToBuf(buf, '"');
    for (i = 0; i < sp->length; i++) {
        c = sp->value[i];
        if (c == '"' || c == '\\') {
            mprPutCharToBuf(buf, '\\');
            mprPutCharToBuf(buf, c);
        } else {
            mprPutCharToBuf(buf, c);
        }
    }
    mprPutCharToBuf(buf, '"');
    mprAddNullToBuf(buf);
    return ejsCreateStringFromAsc(ejs, mprGetBufStart(buf));
}


/*
    Convert the string to lower case.

    function toLowerCase(locale: String = null): String
 */
static EjsString *toLowerCase(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    return ejsToLower(ejs, sp);
}


/*
    Convert the string to PascalCase. Return a new string.

    function toPascal(): String
 */
static EjsString *toPascal(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString   *result;

    if ((result = ejsCreateBareString(ejs, sp->length)) == 0) {
        return 0;
    }
    memcpy(result->value, sp->value, sp->length * sizeof(wchar));
    result->value[0] = toupper((uchar) sp->value[0]);
    return ejsInternString(result);
}


/*
    Convert to a string

    override function toString(): String
 */
static EjsString *stringToString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    return sp;
}


/*
    Convert the string to upper case.
    @return Returns a new upper case version of the string.
    @spec ejs-11

    function toUpperCase(locale: String = null): String
 */
static EjsString *toUpperCase(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    return ejsToUpper(ejs, sp);
}


/*
    Scan the input and tokenize according to the format string

    function tokenize(format: String): Array
 */
static EjsArray *tokenize(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsArray    *result;
    wchar       *cp, *buf;
    cchar       *fmt;

    assert(argc == 1 && ejsIs(ejs, argv[0], String));

    buf = sp->value;
    fmt = ejsToMulti(ejs, argv[0]);
    result = ejsCreateArray(ejs, 0);

    for (fmt = ejsToMulti(ejs, argv[0]); *fmt && buf < &sp->value[sp->length]; ) {
        if (*fmt++ != '%') {
            continue;
        }
        switch (*fmt) {
        case 's':
            for (cp = buf; *cp; cp++) {
                if (isspace((uchar) *cp)) {
                    break;
                }
            }
            ejsSetProperty(ejs, result, -1, ejsSubstring(ejs, sp, (int) (buf - sp->value), (int) (cp - buf)));
            buf = cp;
            break;

        case 'd':
            ejsSetProperty(ejs, result, -1, ejsParse(ejs, buf, S_Number));
            while (*buf && !isspace((uchar) *buf)) {
                buf++;
            }
            break;

        default:
            ejsThrowArgError(ejs, "Bad format specifier");
            return 0;
        }
        while (*buf && isspace((uchar) *buf)) {
            buf++;
        }
    }
    return result;
}


static EjsString *trim(Ejs *ejs, EjsString *sp, EjsString *pattern, int where)
{
    wchar       *start, *end, *mark;
    ssize       index, patternLength;

    if (pattern == 0) {
        start = sp->value;
        if (where & MPR_TRIM_START) {
            for (; start < &sp->value[sp->length]; start++) {
                if (!isspace((uchar) *start)) {
                    break;
                }
            }
        }
        end = &sp->value[sp->length - 1];
        if (where & MPR_TRIM_END) {
            for (end = &sp->value[sp->length - 1]; end >= start; end--) {
                if (!isspace((uchar) *end)) {
                    break;
                }
            }
        }
        end++;
    } else {
        patternLength = pattern->length;
        if (patternLength <= 0 || patternLength > sp->length) {
            return sp;
        }
        mark = sp->value;
        if (where & MPR_TRIM_START) {
            for (; &mark[patternLength] <= &sp->value[sp->length]; mark += patternLength) {
                index = indexof(mark, patternLength, pattern, patternLength, 1);
                if (index != 0) {
                    break;
                }
            }
        }
        start = mark;

        mark = &sp->value[sp->length - patternLength];
        if (where & MPR_TRIM_END) {
            for (; mark >= sp->value; mark -= patternLength) {
                index = indexof(mark, patternLength, pattern, patternLength, 1);
                if (index != 0) {
                    break;
                }
            }
        }
        end = mark + patternLength;
    }
    return ejsSubstring(ejs, sp, (int) (start - sp->value), (int) (end - start));
}


/**
    Returns a trimmed copy of the string. Normally used to trim white space, but can be used to trim any substring
    from the start or end of the string.
    @param str May be set to a substring to trim from the string. If not set, it defaults to any white space.
    @return Returns a (possibly) modified copy of the string

    function trim(str: String = null): String
 */
static EjsString *trimString(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    EjsString   *pattern;

    if (argc == 0) {
        return trim(ejs, sp, NULL, MPR_TRIM_START | MPR_TRIM_END);

    } else {
        pattern = (EjsString*) ejsToString(ejs, argv[0]);
        return trim(ejs, sp, pattern, MPR_TRIM_START | MPR_TRIM_END);
    }
}


/*  
    function trimStart(str: String = null): String
 */
static EjsString *trimStartString(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    EjsString   *pattern;

    assert(argc == 0 || (argc == 1 && ejsIs(ejs, argv[0], String)));

    if (argc == 0) {
        return trim(ejs, sp, NULL, MPR_TRIM_START);

    } else {
        pattern = (EjsString*) argv[0];
        return trim(ejs, sp, pattern, MPR_TRIM_START);
    }
}


/*  
    function trimEnd(str: String = null): String
 */
static EjsString *trimEndString(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    EjsString   *pattern;

    assert(argc == 0 || (argc == 1 && ejsIs(ejs, argv[0], String)));

    if (argc == 0) {
        return trim(ejs, sp, NULL, MPR_TRIM_END);

    } else {
        pattern = (EjsString*) argv[0];
        return trim(ejs, sp, pattern, MPR_TRIM_END);
    }
}

/*********************************** Support **********************************/
#if UNUSED
/**
    Fast append a string. This modifies the original "dest" string. BEWARE: strings are meant to be immutable.
    Only use this when constructing strings.
 */
static int catString(Ejs *ejs, EjsString *dest, char *str, ssize len)
{
    EjsString   *castSrc;
    char        *oldBuf, *buf;
    int         oldLen, newLen;

    assert(dest);

    castSrc = 0;

    oldBuf = dest->value;
    oldLen = dest->length;
    newLen = oldLen + len + 1;

#if FUTURE
    if (newLen < MPR_SLAB_STR_MAX) {
        buf = oldBuf;
    } else {
#endif
        buf = (char*) mprRealloc(oldBuf, newLen);
        if (buf == 0) {
            return -1;
        }
        dest->value = buf;
#if FUTURE
    }
#endif
    memcpy(&buf[oldLen], str, len);
    dest->length += len;
    buf[dest->length] = '\0';

    return 0;
}
#endif


/*
    Append the given string to the result
 */
static EjsString *buildString(Ejs *ejs, EjsString *result, wchar *str, ssize len)
{
    EjsString   *newBuf;
    ssize       room, size, extra;

    assert(result);

    room = mprGetBlockSize(result) - sizeof(EjsString);
    if ((result->length + len + 1) >= room) {
        extra = max(result->length / 8, len);
        size = max(result->length + extra, 512);
        if ((newBuf = ejsCreateBareString(ejs, size)) == NULL) {
            return NULL;
        }
        memcpy(newBuf->value, result->value, result->length * sizeof(wchar));
        newBuf->length = result->length;
        result = newBuf;
    }
    memcpy(&result->value[result->length], str, len * sizeof(wchar));
    result->length += len;
    result->value[result->length] = 0;
    /* Don't intern */
    return result;
}


/*
    Find a substring. Search forward or backwards. Return the index in the string where the pattern was found.
    Return -1 if not found.
 */
static ssize indexof(wchar *str, ssize len, EjsString *pattern, ssize patternLength, int dir)
{
    wchar       *s1, *s2;
    ssize       i, j;

    assert(dir == 1 || dir == -1);

    if (dir > 0) {
        for (i = 0; i < len; i++) {
            s1 = &str[i];
            for (j = 0, s2 = pattern->value; j < patternLength; s1++, s2++, j++) {
                if (*s1 != *s2) {
                    break;
                }
            }
            if (*s2 == '\0') {
                return i;
            }
        }
    } else {
        for (i = len - 1; i >= 0; i--) {
            s1 = &str[i];
            for (j = 0, s2 = pattern->value; j < patternLength; s1++, s2++, j++) {
                if (*s1 != *s2) {
                    break;
                }
            }
            if (*s2 == '\0') {
                return i;
            }
        }
    }
    return -1;
}

/******************************************** String API *******************************************/

PUBLIC int ejsAtoi(Ejs *ejs, EjsString *sp, int radix)
{
    int     num, i;

    num = 0;
    for (i = 0; i < (int) sp->length; i++) {
        num = num * radix + (sp->value[i] - '0');
    }
    return num;
}


PUBLIC EjsString *ejsJoinString(Ejs *ejs, EjsString *s1, EjsString *s2)
{
    EjsString   *result;
    ssize       len;

    if (s1->length == 0) {
        return s2;
    }
    if (s2->length == 0) {
        return s1;
    }
    len = s1->length + s2->length;
    if ((result = ejsCreateBareString(ejs, len)) == NULL) {
        return NULL;
    }
    memcpy(result->value, s1->value, s1->length * sizeof(wchar));
    memcpy(&result->value[s1->length], s2->value, s2->length * sizeof(wchar));
    return ejsInternString(result);
}



/*
    Join a set of unicode string arguments onto another.
    TODO - rename to join
 */
PUBLIC EjsString *ejsJoinStrings(Ejs *ejs, EjsString *src, ...)
{
    EjsString   *sp, *result;
    va_list     args;
    ssize       len;

    va_start(args, src);
    for (len = 0, sp = src; sp; ) {
        sp = va_arg(args, EjsString*);
        len += sp->length;
    }
    if ((result = ejsCreateBareString(ejs, len)) == NULL) {
        return NULL;
    }
    result->length = 0;
    va_start(args, src);
    for (sp = src; sp; ) {
        sp = va_arg(args, EjsString*);
        memcpy(&result->value[result->length], src->value, src->length);
        result->length += src->length;
    }
    va_end(args);
    return ejsInternString(result);
}


PUBLIC int ejsStartsWithAsc(Ejs *ejs, EjsString *sp, cchar *pat)
{
    ssize   i;

    assert(sp);
    assert(pat);

    if (pat == 0 || *pat == '\0') {
        return 0;
    }
    for (i = 0; pat[i] && i < sp->length; i++) {
        if (sp->value[i] != pat[i]) {
            break;
        }
    }
    if (pat[i]) {
        return -1;
    }
    return 0;
}


PUBLIC int ejsCompareAsc(Ejs *ejs, EjsString *sp, cchar *str)
{
    wchar       *s1;
    cchar       *s2;
    int         rc;

    s1 = sp->value;
    s2 = str;

    if (s1 == 0) {
        return -1;
    }
    if (s2 == 0) {
        return 1;
    }
    if ((cchar*) s1 == s2) {
        return 0;
    }
    for (rc = 0; *s1 && *s2 && rc == 0; s1++, s2++) {
        rc = *s1 - *s2;
    }
    if (rc) {
        return rc < 0 ? -1 : 1;
    }
    if (*s1 == '\0' && *s2) {
        return -1;
    }
    if (*s2 == '\0' && *s1) {
        return 1;
    }
    return rc;
}


/*
    Compare interned strings
 */
PUBLIC int ejsCompareString(Ejs *ejs, EjsString *sp1, EjsString *sp2)
{
    wchar       *s1;
    wchar       *s2;
    int         rc;

    s1 = sp1->value;
    s2 = sp2->value;

    if (s1 == s2) {
        /* No need to compare interned strings char-by-char */
        return 0;
    } else if (s1 == 0) {
        return -1;
    } else if (s2 == 0) {
        return 1;
    }
    for (rc = 0; *s1 && *s2 && rc == 0; s1++, s2++) {
        rc = *s1 - *s2;
    }
    if (rc) {
        return rc < 0 ? -1 : 1;
    } else if (*s1 == '\0' && *s2) {
        return -1;
    } else if (*s2 == '\0' && *s1) {
        return 1;
    }
    assert(0);
    return 0;
}


PUBLIC int ejsCompareSubstring(Ejs *ejs, EjsString *sp1, EjsString *sp2, ssize offset, ssize len)
{
    assert(0 <= len && len < MAXINT);

    if (offset < 0 || offset >= sp2->length) {
        return -1;
    }
    return ejsCompareWide(ejs, sp1, &sp2->value[offset], len);
}


PUBLIC int ejsCompareWide(Ejs *ejs, EjsString *sp1, wchar *sp2, ssize len)
{
    wchar       *s1;
    wchar       *s2;
    int         rc;

    assert(0 <= len && len < MAXINT);

    s1 = sp1->value;
    s2 = sp2;

    if (s1 == s2) {
        return 0;
    } else if (s1 == 0) {
        return -1;
    } else if (s2 == 0) {
        return 1;
    }
    for (rc = 0; len > 0 && *s1 && *s2 && rc == 0; s1++, s2++, len--) {
        rc = *s1 - *s2;
    }
    if (rc) {
        return rc < 0 ? -1 : 1;
    } else if (len == 0) {
        return 0;
    } else if (*s1 == 0 && *s2 == 0) {
        return 0;
    } else if (*s1 == '\0' && *s2) {
        return -1;
    } else if (*s2 == '\0' && *s1) {
        return 1;
    }
    return 0;
}


/*
    Return the index in a unicode string of a given unicode character code. Return -1 if not found.
 */
PUBLIC int ejsContainsChar(Ejs *ejs, EjsString *sp, int charPat)
{
    int     i;

    assert(sp);

    for (i = 0; i < sp->length; i++) {
        if (sp->value[i] == charPat) {
            return i;
        }
    }
    return -1;
}


#if UNUSED && KEEP
/*
    Return TRUE if a unicode string contains a given unicode string after doing a case insensitive comparison.
 */
PUBLIC int ejsContainsStringAnyCase(Ejs *ejs, EjsString *sp, EjsString *pat)
{
    int     i, j;

    assert(sp);
    assert(pat);
    assert(pat->value);

    for (i = 0; i < sp->length; i++) {
        for (j = 0; j < pat->length; j++) {
            //  TODO UNICODE - tolower only works for ASCII
            if (tolower((uchar) sp->value[i]) != tolower((uchar) pat->value[j])) {
                break;
            }
        }
        if (j == pat->length) {
            return 1;
        }
    }
    return 0;
}
#endif


PUBLIC int ejsContainsAsc(Ejs *ejs, EjsString *sp, cchar *pat)
{
    ssize   len;
    int     i, j, k;

    assert(sp);

    if (pat == 0 || *pat == '\0' || sp == 0) {
        return 0;
    }
    len = strlen(pat);
    
    for (i = 0; i < sp->length; i++) {
        for (j = 0, k = i; j < len; j++, k++) {
            if (sp->value[k] != pat[j]) {
                break;
            }
        }
        if (j == len) {
            return i;
        }
    }
    return -1;
}


PUBLIC int ejsContainsString(Ejs *ejs, EjsString *sp, EjsString *pat)
{
    int     i, j, k;

    assert(sp);
    assert(pat);
    assert(pat->value);

    if (pat == 0) {
        return -1;
    }
    for (i = 0; i < sp->length; i++) {
        for (j = 0, k = i; j < pat->length; j++, k++) {
            if (sp->value[k] != pat->value[j]) {
                break;
            }
        }
        if (j == pat->length) {
            return i;
        }
    }
    return -1;
}


PUBLIC char *ejsToMulti(Ejs *ejs, EjsAny *ev)
{
    if (ev == 0) {
        ev = ESV(empty);
    }
    if (!ejsIs(ejs, ev, String)) {
        if ((ev = ejsCast(ejs, ev, String)) == 0) {
            return MPR->emptyString;
        }
    }
    assert(ejsIs(ejs, ev, String));
    //  UNICODE
#if ME_CHAR_LEN == 1
{
    EjsString   *s = (EjsString*) ev;
    char        *ptr;
    ptr = mprMemdupMem(s->value, s->length + 1);
    ptr[s->length] = '\0';
    return ptr;
}
#else
    return awtom(((EjsString*) ev)->value, NULL);
#endif
}


PUBLIC EjsString *ejsSprintf(Ejs *ejs, cchar *fmt, ...)
{
    va_list     ap;
    char        *result;

    assert(fmt);

    va_start(ap, fmt);
    result = sfmtv(fmt, ap);
    va_end(ap);
    return ejsCreateStringFromAsc(ejs, result);
}


/*
    Get a substring from "src" starting at "start" of length "len"
    If len < 0, use entire string.
 */
PUBLIC EjsString *ejsSubstring(Ejs *ejs, EjsString *src, ssize start, ssize len)
{
    EjsString   *result;

    assert(src);
    assert(start >= 0);
    assert((start + len) <= src->length);

    if (len < 0) {
        len = src->length - start;
    }
    if (len < 0 || (start + len) > src->length || start < 0) {
        return ESV(empty);
    }
    len = min(len, src->length);
    if ((result = ejsCreateBareString(ejs, len)) == NULL) {
        return NULL;
    }
    memcpy(result->value, &src->value[start], len);
    return ejsInternString(result);
}


PUBLIC EjsString *ejsToLower(Ejs *ejs, EjsString *sp)
{
    EjsString   *result;
    int         i;

    assert(sp);
    assert(sp->value);

    result = (EjsString*) ejsCreateBareString(ejs, sp->length);
    for (i = 0; i < sp->length; i++) {
        result->value[i] = tolower((uchar) sp->value[i]);
    }
    return ejsInternString(result);
}


PUBLIC EjsString *ejsToUpper(Ejs *ejs, EjsString *sp)
{
    EjsString   *result;
    int         i;

    assert(sp);
    assert(sp->value);

    result = (EjsString*) ejsCreateBareString(ejs, sp->length);
    for (i = 0; i < sp->length; i++) {
        result->value[i] = toupper((uchar) sp->value[i]);
    }
    return ejsInternString(result);
}


#if UNUSED && KEEP
PUBLIC EjsString *ejsTrimString(Ejs *ejs, EjsString *sp, cchar *pat, int flags)
{
    EjsString   *result;
    wchar       *start, *end;
    cchar       *p;
    int         plen, trimmed;

    assert(pat);

    start = sp->value;
    plen = strlen(pat);
    trimmed = 0;

    if (flags & MPR_TRIM_START) {
        for (; *start; start++) {
            for (p = pat; *p; p++) {
                if (*start == *p) {
                    break;
                }
            }
            if (*start != *p) {
                break;
            }
            trimmed++;
        }
    }

    if (flags & MPR_TRIM_END) {
        for (end = &sp->value[sp->length]; end > start; end--) {
            for (p = pat; *p; p++) {
                if (*start == *p) {
                    break;
                }
            }
            if (*start != *p) {
                break;
            }
            trimmed++;
        }
    }
    result = ejsCreateBareString(ejs, sp->length - trimmed);
    memcpy(result->value, start, result->length);
    return ejsInternString(result);
}
#endif


PUBLIC EjsString *ejsTruncateString(Ejs *ejs, EjsString *sp, ssize len)
{
    EjsString   *result;

    assert(sp);

    len = min(len, sp->length);
    result = ejsCreateBareString(ejs, len);
    memcpy(result->value, sp->value, len);
    return ejsInternString(result);
}


/*********************************** Interning *********************************/

static void revive(EjsString *sp)
{
    MprMem  *mp;

    mp = (MprMem*) MPR_GET_MEM(sp);
    mp->mark = MPR->heap->mark;
}


/*
    Intern a unicode string. Lookup a string and return an interned string (this may be an existing interned string)
 */
PUBLIC EjsString *ejsInternString(EjsString *str)
{
    EjsString   *head, *sp;
    EjsIntern   *ip;
    ssize       i, len;
    int         index, step;

    ip = ((EjsService*) MPR->ejsService)->intern;
    step = 0;

    lock(ip);
    ip->accesses++;
    index = whash(str->value, str->length) % ip->size;
    if ((head = &ip->buckets[index]) != NULL) {
        for (sp = head->next; sp != head; sp = sp->next, step++) {
            if (str == sp) {
                revive(sp);
                unlock(ip);
                return sp;
            }
            if (sp->length == str->length) {
                //  TODO but they are the same?
                len = min(sp->length, str->length);
                //  OPT
                for (i = 0; i < len; i++) {
                    if (sp->value[i] != str->value[i]) {
                        break;
                    }
                }
                if (i == sp->length && i == str->length) {
                    ip->reuse++;
                    revive(sp);
                    unlock(ip);
                    return sp;
                }
            }
        }
    }
    ip->count++;
    linkString(head, str);
    if (step > EJS_MAX_COLLISIONS && ip->count > (ip->size/2)) {
        /*  Remake the entire hash - should not happen often */
        //  OPT - stop holding lock while rebuildingIntern
        rebuildIntern(ip);
    }
    unlock(ip);
    return str;
}


/*
    Intern a wide C string and return an interned wide string
 */
PUBLIC EjsString *ejsInternWide(Ejs *ejs, wchar *value, ssize len)
{
    EjsString   *head, *sp;
    EjsIntern   *ip;
    ssize       i, end;
    int         index, step;

    assert(0 <= len && len < MAXINT);

    ip = ejs->service->intern;
    step = 0;

    lock(ip);
    //  TODO - accesses should be debug only
    ip->accesses++;
    index = whash(value, len) % ip->size;
    if ((head = &ip->buckets[index]) != NULL) {
        for (sp = head->next; sp != head; sp = sp->next, step++) {
            //  TODO - why not compare sp->value with value?
            if (sp->length == len) {
                end = min(sp->length, len);
                for (i = 0; i < end && value[i]; i++) {
                    if (sp->value[i] != value[i]) {
                        break;
                    }
                }
                if (i == sp->length) {
                    ip->reuse++;
                    revive(sp);
                    unlock(ip);
                    return sp;
                }
            }
        }
    }
    if ((sp = ejsAlloc(ejs, ESV(String), (len + 1) * sizeof(wchar))) != NULL) {
        memcpy(sp->value, value, len * sizeof(wchar));
        sp->value[len] = 0;
    }
    sp->length = len;
    ip->count++;
    linkString(head, sp);
    if (step > EJS_MAX_COLLISIONS && ip->count > (ip->size/2)) {
        /*  Remake the entire hash - should not happen often */
        //  OPT - lock while rebuildingIntern
        rebuildIntern(ip);
    }
    unlock(ip);
    return sp;
}


PUBLIC EjsString *ejsInternAsc(Ejs *ejs, cchar *value, ssize len)
{
    EjsString   *head, *sp;
    EjsIntern   *ip;
    ssize       i, end;
    int         index, step;

    assert(0 <= len && len < MAXINT);

    step = 0;
    ip = ejs->service->intern;

    lock(ip);
    ip->accesses++;
    assert(ip->size > 0);
    index = shash(value, len) % ip->size;
    if ((head = &ip->buckets[index]) != NULL) {
        for (sp = head->next; sp != head; sp = sp->next, step++) {
            if (sp->length == len) {
                end = min(len, sp->length);
                for (i = 0; i < end && value[i]; i++) {
                    if (sp->value[i] != (uchar) value[i]) {
                        break;
                    }
                }
                if (i == sp->length) {
                    ip->reuse++;
                    revive(sp);
                    unlock(ip);
                    return sp;
                }
            }
        }
    }
    if ((sp = ejsAlloc(ejs, ESV(String), (len + 1) * sizeof(wchar))) != NULL) {
#if ME_CHAR_LEN > 1
        for (i = 0; i < len; i++) {
            sp->value[i] = value[i];
        }
        sp->length = len;
#else
        memcpy(sp->value, value, len * sizeof(wchar));
#endif
        sp->value[len] = 0;
    }
    sp->length = len;
    ip->count++;
    linkString(head, sp);
    if (step > EJS_MAX_COLLISIONS && ip->count > (ip->size/2)) {
        /*  Remake the entire hash - should not happen often */
        //  OPT - BAD holding lock while rebuildingIntern
        rebuildIntern(ip);
    }
    unlock(ip);
    return sp;
}


#if ME_CHAR_LEN == 1
PUBLIC EjsString *ejsInternMulti(Ejs *ejs, cchar *value, ssize len)
{
    return ejsInternAsc(ejs, value, len);
}

#else /* ME_CHAR_LEN > 1 */

PUBLIC EjsString *ejsInternMulti(Ejs *ejs, cchar *value, ssize len)
{
    EjsString   *head, *sp, src;
    ssize       i, end;
    int         index;

    assert(0 < len && len < MAXINT);

    len = mtow(NULL, MAXSSIZE, value, len);
    assert(len < MAXINT);

    /*
        Have to convert the multibyte string to unicode before comparision. Convert into an EjsString to it is ready
        to intern if not found.
     */
    if ((src = ejsAlloc(ejs, ESV(String), (len + 1) * sizeof(wchar))) != NULL) {
        src->length = mtow(src->value, len + 1, value, len);
        value = src->value;
    }
    lock(ip);
    ip->accesses++;
    index = whash(value, len) % ip->size;
    if ((head = &ip->buckets[index]) != NULL) {
        for (sp = head->next; sp != head; sp = sp->next) {
            end = min(len, sp->length);
            for (i = 0; i < end && value[i]; i++) {
                if (sp->value[i] != sp->value[i]) {
                    break;
                }
            }
            if (i == sp->length && value[i] == 0) {
                ip->reuse++;
                revive(sp);
                unlock(ip);
                return sp;
            }
        }
    }
    ip->count++;
    linkString(head, src);
    if (step > EJS_MAX_COLLISIONS && ip->count > (ip->size/2)) {
        /*  Remake the entire hash - should not happen often */
        //  OPT - BAD holding lock while rebuildingIntern
        rebuildIntern(ip);
    }
    unlock(ip);
    return sp;
}
#endif /* ME_CHAR_LEN > 1 */


static int getInternHashSize(int size)
{
    int     i;

    for (i = 0; i < internHashSizes[i]; i++) {
        if (size < internHashSizes[i]) {
            return internHashSizes[i];
        }
    }
    return internHashSizes[i - 1];
}


static int rebuildIntern(EjsIntern *ip)
{
    EjsString   *oldBuckets, *sp, *next, *head;
    int         i, newSize, oldSize;

    assert(ip);

    oldBuckets = ip->buckets;
    newSize = getInternHashSize(ip->size + 1);
    oldSize = 0;
    if (oldBuckets) {
        oldSize = ip->size;
        if (oldSize >= newSize) {
            return 0;
        }
    }
    if ((ip->buckets = mprAllocZeroed((newSize * sizeof(EjsString)))) == NULL) {
        return MPR_ERR_MEMORY;
    }
    ip->size = newSize;
    for (i = 0; i < newSize; i++) {
        sp = &ip->buckets[i];
        sp->next = sp->prev = sp;
    }
    if (oldBuckets) {
        for (i = 0; i < oldSize; i++) {
            head = &oldBuckets[i];
            for (sp = head->next; sp != head; sp = next) {
                next = sp->next;
                sp->next = sp->prev = sp;
                ejsInternString(sp);
            }
        }
    }
    return 0;
}


/*
    Must be called locked
 */
static void linkString(EjsString *head, EjsString *sp)
{
    assert(sp != head);
    assert(sp->next == NULL || sp->next == sp);
    assert(sp->prev == NULL || sp->next == sp);

    sp->next = head->next;
    sp->prev = head;
    head->next->prev = sp;
    head->next = sp;
    assert(sp != sp->next);
    assert(sp != sp->prev);
}


/*
    Unlink a string from the intern cache. This unlinks from the hash chains. 
    This routine is idempotent. ejsDestroyIntern takes advantage of this.
    Must be called locked.
 */
static void unlinkString(EjsString *sp)
{
    /*
        Some strings are not interned (ejsCreateBareString). These have sp->next == NULL.
     */
    if (sp->next) {
        sp->prev->next = sp->next;
        sp->next->prev = sp->prev;
        sp->next = sp->prev = sp;
    }
}


/*********************************** Factory **********************************/

PUBLIC EjsString *ejsCreateString(Ejs *ejs, wchar *value, ssize len)
{
    if (value == 0) {
        return ESV(empty);
    }
    if (len < 0) {
        len = slen(value);
    }
    return ejsInternWide(ejs, value, len);
}


PUBLIC EjsString *ejsCreateStringFromAsc(Ejs *ejs, cchar *value)
{
    if (value == NULL) {
        value = "";
    }
    return ejsInternAsc(ejs, value, slen(value));
}


PUBLIC EjsString *ejsCreateStringFromMulti(Ejs *ejs, cchar *value, ssize len)
{
    assert(0 <= len && len < MAXINT);
    if (value == 0 || len < 0) {
        return ESV(empty);
    }
    return ejsInternMulti(ejs, value, len);
}


PUBLIC EjsString *ejsCreateStringFromBytes(Ejs *ejs, cchar *value, ssize len)
{
    assert(0 <= len && len < MAXINT);
    return ejsInternAsc(ejs, value, len);
}


/*
    Create an empty string object and do not intern. Caller's should call ejsInternString when the string value is defined.
 */
PUBLIC EjsString *ejsCreateBareString(Ejs *ejs, ssize len)
{
    EjsString   *sp;
    
    assert(0 <= len && len < MAXINT);
    if ((sp = ejsAlloc(ejs, ESV(String), (len + 1) * sizeof(wchar))) != NULL) {
        sp->length = len;
        sp->value[0] = 0;
        sp->value[len] = 0;
    }
    return sp;
}


#if UNUSED && KEEP
PUBLIC EjsString *ejsCreateNonInternedString(Ejs *ejs, wchar *value, ssize len)
{
    EjsString   *sp;
    
    assert(0 <= len && len < MAXINT);
    if ((sp = ejsAlloc(ejs, ESV(String), (len + 1) * sizeof(wchar))) != NULL) {
        memcpy(sp->value, value, (len + 1) * sizeof(wchar));
        sp->length = len;
        sp->value[len] = 0;
    }
    return sp;
}
#endif


PUBLIC void ejsManageString(EjsString *sp, int flags)
{
    EjsIntern   *ip;
    MprMem      *mp;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(TYPE(sp));

    } else if (flags & MPR_MANAGE_FREE) {
        if (sp->next && MPR->ejsService) {
            /*
                Other threads race with this if doing parallel GC (the default). 
                The revive() routine may have marked the string, so test here if 
                it has been revived and only free if not.
                OPT - better to be lock free and try lock. If failed, GC will get next time
             */
            mp = MPR_GET_MEM(sp);
            if (mp->mark != MPR->heap->mark) {
                ip = ((EjsService*) MPR->ejsService)->intern;
                lock(ip);
                /* Must retest here because revive may modify the mark to revive */
                if (mp->mark != MPR->heap->mark) {
                    ip->count--;
                    assert(ip->count >= 0);
                    unlinkString(sp);
                }
                unlock(ip);
            }
        }
    }
}


PUBLIC EjsIntern *ejsCreateIntern(EjsService *sp)
{
    EjsIntern   *intern;
    
    intern = mprAllocObj(EjsIntern, manageIntern);
    intern->mutex = mprCreateLock();
    rebuildIntern(intern);
    return intern;
}


PUBLIC void ejsDestroyIntern(EjsIntern *ip)
{
    ip->size = 0;
    ip->buckets = 0;
}


static void manageIntern(EjsIntern *intern, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        /* Do not mark strings - prevents GC */
        mprMark(intern->buckets);
        mprMark(intern->mutex);

    } else if (flags & MPR_MANAGE_FREE) {
        ejsDestroyIntern(intern);
    }
}


PUBLIC void ejsInitStringType(Ejs *ejs, EjsType *type)
{
    type->helpers.cast = (EjsCastHelper) castString;
    type->helpers.clone = (EjsCloneHelper) cloneString;
    type->helpers.getProperty = (EjsGetPropertyHelper) getStringProperty;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeStringOperator;
    type->helpers.lookupProperty = (EjsLookupPropertyHelper) lookupStringProperty;
    type->numericIndicies = 1;
}


PUBLIC void ejsConfigureStringType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "String"))) == 0) {
        return;
    }
    ejsBindMethod(ejs, type, ES_String_fromCharCode, fromCharCode);
    ejsBindConstructor(ejs, type, stringConstructor);
    
    prototype = type->prototype;
    ejsBindMethod(ejs, prototype, ES_String_caseCompare, caseCompare);
    ejsBindMethod(ejs, prototype, ES_String_caselessCompare, caselessCompare);
    ejsBindMethod(ejs, prototype, ES_String_charAt, charAt);
    ejsBindMethod(ejs, prototype, ES_String_charCodeAt, charCodeAt);
    ejsBindMethod(ejs, prototype, ES_String_concat, concatString);
    ejsBindMethod(ejs, prototype, ES_String_contains, containsString);
    ejsBindMethod(ejs, prototype, ES_String_endsWith, endsWith);
    ejsBindMethod(ejs, prototype, ES_String_expand, expandString);
    ejsBindMethod(ejs, prototype, ES_String_format, formatString);
    ejsBindMethod(ejs, prototype, ES_String_iterator_get, getStringIterator);
    ejsBindMethod(ejs, prototype, ES_String_iterator_getValues, getStringValues);
    ejsBindMethod(ejs, prototype, ES_String_indexOf, indexOf);
    ejsBindMethod(ejs, prototype, ES_String_isDigit, isDigit);
    ejsBindMethod(ejs, prototype, ES_String_isAlpha, isAlpha);
    ejsBindMethod(ejs, prototype, ES_String_isAlphaNum, isAlphaNum);
    ejsBindMethod(ejs, prototype, ES_String_isLower, isLower);
    ejsBindMethod(ejs, prototype, ES_String_isSpace, isSpace);
    ejsBindMethod(ejs, prototype, ES_String_isUpper, isUpper);
    ejsBindMethod(ejs, prototype, ES_String_lastIndexOf, lastIndexOf);
    ejsBindMethod(ejs, prototype, ES_String_length, stringLength);
    ejsBindMethod(ejs, prototype, ES_String_match, match);
    ejsBindMethod(ejs, prototype, ES_String_remove, removeCharsFromString);
    ejsBindMethod(ejs, prototype, ES_String_slice, sliceString);
    ejsBindMethod(ejs, prototype, ES_String_split, split);
    ejsBindMethod(ejs, prototype, ES_String_printable, printable);
    ejsBindMethod(ejs, prototype, ES_String_quote, quote);
    ejsBindMethod(ejs, prototype, ES_String_replace, replace);
    ejsBindMethod(ejs, prototype, ES_String_reverse, reverseString);
    ejsBindMethod(ejs, prototype, ES_String_search, searchString);
    ejsBindMethod(ejs, prototype, ES_String_startsWith, startsWith);
    ejsBindMethod(ejs, prototype, ES_String_substring, substring);
    ejsBindMethod(ejs, prototype, ES_String_toCamel, toCamel);
    ejsBindMethod(ejs, prototype, ES_String_toJSON, ejsToLiteralString);
    ejsBindMethod(ejs, prototype, ES_String_toLowerCase, toLowerCase);
    ejsBindMethod(ejs, prototype, ES_String_toPascal, toPascal);
    ejsBindMethod(ejs, prototype, ES_String_toString, stringToString);
    ejsBindMethod(ejs, prototype, ES_String_toUpperCase, toUpperCase);
    ejsBindMethod(ejs, prototype, ES_String_tokenize, tokenize);
    ejsBindMethod(ejs, prototype, ES_String_trim, trimString);
    ejsBindMethod(ejs, prototype, ES_String_trimStart, trimStartString);
    ejsBindMethod(ejs, prototype, ES_String_trimEnd, trimEndString);

#if FUTURE
    ejsBindMethod(ejs, prototype, ES_String_LBRACKET, operLBRACKET);
    ejsBindMethod(ejs, prototype, ES_String_PLUS, operPLUS);
    ejsBindMethod(ejs, prototype, ES_String_MINUS, operMINUS);
    ejsBindMethod(ejs, prototype, ES_String_LT, operLT);
    ejsBindMethod(ejs, prototype, ES_String_GT, operGT);
    ejsBindMethod(ejs, prototype, ES_String_EQ, operEQ);
    ejsBindMethod(ejs, prototype, ES_String_MOD, operMOD);
    ejsBindMethod(ejs, prototype, ES_String_MUL, operMUL);
#endif
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
