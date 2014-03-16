/**
    ejsFileSystem.c - FileSystem class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Methods *********************************/
/*
    static function drives(): Array
 */
static EjsArray *fs_drives(Ejs *ejs, EjsFileSystem *unused, int argc, EjsObj **argv)
{
    EjsArray    *ap;

    if ((ap = ejsCreateArray(ejs, 0)) == 0) {
        return 0;
    }
#if ME_WIN_LIKE
{
    char        dbuf[2];
    int         i, mask;

    mask = GetLogicalDrives();
    for (i = 0; i < 26; i++) {
        if (mask & (1 << i)) {
            dbuf[0] = 'A' + i;
            dbuf[1] = '\0';
            ejsAddItem(ejs, ap, ejsCreateStringFromAsc(ejs, dbuf));
        }
    }
}
#endif
    return ap;
}


/*
    Constructor

    function FileSystem(path: String)
 */
static EjsFileSystem *fileSystemConstructor(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    cchar   *path;

#if UNUSED
    assert(argc == 1 && ejsIs(ejs, argv[0], String));
    path = ejsToMulti(ejs, argv[0]);
#else
    assert(argc == 1 && ejsIs(ejs, argv[0], Path));
    path = ((EjsPath*) argv[0])->value;
#endif
    fp->path = mprNormalizePath(path);
    fp->fs = mprLookupFileSystem(path);
    return fp;
}


#if ES_space
/*
    Return the amount of free space in the file system that would contain the given path.

    function freeSpace(path: String = null): Number
 */
static EjsObj *fileSystemSpace(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
#if BREW
    Mpr     *mpr;
    uint    space;

    mpr = mprGetMpr();
    space = IFILEMGR_GetFreeSpace(mpr->fileMgr, 0);
    ejsSetReturnValueToInteger(ejs, space);
#endif
    return 0;
}
#endif


/*
    Determine if the file system has a drive specs (C:) in paths

    static function hasDrives(): Boolean
 */
static EjsBoolean *hasDrives(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, fp->fs->hasDriveSpecs);
}


#if ES_isReady
/*
    Determine if the file system is ready for I/O

    function get isReady(): Boolean
 */
static EjsBoolean *isReady(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    int         rc;

    rc = mprGetPathInfo(ejs, fp->path, &info);
    return ejsCreateBoolean(ejs, rc == 0 && info.isDir);
}
#endif


#if ES_isWritable
static EjsBoolean *isWritable(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    int         rc;

    rc = mprGetPathInfo(ejs, fp->path, &info);
    return ejsCreateBoolean(ejs, rc == 0 && info.isDir);
}
#endif


/*
    Get the newline characters

    function get newline(): String
 */
static EjsString *getNewline(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, mprGetPathNewline(fp->path));
}


/*
    set the newline characters

    function set newline(terminator: String): Void
 */
static EjsObj *setNewline(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    cchar   *nl;

    assert(ejsIs(ejs, argv[0], String));
    nl = ejsToMulti(ejs, (EjsString*) argv[0]);
    mprSetPathNewline(fp->path, nl);
    return 0;
}


static EjsPath *root(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    cchar   *separators;
    char    *path, *cp;

    separators = mprGetPathSeparators(fp->path);
    path = mprGetAbsPath(fp->path);
    if ((cp = strchr(path, separators[0])) != 0) {
        *++cp = '\0';
    }
    return ejsCreatePathFromAsc(ejs, path);
}


/*
    Return the path directory separators

    static function get separators(): String
 */
static EjsString *getSeparators(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, fp->fs->separators);
}


/*
    Set the path directory separators

    static function set separators(value: String): void
 */
static EjsObj *setSeparators(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    assert(argc == 1 && ejsIs(ejs, argv[0], String));
    mprSetPathSeparators(fp->path, ejsToMulti(ejs, argv[0]));
    return 0;
}


#if ES_size
static EjsObj *size(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    return 0;
}
#endif

/*********************************** Factory **********************************/

static void manageFileSystem(EjsFileSystem *fs, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(TYPE(fs));
        mprMark(fs->path);
    }
}


PUBLIC EjsFileSystem *ejsCreateFileSystem(Ejs *ejs, cchar *path)
{
    EjsFileSystem   *fs;
    EjsObj          *arg;

    fs = ejsCreateObj(ejs, ESV(FileSystem), 0);
    if (fs == 0) {
        return 0;
    }
    arg = (EjsObj*) ejsCreateStringFromAsc(ejs, path);
    fileSystemConstructor(ejs, fs, 1, (EjsObj**) &arg);
    return fs;
}


PUBLIC void ejsConfigureFileSystemType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "FileSystem"), sizeof(EjsFileSystem), manageFileSystem,
            EJS_TYPE_OBJ)) == 0) {
        return;
    }
    prototype = type->prototype;
    ejsBindConstructor(ejs, type, fileSystemConstructor);
    ejsBindMethod(ejs, type, ES_FileSystem_drives, fs_drives);
#if ES_space
    ejsBindMethod(ejs, prototype, ES_FileSystem_space, fileSystemSpace);
#endif
    ejsBindMethod(ejs, prototype, ES_FileSystem_hasDrives, hasDrives);
#if ES_isReady
    ejsBindMethod(ejs, prototype, ES_FileSystem_isReady, isReady);
#endif
#if ES_isWritable
    ejsBindMethod(ejs, prototype, ES_FileSystem_isWritable, isWritable);
#endif
    ejsBindAccess(ejs, prototype, ES_FileSystem_newline, getNewline, setNewline);
    ejsBindMethod(ejs, prototype, ES_FileSystem_root, root);
    ejsBindAccess(ejs, prototype, ES_FileSystem_separators, getSeparators, setSeparators);
#if ES_size
    ejsBindMethod(ejs, prototype, ES_FileSystem_size, size);
#endif
}

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
