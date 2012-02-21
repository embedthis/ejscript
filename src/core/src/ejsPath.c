/*
    ejsPath.c - Path class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"
#include    "pcre.h"

/************************************ Forwards ********************************/

static EjsArray *getPathFiles(Ejs *ejs, EjsArray *results, cchar *dir, int flags, EjsRegExp *exclude, EjsRegExp *include);
static cchar *getPathString(Ejs *ejs, EjsObj *vp);
static void getUserGroup(Ejs *ejs, EjsObj *attributes, int *owner, int *group);
static EjsArray *globPath(Ejs *ejs, EjsArray *results, cchar *path, cchar *base, cchar *pattern, int flags, 
        EjsRegExp *exclude, EjsRegExp *include);
static int globMatch(Ejs *ejs, cchar *s, cchar *pat, int isDir, int flags, cchar *seps, int count, cchar **nextPartPattern);

/************************************ Helpers *********************************/

static EjsAny *castPath(Ejs *ejs, EjsPath *fp, EjsType *type)
{
    if (type->sid == S_String) {
        return ejsCreateStringFromAsc(ejs, fp->value);
    }
    return (ejs->service->potHelpers.cast)(ejs, fp, type);
}


static EjsPath *clonePath(Ejs *ejs, EjsPath *src, bool deep)
{
    return ejsCreatePathFromAsc(ejs, src->value);
}


static EjsAny *coercePathOperands(Ejs *ejs, EjsPath *lhs, int opcode,  EjsAny *rhs)
{
    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsInvokeOperator(ejs, ejsCreateStringFromAsc(ejs, lhs->value), opcode, rhs);

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (!ejsIsDefined(ejs, rhs)) {
            return ((opcode == EJS_OP_COMPARE_EQ) ? ESV(false): ESV(true));
        }
        return ejsInvokeOperator(ejs, ejsCreateStringFromAsc(ejs, lhs->value), opcode, rhs);

    case EJS_OP_COMPARE_STRICTLY_NE:
        return ESV(true);

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return ESV(false);

    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_TRUE:
        return ESV(true);

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return ESV(false);

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return ESV(false);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return ESV(undefined);
    }
    return 0;
}


static EjsAny *invokePathOperator(Ejs *ejs, EjsPath *lhs, int opcode,  EjsPath *rhs, void *data)
{
    EjsAny      *result;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if ((result = coercePathOperands(ejs, lhs, opcode, rhs)) != 0) {
            return result;
        }
    }

    /*  
        Types now match, both paths
     */
    switch (opcode) {
    case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_EQ:
        if (lhs == rhs || (lhs->value == rhs->value)) {
            return ESV(true);
        }
        return ejsCreateBoolean(ejs,  mprSamePath(lhs->value, rhs->value));

    case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_STRICTLY_NE:
        return ejsCreateBoolean(ejs,  !mprSamePath(lhs->value, rhs->value));

    case EJS_OP_COMPARE_LT:
        return ejsCreateBoolean(ejs,  scmp(lhs->value, rhs->value) < 0);

    case EJS_OP_COMPARE_LE:
        return ejsCreateBoolean(ejs,  scmp(lhs->value, rhs->value) <= 0);

    case EJS_OP_COMPARE_GT:
        return ejsCreateBoolean(ejs,  scmp(lhs->value, rhs->value) > 0);

    case EJS_OP_COMPARE_GE:
        return ejsCreateBoolean(ejs,  scmp(lhs->value, rhs->value) >= 0);

    /*
        Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return ((lhs->value) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_ZERO:
        return ((lhs->value == 0) ? ESV(true): ESV(false));


    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return ESV(false);

    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsCreatePathFromAsc(ejs, mprJoinPath(lhs->value, rhs->value));

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
    mprAssert(0);
}


/************************************ Methods *********************************/
/*
    Constructor
    function Path(path: String = ".")
 */
static EjsPath *pathConstructor(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *path;

    if (argc >= 1) {
        if ((path = getPathString(ejs, argv[0])) == 0) {
            return fp;
        }
        fp->value = path;
    } else {
        fp->value = sclone(".");
    }
    return fp;
}


/*
    Return an absolute path name for the file
    function get absolute(): Path
 */
static EjsPath *absolutePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, mprGetAbsPath(fp->value));
}


/*
    Get when the file was last accessed.
    function get accessed(): Date
 */
static EjsDate *getAccessedDate(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    mprGetPathInfo(fp->value, &info);
    if (!info.valid) {
        return ESV(null);
    }
    return ejsCreateDate(ejs, ((MprTime) info.atime) * 1000);
}


/*
    Get file attributes
    function get attributes(): Object
 */
static EjsObj *getAttributes(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    EjsObj      *attributes;

    mprGetPathInfo(fp->value, &info);
    if (!info.valid) {
        return ESV(null);
    }
    attributes = ejsCreateEmptyPot(ejs);
    ejsSetPropertyByName(ejs, attributes, EN("owner"), itos(info.owner));
    ejsSetPropertyByName(ejs, attributes, EN("group"), itos(info.group));
    ejsSetPropertyByName(ejs, attributes, EN("permissions"), sfmt("0%0d", info.perms));
    return attributes;
}


/*
    Set file attributes
    function attributes(options: Object)
 */
