/**
    ejsString.c - Ejscript string class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"
#include    "pcre.h"

/***************************** Forward Declarations ***************************/

static int catString(Ejs *ejs, EjsString *dest, char *str, int len);
static int indexof(cchar *str, int len, cchar *pattern, int patlen, int dir);

/******************************************************************************/
/*
    Cast the string operand to a primitive type
 */
static EjsObj *castString(Ejs *ejs, EjsString *sp, EjsType *type)
{
    EjsObj  *result;
    char    *buf;

    mprAssert(sp);
    mprAssert(type);

    if (type == ejs->pathType) {
        return (EjsObj*) ejsCreatePath(ejs, sp->value);
    } else if (type == ejs->uriType) {
        return (EjsObj*) ejsCreateUri(ejs, sp->value);
    }
    switch (type->id) {
    case ES_Boolean:
        if (sp->value[0]) {
            return (EjsObj*) ejs->trueValue;
        }
        return (EjsObj*) ejs->falseValue;

    case ES_Number:
        return (EjsObj*) ejsParse(ejs, sp->value, ES_Number);

    case ES_RegExp:
        if (sp->value && sp->value[0] == '/') {
            return (EjsObj*) ejsCreateRegExp(ejs, sp->value);
        }
        buf = mprStrcat(ejs, -1, "/", sp->value, "/", NULL);
        result = (EjsObj*) ejsCreateRegExp(ejs, buf);
        mprFree(buf);
        return result;

    case ES_String:
        return (EjsObj*) sp;

    default:
        ejsThrowTypeError(ejs, "Can't cast to required type");
        return 0;
    }
    return 0;
}


/*
    Clone a string. Shallow copies simply return a reference as strings are immutable.
 */
static EjsString *cloneString(Ejs *ejs, EjsString *sp, bool deep)
{
    if (deep) {
        return ejsCreateStringWithLength(ejs, sp->value, sp->length);
    }
    return sp;
}


//  TODO - can remove as free var will do this automatically
static void destroyString(Ejs *ejs, EjsString *sp)
{
    mprAssert(sp);

    mprFree(sp->value);
    sp->value = 0;
    ejsFreeVar(ejs, (EjsObj*) sp, -1);
}


/*
    Get a string element. Slot numbers correspond to character indicies.
 */
static EjsObj *getStringProperty(Ejs *ejs, EjsString *sp, int index)
{
    if (index < 0 || index >= sp->length) {
        return (EjsObj*) ejs->emptyStringValue;
        return 0;
    }
    return (EjsObj*) ejsCreateStringWithLength(ejs, &sp->value[index], 1);
}


static EjsObj *coerceStringOperands(Ejs *ejs, EjsObj *lhs, int opcode,  EjsObj *rhs)
{
    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsInvokeOperator(ejs, lhs, opcode, (EjsObj*) ejsToString(ejs, rhs));

    /*
        Overloaded operators
     */
    case EJS_OP_MUL:
        if (ejsIsNumber(rhs)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, lhs, opcode, (EjsObj*) ejsToNumber(ejs, rhs));

    case EJS_OP_REM:
        return 0;

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_OR:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        return ejsInvokeOperator(ejs, lhs, opcode, (EjsObj*) ejsToString(ejs, rhs));

    case EJS_OP_COMPARE_STRICTLY_NE:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return (EjsObj*) ejs->falseValue;

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT:
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToBoolean(ejs, lhs), opcode, rhs);

    case EJS_OP_NOT:
    case EJS_OP_NEG:
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_TRUE:
        return (EjsObj*) (((EjsString*) lhs)->value ? ejs->trueValue : ejs->falseValue);

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return (EjsObj*) (((EjsString*) lhs)->value ? ejs->falseValue: ejs->trueValue);

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return (EjsObj*) ejs->falseValue;

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %s", opcode, lhs->type->qname.name);
        return ejs->undefinedValue;
    }
    return 0;
}


static EjsObj *invokeStringOperator(Ejs *ejs, EjsString *lhs, int opcode,  EjsString *rhs, void *data)
{
    EjsObj      *result;
    EjsObj      *arg;

    if (rhs == 0 || lhs->obj.type != rhs->obj.type) {
        if (!ejsIsA(ejs, (EjsObj*) lhs, ejs->stringType) || !ejsIsA(ejs, (EjsObj*) rhs, ejs->stringType)) {
            if ((result = coerceStringOperands(ejs, (EjsObj*) lhs, opcode, (EjsObj*) rhs)) != 0) {
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
        if (lhs == rhs || (lhs->value == rhs->value)) {
            return (EjsObj*) ejs->trueValue;
        }
        return (EjsObj*) ejsCreateBoolean(ejs,  mprMemcmp(lhs->value, lhs->length, rhs->value, rhs->length) == 0);

    case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_STRICTLY_NE:
        if (lhs->length != rhs->length) {
            return (EjsObj*) ejs->trueValue;
        }
        return (EjsObj*) ejsCreateBoolean(ejs,  mprMemcmp(lhs->value, lhs->length, rhs->value, rhs->length) != 0);

    case EJS_OP_COMPARE_LT:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprMemcmp(lhs->value, lhs->length, rhs->value, rhs->length) < 0);

    case EJS_OP_COMPARE_LE:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprMemcmp(lhs->value, lhs->length, rhs->value, rhs->length) <= 0);

    case EJS_OP_COMPARE_GT:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprMemcmp(lhs->value, lhs->length, rhs->value, rhs->length) > 0);

    case EJS_OP_COMPARE_GE:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprMemcmp(lhs->value, lhs->length, rhs->value, rhs->length) >= 0);

    /*
        Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return (EjsObj*) ((lhs->value) ? ejs->trueValue: ejs->falseValue);

    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ((lhs->value == 0) ? ejs->trueValue: ejs->falseValue);


    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return (EjsObj*) ejs->falseValue;

    /*
        Binary operators
     */
    case EJS_OP_ADD:
        result = (EjsObj*) ejsCreateString(ejs, lhs->value);
        ejsStrcat(ejs, (EjsString*) result, (EjsObj*) rhs);
        return result;

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_OR:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, (EjsObj*) lhs), opcode, (EjsObj*) rhs);

#if EXTENSIONS || 1
    /*
        Overloaded
     */
    case EJS_OP_SUB:
        arg = (EjsObj*) rhs;
        return ejsRunFunctionBySlot(ejs, (EjsObj*) lhs, ES_String_MINUS, 1, &arg);

    case EJS_OP_REM:
        arg = (EjsObj*) rhs;
        return ejsRunFunctionBySlot(ejs, (EjsObj*) lhs, ES_String_MOD, 1, &arg);
