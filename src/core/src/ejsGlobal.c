/**
    ejsGlobal.c - Global functions and variables

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Locals ***********************************/
/*  
    Assert a condition is true.
    static function assert(condition: Boolean): Boolean
 */
static EjsBoolean *g_assert(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    EjsFrame        *fp;
    MprChar         *source;
    EjsBoolean      *b;

    mprAssert(argc == 1);

    if (!ejsIs(ejs, argv[0], Boolean)) {
        b = (EjsBoolean*) ejsCast(ejs, argv[0], Boolean);
    } else {
        b = (EjsBoolean*) argv[0];
    }
    mprAssert(b);

    if (b == 0 || !b->value) {
        fp = ejs->state->fp;
        if (ejsGetDebugInfo(ejs, (EjsFunction*) fp, fp->pc, NULL, NULL, &source) >= 0) {
            ejsThrowAssertError(ejs, "%w", source);
        } else {
            ejsThrowAssertError(ejs, "Assertion error");
        }
        return 0;
    }
    return ESV(true);
}


/*  
    function blend(dest: Object, src: Object, options = null): Object
 */
static EjsObj *g_blend(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *src, *dest, *options;
    int         flags;

    options = (argc >= 3) ? argv[2] : 0;
    if (options) {
        flags = 0;
        flags |= ejsGetPropertyByName(ejs, options, EN("functions")) == ESV(true) ? EJS_BLEND_FUNCTIONS : 0;
        flags |= ejsGetPropertyByName(ejs, options, EN("trace")) == ESV(true) ? EJS_BLEND_TRACE : 0;

        flags |= ejsGetPropertyByName(ejs, options, EN("overwrite")) == ESV(false) ? 0 : EJS_BLEND_OVERWRITE;
        flags |= ejsGetPropertyByName(ejs, options, EN("subclass")) == ESV(false) ? 0 : EJS_BLEND_SUBCLASSES;
        flags |= ejsGetPropertyByName(ejs, options, EN("deep")) == ESV(false) ? 0 : EJS_BLEND_DEEP;
    } else {
        flags = EJS_BLEND_DEEP | EJS_BLEND_OVERWRITE | EJS_BLEND_SUBCLASSES;
    }
    dest = argv[0];
    src = argv[1];
    ejsBlendObject(ejs, dest, src, flags);
    return dest;
}


/*  
    Clone the base class. Used by Record.es
    static function cloneBase(klass: Type): Void
 */
static EjsObj *g_cloneBase(Ejs *ejs, EjsObj *ignored, int argc, EjsObj **argv)
{
    EjsType     *type;
    
    mprAssert(argc == 1);
    
    type = (EjsType*) argv[0];
    type->baseType = ejsClone(ejs, type->baseType, 0);
    return 0;
}


/*  
    function eval(script: String, cache: String = null): String
 */
static EjsObj *g_eval(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsString   *script;
    cchar       *cache;

    script = (EjsString*) argv[0];
    if (argc < 2 || ejsIs(ejs, argv[1], Null)) {
        cache = NULL;
    } else {
        cache = ejsToMulti(ejs, argv[1]);
    }
    MPR_VERIFY_MEM();
    if (ejsLoadScriptLiteral(ejs, script, cache, EC_FLAGS_NO_OUT | EC_FLAGS_DEBUG | EC_FLAGS_THROW | EC_FLAGS_VISIBLE) < 0) {
        return 0;
    }
    MPR_VERIFY_MEM();
    return ejs->result;
}


#if ES_hashcode
/*  
    Get the hash code for the object.
    function hashcode(o: Object): Number
 */
static EjsNumber *g_hashcode(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    mprAssert(argc == 1);
    return ejsCreateNumber(ejs, (MprNumber) PTOL(argv[0]));
}
#endif


/*  
    Load a script or module. Name should have an extension. Name will be located according to the EJSPATH search strategy.

    static function load(filename: String, options: Object): void

    options = { cache: String|Path, reload: true }
 */
static EjsObj *g_load(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *options, *vp;
    cchar       *path, *cache, *cp;
    int         reload;

    cache = 0;
    reload = 1;
    path = ejsToMulti(ejs, argv[0]);
    options = (argc >= 2) ? argv[1] : 0;

    if (options) {
        if ((vp = ejsGetPropertyByName(ejs, options, EN("cache"))) != 0) {
            cache = ejsToMulti(ejs, ejsToString(ejs, vp));
        }
        reload = ejsGetPropertyByName(ejs, options, EN("reload")) == ESV(true);
    }
    if ((cp = strrchr(path, '.')) != NULL && strcmp(cp, EJS_MODULE_EXT) != 0) {
        if (ejs->service->loadScriptFile == 0) {
            ejsThrowIOError(ejs, "load: Compiling is not enabled for %s", path);
        } else {
            return (ejs->service->loadScriptFile)(ejs, path, cache);
        }
    } else {
        ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, path), -1, -1, (reload) ? EJS_LOADER_RELOAD : 0);
        return (ejs->exception) ? 0 : ejs->result;
    }
    return 0;
}