static EjsObj *setAttributes(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    EjsObj      *attributes;

    attributes = argv[0];
    mprGetPathInfo(fp->value, &info);
    if (!info.valid) {
        ejsThrowIOError(ejs, "Can't access %s", fp->value);
        return ESV(null);
    }
    ejsSetPathAttributes(ejs, fp->value, attributes);
    return 0;
}


/*
    Get the base name of a file
    function basename(): Path
 */
static EjsPath *getPathBasename(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, mprGetPathBase(fp->value));
}


/*
    Get the path components
    function components(): Array
 */
static EjsArray *getPathComponents(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
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
    return ap;
}


//  MOB - push into MPR
int ejsSetPathAttributes(Ejs *ejs, cchar *path, EjsObj *attributes)
{
    EjsObj  *permissions;
    int     perms;

    if (attributes == 0) {
        return 0;
    }
#if BLD_UNIX_LIKE
{
    int     owner, group;

    getUserGroup(ejs, attributes, &owner, &group);
    if (owner >= 0 || group >= 0) {
        if (chown(path, owner, group) < 0) {
            ejsThrowStateError(ejs, "Can't change group. Error %d", mprGetError());
        }
    }
}
#endif
    if ((permissions = ejsGetPropertyByName(ejs, attributes, EN("permissions"))) != 0) {
        perms = ejsGetInt(ejs, permissions);
        if (chmod(path, perms) < 0) {
            ejsThrowIOError(ejs, "Can't change permissions. Error %d", mprGetError());
        }
    }
    return 0;
}


/*
    Copy a file
    function copy(to: Object, options: Object = null): Void
 */