#endif

    case EJS_OP_NEG:
    case EJS_OP_LOGICAL_NOT:
    case EJS_OP_NOT:
        /* Already handled in coerceStringOperands */
    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %s", opcode, lhs->obj.type->qname.name);
        return 0;
    }
    mprAssert(0);
}


/*
    Lookup an string index.
 */
static int lookupStringProperty(struct Ejs *ejs, EjsString *sp, EjsName *qname)
{
    int     index;

    if (qname == 0 || ! isdigit((int) qname->name[0])) {
        return EJS_ERR;
    }
    index = atoi(qname->name);
    if (index < sp->length) {
        return index;
    }

    return EJS_ERR;
}


/************************************ Methods *********************************/
/*
    String constructor.

        function String()
        function String(str: String)
 */
static EjsObj *stringConstructor(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsArray    *args;
    EjsString   *str;

    mprAssert(argc == 0 || (argc == 1 && ejsIsArray(argv[0])));
    sp->length = 0;

    if (argc == 1) {
        args = (EjsArray*) argv[0];
        if (args->length > 0) {
            str = ejsToString(ejs, ejsGetProperty(ejs, (EjsObj*) args, 0));
            if (str) {
                sp->value = mprStrdup(sp, str->value);
                sp->length = str->length;
            }
        } else {
            sp->value = mprStrdup(ejs, "");
            if (sp->value == 0) {
                return 0;
            }
            sp->length = 0;
        }

    } else {
        sp->value = mprStrdup(ejs, "");
        if (sp->value == 0) {
            return 0;
        }
        sp->length = 0;
    }
    return (EjsObj*) sp;
}


/*
    Do a case sensitive comparison between this string and another.

    function caseCompare(compare: String): Number
 */
static EjsObj *caseCompare(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    int     result;

    mprAssert(argc == 1 && ejsIsString(argv[0]));

    result = mprStrcmp(sp->value, ((EjsString*) argv[0])->value);
    return (EjsObj*) ejsCreateNumber(ejs, result);
}


/*
    Do a case insensitive comparison between this string and another.

    function caselessCompare(compare: String): Number
 */
static EjsObj *caselessCompare(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    int     result;

    mprAssert(argc == 1 && ejsIsString(argv[0]));

    result = mprStrcmpAnyCase(sp->value, ((EjsString*) argv[0])->value);
    return (EjsObj*) ejsCreateNumber(ejs, result);
}


/*
    Return a string containing the character at a given index

    function charAt(index: Number): String
 */
static EjsObj *charAt(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    int     index;

    mprAssert(argc == 1 && ejsIsNumber(argv[0]));
    index = ejsGetInt(ejs, argv[0]);
    if (index < 0 || index >= sp->length) {
        return (EjsObj*) ejs->emptyStringValue;
    }
    return (EjsObj*) ejsCreateStringWithLength(ejs, &sp->value[index], 1);
}


/*
    Return an integer containing the character at a given index

    function charCodeAt(index: Number = 0): Number
 */

static EjsObj *charCodeAt(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    int     index;

    index = (argc == 1) ? ejsGetInt(ejs, argv[0]) : 0;
    if (index < 0) {
        index = sp->length -1 ;
    }
    if (index < 0 || index >= sp->length) {
        return (EjsObj*) ejs->nanValue;;
    }
    return (EjsObj*) ejsCreateNumber(ejs, (uchar) sp->value[index]);
}


/*
    Catenate args to a string and return a new string.

    function concat(...args): String
 */
static EjsObj *concatString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsArray    *args;
    EjsString   *result;
    int         i, count;

    mprAssert(argc == 1 && ejsIsArray(argv[0]));
    args = (EjsArray*) argv[0];

    result = ejsDupString(ejs, sp);

    count = ejsGetPropertyCount(ejs, (EjsObj*) args);
    for (i = 0; i < args->length; i++) {
        if (ejsStrcat(ejs, result, ejsGetProperty(ejs, (EjsObj*) args, i)) < 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
    }
    return (EjsObj*) result;
}


/**
    Check if a string contains the pattern (string or regexp)

    function contains(pattern: Object): Boolean
 */
static EjsObj *containsString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsObj      *pat;

    pat = argv[0];

    if (ejsIsString(pat)) {
        return (EjsObj*) ejsCreateBoolean(ejs, strstr(sp->value, ((EjsString*) pat)->value) != 0);
    } else if (ejsIsRegExp(pat)) {
        EjsRegExp   *rp;
        int         count;
        rp = (EjsRegExp*) argv[0];
        count = pcre_exec(rp->compiled, NULL, sp->value, sp->length, 0, 0, 0, 0);
        return (EjsObj*) ejsCreateBoolean(ejs, count >= 0);
    }
    ejsThrowTypeError(ejs, "Wrong argument type");
    return 0;
}


/**
    Check if a string ends with a given pattern

    function endsWith(pattern: String): Boolean
 */
static EjsObj *endsWith(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    cchar       *pattern;
    int         len;

    mprAssert(argc == 1 && ejsIsString(argv[0]));

    pattern = ejsGetString(ejs, argv[0]);
    len = (int) strlen(pattern);
    if (len > sp->length) {
        return (EjsObj*) ejs->falseValue;
    }
    return (EjsObj*) ejsCreateBoolean(ejs, strncmp(&sp->value[sp->length - len], pattern, len) == 0);
}


/**
    Format the arguments

    function format(...args): String

    Format:         %[modifier][width][precision][type]
    Modifiers:      +- #,
 */
