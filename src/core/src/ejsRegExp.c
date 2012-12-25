/**
    ejsRegExp.c - RegExp type class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"
#include    "pcre.h"

/********************************** Defines ***********************************/

#if !defined(BIT_MAX_REGEX_MATCHES)
    #define BIT_MAX_REGEX_MATCHES 64
#endif

/********************************* Forwards ***********************************/

static char *makeFlags(EjsRegExp *rp);
static int parseFlags(EjsRegExp *rp, wchar *flags);

/******************************************************************************/
/*
    Cast the operand to the specified type

    function cast(type: Type) : Object
 */
static EjsAny *castRegExp(Ejs *ejs, EjsRegExp *rp, EjsType *type)
{
    wchar   *pattern;
    char    *flags;
    ssize   len;
    int     i, j;

    switch (type->sid) {
    case S_Boolean:
        return ESV(true);

    case S_String:
        flags = makeFlags(rp);
        len = wlen(rp->pattern);
        pattern = mprAlloc((len * 2 + 1) * sizeof(wchar));
        for (i = j = 0; i < len; i++) {
            if (rp->pattern[i] == '/') {
                pattern[j++] = '\\';
            }
            pattern[j++] = rp->pattern[i];
        }
        pattern[j] = 0;
        return ejsSprintf(ejs, "/%w/%s", pattern, flags);

    default:
        ejsThrowTypeError(ejs, "Cannot cast to this type");
        return 0;
    }
    return 0;
}


/*********************************** Methods **********************************/
/*
    RegExp constructor

    RegExp(pattern: String, flags: String = null)
 */

static EjsRegExp *regex_Constructor(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
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
    if ((rp->compiled = pcre_compile2(rp->pattern, rp->options, &errCode, &errMsg, &column, NULL)) == 0) {
        ejsThrowArgError(ejs, "Cannot compile regular expression '%s'. Error %s at column %d", rp->pattern, errMsg, column);
        return 0;
    }
    return rp;
}


static EjsNumber *regex_getLastIndex(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, rp->endLastMatch);
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
static EjsArray *regex_exec(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    EjsArray    *results;
    EjsString   *match, *str;
    int         matches[BIT_MAX_REGEX_MATCHES * 3];
    int         count, start, len, i, index;

    str = (EjsString*) argv[0];
    if (argc == 2) {
        start = (int) ejsGetNumber(ejs, argv[1]);
    } else {
        start = rp->endLastMatch;
    }
    rp->matched = 0;
    assure(rp->compiled);
    count = pcre_exec(rp->compiled, NULL, str->value, (int) str->length, start, 0, matches, sizeof(matches) / sizeof(int));
    if (count < 0) {
        rp->endLastMatch = 0;
        return ESV(null);
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
    return results;
}


static EjsBoolean *regex_getGlobalFlag(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, rp->global);
}


static EjsBoolean *regex_getIgnoreCase(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, rp->ignoreCase);
}


static EjsBoolean *regex_getMultiline(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, rp->multiline);
}


static EjsString *regex_getSource(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return ejsCreateString(ejs, rp->pattern, wlen(rp->pattern));
}


static EjsString *regex_matched(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    if (rp->matched == 0) {
        return ESV(null);
    }
    return rp->matched;
}


static EjsNumber *regex_start(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, rp->startLastMatch);
}


static EjsBoolean *regex_sticky(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, rp->sticky);
}


static EjsBoolean *regex_test(Ejs *ejs, EjsRegExp *rp, int argc, EjsObj **argv)
{
    EjsString   *str;
    int         count;

    str = (EjsString*) argv[0];
    assure(rp->compiled);
    count = pcre_exec(rp->compiled, NULL, str->value, (int) str->length, rp->endLastMatch, 0, 0, 0);
    if (count < 0) {
        rp->endLastMatch = 0;
        return ESV(false);
    }
    return ESV(true);
}


