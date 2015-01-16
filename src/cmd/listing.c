/**
    listing.c - Assembler listing generator.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejsmod.h"
#include    "ejsByteCodeTable.h"

/***************************** Forward Declarations ****************************/

static EjsString *getBlockName(EjsMod *mp, EjsObj *block, int slotNum);
static uchar getByte(EjsMod *mp);
static uint  getInt32(EjsMod *mp);
static double getDouble(EjsMod *mp);
static int64 getNum(EjsMod *dp);
static EjsString *getString(Ejs *ejs, EjsMod *dp);
static void getGlobal(EjsMod *mp, char *buf, int buflen);
static void lstSlotAssignments(EjsMod *mp, EjsModule *module, EjsObj *parent, int slotNum, EjsObj *obj);
static char *getAttributeString(EjsMod *mp, int attributes);
static void interp(EjsMod *mp, EjsModule *module, EjsFunction *fun);
static void leadin(EjsMod *mp, EjsModule *module, int classDec, int inFunction);
static void lstBlock(EjsMod *mp, EjsModule *module, EjsObj *block, int slotNum, EjsString *name, int numProp);
static void lstClass(EjsMod *mp, EjsModule *module, int slotNum, EjsType *klass, int attributes);
static void lstClose(EjsMod *mp, MprList *modules, int origin);
static void lstDependency(EjsMod *mp, EjsModule *module, EjsModule *dependant);
static void lstEndModule(EjsMod *mp, EjsModule *module);
static void lstException(EjsMod *mp, EjsModule *module, EjsFunction *fun);
static void lstFunction(EjsMod *mp, EjsModule *module, EjsObj *block, int slotNum, EjsName qname, EjsFunction *fun, 
    int attributes);
static int  lstOpen(EjsMod *mp, char *moduleFilename, EjsModuleHdr *hdr);
static void lstProperty(EjsMod *mp, EjsModule *module, EjsObj *block, int slotNum, EjsName qname, int attributes, 
    EjsName typeName);
static void lstModule(EjsMod *mp, EjsModule *module);
static EjsString *mapSpace(Ejs *ejs, EjsString *space);

/*********************************** Code *************************************/
/*
    Listing loader callback. This is invoked at key points when loading a module file.
 */
void emListingLoadCallback(Ejs *ejs, int kind, ...)
{
    va_list         args;
    EjsModuleHdr    *hdr;
    EjsMod          *mp;
    Lst             *lst;
    MprList         *modules;
    char            *name;
    int             nextModule;

    va_start(args, kind);
    mp = ejs->loadData;
    lst = mprAlloc(sizeof(Lst));

    /*
        Decode the record type and create a list for later processing. We need to process
        after the loader has done fixup for forward type references.
     */
    switch (kind) {

    case EJS_SECT_BLOCK:
        lst->module = va_arg(args, EjsModule*);
        lst->owner = va_arg(args, EjsObj*);
        lst->slotNum = va_arg(args, int);
        lst->name = va_arg(args, EjsString*);
        lst->numProp = va_arg(args, int);
        break;

    case EJS_SECT_BLOCK_END:
        break;

    case EJS_SECT_CLASS:
        lst->module = va_arg(args, EjsModule*);
        lst->slotNum = va_arg(args, int);
        lst->qname = va_arg(args, EjsName);
        lst->type = va_arg(args, EjsType*);
        lst->attributes = va_arg(args, int);
        break;

    case EJS_SECT_CLASS_END:
        break;

    case EJS_SECT_DEPENDENCY:
        lst->module = va_arg(args, EjsModule*);
        lst->dependency = va_arg(args, EjsModule*);
        break;

    case EJS_SECT_END:
        modules = va_arg(args, MprList*);
        nextModule = va_arg(args, int);
        lstClose(mp, modules, nextModule);
        return;

    case EJS_SECT_EXCEPTION:
        lst->module = va_arg(args, EjsModule*);
        lst->fun = va_arg(args, EjsFunction*);
        break;

    case EJS_SECT_FUNCTION:
        lst->module = va_arg(args, EjsModule*);
        lst->owner = va_arg(args, EjsObj*);
        lst->slotNum = va_arg(args, int);
        lst->qname = va_arg(args, EjsName);
        lst->fun = va_arg(args, EjsFunction*);
        lst->attributes = va_arg(args, int);
        break;

    case EJS_SECT_FUNCTION_END:
        break;

    case EJS_SECT_START:
        name = va_arg(args, char*);
        hdr = va_arg(args, EjsModuleHdr*);
        lstOpen(mp, name, hdr);
        return;

    case EJS_SECT_PROPERTY:
        lst->module = va_arg(args, EjsModule*);
        lst->owner = va_arg(args, EjsObj*);
        lst->slotNum = va_arg(args, int);
        lst->qname = va_arg(args, EjsName);
        lst->attributes = va_arg(args, int);
        lst->typeName = va_arg(args, EjsName);
        break;

    case EJS_SECT_MODULE:
        break;

    case EJS_SECT_MODULE_END:
        break;
            
    case EJS_SECT_DEBUG:
        break;

    default:
        assert(0);
    }
    lst->kind = kind;
    mprAddItem(mp->lstRecords, lst);
}


