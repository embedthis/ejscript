/**
    ejsFileSystem.c - FileSystem class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Methods *********************************/
/*
    Constructor

    function FileSystem(path: String)
 */
static EjsObj *fileSystemConstructor(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    cchar   *path;

    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));

    path = ejsToMulti(ejs, argv[0]);
    fp->path = mprGetNormalizedPath(path);
    fp->fs = mprLookupFileSystem(path);
    return (EjsObj*) fp;
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
static EjsObj *hasDrives(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, fp->fs->hasDriveSpecs);
}


#if ES_isReady
/*
    Determine if the file system is ready for I/O

    function get isReady(): Boolean
 */
static EjsObj *isReady(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    int         rc;

    rc = mprGetPathInfo(ejs, fp->path, &info);
    return (EjsObj*) ejsCreateBoolean(ejs, rc == 0 && info.isDir);
}
#endif


#if ES_isWritable
static EjsObj *isWritable(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    MprPath     info;
    int         rc;

    rc = mprGetPathInfo(ejs, fp->path, &info);
    return (EjsObj*) ejsCreateBoolean(ejs, rc == 0 && info.isDir);
}
#endif


/*
    Get the newline characters

    function get newline(): String
 */
static EjsObj *getNewline(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringFromAsc(ejs, mprGetPathNewline(fp->path));
}


/*
    set the newline characters

    function set newline(terminator: String): Void
 */
static EjsObj *setNewline(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    cchar   *nl;

    mprAssert(ejsIsString(ejs, argv[0]));
    nl = ejsToMulti(ejs, (EjsString*) argv[0]);
    mprSetPathNewline(fp->path, nl);
    return 0;
}


static EjsObj *root(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    cchar   *separators;
    char    *path, *cp;

    separators = mprGetPathSeparators(fp->path);
    path = mprGetAbsPath(fp->path);
    if ((cp = strchr(path, separators[0])) != 0) {
        *++cp = '\0';
    }
    return (EjsObj*) ejsCreatePathFromAsc(ejs, path);
}


/*
    Return the path directory separators

    static function get separators(): String
 */
static EjsObj *getSeparators(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringFromAsc(ejs, fp->fs->separators);
}


/*
    Set the path directory separators

    static function set separators(value: String): void
 */
static EjsObj *setSeparators(Ejs *ejs, EjsFileSystem *fp, int argc, EjsObj **argv)
{
    mprAssert(argc == 1 && ejsIsString(ejs, argv[0]));
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
        //  MOB -- type required
        mprMark(TYPE(fs));
        mprMark(fs->path);
    }
}


EjsFileSystem *ejsCreateFileSystem(Ejs *ejs, cchar *path)
{
    EjsFileSystem   *fs;
    EjsObj          *arg;

    fs = ejsCreateObj(ejs, ejs->fileSystemType, 0);
    if (fs == 0) {
        return 0;
    }
    arg = (EjsObj*) ejsCreateStringFromAsc(ejs, path);
    fileSystemConstructor(ejs, fs, 1, (EjsObj**) &arg);
    return fs;
}


void ejsConfigureFileSystemType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    type = ejsConfigureNativeType(ejs, N("ejs", "FileSystem"), sizeof(EjsFileSystem), (MprManager) manageFileSystem, 
        EJS_OBJ_HELPERS);
    ejs->fileSystemType = type;
    prototype = type->prototype;

    ejsBindConstructor(ejs, type, (EjsProc) fileSystemConstructor);
#if ES_space
    ejsBindMethod(ejs, prototype, ES_FileSystem_space, (EjsProc) fileSystemSpace);
#endif
    ejsBindMethod(ejs, prototype, ES_FileSystem_hasDrives, (EjsProc) hasDrives);
#if ES_isReady
    ejsBindMethod(ejs, prototype, ES_FileSystem_isReady, (EjsProc) isReady);
#endif
#if ES_isWritable
    ejsBindMethod(ejs, prototype, ES_FileSystem_isWritable, (EjsProc) isWritable);
#endif
    ejsBindAccess(ejs, prototype, ES_FileSystem_newline, (EjsProc) getNewline, (EjsProc) setNewline);
    ejsBindMethod(ejs, prototype, ES_FileSystem_root, (EjsProc) root);
    ejsBindAccess(ejs, prototype, ES_FileSystem_separators, (EjsProc) getSeparators, (EjsProc) setSeparators);
#if ES_size
    ejsBindMethod(ejs, prototype, ES_FileSystem_size, (EjsProc) size);
#endif
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
