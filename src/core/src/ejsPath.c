/*
    ejsPath.c - Path class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Forwards ********************************/

static char *getPathString(Ejs *ejs, EjsObj *vp);

/************************************ Helpers *********************************/

static EjsObj *castPath(Ejs *ejs, EjsPath *fp, EjsType *type)
{
    if (type->id == ES_String) {
        return (EjsObj*) ejsCreateString(ejs, fp->path);
    }
    return (ejs->objectType->helpers.cast)(ejs, (EjsObj*) fp, type);
}


static EjsPath *clonePath(Ejs *ejs, EjsPath *src, bool deep)
{
    EjsPath     *dest;

    dest = (EjsPath*) ejsCloneObject(ejs, (EjsObj*) src, deep);

    dest->info = src->info;
    dest->path = mprStrdup(dest, src->path);
    return dest;
}


//  TODO - should not need this
static void destroyPath(Ejs *ejs, EjsPath *pp)
{
    mprAssert(pp);

    mprFree(pp->path);
    pp->path = 0;
    ejsFreeVar(ejs, (EjsObj*) pp, -1);
}


static EjsObj *coercePathOperands(Ejs *ejs, EjsPath *lhs, int opcode,  EjsObj *rhs)
{
    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsInvokeOperator(ejs, (EjsObj*) ejsCreateString(ejs, lhs->path), opcode, rhs);

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIsNull(rhs) || ejsIsUndefined(rhs)) {
            return (EjsObj*) ((opcode == EJS_OP_COMPARE_EQ) ? ejs->falseValue: ejs->trueValue);
        }
        return ejsInvokeOperator(ejs, (EjsObj*) ejsCreateString(ejs, lhs->path), opcode, rhs);

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
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %s", opcode, lhs->obj.type->qname.name);
        return ejs->undefinedValue;
    }
    return 0;
}


static EjsObj *invokePathOperator(Ejs *ejs, EjsPath *lhs, int opcode,  EjsPath *rhs, void *data)
{
    EjsObj      *result;

    if (rhs == 0 || lhs->obj.type != rhs->obj.type) {
        if ((result = coercePathOperands(ejs, lhs, opcode, (EjsObj*) rhs)) != 0) {
            return result;
        }
    }

    /*  Types now match, both paths
     */
    switch (opcode) {
    case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_EQ:
        if (lhs == rhs || (lhs->path == rhs->path)) {
            return (EjsObj*) ejs->trueValue;
        }
        return (EjsObj*) ejsCreateBoolean(ejs,  mprStrcmp(lhs->path, rhs->path) == 0);

    case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_STRICTLY_NE:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprStrcmp(lhs->path, rhs->path) != 0);

    case EJS_OP_COMPARE_LT:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprStrcmp(lhs->path, rhs->path) < 0);

    case EJS_OP_COMPARE_LE:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprStrcmp(lhs->path, rhs->path) <= 0);

    case EJS_OP_COMPARE_GT:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprStrcmp(lhs->path, rhs->path) > 0);

    case EJS_OP_COMPARE_GE:
        return (EjsObj*) ejsCreateBoolean(ejs,  mprStrcmp(lhs->path, rhs->path) >= 0);

    /*
        Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return (EjsObj*) ((lhs->path) ? ejs->trueValue: ejs->falseValue);

    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ((lhs->path == 0) ? ejs->trueValue: ejs->falseValue);


    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return (EjsObj*) ejs->falseValue;

    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return (EjsObj*) ejsCreatePath(ejs, mprJoinPath(ejs, lhs->path, rhs->path));

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %s", opcode, lhs->obj.type->qname.name);
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
    fp->path = mprStrdup(fp, path);
    return (EjsObj*) fp;
}


/*
    Return an absolute path name for the file
    function get absolutePath()
 */
static EjsObj *absolutePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathAndFree(ejs, mprGetAbsPath(fp, fp->path));
}


/*
    Get when the file was last accessed.
    function get accessed(): Date
 */