static EjsObj *formatString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsArray    *args, *inner;
    EjsString   *result;
    EjsObj      *value;
    char        *buf;
    char        fmt[32];
    int         c, i, len, nextArg, start, kind, last;

    mprAssert(argc == 1 && ejsIsArray(argv[0]));

    args = (EjsArray*) argv[0];

    /*
        Flatten the args if there is only one element and it is itself an array. This happens when invoked
        via the overloaded operator '%' which in turn invokes format()
     */
    if (args->length == 1) {
        inner = ejsGetProperty(ejs, (EjsObj*) args, 0);
        if (ejsIsArray(inner)) {
            args = inner;
        }
    }
    result = ejsCreateString(ejs, 0);

    if (result == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }

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
            catString(ejs, result, &sp->value[last], i - last);
        }

        /*
            Find the end of the format specifier and determine the format type (kind)
         */
        start = i++;
        i += (int) strspn(&sp->value[i], "-+ #,0*123456789.");
        kind = sp->value[i];

        if (strchr("cdefginopsSuxX", kind)) {
            len = i - start + 1;
            mprMemcpy(fmt, sizeof(fmt) - 4, &sp->value[start], len);
            fmt[len] = '\0';

            if (nextArg < args->length) {
                value = ejsGetProperty(ejs, (EjsObj*) args, nextArg);
            } else {
                value = ejs->nullValue;
            }
            buf = 0;
            switch (kind) {
            case 'd': case 'i': case 'o': case 'u':
                value = (EjsObj*) ejsToNumber(ejs, value);
                strcpy(&fmt[len - 1], ".0f");
                buf = mprAsprintf(ejs, -1, fmt, ejsGetNumber(ejs, value));
                break;

            case 'e': case 'g': case 'f':
                value = (EjsObj*) ejsToNumber(ejs, value);
                buf = mprAsprintf(ejs, -1, fmt, ejsGetNumber(ejs, value));
                break;

            case 's':
                value = (EjsObj*) ejsToString(ejs, value);
                buf = mprAsprintf(ejs, -1, fmt, ejsGetString(ejs, value));
                break;

            case 'X': case 'x':
                buf = mprAsprintf(ejs, -1, fmt, (int64) ejsGetNumber(ejs, value));
                break;

            case 'n':
                buf = mprAsprintf(ejs, -1, fmt, 0);

            default:
                ejsThrowArgError(ejs, "Bad format specifier");
                return 0;
            }
            catString(ejs, result, buf, strlen(buf));
            mprFree(buf);
            last = i + 1;
            nextArg++;
        } else if (kind == '%') {
            catString(ejs, result, "%", 1);
            last = i + 1;
        }
    }
    i = (int) strlen(sp->value);
    if (i > last) {
        catString(ejs, result, &sp->value[last], i - last);
    }
    return (EjsObj*) result;
}


/*
    Create a string from character codes

    static function fromCharCode(...codes): String
 */
static EjsObj *fromCharCode(Ejs *ejs, EjsString *unused, int argc, EjsObj **argv)
{
    EjsString   *result;
    EjsArray    *args;
    EjsObj      *vp;
    int         i;

    mprAssert(argc == 1 && ejsIsArray(argv[0]));
    args = (EjsArray*) argv[0];

    result = (EjsString*) ejsCreateBareString(ejs, argc + 1);
    if (result == 0) {
        return 0;
    }
    for (i = 0; i < args->length; i++) {
        vp = ejsGetProperty(ejs, (EjsObj*) args, i);
        result->value[i] = ejsGetInt(ejs, ejsToNumber(ejs, vp));
    }
    result->value[i] = '\0';
    result->length = args->length;

    return (EjsObj*) result;
}


/*
    Function to iterate and return the next character code.
    NOTE: this is not a method of String. Rather, it is a callback function for Iterator
 */
static EjsObj *nextStringKey(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsString   *sp;

    sp = (EjsString*) ip->target;

    if (!ejsIsString(sp)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }

    if (ip->index < sp->length) {
        return (EjsObj*) ejsCreateNumber(ejs, ip->index++);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return the default iterator. This returns the array index names.

    iterator function get(): Iterator
 */
static EjsObj *getStringIterator(Ejs *ejs, EjsObj *sp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, sp, (EjsProc) nextStringKey, 0, NULL);
}


/*
    Function to iterate and return the next string character (as a string).
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsObj *nextStringValue(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsString   *sp;

    sp = (EjsString*) ip->target;
    if (!ejsIsString(sp)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }

    if (ip->index < sp->length) {
        return (EjsObj*) ejsCreateStringWithLength(ejs, &sp->value[ip->index++], 1);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return an iterator to return the next array element value.

    iterator function getValues(): Iterator
 */
static EjsObj *getStringValues(Ejs *ejs, EjsObj *sp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, sp, (EjsProc) nextStringValue, 0, NULL);
}


/*
    Get the length of a string.
    @return Returns the number of characters in the string

    override function get length(): Number
 */

static EjsObj *stringLength(Ejs *ejs, EjsString *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, ap->length);
}


/*
    Return the position of the first occurance of a substring

    function indexOf(pattern: String, startIndex: Number = 0): Number
 */
static EjsObj*indexOf(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    cchar   *pattern;
    int     index, start, patternLength;

    mprAssert(1 <= argc && argc <= 2);
    mprAssert(ejsIsString(argv[0]));

    pattern = ejsGetString(ejs, argv[0]);
    patternLength = ((EjsString*) argv[0])->length;

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
        return (EjsObj*) ejs->minusOneValue;
    }
    return (EjsObj*) ejsCreateNumber(ejs, index + start);
}


static EjsObj *isAlpha(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    cchar   *cp;

    if (sp->length == 0) {
        return (EjsObj*) ejs->falseValue;
    }
    for (cp = sp->value; cp < &sp->value[sp->length]; cp++) {
        if (*cp & 0x80 || !isalpha((int) *cp)) {
            return (EjsObj*) ejs->falseValue;
        }
    }
    return (EjsObj*) ejs->trueValue;
}


static EjsObj *isAlphaNum(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    cchar   *cp;

    if (sp->length == 0) {
        return (EjsObj*) ejs->falseValue;
    }
    for (cp = sp->value; cp < &sp->value[sp->length]; cp++) {
        if (*cp & 0x80 || !isalnum((int) *cp)) {
            return (EjsObj*) ejs->falseValue;
        }
    }
    return (EjsObj*) ejs->trueValue;
}


static EjsObj *isDigit(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    cchar   *cp;

    if (sp->length == 0) {
        return (EjsObj*) ejs->falseValue;
    }
    for (cp = sp->value; cp < &sp->value[sp->length]; cp++) {
        if (*cp & 0x80 || !isdigit((int) *cp)) {
            return (EjsObj*) ejs->falseValue;
        }
    }
    return (EjsObj*) ejs->trueValue;
}


static EjsObj *isLower(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    cchar   *cp;

    if (sp->length == 0) {
        return (EjsObj*) ejs->falseValue;
    }
    for (cp = sp->value; cp < &sp->value[sp->length]; cp++) {
        if (!islower((int) *cp)) {
            return (EjsObj*) ejs->falseValue;
        }
    }
    return (EjsObj*) ejs->trueValue;
}


static EjsObj *isSpace(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    cchar   *cp;

    if (sp->length == 0) {
        return (EjsObj*) ejs->falseValue;
    }
    for (cp = sp->value; cp < &sp->value[sp->length]; cp++) {
        if (*cp & 0x80 || !isspace((int) *cp)) {
            return (EjsObj*) ejs->falseValue;
        }
    }
    return (EjsObj*) ejs->trueValue;
}