static EjsObj *copyPath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFile     *from, *to;
    EjsObj      *options;
    cchar       *toPath;
    ssize       bytes;
    char        *buf;

    mprAssert(argc >= 1);
    options = (argc >= 2) ? argv[1] : 0;

    from = to = 0;
    if ((toPath = getPathString(ejs, argv[0])) == 0) {
        return 0;
    }
    from = mprOpenFile(fp->value, O_RDONLY | O_BINARY, 0);
    if (from == 0) {
        ejsThrowIOError(ejs, "Can't open %s", fp->value);
        return 0;
    }
    to = mprOpenFile(toPath, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, EJS_FILE_PERMS);
    if (to == 0) {
        ejsThrowIOError(ejs, "Can't create %s, errno %d", toPath, errno);
        mprCloseFile(from);
        return 0;
    }
    if (options) {
        ejsSetPathAttributes(ejs, toPath, options);
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
static EjsDate *getCreatedDate(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    mprGetPathInfo(fp->value, &info);
    if (!info.valid) {
        return ESV(null);
    }
    return ejsCreateDate(ejs, ((MprTime) info.ctime) * 1000);
}


/**
    Get the directory name portion of a file.
    function get dirname(): Path
 */
static EjsPath *getPathDirname(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (EjsPath*) ejsCreatePathFromAsc(ejs, mprGetPathDir(fp->value));
}


/*
    Test to see if this file exists.
    function get exists(): Boolean
 */
static EjsBoolean *getPathExists(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    return ejsCreateBoolean(ejs, mprGetPathInfo(fp->value, &info) == 0);
}


/*
    Get the file extension portion of the file name.
    function get extension(): String
 */
static EjsString *getPathExtension(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    char    *ext;

    if ((ext = mprGetPathExt(fp->value)) == 0) {
        return ESV(empty);
    }
    return ejsCreateStringFromAsc(ejs, ext);
}


/*
    Function to iterate and return the next element index.
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsAny *nextPathKey(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsPath     *fp;

    fp = (EjsPath*) ip->target;
    if (!ejsIs(ejs, fp, Path)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }
    if (ip->index < mprGetListLength(fp->files)) {
        return ejsCreateNumber(ejs, ip->index++);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return the default iterator for use with "for ... in". This will iterate over the files in a directory.
    iterator function get(): Iterator
 */
static EjsAny *getPathIterator(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    fp->files = mprGetPathFiles(fp->value, 0);
    return ejsCreateIterator(ejs, fp, nextPathKey, 0, NULL);
}


/*
    Function to iterate and return the next element value.
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsAny *nextPathValue(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsPath     *fp;
    MprDirEntry *dp;

    fp = (EjsPath*) ip->target;
    if (!ejsIs(ejs, fp, Path)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }
    if (ip->index < mprGetListLength(fp->files)) {
        dp = (MprDirEntry*) mprGetItem(fp->files, ip->index++);
        return ejsCreatePathFromAsc(ejs, dp->name);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return an iterator to enumerate the bytes in the file. For use with "for each ..."
    iterator function getValues(): Iterator
 */
static EjsAny *getPathValues(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    fp->files = mprGetPathFiles(fp->value, 0);
    return ejsCreateIterator(ejs, fp, nextPathValue, 0, NULL);
}


/*
    Flags for path_files and getPathFiles
 */
#define FILES_DESCEND       MPR_PATH_DESCEND
#define FILES_DEPTH_FIRST   MPR_PATH_DEPTH_FIRST
#define FILES_HIDDEN        MPR_PATH_INC_HIDDEN
#define FILES_NODIRS        MPR_PATH_NODIRS
#define FILES_RELATIVE      MPR_PATH_RELATIVE
#define FILES_MISSING       0x10000
#define FILES_CASELESS      0x20000

/*
    Get the files in a directory and subdirectories
    function files(options: Object = null): Array
 */
static EjsArray *path_files(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    EjsObj      *options;
    EjsRegExp   *exclude, *include;
    int         flags;

    mprAssert(argc == 0 || argc == 1);
    options = (argc == 1) ? argv[0]: 0;
    exclude = include = 0;
    flags = 0;

    if (options) {
        if (ejsGetPropertyByName(ejs, options, EN("descend")) == ESV(true)) {
            flags |= FILES_DESCEND;
        } 
        if (ejsGetPropertyByName(ejs, options, EN("depthFirst")) == ESV(true)) {
            flags |= FILES_DEPTH_FIRST;
        }
        if (ejsGetPropertyByName(ejs, options, EN("hidden")) == ESV(true)) {
            flags |= FILES_HIDDEN;
        }
        if (ejsGetPropertyByName(ejs, options, EN("nodirs")) == ESV(true)) {
            flags |= FILES_NODIRS;
        }
        exclude = ejsGetPropertyByName(ejs, options, EN("exclude"));
        if (exclude && !ejsIs(ejs, exclude, RegExp)) {
            ejsThrowArgError(ejs, "Exclude option must be a regular expression");
            return 0;
        }
        include = ejsGetPropertyByName(ejs, options, EN("include"));
        if (include && !ejsIs(ejs, include, RegExp)) {
            ejsThrowArgError(ejs, "Include option must be a regular expression");
            return 0;
        }
        if (ejsGetPropertyByName(ejs, options, EN("missing")) == ESV(true)) {
            flags |= FILES_MISSING;
        }
        if (ejsGetPropertyByName(ejs, options, EN("relative")) == ESV(true)) {
            flags |= FILES_RELATIVE;
        } 
    }
    return getPathFiles(ejs, ejsCreateArray(ejs, 0), fp->value, flags, exclude, include);
}


static EjsArray *getPathFiles(Ejs *ejs, EjsArray *results, cchar *dir, int flags, EjsRegExp *exclude, EjsRegExp *include)
{
    MprDirEntry *dp;
    MprList     *list;
    cchar       *path;
    int         next, included;

    if ((list = mprGetPathFiles(dir, flags)) == 0) {
        if (flags & FILES_MISSING) {
            ejsThrowIOError(ejs, "Can't read directory");
        }
        return results;
    }
    for (next = 0; (dp = mprGetNextItem(list, &next)) != 0; ) {
        path = dp->name;
        included = 1;
        if (include && pcre_exec(include->compiled, NULL, path, (int) slen(path), 0, 0, NULL, 0) < 0) {
            included = 0;
        }
        if (exclude && pcre_exec(exclude->compiled, NULL, path, (int) slen(path), 0, 0, NULL, 0) >= 0) {
            continue;
        }
        if (included) {
            ejsSetProperty(ejs, results, -1, ejsCreatePathFromAsc(ejs, path));
        }
    }
    return results;
}


/*
    Get the files in a directory and subdirectories
    function glob(pattern: String, options: Object = null): Array
 */
static EjsArray *path_glob(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFileSystem   *fs;
    EjsObj          *options;
    EjsArray        *result, *patterns;
    EjsRegExp       *exclude, *include;
    char            *pattern, *start, *special;
    cchar           *path, *base;
    int             flags, i;

    options = (argc >= 2) ? argv[1]: 0;
    include = exclude = 0;
    result = ejsCreateArray(ejs, 0);
    fs = mprLookupFileSystem(fp->value);
    flags = 0;

    if (options) {
        if (ejsGetPropertyByName(ejs, options, EN("depthFirst")) == ESV(true)) {
            flags |= FILES_DEPTH_FIRST;
        }
        if (ejsGetPropertyByName(ejs, options, EN("hidden")) == ESV(true)) {
            flags |= FILES_HIDDEN;
        }
        exclude = ejsGetPropertyByName(ejs, options, EN("exclude"));
        if (exclude && !ejsIs(ejs, exclude, RegExp)) {
            ejsThrowArgError(ejs, "Exclude option must be a regular expression");
            return 0;
        }
        include = ejsGetPropertyByName(ejs, options, EN("include"));
        if (include && !ejsIs(ejs, include, RegExp)) {
            ejsThrowArgError(ejs, "Include option must be a regular expression");
            return 0;
        }
        if (ejsGetPropertyByName(ejs, options, EN("missing")) == ESV(true)) {
            flags |= FILES_MISSING;
        }
        if (ejsGetPropertyByName(ejs, options, EN("relative")) == ESV(true)) {
            flags |= FILES_RELATIVE;
        } 
    }
    if (!ejsIs(ejs, argv[0], Array)) {
        patterns = ejsCreateArray(ejs, 0);
        ejsAddItem(ejs, patterns, ejsToString(ejs, argv[0]));
    } else {
        patterns = (EjsArray*) argv[0];
    }

    for (i = 0; i < patterns->length; i++) {
        /* 
            Optimize by converting absolute pattern path prefixes into the base directory.
            This allows path.glob('/path/ *')
         */
        pattern = ejsToMulti(ejs, ejsGetItem(ejs, patterns, i));
        path = fp->value;
        base = "";
        if (mprIsPathAbs(pattern)) {
            start = pattern;
            if ((special = strpbrk(start, "*?")) != 0) {
                if (special > start) {
                    for (pattern = special; pattern > start && !strchr(fs->separators, *pattern); pattern--) { }
                    if (pattern > start) {
                        *pattern++ = '\0';
                        path = mprJoinPath(path, start);
                        base = start;
                    }
                }
            } else {
                pattern = (char*) mprGetPathBaseRef(start);
                if (pattern > start) {
                    pattern[-1] = '\0';
                    path = mprJoinPath(path, start);
                    base = start;
                }
            }
        }
        if (!globPath(ejs, result, path, base, pattern, flags, exclude, include)) {
            return 0;
        }
    }
    return result;
}


/*
    Match a string against a pattern using glob style matching.
    Pat may contain a fully path of patterns. Only the first portion up to a file separator is used. The remaining portion
        is returned in nextPartPattern.
    seps contains the file system separator characters

    Wildcard Patterns:
    ?           Matches any single character
    *           Matches zero or more characters of the file or directory
    ** /        Matches zero or more directories (spaces introduced to avoid closing comment)
    **          Matches zero or more files or directories. Equivlane to ** / *
    trailing/   Trailing slash matches only directory
 */
static int globMatch(Ejs *ejs, cchar *s, cchar *pat, int isDir, int flags, cchar *seps, int count, cchar **nextPartPattern)
{
    int     match;
//  MOB - need recursion limits
    *nextPartPattern = 0;

    while (*s && *pat && *pat != seps[0] && *pat != seps[1]) {
        match = (flags & FILES_CASELESS) ? (*pat == *s) : (tolower((int) *pat) == tolower((int) *s));
        if (match || *pat == '?') {
            ++pat; ++s;
        } else if (*pat == '*') {
            if (*++pat == '\0') {
                /* Terminal star matches files and directories */
                return 1;
            }
            if (*pat == '*') {
                /* Double star - matches zero or more directories */
                if (isDir) {
                    *nextPartPattern = pat - 1;
                    return 1;
                }
                if (pat[1] && (pat[1] == seps[0] || pat[1] == seps[1])) {
                    /* Double star/ */
                    if (pat[2] == '\0') {
                        /* Trailing slash and not a directory */
                        return 0;
                    }
                    pat += 2;
                } else {
                    /* Plain double star matches all (alias for ** / *) */
                    if (pat[1] == '\0') {
                        *nextPartPattern = pat - 1;
                        return 1;
                    }
                }
            } else {
                /* Single star */
                if (count > 2000) {
                    ejsThrowArgError(ejs, "Glob match is too recursive");
                    return 0;
                }
                if (*pat == seps[0] || *pat == seps[1]) {
                    s = "";
                    break;
                }
                while (*s) {
                    if (globMatch(ejs, s++, pat, isDir, flags, seps, count + 1, nextPartPattern)) {
                        return 1;
                    }
                }
                return 0;
            }
        } else {
            return 0;
        }
    }
    if (*pat == '*') {
        ++pat;
    }
    if (*s) {
        return 0;
    }
    if (*pat == '\0') {
        return 1;
    }
    if (*pat && (*pat == seps[0] || *pat == seps[1])) {
        if (*++pat == '\0') {
            /* Terminal / matches only directories */
            return isDir;
        }
        *nextPartPattern = pat;
        return 1;
    }
    return 0;
}


static EjsArray *globPath(Ejs *ejs, EjsArray *results, cchar *path, cchar *base, cchar *pattern, int flags, 
        EjsRegExp *exclude, EjsRegExp *include)
{
    MprFileSystem   *fs;
    MprDirEntry     *dp;
    MprList         *list;
    cchar           *filename, *nextPartPattern, *nextPath, *matchFile;
    int             next, add;

    if ((list = mprGetPathFiles(path, flags | MPR_PATH_RELATIVE)) == 0) {
        if (flags & FILES_MISSING) {
            ejsThrowIOError(ejs, "Can't read directory");
            return 0;
        }
        return results;
    }
    fs = mprLookupFileSystem(path);

    for (next = 0; (dp = mprGetNextItem(list, &next)) != 0; ) {
        if (!globMatch(ejs, dp->name, pattern, dp->isDir, flags, fs->separators, 0, &nextPartPattern)) {
            continue;
        }
        add = 1;
        //  MOB - OPT
        if (nextPartPattern && strcmp(nextPartPattern, "**") != 0 && strcmp(nextPartPattern, "**/") != 0
                   && strcmp(nextPartPattern, "**/*") != 0) {
            /* Double star matches zero or more */
            add = 0;
        }
        filename = (flags & MPR_PATH_RELATIVE) ? mprJoinPath(base, dp->name) : mprJoinPath(path, dp->name);
        if (add && (include || exclude)) {
            matchFile = dp->isDir ? sfmt("%s%c", filename, fs->separators[0]) : filename;
            if (include && pcre_exec(include->compiled, NULL, matchFile, (int) slen(matchFile), 0, 0, NULL, 0) < 0) {
                add = 0;
            }
            if (exclude && pcre_exec(exclude->compiled, NULL, matchFile, (int) slen(matchFile), 0, 0, NULL, 0) >= 0) {
                continue;
            }
        }
        if (!(flags & FILES_DEPTH_FIRST) && add) {
            /* Exclude mid-pattern directories and terminal directories if only "files" */
            ejsSetProperty(ejs, results, -1, ejsCreatePathFromAsc(ejs, filename));
        }
        if (dp->isDir && nextPartPattern) {
            nextPath = (flags & MPR_PATH_RELATIVE) ? mprJoinPath(path, dp->name) : filename;
            globPath(ejs, results, nextPath, filename, nextPartPattern, flags, exclude, include);
        }
        if ((flags & FILES_DEPTH_FIRST) && add) {
            ejsSetProperty(ejs, results, -1, ejsCreatePathFromAsc(ejs, filename));
        }
    }
    return results;
}


/*
    Determine if the file path has a drive spec (C:) in the file name
    static function hasDrive(): Boolean
 */
static EjsBoolean *pathHasDrive(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, 
        (isalpha((int) fp->value[0]) && fp->value[1] == ':' && (fp->value[2] == '/' || fp->value[2] == '\\')));
}