static EjsObj *getAccessedDate(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    mprGetPathInfo(ejs, fp->path, &info);
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
    return (EjsObj*) ejsCreatePathAndFree(ejs, mprGetPathBase(ejs, fp->path));
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

    fs = mprLookupFileSystem(ejs, fp->path);
    ap = ejsCreateArray(ejs, 0);
    index = 0;
    for (last = cp = mprGetAbsPath(fp, fp->path); *cp; cp++) {
        if (*cp == fs->separators[0] || *cp == fs->separators[1]) {
            *cp++ = '\0';
            ejsSetProperty(ejs, ap, index++, ejsCreateString(ejs, last));
            last = cp;
        }
    }
    if (cp > last) {
        ejsSetProperty(ejs, ap, index++, ejsCreateString(ejs, last));
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
    char        *buf, *toPath;
    int         bytes;
    int         rc;

    mprAssert(argc == 1);
    if ((toPath = getPathString(ejs, argv[0])) == 0) {
        return 0;
    }
    from = mprOpen(ejs, fp->path, O_RDONLY | O_BINARY, 0);
    if (from == 0) {
        ejsThrowIOError(ejs, "Cant open %s", fp->path);
        return 0;
    }
    to = mprOpen(ejs, toPath, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, EJS_FILE_PERMS);
    if (to == 0) {
        ejsThrowIOError(ejs, "Cant create %s", toPath);
        mprFree(from);
        return 0;
    }
    if ((buf = mprAlloc(ejs, MPR_BUFSIZE)) == NULL) {
        ejsThrowMemoryError(ejs);
        mprFree(to);
        mprFree(from);
        return 0;
    }

    rc = 0;
    while ((bytes = mprRead(from, buf, MPR_BUFSIZE)) > 0) {
        if (mprWrite(to, buf, bytes) != bytes) {
            ejsThrowIOError(ejs, "Write error to %s", toPath);
            rc = 0;
            break;
        }
    }
    mprFree(from);
    mprFree(to);
    mprFree(buf);
    return 0;
}


/*
    Return when the file was created.
    function get created(): Date
 */
static EjsObj *getCreatedDate(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    mprGetPathInfo(ejs, fp->path, &info);
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
    return (EjsObj*) ejsCreatePathAndFree(ejs, mprGetPathDir(ejs, fp->path));
}


/*
    Test to see if this file exists.
    function get exists(): Boolean
 */
static EjsObj *getPathExists(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    return (EjsObj*) ejsCreateBoolean(ejs, mprGetPathInfo(ejs, fp->path, &info) == 0);
}


/*
    Get the file extension portion of the file name.
    function get extension(): String
 */
static EjsObj *getPathExtension(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    char    *cp;

    if ((cp = strrchr(fp->path, '.')) == 0) {
        return (EjsObj*) ejs->emptyStringValue;
    }
    return (EjsObj*) ejsCreateString(ejs, &cp[1]);
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
    if (ip->index < mprGetListCount(fp->files)) {
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
    mprFree(fp->files);
    fp->files = mprGetPathFiles(fp, fp->path, 0);
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
    if (ip->index < mprGetListCount(fp->files)) {
        dp = (MprDirEntry*) mprGetItem(fp->files, ip->index++);
        return (EjsObj*) ejsCreatePath(ejs, dp->name);
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
    mprFree(fp->files);
    fp->files = mprGetPathFiles(fp, fp->path, 0);
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
    list = mprGetPathFiles(array, fp->path, enumDirs);
    if (list == 0) {
        ejsThrowIOError(ejs, "Can't read directory");
        return 0;
    }
    noPath = (fp->path[0] == '.' && fp->path[1] == '\0') || 
        (fp->path[0] == '.' && fp->path[1] == '/' && fp->path[2] == '\0');

    for (next = 0; (dp = mprGetNextItem(list, &next)) != 0; ) {
        if (strcmp(dp->name, ".") == 0 || strcmp(dp->name, "..") == 0) {
            continue;
        }
        if (enumDirs || !(dp->isDir)) {
            if (noPath) {
                ejsSetProperty(ejs, array, -1, ejsCreatePath(ejs, dp->name));
            } else {
                /*
                    Prepend the directory name
                 */
                path = mprJoinPath(ejs, fp->path, dp->name);
                ejsSetProperty(ejs, array, -1, ejsCreatePathAndFree(ejs, path));
            }
        }
    }
    mprFree(list);
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
        (isalpha((int) fp->path[0]) && fp->path[1] == ':' && (fp->path[2] == '/' || fp->path[2] == '\\')));
}


/*
    function get isAbsolute(): Boolean
 */
static EjsObj *isPathAbsolute(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) (mprIsAbsPath(ejs, fp->path) ? ejs->trueValue: ejs->falseValue);
}


/*
    Determine if the file name is a directory
    function get isDir(): Boolean
 */
static EjsObj *isPathDir(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    int         rc;

    rc = mprGetPathInfo(ejs, fp->path, &info);
    return (EjsObj*) ejsCreateBoolean(ejs, rc == 0 && info.isDir);
}


/*
    function get isLink(): Boolean
 */
static EjsObj *isPathLink(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    int         rc;

    rc = mprGetPathInfo(ejs, fp->path, &info);
    return (EjsObj*) ejsCreateBoolean(ejs, rc == 0 && info.isLink);
}


/*
    Determine if the file name is a regular file
    function get isRegular(): Boolean
 */
static EjsObj *isPathRegular(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    mprGetPathInfo(ejs, fp->path, &info);
    return (EjsObj*) ejsCreateBoolean(ejs, info.isReg);
}


/*
    function get isRelative(): Boolean
 */
static EjsObj *isPathRelative(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) (mprIsRelPath(ejs, fp->path) ? ejs->trueValue: ejs->falseValue);
}


