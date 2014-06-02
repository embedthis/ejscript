/**
    ecCompile.c - Interface to the compiler

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejsCompiler.h"

/***************************** Forward Declarations ***************************/

static void compileError(EcCompiler *cp, cchar *fmt, ...);
static int compileInner(EcCompiler *cp, int argc, char **argv);
static EjsObj *loadScriptLiteral(Ejs *ejs, EjsString *script, cchar *cache);
static EjsObj *loadScriptFile(Ejs *ejs, cchar *path, cchar *cache);
static void manageCompiler(EcCompiler *cp, int flags);

/************************************ Code ************************************/

PUBLIC EcCompiler *ecCreateCompiler(Ejs *ejs, int flags)
{
    EcCompiler      *cp;

    if ((cp = mprAllocObj(EcCompiler, manageCompiler)) == 0) {
        return 0;
    }
    cp->ejs = ejs;
    cp->strict = 0;
    cp->tabWidth = EC_TAB_WIDTH;
    cp->warnLevel = 1;
    cp->shbang = 1;
    cp->optimizeLevel = 9;
    cp->warnLevel = 1;
    cp->outputDir = sclone(".");

    if (flags & EC_FLAGS_DOC) {
        cp->doc = 1;
    }
    if (flags & EC_FLAGS_BIND) {
        cp->bind = 1;
    }
    if (flags & EC_FLAGS_DEBUG) {
        cp->debug = 1;
    }
    if (flags & EC_FLAGS_MERGE) {
        cp->merge = 1;
    }
    if (flags & EC_FLAGS_NO_OUT) {
        cp->noout = 1;
    }
    if (flags & EC_FLAGS_VISIBLE) {
        cp->visibleGlobals = 1;
    }
    if (ecResetModuleList(cp) < 0) {
        return 0;
    }
    ecInitLexer(cp);
    ecResetParser(cp);
    return cp;
}

        
static void manageCompiler(EcCompiler *cp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(cp->nodes);
        mprMark(cp->certFile);
        mprMark(cp->docToken);
        mprMark(cp->extraFiles);
        mprMark(cp->file);
        mprMark(cp->keywords);
        mprMark(cp->peekToken);
        mprMark(cp->putback);
        mprMark(cp->state);
        mprMark(cp->stream);
        mprMark(cp->token);
        mprMark(cp->outputDir);
        mprMark(cp->outputFile);
        mprMark(cp->fixups);
        mprMark(cp->require);
        mprMark(cp->modules);
        mprMark(cp->errorMsg);
    }
}


PUBLIC int ecCompile(EcCompiler *cp, int argc, char **argv)
{
    Ejs     *ejs;
    int     rc, saveCompiling, paused;

    ejs = cp->ejs;
    saveCompiling = ejs->compiling;
    ejs->compiling = 1;
    
    paused = ejsBlockGC(ejs);
    rc = compileInner(cp, argc, argv);
    ejsUnblockGC(ejs, paused);
    ejs->compiling = saveCompiling;
    return rc;
}


