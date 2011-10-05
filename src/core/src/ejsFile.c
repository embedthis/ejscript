/**
    ejsFile.c - File class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/********************************** Defines ***********************************/

#if BLD_WIN_LIKE
#define isDelim(fp, c)  (c == '/' || c == fp->delimiter)
#else
#define isDelim(fp, c)  (c == fp->delimiter)
#endif

#define FILE_OPEN           0x1     /* File is opened */
#define FILE_READ           0x2     /* File is opened for reading */
#define FILE_WRITE          0x4     /* File is opened for writing */

/**************************** Forward Declarations ****************************/

static EjsObj *closeFile(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv);
static int mapMode(cchar *mode);
static EjsObj *openFile(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv);
static ssize readData(Ejs *ejs, EjsFile *fp, EjsByteArray *ap, ssize offset, ssize count);

#if BLD_CC_MMU && FUTURE
static void *mapFile(EjsFile *fp, uint size, int mode);
static void unmapFile(EjsFile *fp);
#endif

/************************************ Helpers *********************************/
/*  
    Index into a file and extract a byte. This is random access reading.
 */
static EjsNumber *getFileProperty(Ejs *ejs, EjsFile *fp, int slotNum)
{
    MprOff  offset;
    int     c;

    if (!(fp->mode & FILE_OPEN)) {
        ejsThrowIOError(ejs, "File is not open");
        return 0;
    }
#if KEEP
    if (fp->mode & FILE_READ) {
        if (slotNum >= fp->info.size) {
            ejsThrowOutOfBoundsError(ejs, "Bad file index");
            return 0;
        }
    }
    if (slotNum < 0) {
        ejsThrowOutOfBoundsError(ejs, "Bad file index");
        return 0;
    }
#endif

#if BLD_CC_MMU && FUTURE
    //  must check against mapped size here.
    c = fp->mapped[slotNum];
#else
    offset = mprSeekFile(fp->file, SEEK_CUR, 0);
    if (offset != slotNum) {
        if (mprSeekFile(fp->file, SEEK_SET, slotNum) != slotNum) {
            ejsThrowIOError(ejs, "Can't seek to file offset");
            return 0;
        }
    }
    c = mprPeekFileChar(fp->file);
    if (c < 0) {
        ejsThrowIOError(ejs, "Can't read file");
        return 0;
    }
#endif
    return ejsCreateNumber(ejs, c);
}



static int lookupFileProperty(Ejs *ejs, EjsFile *fp, EjsName qname)
{
    int     index;

    if (qname.name == 0 || !isdigit((int) qname.name->value[0])) {
        return EJS_ERR;
    }
    if (!(fp->mode & FILE_OPEN)) {
        ejsThrowIOError(ejs, "File is not open");
        return 0;
    }
    index = ejsAtoi(ejs, qname.name, 10);
    if (index < mprGetFileSize(fp->file)) {
        return index;
    }
    return EJS_ERR;
}


/*  
    Set a byte in the file at the offset designated by slotNum.
 */
static int setFileProperty(Ejs *ejs, EjsFile *fp, int slotNum, EjsObj *value)
{
    MprOff  offset;
    int     c;

    if (!(fp->mode & FILE_OPEN)) {
        ejsThrowIOError(ejs, "File is not open");
        return 0;
    }
    if (!(fp->mode & FILE_WRITE)) {
        ejsThrowIOError(ejs, "File is not opened for writing");
        return 0;
    }
    c = ejsIs(ejs, value, Number) ? ejsGetInt(ejs, value) : ejsGetInt(ejs, ejsToNumber(ejs, value));

    offset = mprSeekFile(fp->file, SEEK_CUR, 0);
    if (slotNum < 0) {
        //  could have an mprGetPosition(file) API
        slotNum = (int) offset;
    }

#if BLD_CC_MMU && FUTURE
    fp->mapped[slotNum] = c;
#else
    if (offset != slotNum && mprSeekFile(fp->file, SEEK_SET, slotNum) != slotNum) {
        ejsThrowIOError(ejs, "Can't seek to file offset");
        return 0;
    }
    if (mprPutFileChar(fp->file, c) < 0) {
        ejsThrowIOError(ejs, "Can't write file");
        return 0;
    }
#endif
    return slotNum;
}


/************************************ Methods *********************************/

