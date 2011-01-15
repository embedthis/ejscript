/**
    ejsModule.c - Ejscript module management

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/********************************** Forwards **********************************/

static void manageConstants(EjsConstants *constants, int flags);
static void manageDebug(EjsDebug *debug, int flags);
static void manageModule(EjsModule *module, int flags);

/************************************ Code ************************************/

EjsModule *ejsCreateModule(Ejs *ejs, EjsString *name, int version, EjsConstants *constants)
{
    EjsModule   *mp;

    mprAssert(version >= 0);

    if ((mp = mprAllocObj(EjsModule, manageModule)) == NULL) {
        mprAssert(mp);
        return 0;
    }
    mp->name = name;
    mp->version = version;
    mp->vname = (version) ? ejsSprintf(ejs, "%@-%d", name, version) : mp->name;
    if (constants) {
        mp->constants = constants;
    } else if ((mp->constants = ejsCreateConstants(ejs, EJS_INDEX_INCR, EC_BUFSIZE)) == NULL) {
        return 0;
    }
    mp->constants->mp = mp;
    mprAssert(mp->checksum == 0);
    return mp;
}


static void manageModule(EjsModule *mp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(mp->name);
        mprMark(mp->vname);
        mprMark(mp->path);
        mprMark(mp->loadState);
        mprMark(mp->dependencies);
        mprMark(mp->file);
        mprMark(mp->code);
        mprMark(mp->initializer);
        mprMark(mp->constants);
        mprMark(mp->doc);
        mprMark(mp->scope);
        mprMark(mp->currentMethod);
        mprMark(mp->current);
        mprMark(mp->ejs);

    } else if (flags & MPR_MANAGE_FREE) {
        mprCloseFile(mp->file);
        if (mp->ejs) {
            mprAssert(mp->ejs->name);
            ejsRemoveModule(mp->ejs, mp);
        }
    }
}


static void manageNativeModule(EjsNativeModule *nm, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(nm->name);
    }
}


/*
    Register a native module callback to be invoked when it it time to configure the module. This is used by loadable modules
    when they are built statically.
 */
int ejsAddNativeModule(Ejs *ejs, EjsString *name, EjsNativeCallback callback, int checksum, int flags)
{
    EjsService          *sp;
    EjsNativeModule     *nm;

    if ((nm = mprAllocObj(EjsNativeModule, manageNativeModule)) == NULL) {
        return MPR_ERR_MEMORY;
    }
    nm->name = name;
    nm->callback = callback;
    nm->checksum = checksum;
    nm->flags = flags;

    sp = ejs->service;
    if (mprAddKey(sp->nativeModules, nm->name->value, nm) == 0) {
        return EJS_ERR;
    }
    return 0;
}


EjsNativeModule *ejsLookupNativeModule(Ejs *ejs, EjsString *name) 
{
    return mprLookupHash(ejs->service->nativeModules, name->value);
}


int ejsSetModuleConstants(Ejs *ejs, EjsModule *mp, EjsConstants *constants)
{
    mp->constants = constants;
    return 0;
}


/*
    Lookup a module name in the set of loaded modules
    If minVersion is <= 0, then any version up to, but not including maxVersion is acceptable.
    If maxVersion is < 0, then any version greater than minVersion is acceptable.
    If both are zero, then match the name itself and ignore minVersion and maxVersion
    If both are -1, then any version is acceptable.
    If both are equal, then only that version is acceptable.
 */
EjsModule *ejsLookupModule(Ejs *ejs, EjsString *name, int minVersion, int maxVersion)
{
    EjsModule   *mp, *best;
    int         next;

    if (maxVersion < 0) {
        maxVersion = MAXINT;
    }
    best = 0;
    for (next = 0; (mp = (EjsModule*) mprGetNextItem(ejs->modules, &next)) != 0; ) {
        if ((minVersion == 0 && maxVersion == 0) || (minVersion <= mp->version && mp->version <= maxVersion)) {
            if (mp->name == name) {
                if (best == 0 || best->version < mp->version) {
                    best = mp;
                }
            }
        }
    }
    return best;
}