/*
    Join path segments. Returns a normalized path.
    function join(...others): Path
 */
static EjsObj *joinPath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    EjsArray    *args;
    cchar       *other;
    char        *result, *prior;
    int         i;

    args = (EjsArray*) argv[0];
    result = fp->path;
    for (i = 0; i < args->length; i++) {
        if ((other = getPathString(ejs, ejsGetProperty(ejs, (EjsObj*) args, i))) == NULL) {
            return 0;
        }
        prior = result;
        result = mprJoinPath(ejs, prior, other);
        if (prior != fp->path) {
            mprFree(prior);
        }
    }
    return (EjsObj*) ejsCreatePathAndFree(ejs, result);
}


/*
    Join extension
  
    function joinExt(ext: String): Path
 */
static EjsObj *joinPathExt(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *ext;

    //  MOB -- but if the path has "." in the path, then can't easily join an extension
    if (mprGetPathExtension(ejs, fp->path)) {
        return (EjsObj*) fp;
    }
    ext = ejsGetString(ejs, argv[0]);
    while (ext && *ext == '.') {
        ext++;
    }
    return (EjsObj*) ejsCreatePath(ejs, mprStrcat(ejs, -1, fp->path, ".", ext, NULL));
}


/*
    Get the length of the path name.
  
    override function get length(): Number
 */
static EjsObj *pathLength(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (int) strlen(fp->path));
}


static EjsObj *pathLinkTarget(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    char    *path;

    if ((path = mprGetPathLink(ejs, fp->path)) == 0) {
        return (EjsObj*) ejs->nullValue;
    }
    return (EjsObj*) ejsCreatePathAndFree(ejs, mprGetPathLink(ejs, fp->path));
}


/*
    function makeDir(options: Object = null): Void
  
    Options: permissions, owner, group
 */
