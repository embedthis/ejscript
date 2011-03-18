/*
    ejsPath.c - Path class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Forwards ********************************/

static cchar *getPathString(Ejs *ejs, EjsObj *vp);

/************************************ Helpers *********************************/

static EjsObj *castPath(Ejs *ejs, EjsPath *fp, EjsType *type)
{
    if (type->id == ES_String) {
        return (EjsObj*) ejsCreateStringFromAsc(ejs, fp->value);
    }
    return (ejs->potHelpers.cast)(ejs, (EjsObj*) fp, type);
}


static EjsPath *clonePath(Ejs *ejs, EjsPath *src, bool deep)
{
    EjsPath     *dest;

    if ((dest = ejsCreateObj(ejs, TYPE(src), 0)) == 0) {
        return 0;
    }
    dest->value = sclone(src->value);
    dest->info = src->info;
    return dest;
}


static EjsObj *coercePathOperands(Ejs *ejs, EjsPath *lhs, int opcode,  EjsObj *rhs)
{
    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsInvokeOperator(ejs, (EjsObj*) ejsCreateStringFromAsc(ejs, lhs->value), opcode, rhs);

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIsNull(ejs, rhs) || ejsIsUndefined(ejs, rhs)) {
            return (EjsObj*) ((opcode == EJS_OP_COMPARE_EQ) ? ejs->falseValue: ejs->trueValue);
        }
        return ejsInvokeOperator(ejs, (EjsObj*) ejsCreateStringFromAsc(ejs, lhs->value), opcode, rhs);

    case EJS_OP_COMPARE_STRICTLY_NE:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return (EjsObj*) ejs->falseValue;

    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_TRUE:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return (EjsObj*) ejs->falseValue;

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return (EjsObj*) ejs->falseValue;

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return ejs->undefinedValue;
    }
    return 0;
}


static EjsObj *invokePathOperator(Ejs *ejs, EjsPath *lhs, int opcode,  EjsPath *rhs, void *data)
{
    EjsObj      *result;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if ((result = coercePathOperands(ejs, lhs, opcode, (EjsObj*) rhs)) != 0) {
            return result;
        }
    }

    /*  Types now match, both paths
     */
    switch (opcode) {
    case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_EQ:
        if (lhs == rhs || (lhs->value == rhs->value)) {
            return (EjsObj*) ejs->trueValue;
        }
        return (EjsObj*) ejsCreateBoolean(ejs,  scmp(lhs->value, rhs->value) == 0);

    case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_STRICTLY_NE:
        return (EjsObj*) ejsCreateBoolean(ejs,  scmp(lhs->value, rhs->value) != 0);

    case EJS_OP_COMPARE_LT:
        return (EjsObj*) ejsCreateBoolean(ejs,  scmp(lhs->value, rhs->value) < 0);

    case EJS_OP_COMPARE_LE:
        return (EjsObj*) ejsCreateBoolean(ejs,  scmp(lhs->value, rhs->value) <= 0);

    case EJS_OP_COMPARE_GT:
        return (EjsObj*) ejsCreateBoolean(ejs,  scmp(lhs->value, rhs->value) > 0);

    case EJS_OP_COMPARE_GE:
        return (EjsObj*) ejsCreateBoolean(ejs,  scmp(lhs->value, rhs->value) >= 0);

    /*
        Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return (EjsObj*) ((lhs->value) ? ejs->trueValue: ejs->falseValue);

    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ((lhs->value == 0) ? ejs->trueValue: ejs->falseValue);


    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return (EjsObj*) ejs->falseValue;

    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return (EjsObj*) ejsCreatePathFromAsc(ejs, mprJoinPath(lhs->value, rhs->value));

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
    mprAssert(0);
}


/************************************ Methods *********************************/
/*
    Constructor
    function Path(path: String)
 */
static EjsObj *pathConstructor(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *path;

    mprAssert(argc == 1);
    if ((path = getPathString(ejs, argv[0])) == 0) {
        return (EjsObj*) fp;
    }
    fp->value = path;
    return (EjsObj*) fp;
}


/*
    Return an absolute path name for the file
    function get absolutePath()
 */
static EjsObj *absolutePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathFromAsc(ejs, mprGetAbsPath(fp->value));
}


/*
    Get when the file was last accessed.
    function get accessed(): Date
 */
static EjsObj *getAccessedDate(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    mprGetPathInfo(fp->value, &info);
    if (!info.valid) {
        return (EjsObj*) ejs->nullValue;
    }
    return (EjsObj*) ejsCreateDate(ejs, ((MprTime) info.atime) * 1000);
}


