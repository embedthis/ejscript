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
static EjsObj *assertMethod(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    EjsFrame        *fp;
    EjsBoolean      *b;

    mprAssert(argc == 1);

    if (! ejsIsBoolean(argv[0])) {
        b = (EjsBoolean*) ejsCast(ejs, argv[0], ejs->booleanType);
    } else {
        b = (EjsBoolean*) argv[0];
    }
    mprAssert(b);

    if (b == 0 || !b->value) {
        fp = ejs->state->fp;
        if (fp->currentLine) {
            ejsThrowAssertError(ejs, "%s", fp->currentLine);
        } else {
            ejsThrowAssertError(ejs, "Assertion error");
        }
        return 0;
    }
    return vp;
}


#if MOVE_TO_DEBUG_CLASS || 1
/*  
    Trap to the debugger
    static function breakpoint(): Void
 */
static EjsObj *breakpoint(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
#if BLD_DEBUG && DEBUG_IDE
#if BLD_HOST_CPU_ARCH == MPR_CPU_IX86 || BLD_HOST_CPU_ARCH == MPR_CPU_IX64
#if WINCE
    /* Do nothing */
#elif BLD_WIN_LIKE
    __asm { int 3 };
#else
    asm("int $03");
#endif
#endif
#endif
    return 0;
}
#endif


/*  
    function blend(dest: Object, src: Object, overwrite: Boolean = true): void
 */
static EjsObj *blend(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *src, *dest;
    int         overwrite;

    overwrite = (argc == 3) ? (argv[2] == (EjsObj*) ejs->trueValue) : 1;
    dest = argv[0];
    src = argv[1];
    ejsBlendObject(ejs, dest, src, overwrite);
    return dest;
}


/*  
    Clone the base class. Used by Record.es
    static function cloneBase(klass: Type): Void
 */
static EjsObj *cloneBase(Ejs *ejs, EjsObj *ignored, int argc, EjsObj **argv)
{
    EjsType     *type;
    
    mprAssert(argc == 1);
    
    type = (EjsType*) argv[0];
    type->baseType = (EjsType*) ejsClone(ejs, (EjsObj*) type->baseType, 0);
    return 0;
}


/** DEPRECATED
    MOB - remove
    Print the arguments to the standard error with a new line.
    static function error(...args): void
static EjsObj *error(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsString   *s;
    EjsObj      *args, *vp;
    int         rc, i, count;

    mprAssert(argc == 1 && ejsIsArray(argv[0]));

    args = argv[0];
    count = ejsGetPropertyCount(ejs, args);

    for (i = 0; i < count; i++) {
        if ((vp = ejsGetProperty(ejs, args, i)) != 0) {
            if (!ejsIsString(vp)) {
                vp = (EjsObj*) ejsToJSON(ejs, vp, NULL);
            }
            if (ejs->exception) {
                return 0;
            }
            if (vp) {
                s = (EjsString*) vp;
                rc = write(2, s->value, s->length);
            }
        }
    }
    rc = write(2, "\n", 1);
    return 0;
}
 */


/*  
    function eval(script: String, cache: String = null): String
 */
static EjsObj *eval(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    cchar       *script, *cache;

    script = ejsGetString(ejs, argv[0]);
    if (argc < 2 || argv[1] == ejs->nullValue) {
        cache = NULL;
    } else {
        cache = ejsGetString(ejs, argv[1]);
    }
    if (ejsLoadScriptLiteral(ejs, script, cache, EC_FLAGS_NO_OUT | EC_FLAGS_DEBUG | EC_FLAGS_THROW | EC_FLAGS_VISIBLE) < 0) {
        return 0;
    }
    return ejs->result;
}


#if UNUSED
/*
    Format the stack
    function formatStack(): String
 */
static EjsObj *formatStackMethod(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, ejsFormatStack(ejs, NULL));
}
#endif


#if ES_hashcode
/*  
    Get the hash code for the object.
    function hashcode(o: Object): Number
 */
static EjsObj *hashcode(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    mprAssert(argc == 1);
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) PTOL(argv[0]));
}
#endif