/*
    Loader completion routine. Process listing records and emit the listing file.
 */
static void lstClose(EjsMod *mp, MprList *modules, int firstModule)
{
    EjsModule   *module;
    Lst         *lst;
    bool        headerOutput;
    int         next, nextModule, count;

    for (nextModule = firstModule; (module = (EjsModule*) mprGetNextItem(modules, &nextModule)) != 0; ) {
        headerOutput = 0;
        count = 0;
        for (next = 0; (lst = (Lst*) mprGetNextItem(mp->lstRecords, &next)) != 0; ) {
            if (lst->module != module) {
                continue;
            }
            if (!headerOutput) {
                lstModule(mp, lst->module);
                headerOutput = 1;
            }
            switch (lst->kind) {
            case EJS_SECT_BLOCK:
                lstBlock(mp, lst->module, lst->owner, lst->slotNum, lst->name, lst->numProp);
                count++;
                break;

            case EJS_SECT_CLASS:
                lstClass(mp, lst->module, lst->slotNum, lst->type, lst->attributes);
                count++;
                break;

            case EJS_SECT_DEPENDENCY:
                lstDependency(mp, lst->module, lst->dependency);
                break;

            case EJS_SECT_EXCEPTION:
                lstException(mp, lst->module, lst->fun);
                break;

            case EJS_SECT_FUNCTION:
                lstFunction(mp, lst->module, lst->owner, lst->slotNum, lst->qname, lst->fun, lst->attributes);
                count++;
                break;

            case EJS_SECT_PROPERTY:
                lstProperty(mp, lst->module, lst->owner, lst->slotNum, lst->qname, lst->attributes, lst->typeName);
                count++;
                break;

            default:
            case EJS_SECT_START:
            case EJS_SECT_END:
                assert(0);
                break;
            }
        }
        if (count > 0) {
            lstEndModule(mp, module);
        }
    }
    mprCloseFile(mp->file);
    mp->file = 0;
}


static int lstOpen(EjsMod *mp, char *moduleFilename, EjsModuleHdr *hdr)
{
    char    *path, *name, *ext;

    assert(mp);

    name = mprGetPathBase(moduleFilename);
    if ((ext = strstr(name, EJS_MODULE_EXT)) != 0) {
        *ext = '\0';
    }
    path = sjoin(name, EJS_LISTING_EXT, NULL);
    path = mprJoinPath(mp->outputDir, path);
    if ((mp->file = mprOpenFile(path,  O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0664)) == 0) {
        mprLog("Cannot create %s", 0, path);
        return EJS_ERR;
    }
    mprEnableFileBuffering(mp->file, 0, 0);
    mprFprintf(mp->file, "#\n#  %s -- Module Listing for %s\n#\n", path, moduleFilename);
    return 0;
}


static void lstBlock(EjsMod *mp, EjsModule *module, EjsObj *owner, int slotNum, EjsString *name, int numProp)
{
    EjsString   *blockName;

    blockName = getBlockName(mp, owner, slotNum);
    mprFprintf(mp->file, "BLOCK:      [%@-%02d]  %@ (Slots %d)\n", blockName, slotNum, name, numProp);
}


/*
    List a class (type)
 */
static void lstClass(EjsMod *mp, EjsModule *module, int slotNum, EjsType *klass, int attributes)
{
    Ejs         *ejs;

    ejs = mp->ejs;
    mprFprintf(mp->file, "\n");

    if (klass->baseType) {
        mprFprintf(mp->file, "CLASS:      %sclass %@ extends %@\n", getAttributeString(mp, attributes), 
            klass->qname.name, klass->baseType->qname.name);
    } else {
        mprFprintf(mp->file, "CLASS:      %sclass %@\n", getAttributeString(mp, attributes), klass->qname.name);
    }
    leadin(mp, module, 1, 0);
    mprFprintf(mp->file, 
        "        #  Class Details: %d class traits, %d prototype (instance) traits, %s, requested slot %d\n",
        ejsGetLength(ejs, (EjsObj*) klass),
        klass->prototype ? ejsGetLength(ejs, klass->prototype) : 0, 
        klass->hasInstanceVars ? "has-state": "", slotNum);
}