//  TODO - rename
static int ejsGetNumOption(Ejs *ejs, EjsObj *options, cchar *field, int defaultValue, bool optional)
{
    EjsObj      *vp;
    EjsNumber   *num;

    vp = ejsGetPropertyByName(ejs, options, EN(field));
    if (vp == 0) {
        if (optional) {
            return defaultValue;
        }
        ejsThrowArgError(ejs, "Required option \"%s\" is missing", field);
        return 0;
    }
    num = ejsToNumber(ejs, vp);
    if (!ejsIs(ejs, num, Number)) {
        ejsThrowArgError(ejs, "Bad option type for field \"%s\"", field);
        return 0;
    }
    return (int) num->value;
}


static cchar *getStrOption(Ejs *ejs, EjsObj *options, cchar *field, cchar *defaultValue, bool optional)
{
    EjsObj      *vp;
    EjsString   *str;

    vp = ejsGetPropertyByName(ejs, options, EN(field));
    if (vp == 0) {
        if (optional) {
            return sclone(defaultValue);
        }
        ejsThrowArgError(ejs, "Required option %s is missing", field);
        return 0;
    }
    str = ejsToString(ejs, vp);
    if (!ejsIs(ejs, str, String)) {
        ejsThrowArgError(ejs, "Bad option type for field \"%s\"", field);
        return 0;
    }
    return ejsToMulti(ejs, str);
}


/*  
    Constructor
    function File(path: Object, options: Object = null)
 */
static EjsFile *fileConstructor(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    EjsObj      *pp;
    cchar       *path;

    if (argc < 1 || argc > 2) {
        ejsThrowArgError(ejs, "Bad args");
        return 0;
    }
    pp = argv[0];
    if (ejsIs(ejs, pp, Path)) {
        path = ((EjsPath*) pp)->value;
    } else if (ejsIs(ejs, pp, String)) {
        path = ejsToMulti(ejs, pp);
    } else {
        ejsThrowIOError(ejs, "Bad path");
        return 0;
    }
    fp->path = mprNormalizePath(path);
    if (argc == 2) {
        openFile(ejs, fp, 1, &argv[1]);
    }
    return fp;
}


/*  
    function get canRead(): Boolean
 */
static EjsBoolean *canReadFile(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, fp->mode & FILE_OPEN && (fp->mode & FILE_READ));
}


/*  
    function get canRead(): Boolean
 */
static EjsBoolean *canWriteFile(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, fp->mode & FILE_OPEN && (fp->mode & FILE_WRITE));
}

/*  
    Close the file and free up all associated resources.
    function close(): void
 */
static EjsObj *closeFile(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    if (fp->mode & FILE_OPEN && fp->mode & FILE_WRITE) {
        if (mprFlushFile(fp->file) < 0) {
            if (ejs) {
                ejsThrowIOError(ejs, "Can't flush file data");
            } else {
                mprError("Can't flush file data");
            }
            return 0;
        }
    }
    if (fp->file) {
        mprCloseFile(fp->file);
        fp->file = 0;
    }
#if BLD_CC_MMU && FUTURE
    if (fp->mapped) {
        unmapFile(fp);
        fp->mapped = 0;
    }
#endif
    fp->mode = 0;
    fp->modeString = 0;
    return 0;
}


/*  
    Function to iterate and return the next element index.
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsNumber *nextKey(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsFile     *fp;

    fp = (EjsFile*) ip->target;
    if (!ejsIs(ejs, fp, File)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }
    if (ip->index < fp->info.size) {
        return ejsCreateNumber(ejs, ip->index++);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*  
    Return the default iterator for use with "for ... in". This returns byte offsets in the file.
    iterator native function get(): Iterator
 */
static EjsIterator *getFileIterator(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    mprGetPathInfo(fp->path, &fp->info);
    return ejsCreateIterator(ejs, fp, nextKey, 0, NULL);
}


/*  
    Function to iterate and return the next element value.
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsObj *nextValue(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsFile     *fp;

    fp = (EjsFile*) ip->target;
    if (!ejsIs(ejs, fp, File)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }

    if (ip->index < fp->info.size) {
#if !BLD_CC_MMU || 1
        if (mprSeekFile(fp->file, SEEK_CUR, 0) != ip->index) {
            if (mprSeekFile(fp->file, SEEK_SET, ip->index) != ip->index) {
                ejsThrowIOError(ejs, "Can't seek to %d", ip->index);
                return 0;
            }
        }
        ip->index++;
        return (EjsObj*) ejsCreateNumber(ejs, mprGetFileChar(fp->file));
#else
        return (EjsObj*) ejsCreateNumber(ejs, fp->mapped[ip->index++]);
#endif
    }

#if BLD_CC_MMU && FUTURE
    unmapFile(fp);
    fp->mapped = 0;
#endif

    ejsThrowStopIteration(ejs);
    return 0;
}


/*  
    Return an iterator to enumerate the bytes in the file. For use with "for each ..."
    iterator native function getValues(): Iterator
 */