/*
    function get isAbsolute(): Boolean
 */
static EjsBoolean *isPathAbsolute(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (mprIsPathAbs(fp->value) ? ESV(true): ESV(false));
}


/*
    Determine if the file name is a directory
    function get isDir(): Boolean
 */
static EjsBoolean *isPathDir(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    int         rc;

    rc = mprGetPathInfo(fp->value, &info);
    return ejsCreateBoolean(ejs, rc == 0 && info.isDir);
}


/*
    function get isLink(): Boolean
 */
static EjsBoolean *isPathLink(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    //  MOB -work around. GetPathInfo will return err if the target of the symlink does not exist.
    info.isLink = 0;
    mprGetPathInfo(fp->value, &info);
    return ejsCreateBoolean(ejs, info.isLink);
}


/*
    Determine if the file name is a regular file
    function get isRegular(): Boolean
 */
static EjsBoolean *isPathRegular(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    mprGetPathInfo(fp->value, &info);
    return ejsCreateBoolean(ejs, info.isReg);
}


/*
    function get isRelative(): Boolean
 */
static EjsBoolean *isPathRelative(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return (mprIsPathRel(fp->value) ? ESV(true): ESV(false));
}


/*
    Join path segments. Returns a normalized path.
    function join(...others): Path
 */
static EjsPath *joinPath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    EjsArray    *args;
    cchar       *other, *result;
    int         i;

    args = (EjsArray*) argv[0];
    result = fp->value;
    for (i = 0; i < args->length; i++) {
        if ((other = getPathString(ejs, ejsGetProperty(ejs, args, i))) == NULL) {
            return 0;
        }
        result = mprJoinPath(result, other);
    }
    return ejsCreatePathFromAsc(ejs, result);
}