static void lstDependency(EjsMod *mp, EjsModule *module, EjsModule *dependant)
{
    leadin(mp, module, 0, 0);
    mprFprintf(mp->file, "DEPENDENCY: require %@ (sum %d)\n\n", dependant->vname, dependant->checksum);
}


static void lstEndModule(EjsMod *mp, EjsModule *module)
{
    char    *pp, *end;
    ssize   size;
    int     i;

    assert(mp);

    mprFprintf(mp->file,
        "\n----------------------------------------------------------------------------------------------\n");

    lstSlotAssignments(mp, module, NULL, 0, mp->ejs->global);

    /*
        Dump the constant pool
     */
    size = module->constants->poolLength;
    mprFprintf(mp->file,
        "\n----------------------------------------------------------------------------------------------\n"
        "#\n"
        "#  Constant Pool (size %d bytes)\n"
        "#\n", size);

    pp = (char*) module->constants->pool;
    end = &((char*) module->constants->pool)[size];
    for (i = 0; pp < end; i++) {
        mprFprintf(mp->file, "%04d   \"%s\"\n", i, pp);
        pp += strlen(pp) + 1;
    }
}


static void lstFunction(EjsMod *mp, EjsModule *module, EjsObj *block, int slotNum, EjsName qname, EjsFunction *fun, 
        int attributes)
{
    Ejs         *ejs;
    EjsTrait    *trait;
    EjsName     lname;
    EjsType     *resultType;
    EjsPot      *activation;
    EjsString   *space, *blockName;
    int         i, numLocals, numProp;

    ejs = mp->ejs;
    activation = fun->activation;
    numProp = activation ? activation->numProp : 0;
    space = mapSpace(ejs, qname.space);

    mprFprintf(mp->file,  "\nFUNCTION:   ");

    /*
        Do the function declaration
     */
    if (attributes) {
        if (slotNum < 0) {
            /* Special just for global initializers */
            mprFprintf(mp->file,  "[initializer]  %@ %sfunction %@(", space, getAttributeString(mp, attributes), 
                qname.name);
        } else {
            blockName = getBlockName(mp, block, slotNum);
            mprFprintf(mp->file,  "[%@-%02d]  %@ %sfunction %@(", blockName, slotNum, space,
                getAttributeString(mp, attributes), qname.name);
        }
    } else {
        blockName = getBlockName(mp, block, slotNum);
        mprFprintf(mp->file,  "[%@-%02d]  %@ function %@(", blockName, slotNum, space, qname.name);
    }

    for (i = 0; i < (int) fun->numArgs; ) {
        lname = ejsGetPropertyName(ejs, activation, i);
        trait = ejsGetPropertyTraits(ejs, activation, i);
        if (trait->type) {
            mprFprintf(mp->file,  "%@: %@", lname.name, trait->type->qname.name);
        } else {
            mprFprintf(mp->file,  "%@", lname.name);
        }
        if (++i < (int) fun->numArgs) {
            mprFprintf(mp->file,  ", ");
        }
    }

    resultType = fun->resultType;
    mprFprintf(mp->file,  ") : %@\n", resultType ? resultType->qname.name : EST(Void)->qname.name);

    /*
        Repeat the args
     */
    for (i = 0; i < (int) fun->numArgs; i++) {
        lname = ejsGetPropertyName(ejs, activation, i);
        trait = ejsGetPropertyTraits(ejs, activation, i);
        mprFprintf(mp->file,  "     ARG:   [arg-%02d]   %@ %@", i, lname.space, lname.name);
        if (trait->type) {
            mprFprintf(mp->file,  " : %@", trait->type->qname.name);
        }
        mprFprintf(mp->file,  "\n");
    }
    numLocals = numProp - fun->numArgs;
    for (i = 0; i < numLocals; i++) {
        lname = ejsGetPropertyName(ejs, activation, i + fun->numArgs);
        trait = ejsGetPropertyTraits(ejs, activation, i + fun->numArgs);
        mprFprintf(mp->file,  "   LOCAL:   [local-%02d]  var %@", i + fun->numArgs, lname.name);
        if (trait->type) {
            mprFprintf(mp->file,  " : %@", trait->type->qname.name);
        }
        mprFprintf(mp->file,  "\n");
    }
    if (fun->body.code) {
        mprFprintf(mp->file,  "\n");
        interp(mp, module, fun);
    }
    mprFprintf(mp->file,  "\n");
}


