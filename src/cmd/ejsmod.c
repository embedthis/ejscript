/**
    ejsmod.c - Module manager 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejsmod.h"

#if BIT_PACK_EJSCRIPT

/****************************** Forward Declarations **************************/

static void getDepends(Ejs *ejs, MprList *list, EjsModule *mp);
static void manageMod(EjsMod *mp, int flags);
static int  process(EjsMod *mp, cchar *output, int argc, char **argv);
static void require(MprList *list, cchar *name);

/************************************ Code ************************************/

MAIN(ejsmodMain, int argc, char **argv, char **envp)
{
    Mpr             *mpr;
    EjsMod          *mp;
    Ejs             *ejs;
    MprList         *requiredModules;
    char            *argp, *searchPath, *output, *modules, *name, *tok;
    int             nextArg, err, flags;

    err = 0;
    output = searchPath = 0;
    requiredModules = 0;
    
    /*
        Initialze the Multithreaded Portable Runtime (MPR)
     */
    mpr = mprCreate(argc, argv, 0);
    mprSetAppName(argv[0], 0, 0);

    /*
        Allocate the primary control structure
     */
    if ((mp = mprAllocObj(EjsMod, manageMod)) == NULL) {
        return MPR_ERR_MEMORY;
    }
    mprAddRoot(mp);
    mp->lstRecords = mprCreateList(0, 0);
    mp->blocks = mprCreateList(0, 0);
    mp->docDir = sclone(".");
    mp->outputDir = sclone(".");
    
    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }
        if (strcmp(argp, "--cslots") == 0) {
            mp->cslots = 1;
            mp->genSlots = 1;
            
        } else if (strcmp(argp, "--debugger") == 0 || strcmp(argp, "-D") == 0) {
            mprSetDebugMode(1);

        } else if (strcmp(argp, "--depends") == 0) {
            mp->depends = 1;

        } else if (strcmp(argp, "--dir") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                mp->outputDir = sclone(argv[++nextArg]);
            }
            
        } else if (strcmp(argp, "--error") == 0) {
            /*
                Undocumented switch
             */
            mp->exitOnError++;
            mp->warnOnError++;
            
        } else if (strcmp(argp, "--html") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                mp->docDir = sclone(argv[++nextArg]);
                mp->html = 1;
            }
            
        } else if (strcmp(argp, "--listing") == 0) {
            mp->listing = 1;
            mp->showAsm = 1;
            
        } else if (strcmp(argp, "--log") == 0) {
            /*
                Undocumented switch
             */
            if (nextArg >= argc) {
                err++;
            } else {
                mprStartLogging(argv[++nextArg], 0);
                mprSetCmdlineLogging(1);
            }

        } else if (strcmp(argp, "--out") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                output = argv[++nextArg];
                mp->cslots = 1;
                mp->genSlots = 1;
            }

        } else if (strcmp(argp, "--search") == 0 || strcmp(argp, "--searchpath") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                searchPath = argv[++nextArg];
            }

        } else if (strcmp(argp, "--version") == 0 || strcmp(argp, "-V") == 0) {
            mprPrintf("%s-%s\n", BIT_VERSION, BIT_BUILD_NUMBER);  
            return 0;

        } else if (strcmp(argp, "--require") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                if (requiredModules == 0) {
                    requiredModules = mprCreateList(-1, 0);
                }
                modules = sclone(argv[++nextArg]);
                /*  Fix for Xcode and Visual Studio */
                if (modules[0] == ' ' || scmp(modules, "null") == 0) {
                    modules[0] = '\0';                    
                }
                name = stok(modules, " \t", &tok);
                while (name != NULL) {
                    require(requiredModules, name);
                    name = stok(NULL, " \t", &tok);
                }
            }

        } else if (strcmp(argp, "--warn") == 0) {
            mp->warnOnError++;

        } else if (strcmp(argp, "--xml") == 0) {
            mp->xml = 1;

        } else {
            err++;
            break;
        }
    }
    if (argc == nextArg) {
        err++;
    }
    if (mp->genSlots == 0 && mp->listing == 0 && mp->html == 0 && mp->xml == 0 && mp->depends == 0) {
        mp->listing = 1;
    }
    if (mp->depends && requiredModules == 0) {
        requiredModules = mprCreateList(-1, 0);
    }
    if (err) {
        /*
            Examples:
                ejsmod file.mod                              # Defaults to --listing
                ejsmod --listing embedthis.mod 
                ejsmod --out slots.h embedthis.mod 
         */
        mprEprintf("Usage: %s [options] modules ...\n"
            "  Ejscript module manager options:\n"
            "  --cslots              # Generate a C slot definitions file\n"
            "  --html dir            # Generate HTML documentation to the specified directory\n"
            "  --listing             # Create assembler listing files (default)\n"
            "  --out file            # Output file for all C slots (implies --cslots)\n"
            "  --require \"modules\"   # List of modules to preload\n"
            "  --search ejsPath      # Module file search path\n"
            "  --version             # Emit the program version information\n"
            "  --warn                # Warn about undocumented methods or parameters in doc\n\n", mpr->name);
        return -1;
    }

    /*
        Need an interpreter to load modules
     */
    flags = EJS_FLAG_NO_INIT;
    if (mp->html || mp->xml) {
        flags |= EJS_FLAG_DOC;
    }
    if ((ejs = ejsCreateVM(0, 0, flags)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (ejsLoadModules(ejs, searchPath, requiredModules) < 0) {
        return MPR_ERR_CANT_READ;
    }
    mp->ejs = ejs;

    if (nextArg < argc) {
        if (process(mp, output, argc - nextArg, &argv[nextArg]) < 0) {
            err++;
        }
    }
    if (mp->errorCount > 0) {
        err = -1;
    }
    mprRemoveRoot(mp);
    ejsDestroyVM(ejs);
    mprDestroy(MPR_EXIT_DEFAULT);
    return err;
}


static void manageMod(EjsMod *mp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(mp->ejs);
        mprMark(mp->currentLine);
        mprMark(mp->currentFileName);
        mprMark(mp->lstRecords);
        mprMark(mp->packages);
        mprMark(mp->blocks);
        mprMark(mp->currentBlock);
        mprMark(mp->docDir);
        mprMark(mp->path);
        mprMark(mp->file);
        mprMark(mp->module);
        mprMark(mp->fun);
        mprMark(mp->pc);
    }
}