int ejsAddModule(Ejs *ejs, EjsModule *mp)
{
    mprAssert(ejs->modules);
    //MOB
    mprAssert(ejs->modules->length < 40);
    mp->ejs = ejs;
    //MOB printf("Add modules (before) len %d mustYield %d newCount %d\n", ejs->modules->length, MPR->heap.mustYield, MPR->heap.newCount);
    return mprAddItem(ejs->modules, mp);
}


int ejsRemoveModule(Ejs *ejs, EjsModule *mp)
{
    mprAssert(ejs->modules);
    mp->ejs = 0;
    //MOB printf("Remove modules (before) %d\n", ejs->modules->length);
    return mprRemoveItem(ejs->modules, mp);
}


void ejsRemoveModules(Ejs *ejs)
{
    EjsModule   *mp;
    int         next;

    for (next = 0; (mp = mprGetNextItem(ejs->modules, &next)) != 0; ) {
        mp->ejs = 0;
    }
}


/************************************************* Constants ***************************************************/

static void manageConstants(EjsConstants *cp, int flags)
{
    int     i;
    
    if (flags & MPR_MANAGE_MARK) {
        mprMark(cp->pool);
        mprMark(cp->table);
        mprMark(cp->index);
        /*
            Racing here, but okay. May miss a few new elements, but they will be picked up on the next sweep.
         */
        for (i = 0; i < cp->indexCount; i++) {
            if (!(PTOI(cp->index[i]) & 0x1)) {
                mprMark(cp->index[i]);
            }
        }
    } else if (flags & MPR_MANAGE_FREE) {
        //MOB
        i = 7;
        i = 10;
    }
}


EjsConstants *ejsCreateConstants(Ejs *ejs, int count, ssize size)
{
    EjsConstants    *constants;

    mprAssert(ejs);

    if ((constants = mprAllocObj(EjsConstants, manageConstants)) == 0) {
        return NULL;
    }
    if (ejs->compiling) {
        if ((constants->table = mprCreateHash(EJS_DOC_HASH_SIZE, MPR_HASH_STATIC_VALUES)) == 0) {
            return 0;
        }
    }
    if ((constants->pool = mprAlloc(size)) == 0) {
        return 0;
    }
    constants->poolSize = size;
    constants->poolLength = 0;

    if ((constants->index = mprAlloc(count * sizeof(EjsString*))) == NULL) {
        return 0;
    }
    constants->index[0] = ejs->emptyString;
    constants->indexCount = 1;
#if UNUSED
    //  MOB -- get another solution for hold/release
    mprHold(constants->index);
#endif
    return constants;
}


int ejsGrowConstants(Ejs *ejs, EjsConstants *constants, ssize len)
{
    int     indexSize;

    if ((constants->poolLength + len) >= constants->poolSize) {
        constants->poolSize = ((constants->poolSize + len) + EC_BUFSIZE - 1) / EC_BUFSIZE * EC_BUFSIZE;
        if ((constants->pool = mprRealloc(constants->pool, constants->poolSize)) == 0) {
            return MPR_ERR_MEMORY;
        }
    }
    if (constants->indexCount >= constants->indexSize) {
        indexSize = (constants->indexCount + EJS_INDEX_INCR - 1) / EJS_INDEX_INCR * EJS_INDEX_INCR;
        if ((constants->index = mprRealloc(constants->index, indexSize * sizeof(EjsString*))) == NULL) {
            return MPR_ERR_MEMORY;
        }
        constants->indexSize = indexSize;
    }
    return 0;
}


int ejsAddConstant(Ejs *ejs, EjsConstants *constants, cchar *str)
{
    ssize       len, oldLen;

    if (constants->locked) {
        mprError("Constant pool for module is locked. Can't add constant \"%s\".",  str);
        return MPR_ERR_CANT_WRITE;
    }
    len = slen(str) + 1;
    if (ejsGrowConstants(ejs, constants, len) < 0) {
        return MPR_ERR_MEMORY;
    }
    memcpy(&constants->pool[constants->poolLength], str, len);
    oldLen = constants->poolLength;
    constants->poolLength += len;

    mprAddKey(constants->table, str, ITOP(constants->indexCount));
    constants->index[constants->indexCount] = ITOP(oldLen << 1 | 1);
    return constants->indexCount++;
}