/*
    Get the base name of a file
    function basename(): Path
 */
static EjsObj *getPathBasename(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathFromAsc(ejs, mprGetPathBase(fp->value));
}


/*
    Get the path components
    function components(): Array
 */
static EjsObj *getPathComponents(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFileSystem   *fs;
    EjsArray        *ap;
    char            *cp, *last;
    int             index;

    fs = mprLookupFileSystem(fp->value);
    ap = ejsCreateArray(ejs, 0);
    index = 0;
    for (last = cp = mprGetAbsPath(fp->value); *cp; cp++) {
        if (*cp == fs->separators[0] || *cp == fs->separators[1]) {
            *cp++ = '\0';
            ejsSetProperty(ejs, ap, index++, ejsCreateStringFromAsc(ejs, last));
            last = cp;
        }
    }
    if (cp > last) {
        ejsSetProperty(ejs, ap, index++, ejsCreateStringFromAsc(ejs, last));
    }
    return (EjsObj*) ap;
}


/*
    Copy a file
    function copy(to: Object, options: Object = null): Void
    TODO - not implementing copy options parameter.
 */
static EjsObj *copyPath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFile     *from, *to;
    cchar       *toPath;
    ssize       bytes;
    char        *buf;

    mprAssert(argc == 1);

    from = to = 0;
    if ((toPath = getPathString(ejs, argv[0])) == 0) {
        return 0;
    }
    from = mprOpenFile(fp->value, O_RDONLY | O_BINARY, 0);
    if (from == 0) {
        ejsThrowIOError(ejs, "Cant open %s", fp->value);
        return 0;
    }
    to = mprOpenFile(toPath, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, EJS_FILE_PERMS);
    if (to == 0) {
        ejsThrowIOError(ejs, "Cant create %s", toPath);
        mprCloseFile(from);
        return 0;
    }
    if ((buf = mprAlloc(MPR_BUFSIZE)) == NULL) {
        ejsThrowMemoryError(ejs);
        mprCloseFile(to);
        mprCloseFile(from);
        return 0;
    }
    while ((bytes = mprReadFile(from, buf, MPR_BUFSIZE)) > 0) {
        if (mprWriteFile(to, buf, bytes) != bytes) {
            ejsThrowIOError(ejs, "Write error to %s", toPath);
            break;
        }
    }
    mprCloseFile(from);
    mprCloseFile(to);
    return 0;
}


/*
    Return when the file was created.
    function get created(): Date
 */
static EjsObj *getCreatedDate(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    mprGetPathInfo(fp->value, &info);
    if (!info.valid) {
        return (EjsObj*) ejs->nullValue;
    }
    return (EjsObj*) ejsCreateDate(ejs, ((MprTime) info.ctime) * 1000);
}


/**
    Get the directory name portion of a file.
    function get dirname(): Path
 */
static EjsObj *getPathDirname(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathFromAsc(ejs, mprGetPathDir(fp->value));
}


/*
    Test to see if this file exists.
    function get exists(): Boolean
 */
static EjsObj *getPathExists(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    return (EjsObj*) ejsCreateBoolean(ejs, mprGetPathInfo(fp->value, &info) == 0);
}


/*
    Get the file extension portion of the file name.
    function get extension(): String
 */
static EjsObj *getPathExtension(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    char    *ext;

    if ((ext = mprGetPathExtension(fp->value)) == 0) {
        return (EjsObj*) ejs->emptyString;
    }
    return (EjsObj*) ejsCreateStringFromAsc(ejs, ext);
}


/*
    Function to iterate and return the next element index.
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsObj *nextPathKey(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsPath     *fp;

    fp = (EjsPath*) ip->target;
    if (!ejsIsPath(ejs, fp)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }
    if (ip->index < mprGetListLength(fp->files)) {
        return (EjsObj*) ejsCreateNumber(ejs, ip->index++);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return the default iterator for use with "for ... in". This will iterate over the files in a directory.
    iterator function get(): Iterator
 */
static EjsObj *getPathIterator(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    fp->files = mprGetPathFiles(fp->value, 0);
    return (EjsObj*) ejsCreateIterator(ejs, (EjsObj*) fp, (EjsProc) nextPathKey, 0, NULL);
}


