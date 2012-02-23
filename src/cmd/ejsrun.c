/**
    ejsrun.c - Run an ejs script.

    This program runs a script of the same name as this script.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejsCompiler.h"

/*********************************** Locals ***********************************/

typedef struct App {
    Ejs         *ejs;
} App;

static App *app;

static void manageApp(App *app, int flags);

/************************************ Code ************************************/

MAIN(ejsMain, int argc, char **argv, char **envp)
{
    Mpr     *mpr;
    Ejs     *ejs;
    char    *argp, *searchPath, *path, *homeDir;
    int     nextArg, err;

    /*  
        Initialize Multithreaded Portable Runtime (MPR)
     */
    mpr = mprCreate(argc, argv, 0);
    app = mprAllocObj(App, manageApp);
    mprAddRoot(app);
    mprAddStandardSignals();

    if (mprStart(mpr) < 0) {
        mprError("Can't start mpr services");
        return EJS_ERR;
    }
    err = 0;
    searchPath = 0;
    argc = mpr->argc;
    argv = mpr->argv;

    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }
        if (smatch(argp, "--chdir") || smatch(argp, "--home") || smatch(argp, "-C")) {
            if (nextArg >= argc) {
                err++;
            } else {
                homeDir = argv[++nextArg];
                if (chdir((char*) homeDir) < 0) {
                    mprError("Can't change directory to %s", homeDir);
                }
            }

        } else if (smatch(argp, "--debugger") || smatch(argp, "-D")) {
            mprSetDebugMode(1);

        } else if (smatch(argp, "--log")) {
            if (nextArg >= argc) {
                err++;
            } else {
                mprStartLogging(argv[++nextArg], 0);
                mprSetCmdlineLogging(1);
            }

        } else if (smatch(argp, "--name")) {
            /* Just ignore. Used to tag commands with a unique command line */ 
            nextArg++;

        } else if (smatch(argp, "--search") || smatch(argp, "--searchpath")) {
            if (nextArg >= argc) {
                err++;
            } else {
                searchPath = argv[++nextArg];
            }

        } else if (smatch(argp, "--verbose") || smatch(argp, "-v")) {
            mprStartLogging("stderr:2", 0);
            mprSetCmdlineLogging(1);

        } else if (smatch(argp, "--version") || smatch(argp, "-V")) {
            mprPrintfError("%s %s-%s\n", BLD_NAME, BLD_VERSION, BLD_NUMBER);
            return 0;

        } else {
            err++;
            break;
        }
    }
#if UNUSED
    if (err) {
        /*  
         */
        mprPrintfError("Usage: %s [options] script.es [arguments] ...\n"
            "  Options:\n"
            "  --debugger               # Disable timeouts to make using a debugger easier\n"
            "  --log logSpec            # Internal compiler diagnostics logging\n"
            "  --search ejsPath         # Module search path\n"
            "  --standard               # Default compilation mode to standard (default)\n"
            "  --verbose | -v           # Same as --log stderr:2 \n"
            "  --version                # Emit the version information\n",
            mpr->name);
        return -1;
    }
#endif
#if DEBUG_IDE || 1
    path = mprJoinPath(mprGetAppDir(), "bit");
#else
    path = mprJoinPath(mprGetAppDir(), mprGetPathBase(argv[0]));
#endif
    path = mprReplacePathExt(path, ".es");
    argv[0] = path;
    if ((ejs = ejsCreateVM(argc, (cchar **) &argv[0], 0)) == 0) {
        return MPR_ERR_MEMORY;
    }
    app->ejs = ejs;
    if (ejsLoadModules(ejs, searchPath, NULL) < 0) {
        return MPR_ERR_CANT_READ;
    }
    mprLog(1, "Load script \"%s\"", path);
#if UNUSED
    if (ejsLoadScriptFile(ejs, path, NULL, EC_FLAGS_DEBUG | EC_FLAGS_THROW) < 0) {
        ejsReportError(ejs, "Error in script");
        err = MPR_ERR;
    }
#else
    EcCompiler      *ec;
    int             flags = EC_FLAGS_BIND | EC_FLAGS_DEBUG | EC_FLAGS_NO_OUT | EC_FLAGS_THROW;
    if ((ec = ecCreateCompiler(ejs, flags)) == 0) {
        return MPR_ERR_MEMORY;
    }
    mprAddRoot(ec);
    ecSetOptimizeLevel(ec, 9);
    ecSetWarnLevel(ec, 1);
    if (ecCompile(ec, 1, (char**) &path) < 0) {
        if (flags & EC_FLAGS_THROW) {
            ejsThrowSyntaxError(ejs, "%s", ec->errorMsg ? ec->errorMsg : "Can't parse script");
        }
        err = MPR_ERR;
    } else {
        mprRemoveRoot(ec);
        if (ejsRunProgram(ejs, NULL, NULL) < 0) {
            ejsReportError(ejs, "Error in script");
            err = MPR_ERR;
        }
    }
#endif
    app->ejs = 0;
    mprTerminate(MPR_EXIT_DEFAULT, err);
    ejsDestroyVM(ejs);
    mprDestroy(MPR_EXIT_DEFAULT);
    return err;
}


static void manageApp(App *app, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->ejs);
    }
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