static void require(MprList *list, cchar *name) 
{
    assert(list);

    if (name && *name) {
        mprAddItem(list, name);
    }
}


static int process(EjsMod *mp, cchar *output, int argc, char **argv)
{
    Ejs         *ejs;
    EjsModule   *module;
    MprFile     *outfile;
    MprList     *depends;
    int         count, i, next, moduleCount;

    ejs = mp->ejs;
    
    if (output) {
        outfile = mprOpenFile(output, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0664);
    } else {
        outfile = 0;
    }
    ejs->loaderCallback = (mp->listing) ? emListingLoadCallback : 0;
    mp->firstGlobal = ejsGetLength(ejs, ejs->global);

    /*
        For each module on the command line
     */
    for (i = 0; i < argc && !mp->fatalError; i++) {
        moduleCount = mprGetListLength(ejs->modules);
        ejs->loadData = mp;
        if (!mprPathExists(argv[i], R_OK)) {
            mprError("Cannot access module %s", argv[i]);
            return EJS_ERR;
        }
        if ((ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, argv[i]), -1, -1, EJS_LOADER_NO_INIT)) < 0) {
            ejs->loaderCallback = NULL;
            mprError("Cannot load module %s\n%s", argv[i], ejsGetErrorMsg(ejs, 0));
            return EJS_ERR;
        }
        if (mp->genSlots) {
            for (next = moduleCount; (module = mprGetNextItem(ejs->modules, &next)) != 0; ) {
                emCreateSlotFiles(mp, module, outfile);
            }
        }
        if (mp->depends) {
            depends = mprCreateList(-1, 0);
            for (next = moduleCount; (module = mprGetNextItem(ejs->modules, &next)) != 0; ) {
                getDepends(ejs, depends, module);
            }
            count = mprGetListLength(depends);
            for (next = 1; (module = mprGetNextItem(depends, &next)) != 0; ) {
                int version = module->version;
                mprPrintf("%@-%d.%d.%d%s", module->name, EJS_MAJOR(version), EJS_MINOR(version), EJS_PATCH(version),
                    (next >= count) ? "" : " ");
            }
            printf("\n");
        }
    }
    if (mp->html || mp->xml) {
        emCreateDoc(mp);
    }
    mprCloseFile(outfile);
    return 0;
}


static void getDepends(Ejs *ejs, MprList *list, EjsModule *mp) 
{
    EjsModule   *module;
    int         next;

    if (mprLookupItem(list, mp) < 0) {
        mprAddItem(list, mp);
    }
    for (next = 0; (module = mprGetNextItem(mp->dependencies, &next)) != 0; ) {
        if (mprLookupItem(list, module) < 0) {
            mprAddItem(list, module);
        }
    }
}

#else

MAIN(ejsMain, int argc, char **argv, char **envp)
{
    printf("Ejscript not enabled\n");
    return 255;
}

#endif /* BIT_PACK_EJSCRIPT */
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
