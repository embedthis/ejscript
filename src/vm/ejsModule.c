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

    assert(version >= 0);

    if ((mp = mprAllocObj(EjsModule, manageModule)) == NULL) {
        assert(mp);
        return 0;
    }
    mp->name = name;
    mp->version = version;
    mp->vname = (version) ? ejsSprintf(ejs, "%@-%d", name, version) : mp->name;
    if ((mp->constants = constants) == 0) {
        if (ejsCreateConstants(ejs, mp, 0, EC_BUFSIZE, NULL) < 0) {
            return 0;
        }
    }
    assert(mp->checksum == 0);
    return mp;
}


static void manageModule(EjsModule *mp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(mp->name);
        mprMark(mp->vname);
        mprMark(mp->mutex);
        mprMark(mp->constants);
        mprMark(mp->initializer);
        mprMark(mp->loadState);
        mprMark(mp->dependencies);
        mprMark(mp->file);
        mprMark(mp->current);
        mprMark(mp->currentMethod);
        mprMark(mp->scope);
        mprMark(mp->path);
        mprMark(mp->code);
        mprMark(mp->doc);
        mprMark(mp->globalProperties);

    } else if (flags & MPR_MANAGE_FREE) {
        mprCloseFile(mp->file);
        ejsRemoveModuleFromAll(mp);
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
int ejsAddNativeModule(Ejs *ejs, cchar *name, EjsNativeCallback callback, int checksum, int flags)
{
    EjsService          *sp;
    EjsNativeModule     *nm;

    sp = ejs->service;
    if (ejsLookupNativeModule(ejs, name)) {
        return 0;
    }
    if ((nm = mprAllocObj(EjsNativeModule, manageNativeModule)) == NULL) {
        return MPR_ERR_MEMORY;
    }
    nm->name = sclone(name);
    nm->callback = callback;
    nm->checksum = checksum;
    nm->flags = flags;

    if (mprAddKey(sp->nativeModules, nm->name, nm) == 0) {
        return EJS_ERR;
    }
    return 0;
}


EjsNativeModule *ejsLookupNativeModule(Ejs *ejs, cchar *name) 
{
    return mprLookupKey(ejs->service->nativeModules, name);
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
    assert(ejs->modules);
    return mprAddItem(ejs->modules, mp);
}


void ejsRemoveModule(Ejs *ejs, EjsModule *mp)
{
    mprRemoveItem(ejs->modules, mp);
}


void ejsRemoveModuleFromAll(EjsModule *mp)
{
    Ejs         *ejs;
    EjsService  *sp;
    int         next;

    if ((sp = MPR->ejsService) == 0) {
        return;
    }
    lock(sp);
    for (ITERATE_ITEMS(sp->vmlist, ejs, next)) {
        mprRemoveItem(ejs->modules, mp);
    }
    unlock(sp);
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
    }
}


/*
    Create the module constants. Count is the number of strings in the constant pool. Size is the size of the pool.
    The optional pool parameter supplies a pre-allocated buffer of constant strings.
 */
int ejsCreateConstants(Ejs *ejs, EjsModule *mp, int count, ssize size, char *pool)
{
    EjsConstants    *constants;
    char            *pp;
    int             i;

    assert(ejs);

    if ((constants = mprAllocObj(EjsConstants, manageConstants)) == 0) {
        return MPR_ERR_MEMORY;
    }
    lock(mp);
    mp->constants = constants;

    if (ejs->compiling && ((constants->table = mprCreateHash(EJS_DOC_HASH_SIZE, MPR_HASH_STATIC_VALUES)) == 0)) {
        unlock(mp);
        return MPR_ERR_MEMORY;
    }
    constants->poolSize = size;
    if ((constants->pool = pool) == 0) {
        assert(count == 0);
        if ((constants->pool = mprAlloc(size)) == 0) {
            unlock(mp);
            return MPR_ERR_MEMORY;
        }
    }
    if (count) {
        constants->poolLength = size;
        if ((constants->index = mprAlloc(count * sizeof(EjsString*))) == NULL) {
            unlock(mp);
            return MPR_ERR_MEMORY;
        }
        assert(pool);
        if (pool) {
            for (pp = pool, i = 0; pp < &pool[constants->poolLength]; i++) {
                constants->index[i] = (void*) (((pp - pool) << 1) | 0x1);
                pp += slen(pp) + 1;
            }
            constants->indexCount = count;
        }
    }
    unlock(mp);
    return 0;
}