EjsString *ejsCreateStringFromConst(Ejs *ejs, EjsModule *mp, int index)
{
    EjsConstants    *constants;
    EjsString       *sp;
    cchar           *str;
    int             value;

    constants = mp->constants;
    if (index < 0 || index >= constants->indexCount) {
        mprAssert(!(index < 0 || index >= constants->indexCount));
        return 0;
    }
    value = PTOI(constants->index[index]);
    if (value & 0x1) {
        str = &constants->pool[value >> 1];
        constants->index[index] = sp = ejsInternMulti(ejs, str, slen(str));
    }
    mprAssert(constants->index[index]);
    return constants->index[index];
}


/************************************************** Debug ******************************************************/

EjsDebug *ejsCreateDebug(Ejs *ejs)
{
    EjsDebug    *debug;
    ssize       size;

    size = sizeof(EjsDebug) + (EJS_DEBUG_INCR * sizeof(EjsLine));
    if ((debug = mprAllocBlock(size, MPR_ALLOC_MANAGER)) == 0) {
        return NULL;
    }
    mprSetManager(debug, manageDebug);
    debug->size = EJS_DEBUG_INCR;
    debug->numLines = 0;
    return debug;
}


int ejsAddDebugLine(Ejs *ejs, EjsDebug **debugp, int offset, MprChar *source)
{
    EjsDebug    *debug;
    EjsLine     *line;
    ssize       len;
    int         numLines;

    mprAssert(debugp);
    
    if (*debugp == 0) {
        *debugp = ejsCreateDebug(ejs);
    }
    debug = *debugp;
    if (debug->numLines >= debug->size) {
        debug->size += EJS_DEBUG_INCR;
        len = sizeof(EjsDebug) + (debug->size * sizeof(EjsLine));
        if ((debug = mprRealloc(debug, len)) == 0) {
            return MPR_ERR_MEMORY;
        }
        *debugp = debug;
    }
    if (debug->numLines > 0 && offset == debug->lines[debug->numLines - 1].offset) {
        line = &debug->lines[debug->numLines - 1];
        numLines = debug->numLines;
    } else {
        line = &debug->lines[debug->numLines];
        numLines = debug->numLines + 1;
    }
    line->source = source;
    line->offset = offset;
    debug->numLines = numLines;
    return 0;
}


static void manageDebug(EjsDebug *debug, int flags)
{
    int     i;

    if (flags & MPR_MANAGE_MARK) {
        for (i = 0; i < debug->numLines; i++) {
            mprMark(debug->lines[i].source);
        }
    }
}


/*
    Demand load debug information for a function
 */
static EjsDebug *loadDebug(Ejs *ejs, EjsFunction *fun)
{
    EjsModule   *mp;
    EjsDebug    *debug;
    EjsLine     *line;
    EjsCode     *code;
    MprOffset   prior;
    ssize       size;
    int         i, length;

    mp = fun->body.code->module;
    code = fun->body.code;
    prior = 0;
    debug = NULL;

    if (mp->file == 0) {
        if ((mp->file = mprOpenFile(mp->path, O_RDONLY | O_BINARY, 0666)) == NULL) {
            ejsThrowIOError(ejs, "Can't open module file %s", mp->path);
            return NULL;
        }
        mprEnableFileBuffering(mp->file, 0, 0);
    } else {
        prior = mprGetFilePosition(mp->file);
    }
    if (mprSeekFile(mp->file, SEEK_SET, code->debugOffset) != code->debugOffset) {
        mprSeekFile(mp->file, SEEK_SET, prior);
        mprAssert(0);
        return 0;
    }
    length = ejsModuleReadInt(ejs, mp);
    if (!mp->hasError) {
        size = sizeof(EjsDebug) + length * sizeof(EjsLine);
        if ((debug = mprAllocBlock(size, MPR_ALLOC_MANAGER)) != 0) {
            mprSetManager(debug, manageDebug);
            debug->size = length;
            debug->numLines = length;
            for (i = 0; i < debug->numLines; i++) {
                line = &debug->lines[i];
                line->offset = ejsModuleReadInt(ejs, mp);
                line->source = ejsModuleReadMultiAsWide(ejs, mp);
            }
        }
    }
    if (prior) {
        mprSeekFile(mp->file, SEEK_SET, prior);
    } else {
        mprCloseFile(mp->file);
        mp->file = 0;
    }
    if (mp->hasError) {
        return NULL;
    }
    return debug;
}