/*  
    Compute an MD5 checksum
    static function md5(name: String): String
 */
static EjsString *g_md5(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsString   *str;
    char        *hash;

    MPR_VERIFY_MEM();
    str = (EjsString*) argv[0];
    hash = mprGetMD5WithPrefix(ejsToMulti(ejs, str), str->length, NULL);
    return ejsCreateStringFromAsc(ejs, hash);
}


/*  
    Merge one object into another. This is useful for inheriting and optionally overwriting option hashes (among other
    things). The blending is done at the primitive property level. If overwrite is true, the property is replaced. If
    overwrite is false, the property will be added if it does not already exist
 */
int ejsBlendObject(Ejs *ejs, EjsObj *dest, EjsObj *src, int flags)
{
    EjsTrait    *trait;
    EjsObj      *vp, *dp;
    EjsName     name;
    int         i, count, start, deep, functions, overwrite, privateProps, trace;

    count = ejsGetLength(ejs, src);
    start = (flags & EJS_BLEND_SUBCLASSES) ? 0 : TYPE(src)->numInherited;
    deep = (flags & EJS_BLEND_DEEP) ? 1 : 0;
    overwrite = (flags & EJS_BLEND_OVERWRITE) ? 1 : 0;
    functions = (flags & EJS_BLEND_FUNCTIONS) ? 1 : 0;
    privateProps = (flags & EJS_BLEND_PRIVATE) ? 1 : 0;
    trace = (flags & EJS_BLEND_TRACE) ? 1 : 0;

    for (i = start; i < count; i++) {
        if ((trait = ejsGetPropertyTraits(ejs, src, i)) != 0) {
            if (trait->attributes & (EJS_TRAIT_DELETED | EJS_FUN_INITIALIZER | EJS_FUN_MODULE_INITIALIZER)) {
                continue;
            }
        }
        if ((vp = ejsGetProperty(ejs, src, i)) == 0) {
            continue;
        }
        if (!functions && ejsIsFunction(ejs, ejsGetProperty(ejs, src, i))) {
            continue;
        }
        name = ejsGetPropertyName(ejs, src, i);
        if (!privateProps && ejsContainsAsc(ejs, name.space, ",private") >= 0) {
            continue;
        }
        if (trace) {
            mprLog(0, "NAME %N", name);
        }
        /* NOTE: treats arrays as primitive types */
        if (deep && !ejsIs(ejs, vp, Array) && !ejsIsXML(ejs, vp) && ejsGetLength(ejs, vp) > 0) {
            if ((dp = ejsGetPropertyByName(ejs, dest, name)) == 0 || ejsGetLength(ejs, dp) == 0) {
                ejsSetPropertyByName(ejs, dest, name, ejsClonePot(ejs, vp, deep));
            } else {
                ejsBlendObject(ejs, dp, vp, flags);
            }
        } else {
            /* Primitive type (including arrays) */
            if (overwrite) {
                ejsSetPropertyByName(ejs, dest, name, vp);
            } else if (ejsLookupProperty(ejs, dest, name) < 0) {
                ejsSetPropertyByName(ejs, dest, name, vp);
            }
        }
    }
    return 0;
}


/*     
    Parse the input and convert to a primitive type
    static function parse(input: String, preferredType: Type = null): void
 */
static EjsObj *g_parse(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsString   *input;
    int         preferred;

    input = (EjsString*) argv[0];
    if (argc == 2 && !ejsIsType(ejs, argv[1])) {
        ejsThrowArgError(ejs, "PreferredType argument is not a type");
        return 0;
    }
    preferred = (argc == 2) ? ((EjsType*) argv[1])->sid : -1;
    return ejsParse(ejs, input->value, preferred);
}


/*
    Parse the input as an integer
    static function parseInt(input: String, radix: Number = 10): Number
    Formats:
        [(+|-)][0][OCTAL_DIGITS]
        [(+|-)][0][(x|X)][HEX_DIGITS]
        [(+|-)][DIGITS]
 */
static EjsNumber *g_parseInt(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprNumber   n;
    cchar       *str;
    int         radix, err;

    str = ejsToMulti(ejs, argv[0]);
    radix = (argc >= 2) ? ejsGetInt(ejs, argv[1]) : 0;
    while (isspace((int) *str)) {
        str++;
    }
    if (*str == '-' || *str == '+' || isdigit((int) *str)) {
        n = (MprNumber) stoiradix(str, radix, &err);
        if (err) {
            return ESV(nan);
        }
        return ejsCreateNumber(ejs, n);
    }
    return ESV(nan);
}


/*  
    Print the arguments to the standard output with a new line.
    static function print(...args): void
    DEPRECATED static function output(...args): void
 */
