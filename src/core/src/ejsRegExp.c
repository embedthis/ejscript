/**
    ejsRegExp.c - RegExp type class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

#include    "pcre.h"

/***************************** Forward Declarations ***************************/

static char *makeFlags(EjsRegExp *rp);
static int parseFlags(EjsRegExp *rp, cchar *flags);

/******************************************************************************/
/*
    Cast the operand to the specified type

    function cast(type: Type) : Object
 */
static EjsObj *castRegExp(Ejs *ejs, EjsRegExp *rp, EjsType *type)
{
    char    *pattern, *flags;

    switch (type->id) {
    case ES_Boolean:
        return (EjsObj*) ejs->trueValue;

    case ES_String:
        flags = makeFlags(rp);
        pattern = mprStrcat(rp, -1, "/", rp->pattern, "/", flags, NULL);
        mprFree(flags);
        return (EjsVar*) ejsCreateStringAndFree(ejs, pattern);

    default:
        ejsThrowTypeError(ejs, "Can't cast to this type");
        return 0;
    }
    return 0;
}


static void destroyRegExp(Ejs *ejs, EjsRegExp *rp)
{
    mprAssert(rp);

    if (rp->compiled) {
        free(rp->compiled);
        rp->compiled = 0;
    }
    ejsFreeVar(ejs, (EjsObj*) rp, -1);
}


/*********************************** Methods **********************************/
/*
    RegExp constructor

    RegExp(pattern: String, flags: String = null)
 */

static EjsObj *regexConstructor(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    cchar       *errMsg, *pattern, *flags;
    int         column, errCode;

    pattern = ejsGetString(ejs, argv[0]);
    rp->options = PCRE_JAVASCRIPT_COMPAT;

    if (argc == 2) {
        flags = (char*) ejsGetString(ejs, argv[1]);
        rp->options |= parseFlags(rp, flags);
    }
    rp->pattern = mprStrdup(rp, pattern);
    if (rp->compiled) {
        free(rp->compiled);
    }
    rp->compiled = (void*) pcre_compile2(pattern, rp->options, &errCode, &errMsg, &column, NULL);
    if (rp->compiled == NULL) {
        ejsThrowArgError(ejs, "Can't compile regular expression. Error %s at column %d", errMsg, column);
    }
    return (EjsObj*) rp;
}


static EjsObj *getLastIndex(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, rp->endLastMatch);
}


/*
    function set lastIndex(value: Number): Void
 */
static EjsObj *setLastIndex(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    rp->endLastMatch = (int) ejsGetNumber(ejs, argv[0]);
    return 0;
}


/*
    function exec(str: String, start: Number = 0): Array
 */
static EjsObj *exec(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    EjsArray    *results;
    EjsString   *match;
    cchar       *str;
    int         matches[EJS_MAX_REGEX_MATCHES * 3];
    int         start, len, i, count, index;

    str = ejsGetString(ejs, argv[0]);
    if (argc == 2) {
        start = (int) ejsGetNumber(ejs, argv[1]);
    } else {
        start = rp->endLastMatch;
    }
    rp->matched = 0;
    mprAssert(rp->compiled);
    count = pcre_exec(rp->compiled, NULL, str, (int) strlen(str), start, 0, matches, sizeof(matches) / sizeof(int));
    if (count < 0) {
        rp->endLastMatch = 0;
        return (EjsObj*) ejs->nullValue;
    }
    results = ejsCreateArray(ejs, count);
    for (index = 0, i = 0; i < count; i++, index += 2) {
        len = matches[index + 1] - matches[index];
        match = ejsCreateStringWithLength(ejs, &str[matches[index]], len);
        ejsSetProperty(ejs, (EjsObj*) results, i, (EjsObj*) match);
        if (index == 0) {
            rp->matched = match;
        }
    }
    if (rp->global) {
        /* Only save if global flag used as per spec */
        rp->startLastMatch = matches[0];
        rp->endLastMatch = matches[1];
    }
    return (EjsObj*) results;
}


static EjsObj *getGlobalFlag(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, rp->global);
}


static EjsObj *getIgnoreCase(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, rp->ignoreCase);
}


static EjsObj *getMultiline(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, rp->multiline);
}


static EjsObj *getSource(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, rp->pattern);
}


static EjsObj *matched(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    if (rp->matched == 0) {
        return (EjsObj*) ejs->nullValue;
    }
    return (EjsObj*) rp->matched;
}


static EjsObj *start(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, rp->startLastMatch);
}


static EjsObj *sticky(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, rp->sticky);
}


static EjsObj *test(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    cchar       *str;
    int         count;

    str = ejsGetString(ejs, argv[0]);
    mprAssert(rp->compiled);
    count = pcre_exec(rp->compiled, NULL, str, (int) strlen(str), rp->endLastMatch, 0, 0, 0);
    if (count < 0) {
        rp->endLastMatch = 0;
        return (EjsObj*) ejs->falseValue;
    }
    return (EjsObj*) ejs->trueValue;
}


EjsString *ejsRegExpToString(Ejs *ejs, EjsRegExp *rp)
{
    return (EjsString*) castRegExp(ejs, rp, ejs->stringType);
}