static EjsObj *makePathDir(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    EjsName     qname;
    EjsObj      *options, *permissions;
#if FUTURE
    EjsObj      *owner, *group;
    cchar       *ownerName, *groupName;
#endif
    int         perms;
    
    perms = 0755;

    if (argc == 1) {
        options = argv[0];

        permissions = ejsGetPropertyByName(ejs, options, ejsName(&qname, EJS_PUBLIC_NAMESPACE, "permissions"));
#if FUTURE
        owner = ejsGetPropertyByName(ejs, options, ejsName(&qname, EJS_PUBLIC_NAMESPACE, "owner"));
        group = ejsGetPropertyByName(ejs, options, ejsName(&qname, EJS_PUBLIC_NAMESPACE, "group"));
#endif
        if (permissions) {
            perms = ejsGetInt(ejs, permissions);
        }
    }
    if (mprGetPathInfo(ejs, fp->path, &info) == 0 && info.isDir) {
        return 0;
    }
    if (mprMakeDir(ejs, fp->path, perms, 1) < 0) {
        ejsThrowIOError(ejs, "Cant create directory %s", fp->path);
        return 0;
    }
#if FUTURE
    if (owner) {
        ownerName = ejsGetString(ejs, owner);
    }
    if (group) {
        groupName = ejsGetString(ejs, group);
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

    target = ((EjsPath*) argv[0])->path;
    hard = (argc >= 2) ? (argv[1] == (EjsObj*) ejs->trueValue) : 0;
    if (mprMakeLink(ejs, fp->path, target, hard) < 0) {
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

    if ((path = mprGetTempPath(ejs, fp->path)) == NULL) {
        ejsThrowIOError(ejs, "Can't make temp file");
        return 0;
    }
    return (EjsObj*) ejsCreatePathAndFree(ejs, path);
}


/*
    function map(separator: String): Path
 */
static EjsObj *pa_map(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *sep;
    char    *path;
    int     separator;

    sep = ejsGetString(ejs, argv[0]);
    separator = *sep ? *sep : '/';
    path = mprStrdup(ejs, fp->path);
    mprMapSeparators(ejs, path, separator);
    return (EjsObj*) ejsCreatePathAndFree(ejs, path);
}


EjsObj *getMimeType(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, mprLookupMimeType(ejs, fp->path));
}


/*
    Get when the file was created or last modified.
  
    function get modified(): Date
 */
static EjsObj *getModifiedDate(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    mprGetPathInfo(ejs, fp->path, &info);
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
    return (EjsObj*) ejsCreateString(ejs, fp->path);
}


/*
    function get natural(): Path
 */
static EjsObj *getNaturalPath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathAndFree(ejs, mprGetNativePath(ejs, fp->path));
}


/*
    function get normalize(): Path
 */
static EjsObj *normalizePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathAndFree(ejs, mprGetNormalizedPath(ejs, fp->path));
}


/*
    Get the parent directory of the absolute path of the file.
  
    function get parent(): String
 */
static EjsObj *getPathParent(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathAndFree(ejs, mprGetPathParent(fp, fp->path));
}


/*
    Get the path permissions
  
    function get perms(): Number
 */
