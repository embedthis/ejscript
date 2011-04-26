/**
    ejsRegExp.c - RegExp type class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"
#include    "pcre.h"

/***************************** Forward Declarations ***************************/

static char *makeFlags(EjsRegExp *rp);
static int parseFlags(EjsRegExp *rp, MprChar *flags);

/******************************************************************************/
/*
    Cast the operand to the specified type

    function cast(type: Type) : Object
 */
static EjsAny *castRegExp(Ejs *ejs, EjsRegExp *rp, EjsType *type)
{
    char    *flags;

    switch (type->sid) {
    case S_Boolean:
        return S(true);

    case S_String:
        flags = makeFlags(rp);
        return ejsSprintf(ejs, "/%w/%s", rp->pattern, flags);

    default:
        ejsThrowTypeError(ejs, "Can't cast to this type");
        return 0;
    }
    return 0;
}


/*********************************** Methods **********************************/
/*
    RegExp constructor

    RegExp(pattern: String, flags: String = null)
 */

static EjsObj *regex_Constructor(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    cchar       *errMsg;
    int         column, errCode;

    rp->pattern = wclone(ejsToString(ejs, argv[0])->value);
    rp->options = PCRE_JAVASCRIPT_COMPAT;

    if (argc == 2) {
        rp->options |= parseFlags(rp, ejsToString(ejs, argv[1])->value);
    }
    if (rp->compiled) {
        free(rp->compiled);
    }
    rp->compiled = (void*) pcre_compile2(rp->pattern, rp->options, &errCode, &errMsg, &column, NULL);
    if (rp->compiled == NULL) {
        ejsThrowArgError(ejs, "Can't compile regular expression. Error %s at column %d", errMsg, column);
        return 0;
    }
    return (EjsObj*) rp;
}


static EjsObj *regex_getLastIndex(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, rp->endLastMatch);
}


/*
    function set lastIndex(value: Number): Void
 */
static EjsObj *regex_setLastIndex(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    rp->endLastMatch = (int) ejsGetNumber(ejs, argv[0]);
    return 0;
}


/*
    function exec(str: String, start: Number = 0): Array
 */
static EjsObj *regex_exec(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    EjsArray    *results;
    EjsString   *match, *str;
    int         matches[EJS_MAX_REGEX_MATCHES * 3];
    int         count, start, len, i, index;

    str = (EjsString*) argv[0];
    if (argc == 2) {
        start = (int) ejsGetNumber(ejs, argv[1]);
    } else {
        start = rp->endLastMatch;
    }
    rp->matched = 0;
    mprAssert(rp->compiled);
    count = pcre_exec(rp->compiled, NULL, str->value, (int) str->length, start, 0, matches, sizeof(matches) / sizeof(int));
    if (count < 0) {
        rp->endLastMatch = 0;
        return (EjsObj*) S(null);
    }
    results = ejsCreateArray(ejs, count);
    for (index = 0, i = 0; i < count; i++, index += 2) {
        len = matches[index + 1] - matches[index];
        match = ejsCreateString(ejs, &str->value[matches[index]], len);
        ejsSetProperty(ejs, results, i, match);
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


static EjsObj *regex_getGlobalFlag(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, rp->global);
}


static EjsObj *regex_getIgnoreCase(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, rp->ignoreCase);
}


static EjsObj *regex_getMultiline(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, rp->multiline);
}


static EjsObj *regex_getSource(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, rp->pattern, wlen(rp->pattern));
}


static EjsObj *regex_matched(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    if (rp->matched == 0) {
        return (EjsObj*) S(null);
    }
    return (EjsObj*) rp->matched;
}


static EjsObj *regex_start(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, rp->startLastMatch);
}


static EjsObj *regex_sticky(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, rp->sticky);
}