/*
    Function to iterate and return the next element value.
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsObj *nextPathValue(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsPath     *fp;
    MprDirEntry *dp;

    fp = (EjsPath*) ip->target;
    if (!ejsIsPath(ejs, fp)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }
    if (ip->index < mprGetListLength(fp->files)) {
        dp = (MprDirEntry*) mprGetItem(fp->files, ip->index++);
        return (EjsObj*) ejsCreatePathFromAsc(ejs, dp->name);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return an iterator to enumerate the bytes in the file. For use with "for each ..."
    iterator function getValues(): Iterator
 */
static EjsObj *getPathValues(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    fp->files = mprGetPathFiles(fp->value, 0);
    return (EjsObj*) ejsCreateIterator(ejs, (EjsObj*) fp, (EjsProc) nextPathValue, 0, NULL);
}


/*
    Get the files in a directory.
    function getFiles(enumDirs: Boolean = false): Array

    TODO - need pattern to match (what about "." and ".." and ".*")
    TODO - move this functionality into mprFile (see appweb dirHandler.c)
 */
static EjsObj *getPathFiles(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    EjsArray        *array;
    MprList         *list;
    MprDirEntry     *dp;
    char            *path;
    bool            enumDirs, noPath;
    int             next;

    mprAssert(argc == 0 || argc == 1);
    enumDirs = (argc == 1) ? ejsGetBoolean(ejs, argv[0]): 0;

    array = ejsCreateArray(ejs, 0);
    if (array == 0) {
        return 0;
    }
    list = mprGetPathFiles(fp->value, enumDirs);
    if (list == 0) {
        ejsThrowIOError(ejs, "Can't read directory");
        return 0;
    }
    noPath = (fp->value[0] == '.' && fp->value[1] == '\0') || 
        (fp->value[0] == '.' && fp->value[1] == '/' && fp->value[2] == '\0');

    for (next = 0; (dp = mprGetNextItem(list, &next)) != 0; ) {
        if (strcmp(dp->name, ".") == 0 || strcmp(dp->name, "..") == 0) {
            continue;
        }
        if (enumDirs || !(dp->isDir)) {
            if (noPath) {
                ejsSetProperty(ejs, array, -1, ejsCreatePathFromAsc(ejs, dp->name));
            } else {
                /*
                    Prepend the directory name
                 */
                path = mprJoinPath(fp->value, dp->name);
                ejsSetProperty(ejs, array, -1, ejsCreatePathFromAsc(ejs, path));
            }
        }
    }
    return (EjsObj*) array;
}


#if FUTURE
static EjsObj *fileSystem(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    //  TODO
    return 0;
}
#endif


/*
    Determine if the file path has a drive spec (C:) in the file name
    static function hasDrive(): Boolean
 */
static EjsObj *pathHasDrive(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, 
        (isalpha((int) fp->value[0]) && fp->value[1] == ':' && (fp->value[2] == '/' || fp->value[2] == '\\')));
}


/*
    function get isAbsolute(): Boolean
 */
static EjsObj *isPathAbsolute(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) (mprIsAbsPath(fp->value) ? ejs->trueValue: ejs->falseValue);
}


/*
    Determine if the file name is a directory
    function get isDir(): Boolean
 */
static EjsObj *isPathDir(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    int         rc;

    rc = mprGetPathInfo(fp->value, &info);
    return (EjsObj*) ejsCreateBoolean(ejs, rc == 0 && info.isDir);
}


/*
    function get isLink(): Boolean
 */
static EjsObj *isPathLink(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    int         rc;

    rc = mprGetPathInfo(fp->value, &info);
    return (EjsObj*) ejsCreateBoolean(ejs, rc == 0 && info.isLink);
}


/*
    Determine if the file name is a regular file
    function get isRegular(): Boolean
 */
static EjsObj *isPathRegular(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    mprGetPathInfo(fp->value, &info);
    return (EjsObj*) ejsCreateBoolean(ejs, info.isReg);
}


/*
    function get isRelative(): Boolean
 */
static EjsObj *isPathRelative(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) (mprIsRelPath(fp->value) ? ejs->trueValue: ejs->falseValue);
}


/*
    Join path segments. Returns a normalized path.
    function join(...others): Path
 */
static EjsObj *joinPath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    EjsArray    *args;
    cchar       *other, *result;
    int         i;

    args = (EjsArray*) argv[0];
    result = fp->value;
    for (i = 0; i < args->length; i++) {
        if ((other = getPathString(ejs, ejsGetProperty(ejs, (EjsObj*) args, i))) == NULL) {
            return 0;
        }
        result = mprJoinPath(result, other);
    }
    return (EjsObj*) ejsCreatePathFromAsc(ejs, result);
}


/*
    Join extension
  
    function joinExt(ext: String): Path
 */