static int compileInner(EcCompiler *cp, int argc, char **argv)
{
    Ejs         *ejs;
    EjsModule   *mp;
    MprList     *nodes;
    EjsBlock    *block;
    EcLocation  loc;
    cchar       *ext;
    char        *msg;
    int         next, i, j, nextModule, lflags, rc, paused;

    ejs = cp->ejs;
    if ((nodes = mprCreateList(-1, 0)) == 0) {
        return EJS_ERR;
    }
    cp->nodes = nodes;

    /*
        Warn about source files mentioned multiple times.
        TODO OPT. This is slow.
     */
    for (i = 0; i < argc; i++) {
        for (j = 0; j < argc; j++) {
            if (i == j) {
                continue;
            }
            if (mprSamePath(argv[i], argv[j])) {
                compileError(cp, "Loading source %s multiple times. Ignoring extra copies.", argv[i]);
                return EJS_ERR;
            }
        }
        if (cp->outputFile && mprSamePath(cp->outputFile, argv[i])) {
            compileError(cp, "Output file is the same as input file: %s", argv[i]);
            return EJS_ERR;
        }
    }

    /*
        Compile source files and load any module files
     */
    for (i = 0; i < argc && !cp->fatalError; i++) {
        ext = mprGetPathExt(argv[i]);
        if (scaselesscmp(ext, "mod") == 0 || scaselesscmp(ext, ME_SHOBJ) == 0) {
            nextModule = mprGetListLength(ejs->modules);
            lflags = cp->strict ? EJS_LOADER_STRICT : 0;
            if ((rc = ejsLoadModule(cp->ejs, ejsCreateStringFromAsc(ejs, argv[i]), -1, -1, lflags)) < 0) {
                msg = sfmt("Error initializing module %s\n%s", argv[i], ejsGetErrorMsg(cp->ejs, 1));
                memset(&loc, 0, sizeof(EcLocation));
                loc.filename = sclone(argv[i]);
                if (rc == MPR_ERR_CANT_INITIALIZE) {
                    ecError(cp, "Error", &loc, msg);
                } else {
                    ecError(cp, "Error", &loc, msg);
                }
                cp->nodes = NULL;
                return EJS_ERR;
            }
            if (cp->merge) {
                /*
                    If merging, we must emit the loaded module into the output. So add to the compiled modules list.
                 */
                for (next = nextModule; (mp = mprGetNextItem(ejs->modules, &next)) != 0; ) {
                    if (mprLookupItem(cp->modules, mp) < 0 && mprAddItem(cp->modules, mp) < 0) {
                        compileError(cp, "Cannot add module %s", mp->name);
                    }
                }
            }
            mprAddItem(nodes, 0);
        } else  {
            paused = ejsBlockGC(ejs);
            mprAddItem(nodes, ecParseFile(cp, argv[i]));
            ejsUnblockGC(ejs, paused);
        }
    }
    /*
        Allocate the eval frame stack. This is used for property lookups. We have one dummy block at the top always.
     */
    block = ejsCreateBlock(ejs, 0);
    mprSetName(block, "Compiler");
    ejsPushBlock(ejs, block);
    
    /*
        Process the internal representation and generate code
     */
    paused = ejsBlockGC(ejs);
    if (!cp->parseOnly && cp->errorCount == 0) {
        ecResetParser(cp);
        if (ecAstProcess(cp) < 0) {
            ejsPopBlock(ejs);
            cp->nodes = NULL;
            ejsUnblockGC(ejs, paused);
            return EJS_ERR;
        }
        if (cp->errorCount == 0) {
            ecResetParser(cp);
            if (ecCodeGen(cp) < 0) {
                ejsPopBlock(ejs);
                cp->nodes = NULL;
                ejsUnblockGC(ejs, paused);
                return EJS_ERR;
            }
        }
    }
    ejsPopBlock(ejs);

    /*
        Add compiled modules to the interpreter
     */
    for (next = 0; ((mp = (EjsModule*) mprGetNextItem(cp->modules, &next)) != 0); ) {
        if (mp->code) {
            ejsAddModule(cp->ejs, mp);
        }
    }
    cp->nodes = NULL;
    ejsUnblockGC(ejs, paused);
    if (!paused) {
        mprYield(0);
    }
    return (cp->errorCount > 0) ? EJS_ERR: 0;
}


PUBLIC int ejsInitCompiler(EjsService *service)
{
    service->loadScriptLiteral = loadScriptLiteral;
    service->loadScriptFile = loadScriptFile;
    return 0;
}


/*
    Load a script file. This indirect routine is used by the core VM to compile a file when required.
 */
static EjsObj *loadScriptFile(Ejs *ejs, cchar *path, cchar *cache)
{
    if (ejsLoadScriptFile(ejs, path, cache, EC_FLAGS_NO_OUT | EC_FLAGS_DEBUG | EC_FLAGS_THROW) < 0) {
        return 0;
    }
    return ejs->result;
}


/*
    Function for ejs->loadScriptLiteral. This indirect routine is used by the core VM to compile a script when required.
 */
static EjsObj *loadScriptLiteral(Ejs *ejs, EjsString *script, cchar *cache)
{
    if (ejsLoadScriptLiteral(ejs, script, cache, EC_FLAGS_NO_OUT | EC_FLAGS_DEBUG | EC_FLAGS_THROW) < 0) {
        return 0;
    }
    return ejs->result;
}


PUBLIC int ejsLoadScriptFile(Ejs *ejs, cchar *path, cchar *cache, int flags)
{
    EcCompiler      *ec;

    if ((ec = ecCreateCompiler(ejs, flags)) == 0) {
        return MPR_ERR_MEMORY;
    }
    mprAddRoot(ec);
    if (cache) {
        ec->noout = 0;
        ecSetOutputFile(ec, cache);
    } else {
        ec->noout = 1;
    }
    if (ecCompile(ec, 1, (char**) &path) < 0) {
        if (flags & EC_FLAGS_THROW && !ejs->exception) {
            ejsThrowSyntaxError(ejs, "%s", ec->errorMsg ? ec->errorMsg : "Cannot parse script");
        }
        mprRemoveRoot(ec);
        return EJS_ERR;
    }
    mprRemoveRoot(ec);
    if (ejsRun(ejs) < 0) {
        return EJS_ERR;
    }
    return 0;
}


/*
    Load and initialize a script literal
 */
PUBLIC int ejsLoadScriptLiteral(Ejs *ejs, EjsString *script, cchar *cache, int flags)
{
    EcCompiler      *cp;
    cchar           *path;

    if ((cp = ecCreateCompiler(ejs, flags)) == 0) {
        return MPR_ERR_MEMORY;
    }
    mprAddRoot(cp);
    if (cache) {
        cp->noout = 0;
        ecSetOutputFile(cp, cache);
    } else {
        cp->noout = 1;
    }
    //  UNICODE -- should this API be multi or unicode
    if (ecOpenMemoryStream(cp, ejsToMulti(ejs, script), script->length) < 0) {
        mprError("ejs compiler", "Cannot open memory stream");
        mprRemoveRoot(cp);
        return EJS_ERR;
    }
    path = "__script__";
    if (ecCompile(cp, 1, (char**) &path) < 0) {
        if (flags & EC_FLAGS_THROW) {
            ejsThrowSyntaxError(ejs, "%s", cp->errorMsg ? cp->errorMsg : "Cannot parse script");
        }
        mprRemoveRoot(cp);
        return EJS_ERR;
    }
    ecCloseStream(cp);
    mprRemoveRoot(cp);
    if (ejsRun(ejs) < 0) {
        return EJS_ERR;
    }
    return 0;
}