static EjsObj *isUpper(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    cchar   *cp;

    if (sp->length == 0) {
        return (EjsObj*) ejs->falseValue;
    }
    for (cp = sp->value; cp < &sp->value[sp->length]; cp++) {
        if (!isupper((int) *cp)) {
            return (EjsObj*) ejs->falseValue;
        }
    }
    return (EjsObj*) ejs->trueValue;
}


/*
    Return the position of the last occurance of a substring

    function lastIndexOf(pattern: String, start: Number = -1): Number
 */
static EjsObj *lastIndexOf(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    cchar   *pattern;
    int     start, patternLength, index;

    mprAssert(1 <= argc && argc <= 2);

    pattern = ejsGetString(ejs, argv[0]);
    patternLength = ((EjsString*) argv[0])->length;

    if (argc == 2) {
        start = ejsGetInt(ejs, argv[1]);
        if (start >= sp->length) {
            start = sp->length - 1;
        }
        if (start < 0) {
            start = 0;
        }
    } else {
        start = 0;
    }
    index = indexof(sp->value, sp->length, pattern, patternLength, -1);
    if (index < 0) {
        return (EjsObj*) ejs->minusOneValue;
    }
    return (EjsObj*) ejsCreateNumber(ejs, index);
}


/*
    Match a pattern

    function match(pattern: RegExp): Array
 */
static EjsObj *match(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsRegExp   *rp;
    EjsArray    *results;
    EjsString   *match;
    int         matches[EJS_MAX_REGEX_MATCHES * 3];
    int         i, count, len, resultCount;

    rp = (EjsRegExp*) argv[0];
    rp->endLastMatch = 0;
    results = NULL;
    resultCount = 0;

    do {
        count = pcre_exec(rp->compiled, NULL, sp->value, sp->length, rp->endLastMatch, 0, matches, 
            sizeof(matches) / sizeof(int));
        if (count <= 0) {
            break;
        }
        if (results == 0) {
            results = ejsCreateArray(ejs, count);
        }
        for (i = 0; i < count * 2; i += 2) {
            len = matches[i + 1] - matches[i];
            match = ejsCreateStringWithLength(ejs, &sp->value[matches[i]], len);
            ejsSetProperty(ejs, results, resultCount++, match);
            rp->endLastMatch = matches[i + 1];
            if (rp->global) {
                break;
            }
        }
    } while (rp->global);
    if (results == NULL) {
        return (EjsObj*) ejs->nullValue;
    }
    return (EjsObj*) results;
}


static EjsObj *printable(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString       *result;
    char            buf[16];
    int             i, j, k, len, nonprint;

    nonprint = 0;
    for (i = 0; i < sp->length; i++)  {
        if (!isprint((uchar) sp->value[i])) {
            nonprint++;
        }
    }
    if (nonprint == 0) {
        return (EjsObj*) sp;
    }

    result = ejsCreateBareString(ejs, sp->length + (nonprint * 6) + 1);
    if (result == 0) {
        return 0;
    }
    for (i = 0, j = 0; i < sp->length; i++)  {
        if (isprint((uchar) sp->value[i])) {
            result->value[j++] = sp->value[i];
        } else {
            result->value[j++] = '\\';
            result->value[j++] = 'u';
            mprItoa(buf, 4, (uchar) sp->value[i], 16);
            len = (int) strlen(buf);
            for (k = len; k < 4; k++) {
                result->value[j++] = '0';
            }
            for (k = 0; buf[k]; k++) {
                result->value[j++] = buf[k];
            }
        }
    }
    result->value[j] = '\0';
    result->length = j;
    return (EjsObj*) result;
}


static EjsObj *quote(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString       *result;

    result = ejsCreateBareString(ejs, sp->length + 2);
    if (result == 0) {
        return 0;
    }
    memcpy(&result->value[1], sp->value, sp->length);
    result->value[0] = '"';
    result->value[sp->length + 1] = '"';
    result->value[sp->length + 2] = '\0';
    result->length = sp->length + 2;
    return (EjsObj*) result;
}


/*
    Remove characters and return a new string.

    function remove(start: Number, end: Number = -1): String

 */
static EjsObj *removeCharsFromString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString       *result;
    int             start, end, i, j;

    mprAssert(1 <= argc && argc <= 2);

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
    result = ejsCreateBareString(ejs, sp->length - (end - start));
    if (result == 0) {
        return 0;
    }
    for (j = i = 0; i < start; i++, j++) {
        result->value[j] = sp->value[i];
    }
    for (i = end; i < sp->length; i++, j++) {
        result->value[j] = sp->value[i];
    }
    result->value[j] = '\0';
    return (EjsObj*) result;
}


static EjsString *getReplacementText(Ejs *ejs, EjsFunction *fn, int count, int *matches, EjsString *sp)
{
    EjsObj  *result, *argv[EJS_MAX_REGEX_MATCHES * 3];
    int     i, offset, argc;

    mprAssert(fn);
    
    argc = 0;
    argv[argc++] = (EjsObj*) ejsCreateStringWithLength(ejs, &sp->value[matches[0]], matches[1] - matches[0]);
    for (i = 1; i < count; i++) {
        offset = i * 2;
        argv[argc++] = (EjsObj*) ejsCreateStringWithLength(ejs, 
            &sp->value[matches[offset]], matches[offset + 1] - matches[offset]);
    }
    argv[argc++] = (EjsObj*) ejsCreateNumber(ejs, matches[0]);
    argv[argc++] = (EjsObj*) sp;
    if ((result = ejsRunFunction(ejs, fn, ejs->global, argc, argv)) == 0) {
        return 0;
    }
    return (EjsString*) ejsToString(ejs, result);
}


/*
    Search and replace

    function replace(pattern: (String|Regexp), replacement: (String|Function)): String

 */
