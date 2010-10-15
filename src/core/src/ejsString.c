/**
    ejsString.c - Ejscript string class

    Copyright (c) All Rights Reserved. See details at the end of the file.

    #ifdef UNICODE
        #define U(s) L ## s
    #else
        #define U(s) s
    #endif
 */

/********************************** Includes **********************************/

#include    "ejs.h"
#include    "pcre.h"

/*********************************** Locals ***********************************/

//  MOB - review min size
static int internHashSizes[] = {
     389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 0
};

/***************************** Forward Declarations ***************************/

static EjsString *buildString(Ejs *ejs, EjsString *buf, char *str, int len);
static int indexof(EjsChar *str, int len, EjsString *pattern, int patternLength, int dir);
static int getInternHashSize(int size);
static int remakeInternHash(EjsHeap *heap);

/************************************* Code ***********************************/

int ejsCreateStringService(Ejs *ejs)
{
    EjsHeap     *heap;

    heap = ejs->heap;
    remakeInternHash(heap);
    return 0;
}


/******************************************************************************/
/*
    Cast the string operand to a primitive type
 */
static EjsObj *castString(Ejs *ejs, EjsString *sp, EjsType *type)
{
    EjsObj  *result;

    mprAssert(sp);
    mprAssert(type);

    if (type == ejs->pathType) {
        return (EjsObj*) ejsCreatePath(ejs, sp);
    } else if (type == ejs->uriType) {
        return (EjsObj*) ejsCreateUri(ejs, sp);
    }
    switch (type->id) {
    case ES_Boolean:
        if (sp != ejs->emptyString) {
            return (EjsObj*) ejs->trueValue;
        }
        return (EjsObj*) ejs->falseValue;

    case ES_Number:
        return (EjsObj*) ejsParse(ejs, ejsToMulti(ejs, sp), ES_Number);

    case ES_RegExp:
        if (sp && sp->value[0] == '/') {
            return (EjsObj*) ejsCreateRegExp(ejs, sp);
        }
        result = (EjsObj*) ejsCreateRegExp(ejs, ejsSprintf(ejs, "/%S/", sp));
        return result;

    case ES_String:
        return (EjsObj*) sp;

    default:
        ejsThrowTypeError(ejs, "Can't cast to required type");
        return 0;
    }
    return 0;
}


static EjsString *cloneString(Ejs *ejs, EjsString *sp, bool deep)
{
    /* Strings immutable */
    return sp;
}


/*
    Get a string element. Slot numbers correspond to character indicies.
 */
static EjsObj *getStringProperty(Ejs *ejs, EjsString *sp, int index)
{
    if (index < 0 || index >= sp->length) {
        return (EjsObj*) ejs->emptyString;
        return 0;
    }
    return (EjsObj*) ejsCreateStringWithLength(ejs, &sp->value[index], 1);
}


static EV *coerceStringOperands(Ejs *ejs, EV *lhs, int opcode,  EV *rhs)
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
        if (ejsIsNumber(ejs, rhs)) {
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
        return ejs->trueValue;

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return ejs->falseValue;

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
        return (((EjsString*) lhs) ? ejs->trueValue : ejs->falseValue);

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return (((EjsString*) lhs) ? ejs->falseValue: ejs->trueValue);

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return ejs->falseValue;

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %S", opcode, TYPE(lhs)->qname.name);
        return ejs->undefinedValue;
    }
    return 0;
}