/** 
    MOB REMOVE
    DEPREACATED
    Read a line of input
    static function input(): String
static EjsObj *input(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprFileSystem   *fs;
    MprBuf          *buf;
    EjsObj          *result;
    int             c;

    fs = mprGetMpr(ejs)->fileSystem;

    buf = mprCreateBuf(ejs, -1, -1);
    while ((c = getchar()) != EOF) {
#if BLD_WIN_LIKE
        if (c == fs->newline[0]) {
            continue;
        } else if (c == fs->newline[1]) {
            break;
        }
#else
        if (c == fs->newline[0]) {
            break;
        }
#endif
        mprPutCharToBuf(buf, c);
    }
    if (c == EOF && mprGetBufLength(buf) == 0) {
        return (EjsObj*) ejs->nullValue;
    }
    mprAddNullToBuf(buf);
    result = (EjsObj*) ejsCreateString(ejs, mprGetBufStart(buf));
    mprFree(buf);
    return result;
}
 */


/*  
    Load a script or module. Name should have an extension. Name will be located according to the EJSPATH search strategy.
    static function load(filename: String, cache: String): void
 */
static EjsObj *load(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    cchar       *path, *cache, *cp;

    path = ejsGetString(ejs, argv[0]);
    cache = (argc < 2) ? 0 : ejsGetString(ejs, argv[1]);

    if ((cp = strrchr(path, '.')) != NULL && strcmp(cp, EJS_MODULE_EXT) != 0) {
        if (ejs->service->loadScriptFile == 0) {
            ejsThrowIOError(ejs, "load: Compiling is not enabled for %s", path);
        } else {
            return (ejs->service->loadScriptFile)(ejs, path, cache);
        }
    } else {
        ejsLoadModule(ejs, path, -1, -1, 0);
        return (ejs->exception) ? 0 : ejs->result;
    }
    return 0;
}


/*  
    Compute an MD5 checksum
    static function md5(name: String): void
 */