static EjsObj *getFileValues(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    mprGetPathInfo(fp->path, &fp->info);

    return (EjsObj*) ejsCreateIterator(ejs, (EjsObj*) fp, nextValue, 0, NULL);
}


/*  
    Get a path object for the file
    function get path(): Path
 */
static EjsObj *getFilePath(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathFromAsc(ejs, fp->path);
}


/*  
    Get the current I/O position in the file.
    function get position(): Number
 */
static EjsObj *getFilePosition(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    if (fp->file == 0) {
        ejsThrowStateError(ejs, "File not opened");
        return 0;
    }
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) mprGetFilePosition(fp->file));
}


/*  
    Seek to a new location in the file and set the File marker to a new read/write position.
    function set position(value: Number): void
 */
static EjsObj *setFilePosition(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    MprOff      pos;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Number));
    pos = ejsGetInt(ejs, argv[0]);

    if (fp->file == 0) {
        ejsThrowStateError(ejs, "File not opened");
        return 0;
    }
    pos = ejsGetInt(ejs, argv[0]);
    if (mprSeekFile(fp->file, SEEK_SET, pos) != pos) {
        ejsThrowIOError(ejs, "Can't seek to %Ld", pos);
    }
    return 0;
}


/*  
    function get isOpen(): Boolean
 */
static EjsObj *isFileOpen(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, fp->mode & FILE_OPEN);
}


/*  
    Constructor
    function open(options: Object = null): File
    NOTE: options can be an options hash or as mode string
 */
static EjsObj *openFile(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    EjsObj  *options;
    cchar   *mode;
    int     perms, omode;

    if (argc < 0 || argc > 1) {
        ejsThrowArgError(ejs, "Bad args");
        return 0;
    }
    options = argv[0];
    if (argc == 0 || !ejsIsDefined(ejs, options)) {
        omode = O_RDONLY | O_BINARY;
        perms = EJS_FILE_PERMS;
        fp->mode = FILE_READ;
        mode = "r";
    } else {
        if (ejsIs(ejs, options, String)) {
            mode = ejsToMulti(ejs, options);
            perms = EJS_FILE_PERMS;
        } else {
            perms = ejsGetNumOption(ejs, options, "permissions", EJS_FILE_PERMS, 1);
            mode = getStrOption(ejs, options, "mode", "r", 1);
            if (ejs->exception) {
                return 0;
            }
        }
        omode = mapMode(mode);
        if (!(omode & O_WRONLY)) {
            fp->mode |= FILE_READ;
        }
        if (omode & (O_WRONLY | O_RDWR)) {
            fp->mode |= FILE_WRITE;
        }
    }
    fp->modeString = sclone(mode);
    fp->perms = perms;

    if (fp->file) {
        mprCloseFile(fp->file);
    }
    fp->file = mprOpenFile(fp->path, omode, perms);
    if (fp->file == 0) {
        ejsThrowIOError(ejs, "Can't open %s", fp->path);
        return 0;
    }

#if BLD_CC_MMU && FUTURE
    mprGetPathInfo(&fp->info);
    fp->mapped = mapFile(fp, fp->info.size, MPR_MAP_READ | MPR_MAP_WRITE);
#endif
    fp->mode |= FILE_OPEN;
    return (EjsObj*) fp;
}


static EjsObj *getFileOptions(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    EjsObj      *options;

    options = (EjsObj*) ejsCreateEmptyPot(ejs);
    ejsSetPropertyByName(ejs, options, EN("mode"), ejsCreateStringFromAsc(ejs, fp->modeString));
    ejsSetPropertyByName(ejs, options, EN("permissions"), ejsCreateNumber(ejs, fp->perms));
    return options;
}

/*  
    Read data bytes from a file
    function readBytes(count: Number = -1): ByteArray
 */