static EjsObj *replace(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString   *result, *replacement;
    EjsFunction *replacementFunction;
    cchar       *pattern;
    int         matches[EJS_MAX_REGEX_MATCHES * 3];
    int         index, patternLength;

    result = 0;
    if (ejsIsFunction(argv[1])) {
        replacementFunction = (EjsFunction*) argv[1];
        replacement = 0;
    } else {
        replacement = (EjsString*) ejsToString(ejs, argv[1]);
        replacementFunction = 0;
    }

    if (ejsIsString(argv[0])) {
        pattern = ejsGetString(ejs, argv[0]);
        patternLength = ((EjsString*) argv[0])->length;

        index = indexof(sp->value, sp->length, pattern, patternLength, 1);
        if (index >= 0) {
            result = ejsCreateString(ejs, 0);
            if (result == 0) {
                return 0;
            }
            result->obj.permanent = 1;
            catString(ejs, result, sp->value, index);
            if (replacementFunction) {
                matches[0] = matches[2] = index;
                matches[1] = matches[3] = index + patternLength;
                replacement = getReplacementText(ejs, replacementFunction, 2, matches, sp);
            }
            catString(ejs, result, replacement->value, replacement->length);

            index += patternLength;
            if (index < sp->length) {
                catString(ejs, result, &sp->value[index], sp->length - index);
            }

        } else {
            result = ejsDupString(ejs, sp);
        }

    } else if (ejsIsRegExp(argv[0])) {
        EjsRegExp   *rp;
        char        *cp, *lastReplace, *end;
        int         count, endLastMatch, startNextMatch, submatch;

        rp = (EjsRegExp*) argv[0];
        result = ejsCreateString(ejs, 0);
        result->obj.permanent = 1;
        startNextMatch = endLastMatch = 0;

        do {
            if (startNextMatch > sp->length) {
                break;
            }
            count = pcre_exec(rp->compiled, NULL, sp->value, sp->length, startNextMatch, 0, matches, 
                    sizeof(matches) / sizeof(int));
            if (count <= 0) {
                break;
            }
            if (endLastMatch < matches[0]) {
                /* Append prior string text */
                catString(ejs, result, &sp->value[endLastMatch], matches[0] - endLastMatch);
            }
            /*
                Process the replacement template
             */
            if (replacementFunction) {
                replacement = getReplacementText(ejs, replacementFunction, count, matches, sp);
            }
            end = &replacement->value[replacement->length];
            lastReplace = replacement->value;

            for (cp = replacement->value; cp < end; ) {
                if (*cp == '$') {
                    if (lastReplace < cp) {
                        catString(ejs, result, lastReplace, (int) (cp - lastReplace));
                    }
                    switch (*++cp) {
                    case '$':
                        catString(ejs, result, "$", 1);
                        break;
                    case '&':
                        /* Replace the matched string */
                        catString(ejs, result, &sp->value[matches[0]], matches[1] - matches[0]);
                        break;
                    case '`':
                        /* Insert the portion that preceeds the matched string */
                        catString(ejs, result, sp->value, matches[0]);
                        break;
                    case '\'':
                        /* Insert the portion that follows the matched string */
                        catString(ejs, result, &sp->value[matches[1]], sp->length - matches[1]);
                        break;
                    default:
                        /* Insert the nth submatch */
                        if (isdigit((int) *cp)) {
                            submatch = (int) mprAtoi(cp, 10);
                            while (isdigit((int) *++cp))
                                ;
                            cp--;
                            if (submatch < count) {
                                submatch *= 2;
                                catString(ejs, result, &sp->value[matches[submatch]], 
                                    matches[submatch + 1] - matches[submatch]);
                            }

                        } else {
                            ejsThrowArgError(ejs, "Bad replacement $ specification");
                            result->obj.permanent = 0;
                            return 0;
                        }
                    }
                    lastReplace = cp + 1;
                }
                cp++;
            }
            if (lastReplace < cp && lastReplace < end) {
                catString(ejs, result, lastReplace, (int) (cp - lastReplace));
            }
            endLastMatch = matches[1];
            startNextMatch = (startNextMatch == endLastMatch) ? startNextMatch + 1 : endLastMatch;
        } while (rp->global);

        if (endLastMatch < sp->length) {
            /* Append remaining string text */
            catString(ejs, result, &sp->value[endLastMatch], sp->length - endLastMatch);
        }

    } else {
        ejsThrowTypeError(ejs, "Wrong argument type");
        result->obj.permanent = 0;
        return 0;
    }
    result->obj.permanent = 0;
    return (EjsObj*) result;
}


static EjsObj *reverseString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    int         i, j, tmp;

    if ((sp = ejsCreateStringWithLength(ejs, sp->value, sp->length)) == 0) {
        return 0;
    }
    if (sp->length <= 1) {
        return (EjsObj*) sp;
    }
    i = (sp->length - 2) / 2;
    j = (sp->length + 1) / 2;
    for (; i >= 0; i--, j++) {
        tmp = sp->value[i];
        sp->value[i] = sp->value[j];
        sp->value[j] = tmp;
    }
    return (EjsObj*) sp;
}


/*
    Search for a pattern
    function search(pattern: (String | RegExp)): Number
 */
static EjsObj *searchString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    cchar       *pattern;
    int         index, patternLength;

    if (ejsIsString(argv[0])) {
        pattern = ejsGetString(ejs, argv[0]);
        patternLength = ((EjsString*) argv[0])->length;

        index = indexof(sp->value, sp->length, pattern, patternLength, 1);
        return (EjsObj*) ejsCreateNumber(ejs, index);

    } else if (ejsIsRegExp(argv[0])) {
        EjsRegExp   *rp;
        int         matches[EJS_MAX_REGEX_MATCHES * 3];
        int         count;
        rp = (EjsRegExp*) argv[0];
        count = pcre_exec(rp->compiled, NULL, sp->value, sp->length, 0, 0, matches, sizeof(matches) / sizeof(int));
        if (count < 0) {
            return (EjsObj*) ejs->minusOneValue;
        }
        return (EjsObj*) ejsCreateNumber(ejs, matches[0]);

    } else {
        ejsThrowTypeError(ejs, "Wrong argument type");
        return 0;
    }
}


/*
    Return a substring. End is one past the last character.

    function slice(start: Number, end: Number = -1, step: Number = 1): String
 */
static EjsObj *sliceString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString       *result;
    int             start, end, step, i, j, size;

    mprAssert(1 <= argc && argc <= 3);

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
    result = ejsCreateBareString(ejs, size / abs(step) + 1);
    if (result == 0) {
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
    return (EjsObj*) result;
}


/*
    Split a string

    function split(delimiter: (String | RegExp), limit: Number = -1): Array
 */