static EjsObj *getPerms(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    if (mprGetPathInfo(ejs, fp->path, &info) < 0) {
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
    if (chmod(fp->path, perms) < 0) {
        ejsThrowIOError(ejs, "Can't update permissions for %s", fp->path);
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
    path = mprGetPortablePath(ejs, fp->path);
    if (lower) {
        mprStrLower(path);
    }
    return (EjsObj*) ejsCreatePathAndFree(ejs, path);
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

    mprAssert(argc == 1 && ejsIsString(argv[0]));
    path = ejsGetString(ejs, argv[0]);

    file = mprOpen(ejs, path, O_RDONLY | O_BINARY, 0);
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
        return 0;
    }

    rc = 0;
    offset = 0;
    while ((bytes = mprRead(file, buffer, MPR_BUFSIZE)) > 0) {
        if (ejsCopyToByteArray(ejs, result, offset, buffer, bytes) < 0) {
            ejsThrowMemoryError(ejs);
            rc = -1;
            break;
        }
        offset += bytes;
    }
    ejsSetByteArrayPositions(ejs, result, 0, offset);

    mprFree(file);
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

    mprAssert(argc == 1 && ejsIsString(argv[0]));
    path = ejsGetString(ejs, argv[0]);

    result = ejsCreateArray(ejs, 0);
    if (result == NULL) {
        ejsThrowMemoryError(ejs);
        return 0;
    }

    file = mprOpen(ejs, path, O_RDONLY | O_BINARY, 0);
    if (file == 0) {
        ejsThrowIOError(ejs, "Can't open %s", path);
        return 0;
    }

    /*
        TODO - need to be smarter about running out of memory here if the file is very large.
     */
    data = mprCreateBuf(ejs, 0, (int) mprGetFileSize(file) + 1);
    result = ejsCreateArray(ejs, 0);
    if (result == NULL || data == NULL) {
        ejsThrowMemoryError(ejs);
        mprFree(file);
        return 0;
    }

    rc = 0;
    while ((bytes = mprRead(file, buffer, MPR_BUFSIZE)) > 0) {
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
            if (ejsSetProperty(ejs, result, lineno++, 
                    ejsCreateStringWithLength(ejs, start, (int) (cp - start))) < 0) {
                break;
            }
            start = cp + 1;
        } else if (*cp == '\r') {
            start = cp + 1;
        }
    }
    if (cp > start) {
        ejsSetProperty(ejs, result, lineno++, ejsCreateStringWithLength(ejs, start, (int) (cp - start)));
    }

    mprFree(file);
    mprFree(data);

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
    EjsObj      *result;
    cchar       *path;
    char        buffer[MPR_BUFSIZE];
    int         bytes, rc;

    mprAssert(argc == 1 && ejsIsString(argv[0]));
    path = ejsGetString(ejs, argv[0]);

    file = mprOpen(ejs, path, O_RDONLY | O_BINARY, 0);
    if (file == 0) {
        ejsThrowIOError(ejs, "Can't open %s", path);
        return 0;
    }

    /*
        TODO - need to be smarter about running out of memory here if the file is very large.
     */
    data = mprCreateBuf(ejs, 0, (int) mprGetFileSize(file) + 1);
    if (data == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }

    rc = 0;
    while ((bytes = mprRead(file, buffer, MPR_BUFSIZE)) > 0) {
        if (mprPutBlockToBuf(data, buffer, bytes) != bytes) {
            ejsThrowMemoryError(ejs);
            rc = -1;
            break;
        }
    }
    result = (EjsObj*) ejsCreateStringWithLength(ejs, mprGetBufStart(data),  mprGetBufLength(data));
    mprFree(file);
    mprFree(data);
    return result;
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
  
    function get relativePath(): Path
 */
static EjsObj *relativePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathAndFree(ejs, mprGetRelPath(fp, fp->path));
}


/*
    Remove the file associated with the File object. This may be a file or directory.
  
    function remove(): void
 */
static EjsObj *removePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    if (mprGetPathInfo(ejs, fp->path, &info) == 0) {
        if (mprDeletePath(ejs, fp->path) < 0) {
            ejsThrowIOError(ejs, "Cant remove %s", fp->path);
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

    mprAssert(argc == 1 && ejsIsString(argv[0]));
    to = ejsGetString(ejs, argv[0]);

    unlink((char*) to);
    if (rename(fp->path, to) < 0) {
        ejsThrowIOError(ejs, "Cant rename file %s to %s", fp->path, to);
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
    cchar       *next;
    char        *result, *prior;
    int         i;

    args = (EjsArray*) argv[0];
    result = fp->path;
    for (i = 0; i < args->length; i++) {
        if ((next = getPathString(ejs, ejsGetProperty(ejs, (EjsObj*) args, i))) == NULL) {
            return 0;
        }
        prior = result;
        result = mprResolvePath(ejs, prior, next);
        if (prior != fp->path) {
            mprFree(prior);
        }
    }
    return (EjsObj*) ejsCreatePath(ejs, result);
}


/*
    Return true if the paths refer to the same file.
  
    function same(other: Object): Boolean
 */
static EjsObj *isPathSame(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *other;

    if (ejsIsString(argv[0])) {
        other = ejsGetString(ejs, argv[0]);
    } else if (ejsIsPath(ejs, argv[0])) {
        other = ((EjsPath*) (argv[0]))->path;
    } else {
        return (EjsObj*) ejs->falseValue;
    }
    return (EjsObj*) (mprSamePath(ejs, fp->path, other) ? ejs->trueValue : ejs->falseValue);
}


/*
    function get separator(): String
 */
static EjsObj *pathSeparator(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFileSystem   *fs;
    cchar           *cp;

    if ((cp = mprGetFirstPathSeparator(ejs, fp->path)) != 0) {
        return (EjsObj*) ejsCreateStringAndFree(ejs, mprAsprintf(ejs, -1, "%c", (int) *cp));
    }
    fs = mprLookupFileSystem(ejs, fp->path);
    return (EjsObj*) ejsCreateStringAndFree(ejs, mprAsprintf(ejs, -1, "%c", (int) fs->separators[0]));
}


/*
    Get the size of the file associated with this Path
  
    function get size(): Number
 */
static EjsObj *getPathFileSize(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    if (mprGetPathInfo(ejs, fp->path, &fp->info) < 0) {
        return (EjsObj*) ejs->minusOneValue;
    }
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) fp->info.size);
}