static EjsObj *regex_test(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    EjsString   *str;
    int         count;

    str = (EjsString*) argv[0];
    mprAssert(rp->compiled);
    count = pcre_exec(rp->compiled, NULL, str->value, (int) str->length, rp->endLastMatch, 0, 0, 0);
    if (count < 0) {
        rp->endLastMatch = 0;
        return (EjsObj*) S(false);
    }
    return (EjsObj*) S(true);
}


EjsString *ejsRegExpToString(Ejs *ejs, EjsRegExp *rp)
{
    return (EjsString*) castRegExp(ejs, rp, ST(String));
}

/*********************************** Factory **********************************/
/*
    Create an initialized regular expression object. The pattern should include
    the slash delimiters. For example: /abc/ or /abc/g
 */
EjsRegExp *ejsCreateRegExp(Ejs *ejs, EjsString *pattern)
{
    EjsRegExp   *rp;
    cchar       *errMsg;
    MprChar     *flags;
    int         column, errCode;

    if (pattern->length == 0 || pattern->value[0] != '/') {
        ejsThrowArgError(ejs, "Bad regular expression pattern. Must start with '/'");
        return 0;
    }
    rp = ejsCreateObj(ejs, ST(RegExp), 0);
    if (rp != 0) {
        /*
            Strip off flags for passing to pcre_compile2
         */
        rp->pattern = sclone(&pattern->value[1]);
        if ((flags = wrchr(rp->pattern, '/')) != 0) {
            rp->options = parseFlags(rp, &flags[1]);
            *flags = 0;
        }
        //  TODO - UNICODE is pattern meant to be 
        rp->compiled = pcre_compile2(rp->pattern, rp->options, &errCode, &errMsg, &column, NULL);
        if (rp->compiled == NULL) {
            ejsThrowArgError(ejs, "Can't compile regular expression. Error %s at column %d", errMsg, column);
            return 0;
        }
    }
    return rp;
}


static int parseFlags(EjsRegExp *rp, MprChar *flags)
{
    MprChar     *cp;
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
    return sclone(buf);
}


static void manageRegExp(EjsRegExp *rp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(rp->pattern);

    } else if (flags & MPR_MANAGE_FREE) {
        if (rp->compiled) {
            free(rp->compiled);
            rp->compiled = 0;
        }
    }
}


void ejsCreateRegExpType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsCreateNativeType(ejs, N("ejs", "RegExp"), sizeof(EjsRegExp), S_RegExp, ES_RegExp_NUM_CLASS_PROP,
        manageRegExp, EJS_OBJ_HELPERS);
    type->helpers.cast = (EjsCastHelper) castRegExp;
}


void ejsConfigureRegExpType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    type = ST(RegExp);
    prototype = type->prototype;

    ejsBindConstructor(ejs, type, (EjsProc) regex_Constructor);
    ejsBindMethod(ejs, prototype, ES_RegExp_exec, (EjsProc) regex_exec);
    ejsBindAccess(ejs, prototype, ES_RegExp_lastIndex, (EjsProc) regex_getLastIndex, (EjsProc) regex_setLastIndex);
    ejsBindMethod(ejs, prototype, ES_RegExp_global, (EjsProc) regex_getGlobalFlag);
    ejsBindMethod(ejs, prototype, ES_RegExp_ignoreCase, (EjsProc) regex_getIgnoreCase);
    ejsBindMethod(ejs, prototype, ES_RegExp_multiline, (EjsProc) regex_getMultiline);
    ejsBindMethod(ejs, prototype, ES_RegExp_source, (EjsProc) regex_getSource);
    ejsBindMethod(ejs, prototype, ES_RegExp_matched, (EjsProc) regex_matched);
    ejsBindMethod(ejs, prototype, ES_RegExp_start, (EjsProc) regex_start);
    ejsBindMethod(ejs, prototype, ES_RegExp_sticky, (EjsProc) regex_sticky);
    ejsBindMethod(ejs, prototype, ES_RegExp_test, (EjsProc) regex_test);
    ejsBindMethod(ejs, prototype, ES_RegExp_toString, (EjsProc) ejsRegExpToString);
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