void lstException(EjsMod *mp, EjsModule *module, EjsFunction *fun)
{
    Ejs             *ejs;
    EjsEx           *ex;
    EjsCode         *code;
    cchar           *exKind;
    int             i;

    ejs = mp->ejs;
    code = fun->body.code;

    if (code->numHandlers <= 0) {
        return;
    }
    mprFprintf(mp->file,
        "\n"
        "#\n"
        "#  Exception Section\n"
        "#    Kind     TryStart TryEnd  HandlerStart  HandlerEnd   CatchType\n"
        "#\n");

    for (i = 0; i < code->numHandlers; i++) {
        ex = code->handlers[i];

        if (ex->flags & EJS_EX_FINALLY) {
            exKind  = "finally";
        } else if (ex->flags & EJS_EX_ITERATION) {
            exKind  = "iteration";
        } else if (ex->flags & EJS_EX_CATCH) {
            exKind = "catch";
        } else {
            exKind = "unknown";
        }
        mprFprintf(mp->file,
            "%-3d %-10s %5d   %5d      %5d        %5d       %@\n",
            i, exKind, ex->tryStart, ex->tryEnd, ex->handlerStart, ex->handlerEnd,
            ex->catchType ? ex->catchType->qname.name : ESV(empty));
    }
    mprFprintf(mp->file, "\n");
}


static void lstProperty(EjsMod *mp, EjsModule *module, EjsObj *block, int slotNum, EjsName qname, int attributes, 
        EjsName typeName)
{
    Ejs         *ejs;
    EjsType     *propType;
    EjsString   *space, *blockName;

    ejs = mp->ejs;
    space = mapSpace(ejs, qname.space);
    mprFprintf(mp->file, "VARIABLE:   ");

    blockName = getBlockName(mp, block, slotNum);
    mprFprintf(mp->file, "[%@-%02d]  %@ %svar %@", blockName, slotNum, space,
        getAttributeString(mp, attributes), qname.name);

    if (typeName.name && typeName.name->value[0]) {
        mprFprintf(mp->file, " : %@", typeName.name);
    }
    mprFprintf(mp->file, "\n");

    if (block == 0) {
        /*
            Nested block.
         */
        if (typeName.name) {
            propType = (EjsType*) ejsGetPropertyByName(ejs, ejs->global, typeName);
        } else {
            propType = 0;
        }
        assert(mp->currentBlock && ejsIsBlock(ejs, mp->currentBlock));
        slotNum = ejsDefineProperty(ejs, (EjsObj*) mp->currentBlock, -1, qname, propType, attributes, 0);
    }
}


static void lstModule(EjsMod *mp, EjsModule *module)
{
    mprFprintf(mp->file,
        "\n==============================================================================================\n\n"
        "MODULE:   %@", module->vname);

    if (module->hasInitializer) {
        mprFprintf(mp->file, " <%s>\n", module->hasInitializer ? "hasInitializer, " : "");
    }
    mprFprintf(mp->file, "\n");
}