/*
    One-line embedding. Evaluate a file. This will compile and interpret the given Ejscript source file.
 */
PUBLIC int ejsEvalFile(cchar *path)
{
    Ejs     *ejs;

    mprCreate(0, 0, 0);
    if ((ejs = ejsCreateVM(0, 0, 0)) == 0) {
        mprDestroy(0);
        return MPR_ERR_MEMORY;
    }
    mprAddRoot(ejs);
    if (ejsLoadModules(ejs, 0, 0) < 0) {
        mprDestroy(0);
        return MPR_ERR_CANT_READ;
    }
    if (ejsLoadScriptFile(ejs, path, NULL, EC_FLAGS_NO_OUT | EC_FLAGS_DEBUG) < 0) {
        ejsReportError(ejs, "Error in program");
        mprDestroy(0);
        return MPR_ERR;
    }
    mprDestroy();
    return 0;
}


/*
    One-line embedding. Evaluate a script. This will compile and interpret the given script.
 */
PUBLIC int ejsEvalScript(cchar *script)
{
    Ejs     *ejs;

    mprCreate(0, 0, 0);
    if ((ejs = ejsCreateVM(0, 0, 0)) == 0) {
        mprDestroy(0);
        return MPR_ERR_MEMORY;
    }
    mprAddRoot(ejs);
    if (ejsLoadModules(ejs, 0, 0) < 0) {
        mprDestroy(0);
        return MPR_ERR_CANT_READ;
    }
    if (ejsLoadScriptLiteral(ejs, ejsCreateStringFromAsc(ejs, script), NULL, EC_FLAGS_NO_OUT | EC_FLAGS_DEBUG) < 0) {
        ejsReportError(ejs, "Error in program");
        mprDestroy(0);
        return MPR_ERR;
    }
    mprDestroy();
    return 0;
}


static void compileError(EcCompiler *cp, cchar *fmt, ...)
{
    va_list     args;


    cp->errorCount++;
    cp->error = 1;
    va_start(args, fmt);
    ecError(cp, "Error", NULL, fmt, args);
    va_end(args);
}


PUBLIC void ecError(EcCompiler *cp, cchar *severity, EcLocation *loc, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    ecErrorv(cp, severity, loc, fmt, args);
    va_end(args);
}


/*
    Create a line of spaces with an "^" pointer at the current parse error.
 */
static char *makeHighlight(EcCompiler *cp, wchar *source, int col)
{
    wchar       *up, *sp;
    char        *dest, *dp;
    int         tabCount, len, i;

    tabCount = 0;

    for (up = source, len = 0; *up; up++) {
        if (*up == '\t') {
            tabCount++;
        }
        len++;
    }
    len = (int) len + (tabCount * cp->tabWidth);
    len = max(len, col);

    /*
        Allow for "^" to be after the last char, plus one null.
     */
    if ((dest = mprAlloc(len + 2)) == NULL) {
        assert(dest);
        return 0;
    }
    for (i = 0, dp = dest, sp = source; *sp; sp++, i++) {
        if (*sp== '\t') {
            *dp++ = *sp;
        } else {
            *dp++ = ' ';
        }
    }
    /*
        Cover the case where the ^ must go after the end of the input
     */
    if (col >= 0) {
        dest[col] = '^';
        if (dp == &dest[col]) {
            ++dp;
        }
        *dp = '\0';
    }
    return dest;
}


PUBLIC void ecErrorv(EcCompiler *cp, cchar *severity, EcLocation *loc, cchar *fmt, va_list args)
{
    cchar   *appName;
    char    *pointer, *errorMsg, *msg;

    appName = mprGetAppName(cp);
    msg = sfmtv(fmt, args);

    if (loc) {
        if (loc->source) {
            pointer = makeHighlight(cp, loc->source, loc->column);
            errorMsg = sfmt("%s: %s: %s: %d: %s\n  %w  \n  %s", appName, severity, loc->filename, 
                loc->lineNumber, msg, loc->source, pointer);
        } else if (loc->lineNumber >= 0) {
            errorMsg = sfmt("%s: %s: %s: %d: %s", appName, severity, loc->filename, loc->lineNumber, msg);
        } else {
            errorMsg = sfmt("%s: %s: %s: 0: %s", appName, severity, loc->filename, msg);
        }
    } else {
        errorMsg = sfmt("%s: %s: %s", appName, severity, msg);
    }
    cp->errorMsg = srejoin(cp->errorMsg, errorMsg, NULL);
    mprBreakpoint();
}


PUBLIC void ecSetRequire(EcCompiler *cp, MprList *modules)
{
    cp->require = modules;
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