static EjsObj *joinPathExt(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *ext;

    //  MOB -- but if the path has "." in the path, then can't easily join an extension
    if (mprGetPathExtension(fp->value)) {
        return (EjsObj*) fp;
    }
    ext = ejsToMulti(ejs, argv[0]);
    while (ext && *ext == '.') {
        ext++;
    }
    return (EjsObj*) ejsCreatePathFromAsc(ejs, sjoin(fp->value, ".", ext, NULL));
}


/*
    Get the length of the path name.
  
    override function get length(): Number
 */
static EjsObj *pathLength(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) strlen(fp->value));
}


static EjsObj *pathLinkTarget(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    char    *path;

    if ((path = mprGetPathLink(fp->value)) == 0) {
        return (EjsObj*) ejs->nullValue;
    }
    return (EjsObj*) ejsCreatePathFromAsc(ejs, mprGetPathLink(fp->value));
}


/*
    function makeDir(options: Object = null): Void
  
    Options: permissions, owner, group
 */
static EjsObj *makePathDir(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    EjsObj      *options, *permissions;
#if FUTURE
    EjsObj      *owner, *group;
    cchar       *ownerName, *groupName;
#endif
    int         perms;
    
    perms = 0755;

    if (argc == 1) {
        options = argv[0];

        permissions = ejsGetPropertyByName(ejs, options, N(EJS_PUBLIC_NAMESPACE, "permissions"));
#if FUTURE
        owner = ejsGetPropertyByName(ejs, options, N(EJS_PUBLIC_NAMESPACE, "owner"));
        group = ejsGetPropertyByName(ejs, options, N(EJS_PUBLIC_NAMESPACE, "group"));
#endif
        if (permissions) {
            perms = ejsGetInt(ejs, permissions);
        }
    }
    if (mprGetPathInfo(fp->value, &info) == 0 && info.isDir) {
        return 0;
    }
    if (mprMakeDir(fp->value, perms, 1) < 0) {
        ejsThrowIOError(ejs, "Cant create directory %s", fp->value);
        return 0;
    }
#if FUTURE
    if (owner) {
        ownerName = ejsToMulti(ejs, owner);
    }
    if (group) {
        groupName = ejsToMulti(ejs, group);
    }
#endif
    return 0;
}


/*
    function makeLink(target: Path, hard: Boolean = false): Void
 */
static EjsObj *makePathLink(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *target;
    int     hard;

    target = ((EjsPath*) argv[0])->value;
    hard = (argc >= 2) ? (argv[1] == (EjsObj*) ejs->trueValue) : 0;
    if (mprMakeLink(fp->value, target, hard) < 0) {
        ejsThrowIOError(ejs, "Can't make link");
    }
    return 0;
}


/*
    Make a temporary file. Creates a new, uniquely named temporary file.
    NOTE: Still the callers responsibility to remove the temp file
  
    function temp(): Path
 */
static EjsObj *pathTemp(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    char    *path;

    if ((path = mprGetTempPath(fp->value)) == NULL) {
        ejsThrowIOError(ejs, "Can't make temp file");
        return 0;
    }
    return (EjsObj*) ejsCreatePathFromAsc(ejs, path);
}


/*
    function map(separator: String): Path
 */
static EjsObj *pa_map(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *sep;
    char    *path;
    int     separator;

    sep = ejsToMulti(ejs, argv[0]);
    separator = *sep ? *sep : '/';
    path = sclone(fp->value);
    mprMapSeparators(path, separator);
    return (EjsObj*) ejsCreatePathFromAsc(ejs, path);
}


EjsObj *getMimeType(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringFromAsc(ejs, mprLookupMime(NULL, fp->value));
}


/*
    Get when the file was created or last modified.
  
    function get modified(): Date
 */
static EjsObj *getModifiedDate(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    mprGetPathInfo(fp->value, &info);
    if (!info.valid) {
        return (EjsObj*) ejs->nullValue;
    }
    return (EjsObj*) ejsCreateDate(ejs, ((MprTime) info.mtime) * 1000);
}


/*
    function get name(): String
 */
static EjsObj *pa_name(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringFromAsc(ejs, fp->value);
}


/*
    function get natural(): Path
 */
static EjsObj *getNaturalPath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathFromAsc(ejs, mprGetNativePath(fp->value));
}


/*
    function get normalize(): Path
 */
static EjsObj *normalizePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathFromAsc(ejs, mprGetNormalizedPath(fp->value));
}