static int decodeOperands(EjsMod *mp, EjsOptable *opt, char *argbuf, int argbufLen, int address, int *stackEffect)
{
    EjsString   *sval;
    int         *argp;
    char        *bufp;
    uchar       *start;
    double      dval;
    int         i, argc, ival, len, buflen, j, numEntries;

    *stackEffect = opt->stackEffect;

    /*
        Keep a local progressive pointer into the argbuf and a length of the remaining room in the buffer.
     */
    *argbuf = '\0';
    bufp = argbuf;
    buflen = argbufLen;

    for (argc = 0, argp = opt->args; *argp; argc++, argp++) ;

    start = mp->pc;
    ival = 0;

    for (i = 0, argp = opt->args; i < argc; i++) {
        switch (opt->args[i]) {
        case EBC_NONE:
            break;

        case EBC_BYTE:
            ival = getByte(mp);
            fmt(bufp, buflen,  "<%d> ", ival);
            break;

        case EBC_DOUBLE:
            dval = getDouble(mp);
            fmt(bufp, buflen,  "<%f> ", dval);
            break;

        case EBC_ARGC:
        case EBC_ARGC2:
            ival = (int) getNum(mp);
            fmt(bufp, buflen,  "<argc: %d> ", ival);
            break;

        case EBC_ARGC3:
            ival = (int) getNum(mp);
            fmt(bufp, buflen,  "<argc: %d> ", ival);
            break;

        case EBC_NEW_ARRAY:
            ival = (int) getNum(mp);     /* argc */
            fmt(bufp, buflen,  "<argc: %d>", ival);
            bufp += strlen(bufp);
            *stackEffect -= (ival * 2);
            break;

        case EBC_NEW_OBJECT:
            ival = (int) getNum(mp);     /* argc */
            fmt(bufp, buflen,  "<argc: %d> <att: ", ival);
            bufp += strlen(bufp);
            for (j = 0; j < ival; j++) {
                /* Discard attributes */ getNum(mp);
                fmt(bufp, buflen,  "%d ", ival);
                len = (int) strlen(bufp);
                bufp += len;
                buflen -= len;
            }
            fmt(bufp, buflen,  ">", ival);
            *stackEffect -= (ival * 3);
            break;

        case EBC_SLOT:
            ival = (int) getNum(mp);
            fmt(bufp, buflen,  "<slot: %d> ", ival);
            break;

        case EBC_NUM:
            ival = (int) getNum(mp);
            fmt(bufp, buflen,  "<%d> ", ival);
            break;

        case EBC_JMP8:
            ival = getByte(mp);
            fmt(bufp, buflen,  "<addr: %d> ", ((char) ival) + address + 1);
            break;

        case EBC_JMP:
            ival = getInt32(mp);
            fmt(bufp, buflen,  "<addr: %d> ", ival + address + 4);
            break;

        case EBC_INIT_DEFAULT8:
            numEntries = getByte(mp);
            fmt(bufp, buflen,  "<%d> ", numEntries);
            len = (int) strlen(bufp);
            bufp += len;
            buflen -= len;
            for (j = 0; j < numEntries; j++) {
                ival = getByte(mp);
                fmt(bufp, buflen,  "<%d> ", ival + 2);
                len = (int) strlen(bufp);
                bufp += len;
                buflen -= len;
            }
            break;

        case EBC_INIT_DEFAULT:
            numEntries = getByte(mp);
            fmt(bufp, buflen,  "<%d> ", numEntries);
            len = (int) strlen(bufp);
            bufp += len;
            buflen -= len;
            for (j = 0; j < numEntries; j++) {
                ival = getInt32(mp);
                fmt(bufp, buflen,  "<%d> ", ival + 2);
                len = (int) strlen(bufp);
                bufp += len;
                buflen -= len;
            }
            break;

        case EBC_STRING:
            sval = getString(mp->ejs, mp);
            assert(sval);
            fmt(bufp, buflen,  "<%@> ", sval);
            break;

        case EBC_GLOBAL:
            getGlobal(mp, bufp, buflen);
            break;

        default:
            mprLog("ejs", 0, "Bad arg type in opcode table");
            break;
        }
        len = (int) strlen(bufp);
        bufp += len;
        buflen -= len;

        if (opt->args[i] == EBC_ARGC) {
            *stackEffect -= ival;
        } else if (opt->args[i] == EBC_ARGC2) {
            *stackEffect -= (ival * 2);
        } else if (opt->args[i] == EBC_ARGC3) {
            *stackEffect -= (ival * 3);
        }
        if (i == 0 && opt->stackEffect == EBC_POPN) {
            *stackEffect = -ival;
        }
    }
    return (int) (mp->pc - start);
}


/*
    Interpret the code for a function
 */
static void interp(EjsMod *mp, EjsModule *module, EjsFunction *fun)
{
    EjsOptable  *optable, *opt;
    EjsCode     *code;
    wchar       *currentLine;
    uchar       *start;
    char        argbuf[ME_MAX_PATH], lineInfo[ME_MAX_PATH], name[ME_MAX_PATH];
    char        *currentFile, *src, *dest;
    int         maxOp, opcode, lineNumber, stack, codeLen, address, stackEffect, nbytes, i, lastLine;

    assert(mp);
    assert(module);
    assert(fun);

    /*
        Store so that getNum and getString can easily read instructions
     */
    code = fun->body.code;
    mp->fun = fun;
    mp->module = module;
    mp->pc = code->byteCode;
    codeLen = code->codeLen;
    start = mp->pc;
    stack = 0;
    currentLine = 0;
    lineNumber = 0;
    currentFile = 0;
    lastLine = -1;

    optable = ejsGetOptable(mp);

    for (maxOp = 0, opt = optable; opt->name; opt++) {
        maxOp++;
    }
    
    while ((mp->pc - start) < codeLen) {
        address = (int) (mp->pc - start);
        opcode = *mp->pc++;
        argbuf[0] = '\0';
        stackEffect = 0;

        if (opcode < 0 || opcode >= maxOp) {
            mprLog("ejs listing", 0, "Bad opcode %x at address %d.\n", opcode, address);
            return;
        }
        opt = &optable[opcode];

        if (mp->showAsm) {
            /*
                Output address [stack] opcode
                Format:  "address: [stackDepth] opcode <args> ..."
             */
            mprFprintf(mp->file,  "    %04d: [%d] %02x ", address, stack, opcode);
        }
        nbytes = decodeOperands(mp, opt, argbuf, (int) sizeof(argbuf), (int) (mp->pc - start), &stackEffect);

        if (mp->showAsm) {
            for (i = 24 - (nbytes * 3); i >= 0; i--) {
                mprFprintf(mp->file, ".");
            }
            for (dest = name, src = opt->name; *src; src++, dest++) {
                if (*src == '_') {
                    *dest = *++src;
                } else {
                    *dest = tolower((uchar) *src);
                }
            }
            *dest++ = '\0';
            mprFprintf(mp->file,  " %s %s\n", name, argbuf);

        } else {
            for (i = 24 - (nbytes * 3); i >= 0; i--) {
                mprFprintf(mp->file, " ");
            }
            mprFprintf(mp->file,  " %s\n", argbuf);
        }
        stack += stackEffect;

        if (opcode == EJS_OP_RETURN_VALUE || opcode == EJS_OP_RETURN) {
            stack = 0;
        }
        if (stack < 0) {
            if (mp->warnOnError) {
                mprEprintf("Instruction stack is negative %d\n", stack);
            }
            if (mp->exitOnError) {
                exit(255);
            }
        }
        ejsGetDebugInfo(mp->ejs, fun, mp->pc, &currentFile, &lineNumber, &currentLine);
        if (currentFile && currentLine && *currentLine && lineNumber != lastLine) {
            fmt(lineInfo, sizeof(lineInfo), "%s:%d", currentFile, lineNumber);
            mprFprintf(mp->file, "\n    # %-25s %w\n\n", lineInfo, currentLine);
            lastLine = lineNumber;
        }
    }
}