static EjsObj *readFileBytes(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    EjsByteArray    *result;
    MprPath         info;
    ssize           count, totalRead;

    if (argc == 0) {
        count = -1;
    } else if (argc != 1) {
        count = 0;
        ejsThrowArgError(ejs, "Bad args");
        return 0;
    } else {
        mprAssert(argc == 1 && ejsIs(ejs, argv[0], Number));
        count = ejsGetInt(ejs, argv[0]);
    }
    if (fp->file == 0) {
        ejsThrowStateError(ejs, "File not open");
        return 0;
    }
    if (!(fp->mode & FILE_READ)) {
        ejsThrowStateError(ejs, "File not opened for reading");
        return 0;
    }
    if (count < 0) {
        //  TODO OPT could this be cached in fp->info 
        if (mprGetPathInfo(fp->path, &info) == 0) {
            count = (int) info.size;
            count -= (int) mprGetFilePosition(fp->file);
        } else {
            count = MPR_BUFSIZE;
        }
        mprAssert(count >= 0);
    }
    result = ejsCreateByteArray(ejs, count);
    if (result == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    totalRead = readData(ejs, fp, result, 0, count);
    if (totalRead < 0) {
        ejsThrowIOError(ejs, "Can't read from file: %s", fp->path);
        return 0;
    } else if (totalRead == 0) {
        return ESV(null);
    }
    ejsSetByteArrayPositions(ejs, result, 0, totalRead);
    return (EjsObj*) result;
}


/*  
    Read data as a string
    function readString(count: Number = -1): String
 */
static EjsString *readFileString(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    EjsString       *result;
    MprPath         info;
    ssize           totalRead;
    int             count;

    if (argc == 0) {
        count = -1;
    } else if (argc != 1) {
        count = 0;
        ejsThrowArgError(ejs, "Bad args");
        return 0;
    } else {
        mprAssert(argc == 1 && ejsIs(ejs, argv[0], Number));
        count = ejsGetInt(ejs, argv[0]);
    }
    if (fp->file == 0) {
        ejsThrowStateError(ejs, "File not open");
        return 0;
    }
    if (!(fp->mode & FILE_READ)) {
        ejsThrowStateError(ejs, "File not opened for reading");
        return 0;
    }
    if (count < 0) {
        //  TODO OPT could this be cached in fp->info 
        if (mprGetPathInfo(fp->path, &info) == 0) {
            count = (int) info.size;
            count -= (int) mprGetFilePosition(fp->file);
        } else {
            count = MPR_BUFSIZE;
        }
        mprAssert(count >= 0);
    }
    if ((result = ejsCreateBareString(ejs, count)) == NULL) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    totalRead = mprReadFile(fp->file, result->value, count);
    if (totalRead != count) {
        ejsThrowIOError(ejs, "Can't read from file: %s", fp->path);
        return 0;
    }
    return ejsInternString(result);
}


/*  
    Read data bytes from a file. If offset is < 0, then append to the write position.
    function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number
 */
static EjsNumber *readFile(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    EjsByteArray    *buffer;
    MprPath         info;
    ssize           offset, count, totalRead;

    mprAssert(1 <= argc && argc <= 3);

    buffer = (EjsByteArray*) argv[0];
    offset = (argc >= 2) ? ejsGetInt(ejs, argv[1]): 0;
    count = (argc >= 3) ? ejsGetInt(ejs, argv[2]): -1;

    if (fp->file == 0) {
        ejsThrowStateError(ejs, "File not open");
        return 0;
    }
    if (!(fp->mode & FILE_READ)) {
        ejsThrowStateError(ejs, "File not opened for reading");
        return 0;
    }
    if (offset >= buffer->length) {
        ejsThrowOutOfBoundsError(ejs, "Bad read offset value");
        return 0;
    }
    if (offset < 0) {
        offset = buffer->writePosition;
    } else if (offset == 0) {
        ejsSetByteArrayPositions(ejs, buffer, 0, 0);
    }
    if (count < 0) {
        //  TODO OPT could this be cached in fp->info 
        if (mprGetPathInfo(fp->path, &info) == 0) {
            count = (int) info.size;
            count -= (int) mprGetFilePosition(fp->file);
        } else {
            count = MPR_BUFSIZE;
        }
        mprAssert(count >= 0);
    }
    totalRead = readData(ejs, fp, buffer, offset, count);
    if (totalRead < 0) {
        return 0;
    } else if (totalRead == 0) {
        return ESV(zero);
    }
    ejsSetByteArrayPositions(ejs, buffer, -1, offset + totalRead);
    return ejsCreateNumber(ejs, (MprNumber) totalRead);
}


/*  
    Get the size of the file associated with this File object.
    override function get size(): Number
 */
static EjsObj *getFileSize(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    if (mprGetPathInfo(fp->path, &info) < 0) {
        return (EjsObj*) ESV(minusOne);
    }
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) info.size);
}