int ejsGrowConstants(Ejs *ejs, EjsModule *mp, ssize len)
{
    EjsConstants    *cp;
    int             indexSize;

    lock(mp);
    cp = mp->constants;
    if ((cp->poolLength + len) >= cp->poolSize) {
        cp->poolSize = ((cp->poolSize + len) + EC_BUFSIZE - 1) / EC_BUFSIZE * EC_BUFSIZE;
        if ((cp->pool = mprRealloc(cp->pool, cp->poolSize)) == 0) {
            unlock(mp);
            return MPR_ERR_MEMORY;
        }
    }
    if (cp->indexCount >= cp->indexSize) {
        indexSize = ((cp->indexCount + EJS_INDEX_INCR) / EJS_INDEX_INCR) * EJS_INDEX_INCR;
        if ((cp->index = mprRealloc(cp->index, indexSize * sizeof(EjsString*))) == NULL) {
            unlock(mp);
            return MPR_ERR_MEMORY;
        }
        cp->indexSize = indexSize;
    }
    unlock(mp);
    return 0;
}


int ejsAddConstant(Ejs *ejs, EjsModule *mp, cchar *str)
{
    EjsConstants    *cp;
    ssize           len, oldLen;
    int             index;

    cp = mp->constants;
    if (cp->locked) {
        mprError("Constant pool for module is locked. Cannot add constant \"%s\".",  str);
        return MPR_ERR_CANT_WRITE;
    }
    lock(mp);
    len = slen(str) + 1;
    if (ejsGrowConstants(ejs, mp, len) < 0) {
        unlock(mp);
        return MPR_ERR_MEMORY;
    }
    memcpy(&cp->pool[cp->poolLength], str, len);
    oldLen = cp->poolLength;
    cp->poolLength += len;

    mprAddKey(cp->table, str, ITOP(cp->indexCount));
    cp->index[cp->indexCount] = ITOP(oldLen << 1 | 1);
    index = cp->indexCount++;
    unlock(mp);
    return index;
}


EjsString *ejsCreateStringFromConst(Ejs *ejs, EjsModule *mp, int index)
{
    EjsConstants    *constants;
    EjsString       *sp;
    cchar           *str;
    int             value;

    lock(mp);
    constants = mp->constants;
    if (index < 0 || index >= constants->indexCount) {
        assert(!(index < 0 || index >= constants->indexCount));
        unlock(mp);
        return 0;
    }
    value = PTOI(constants->index[index]);
    if (value & 0x1) {
        str = &constants->pool[value >> 1];
        constants->index[index] = sp = ejsInternMulti(ejs, str, slen(str));
    }
    unlock(mp);
    assert(constants->index[index]);
    return constants->index[index];
}


/************************************************** Debug ******************************************************/

EjsDebug *ejsCreateDebug(Ejs *ejs, int length)
{
    EjsDebug    *debug;
    ssize       size;
    int         count;

    count = (length > 0) ? length : EJS_DEBUG_INCR;
    size = sizeof(EjsDebug) + (count * sizeof(EjsLine));
    if ((debug = mprAllocBlock(size, MPR_ALLOC_MANAGER)) == 0) {
        return NULL;
    }
    mprSetManager(debug, (MprManager) manageDebug);
    debug->size = count;
    debug->numLines = length;
    debug->magic = EJS_DEBUG_MAGIC;
    return debug;
}