static void lstVarSlot(EjsMod *mp, EjsModule *module, EjsName *qname, EjsTrait *trait, int slotNum)
{
    Ejs         *ejs;
    EjsString   *space;

    assert(slotNum >= 0);
    assert(qname);

    ejs = mp->ejs;
    space = mapSpace(ejs, qname->space);

    if (qname->name == 0 || qname->name->value[0] == '\0') {
        mprFprintf(mp->file, "%04d    <inherited>\n", slotNum);

    } else if (trait && trait->type) {
        if (trait->type == EST(Function)) {
            mprFprintf(mp->file, "%04d    %@ function %@\n", slotNum, space, qname->name);

        } else {
            mprFprintf(mp->file, "%04d    %@ var %@: %@\n", slotNum, space, qname->name, trait->type->qname.name);
        }

    } else {
        mprFprintf(mp->file, "%04d    %@ var %@\n", slotNum, space, qname->name);
    }
}


/*
    List the various property slot assignments
 */
static void lstSlotAssignments(EjsMod *mp, EjsModule *module, EjsObj *parent, int slotNum, EjsObj *obj)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsType         *type;
    EjsObj          *vp;
    EjsPot          *prototype;
    EjsFunction     *fun;
    EjsBlock        *block;
    EjsName         qname;
    int             i, numInherited, count;

    assert(obj);
    assert(module);

    ejs = mp->ejs;
    if (VISITED(obj)) {
        return;
    }
    SET_VISITED(obj, 1);

    if (obj == ejs->global) {
        mprFprintf(mp->file,  "\n#\n"
            "#  Global slot assignments (Num prop %d)\n"
            "#\n", ejsGetLength(ejs, obj));

        /*
            List slots for global
         */
        for (i = module->firstGlobal; i < module->lastGlobal; i++) {
            trait = ejsGetPropertyTraits(ejs, ejs->global, i);
            qname = ejsGetPropertyName(ejs, ejs->global, i);
            if (qname.name == 0) {
                continue;
            }
            lstVarSlot(mp, module, &qname, trait, i);
        }

        /*
            List slots for the initializer
         */
        fun = (EjsFunction*) module->initializer;
        if (fun) {
            mprFprintf(mp->file,  "\n#\n"
                "#  Initializer slot assignments (Num prop %d)\n"
                "#\n", ejsGetLength(ejs, (EjsObj*) fun));

            count = ejsGetLength(ejs, (EjsObj*) fun);
            for (i = 0; i < count; i++) {
                trait = ejsGetPropertyTraits(ejs, (EjsObj*) fun, i);
                qname = ejsGetPropertyName(ejs, (EjsObj*) fun, i);
                if (qname.name == 0) {
                    continue;
                }
                assert(trait);
                lstVarSlot(mp, module, &qname, trait, i);
            }
        }

    } else if (ejsIsFunction(ejs, obj)) {
        fun = (EjsFunction*) obj;
        count = ejsGetLength(ejs, (EjsObj*) obj);
        if (count > 0) {
            mprFprintf(mp->file,  "\n#\n"
                "#  Local slot assignments for the \"%@\" function (Num slots %d)\n"
                "#\n", fun->name, count);

            for (i = 0; i < count; i++) {
                trait = ejsGetPropertyTraits(ejs, obj, i);
                assert(trait);
                qname = ejsGetPropertyName(ejs, obj, i);
                lstVarSlot(mp, module, &qname, trait, i);
            }
        }

    } else if (ejsIsType(ejs, obj)) {
        /*
            Types
         */
        type = (EjsType*) obj;
        mprFprintf(mp->file,  "\n#\n"
            "#  Class slot assignments for the \"%@\" class (Num slots %d)\n"
            "#\n", type->qname.name,
            ejsGetLength(ejs, (EjsObj*) type));

        count = ejsGetLength(ejs, (EjsObj*) type);
        for (i = 0; i < count; i++) {
            trait = ejsGetPropertyTraits(ejs, (EjsObj*) type, i);
            assert(trait);
            qname = ejsGetPropertyName(ejs, obj, i);
            lstVarSlot(mp, module, &qname, trait, i);
        }

        prototype = type->prototype;
        if (type->baseType && type->baseType->prototype) {
            numInherited = ejsGetLength(ejs, type->baseType->prototype);
        } else {
            numInherited = 0;
        }
        mprFprintf(mp->file,  "\n#\n"
            "#  Instance slot assignments for the \"%@\" class (Num prop %d, num inherited %d)\n"
            "#\n", type->qname.name,
            prototype ? ejsGetLength(ejs, prototype): 0, numInherited);

        if (prototype) {
            count = ejsGetLength(ejs, prototype);
            for (i = 0; i < count; i++) {
                trait = ejsGetPropertyTraits(ejs, prototype, i);
                assert(trait);
                qname = ejsGetPropertyName(ejs, prototype, i);
                if (qname.name) {
                    lstVarSlot(mp, module, &qname, trait, i);
                }
            }
        }

    } else if (ejsIsBlock(ejs, obj)) {
        qname = ejsGetPropertyName(ejs, parent, slotNum);
        block = (EjsBlock*) obj;
        count = ejsGetLength(ejs, (EjsObj*) block);
        if (count > 0) {
            mprFprintf(mp->file,  
                "\n#\n"
                "#  Block slot assignments for the \"%@\" (Num slots %d)\n"
                "#\n", qname.name, ejsGetLength(ejs, obj));
            
            count = ejsGetLength(ejs, obj);
            for (i = 0; i < count; i++) {
                trait = ejsGetPropertyTraits(ejs, obj, i);
                assert(trait);
                qname = ejsGetPropertyName(ejs, obj, i);
                lstVarSlot(mp, module, &qname, trait, i);
            }
        }
    }

    /*
        Now recurse on types, functions and blocks
     */
    if (obj == ejs->global) {
        i = module->firstGlobal;
        count = module->lastGlobal;
    } else {
        i = 0;
        count = ejsGetLength(ejs, obj);
    }
    for (; i < count; i++) {
        qname = ejsGetPropertyName(ejs, obj, i);
        vp = ejsGetProperty(ejs, obj, i);
        if (vp == 0) {
            continue;
        }
        if (ejsIsType(ejs, vp) || ejsIsFunction(ejs, vp) || ejsIsBlock(ejs, vp)) {
            lstSlotAssignments(mp, module, obj, i, vp);
        }
    }
    SET_VISITED(obj, 0);
}