EjsLine *ejsGetDebugLine(Ejs *ejs, EjsFunction *fun, uchar *pc)
{
    EjsCode     *code;
    EjsDebug    *debug;
    int         i, offset;

    code = fun->body.code;
    if (code == 0) {
        return NULL;
    }
    offset = (int) (pc - code->byteCode) - 1;
    debug = code->debug;
    if (debug == 0) {
        if (code->debugOffset == 0) {
            return NULL;
        }
        if (debug == 0 && code->debugOffset) {
            if ((debug = loadDebug(ejs, fun)) == 0) {
                return NULL;
            }
            code->debug = debug;
        }
    }
    /*
        Source format is:  path|line| code
     */
    if (debug->numLines > 0) {
        for (i = 0; i < debug->numLines; i++) {
            if (offset < debug->lines[i].offset) {
                break;
            }
        }
        if (i > 0) {
            i--;
        }
        return &debug->lines[i];
    }
    return NULL;
}


int ejsGetDebugInfo(Ejs *ejs, EjsFunction *fun, uchar *pc, char **pathp, int *linep, MprChar **sourcep)
{
    EjsLine     *line;
    MprChar     *str, *tok, *path, *lineno, *source;
    int         i;

    if ((line = ejsGetDebugLine(ejs, fun, pc)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    str = wclone(line->source);
    path = wtok(str, "|", &tok);
    lineno = wtok(NULL, "|", &tok);
    source = tok;
    if (pathp) {
        *pathp = wclone(path);
    }
    if (linep) {
        *linep = (int) wtoi(lineno, 10, NULL);
    }
    if (sourcep) {
        *sourcep = wclone(source);
    }
    return i;
}


/************************************************** Encoding ******************************************************/
/*
    Decode a 32 bit integer. The maximum encoded value is EJS_ENCODE_MAX_WORD as the value is number encoded
 */
int ejsDecodeInt32(Ejs *ejs, uchar **pp)
{
    uchar   *start;
    int     value;

    start = *pp;
    value = (int) ejsDecodeNum(ejs, pp);
    *pp = start + 4;
    return value;
}


int64 ejsDecodeNum(Ejs *ejs, uchar **pp)
{
    uchar   *pos;
    uint64  t;
    uint    c;
    int     sign, shift;

    pos = *pp;
    c = (uint) *pos++;

    /*
        Map sign bit (0,1) to 1,-1
     */
    sign = 1 - ((c & 0x1) << 1);
    t = (c >> 1) & 0x3f;
    shift = 6;

    while (c & 0x80) {
        c = *pos++;
        t |= (c & 0x7f) << shift;
        shift += 7;
    }
    *pp = pos;
    return t * sign;
}


/*
    Decode a double using swapping
 */
double ejsDecodeDouble(Ejs *ejs, uchar **pp)
{
    double   value;

    memcpy(&value, *pp, sizeof(double));
    value = ejsSwapDouble(ejs, value);
    *pp += sizeof(double);
    return value;
}


/*
    Encode a number in a RLL encoding. Encoding is:
        Bit     0:  Sign
        Bits  1-6:  Low 6 bits (0-64)
        Bit     7:  Extension bit
        Bits 8-15:  Next 7 bits
        Bits   16:  Extension bit
        ...
 */
int ejsEncodeNum(Ejs *ejs, uchar *pos, int64 number)
{
    uchar       *start;
    uint        encoded;
    uint64      unumber;

    mprAssert(pos);

    start = pos;
    if (number < 0) {
        unumber = -number;
        encoded = (uint) (((unumber & 0x3F) << 1) | 1);
    } else {
        encoded = (uint) (((number & 0x3F) << 1));
        unumber = number;
    }
    unumber >>= 6;

    while (unumber) {
        *pos++ = encoded | 0x80;
        encoded = (int) (unumber & 0x7f);
        unumber >>= 7;
    }
    *pos++ = encoded;
    mprAssert((pos - start) < 11);
    return (int) (pos - start);
}


#if UNUSED && KEEP
int ejsEncodeUint(Ejs *ejs, uchar *pos, uint number)
{
    uchar       *start;
    uint        encoded;

    mprAssert(pos);

    start = pos;
    encoded = (uint) (((number & 0x3F) << 1));
    number >>= 6;

    while (number) {
        *pos++ = encoded | 0x80;
        encoded = (int) (number & 0x7f);
        number >>= 7;
    }
    *pos++ = encoded;
    mprAssert((pos - start) < 11);
    return (int) (pos - start);
}
#endif


/*
    Encode a 32 bit integer. The maximum encoded value is EJS_ENCODE_MAX_WORD as the value is number encoded
 */
int ejsEncodeInt32(Ejs *ejs, uchar *pos, int number)
{
    int         len;

    mprAssert(pos);
    if (abs(number) > EJS_ENCODE_MAX_WORD) {
        mprAssert("Code generation error. Word exceeds maximum");
        return 0;
    }
    len = ejsEncodeNum(ejs, pos, (int64) number);
    mprAssert(len <= 4);
    return 4;
}


/*
    Encode a double using swapping
 */
int ejsEncodeDouble(Ejs *ejs, uchar *pos, double number)
{
    number = ejsSwapDouble(ejs, number);
    memcpy(pos, &number, sizeof(double));
    return sizeof(double);
}


int ejsEncodeByteAtPos(Ejs *ejs, uchar *pos, int value)
{
    mprAssert(pos);
    *pos = value;
    return 0;
}


int ejsEncodeInt32AtPos(Ejs *ejs, uchar *pos, int value)
{
    return ejsEncodeInt32(ejs, pos, value);
}



void ejsModuleReadBlock(Ejs *ejs, EjsModule *mp, char *buf, int len)
{
    mprAssert(mp);

    if (mprReadFile(mp->file, buf, len) != len) {
        mp->hasError = 1;
    }
}


int ejsModuleReadByte(Ejs *ejs, EjsModule *mp)
{
    int     c;

    mprAssert(mp);

    if ((c = mprGetFileChar(mp->file)) < 0) {
        mp->hasError = 1;
        return 0;
    }
    return c;
}


/*
    Read a string constant. String constants are stored as token offsets into
    the constant pool. The pool contains null terminated UTF-8 strings.
 */
EjsString *ejsModuleReadConst(Ejs *ejs, EjsModule *mp)
{
    int     t;

    t = ejsModuleReadInt(ejs, mp);
    return ejsCreateStringFromConst(ejs, mp, t);
}


/*
    Decode a 4 byte number from a file
 */
int ejsModuleReadInt32(Ejs *ejs, EjsModule *mp)
{
    uchar   buf[4], *pp;

    mprAssert(mp);

    if (mprReadFile(mp->file, buf, 4) != 4) {
        mp->hasError = 1;
        return 0;
    }
    pp = buf;
    return ejsDecodeInt32(ejs, &pp);
}



/*
    Read a multibyte string. The length is encoded first, followed by a string of bytes.
 */
char *ejsModuleReadMulti(Ejs *ejs, EjsModule *mp)
{
    char    *buf;
    int     len;

    mprAssert(mp);

    len = ejsModuleReadInt(ejs, mp);
    if (mp->hasError || (buf = mprAlloc(len)) == 0) {
        return NULL;
    }
    if (mprReadFile(mp->file, buf, len) != len) {
        mp->hasError = 1;
        return NULL;
    }
    return buf;
}


/*
    Read a multibyte string. The length is encoded first, followed by a string of bytes.
 */
MprChar *ejsModuleReadMultiAsWide(Ejs *ejs, EjsModule *mp)
{
    mprAssert(mp);

    //  MOB OPT - need direct multi to wide without the double copy
    return amtow(ejsModuleReadMulti(ejs, mp), NULL);
}


int ejsModuleReadInt(Ejs *ejs, EjsModule *mp)
{
    return (int) ejsModuleReadNum(ejs, mp);
}


/*
    Read an encoded number. Numbers are variable-length and little-endian encoded in 7 bits with the 0x80 
    bit of each byte being a continuation bit.
 */
int64 ejsModuleReadNum(Ejs *ejs, EjsModule *mp)
{
    int64   t;
    int     c;
    int     sign, shift;

    mprAssert(ejs);
    mprAssert(mp);
    mprAssert(mp->file);

    if ((c = mprGetFileChar(mp->file)) < 0) {
        mp->hasError = 1;
        return 0;
    }
    /* Map sign bit (0,1) to 1,-1 */
    sign = 1 - ((c & 0x1) << 1);
    t = (c >> 1) & 0x3f;
    shift = 6;
    
    while (c & 0x80) {
        if ((c = mprGetFileChar(mp->file)) < 0) {
            mp->hasError = 1;
            return 0;
        }
        t |= (c & 0x7f) << shift;
        shift += 7;
    }
    return (t * sign);
}


/*
    Read a constant qname. String constants are stored as token offsets into
    the constant pool. The pool contains null terminated UTF-8 strings.
 */
EjsName ejsModuleReadName(Ejs *ejs, EjsModule *mp)
{
    EjsName     qname;
    int         t;

    t = ejsModuleReadInt(ejs, mp);
    qname.name = ejsCreateStringFromConst(ejs, mp, t);
    t = ejsModuleReadInt(ejs, mp);
    qname.space = ejsCreateStringFromConst(ejs, mp, t);
    return qname;
}


#if UNUSED && KEEP
int ejsSwapInt16(Ejs *ejs, int word)
{
    if (mprGetEndian(ejs) == MPR_LITTLE_ENDIAN) {
        return word;
    }
    return ((word & 0xFF) << 8) | ((word & 0xFF00) >> 8);
}
#endif


int ejsSwapInt32(Ejs *ejs, int word)
{
    if (mprGetEndian(ejs) == MPR_LITTLE_ENDIAN) {
        return word;
    }
    return ((word & 0xFF000000) >> 24) | ((word & 0xFF0000) >> 8) | ((word & 0xFF00) << 8) | ((word & 0xFF) << 24);
}


int64 ejsSwapInt64(Ejs *ejs, int64 a)
{
    int64   low, high;

    if (mprGetEndian(ejs) == MPR_LITTLE_ENDIAN) {
        return a;
    }
    low = a & 0xFFFFFFFF;
    high = (a >> 32) & 0xFFFFFFFF;
    return (int64) ((low & 0xFF) << 24 | (low & 0xFF00 << 8) | (low & 0xFF0000 >> 8) | (low & 0xFF000000 >> 16) |
            ((high & 0xFF) << 24 | (high & 0xFF00 << 8) | (high & 0xFF0000 >> 8) | (high & 0xFF000000 >> 16)) << 32);
}


double ejsSwapDouble(Ejs *ejs, double a)
{
    union {
        int64   i;
        double  d;
    } alias;
    
    if (mprGetEndian(ejs) == MPR_LITTLE_ENDIAN) {
        return a;
    }
    alias.d = a;
    return (double) ejsSwapInt64(ejs, alias.i);
}


#if UNUSED && KEEP
double ejsSwapDouble(Ejs *ejs, double a)
{
    int64   low, high;

    if (mprGetEndian(ejs) == MPR_LITTLE_ENDIAN) {
        return a;
    }
    low = ((int64) a) & 0xFFFFFFFF;
    high = (((int64) a) >> 32) & 0xFFFFFFFF;
    return  (double) ((low & 0xFF) << 24 | (low & 0xFF00 << 8) | (low & 0xFF0000 >> 8) | (low & 0xFF000000 >> 16) |
            ((high & 0xFF) << 24 | (high & 0xFF00 << 8) | (high & 0xFF0000 >> 8) | (high & 0xFF000000 >> 16)) << 32);
}
#endif

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
