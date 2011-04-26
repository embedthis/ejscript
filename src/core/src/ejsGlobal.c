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
static EjsObj *g_assert(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    EjsFrame        *fp;
    MprChar         *source;
    EjsBoolean      *b;

    mprAssert(argc == 1);

    if (! ejsIs(ejs, argv[0], Boolean)) {
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
    return vp;
}


/*  
    function blend(dest: Object, src: Object, overwrite: Boolean = true): void
 */
static EjsObj *g_blend(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *src, *dest;
    int         overwrite;

    overwrite = (argc == 3) ? (argv[2] == (EjsObj*) S(true)) : 1;
    dest = argv[0];
    src = argv[1];
    ejsBlendObject(ejs, dest, src, overwrite);
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
static EjsObj *g_hashcode(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    mprAssert(argc == 1);
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) PTOL(argv[0]));
}
#endif


/*  
    Load a script or module. Name should have an extension. Name will be located according to the EJSPATH search strategy.
    static function load(filename: String, cache: String): void
 */
static EjsObj *g_load(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    cchar       *path, *cache, *cp;

    MPR_VERIFY_MEM();

    path = ejsToMulti(ejs, argv[0]);
    cache = (argc < 2) ? 0 : ejsToMulti(ejs, argv[1]);

    if ((cp = strrchr(path, '.')) != NULL && strcmp(cp, EJS_MODULE_EXT) != 0) {
        if (ejs->service->loadScriptFile == 0) {
            ejsThrowIOError(ejs, "load: Compiling is not enabled for %s", path);
        } else {
            return (ejs->service->loadScriptFile)(ejs, path, cache);
        }
    } else {
        ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, path), -1, -1, EJS_LOADER_RELOAD);
        return (ejs->exception) ? 0 : ejs->result;
    }
    return 0;
}


/*  
    Compute an MD5 checksum
    static function md5(name: String): void
 */
static EjsObj *g_md5(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsString   *str;
    char        *hash;

    MPR_VERIFY_MEM();
    str = (EjsString*) argv[0];
    hash = mprGetMD5Hash(ejsToMulti(ejs, str), str->length, NULL);
    return (EjsObj*) ejsCreateStringFromAsc(ejs, hash);
}


/*  
    Merge one object into another. This is useful for inheriting and optionally overwriting option hashes (among other
    things). The blending is done at the primitive property level. If overwrite is true, the property is replaced. If
    overwrite is false, the property will be added if it does not already exist
 */
int ejsBlendObject(Ejs *ejs, EjsObj *dest, EjsObj *src, int overwrite)
{
    EjsObj      *vp, *dp;
    EjsName     name;
    int         i, count;

    count = ejsGetPropertyCount(ejs, src);
    for (i = 0; i < count; i++) {
        vp = ejsGetProperty(ejs, src, i);
        if (vp == 0) {
            continue;
        }
        name = ejsGetPropertyName(ejs, src, i);
        /* NOTE: treats arrays as primitive types */
        if (!ejsIs(ejs, vp, Array) && !ejsIsXML(ejs, vp) && ejsGetPropertyCount(ejs, vp) > 0) {
            if ((dp = ejsGetPropertyByName(ejs, dest, name)) == 0 || ejsGetPropertyCount(ejs, dp) == 0) {
                ejsSetPropertyByName(ejs, dest, name, ejsClonePot(ejs, (EjsObj*) vp, 1));
            } else {
                ejsBlendObject(ejs, dp, vp, overwrite);
            }
        } else {
            /* Primitive type (including arrays) */
            if (overwrite) {
                ejsSetPropertyByName(ejs, dest, name, vp);
            } else {
                if (ejsLookupProperty(ejs, dest, name) < 0) {
                    ejsSetPropertyByName(ejs, dest, name, vp);
                }
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
    static function parseInt(input: String, radix: Number = 10): void
    Formats:
        [(+|-)][0][OCTAL_DIGITS]
        [(+|-)][0][(x|X)][HEX_DIGITS]
        [(+|-)][DIGITS]
 */
static EjsObj *g_parseInt(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
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
        n = (MprNumber) stoi(str, radix, &err);
        if (err) {
            return S(nan);
        }
        return (EjsObj*) ejsCreateNumber(ejs, n);
    }
    return S(nan);
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
    ssize       rc;
    int         i, count;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Array));

    args = argv[0];
    count = ejsGetPropertyCount(ejs, args);

    for (i = 0; i < count; i++) {
        if ((vp = ejsGetProperty(ejs, args, i)) != 0) {
            s  = (ejsIs(ejs, vp, String)) ? (EjsString*) vp : (EjsString*) ejsToString(ejs, vp);
            if (ejs->exception) {
                return 0;
            }
            data = ejsToMulti(ejs, s);
            rc = write(1, (char*) data, (int) strlen(data));
            if ((i+1) < count) {
                rc = write(1, " ", 1);
            }
        }
    }
    rc = write(1, "\n", 1);
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


void ejsCreateGlobalBlock(Ejs *ejs)
{
    EjsBlock    *block;
    int         sizeSlots;

    sizeSlots = (ejs->empty) ? 0 : max(ES_global_NUM_CLASS_PROP, EJS_NUM_GLOBAL);
    block = ejsCreateBlock(ejs, sizeSlots);
    ejs->global = block;
    block->isGlobal = 1;
    block->pot.numProp = (ejs->empty) ? 0: ES_global_NUM_CLASS_PROP;
    block->pot.shortScope = 1;
    SET_DYNAMIC(block, 1);
    mprSetName(block, "global");

    /*  
        Create the standard namespaces. Order matters here. This is the (reverse) order of lookup.
        Empty is first to maximize speed of searching dynamic properties. Ejs second to maximize builtin lookups.
     */
    ejsSetSpecial(ejs, S_iteratorSpace, ejsDefineReservedNamespace(ejs, block, NULL, EJS_ITERATOR_NAMESPACE));
    ejsSetSpecial(ejs, S_publicSpace, ejsDefineReservedNamespace(ejs, block, NULL, EJS_PUBLIC_NAMESPACE));
    ejsSetSpecial(ejs, S_ejsSpace, ejsDefineReservedNamespace(ejs, block, NULL, EJS_EJS_NAMESPACE));
    ejsSetSpecial(ejs, S_emptySpace, ejsDefineReservedNamespace(ejs, block, NULL, EJS_EMPTY_NAMESPACE));
}


void ejsConfigureGlobalBlock(Ejs *ejs)
{
    EjsBlock    *block;

    block = (EjsBlock*) ejs->global;
    mprAssert(block);
    
    ejsBindFunction(ejs, block, ES_assert, g_assert);
    ejsBindFunction(ejs, block, ES_cloneBase, (EjsProc) g_cloneBase);
    ejsBindFunction(ejs, block, ES_eval, g_eval);
    ejsBindFunction(ejs, block, ES_hashcode, g_hashcode);
    ejsBindFunction(ejs, block, ES_load, g_load);
    ejsBindFunction(ejs, block, ES_md5, g_md5);
    ejsBindFunction(ejs, block, ES_blend, g_blend);
    ejsBindFunction(ejs, block, ES_parse, g_parse);
    ejsBindFunction(ejs, block, ES_parseInt, g_parseInt);
    ejsBindFunction(ejs, block, ES_print, g_printLine);

    ejsSetProperty(ejs, ejs->global, ES_global, ejs->global);
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