/*
    Get the parent directory of the absolute path of the file.
  
    function get parent(): String
 */
static EjsObj *getPathParent(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathFromAsc(ejs, mprGetPathParent(fp->value));
}


/*
    Get the path permissions
  
    function get perms(): Number
 */
static EjsObj *getPerms(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    if (mprGetPathInfo(fp->value, &info) < 0) {
        return (EjsObj*) ejs->nullValue;
        return 0;
    }
    return (EjsObj*) ejsCreateNumber(ejs, info.perms);
}


/*
    Set the path permissions
  
    function set perms(perms: Number): Void
 */
static EjsObj *setPerms(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
#if !VXWORKS
    int     perms;

    perms = ejsGetInt(ejs, argv[0]);
    if (chmod(fp->value, perms) < 0) {
        ejsThrowIOError(ejs, "Can't update permissions for %s", fp->value);
    }
#endif
    return 0;
}


/*
    Get a portable (unix-like) representation of the path
  
    function get portable(lower: Boolean = false): Path
 */
static EjsObj *getPortablePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    char    *path;
    int     lower;

    lower = (argc >= 1 && argv[0] == (EjsObj*) ejs->trueValue);
    path = mprGetPortablePath(fp->value);
    if (lower) {
        path = slower(path);
    }
    return (EjsObj*) ejsCreatePathFromAsc(ejs, path);
}


#if KEEP
/*
    Get the file contents as a byte array
  
    static function readBytes(path: String): ByteArray
 */
static EjsObj *readBytes(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFile         *file;
    EjsByteArray    *result;
    cchar           *path;
    char            buffer[MPR_BUFSIZE];
    int             bytes, offset, rc;

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));
    path = ejsToMulti(ejs, argv[0]);

    file = mprOpenFile(path, O_RDONLY | O_BINARY, 0);
    if (file == 0) {
        ejsThrowIOError(ejs, "Can't open %s", path);
        return 0;
    }

    /*
        TODO - need to be smarter about running out of memory here if the file is very large.
     */
    result = ejsCreateByteArray(ejs, (int) mprGetFileSize(file));
    if (result == 0) {
        ejsThrowMemoryError(ejs);
        mprCloseFile(file);
        return 0;
    }

    rc = 0;
    offset = 0;
    while ((bytes = mprReadFile(file, buffer, MPR_BUFSIZE)) > 0) {
        if (ejsCopyToByteArray(ejs, result, offset, buffer, bytes) < 0) {
            ejsThrowMemoryError(ejs);
            rc = -1;
            break;
        }
        offset += bytes;
    }
    ejsSetByteArrayPositions(ejs, result, 0, offset);
    mprCloseFile(file);
    return (EjsObj*) result;
}


/**
    Read the file contents as an array of lines.
  
    static function readLines(path: String): Array
 */
static EjsObj *readLines(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFile     *file;
    MprBuf      *data;
    EjsArray    *result;
    cchar       *path;
    char        *start, *end, *cp, buffer[MPR_BUFSIZE];
    int         bytes, rc, lineno;

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));
    path = ejsToMulti(ejs, argv[0]);

    result = ejsCreateArray(ejs, 0);
    if (result == NULL) {
        ejsThrowMemoryError(ejs);
        return 0;
    }

    file = mprOpenFile(path, O_RDONLY | O_BINARY, 0);
    if (file == 0) {
        ejsThrowIOError(ejs, "Can't open %s", path);
        return 0;
    }

    /*
        TODO - need to be smarter about running out of memory here if the file is very large.
     */
    data = mprCreateBuf(0, (int) mprGetFileSize(file) + 1);
    result = ejsCreateArray(ejs, 0);
    if (result == NULL || data == NULL) {
        ejsThrowMemoryError(ejs);
        mprCloseFile(file);
        return 0;
    }

    rc = 0;
    while ((bytes = mprReadFile(file, buffer, MPR_BUFSIZE)) > 0) {
        if (mprPutBlockToBuf(data, buffer, bytes) != bytes) {
            ejsThrowMemoryError(ejs);
            rc = -1;
            break;
        }
    }

    start = mprGetBufStart(data);
    end = mprGetBufEnd(data);
    for (lineno = 0, cp = start; cp < end; cp++) {
        if (*cp == '\n') {
            //  MOB - UNICODE ENCODING
            if (ejsSetProperty(ejs, result, lineno++, ejsCreateStringFromAsc(ejs, start, (int) (cp - start))) < 0) {
                break;
            }
            start = cp + 1;
        } else if (*cp == '\r') {
            start = cp + 1;
        }
    }
    if (cp > start) {
        //  MOB - UNICODE ENCODING
        ejsSetProperty(ejs, result, lineno++, ejsCreateStringFromAsc(ejs, start, (int) (cp - start)));
    }
    mprCloseFile(file);
    return (EjsObj*) result;
}