static EjsObj *split(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsArray    *results;
    EjsString   *elt;
    cchar       *delim;
    char        *cp, *mark, *end;
    int         delimLen, limit;

    mprAssert(1 <= argc && argc <= 2);

    limit = (argc == 2) ? ejsGetInt(ejs, argv[1]): -1;
    results = ejsCreateArray(ejs, 0);

    if (ejsIsString(argv[0])) {
        delim = ejsGetString(ejs, argv[0]);
        delimLen = (int) strlen(delim);
        if (delimLen == 0) {
            for (cp = sp->value; (--limit != -1) && *cp; cp++) {
                ejsSetProperty(ejs, results, -1, ejsCreateStringWithLength(ejs, cp, 1));
            }

        } else {
            end = &sp->value[sp->length];
            for (mark = cp = sp->value; (--limit != -1) && mark < end; mark++) {
                if (strncmp(mark, delim, delimLen) == 0) {
                    elt = ejsCreateStringWithLength(ejs, cp, (int) (mark - cp));
                    ejsSetProperty(ejs, results, -1, elt);
                    cp = mark + delimLen;
                    mark = cp - 1;
                }
            }
            elt = ejsCreateStringWithLength(ejs, cp, (int) (mark - cp));
            ejsSetProperty(ejs, results, -1, elt);
        }
        return (EjsObj*) results;

    } else if (ejsIsRegExp(argv[0])) {
        EjsRegExp   *rp;
        EjsString   *match;
        int         matches[EJS_MAX_REGEX_MATCHES * 3], count, resultCount;
        
        rp = (EjsRegExp*) argv[0];
        rp->endLastMatch = 0;
        resultCount = 0;
        do {
            count = pcre_exec(rp->compiled, NULL, sp->value, sp->length, rp->endLastMatch, 0, matches, 
                sizeof(matches) / sizeof(int));
            if (count <= 0) {
                break;
            }
            if (rp->endLastMatch < matches[0]) {
                match = ejsCreateStringWithLength(ejs, &sp->value[rp->endLastMatch], matches[0] - rp->endLastMatch);
                ejsSetProperty(ejs, results, resultCount++, match);
            }
            rp->endLastMatch = matches[1];
        } while (rp->global);

        if (rp->endLastMatch < sp->length) {
            match = ejsCreateStringWithLength(ejs, &sp->value[rp->endLastMatch], sp->length - rp->endLastMatch);
            ejsSetProperty(ejs, results, resultCount++, match);
        }
        return (EjsObj*) results;
    }

    ejsThrowTypeError(ejs, "Wrong argument type");
    return 0;
}


/**
    Check if a string starts with a given pattern

    function startsWith(pattern: String): Boolean
 */
static EjsObj *startsWith(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    cchar       *pattern;
    int         len;

    mprAssert(argc == 1 && ejsIsString(argv[0]));

    pattern = ejsGetString(ejs, argv[0]);
    len = (int) strlen(pattern);

    return (EjsObj*) ejsCreateBoolean(ejs, strncmp(&sp->value[0], pattern, len) == 0);
}


/*
    Extract a substring. Simple routine with positive indicies.

    function substring(start: Number, end: Number = -1): String
 */
static EjsObj *substring(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    int     start, end, tmp;

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
    return (EjsObj*) ejsCreateStringWithLength(ejs, &sp->value[start], end - start);
}


/*
    Convert the string to camelCase. Return a new string.

    function toCamel(): String
 */
static EjsObj *toCamel(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString   *result;

    result = ejsCreateStringWithLength(ejs, sp->value, sp->length);
    if (result == 0) {
        return 0;
    }
    result->value[0] = tolower((int) sp->value[0]);

    return (EjsObj*) result;
}


/*
    Convert to a JSON string

    override function toJSON(): String
 */
static EjsObj *stringToJSON(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsObj  *result;
    MprBuf  *buf;
    int     i, c;

    buf = mprCreateBuf(sp, 0, 0);
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
    result = (EjsObj*) ejsCreateString(ejs, mprGetBufStart(buf));
    mprFree(buf);
    return result;
}


/*
    Convert the string to lower case.

    function toLowerCase(locale: String = null): String
 */
static EjsObj *toLowerCase(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    EjsString       *result;
    int             i;

    result = ejsCreateStringWithLength(ejs, sp->value, sp->length);
    if (result == 0) {
        return 0;
    }
    for (i = 0; i < result->length; i++) {
        result->value[i] = tolower((int) result->value[i]);
    }
    return (EjsObj*) result;
}


/*
    Convert the string to PascalCase. Return a new string.

    function toPascal(): String
 */
static EjsObj *toPascal(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString   *result;

    result = ejsCreateStringWithLength(ejs, sp->value, sp->length);
    if (result == 0) {
        return 0;
    }
    result->value[0] = toupper((int) sp->value[0]);

    return (EjsObj*) result;
}


/*
    Convert to a string

    override function toString(): String
 */
static EjsObj *stringToString(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    return (EjsObj*) sp;
}


/*
    Convert the string to upper case.
    @return Returns a new upper case version of the string.
    @spec ejs-11

    function toUpperCase(locale: String = null): String
 */
static EjsObj *toUpperCase(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    EjsString       *result;
    int             i;

    result = ejsCreateStringWithLength(ejs, sp->value, sp->length);
    if (result == 0) {
        return 0;
    }
    for (i = 0; i < result->length; i++) {
        result->value[i] = toupper((int) result->value[i]);
    }
    return (EjsObj*) result;
}


/*
    Scan the input and tokenize according to the format string

    function tokenize(format: String): Array
 */
static EjsObj *tokenize(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsArray    *result;
    cchar       *fmt;
    char        *end, *buf;

    mprAssert(argc == 1 && ejsIsString(argv[0]));

    buf = sp->value;
    fmt = ejsGetString(ejs, argv[0]);
    result = ejsCreateArray(ejs, 0);

    for (fmt = ejsGetString(ejs, argv[0]); *fmt && buf < &sp->value[sp->length]; ) {
        if (*fmt++ != '%') {
            continue;
        }
        switch (*fmt) {
        case 's':
            for (end = buf; *end; end++) {
                if (isspace((int) *end)) {
                    break;
                }
            }
            ejsSetProperty(ejs, result, -1, ejsCreateStringWithLength(ejs, buf, (int) (end - buf)));
            buf = end;
            break;

        case 'd':
            ejsSetProperty(ejs, result, -1, ejsParse(ejs, buf, ES_Number));
            while (*buf && !isspace((int) *buf)) {
                buf++;
            }
            break;

        default:
            ejsThrowArgError(ejs, "Bad format specifier");
            return 0;
        }
        while (*buf && isspace((int) *buf)) {
            buf++;
        }
    }
    return (EjsObj*) result;
}

/*
   Flags for trim where
 */
#define TRIM_START  0x1
#define TRIM_END    0x2