/*
    override function toJSON(): String
 */
static EjsObj *pathToJSON(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    EjsObj  *result;
    MprBuf  *buf;
    int     i, c, len;

    buf = mprCreateBuf(fp, 0, 0);
    len = (int) strlen(fp->path);
    mprPutCharToBuf(buf, '"');
    for (i = 0; i < len; i++) {
        c = fp->path[i];
        if (c == '"' || c == '\\') {
            mprPutCharToBuf(buf, '\\');
            mprPutCharToBuf(buf, c);
        } else {
            mprPutCharToBuf(buf, c);
        }
    }
    mprPutCharToBuf(buf, '"');
    mprAddNullToBuf(buf);
    result = (EjsObj*) ejsCreateString(ejs, mprGetBufStart(buf));
    mprFree(buf);
    return result;
}


/*
    function toString(): String
 */
static EjsObj *pathToString(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, fp->path);
}


/*
    function trimExt(): Path
 */
static EjsObj *trimExt(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathAndFree(ejs, mprTrimPathExtension(ejs, fp->path));
}


/*
    function truncate(size: Number): Void
 */
static EjsObj *truncatePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    int     size;

    size = ejsGetInt(ejs, argv[0]);
    if (mprTruncatePath(ejs, fp->path, size) < 0) {
        ejsThrowIOError(ejs, "Cant truncate %s", fp->path);
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

    path = ejsGetString(ejs, argv[0]);
    permissions = ejsGetInt(ejs, argv[1]);
    args = (EjsArray*) argv[2];

    /*
        Create fails if already present
     */
    mprDeletePath(ejs, path);
    file = mprOpen(ejs, path, O_CREAT | O_WRONLY | O_BINARY, permissions);
    if (file == 0) {
        ejsThrowIOError(ejs, "Cant create %s", path);
        return 0;
    }

    for (i = 0; i < args->length; i++) {
        data = ejsGetString(ejs, ejsToString(ejs, ejsGetProperty(ejs, (EjsObj*) args, i)));
        length = (int) strlen(data);
        bytes = mprWrite(file, data, length);
        if (bytes != length) {
            ejsThrowIOError(ejs, "Write error to %s", path);
            break;
        }
    }
    mprFree(file);
    return 0;
}
#endif


static char *getPathString(Ejs *ejs, EjsObj *vp)
{
    if (ejsIsString(vp)) {
        return (char*) ejsGetString(ejs, vp);
    } else if (ejsIsPath(ejs, vp)) {
        return ((EjsPath*) vp)->path;
    }
    ejsThrowIOError(ejs, "Bad path");
    return NULL;
}

/*********************************** Factory **********************************/

EjsPath *ejsCreatePath(Ejs *ejs, cchar *path)
{
    EjsPath     *fp;
    EjsObj      *arg;

    fp = (EjsPath*) ejsCreate(ejs, ejs->pathType, 0);
    if (fp == 0) {
        return 0;
    }
    arg = (EjsObj*) ejsCreateString(ejs, path);
    pathConstructor(ejs, fp, 1, (EjsObj**) &arg);
    return fp;
}


EjsPath *ejsCreatePathAndFree(Ejs *ejs, char *value)
{
    EjsPath     *path;

    path = ejsCreatePath(ejs, value);
    mprFree(value);
    return path;
}


void ejsConfigurePathType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *prototype;

    type = ejs->pathType = ejsConfigureNativeType(ejs, EJS_EJS_NAMESPACE, "Path", sizeof(EjsPath));
    ejs->pathType = type;
    prototype = type->prototype;

    type->helpers.cast = (EjsCastHelper) castPath;
    type->helpers.clone = (EjsCloneHelper) clonePath;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokePathOperator;
    type->helpers.destroy = (EjsDestroyHelper) destroyPath;

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