/*********************************** Factory **********************************/
/*
    Create an initialized regular expression object. The pattern should include
    the slash delimiters. For example: /abc/ or /abc/g
 */

EjsRegExp *ejsCreateRegExp(Ejs *ejs, cchar *pattern)
{
    EjsRegExp   *rp;
    cchar       *errMsg;
    char        *flags;
    int         column, errCode;

    mprAssert(pattern[0] == '/');
    if (*pattern != '/') {
        ejsThrowArgError(ejs, "Bad regular expression pattern. Must start with '/'");
        return 0;
    }

    rp = (EjsRegExp*) ejsCreate(ejs, ejs->regExpType, 0);
    if (rp != 0) {
        /*
            Strip off flags for passing to pcre_compile2
         */
        rp->pattern = mprStrdup(rp, &pattern[1]);
        if ((flags = strrchr(rp->pattern, '/')) != 0) {
            rp->options = parseFlags(rp, &flags[1]);
            *flags = '\0';
        }
        if (rp->compiled) {
            free(rp->compiled);
        }
        rp->compiled = pcre_compile2(rp->pattern, rp->options, &errCode, &errMsg, &column, NULL);
        if (rp->compiled == NULL) {
            ejsThrowArgError(ejs, "Can't compile regular expression. Error %s at column %d", errMsg, column);
            return 0;
        }
    }
    return rp;
}


static int parseFlags(EjsRegExp *rp, cchar *flags)
{
    cchar       *cp;
    int         options;

    if (flags == 0 || *flags == '\0') {
        return 0;
    }
    options = PCRE_JAVASCRIPT_COMPAT;
    for (cp = flags; *cp; cp++) {
        switch (tolower((int) *cp)) {
        case 'g':
            rp->global = 1;
            break;
        case 'i':
            rp->ignoreCase = 1;
            options |= PCRE_CASELESS;
            break;
        case 'm':
            rp->multiline = 1;
            options |= PCRE_MULTILINE;
            break;
        case 's':
            options |= PCRE_DOTALL;
            break;
        case 'y':
            rp->sticky = 1;
            break;
        case 'x':
            options |= PCRE_EXTENDED;
            break;
        case 'X':
            options |= PCRE_EXTRA;
            break;
        case 'U':
            options |= PCRE_UNGREEDY;
            break;
        }
    }
    return options;
}


static char *makeFlags(EjsRegExp *rp)
{
    char    buf[16], *cp;

    cp = buf;
    if (rp->global) {
        *cp++ = 'g';
    }
    if (rp->ignoreCase) {
        *cp++ = 'i';
    }
    if (rp->multiline) {
        *cp++ = 'm';
    }
    if (rp->sticky) {
        *cp++ = 'y';
    }
    if (rp->options & PCRE_DOTALL) {
        *cp++ = 's';
    }
    if (rp->options & PCRE_EXTENDED) {
        *cp++ = 'x';
    }
    if (rp->options & PCRE_EXTRA) {
        *cp++ = 'X';
    }
    if (rp->options & PCRE_UNGREEDY) {
        *cp++ = 'U';
    }
    *cp++ = '\0';
    return mprStrdup(rp, buf);
}


void ejsCreateRegExpType(Ejs *ejs)
{
    EjsType     *type;

    type = ejs->regExpType = ejsCreateNativeType(ejs, "ejs", "RegExp", ES_RegExp, sizeof(EjsRegExp));
    type->needFinalize = 1;
    type->helpers.cast = (EjsCastHelper) castRegExp;
    type->helpers.destroy = (EjsDestroyHelper) destroyRegExp;
}


void ejsConfigureRegExpType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *prototype;

    type = ejsConfigureNativeType(ejs, "ejs", "RegExp", sizeof(EjsRegExp));
    prototype = type->prototype;

    ejsBindConstructor(ejs, type, (EjsProc) regexConstructor);
    ejsBindMethod(ejs, prototype, ES_RegExp_exec, (EjsProc) exec);
    ejsBindAccess(ejs, prototype, ES_RegExp_lastIndex, (EjsProc) getLastIndex, (EjsProc) setLastIndex);
    ejsBindMethod(ejs, prototype, ES_RegExp_global, (EjsProc) getGlobalFlag);
    ejsBindMethod(ejs, prototype, ES_RegExp_ignoreCase, (EjsProc) getIgnoreCase);
    ejsBindMethod(ejs, prototype, ES_RegExp_multiline, (EjsProc) getMultiline);
    ejsBindMethod(ejs, prototype, ES_RegExp_source, (EjsProc) getSource);
    ejsBindMethod(ejs, prototype, ES_RegExp_matched, (EjsProc) matched);
    ejsBindMethod(ejs, prototype, ES_RegExp_start, (EjsProc) start);
    ejsBindMethod(ejs, prototype, ES_RegExp_sticky, (EjsProc) sticky);
    ejsBindMethod(ejs, prototype, ES_RegExp_test, (EjsProc) test);
    ejsBindMethod(ejs, prototype, ES_RegExp_toString, (EjsProc) ejsRegExpToString);
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