PUBLIC EjsString *ejsRegExpToString(Ejs *ejs, EjsRegExp *rp)
{
    return (EjsString*) castRegExp(ejs, rp, ESV(String));
}

/*********************************** Factory **********************************/
/*
    Create an initialized regular expression object. The pattern should include
    the slash delimiters. For example: /abc/ or /abc/g
 */
PUBLIC EjsRegExp *ejsCreateRegExp(Ejs *ejs, EjsString *pattern)
{
    EjsRegExp   *rp;
    cchar       *errMsg;
    char        *cp, *dp;
    wchar       *flags;
    int         column, errCode;

    if (pattern->length == 0 || pattern->value[0] != '/') {
        ejsThrowArgError(ejs, "Bad regular expression pattern. Must start with '/'");
        return 0;
    }
    if ((rp = ejsCreateObj(ejs, ESV(RegExp), 0)) == 0) {
        return 0;
    }
    /*
        Strip off flags for passing to pcre_compile2
     */
    rp->pattern = sclone(&pattern->value[1]);
    if ((flags = wrchr(rp->pattern, '/')) != 0) {
        if (flags == rp->pattern) {
            ejsThrowArgError(ejs, "Bad regular expression pattern. Must end with '/'");
            return 0;
        }
        rp->options = parseFlags(rp, &flags[1]);
        *flags = 0;
    }
    for (dp = cp = rp->pattern; *cp; ) {
        if (*cp == '\\' && cp[1] == '/') {
            cp++;
        }
        *dp++ = *cp++;
    }
    *dp++ = *cp++;
    //  TODO - UNICODE is pattern meant to be
    rp->compiled = pcre_compile2(rp->pattern, rp->options, &errCode, &errMsg, &column, NULL);
    if (rp->compiled == NULL) {
        ejsThrowArgError(ejs, "Cannot compile regular expression '%s'. Error %s at column %d", rp->pattern, errMsg, column);
        return 0;
    }
    return rp;
}


static int parseFlags(EjsRegExp *rp, wchar *flags)
{
    wchar       *cp;
    int         options;

    if (flags == 0 || *flags == '\0') {
        return 0;
    }
    options = PCRE_JAVASCRIPT_COMPAT;
    for (cp = flags; *cp; cp++) {
        switch (tolower((uchar) *cp)) {
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


PUBLIC void ejsCreateRegExpType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsCreateCoreType(ejs, N("ejs", "RegExp"), sizeof(EjsRegExp), S_RegExp, ES_RegExp_NUM_CLASS_PROP,
        manageRegExp, EJS_TYPE_OBJ | EJS_TYPE_MUTABLE_INSTANCES);
    type->helpers.cast = (EjsCastHelper) castRegExp;
}


PUBLIC void ejsConfigureRegExpType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "RegExp"))) == 0) {
        return;
    }
    prototype = type->prototype;
    ejsBindConstructor(ejs, type, regex_Constructor);
    ejsBindMethod(ejs, prototype, ES_RegExp_exec, regex_exec);
    ejsBindAccess(ejs, prototype, ES_RegExp_lastIndex, regex_getLastIndex, regex_setLastIndex);
    ejsBindMethod(ejs, prototype, ES_RegExp_global, regex_getGlobalFlag);
    ejsBindMethod(ejs, prototype, ES_RegExp_ignoreCase, regex_getIgnoreCase);
    ejsBindMethod(ejs, prototype, ES_RegExp_multiline, regex_getMultiline);
    ejsBindMethod(ejs, prototype, ES_RegExp_source, regex_getSource);
    ejsBindMethod(ejs, prototype, ES_RegExp_matched, regex_matched);
    ejsBindMethod(ejs, prototype, ES_RegExp_start, regex_start);
    ejsBindMethod(ejs, prototype, ES_RegExp_sticky, regex_sticky);
    ejsBindMethod(ejs, prototype, ES_RegExp_test, regex_test);
    ejsBindMethod(ejs, prototype, ES_RegExp_toString, ejsRegExpToString);
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.

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