/*
    Join extension
  
    function joinExt(ext: String): Path
 */
static EjsPath *joinPathExt(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *ext;

    if (mprGetPathExt(fp->value)) {
        return fp;
    }
    ext = ejsToMulti(ejs, argv[0]);
    while (ext && *ext == '.') {
        ext++;
    }
    if (ext && *ext) {
        return ejsCreatePathFromAsc(ejs, sjoin(fp->value, ".", ext, NULL));
    }
    //  MOB - should this clone?
    return fp;
}


/*
    Get the length of the path name.
  
    override function get length(): Number
 */
static EjsNumber *pathLength(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) strlen(fp->value));
}


/*
    function get linkTarget(): Path
 */
static EjsPath *pathLinkTarget(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    char    *path;

    if ((path = mprGetPathLink(fp->value)) == 0) {
        return ESV(null);
    }
    return ejsCreatePathFromAsc(ejs, mprGetPathLink(fp->value));
}


static void getUserGroup(Ejs *ejs, EjsObj *attributes, int *owner, int *group)
{
#if BLD_UNIX_LIKE
    EjsAny          *ownerName, *groupName;
    struct passwd   *pp;
    struct group    *gp;

    *owner = *group = -1;
    if ((groupName = ejsGetPropertyByName(ejs, attributes, EN("group"))) != 0) {
        if (ejsIs(ejs, groupName, Number)) {
            *group = ejsGetInt(ejs, groupName);
        } else {
            groupName = ejsToString(ejs, groupName);
            if ((gp = getgrnam(ejsToMulti(ejs, groupName))) == 0) {
                ejsThrowArgError(ejs, "Can't find group %@", groupName);
                return;
            }
            *group = gp->gr_gid;
        }
    }
    if ((ownerName = ejsGetPropertyByName(ejs, attributes, EN("owner"))) != 0) {
        if (ejsIs(ejs, ownerName, Number)) {
            *owner = ejsGetInt(ejs, ownerName);
        } else {
            if ((pp = getpwnam(ejsToMulti(ejs, ownerName))) == 0) {
                ejsThrowArgError(ejs, "Can't find user %@", ownerName);
                return;
            }
            *owner = pp->pw_uid;
        }
    }
#else
    *owner = *group = -1;
#endif
}


/*
    function makeDir(attributes: Object = null): Void
  
    Options: permissions, owner, group
 */
static EjsObj *makePathDir(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    EjsObj      *attributes, *permissions;
    int         rc, perms, owner, group;
    
    attributes = (argc >= 1) ? argv[0] : 0;
    perms = 0755;
    group = owner = -1;
    if (argc == 1) {
        getUserGroup(ejs, attributes, &owner, &group);
        if ((permissions = ejsGetPropertyByName(ejs, attributes, EN("permissions"))) != 0) {
            perms = ejsGetInt(ejs, permissions);
        }
    }
    if (mprGetPathInfo(fp->value, &info) < 0) {
        if ((rc = mprMakeDir(fp->value, perms, owner, group, 1)) < 0) {
            if (rc == MPR_ERR_CANT_COMPLETE) {
                ejsThrowStateError(ejs, "Can't set directory permissions. Error %d", mprGetError());
            } else {
                ejsThrowStateError(ejs, "Can't make directory. Error %d", mprGetError());
            }
            return ESV(false);
        }
    } else if (!info.isDir) {
        /* Not a directory */
        return ESV(false);
    }
    ejsSetPathAttributes(ejs, fp->value, attributes);
    return ESV(true);
}