/*  
    function truncate(size: Number): Void
 */
EjsObj *truncateFile(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    int     size;

    size = ejsGetInt(ejs, argv[0]);
    if (mprTruncateFile(fp->path, size) < 0) {
        ejsThrowIOError(ejs, "Cant truncate %s", fp->path);
    }
    return 0;
}


/*  
    Write data to the file
    function write(data: Object): Number
 */
EjsObj *writeFile(Ejs *ejs, EjsFile *fp, int argc, EjsObj **argv)
{
    EjsArray        *args;
    EjsByteArray    *ap;
    EjsObj          *vp;
    EjsString       *str;
    cchar           *buf;
    ssize           len, written;
    int             i;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Array));

    args = (EjsArray*) argv[0];

    if (!(fp->mode & FILE_WRITE)) {
        ejsThrowStateError(ejs, "File not opened for writing");
        return 0;
    }
    written = 0;

    for (i = 0; i < args->length; i++) {
        vp = ejsGetProperty(ejs, (EjsObj*) args, i);
        mprAssert(vp);
        switch (TYPE(vp)->sid) {
        case S_ByteArray:
            ap = (EjsByteArray*) vp;
            //  TODO UNICODE ENCODING
            buf = (cchar*) &ap->value[ap->readPosition];
            len = ap->writePosition - ap->readPosition;
            break;

        case S_String: // UNICODE
            buf = awtom(((EjsString*) vp)->value, &len);
            break;

        default:
            str = ejsToString(ejs, vp);
            buf = awtom(((EjsString*) str)->value, &len);
            break;
        }
        if (mprWriteFile(fp->file, buf, len) != len) {
            mprLog(0, "Write IO error %d\n", mprGetOsError());
            ejsThrowIOError(ejs, "Can't write to %s", fp->path);
            return 0;
        }
        written += len;
        /* Use GC to free buf as it may not be allocated */
    }
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) written);
}


/*********************************** Support **********************************/

/*  
    Read the specified count of bytes into the byte array. Grow the array if required and growable
 */
static ssize readData(Ejs *ejs, EjsFile *fp, EjsByteArray *ap, ssize offset, ssize count)
{
    ssize   len, bytes;

    if (count <= 0) {
        return 0;
    }
    len = ap->length - offset;
    if (len < count) {
        if (ap->resizable) {
            ejsGrowByteArray(ejs, ap, ap->length + (count - len));
        }
        len = ap->length - offset;
    }
    bytes = mprReadFile(fp->file, &ap->value[offset], len);
    if (bytes < 0) {
        ejsThrowIOError(ejs, "Error reading from %s", fp->path);
    }
    return bytes;
}


#if BLD_CC_MMU && FUTURE
static void *mapFile(EjsFile *fp, uint size, int mode)
{
    Mpr         *mpr;
    void        *ptr;

    mpr = mprGetMpr();
    x = ~(mpr->alloc.pageSize - 1);
    size = (size + mpr->alloc.pageSize - 1) & ~(mpr->alloc.pageSize - 1);
#if MACOSX || LINUX || FREEBSD
    //  USE MAP_SHARED instead of MAP_PRIVATE if opened for write
    ptr = mmap(0, size, mode, MAP_FILE | MAP_PRIVATE, fp->file->fd, 0);
#else
    ptr = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, mapProt(mode));
#endif

    if (ptr == 0) {
        mprSetMemError(mpr);
        return 0;
    }
    return ptr;
}


static void unmapFile(EjsFile *fp)
{
#if MACOSX || LINUX || FREEBSD
    munmap(fp->mapped, fp->info.size);
#else
    VirtualFree(file->mapped, 0, MEM_RELEASE);
#endif
}
#endif