static EjsObj *trim(Ejs *ejs, EjsString *sp, cchar *pattern, int where)
{
    cchar           *start, *end, *mark;
    int             index, patternLength;

    if (pattern == 0) {
        start = sp->value;
        if (where & TRIM_START) {
            for (; start < &sp->value[sp->length]; start++) {
                if (!isspace((int) *start)) {
                    break;
                }
            }
        }
        end = &sp->value[sp->length - 1];
        if (where & TRIM_END) {
            for (end = &sp->value[sp->length - 1]; end >= start; end--) {
                if (!isspace((int) *end)) {
                    break;
                }
            }
        }
        end++;
    } else {
        patternLength = strlen(pattern);
        if (patternLength <= 0 || patternLength > sp->length) {
            return (EjsObj*) sp;
        }
        mark = sp->value;
        if (where & TRIM_START) {
            for (; &mark[patternLength] <= &sp->value[sp->length]; mark += patternLength) {
                index = indexof(mark, patternLength, pattern, patternLength, 1);
                if (index != 0) {
                    break;
                }
            }
        }
        start = mark;

        mark = &sp->value[sp->length - patternLength];
        if (where & TRIM_END) {
            for (; mark >= sp->value; mark -= patternLength) {
                index = indexof(mark, patternLength, pattern, patternLength, 1);
                if (index != 0) {
                    break;
                }
            }
        }
        end = mark + patternLength;
    }
    return (EjsObj*) ejsCreateStringWithLength(ejs, start, (int) (end - start));
}


/**
    Returns a trimmed copy of the string. Normally used to trim white space, but can be used to trim any substring
    from the start or end of the string.
    @param str May be set to a substring to trim from the string. If not set, it defaults to any white space.
    @return Returns a (possibly) modified copy of the string

    function trim(str: String = null): String
 */
static EjsObj *trimString(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    cchar   *pattern;

    mprAssert(argc == 0 || (argc == 1 && ejsIsString(argv[0])));

    if (argc == 0) {
        return trim(ejs, sp, NULL, TRIM_START | TRIM_END);

    } else {
        pattern = ejsGetString(ejs, argv[0]);
        return trim(ejs, sp, pattern, TRIM_START | TRIM_END);
    }
}


/*  function trimStart(str: String = null): String
 */
static EjsObj *trimStartString(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    cchar   *pattern;

    mprAssert(argc == 0 || (argc == 1 && ejsIsString(argv[0])));

    if (argc == 0) {
        return trim(ejs, sp, NULL, TRIM_START);

    } else {
        pattern = ejsGetString(ejs, argv[0]);
        return trim(ejs, sp, pattern, TRIM_START);
    }
}


/*  function trimEnd(str: String = null): String
 */
static EjsObj *trimEndString(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    cchar   *pattern;

    mprAssert(argc == 0 || (argc == 1 && ejsIsString(argv[0])));

    if (argc == 0) {
        return trim(ejs, sp, NULL, TRIM_END);

    } else {
        pattern = ejsGetString(ejs, argv[0]);
        return trim(ejs, sp, pattern, TRIM_END);
    }
}

/*********************************** Support **********************************/
/**
    Fast append a string. This modifies the original "dest" string. BEWARE: strings are meant to be immutable.
    Only use this when constructing strings.
 */