/*
    function makeLink(target: Path, hard: Boolean = false): Void
 */
static EjsObj *makePathLink(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *target;
    int     hard;

    target = ((EjsPath*) argv[0])->value;
    hard = (argc >= 2) ? (argv[1] == ESV(true)) : 0;
    if (mprMakeLink(fp->value, target, hard) < 0) {
        ejsThrowIOError(ejs, "Can't make link");
    }
    return 0;
}


/*
    Make a temporary file. Creates a new, uniquely named temporary file. The path object specifies the directory
    to contain the temp file.
    NOTE: Still the callers responsibility to remove the temp file
  
    function temp(): Path
 */
static EjsPath *pathTemp(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    char    *path;

    if ((path = mprGetTempPath(fp->value)) == NULL) {
        ejsThrowIOError(ejs, "Can't make temp file");
        return 0;
    }
    return ejsCreatePathFromAsc(ejs, path);
}


/*
    function map(separator: String): Path
 */
static EjsPath *pa_map(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *sep;
    char    *path;
    int     separator;

    sep = (argc >= 1) ? ejsToMulti(ejs, argv[0]) : "/";
    separator = *sep;
    path = sclone(fp->value);
    mprMapSeparators(path, separator);
    return ejsCreatePathFromAsc(ejs, path);
}


/*
    function get mimeType(): String
 */
static EjsString *getMimeType(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, mprLookupMime(NULL, fp->value));
}


/*
    Get when the file was created or last modified.
  
    function get modified(): Date
 */
static EjsDate *getModifiedDate(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    mprGetPathInfo(fp->value, &info);
    if (!info.valid) {
        return ESV(null);
    }
    return ejsCreateDate(ejs, ((MprTime) info.mtime) * 1000);
}


/*
    function get name(): String
 */
static EjsString *pa_name(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, fp->value);
}


/*
    function get natural(): Path
 */
static EjsPath *getNaturalPath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, mprGetNativePath(fp->value));
}


/*
    function get normalize(): Path
 */
static EjsPath *normalizePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, mprNormalizePath(fp->value));
}


/*
    Get the parent directory of the absolute path of the file.
  
    function get parent(): Path
 */
static EjsPath *getPathParent(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, mprGetPathParent(fp->value));
}


/*
    Get the path permissions
  
    function get perms(): Number
 */
static EjsNumber *getPerms(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    if (mprGetPathInfo(fp->value, &info) < 0) {
        return ESV(null);
    }
    return ejsCreateNumber(ejs, info.perms);
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
static EjsPath *getPortablePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    char    *path;
    int     lower;

    lower = (argc >= 1 && argv[0] == ESV(true));
    path = mprGetPortablePath(fp->value);
    if (lower) {
        path = slower(path);
    }
    return ejsCreatePathFromAsc(ejs, path);
}


#if KEEP
/*
    Get the file contents as a byte array
  
    static function readBytes(path: String): ByteArray
 */
static EjsByteArray *readBytes(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFile         *file;
    EjsByteArray    *result;
    cchar           *path;
    char            buffer[MPR_BUFSIZE];
    int             bytes, offset, rc;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], String));
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
        //  MOB - should use RC Value (== bytes)
        if (ejsCopyToByteArray(ejs, result, offset, buffer, bytes) < 0) {
            ejsThrowMemoryError(ejs);
            rc = -1;
            break;
        }
        offset += bytes;
    }
    ejsSetByteArrayPositions(ejs, result, 0, offset);
    mprCloseFile(file);
    return result;
}


/**
    Read the file contents as an array of lines.
  
    static function readLines(path: String): Array
 */
static EjsArray *readLines(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFile     *file;
    MprBuf      *data;
    EjsArray    *result;
    cchar       *path;
    char        *start, *end, *cp, buffer[MPR_BUFSIZE];
    int         bytes, rc, lineno;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], String));
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
            //  TODO - UNICODE ENCODING
            if (ejsSetProperty(ejs, result, lineno++, ejsCreateStringFromAsc(ejs, start, (int) (cp - start))) < 0) {
                break;
            }
            start = cp + 1;
        } else if (*cp == '\r') {
            start = cp + 1;
        }
    }
    if (cp > start) {
        //  TODO - UNICODE ENCODING
        ejsSetProperty(ejs, result, lineno++, ejsCreateStringFromAsc(ejs, start, (int) (cp - start)));
    }
    mprCloseFile(file);
    return result;
}


/*
    Read the file contents as a string
  
    static function readString(path: String): String
 */
static EjsString *readFileAsString(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFile     *file;
    MprBuf      *data;
    cchar       *path;
    char        buffer[MPR_BUFSIZE];
    int         bytes;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], String));
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
    //  TODO - UNICODE ENCODING
    return ejsCreateStringFromAsc(ejs, mprGetBufStart(data),  mprGetBufLength(data));
}


/*
    Get the file contents as an XML object
  
    static function readXML(path: String): XML
 */