static EjsString *getBlockName(EjsMod *mp, EjsObj *block, int slotNum)
{
    EjsName         qname;

    if (block) {
        if (ejsIsType(mp->ejs, block)) {
            return ((EjsType*) block)->qname.name;

        } else if (ejsIsFunction(mp->ejs, block)) {
            return ((EjsFunction*) block)->name;
        }
    }
    qname = ejsGetPropertyName(mp->ejs, block, slotNum);
    return qname.name;
}


static char *getAttributeString(EjsMod *mp, int attributes)
{
    static char attributeBuf[256];

    attributeBuf[0] = '\0';

    /*
        Order to look best
     */
    if (attributes & EJS_PROP_NATIVE) {
        strcat(attributeBuf, "native ");
    }
    if (attributes & EJS_TRAIT_READONLY) {
        strcat(attributeBuf, "const ");
    }
    if (attributes & EJS_PROP_STATIC) {
        strcat(attributeBuf, "static ");
    }
    if (attributes & EJS_TYPE_FINAL) {
        strcat(attributeBuf, "final ");
    }
    if (attributes & EJS_FUN_OVERRIDE) {
        strcat(attributeBuf, "override ");
    }
    if (attributes & EJS_TYPE_DYNAMIC_INSTANCES) {
        strcat(attributeBuf, "dynamic ");
    }
    if (attributes & EJS_TRAIT_GETTER) {
        strcat(attributeBuf, "get ");
    }
    if (attributes & EJS_TRAIT_SETTER) {
        strcat(attributeBuf, "set ");
    }
    return attributeBuf;
}