static int catString(Ejs *ejs, EjsString *dest, char *str, int len)
{
    EjsString   *castSrc;
    char        *oldBuf, *buf;
    int         oldLen, newLen;

    mprAssert(dest);

    castSrc = 0;

    oldBuf = dest->value;
    oldLen = dest->length;
    newLen = oldLen + len + 1;

#if FUTURE
    if (newLen < MPR_SLAB_STR_MAX) {
        buf = oldBuf;
    } else {
#endif
        buf = (char*) mprRealloc(ejs, oldBuf, newLen);
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


/**
    Fast append a string. This modifies the original "dest" string. BEWARE: strings are meant to be immutable.
    Only use this when constructing strings.
 */
int ejsStrcat(Ejs *ejs, EjsString *dest, EjsObj *src)
{
    EjsString   *castSrc;
    char        *str;
    int         len;

    mprAssert(dest);

    castSrc = 0;
    
    if (ejsIsString(dest)) {
        if (! ejsIsString(src)) {
            castSrc = (EjsString*) ejsToString(ejs, src);
            if (castSrc == 0) {
                return -1;
            }
            len = (int) strlen(castSrc->value);
            str = castSrc->value;

        } else {
            str = ((EjsString*) src)->value;
            len = ((EjsString*) src)->length;
        }
        if (catString(ejs, dest, str, len) < 0) {
            return -1;
        }
    } else {
        /*
            Convert the source to a string and then steal the rusult buffer and assign to the destination
            BUG - should be freeing the destination string.
         */
        castSrc = (EjsString*) ejsToString(ejs, src);
        dest->value = castSrc->value;
        mprStealBlock(dest, dest->value);
        castSrc->value = 0;
    }
    return 0;
}


/*
    Copy a string. Always null terminate.
 */
int ejsStrdup(MprCtx ctx, uchar **dest, cvoid *src, int nbytes)
{
    mprAssert(dest);
    mprAssert(src);

    if (nbytes > 0) {
        *dest = (uchar*) mprAlloc(ctx, nbytes + 1);
        if (*dest == 0) {
            return MPR_ERR_NO_MEMORY;
        }
        strncpy((char*) *dest, (char*) src, nbytes);

    } else {
        *dest = (uchar*) mprAlloc(ctx, 1);
        nbytes = 0;
    }
    (*dest)[nbytes] = '\0';
    return nbytes;
}


/*
    Find a substring. Search forward or backwards. Return the index in the string where the pattern was found.
    Return -1 if not found.
 */
static int indexof(cchar *str, int len, cchar *pattern, int patternLength, int dir)
{
    cchar   *s1, *s2;
    int     i, j;

    mprAssert(dir == 1 || dir == -1);

    if (dir > 0) {
        for (i = 0; i < len; i++) {
            s1 = &str[i];
            for (j = 0, s2 = pattern; j < patternLength; s1++, s2++, j++) {
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
            for (j = 0, s2 = pattern; j < patternLength; s1++, s2++, j++) {
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


/*********************************** Factory **********************************/

EjsString *ejsCreateString(Ejs *ejs, cchar *value)
{
    EjsString   *sp;

    /*
        No need to invoke constructor
     */
    sp = (EjsString*) ejsCreate(ejs, ejs->stringType, 0);
    if (sp != 0) {
        sp->value = mprStrdup(ejs, value);
        if (sp->value == 0) {
            return 0;
        }
        sp->length = (int) strlen(sp->value);
        ejsSetDebugName(sp, sp->value);
    }
    return sp;
}


EjsString *ejsCreateStringAndFree(Ejs *ejs, char *value)
{
    EjsString   *sp;

    sp = (EjsString*) ejsCreate(ejs, ejs->stringType, 0);
    if (sp != 0) {
        if (value == 0) {
            value = mprStrdup(sp, "");
        }
        sp->value = value;
        mprStealBlock(sp, value);
        sp->length = (int) strlen(sp->value);
        ejsSetDebugName(sp, sp->value);
    }
    return sp;
}


EjsString *ejsDupString(Ejs *ejs, EjsString *sp)
{
    return ejsCreateStringWithLength(ejs, sp->value, sp->length);
}


/*
    Initialize a binary string value.
 */
EjsString *ejsCreateStringWithLength(Ejs *ejs, cchar *value, int len)
{
    EjsString   *sp;
    uchar       *dest;

    //  OPT Would be much faster to allocate the string value in the actual object since strings are immutable
    sp = (EjsString*) ejsCreate(ejs, ejs->stringType, 0);
    if (sp != 0) {
        sp->length = ejsStrdup(ejs, &dest, value, len);
        sp->value = (char*) dest;
        if (sp->length < 0) {
            return 0;
        }
    }
    return sp;
}


/*
    Initialize an string with a pre-allocated buffer but without data..
 */
EjsString *ejsCreateBareString(Ejs *ejs, int len)
{
    EjsString   *sp;
    
    //  OPT Would be much faster to allocate the string value in the actual object since strings are immutable
    sp = (EjsString*) ejsCreate(ejs, ejs->stringType, 0);
    if (sp != 0) {
        sp->value = mprAlloc(sp, len + 1);
        if (sp->value == 0) {
            return 0;
        }
        sp->length = len;
        sp->value[len] = '\0';
    }
    return sp;
}


void ejsCreateStringType(Ejs *ejs)
{
    EjsType     *type;

    type = ejs->stringType = ejsCreateNativeType(ejs, "ejs", "String", ES_String, sizeof(EjsString));

    type->helpers.cast = (EjsCastHelper) castString;
    type->helpers.clone = (EjsCloneHelper) cloneString;
    type->helpers.destroy = (EjsDestroyHelper) destroyString;
    type->helpers.getProperty = (EjsGetPropertyHelper) getStringProperty;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeStringOperator;
    type->helpers.lookupProperty = (EjsLookupPropertyHelper) lookupStringProperty;

    type->numericIndicies = 1;

    /*
        Pre-create the empty string.
     */
    ejs->emptyStringValue = (EjsString*) ejsCreateString(ejs, "");
    ejsSetDebugName(ejs->emptyStringValue, "emptyString");
}


void ejsConfigureStringType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *prototype;

    type = ejsGetTypeByName(ejs, "ejs", "String");
    prototype = type->prototype;

    ejsSetProperty(ejs, ejs->global, ES_string, type);

    ejsBindMethod(ejs, type, ES_String_fromCharCode, (EjsProc) fromCharCode);

    ejsBindConstructor(ejs, type, (EjsProc) stringConstructor);
    ejsBindMethod(ejs, prototype, ES_String_caseCompare, (EjsProc) caseCompare);
    ejsBindMethod(ejs, prototype, ES_String_caselessCompare, (EjsProc) caselessCompare);
    ejsBindMethod(ejs, prototype, ES_String_charAt, (EjsProc) charAt);
    ejsBindMethod(ejs, prototype, ES_String_charCodeAt, (EjsProc) charCodeAt);
    ejsBindMethod(ejs, prototype, ES_String_concat, (EjsProc) concatString);
    ejsBindMethod(ejs, prototype, ES_String_contains, (EjsProc) containsString);
    ejsBindMethod(ejs, prototype, ES_String_endsWith, (EjsProc) endsWith);
    ejsBindMethod(ejs, prototype, ES_String_format, (EjsProc) formatString);
    ejsBindMethod(ejs, prototype, ES_String_iterator_get, (EjsProc) getStringIterator);
    ejsBindMethod(ejs, prototype, ES_String_iterator_getValues, (EjsProc) getStringValues);
    ejsBindMethod(ejs, prototype, ES_String_indexOf, (EjsProc) indexOf);
    ejsBindMethod(ejs, prototype, ES_String_isDigit, (EjsProc) isDigit);
    ejsBindMethod(ejs, prototype, ES_String_isAlpha, (EjsProc) isAlpha);
    ejsBindMethod(ejs, prototype, ES_String_isAlphaNum, (EjsProc) isAlphaNum);
    ejsBindMethod(ejs, prototype, ES_String_isLower, (EjsProc) isLower);
    ejsBindMethod(ejs, prototype, ES_String_isSpace, (EjsProc) isSpace);
    ejsBindMethod(ejs, prototype, ES_String_isUpper, (EjsProc) isUpper);
    ejsBindMethod(ejs, prototype, ES_String_lastIndexOf, (EjsProc) lastIndexOf);
    ejsBindMethod(ejs, prototype, ES_String_length, (EjsProc) stringLength);
    ejsBindMethod(ejs, prototype, ES_String_match, (EjsProc) match);
    ejsBindMethod(ejs, prototype, ES_String_remove, (EjsProc) removeCharsFromString);
    ejsBindMethod(ejs, prototype, ES_String_slice, (EjsProc) sliceString);
    ejsBindMethod(ejs, prototype, ES_String_split, (EjsProc) split);
    ejsBindMethod(ejs, prototype, ES_String_printable, (EjsProc) printable);
    ejsBindMethod(ejs, prototype, ES_String_quote, (EjsProc) quote);
    ejsBindMethod(ejs, prototype, ES_String_replace, (EjsProc) replace);
    ejsBindMethod(ejs, prototype, ES_String_reverse, (EjsProc) reverseString);
    ejsBindMethod(ejs, prototype, ES_String_search, (EjsProc) searchString);
    ejsBindMethod(ejs, prototype, ES_String_startsWith, (EjsProc) startsWith);
    ejsBindMethod(ejs, prototype, ES_String_substring, (EjsProc) substring);
    ejsBindMethod(ejs, prototype, ES_String_toCamel, (EjsProc) toCamel);
    ejsBindMethod(ejs, prototype, ES_String_toJSON, (EjsProc) stringToJSON);
    ejsBindMethod(ejs, prototype, ES_String_toLowerCase, (EjsProc) toLowerCase);
    ejsBindMethod(ejs, prototype, ES_String_toPascal, (EjsProc) toPascal);
    ejsBindMethod(ejs, prototype, ES_String_toString, (EjsProc) stringToString);
    ejsBindMethod(ejs, prototype, ES_String_toUpperCase, (EjsProc) toUpperCase);
    ejsBindMethod(ejs, prototype, ES_String_tokenize, (EjsProc) tokenize);
    ejsBindMethod(ejs, prototype, ES_String_trim, (EjsProc) trimString);
    ejsBindMethod(ejs, prototype, ES_String_trimStart, (EjsProc) trimStartString);
    ejsBindMethod(ejs, prototype, ES_String_trimEnd, (EjsProc) trimEndString);

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