int ejsAddDebugLine(Ejs *ejs, EjsDebug **debugp, int offset, wchar *source)
{
    EjsDebug    *debug;
    EjsLine     *line;
    ssize       len;
    int         numLines;

    assert(debugp);
    
    if (*debugp == 0) {
        *debugp = ejsCreateDebug(ejs, 0);
    }
    debug = *debugp;
    assert(debug->magic == EJS_DEBUG_MAGIC);
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

    assert(debug->magic == EJS_DEBUG_MAGIC);

    if (flags & MPR_MANAGE_MARK) {
        for (i = 0; i < debug->numLines; i++) {
            mprMark(debug->lines[i].source);
        }
    } else if (flags & MPR_MANAGE_FREE) {
        /* Value to signify the debug record has been freed */
        debug->magic = 7;
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
    MprOff      prior;
    int         i, length;

    mp = fun->body.code->module;
    code = fun->body.code;
    prior = 0;
    debug = NULL;

    /*
        Synchronize with ejsLoadModule. May be multiple threads using immutable types
     */
    lock(mp);
    if (mp->file == 0) {
        if ((mp->file = mprOpenFile(mp->path, O_RDONLY | O_BINARY, 0666)) == NULL) {
            mprTrace(5, "Cannot open module file %s", mp->path);
            unlock(mp);
            return NULL;
        }
        mprEnableFileBuffering(mp->file, 0, 0);
    } else {
        prior = mprGetFilePosition(mp->file);
    }
    if (mprSeekFile(mp->file, SEEK_SET, code->debugOffset) != code->debugOffset) {
        mprSeekFile(mp->file, SEEK_SET, prior);
        unlock(mp);
        return 0;
    }
    length = ejsModuleReadInt(ejs, mp);
    if (!mp->hasError) {
        if ((debug = ejsCreateDebug(ejs, length)) != 0) {
            assert(debug->numLines == length);
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
    unlock(mp);
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


int ejsGetDebugInfo(Ejs *ejs, EjsFunction *fun, uchar *pc, char **pathp, int *linep, wchar **sourcep)
{
    EjsLine     *line;
    wchar       *str, *tok, *path, *lineno, *source;

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
        *linep = (int) wtoi(lineno);
    }
    if (sourcep) {
        *sourcep = wclone(source);
    }
    return 0;
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

    assert(pos);

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
    assert((pos - start) < 11);
    return (int) (pos - start);
}


#if UNUSED && KEEP
int ejsEncodeUint(Ejs *ejs, uchar *pos, uint number)
{
    uchar       *start;
    uint        encoded;

    assert(pos);

    start = pos;
    encoded = (uint) (((number & 0x3F) << 1));
    number >>= 6;

    while (number) {
        *pos++ = encoded | 0x80;
        encoded = (int) (number & 0x7f);
        number >>= 7;
    }
    *pos++ = encoded;
    assert((pos - start) < 11);
    return (int) (pos - start);
}
#endif


/*
    Encode a 32 bit integer. The maximum encoded value is EJS_ENCODE_MAX_WORD as the value is number encoded
 */
int ejsEncodeInt32(Ejs *ejs, uchar *pos, int number)
{
    int         len;

    assert(pos);
    if (abs(number) > EJS_ENCODE_MAX_WORD) {
        assert("Code generation error. Word exceeds maximum");
        return 0;
    }
    memset(pos, 0, 4);
    len = ejsEncodeNum(ejs, pos, (int64) number);
    assert(len <= 4);
    len = 4;
    return len;
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
    assert(pos);
    *pos = value;
    return 0;
}


int ejsEncodeInt32AtPos(Ejs *ejs, uchar *pos, int value)
{
    return ejsEncodeInt32(ejs, pos, value);
}



void ejsModuleReadBlock(Ejs *ejs, EjsModule *mp, char *buf, int len)
{
    assert(mp);

    if (mprReadFile(mp->file, buf, len) != len) {
        mp->hasError = 1;
    }
}


int ejsModuleReadByte(Ejs *ejs, EjsModule *mp)
{
    int     c;

    assert(mp);

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

    assert(mp);

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

    assert(mp);

    len = ejsModuleReadInt(ejs, mp);
    assert(len >= 0);
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
wchar *ejsModuleReadMultiAsWide(Ejs *ejs, EjsModule *mp)
{
    assert(mp);

    //  OPT - need direct multi to wide without the double copy
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
    int     c, sign, shift;

    assert(ejs);
    assert(mp);
    assert(mp->file);

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
    if (mprGetEndian(ejs) == BIT_LITTLE_ENDIAN) {
        return word;
    }
    return ((word & 0xFF) << 8) | ((word & 0xFF00) >> 8);
}
#endif


int ejsSwapInt32(Ejs *ejs, int word)
{
    if (mprGetEndian(ejs) == BIT_LITTLE_ENDIAN) {
        return word;
    }
    return ((word & 0xFF000000) >> 24) | ((word & 0xFF0000) >> 8) | ((word & 0xFF00) << 8) | ((word & 0xFF) << 24);
}


int64 ejsSwapInt64(Ejs *ejs, int64 a)
{
    int64   low, high;

    if (mprGetEndian(ejs) == BIT_LITTLE_ENDIAN) {
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
    
    if (mprGetEndian(ejs) == BIT_LITTLE_ENDIAN) {
        return a;
    }
    alias.d = a;
    return (double) ejsSwapInt64(ejs, alias.i);
}


#if UNUSED && KEEP
double ejsSwapDouble(Ejs *ejs, double a)
{
    int64   low, high;

    if (mprGetEndian(ejs) == BIT_LITTLE_ENDIAN) {
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

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