static EjsXML *readXML(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return 0;
}
#endif


/*
    Return a relative path name for the file.
  
    function get relative(): Path
 */
static EjsPath *relativePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, mprGetRelPath(fp->value, 0));
}


/*
    Return a relative path name for the file from the given origin
  
    function relativeTo(origin: Path = null): Path
 */
static EjsPath *relativeToPath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *origin;

    origin = (argc >= 1) ? ((EjsPath*) argv[0])->value : 0;
    return ejsCreatePathFromAsc(ejs, mprGetRelPath(fp->value, origin));
}


/*
    Remove the file associated with the File object. This may be a file or directory.
  
    function remove(): Boolean
 */
static EjsObj *removePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprPath     info;

    //  MOB - workaround for isLink
    info.isLink = 0;
    if (mprGetPathInfo(fp->value, &info) == 0 || info.isLink == 1) {
        if (mprDeletePath(fp->value) < 0) {
            return ESV(false);
        }
    }
    return ESV(true);
}


/*
    Rename the file
  
    function rename(to: Path): Boolean
 */
static EjsObj *renamePathFile(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    EjsPath     *to;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Path));
    to = (EjsPath*) argv[0];
    unlink((char*) to->value);
    if (rename(fp->value, to->value) < 0) {
        return ESV(false);
    }
    return ESV(true);
}


/*
    Resolve paths against others. Returns a normalized path.
  
    function resolve(...paths): Path
 */
static EjsPath *resolvePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    EjsArray    *args;
    EjsPath     *result;
    cchar       *next;
    int         i;

    args = (EjsArray*) argv[0];
    result = fp;
    for (i = 0; i < args->length; i++) {
        if ((next = getPathString(ejs, ejsGetProperty(ejs, args, i))) == NULL) {
            return 0;
        }
        result = ejsCreatePathFromAsc(ejs, mprResolvePath(result->value, next));
    }
    return result;
}


/*
    Return the root directory component
  
    function root(other: Object): Boolean
 */
static EjsPath *rootPath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFileSystem   *fs;
    cchar           *cp;

    fs = mprLookupFileSystem(fp->value);
    fp = ejsCreatePathFromAsc(ejs, mprGetAbsPath(fp->value));
    if ((cp = strpbrk(fp->value, fs->separators)) != 0) {
        return ejsCreatePathFromAsc(ejs, snclone(fp->value, cp - fp->value + 1));
    }
    return fp;
}


/*
    Return true if the paths refer to the same file.
  
    function same(other: Object): Boolean
 */
static EjsBoolean *isPathSame(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    cchar   *other;
    int     rc;

    if (ejsIs(ejs, argv[0], String)) {
        other = ejsToMulti(ejs, argv[0]);
    } else if (ejsIs(ejs, argv[0], Path)) {
        other = ((EjsPath*) (argv[0]))->value;
    } else {
        return ESV(false);
    }
    rc = mprSamePath(fp->value, other);
    if (rc) {
        return ESV(true);
    }
    return ESV(false);
    // return (mprSamePath(fp->value, other) ? ESV(true) : ESV(false));
}


/*
    function get separator(): String
 */
static EjsString *pathSeparator(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    MprFileSystem   *fs;
    cchar           *cp;

    if ((cp = mprGetFirstPathSeparator(fp->value)) != 0) {
        return ejsCreateStringFromMulti(ejs, cp, 1);
    }
    fs = mprLookupFileSystem(fp->value);
    return ejsCreateStringFromMulti(ejs, fs->separators, 1);
}


/*
    Get the size of the file associated with this Path
  
    function get size(): Number
 */
static EjsNumber *getPathFileSize(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    if (mprGetPathInfo(fp->value, &fp->info) < 0) {
        return ESV(minusOne);
    }
    return ejsCreateNumber(ejs, (MprNumber) fp->info.size);
}


/*
    override function toJSON(): String
 */
static EjsString *pathToJSON(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
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
    return ejsCreateStringFromAsc(ejs, mprGetBufStart(buf));
}


/*
    function toString(): String
 */
static EjsString *pathToString(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, fp->value);
}


/*
    function trimExt(): Path
 */
static EjsPath *trimExt(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, mprTrimPathExt(fp->value));
}


/*
    function truncate(size: Number): Void
 */
static EjsObj *truncatePath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    int     size;

    size = ejsGetInt(ejs, argv[0]);
    if (mprTruncateFile(fp->value, size) < 0) {
        ejsThrowIOError(ejs, "Can't truncate %s", fp->value);
    }
    return 0;
}


/*
    Return a windows path name for the path on Windows|Cygwin systems. Otherwise returns and absolute path.
    function get absolute(): Path
 */
static EjsPath *windowsPath(Ejs *ejs, EjsPath *fp, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, mprGetWinPath(fp->value));
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
        ejsThrowIOError(ejs, "Can't create %s", path);
        mprCloseFile(file);
        return 0;
    }

    for (i = 0; i < args->length; i++) {
        data = ejsToMulti(ejs, ejsToString(ejs, ejsGetProperty(ejs, args, i)));
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
    if (ejsIs(ejs, vp, String)) {
        return (char*) ejsToMulti(ejs, vp);
    } else if (ejsIs(ejs, vp, Path)) {
        return ((EjsPath*) vp)->value;
    }
    ejsThrowIOError(ejs, "Bad path");
    return NULL;
}