static int mapMode(cchar *mode)
{
    int     omode;

    omode = O_BINARY;
    if (strchr(mode, 'r')) {
        omode |= O_RDONLY;
    }
    if (strchr(mode, 'w')) {
        if (omode & O_RDONLY) {
            omode &= ~O_RDONLY;
            omode |= O_RDWR;
        } else {
            omode |= O_CREAT | O_WRONLY | O_TRUNC;
        }
    }
    if (strchr(mode, 'a')) {
        omode |= O_WRONLY | O_APPEND;
        omode &= ~O_TRUNC;
    }
    if (strchr(mode, '+')) {
        /* Append to existing content */
        omode &= ~O_TRUNC;
    }
    if (strchr(mode, 't')) {
        /* Text mode */
        omode &= ~O_BINARY;
    }
#if O_EXLOCK
    if (strchr(mode, 'l')) {
        /* Exclusive lock */
        omode |= O_EXLOCK;
    }
#endif
#if O_SHLOCK
    if (strchr(mode, 's')) {
        /* Shared lock */
        omode |= O_SHLOCK;
    }
#endif
    if (strchr(mode, 'c')) {
        /* Create - must not exist prior */
        omode |= O_CREAT | O_EXCL;
    }
    return omode;
}


/*********************************** Factory **********************************/

EjsFile *ejsCreateFile(Ejs *ejs, cchar *path)
{
    EjsFile     *fp;
    EjsObj      *arg;

    mprAssert(path && *path);

    fp = ejsCreateObj(ejs, ESV(File), 0);
    if (fp == 0) {
        return 0;
    }
    arg = (EjsObj*) ejsCreateStringFromAsc(ejs, path);
    fileConstructor(ejs, fp, 1, (EjsObj**) &arg);
    return fp;
}


EjsFile *ejsCreateFileFromFd(Ejs *ejs, int fd, cchar *name, int mode)
{
    EjsFile     *fp;

    mprAssert(fd >= 0);
    mprAssert(name);

    if ((fp = ejsCreateObj(ejs, ESV(File), 0)) == NULL) {
        return NULL;
    }
    fp->perms = EJS_FILE_PERMS;
    fp->mode = FILE_OPEN;
    if (!(mode & O_WRONLY)) {
        fp->mode |= FILE_READ;
    }
    if (mode & (O_WRONLY | O_RDWR)) {
        fp->mode |= FILE_WRITE;
    }
    if ((fp->file = mprAttachFileFd(fd, name, mode)) == 0) {
        return 0;
    }
    fp->attached = 1;
    fp->path = MPR->emptyString;
    return fp;
}


static void manageFile(void *ptr, int flags)
{
    EjsFile     *fp;

    fp = (EjsFile*) ptr;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(fp->file);
        mprMark(fp->path);
        mprMark(fp->modeString);
        mprMark(TYPE(fp));

    } else if (flags & MPR_MANAGE_FREE) {
        if (fp->file && !fp->attached) {
            closeFile(0, fp, 0, NULL);
        }
    }
}


void ejsConfigureFileType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "File"), sizeof(EjsFile), manageFile,
            EJS_TYPE_OBJ | EJS_TYPE_NUMERIC_INDICIES | EJS_TYPE_VIRTUAL_SLOTS | EJS_TYPE_MUTABLE_INSTANCES)) == 0) {
        return;
    }
    type->helpers.getProperty    = (EjsGetPropertyHelper) getFileProperty;
    type->helpers.lookupProperty = (EjsLookupPropertyHelper) lookupFileProperty;
    type->helpers.setProperty    = (EjsSetPropertyHelper) setFileProperty;

    prototype = type->prototype;
    ejsBindConstructor(ejs, type, fileConstructor);
    ejsBindMethod(ejs, prototype, ES_File_canRead, canReadFile);
    ejsBindMethod(ejs, prototype, ES_File_canWrite, canWriteFile);
    ejsBindMethod(ejs, prototype, ES_File_close, closeFile);
    ejsBindMethod(ejs, prototype, ES_File_iterator_get, getFileIterator);
    ejsBindMethod(ejs, prototype, ES_File_iterator_getValues, getFileValues);
    ejsBindMethod(ejs, prototype, ES_File_isOpen, isFileOpen);
    ejsBindMethod(ejs, prototype, ES_File_open, openFile);
    ejsBindMethod(ejs, prototype, ES_File_options, getFileOptions);
    ejsBindMethod(ejs, prototype, ES_File_path, getFilePath);
    ejsBindAccess(ejs, prototype, ES_File_position, getFilePosition, setFilePosition);
    ejsBindMethod(ejs, prototype, ES_File_readBytes, readFileBytes);
    ejsBindMethod(ejs, prototype, ES_File_readString, readFileString);
    ejsBindMethod(ejs, prototype, ES_File_read, readFile);
    ejsBindMethod(ejs, prototype, ES_File_size, getFileSize);
    ejsBindMethod(ejs, prototype, ES_File_truncate, truncateFile);
    ejsBindMethod(ejs, prototype, ES_File_write, writeFile);
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