static uchar getByte(EjsMod *mp)
{
    if (mp->showAsm) {
        mprFprintf(mp->file, "%02x ",  mp->pc[0] & 0xFF);
    }
    return *mp->pc++ & 0xFF;
}


static uint getInt32(EjsMod *mp)
{
    uchar   *start;
    uint    value;

    start = mp->pc;
    value = ejsDecodeInt32(mp->ejs, &mp->pc);

    if (mp->showAsm) {
        for (; start < mp->pc; start++) {
            mprFprintf(mp->file, "%02x ", *start & 0xff);
        }
    }
    return value;
}


static double getDouble(EjsMod *mp)
{
    uchar   *start;
    double  value;

    start = mp->pc;
    value = ejsDecodeDouble(mp->ejs, &mp->pc);

    if (mp->showAsm) {
        for (; start < mp->pc; start++) {
            mprFprintf(mp->file, "%02x ", *start & 0xff);
        }
    }
    return value;
}


/*
    Get an encoded number
 */
static int64 getNum(EjsMod *mp)
{
    uchar       *start;
    int64       value;

    start = mp->pc;
    value = ejsDecodeNum(mp->ejs, &mp->pc);

    if (mp->showAsm) {
        for (; start < mp->pc; start++) {
            mprFprintf(mp->file, "%02x ", *start & 0xff);
        }
    }
    return value;
}


/*
    Read an interned string constants are stored as token offsets into the constant pool. The pool contains null
    terminated UTF-8 strings.
 */
static EjsString *getString(Ejs *ejs, EjsMod *mp)
{
    int         number;

    number = (int) getNum(mp);
    if (number < 0) {
        assert(number >= 0);
        return 0;
    }
    return ejsCreateStringFromConst(ejs, mp->module, number);
}


/*
    Return the length of bytes added to buf
 */
static void getGlobal(EjsMod *mp, char *buf, int buflen)
{
    Ejs             *ejs;
    EjsName         qname;
    EjsObj          *vp;
    int             t, slotNum;

    ejs = mp->ejs;
    vp = 0;

    if ((t = (int) getNum(mp)) < 0) {
        fmt(buf, buflen,  "<can't read code>");
        return;
    }

    switch (t & EJS_ENCODE_GLOBAL_MASK) {
    default:
        assert(0);
        return;

    case EJS_ENCODE_GLOBAL_NOREF:
        return;

    case EJS_ENCODE_GLOBAL_SLOT:
        /*
            Type is a builtin primitive type or we are binding globals.
         */
        slotNum = t >> 2;
        if (0 <= slotNum && slotNum < ejsGetLength(ejs, ejs->global)) {
            vp = ejsGetProperty(ejs, ejs->global, slotNum);
        }
        if (vp && ejsIsType(ejs, vp)) {
            fmt(buf, buflen, "<type: 0x%x,  %N> ", t, ((EjsType*) vp)->qname);
        }
        break;

    case EJS_ENCODE_GLOBAL_NAME:
        /*
            Type was unknown at compile time
         */
        qname.name = ejsCreateStringFromConst(ejs, mp->module, t >> 2);
        if (qname.name == 0) {
            assert(0);
            fmt(buf, buflen,  "<var: 0x%x,  missing name> ", t);
            return;
        }
        if ((qname.space = getString(mp->ejs, mp)) == 0) {
            fmt(buf, buflen,  "<var: 0x%x,  missing namespace> ", t);
            return;
        }
        if (qname.name) {
            vp = ejsGetPropertyByName(ejs, ejs->global, qname);
        }
        fmt(buf, buflen, "<var: 0x%x,  %N> ", t, qname);
        break;
    }

    if (vp == 0) {
        fmt(buf, buflen, "<var: %d,  cannot resolve var/typ at slot e> ", t);
    }
}


static void leadin(EjsMod *mp, EjsModule *module, int classDec, int inFunction)
{
    mprFprintf(mp->file, "    ");
}


static EjsString *mapSpace(Ejs *ejs, EjsString *space)
{
    if (ejsContainsAsc(ejs, space, "internal-") != 0) {
        return ejsCreateStringFromAsc(ejs, "internal");
    }
    return space;
}


/*
    @copy   default

    Copyright (c) Embedthis Software. All Rights Reserved.

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