static EjsObj *md5(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsString   *str;

    str = (EjsString*) argv[0];
    return (EjsObj*) ejsCreateStringAndFree(ejs, mprGetMD5Hash(ejs, str->value, str->length, NULL));
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
        if (!ejsIsArray(vp) && !ejsIsXML(ejs, vp) && ejsGetPropertyCount(ejs, vp) > 0) {
            if ((dp = ejsGetPropertyByName(ejs, dest, &name)) == 0 || ejsGetPropertyCount(ejs, dp) == 0) {
                name.name = mprStrdup(dest, name.name);
                name.space = mprStrdup(dest, name.space);
                ejsSetPropertyByName(ejs, dest, &name, ejsCloneObject(ejs, (EjsObj*) vp, 1));
            } else {
                ejsBlendObject(ejs, dp, vp, overwrite);
            }

        } else {
            /* Primitive type (including arrays) */
            if (overwrite) {
                name.name = mprStrdup(dest, name.name);
                name.space = mprStrdup(dest, name.space);
                ejsSetPropertyByName(ejs, dest, &name, vp);
            } else {
                if (ejsLookupProperty(ejs, dest, &name) < 0) {
                    name.name = mprStrdup(dest, name.name);
                    name.space = mprStrdup(dest, name.space);
                    ejsSetPropertyByName(ejs, dest, &name, vp);
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
static EjsObj *parse(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    cchar       *str;
    int         preferred;

    str = ejsGetString(ejs, argv[0]);

    if (argc == 2 && !ejsIsType(argv[1])) {
        ejsThrowArgError(ejs, "PreferredType argument is not a type");
        return 0;
    }
    while (isspace((int) *str)) {
        str++;
    }    
    preferred = (argc == 2) ? ((EjsType*) argv[1])->id : -1;
    return ejsParse(ejs, str, preferred);
}


/*
    Parse the input as an integer
    static function parseInt(input: String, radix: Number = 10): void
    Formats:
        [(+|-)][0][OCTAL_DIGITS]
        [(+|-)][0][(x|X)][HEX_DIGITS]
        [(+|-)][DIGITS]
 */
static EjsObj *parseInt(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprNumber   n;
    cchar       *str;
    int         radix, err;

    str = ejsGetString(ejs, argv[0]);
    radix = (argc >= 2) ? ejsGetInt(ejs, argv[1]) : 0;
    while (isspace((int) *str)) {
        str++;
    }
    if (*str == '-' || *str == '+' || isdigit((int) *str)) {
        n = (MprNumber) mprParseNumber(str, radix, &err);
        if (err) {
            return (EjsObj*) ejs->nanValue;
        }
        return (EjsObj*) ejsCreateNumber(ejs, n);
    }
    return (EjsObj*) ejs->nanValue;
}


/*  
    Print the arguments to the standard output with a new line.
    static function print(...args): void
    DEPRECATED static function output(...args): void
 */
static EjsObj *printLine(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsString   *s;
    EjsObj      *args, *vp;
    int         i, count, rc;

    mprAssert(argc == 1 && ejsIsArray(argv[0]));

    args = argv[0];
    count = ejsGetPropertyCount(ejs, args);

    for (i = 0; i < count; i++) {
        if ((vp = ejsGetProperty(ejs, args, i)) != 0) {
            s  = (ejsIsString(vp)) ? (EjsString*) vp : (EjsString*) ejsToString(ejs, vp);
            if (ejs->exception) {
                return 0;
            }
#if OLD
            if (vp && s) {
                char        *cp;
                if (s->length > 0 && s->value[0] == '"') {
                    cp = mprStrdup(ejs, s->value);
                    rc = write(1, cp, strlen(cp));
                    mprFree(cp);
                } else {
                    rc = write(1, s->value, s->length);
                }
            }
#endif
            rc = write(1, s->value, s->length);
            if ((i+1) < count) {
                rc = write(1, " ", 1);
            }
        }
    }
    rc = write(1, "\n", 1);
    return 0;
}


static EjsNamespace *addNamespace(Ejs *ejs, EjsBlock *block, cchar *space)
{
    EjsNamespace    *ns;

    ns = ejsDefineReservedNamespace(ejs, block, 0, space);
    mprAddHash(ejs->standardSpaces, space, ns);
    return ns;
}


void ejsFreezeGlobal(Ejs *ejs)
{
    EjsTrait    *trait;
    int         i;

    for (i = 0; i < ES_global_NUM_CLASS_PROP; i++) {
        if ((trait = ejsGetTrait(ejs, ejs->global, i)) != 0) {
            trait->attributes |= EJS_TRAIT_READONLY | EJS_TRAIT_FIXED;
        }
    }
}


void ejsCreateGlobalBlock(Ejs *ejs)
{
    EjsBlock    *block;
    int         sizeSlots;

    sizeSlots = (ejs->empty) ? 0 : max(ES_global_NUM_CLASS_PROP, EJS_NUM_GLOBAL);
    ejs->globalBlock = ejsCreateBlock(ejs, sizeSlots);
    ejs->globalBlock->isGlobal = 1;
    ejs->global = (EjsObj*) ejs->globalBlock;
    ejs->global->numSlots = (ejs->empty) ? 0: ES_global_NUM_CLASS_PROP;
    ejs->global->dynamic = 1;
    ejs->global->shortScope = 1;
    ejsSetDebugName(ejs->global, "global");
    
    /*  
        Create the standard namespaces. Order matters here. This is the (reverse) order of lookup.
        Empty is first to maximize speed of searching dynamic properties. Ejs second to maximize builtin lookups.
     */
    block = (EjsBlock*) ejs->global;
    ejs->iteratorSpace =    addNamespace(ejs, block, EJS_ITERATOR_NAMESPACE);
    ejs->publicSpace =      addNamespace(ejs, block, EJS_PUBLIC_NAMESPACE);
    ejs->ejsSpace =         addNamespace(ejs, block, EJS_EJS_NAMESPACE);
    ejs->emptySpace =       addNamespace(ejs, block, EJS_EMPTY_NAMESPACE);
}


void ejsConfigureGlobalBlock(Ejs *ejs)
{
    EjsBlock    *block;

    block = ejs->globalBlock;
    mprAssert(block);
    
//  MOB -- prefix these functions to be more unique need g_ prefix

    ejsBindFunction(ejs, block, ES_assert, assertMethod);
    ejsBindFunction(ejs, block, ES_breakpoint, breakpoint);
    ejsBindFunction(ejs, block, ES_cloneBase, (EjsProc) cloneBase);
    ejsBindFunction(ejs, block, ES_eval, eval);
#if UNUSED
    ejsBindFunction(ejs, block, ES_formatStack, formatStackMethod);
#endif
    ejsBindFunction(ejs, block, ES_hashcode, hashcode);
    ejsBindFunction(ejs, block, ES_load, load);
    ejsBindFunction(ejs, block, ES_md5, md5);
    ejsBindFunction(ejs, block, ES_blend, blend);
    ejsBindFunction(ejs, block, ES_parse, parse);
    ejsBindFunction(ejs, block, ES_parseInt, parseInt);
    ejsBindFunction(ejs, block, ES_print, printLine);

#if UNUSED
    /* DEPRECATED */
    ejsBindFunction(ejs, block, ES_error, error);
    ejsBindFunction(ejs, block, ES_input, input);
    ejsBindFunction(ejs, block, ES_output, printLine);
#endif
    ejsSetProperty(ejs, ejs->global, ES_global, ejs->global);
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