/*********************************** Factory **********************************/

EjsPath *ejsCreatePath(Ejs *ejs, EjsString *path)
{
    EjsPath     *fp;

    if ((fp = ejsCreateObj(ejs, ESV(Path), 0)) == 0) {
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


void ejsCreatePathType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsCreateCoreType(ejs, N("ejs", "Path"), sizeof(EjsPath), S_Path, ES_Path_NUM_CLASS_PROP, managePath, 
        EJS_TYPE_OBJ | EJS_TYPE_IMMUTABLE_INSTANCES);
    type->helpers.cast = (EjsCastHelper) castPath;
    type->helpers.clone = (EjsCloneHelper) clonePath;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokePathOperator;
}


void ejsConfigurePathType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "Path"))) == 0) {
        return;
    }
    //  TODO - rename all and use pa_ prefix
    prototype = type->prototype;
    ejsBindConstructor(ejs, type, pathConstructor);
    ejsBindMethod(ejs, prototype, ES_Path_absolute, absolutePath);
    ejsBindMethod(ejs, prototype, ES_Path_accessed, getAccessedDate);
    ejsBindAccess(ejs, prototype, ES_Path_attributes, getAttributes, setAttributes);
    ejsBindMethod(ejs, prototype, ES_Path_basename, getPathBasename);
    ejsBindMethod(ejs, prototype, ES_Path_components, getPathComponents);
    ejsBindMethod(ejs, prototype, ES_Path_copy, copyPath);
    ejsBindMethod(ejs, prototype, ES_Path_created, getCreatedDate);
    ejsBindMethod(ejs, prototype, ES_Path_dirname, getPathDirname);
    ejsBindMethod(ejs, prototype, ES_Path_exists, getPathExists);
    ejsBindMethod(ejs, prototype, ES_Path_extension, getPathExtension);
    ejsBindMethod(ejs, prototype, ES_Path_files, path_files);
    ejsBindMethod(ejs, prototype, ES_Path_glob, path_glob);
    ejsBindMethod(ejs, prototype, ES_Path_iterator_get, getPathIterator);
    ejsBindMethod(ejs, prototype, ES_Path_iterator_getValues, getPathValues);
    ejsBindMethod(ejs, prototype, ES_Path_hasDrive, pathHasDrive);
    ejsBindMethod(ejs, prototype, ES_Path_isAbsolute, isPathAbsolute);
    ejsBindMethod(ejs, prototype, ES_Path_isDir, isPathDir);
    ejsBindMethod(ejs, prototype, ES_Path_isLink, isPathLink);
    ejsBindMethod(ejs, prototype, ES_Path_isRegular, isPathRegular);
    ejsBindMethod(ejs, prototype, ES_Path_isRelative, isPathRelative);
    ejsBindMethod(ejs, prototype, ES_Path_join, joinPath);
    ejsBindMethod(ejs, prototype, ES_Path_joinExt, joinPathExt);
    ejsBindMethod(ejs, prototype, ES_Path_length, pathLength);
    ejsBindMethod(ejs, prototype, ES_Path_linkTarget, pathLinkTarget);
    ejsBindMethod(ejs, prototype, ES_Path_makeDir, makePathDir);
    ejsBindMethod(ejs, prototype, ES_Path_makeLink, makePathLink);
    ejsBindMethod(ejs, prototype, ES_Path_temp, pathTemp);
    ejsBindMethod(ejs, prototype, ES_Path_map, pa_map);
    ejsBindAccess(ejs, prototype, ES_Path_mimeType, getMimeType, NULL);
    ejsBindMethod(ejs, prototype, ES_Path_modified, getModifiedDate);
    ejsBindMethod(ejs, prototype, ES_Path_name, pa_name);
    ejsBindMethod(ejs, prototype, ES_Path_natural, getNaturalPath);
    ejsBindMethod(ejs, prototype, ES_Path_normalize, normalizePath);
    ejsBindMethod(ejs, prototype, ES_Path_parent, getPathParent);
    ejsBindAccess(ejs, prototype, ES_Path_perms, getPerms, setPerms);
    ejsBindMethod(ejs, prototype, ES_Path_portable, getPortablePath);
    ejsBindMethod(ejs, prototype, ES_Path_relative, relativePath);
    ejsBindMethod(ejs, prototype, ES_Path_relativeTo, relativeToPath);
    ejsBindMethod(ejs, prototype, ES_Path_remove, removePath);
    ejsBindMethod(ejs, prototype, ES_Path_rename, renamePathFile);
    ejsBindMethod(ejs, prototype, ES_Path_resolve, resolvePath);
#if ES_Path_root
    ejsBindMethod(ejs, prototype, ES_Path_root, rootPath);
#endif
    ejsBindMethod(ejs, prototype, ES_Path_same, isPathSame);
    ejsBindMethod(ejs, prototype, ES_Path_separator, pathSeparator);
    ejsBindMethod(ejs, prototype, ES_Path_size, getPathFileSize);
    ejsBindMethod(ejs, prototype, ES_Path_toJSON, pathToJSON);
    ejsBindMethod(ejs, prototype, ES_Path_toString, pathToString);
    ejsBindMethod(ejs, prototype, ES_Path_trimExt, trimExt);
    ejsBindMethod(ejs, prototype, ES_Path_truncate, truncatePath);
    ejsBindMethod(ejs, prototype, ES_Path_windows, windowsPath);
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