static EjsObj *g_printLine(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsString   *s;
    EjsObj      *args, *vp;
    cchar       *data;
    int         i, count;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Array));

    args = argv[0];
    count = ejsGetLength(ejs, args);

    for (i = 0; i < count; i++) {
        if ((vp = ejsGetProperty(ejs, args, i)) != 0) {
            s  = (ejsIs(ejs, vp, String)) ? (EjsString*) vp : (EjsString*) ejsToString(ejs, vp);
            if (ejs->exception) {
                return 0;
            }
            data = ejsToMulti(ejs, s);
            if (write(1, (char*) data, (int) strlen(data)) < 0) {}
            if ((i+1) < count) {
                if (write(1, " ", 1) < 0) {}
            }
        }
    }
    if (write(1, "\n", 1) < 0) {}
    return 0;
}


void ejsFreezeGlobal(Ejs *ejs)
{
    EjsTrait    *trait;
    int         i;

    for (i = 0; i < ES_global_NUM_CLASS_PROP; i++) {
        if ((trait = ejsGetPropertyTraits(ejs, ejs->global, i)) != 0) {
            ejsSetPropertyTraits(ejs, ejs->global, i, NULL, trait->attributes | EJS_TRAIT_READONLY | EJS_TRAIT_FIXED);
        }
    }
}


void ejsCreateGlobalNamespaces(Ejs *ejs)
{
    ejsAddImmutable(ejs, S_iteratorSpace, EN("iterator"), 
        ejsCreateNamespace(ejs, ejsCreateStringFromAsc(ejs, EJS_ITERATOR_NAMESPACE)));
    ejsAddImmutable(ejs, S_publicSpace, EN("public"), 
        ejsCreateNamespace(ejs, ejsCreateStringFromAsc(ejs, EJS_PUBLIC_NAMESPACE)));
    ejsAddImmutable(ejs, S_ejsSpace, EN("ejs"), 
        ejsCreateNamespace(ejs, ejsCreateStringFromAsc(ejs, EJS_EJS_NAMESPACE)));
    ejsAddImmutable(ejs, S_emptySpace, EN("empty"), 
        ejsCreateNamespace(ejs, ejsCreateStringFromAsc(ejs, EJS_EMPTY_NAMESPACE)));
}


void ejsDefineGlobalNamespaces(Ejs *ejs)
{
    /*  
        Order matters here. This is the (reverse) order of lookup.
        Empty is first to maximize speed of searching dynamic properties. Ejs second to maximize builtin lookups.
     */
    ejsAddNamespaceToBlock(ejs, ejs->global, ESV(iteratorSpace));
    ejsAddNamespaceToBlock(ejs, ejs->global, ESV(publicSpace));
    ejsAddNamespaceToBlock(ejs, ejs->global, ESV(ejsSpace));
    ejsAddNamespaceToBlock(ejs, ejs->global, ESV(emptySpace));
}


void ejsConfigureGlobalBlock(Ejs *ejs)
{
    EjsBlock    *block;

    block = (EjsBlock*) ejs->global;
    mprAssert(block);
    
    ejsSetProperty(ejs, ejs->global, ES_global, ejs->global);
    ejsSetProperty(ejs, ejs->global, ES_void, ESV(Void));
    ejsSetProperty(ejs, ejs->global, ES_undefined, ESV(undefined));
    ejsSetProperty(ejs, ejs->global, ES_null, ESV(null));
    ejsSetProperty(ejs, ejs->global, ES_global, ejs->global);
    ejsSetProperty(ejs, ejs->global, ES_NegativeInfinity, ESV(negativeInfinity));
    ejsSetProperty(ejs, ejs->global, ES_Infinity, ESV(infinity));
    ejsSetProperty(ejs, ejs->global, ES_NaN, ESV(nan));
    ejsSetProperty(ejs, ejs->global, ES_double, ESV(Number));
    ejsSetProperty(ejs, ejs->global, ES_num, ESV(Number));
    ejsSetProperty(ejs, ejs->global, ES_boolean, ESV(Boolean));
    ejsSetProperty(ejs, ejs->global, ES_string, ESV(String));
    ejsSetProperty(ejs, ejs->global, ES_true, ESV(true));
    ejsSetProperty(ejs, ejs->global, ES_false, ESV(false));

    ejsBindFunction(ejs, block, ES_assert, g_assert);
    ejsBindFunction(ejs, block, ES_cloneBase, g_cloneBase);
    ejsBindFunction(ejs, block, ES_eval, g_eval);
    ejsBindFunction(ejs, block, ES_hashcode, g_hashcode);
    ejsBindFunction(ejs, block, ES_load, g_load);
    ejsBindFunction(ejs, block, ES_md5, g_md5);
    ejsBindFunction(ejs, block, ES_blend, g_blend);
    ejsBindFunction(ejs, block, ES_parse, g_parse);
    ejsBindFunction(ejs, block, ES_parseInt, g_parseInt);
    ejsBindFunction(ejs, block, ES_print, g_printLine);
}


/*
    @copy   default
 
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.

    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