/*
    Read the file contents as a string
  
    static function readString(path: String): String
 */
static EjsObj *readFileAsString(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFile     *file;
    MprBuf      *data;
    cchar       *path;
    char        buffer[MPR_BUFSIZE];
    int         bytes;

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));
    path = ejsToMulti(ejs, argv[0]);

    file = mprOpenFile(path, O_RDONLY | O_BINARY, 0);
    if (file == 0) {
        ejsThrowIOError(ejs, "Can't open %s", path);
        return 0;
    }

    /*
        TODO - need to be smarter about running out of memory here if the file is very large.
     */
    data = mprCreateBuf(0, (int) mprGetFileSize(file) + 1);
    if (data == 0) {
        ejsThrowMemoryError(ejs);
        mprCloseFile(file);
        return 0;
    }
    while ((bytes = mprReadFile(file, buffer, MPR_BUFSIZE)) > 0) {
        if (mprPutBlockToBuf(data, buffer, bytes) != bytes) {
            ejsThrowMemoryError(ejs);
            break;
        }
    }
    mprCloseFile(file);
    //  MOB - UNICODE ENCODING
    return (EjsObj*) ejsCreateStringFromAsc(ejs, mprGetBufStart(data),  mprGetBufLength(data));
}


/*
    Get the file contents as an XML object
  
    static function readXML(path: String): XML
 */
static EjsObj *readXML(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return 0;
}
#endif


/*
    Return a relative path name for the file.
  
    function get relative(): Path
 */
static EjsObj *relativePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathFromAsc(ejs, mprGetRelPath(fp->value));
}


/*
    Remove the file associated with the File object. This may be a file or directory.
  
    function remove(): void
 */
static EjsObj *removePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    if (mprGetPathInfo(fp->value, &info) == 0) {
        if (mprDeletePath(fp->value) < 0) {
            ejsThrowIOError(ejs, "Cant remove %s", fp->value);
        }
    }
    return 0;
}


/*
    Rename the file
  
    function rename(to: String): Void
 */
static EjsObj *renamePathFile(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar    *to;

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));
    to = ejsToMulti(ejs, argv[0]);
    unlink((char*) to);
    if (rename(fp->value, to) < 0) {
        ejsThrowIOError(ejs, "Cant rename file %s to %s", fp->value, to);
        return 0;
    }
    return 0;
}


/*
    Resolve paths against others. Returns a normalized path.
  
    function resolve(...paths): Path
 */
static EjsObj *resolvePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    EjsArray    *args;
    EjsPath     *result;
    cchar       *next;
    int         i;

    args = (EjsArray*) argv[0];
    result = fp;
    for (i = 0; i < args->length; i++) {
        if ((next = getPathString(ejs, ejsGetProperty(ejs, (EjsObj*) args, i))) == NULL) {
            return 0;
        }
        result = ejsCreatePathFromAsc(ejs, mprResolvePath(result->value, next));
    }
    return (EjsObj*) result;
}


/*
    Return true if the paths refer to the same file.
  
    function same(other: Object): Boolean
 */
static EjsObj *isPathSame(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *other;

    if (ejsIsString(ejs, argv[0])) {
        other = ejsToMulti(ejs, argv[0]);
    } else if (ejsIsPath(ejs, argv[0])) {
        other = ((EjsPath*) (argv[0]))->value;
    } else {
        return (EjsObj*) ejs->falseValue;
    }
    return (EjsObj*) (mprSamePath(fp->value, other) ? ejs->trueValue : ejs->falseValue);
}


/*
    function get separator(): String
 */
static EjsObj *pathSeparator(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFileSystem   *fs;
    cchar           *cp;

    if ((cp = mprGetFirstPathSeparator(fp->value)) != 0) {
        return (EjsObj*) ejsCreateStringFromMulti(ejs, cp, 1);
    }
    fs = mprLookupFileSystem(fp->value);
    return (EjsObj*) ejsCreateStringFromMulti(ejs, fs->separators, 1);
}


/*
    Get the size of the file associated with this Path
  
    function get size(): Number
 */
static EjsObj *getPathFileSize(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    if (mprGetPathInfo(fp->value, &fp->info) < 0) {
        return (EjsObj*) ejs->minusOneValue;
    }
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) fp->info.size);
}