static EV *invokeStringOperator(Ejs *ejs, EjsString *lhs, int opcode,  EjsString *rhs, void *data)
{
    EV      *result, *arg;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if (!ejsIsA(ejs, lhs, ejs->stringType) || !ejsIsA(ejs, rhs, ejs->stringType)) {
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
        if (lhs == rhs) {
            return ejs->trueValue;
        }
        return ejs->falseValue;

    case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_STRICTLY_NE:
        if (lhs != rhs) {
            return ejs->trueValue;
        }
        return ejs->falseValue;

    case EJS_OP_COMPARE_LT:
        return ejsCreateBoolean(ejs, 
            mprMemcmp(lhs->value, lhs->length * sizeof(EjsChar), rhs->value, rhs->length * sizeof(EjsChar)) < 0);

    case EJS_OP_COMPARE_LE:
        return ejsCreateBoolean(ejs, 
            mprMemcmp(lhs->value, lhs->length * sizeof(EjsChar), rhs->value, rhs->length * sizeof(EjsChar)) <= 0);

    case EJS_OP_COMPARE_GT:
        return ejsCreateBoolean(ejs, 
            mprMemcmp(lhs->value, lhs->length * sizeof(EjsChar), rhs->value, rhs->length * sizeof(EjsChar)) > 0);

    case EJS_OP_COMPARE_GE:
        return ejsCreateBoolean(ejs, 
                mprMemcmp(lhs->value, lhs->length * sizeof(EjsChar), rhs->value, rhs->length * sizeof(EjsChar)) >= 0);

    /*
        Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return ((lhs) ? ejs->trueValue: ejs->falseValue);

    case EJS_OP_COMPARE_ZERO:
        return ((lhs == 0) ? ejs->trueValue: ejs->falseValue);


    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return ejs->falseValue;

    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsCatString(ejs, lhs, rhs);

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
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %S", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
    mprAssert(0);
}


/*
    Lookup an string index.
 */
static int lookupStringProperty(Ejs *ejs, EjsString *sp, EjsName qname)
{
    int     index;

    //  MOB UNICODE
    if (!isdigit((int) qname.name->value[0])) {
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
static EjsObj *stringConstructor(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsArray    *args;

    mprAssert(argc == 0 || (argc == 1 && ejsIsArray(ejs, argv[0])));
    sp->length = 0;

    if (argc == 1) {
        args = (EjsArray*) argv[0];
        if (args->length > 0) {
            sp = ejsToString(ejs, ejsGetProperty(ejs, (EjsObj*) args, 0));
        } else {
            sp->value[0] = 0;
            sp->length = 0;
        }
    } else {
        sp->value[0] = 0;
        sp->length = 0;
    }
    //  MOB -- this must update the new object
    return (EjsObj*) sp;
}


/*
    Do a case sensitive comparison between this string and another.

    function caseCompare(compare: String): Number
 */
static EjsObj *caseCompare(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    int     result;

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));

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

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));

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

    mprAssert(argc == 1 && ejsIsNumber(ejs, argv[0]));
    index = ejsGetInt(ejs, argv[0]);
    if (index < 0 || index >= sp->length) {
        return (EjsObj*) ejs->emptyString;
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
    EjsString   *result, *str;
    int         i, count;

    mprAssert(argc == 1 && ejsIsArray(ejs, argv[0]));
    args = (EjsArray*) argv[0];

    result = (EjsString*) ejsClone(ejs, sp, 0);
    count = ejsGetPropertyCount(ejs, (EjsObj*) args);
    for (i = 0; i < args->length; i++) {
        str = ejsToString(ejs, ejsGetProperty(ejs, (EjsObj*) args, i));
        if ((result = ejsCatString(ejs, result, str)) == NULL) {
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

    if (ejsIsString(ejs, pat)) {
        return (EjsObj*) ejsCreateBoolean(ejs, strstr(sp->value, ((EjsString*) pat)->value) != 0);
    } else if (ejsIsRegExp(ejs, pat)) {
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

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));

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

    mprAssert(argc == 1 && ejsIsArray(ejs, argv[0]));

    args = (EjsArray*) argv[0];

    /*
        Flatten the args if there is only one element and it is itself an array. This happens when invoked
        via the overloaded operator '%' which in turn invokes format()
     */
    if (args->length == 1) {
        inner = ejsGetProperty(ejs, (EjsObj*) args, 0);
        if (ejsIsArray(ejs, inner)) {
            args = inner;
        }
    }
    if ((result = ejsCreateBareString(ejs, EJS_BUFSIZE)) == NULL) {
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
            result = buildString(ejs, result, buf, (int) strlen(buf));
            mprFree(buf);
            last = i + 1;
            nextArg++;
        } else if (kind == '%') {
            result = buildString(ejs, result, "%", 1);
            last = i + 1;
        }
    }
    i = (int) strlen(sp->value);
    if (i > last) {
        result = buildString(ejs, result, &sp->value[last], i - last);
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

    mprAssert(argc == 1 && ejsIsArray(ejs, argv[0]));
    args = (EjsArray*) argv[0];

    if ((result = (EjsString*) ejsCreateBareString(ejs, argc + 1)) == NULL) {
        return 0;
    }
    for (i = 0; i < args->length; i++) {
        vp = ejsGetProperty(ejs, (EjsObj*) args, i);
        result->value[i] = ejsGetInt(ejs, ejsToNumber(ejs, vp));
    }
    result->value[i] = '\0';
    result->length = args->length;
    return (EjsObj*) ejsInternString(ejs, result);
}


/*
    Function to iterate and return the next character code.
    NOTE: this is not a method of String. Rather, it is a callback function for Iterator
 */
static EjsObj *nextStringKey(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsString   *sp;

    sp = (EjsString*) ip->target;

    if (!ejsIsString(ejs, sp)) {
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
    if (!ejsIsString(ejs, sp)) {
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
static EjsObj *indexOf(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    EjsString   *pattern;
    int         index, start, patternLength;

    mprAssert(1 <= argc && argc <= 2);
    mprAssert(ejsIsString(ejs, argv[0]));

    pattern = (EjsString*) argv[0];
    patternLength = pattern->length;

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
    EjsString   *pattern;
    int         start, patternLength, index;

    mprAssert(1 <= argc && argc <= 2);

    pattern = (EjsString*) argv[0];
    patternLength = pattern->length;

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

    if ((result = ejsCreateBareString(ejs, sp->length + (nonprint * 6) + 1)) == NULL) {
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
    return (EjsObj*) ejsInternString(ejs, result);
}


static EjsObj *quote(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
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
    return (EjsObj*) ejsInternString(ejs, result);
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
    return (EjsObj*) ejsInternString(ejs, result);
}


static EjsString *getReplacementText(Ejs *ejs, EjsFunction *fn, int count, int *matches, EjsString *sp)
{
    EV      *result, *argv[EJS_MAX_REGEX_MATCHES * 3];
    int     i, offset, argc;

    mprAssert(fn);
    
    argc = 0;
    argv[argc++] = ejsCreateStringWithLength(ejs, &sp->value[matches[0]], matches[1] - matches[0]);
    for (i = 1; i < count; i++) {
        offset = i * 2;
        argv[argc++] = ejsCreateStringWithLength(ejs, 
            &sp->value[matches[offset]], matches[offset + 1] - matches[offset]);
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
static EjsObj *replace(Ejs *ejs, EjsString *sp, int argc, EjsObj **argv)
{
    EjsString   *result, *replacement, *pattern;
    EjsFunction *replacementFunction;
    int         matches[EJS_MAX_REGEX_MATCHES * 3];
    int         index, patternLength, enabled;

    result = 0;
    if (ejsIsFunction(ejs, argv[1])) {
        replacementFunction = (EjsFunction*) argv[1];
        replacement = 0;
    } else {
        replacement = (EjsString*) ejsToString(ejs, argv[1]);
        replacementFunction = 0;
    }

    if (ejsIsString(ejs, argv[0])) {
        pattern = (EjsString*) argv[0];
        patternLength = pattern->length;

        index = indexof(sp->value, sp->length, pattern, patternLength, 1);
        if (index >= 0) {
            if ((result = ejsCreateBareString(ejs, EJS_BUFSIZE)) == NULL) {
                return 0;
            }
            result->length = 0;
            result = buildString(ejs, result, sp->value, index);
            if (replacementFunction) {
                matches[0] = matches[2] = index;
                matches[1] = matches[3] = index + patternLength;
                enabled = ejsEnableGC(ejs, 0);
                replacement = getReplacementText(ejs, replacementFunction, 2, matches, sp);
                ejsEnableGC(ejs, enabled);
            }
            result = buildString(ejs, result, replacement->value, replacement->length);

            index += patternLength;
            if (index < sp->length) {
                result = buildString(ejs, result, &sp->value[index], sp->length - index);
            }
        } else {
            result = (EjsString*) ejsClone(ejs, sp, 0);
        }

    } else if (ejsIsRegExp(ejs, argv[0])) {
        EjsRegExp   *rp;
        char        *cp, *lastReplace, *end;
        int         count, endLastMatch, startNextMatch, submatch;

        rp = (EjsRegExp*) argv[0];
        result = ejsCreateBareString(ejs, EJS_BUFSIZE);
        result->length = 0;
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
                result = buildString(ejs, result, &sp->value[endLastMatch], matches[0] - endLastMatch);
            }
            /*
                Process the replacement template
             */
            if (replacementFunction) {
                enabled = ejsEnableGC(ejs, 0);
                replacement = getReplacementText(ejs, replacementFunction, count, matches, sp);
                ejsEnableGC(ejs, enabled);
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
                        result = buildString(ejs, result, "$", 1);
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
                        /* Insert the nth submatch */
                        if (isdigit((int) *cp)) {
                            submatch = (int) mprAtoi(cp, 10);
                            while (isdigit((int) *++cp))
                                ;
                            cp--;
                            if (submatch < count) {
                                submatch *= 2;
                                result = buildString(ejs, result, &sp->value[matches[submatch]], 
                                    matches[submatch + 1] - matches[submatch]);
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
            startNextMatch = (startNextMatch == endLastMatch) ? startNextMatch + 1 : endLastMatch;
        } while (rp->global);

        if (endLastMatch < sp->length) {
            /* Append remaining string text */
            result = buildString(ejs, result, &sp->value[endLastMatch], sp->length - endLastMatch);
        }

    } else {
        ejsThrowTypeError(ejs, "Wrong argument type");
        return 0;
    }
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
    EjsString   *pattern;
    int         index, patternLength;

    if (ejsIsString(ejs, argv[0])) {
        pattern = (EjsString*) argv[0];
        patternLength = pattern->length;

        index = indexof(sp->value, sp->length, pattern, patternLength, 1);
        return (EjsObj*) ejsCreateNumber(ejs, index);

    } else if (ejsIsRegExp(ejs, argv[0])) {
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
    if ((result = ejsCreateBareString(ejs, size / abs(step) + 1)) == NULL) {
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
    return (EjsObj*) ejsInternString(ejs, result);
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

    if (ejsIsString(ejs, argv[0])) {
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

    } else if (ejsIsRegExp(ejs, argv[0])) {
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
            if (rp->endLastMatch <= matches[0]) {
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

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));

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
EjsString *ejsStringToJSON(Ejs *ejs, EV *vp)
{
    EjsString   *result;
    EjsString   *sp;
    MprBuf      *buf;
    int         i, c;

    if (ejsIsString(ejs, vp)) {
        sp = (EjsString*) vp;
    } else {
        sp = ejsToString(ejs, vp);
    }
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
    result = ejsCreateStringFromCS(ejs, mprGetBufStart(buf));
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

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));

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


static EjsObj *trim(Ejs *ejs, EjsString *sp, EjsString *pattern, int where)
{
    EjsChar     *start, *end, *mark;
    int         index, patternLength;

    if (pattern == 0) {
        start = sp->value;
        if (where & EJS_TRIM_START) {
            for (; start < &sp->value[sp->length]; start++) {
                if (!isspace((int) *start)) {
                    break;
                }
            }
        }
        end = &sp->value[sp->length - 1];
        if (where & EJS_TRIM_END) {
            for (end = &sp->value[sp->length - 1]; end >= start; end--) {
                if (!isspace((int) *end)) {
                    break;
                }
            }
        }
        end++;
    } else {
        patternLength = pattern->length;
        if (patternLength <= 0 || patternLength > sp->length) {
            return (EjsObj*) sp;
        }
        mark = sp->value;
        if (where & EJS_TRIM_START) {
            for (; &mark[patternLength] <= &sp->value[sp->length]; mark += patternLength) {
                index = indexof(mark, patternLength, pattern, patternLength, 1);
                if (index != 0) {
                    break;
                }
            }
        }
        start = mark;

        mark = &sp->value[sp->length - patternLength];
        if (where & EJS_TRIM_END) {
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
    EjsString   *pattern;

    mprAssert(argc == 0 || (argc == 1 && ejsIsString(ejs, argv[0])));

    if (argc == 0) {
        return trim(ejs, sp, NULL, EJS_TRIM_START | EJS_TRIM_END);

    } else {
        pattern = (EjsString*) argv[0];
        return trim(ejs, sp, pattern, EJS_TRIM_START | EJS_TRIM_END);
    }
}


/*  
    function trimStart(str: String = null): String
 */
static EjsObj *trimStartString(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    EjsString   *pattern;

    mprAssert(argc == 0 || (argc == 1 && ejsIsString(ejs, argv[0])));

    if (argc == 0) {
        return trim(ejs, sp, NULL, EJS_TRIM_START);

    } else {
        pattern = (EjsString*) argv[0];
        return trim(ejs, sp, pattern, EJS_TRIM_START);
    }
}


/*  
    function trimEnd(str: String = null): String
 */
static EjsObj *trimEndString(Ejs *ejs, EjsString *sp, int argc,  EjsObj **argv)
{
    EjsString   *pattern;

    mprAssert(argc == 0 || (argc == 1 && ejsIsString(ejs, argv[0])));

    if (argc == 0) {
        return trim(ejs, sp, NULL, EJS_TRIM_END);

    } else {
        pattern = (EjsString*) argv[0];
        return trim(ejs, sp, pattern, EJS_TRIM_END);
    }
}

/*********************************** Support **********************************/
#if UNUSED
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
#endif


static EjsString *buildString(Ejs *ejs, EjsString *buf, char *str, int len)
{
    EjsString   *newBuf;
    int         size;

    mprAssert(buf);

    size = ejsGetBlockSize(buf);
    if (buf->length + len >= size) {
        len = max(buf->length + len, EJS_BUFSIZE);
        if ((newBuf = ejsCreateBareString(ejs, len)) == NULL) {
            return NULL;
        }
        memcpy(newBuf->value, buf->value, buf->length);
        newBuf->length = buf->length;
        buf = newBuf;
    }
    //  MOB -- UNICODE
    memcpy(&buf->value[buf->length], str, len);
    buf->value[buf->length] = 0;
    return buf;
}


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


/*
    Find a substring. Search forward or backwards. Return the index in the string where the pattern was found.
    Return -1 if not found.
 */
static int indexof(EjsChar *str, int len, EjsString *pattern, int patternLength, int dir)
{
    EjsChar     *s1, *s2;
    int         i, j;

    mprAssert(dir == 1 || dir == -1);

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


static int remakeInternHash(EjsHeap *heap)
{
    Ejs         *ejs;
    EjsMem      **oldBuckets, *sp;
    int         i, newSize, oldSize;

    mprAssert(heap);
    ejs = heap->ejs;

    newSize = getInternHashSize(heap->strings.count);
    if (heap->strings.buckets) {
        oldSize = heap->strings.size;
        if (oldSize > newSize) {
            return 0;
        }
    }
    oldBuckets = heap->strings.buckets;
    if ((heap->strings.buckets = (EjsMem**) ejsAlloc(ejs, (newSize * sizeof(EjsMem*)))) == NULL) {
        return EJS_ERR;
    }
    ejsMakePermanent(ejs, heap->strings.buckets);
    heap->strings.size = newSize;
    memset(heap->strings.buckets, 0, newSize * sizeof(EjsMem*));

    if (oldBuckets) {
        for (sp = oldBuckets[i]; sp; sp = sp->next) {
            ejsInternString(ejs, (EjsString*) EJS_GET_PTR(sp));
        }
        ejsMakeTransient(ejs, oldBuckets);
    }
    return 0;
}


static int toMulti(char *dest, EjsChar *src, int len)
{
#if BLD_CHAR_LEN == 1
    if (dest) {
        mprStrcpy(dest, len, src);
    } else {
        len = min(strlen(src), len);
    }
#else
#if WIN || WINCE
    len = WideCharToMultiByte(CP_ACP, 0, src, -1, dest, (DWORD) len, NULL, NULL);
#else
    //  MOB -- Linux wchar_t is 32 bits and so can't use this
    len = wcstombs(dest, src, len);
#endif
#endif
    return len;
}


static int toUni(EjsChar *dest, cchar *src, int len) 
{
#if BLD_CHAR_LEN == 1
    if (dest) {
        mprStrcpy(dest, len, src);
    } else {
        len = min(strlen(src), len);
    }
#else
#if WIN || WINCE
    len = MultiByteToWideChar(CP_ACP, 0, src, -1, dest, len);
#else
    //  MOB -- Linux wchar_t is 32 bits and so can't use this
    len = mbstowcs(dest, src, len);
#endif
#endif
    return len;
}


static inline void unlinkBlock(EjsMem *bp)
{
    bp->prev->next = bp->next;
    bp->next->prev = bp->prev;
}

/******************************************** String API *******************************************/

int ejsAtoi(Ejs *ejs, EjsString *sp, int radix)
{
    int         num, i;

    for (i = 0; i < sp->length; i++) {
        num = num * radix + sp->value[i];
    }
    return num;
}


EjsString *ejsCatString(Ejs *ejs, EjsString *dest, EjsString *src)
{
    EjsString   *result;
    int         len;

    len = dest->length + src->length;
    if ((result = ejsCreateBareString(ejs, len)) == NULL) {
        return NULL;
    }
    memcpy(dest->value, src->value, src->length);
    memcpy(&dest->value[dest->length], src->value, src->length);
    ejsInternString(ejs, result);
    return result;
}



/*
    Catenate a set of unicode string arguments onto another.
 */
EjsString *ejsCatStrings(Ejs *ejs, EjsString *src, ...)
{
    EjsString   *sp, *result;
    va_list     args;
    int         len;

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
    ejsInternString(ejs, result);
    return result;
}


int ejsContainsCString(Ejs *ejs, EjsString *sp, cchar *pat)
{
    int     i, j, len;

    mprAssert(sp);
    mprAssert(pat);

    if (pat == 0 || *pat == '\0') {
        return 0;
    }
    len = strlen(pat);
    
    for (i = 0; i < sp->length; i++) {
        for (j = 0; j < len; j++) {
            if (sp->value[i] != pat[j]) {
                break;
            }
        }
        if (j == len) {
            return i;
        }
    }
    return -1;
}


int ejsStartsWithCString(Ejs *ejs, EjsString *sp, cchar *pat)
{
    int     i, j, len;

    mprAssert(sp);
    mprAssert(pat);

    if (pat == 0 || *pat == '\0') {
        return 0;
    }
    len = strlen(pat);
    
    for (i = 0; pat[i] && i < sp->length; i++) {
        if (sp->value[i] != pat[j]) {
            break;
        }
    }
    if (pat[i]) {
        return 0;
    }
    return 1;
}


int ejsCompareCString(Ejs *ejs, EjsString *sp, cchar *str)
{
    EjsChar     *s1;
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
    if (s1 == s2) {
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


int ejsCompareString(Ejs *ejs, EjsString *sp1, EjsString *sp2)
{
    EjsChar     *s1;
    EjsChar     *s2;
    int         rc;

    s1 = sp1->value;
    s2 = sp2->value;

    if (s1 == 0) {
        return -1;
    }
    if (s2 == 0) {
        return 1;
    }
    if (s1 == s2) {
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


int ejsCompareUString(Ejs *ejs, EjsString *sp, EjsChar *str)
{
    EjsChar     *s1;
    EjsChar     *s2;
    int         rc;

    s1 = sp->value;
    s2 = str;

    if (s1 == 0) {
        return -1;
    }
    if (s2 == 0) {
        return 1;
    }
    if (s1 == s2) {
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
    Return the index in a unicode string of a given unicode character code. Return -1 if not found.
 */
int ejsContainsChar(Ejs *ejs, EjsString *sp, int charPat)
{
    int     i;

    mprAssert(sp);

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
int ejsContainsStringAnyCase(Ejs *ejs, EjsString *sp, EjsString *pat)
{
    int     i, j;

    mprAssert(sp);
    mprAssert(pat);
    mprAssert(pat->value);

    for (i = 0; i < sp->length; i++) {
        for (j = 0; j < pat->length; j++) {
            //  MOB - tolower only works for ASCII
            if (tolower(sp->value[i]) != tolower(pat->value[j])) {
                break;
            }
        }
        if (j == pat->length) {
            return i;
        }
    }
    return -1;
}
#endif


int ejsContainsString(Ejs *ejs, EjsString *sp, EjsString *pat)
{
    int     i, j;

    mprAssert(sp);
    mprAssert(pat);
    mprAssert(pat->value);

    if (pat == 0 || pat->value == 0) {
        return 0;
    }
    for (i = 0; i < sp->length; i++) {
        for (j = 0; j < pat->length; j++) {
            if (sp->value[i] != pat->value[j]) {
                break;
            }
        }
        if (j == pat->length) {
            return i;
        }
    }
    return -1;
}


int ejsContainsUString(Ejs *ejs, EjsString *sp, EjsChar *pat)
{
    EjsChar     *cp;
    int         i;

    mprAssert(sp);
    mprAssert(pat);

    if (pat == NULL) {
        return 0;
    }
    for (i = 0; i < sp->length; i++) {
        for (cp = pat; *cp; cp++) {
            if (sp->value[i] != *cp) {
                break;
            }
        }
        if (*cp == 0) {
            return i;
        }
    }
    return -1;
}


//  MOB -- should be ejsGetCString
cchar *ejsGetString(Ejs *ejs, void *obj)
{
    mprAssert(obj);
    if (!ejsIsString(ejs, obj)) {
        if ((obj = ejsCast(ejs, obj, ejs->stringType)) == 0) {
            return "";
        }
    }
    mprAssert(ejsIsString(ejs, obj));
    return ejsToMulti(ejs, (EjsString*) obj);
}


//  MOB -- move to ejsAlloc
EjsString *ejsInternString(Ejs *ejs, EjsString *sp)
{
    EjsHeap     *heap;
    EjsMem      *bp, *chain, *last;
    EjsString   *np;
    int         index, i;

    heap = ejs->heap;

    index = ejsComputeStringHashCode(sp, heap->strings.size);
    if ((chain = heap->strings.buckets[index]) != NULL) {
        for (; chain; chain = chain->next) {
            np = (EjsString*) EJS_GET_PTR(chain);
            for (i = 0; i < np->length; i++) {
                if (np->value[i] != sp->value[i]) {
                    break;
                }
            }
            if (i == np->length) {
                return np;
            }
            last = chain;
        }
    } else {
        last = NULL;
    }
    bp = EJS_GET_MEM(sp);
    unlinkBlock(bp);
    if (last) {
        last->next = bp;
    } else {
        heap->strings.buckets[index] = bp;
    }
    bp->next = NULL;
    if (++heap->strings.count > heap->strings.size) {
        /*  Remake the entire hash - should not happen often */
        remakeInternHash(heap);
    }
    mprAssert(bp);
    return sp;
}


EjsString *ejsSprintf(Ejs *ejs, cchar *fmt, ...)
{
    va_list     ap;
    char        *result;

    mprAssert(fmt);

    va_start(ap, fmt);
    result = mprVasprintf(ejs, -1, fmt, ap);
    va_end(ap);
    return ejsCreateStringAndFree(ejs, result);
}


EjsString *ejsSubString(Ejs *ejs, EjsString *src, int start, int len)
{
    EjsString   *result;

    mprAssert(src);
    mprAssert(start >= 0);
    mprAssert((start + len) <= src->length);

    if (len < 0) {
        len = src->length - start;
    }
    if ((start + len) > src->length || start < 0) {
        return ejs->emptyString;
    }
    len = min(len, src->length);
    if ((result = ejsCreateBareString(ejs, len)) == NULL) {
        return NULL;
    }
    memcpy(result->value, &src->value[start], len);
    ejsInternString(ejs, result);
    return result;
}


EjsString *ejsToLower(Ejs *ejs, EjsString *sp)
{
    EjsString   *result;
    EjsChar     *cp;

    mprAssert(sp);
    mprAssert(sp->value);

    result = (EjsString*) ejsClone(ejs, sp, 0);
    for (cp = result->value; *cp; cp++) {
        //  MOB -- UNICODE
        *cp = tolower(*cp);
    }
    ejsInternString(ejs, result);
    return result;
}


EjsString *ejsToUpper(Ejs *ejs, EjsString *sp)
{
    EjsString   *result;
    EjsChar     *cp;

    mprAssert(sp);
    mprAssert(sp->value);

    result = (EjsString*) ejsClone(ejs, sp, 0);
    for (cp = result->value; *cp; cp++) {
        //  MOB -- UNICODE
        *cp = toupper(*cp);
    }
    ejsInternString(ejs, result);
    return result;
}


EjsString *ejsToUni(Ejs *ejs, cchar* src)
{
    EjsString   *dest;
    int         len;

    //  MOB -- potentially could just allocate the number of chars that there are chars in src. Can't require more.
    //  this would eliminate a double pass

    if ((len = toUni(NULL, src, -1)) < 0) {
        //MOB - rc or throw
        return ejs->emptyString;
    }
    if ((dest = (EjsString*) ejsAllocValue(ejs, ejs->stringType, (len + 1) * sizeof(EjsChar))) == NULL) {
        return NULL;
    }
    toUni(dest->value, src, len);
    return dest;
}


char *ejsToMulti(Ejs *ejs, EjsString *sp)
{
    char    *str;
    int     len;

    len = toMulti(NULL, sp->value, sp->length);
    if ((str = ejsAlloc(ejs, len + 1)) != NULL) {
        toMulti(str, sp->value, len);
        str[len] = '\0';
    }
    return str;
}


EjsString *ejsTrimString(Ejs *ejs, EjsString *sp, cchar *pat, int flags)
{
    EjsString   *result;
    EjsChar     *start, *end;
    cchar       *p;
    int         plen, trimmed;

    mprAssert(pat);

    start = sp->value;
    plen = strlen(pat);
    trimmed = 0;

    if (flags & EJS_TRIM_START) {
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

    if (flags & EJS_TRIM_END) {
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
    ejsInternString(ejs, result);
    return result;
}


EjsString *ejsTruncateString(Ejs *ejs, EjsString *sp, int len)
{
    EjsString   *result;

    mprAssert(sp);

    len = min(len, sp->length);
    result = ejsCreateBareString(ejs, len);
    memcpy(result->value, sp->value, len);
    ejsInternString(ejs, result);
    return result;
}


/*********************************** Factory **********************************/

//  MOB -- move to ejsAlloc
EjsString *ejsCreateStringWithLength(Ejs *ejs, cchar *value, int len)
{
    EjsHeap     *heap;
    EjsMem      *bp, *chain, *last;
    EjsString   *np, *sp;
    int         index, i;

    heap = ejs->heap;

    index = ejsComputeCStringHashCode(value, heap->strings.size);
    if ((chain = heap->strings.buckets[index]) != NULL) {
        for (; chain; chain = chain->next) {
            np = (EjsString*) EJS_GET_PTR(chain);
            for (i = 0; i < np->length && value[i]; i++) {
                if (np->value[i] != value[i]) {
                    break;
                }
            }
            if (i == np->length) {
                return np;
            }
            last = chain;
        }
    } else {
        last = NULL;
    }
#if UNUSED
    len = (int) strlen(value);
#endif
    if ((sp = (EjsString*) ejsAllocValue(ejs, ejs->stringType, (len + 1) * sizeof(EjsChar))) != NULL) {
        memcpy(sp->value, value, len * sizeof(EjsChar));
        sp->value[len] = 0;
    }
    sp->length = len;
    bp = EJS_GET_MEM(sp);
    unlinkBlock(bp);
    if (last) {
        last->next = bp;
    } else {
        heap->strings.buckets[index] = bp;
    }
    bp->next = NULL;
    if (++heap->strings.count > heap->strings.size) {
        /*  Remake the entire hash - should not happen often */
        remakeInternHash(heap);
    }
    mprAssert(bp);
    return sp;
}


EjsString *ejsCreateStringFromCS(Ejs *ejs, cchar *value)
{
    return ejsCreateStringWithLength(ejs, value, strlen(value));
}


EjsString *ejsCreateStringFromMulti(Ejs *ejs, char *str)
{
    EjsString       *result;

    result = ejsToUni(ejs, str);
    ejsInternString(ejs, result);
    return result;
}


EjsString *ejsCreateStringAndFree(Ejs *ejs, char *value)
{
    EjsString   *result;

    result = ejsCreateStringFromCS(ejs, value);
    mprFree(value);
    return result;
}



EjsString *ejsCreateBareString(Ejs *ejs, int len)
{
    EjsString   *sp;
    
    if ((sp = (EjsString*) ejsAllocValue(ejs, ejs->stringType, (len + 1) * sizeof(EjsChar))) != NULL) {
        sp->length = len;
        sp->value[0] = 0;
        sp->value[len] = 0;
    }
    return sp;
}


void ejsCreateStringType(Ejs *ejs)
{
    EjsType     *type;

    type = ejs->stringType = ejsCreateNativeType(ejs, "ejs", "String", ES_String, sizeof(EjsString));

    type->helpers.cast = (EjsCastHelper) castString;
    type->helpers.clone = (EjsCloneHelper) cloneString;
    type->helpers.getProperty = (EjsGetPropertyHelper) getStringProperty;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeStringOperator;
    type->helpers.lookupProperty = (EjsLookupPropertyHelper) lookupStringProperty;

    type->numericIndicies = 1;

    /*
        Standard (permanent) string values
     */
    ejs->emptyString = (EjsString*) ejsCreateStringFromCS(ejs, "");
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
    ejsBindMethod(ejs, prototype, ES_String_toJSON, (EjsProc) ejsStringToJSON);
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