/*
    override function toJSON(): String
 */
static EjsObj *pathToJSON(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprBuf  *buf;
    int     i, c, len;

    buf = mprCreateBuf(0, 0);
    len = (int) strlen(fp->value);
    mprPutCharToBuf(buf, '"');
    for (i = 0; i < len; i++) {
        c = fp->value[i];
        if (c == '"' || c == '\\') {
            mprPutCharToBuf(buf, '\\');
            mprPutCharToBuf(buf, c);
        } else {
            mprPutCharToBuf(buf, c);
        }
    }
    mprPutCharToBuf(buf, '"');
    mprAddNullToBuf(buf);
    return (EjsObj*) ejsCreateStringFromAsc(ejs, mprGetBufStart(buf));
}


/*
    function toString(): String
 */
static EjsObj *pathToString(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringFromAsc(ejs, fp->value);
}


/*
    function trimExt(): Path
 */
static EjsObj *trimExt(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathFromAsc(ejs, mprTrimPathExtension(fp->value));
}


/*
    function truncate(size: Number): Void
 */
static EjsObj *truncatePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    int     size;

    size = ejsGetInt(ejs, argv[0]);
    if (mprTruncateFile(fp->value, size) < 0) {
        ejsThrowIOError(ejs, "Cant truncate %s", fp->value);
    }
    return 0;
}


#if KEEP
/*
    Put the file contents
  
    static function write(path: String, permissions: Number, ...args): void
 */
static EjsObj *writeToFile(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFile     *file;
    EjsArray    *args;
    char        *path, *data;
    int         i, bytes, length, permissions;

    mprAssert(argc == 3);

    path = ejsToMulti(ejs, argv[0]);
    permissions = ejsGetInt(ejs, argv[1]);
    args = (EjsArray*) argv[2];

    /*
        Create fails if already present
     */
    mprDeletePath(path);
    file = mprOpenFile(path, O_CREAT | O_WRONLY | O_BINARY, permissions);
    if (file == 0) {
        ejsThrowIOError(ejs, "Cant create %s", path);
        mprCloseFile(file);
        return 0;
    }

    for (i = 0; i < args->length; i++) {
        data = ejsToMulti(ejs, ejsToString(ejs, ejsGetProperty(ejs, (EjsObj*) args, i)));
        length = (int) strlen(data);
        bytes = mprWriteFile(file, data, length);
        if (bytes != length) {
            ejsThrowIOError(ejs, "Write error to %s", path);
            break;
        }
    }
    mprCloseFile(file);
    return 0;
}
#endif


static cchar *getPathString(Ejs *ejs, EjsObj *vp)
{
    if (ejsIsString(ejs, vp)) {
        return (char*) ejsToMulti(ejs, vp);
    } else if (ejsIsPath(ejs, vp)) {
        return ((EjsPath*) vp)->value;
    }
    ejsThrowIOError(ejs, "Bad path");
    return NULL;
}

/*********************************** Factory **********************************/

//  MOB -- should take EjsString

EjsPath *ejsCreatePath(Ejs *ejs, EjsString *path)
{
    EjsPath     *fp;

    fp = ejsCreateObj(ejs, ejs->pathType, 0);
    if (fp == 0) {
        return 0;
    }
    pathConstructor(ejs, fp, 1, (EjsObj**) &path);
    return fp;
}


EjsPath *ejsCreatePathFromAsc(Ejs *ejs, cchar *value)
{
    return ejsCreatePath(ejs, ejsCreateStringFromAsc(ejs, value));
}


static void managePath(EjsPath *path, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(path->value);
        mprMark(path->files);
    }
}


void ejsConfigurePathType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    type = ejs->pathType = ejsConfigureNativeType(ejs, N("ejs", "Path"), sizeof(EjsPath), managePath, EJS_OBJ_HELPERS);
    ejs->pathType = type;
    prototype = type->prototype;

    type->helpers.cast = (EjsCastHelper) castPath;
    type->helpers.clone = (EjsCloneHelper) clonePath;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokePathOperator;

    //  TODO - rename all and use pa_ prefix
    ejsBindConstructor(ejs, type, (EjsProc) pathConstructor);
    ejsBindMethod(ejs, prototype, ES_Path_absolute, (EjsProc) absolutePath);
    ejsBindMethod(ejs, prototype, ES_Path_accessed, (EjsProc) getAccessedDate);
    ejsBindMethod(ejs, prototype, ES_Path_basename, (EjsProc) getPathBasename);
    ejsBindMethod(ejs, prototype, ES_Path_components, (EjsProc) getPathComponents);
    ejsBindMethod(ejs, prototype, ES_Path_copy, (EjsProc) copyPath);
    ejsBindMethod(ejs, prototype, ES_Path_created, (EjsProc) getCreatedDate);
    ejsBindMethod(ejs, prototype, ES_Path_dirname, (EjsProc) getPathDirname);
    ejsBindMethod(ejs, prototype, ES_Path_exists, (EjsProc) getPathExists);
    ejsBindMethod(ejs, prototype, ES_Path_extension, (EjsProc) getPathExtension);
    ejsBindMethod(ejs, prototype, ES_Path_files, (EjsProc) getPathFiles);
    ejsBindMethod(ejs, prototype, ES_Path_iterator_get, (EjsProc) getPathIterator);
    ejsBindMethod(ejs, prototype, ES_Path_iterator_getValues, (EjsProc) getPathValues);
    ejsBindMethod(ejs, prototype, ES_Path_hasDrive, (EjsProc) pathHasDrive);
    ejsBindMethod(ejs, prototype, ES_Path_isAbsolute, (EjsProc) isPathAbsolute);
    ejsBindMethod(ejs, prototype, ES_Path_isDir, (EjsProc) isPathDir);
    ejsBindMethod(ejs, prototype, ES_Path_isLink, (EjsProc) isPathLink);
    ejsBindMethod(ejs, prototype, ES_Path_isRegular, (EjsProc) isPathRegular);
    ejsBindMethod(ejs, prototype, ES_Path_isRelative, (EjsProc) isPathRelative);
    ejsBindMethod(ejs, prototype, ES_Path_join, (EjsProc) joinPath);
    ejsBindMethod(ejs, prototype, ES_Path_joinExt, (EjsProc) joinPathExt);
    ejsBindMethod(ejs, prototype, ES_Path_length, (EjsProc) pathLength);
    ejsBindMethod(ejs, prototype, ES_Path_linkTarget, (EjsProc) pathLinkTarget);
    ejsBindMethod(ejs, prototype, ES_Path_makeDir, (EjsProc) makePathDir);
    ejsBindMethod(ejs, prototype, ES_Path_makeLink, (EjsProc) makePathLink);
#if ES_Path_temp
    ejsBindMethod(ejs, prototype, ES_Path_temp, (EjsProc) pathTemp);
#endif
    ejsBindMethod(ejs, prototype, ES_Path_map, (EjsProc) pa_map);
    ejsBindAccess(ejs, prototype, ES_Path_mimeType, (EjsProc) getMimeType, NULL);
    ejsBindMethod(ejs, prototype, ES_Path_modified, (EjsProc) getModifiedDate);
    ejsBindMethod(ejs, prototype, ES_Path_name, (EjsProc) pa_name);
    ejsBindMethod(ejs, prototype, ES_Path_natural, (EjsProc) getNaturalPath);
    ejsBindMethod(ejs, prototype, ES_Path_normalize, (EjsProc) normalizePath);
    ejsBindMethod(ejs, prototype, ES_Path_parent, (EjsProc) getPathParent);
    ejsBindAccess(ejs, prototype, ES_Path_perms, (EjsProc) getPerms, (EjsProc) setPerms);
    ejsBindMethod(ejs, prototype, ES_Path_portable, (EjsProc) getPortablePath);
    ejsBindMethod(ejs, prototype, ES_Path_relative, (EjsProc) relativePath);
    ejsBindMethod(ejs, prototype, ES_Path_remove, (EjsProc) removePath);
    ejsBindMethod(ejs, prototype, ES_Path_rename, (EjsProc) renamePathFile);
    ejsBindMethod(ejs, prototype, ES_Path_resolve, (EjsProc) resolvePath);
    ejsBindMethod(ejs, prototype, ES_Path_same, (EjsProc) isPathSame);
    ejsBindMethod(ejs, prototype, ES_Path_separator, (EjsProc) pathSeparator);
    ejsBindMethod(ejs, prototype, ES_Path_size, (EjsProc) getPathFileSize);
    ejsBindMethod(ejs, prototype, ES_Path_toJSON, (EjsProc) pathToJSON);
    ejsBindMethod(ejs, prototype, ES_Path_toString, (EjsProc) pathToString);
    ejsBindMethod(ejs, prototype, ES_Path_trimExt, (EjsProc) trimExt);
    ejsBindMethod(ejs, prototype, ES_Path_truncate, (EjsProc) truncatePath);
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
