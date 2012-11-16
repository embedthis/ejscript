/**
    ejsModuleWrite.c - Routines to encode and emit Ejscript byte code.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejsCompiler.h"

/****************************** Forward Declarations **************************/

static void createBlockSection(EcCompiler *cp, EjsPot *block, int slotNum, EjsBlock *vp);
static void createClassSection(EcCompiler *cp, EjsPot *block, int slotNum, EjsPot *klass);
static void createDebugSection(EcCompiler *cp, EjsFunction *fun);
static void createDependencySection(EcCompiler *cp);
static void createDocSection(EcCompiler *cp, cchar *tag, EjsPot *block, int slotNum);
static void createExceptionSection(EcCompiler *cp, EjsFunction *mp);
static void createFunctionSection(EcCompiler *cp, EjsPot *block, int slotNum, EjsFunction *fun, int isSetter);
static void createGlobalProperties(EcCompiler *cp);
static void createGlobalType(EcCompiler *cp, EjsType *klass);
static void createPropertySection(EcCompiler *cp, EjsPot *block, int slotNum, EjsObj *vp);
static void createSection(EcCompiler *cp, EjsPot *block, int slotNum);
static int reserveRoom(EcCompiler *cp, int room);

static int sumNum(int value);
static int sumString(EjsString *name);

/*********************************** Code *************************************/
/*
    Write out the module file header
 */
PUBLIC int ecCreateModuleHeader(EcCompiler *cp)
{
    EjsModuleHdr    hdr;

    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = ejsSwapInt32(cp->ejs, EJS_MODULE_MAGIC);
    hdr.fileVersion = ejsSwapInt32(cp->ejs, EJS_MODULE_VERSION);
    ecEncodeBlock(cp, (uchar*) &hdr, sizeof(hdr));
    return (cp->fatalError) ? EJS_ERR : 0;
}


/*
    Create a module section. This writes all classes, functions, variables and blocks contained by the module.
 */
PUBLIC int ecCreateModuleSection(EcCompiler *cp)
{
    EjsConstants    *constants;
    EjsModule       *mp;
    EcState         *state;
    MprBuf          *buf;
    int             checksumOffset;

    state = cp->state;
    buf = state->code->buf;
    mp = state->currentModule;
    constants = mp->constants;

    mprLog(7, "Create module section %s", mp->name);

    ecEncodeByte(cp, EJS_SECT_MODULE);
    ecEncodeConst(cp, mp->name);
    ecEncodeNum(cp, mp->version);

    /*
        Remember this location for the module checksum. Reserve 4 bytes.
     */
    checksumOffset = (int) (mprGetBufEnd(buf) - buf->data);
    ecEncodeInt32(cp, 0);

    /*
        Write the constant pool and lock it against further updates.
     */
    mp->constants->locked = 1;
    ecEncodeNum(cp, constants->poolLength);
    ecEncodeNum(cp, constants->indexCount);
    ecEncodeBlock(cp, (uchar*) constants->pool, (int) constants->poolLength);

    createDependencySection(cp);
    if (mp->hasInitializer) {
        createFunctionSection(cp, NULL, -1, mp->initializer, 0);
    }
    createGlobalProperties(cp);
    ecEncodeByte(cp, EJS_SECT_MODULE_END);

    if (cp->fatalError) {
        return MPR_ERR_CANT_WRITE;
    }
    mp->checksum += (sumString(mp->name) & EJS_ENCODE_MAX_WORD);
    ecEncodeInt32AtPos(cp, checksumOffset, mp->checksum);
    return 0;
}


static void createDependencySection(EcCompiler *cp)
{
    EjsModule   *module, *mp;
    int         i, count, version;

    mp = cp->state->currentModule;
    assure(mp);

    /*
        If merging, don't need references to dependent modules as they are aggregated onto the output
     */
    if (mp->dependencies && !cp->merge) {
        count = mprGetListLength(mp->dependencies);
        for (i = 0; i < count; i++) {
            module = (EjsModule*) mprGetItem(mp->dependencies, i);

            if (module->compiling && cp->outputFile) {
                continue;
            }
            if (mp->name == module->name) {
                /* A module can't depend on itself */
                continue;
            }
            ecEncodeByte(cp, EJS_SECT_DEPENDENCY);
            ecEncodeConst(cp, module->name);
            ecEncodeNum(cp, module->checksum);

            if (!cp->bind) {
                ecEncodeNum(cp, module->minVersion);
                ecEncodeNum(cp, module->maxVersion);
            } else {
                version = EJS_MAKE_COMPAT_VERSION(module->version);
                ecEncodeNum(cp, version);
                ecEncodeNum(cp, version);
            }
            if (cp->fatalError) {
                return;
            }
            mp->checksum += sumString(module->name);
            mprLog(7, "    dependency section for %s from module %s", module->name, mp->name);
        }
    }
}


/*
    Emit all global classes, functions, variables and blocks.
 */
static void createGlobalProperties(EcCompiler *cp)
{
    Ejs             *ejs;
    EjsName         *prop;
    EjsModule       *mp;
    EjsObj          *vp;
    int             next, slotNum;

    ejs = cp->ejs;
    mp = cp->state->currentModule;

    if (mp->globalProperties == 0) {
        return;
    }
    for (next = 0; (prop = (EjsName*) mprGetNextItem(mp->globalProperties, &next)) != 0; ) {
        slotNum = ejsLookupProperty(ejs, ejs->global, *prop);
        if (slotNum < 0) {
            cp->fatalError = 1;
            mprError("Code generation error. Cannot find global property %s.", prop->name);
            return;
        }
        vp = ejsGetProperty(ejs, ejs->global, slotNum);
        if (VISITED(vp)) {
            continue;
        }
        if (ejsIsType(ejs, vp)) {
            createGlobalType(cp, (EjsType*) vp);
        } else {
            createSection(cp, ejs->global, slotNum);
        }
    }
    for (next = 0; (prop = (EjsName*) mprGetNextItem(mp->globalProperties, &next)) != 0; ) {
        slotNum = ejsLookupProperty(ejs, ejs->global, *prop);
        vp = ejsGetProperty(ejs, ejs->global, slotNum);
        SET_VISITED(vp, 0);
    }
}


/*
    Recursively emit a class and its base classes
 */
static void createGlobalType(EcCompiler *cp, EjsType *type)
{
    Ejs             *ejs;
    EjsModule       *mp;
    EjsType         *iface;
    int             next, slotNum;

    ejs = cp->ejs;
    mp = cp->state->currentModule;

    if (VISITED(type) || type->module != mp) {
        return;
    }
    SET_VISITED(type, 1);

    if (type->baseType && !VISITED(type->baseType)) {
        createGlobalType(cp, type->baseType);
    }
    if (type->implements) {
        for (next = 0; (iface = mprGetNextItem(type->implements, &next)) != 0; ) {
            createGlobalType(cp, iface);
        }
    }
    slotNum = ejsLookupProperty(ejs, ejs->global, type->qname);
    assure(slotNum >= 0);

    createSection(cp, ejs->global, slotNum);
}


static void createSection(EcCompiler *cp, EjsPot *block, int slotNum)
{
    Ejs         *ejs;
    EjsTrait    *trait;
    EjsName     qname;
    EjsFunction *fun;
    EjsObj      *vp;

    ejs = cp->ejs;
    vp = ejsGetProperty(ejs, block, slotNum);
    qname = ejsGetPropertyName(ejs, block, slotNum);
    trait = ejsGetPropertyTraits(ejs, block, slotNum);

    /*
        hoistBlockVar will delete hoisted properties but will not (yet) compact to reclaim the slot.
     */
    if (slotNum < 0 || trait == 0 || vp == 0 || qname.name->value[0] == '\0') {
        return;
    }
    assure(qname.name);

    if (ejsIsType(ejs, vp)) {
        createClassSection(cp, block, slotNum, (EjsPot*) vp);

    } else if (ejsIsFunction(ejs, vp)) {
        fun = (EjsFunction*) vp;
        createFunctionSection(cp, block, slotNum, fun, 0);
        if (trait->attributes & EJS_TRAIT_SETTER) {
            assure(fun->setter);
            createFunctionSection(cp, block, slotNum, fun->setter, 1);
        }

    } else if (ejsIsBlock(ejs, vp)) {
        createBlockSection(cp, block, slotNum, (EjsBlock*) vp);

    } else {
        createPropertySection(cp, block, slotNum, vp);
    }
}


/*
    Create a type section in the module file.
 */
static void createClassSection(EcCompiler *cp, EjsPot *block, int slotNum, EjsPot *klass)
{
    Ejs             *ejs;
    EjsModule       *mp;
    EjsType         *type, *iface;
    EjsPot          *prototype;
    EjsTrait        *trait;
    EjsName         qname;
    int             next, attributes, interfaceCount, instanceTraits, count;

    ejs = cp->ejs;
    mp = cp->state->currentModule;

    createDocSection(cp, "class", ejs->global, slotNum);
    qname = ejsGetPropertyName(ejs, ejs->global, slotNum);
    assure(qname.name);

    mprLog(7, "    type section %@ for module %@", qname.name, mp->name);
    
    type = ejsGetProperty(ejs, ejs->global, slotNum);
    assure(type);
    assure(ejsIsType(ejs, type));

    ecEncodeByte(cp, EJS_SECT_CLASS);
    ecEncodeConst(cp, qname.name);
    ecEncodeConst(cp, qname.space);

    trait = ejsGetPropertyTraits(ejs, ejs->global, slotNum);
    attributes = (trait) ? trait->attributes : 0;
    attributes &= ~EJS_TYPE_FIXUP;

    if (type->hasConstructor) {
        attributes |= EJS_TYPE_HAS_CONSTRUCTOR;
    }
    if (type->hasInitializer) {
        attributes |= EJS_TYPE_HAS_TYPE_INITIALIZER;
    }
    if (type->hasInstanceVars) {
        attributes |= EJS_TYPE_HAS_INSTANCE_VARS;
    }
    if (type->callsSuper) {
        attributes |= EJS_TYPE_CALLS_SUPER;
    }
    if (type->implements) {
        for (next = 0; (iface = mprGetNextItem(type->implements, &next)) != 0; ) {
            if (!iface->isInterface) {
                attributes |= EJS_TYPE_FIXUP;
                break;
            }
        }
    }
    ecEncodeNum(cp, attributes);
    ecEncodeNum(cp, (cp->bind) ? slotNum : -1);

    assure(type != type->baseType);
    if (type->baseType) {
        ecEncodeGlobal(cp, (EjsObj*) type->baseType, type->baseType->qname);
    } else {
        ecEncodeNum(cp, EJS_ENCODE_GLOBAL_NOREF);
    }
    ecEncodeNum(cp, ejsGetLength(ejs, (EjsObj*) type));

    instanceTraits = ejsGetLength(ejs, (EjsObj*) type->prototype);
    assure(instanceTraits >= 0);
    ecEncodeNum(cp, instanceTraits);
    
    interfaceCount = (type->implements) ? mprGetListLength(type->implements) : 00;
    assure(interfaceCount >= 0);
    ecEncodeNum(cp, interfaceCount);

    if (type->implements) {
        for (next = 0; (iface = mprGetNextItem(type->implements, &next)) != 0; ) {
            ecEncodeGlobal(cp, (EjsObj*) iface, iface->qname);
        }
    }
    if (cp->fatalError) {
        return;
    }    
    if (type->hasConstructor) {
        assure(type->constructor.isConstructor);
        assure(type->constructor.block.pot.isFunction);
        createFunctionSection(cp, block, slotNum, (EjsFunction*) type, 0);
    }
    /*
        Loop over type traits
     */
    count = ejsGetLength(ejs, type); 
    for (slotNum = 0; slotNum < count; slotNum++) {
        createSection(cp, (EjsPot*) type, slotNum);
    }
    /*
        Loop over prototype (instance) properties.
     */
    prototype = type->prototype;
    if (prototype) {
        count = ejsGetLength(ejs, prototype);
        for (slotNum = 0; slotNum < count; slotNum++) {
            trait = ejsGetPropertyTraits(ejs, prototype, slotNum);
            if (slotNum < type->numInherited) {
                if (trait && !(trait->attributes & EJS_FUN_OVERRIDE)) {
                    continue;
                }
            }
            createSection(cp, prototype, slotNum);
        }
    }
    mp->checksum += sumNum(ejsGetLength(ejs, type) + instanceTraits + interfaceCount);
    mp->checksum += sumString(type->qname.name);
    ecEncodeByte(cp, EJS_SECT_CLASS_END);
}


/*
    NOTE: static methods and methods are both stored in the typeTraits.
    The difference is in how the methods are called by the VM op codes.
 */
static void createFunctionSection(EcCompiler *cp, EjsPot *block, int slotNum, EjsFunction *fun, int isSetter)
{
    Ejs             *ejs;
    EjsModule       *mp;
    EjsTrait        *trait;
    EjsName         qname;
    EjsCode         *code;
    EjsType         *resultType;
    EjsPot          *activation;
    int             i, attributes, numProp;

    assure(fun);

    mp = cp->state->currentModule;
    ejs = cp->ejs;
    activation = fun->activation;
    numProp = (activation) ? activation->numProp: 0;
    code = fun->body.code;
    
    if (block && slotNum >= 0) {
        qname = ejsGetPropertyName(ejs, block, slotNum);
        createDocSection(cp, "fun", block, slotNum);
        trait = ejsGetPropertyTraits(ejs, block, slotNum);
        attributes = trait->attributes;
        if (fun->isInitializer) {
            attributes |= EJS_FUN_INITIALIZER;
        }
        if (fun->moduleInitializer) {
            attributes |= EJS_FUN_MODULE_INITIALIZER;
        }
        if (trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) {
            if (isSetter) {
                attributes &= ~EJS_TRAIT_GETTER;
            } else {
                attributes &= ~EJS_TRAIT_SETTER;
            }
        }
    } else {
        attributes = EJS_FUN_MODULE_INITIALIZER;
        qname = N(EJS_EJS_NAMESPACE, EJS_INITIALIZER_NAME);
    }
    if (fun->isConstructor) {
        assure(fun->block.pot.isFunction);
        attributes |= EJS_FUN_CONSTRUCTOR;
    }
    if (fun->rest) {
        attributes |= EJS_FUN_REST_ARGS;
    }
    if (fun->fullScope) {
        attributes |= EJS_FUN_FULL_SCOPE;
    }
    if (fun->hasReturn) {
        attributes |= EJS_FUN_HAS_RETURN;
    }
    resultType = fun->resultType;

    ecEncodeByte(cp, EJS_SECT_FUNCTION);
    ecEncodeConst(cp, qname.name);
    ecEncodeConst(cp, qname.space);
    ecEncodeNum(cp, attributes);
    ecEncodeByte(cp, fun->strict);
    if (resultType) {
        ecEncodeGlobal(cp, resultType, resultType->qname);
    } else {
        ecEncodeNum(cp, EJS_ENCODE_GLOBAL_NOREF);
    }
    ecEncodeNum(cp, (cp->bind || (block != ejs->global)) ? slotNum: -1);
    ecEncodeNum(cp, numProp);
    ecEncodeNum(cp, fun->numArgs);
    ecEncodeNum(cp, fun->numDefault);

    if (code) {
        assure(code->codeLen > 0);
        ecEncodeNum(cp, code->numHandlers);
        ecEncodeNum(cp, code->codeLen);
        ecEncodeBlock(cp, code->byteCode, code->codeLen);
        if (code->numHandlers > 0) {
            createExceptionSection(cp, fun);
        }
        if (code->debug) {
            createDebugSection(cp, fun);
        }
    } else {
        ecEncodeNum(cp, 0);
        ecEncodeNum(cp, 0);
    }
    /*
        Recursively write args, locals and any nested functions and blocks.
     */
    attributes = 0;
    for (i = 0; i < numProp; i++) {
        createSection(cp, activation, i);
    }
    ecEncodeByte(cp, EJS_SECT_FUNCTION_END);
    mp->checksum += sumNum(fun->numArgs + numProp - fun->numArgs);
    if (code && code->numHandlers) {
        mp->checksum += sumNum(code->numHandlers);
    }
    if (ejsContainsAsc(ejs, qname.name, "--fun_") >= 0) {
        /* Don't sum the name for dynamic functions */
    } else {
        mp->checksum += sumString(qname.name);
    }
}


/*
    NOTE: static methods and methods are both stored in the typeTraits.
    The difference is in how the methods are called by the VM op codes.
 */
static void createExceptionSection(EcCompiler *cp, EjsFunction *fun)
{
    EjsEx       *ex;
    EjsCode     *code;
    int         i;

    assure(fun);

    code = fun->body.code;
    ecEncodeByte(cp, EJS_SECT_EXCEPTION);

    for (i = 0; i < code->numHandlers; i++) {
        ex = code->handlers[i];
        ecEncodeByte(cp, ex->flags);
        ecEncodeNum(cp, ex->tryStart);
        ecEncodeNum(cp, ex->tryEnd);
        ecEncodeNum(cp, ex->handlerStart);
        ecEncodeNum(cp, ex->handlerEnd);
        ecEncodeNum(cp, ex->numBlocks);
        ecEncodeNum(cp, ex->numStack);
        if (ex->catchType) {
            ecEncodeGlobal(cp, ex->catchType, ex->catchType->qname);
        } else {
            ecEncodeNum(cp, EJS_ENCODE_GLOBAL_NOREF);
        }
    }
}


static void createDebugSection(EcCompiler *cp, EjsFunction *fun)
{
    EjsDebug    *debug;
    EjsLine     *line;
    int         i, patchSizeOffset, startDebug;

    assure(fun);
    debug = fun->body.code->debug;

    ecEncodeByte(cp, EJS_SECT_DEBUG);
    patchSizeOffset = ecGetCodeOffset(cp);
    ecEncodeInt32(cp, 0);

    startDebug = ecGetCodeOffset(cp);
    ecEncodeNum(cp, debug->numLines);
    for (i = 0; i < debug->numLines; i++) {
        line = &debug->lines[i];
        ecEncodeNum(cp, line->offset);
        ecEncodeWideAsMulti(cp, line->source);
    }
    ecEncodeInt32AtPos(cp, patchSizeOffset, ecGetCodeOffset(cp) - startDebug);
}


static void createBlockSection(EcCompiler *cp, EjsPot *parent, int slotNum, EjsBlock *block)
{
    Ejs             *ejs;
    EjsName         qname;
    int             i;

    ejs = cp->ejs;
    ecEncodeByte(cp, EJS_SECT_BLOCK);
    qname = ejsGetPropertyName(ejs, parent, slotNum);
    ecEncodeConst(cp, qname.name);
    ecEncodeNum(cp, (cp->bind || (block != ejs->global)) ? slotNum : -1);
    ecEncodeNum(cp, block->pot.numProp);

    /*
        Now emit all the properties
     */
    for (i = 0; i < block->pot.numProp; i++) {
        createSection(cp, (EjsPot*) block, i);
    }
    ecEncodeByte(cp, EJS_SECT_BLOCK_END);
}


static void createPropertySection(EcCompiler *cp, EjsPot *block, int slotNum, EjsObj *vp)
{
    Ejs         *ejs;
    EjsTrait    *trait;
    EjsName     qname;
    EjsModule   *mp;
    int         attributes;

    ejs = cp->ejs;
    mp = cp->state->currentModule;
    qname = ejsGetPropertyName(ejs, block, slotNum);
    
    createDocSection(cp, "var", block, slotNum);

    assure(qname.name->value[0] != '\0');
    trait = ejsGetPropertyTraits(ejs, block, slotNum);
    attributes = trait->attributes;

    mprLog(7, "    global property section %@", qname.name);

    if (trait->type) {
        if (trait->type == EST(Namespace) || (!ejs->initialized && trait->type->qname.name == EST(Namespace)->qname.name)){
            attributes |= EJS_PROP_HAS_VALUE;
        }
    }
    ecEncodeByte(cp, EJS_SECT_PROPERTY);
    ecEncodeName(cp, qname);

    ecEncodeNum(cp, attributes);
    ecEncodeNum(cp, (cp->bind || (block != ejs->global)) ? slotNum : -1);
    if (trait->type) {
        ecEncodeGlobal(cp, trait->type, trait->type->qname);
    } else {
        ecEncodeNum(cp, EJS_ENCODE_GLOBAL_NOREF);
    }

    if (attributes & EJS_PROP_HAS_VALUE) {
        if (vp && ejsIs(ejs, vp, Namespace)) {
            ecEncodeConst(cp, ((EjsNamespace*) vp)->value);
        } else {
            ecEncodeConst(cp, 0);
        }
    }
    mp->checksum += sumString(qname.name);
}


static void createDocSection(EcCompiler *cp, cchar *tag, EjsPot *block, int slotNum)
{
    Ejs         *ejs;
    EjsDoc      *doc;
    char        key[32];

    ejs = cp->ejs;
    assure(slotNum >= 0);
    
    if (!(ejs->flags & EJS_FLAG_DOC)) {
        return;
    }
    if (ejs->doc == 0) {
        ejs->doc = mprCreateHash(EJS_DOC_HASH_SIZE, 0);
    }
    fmt(key, sizeof(key), "%s %Lx %d", tag, PTOL(block), slotNum);
    if ((doc = mprLookupKey(ejs->doc, key)) == 0) {
        return;
    }
    ecEncodeByte(cp, EJS_SECT_DOC);
    ecEncodeConst(cp, doc->docString);
}


/****************************** Constant Management ***************************/
/*
    Add a constant to the constant pool. Grow if required and return the
    constant string offset into the pool.
 */
PUBLIC int ecAddStringConstant(EcCompiler *cp, EjsString *sp)
{
    Ejs     *ejs;
    int     offset;

    ejs = cp->ejs;
    if (sp == 0) {
        sp = ESV(empty);
    }
    offset = ecAddModuleConstant(cp, cp->state->currentModule, ejsToMulti(cp->ejs, sp));
    if (offset < 0) {
        cp->fatalError = 1;
        assure(offset > 0);
        return EJS_ERR;
    }
    return offset;
}


PUBLIC int ecAddCStringConstant(EcCompiler *cp, cchar *str)
{
    int    offset;

    if (str == 0) {
        str = "";
    }
    offset = ecAddModuleConstant(cp, cp->state->currentModule, str);
    if (offset < 0) {
        cp->fatalError = 1;
        assure(offset > 0);
        return EJS_ERR;
    }
    return offset;
}


PUBLIC int ecAddNameConstant(EcCompiler *cp, EjsName qname)
{
    if (ecAddStringConstant(cp, qname.name) < 0 || ecAddStringConstant(cp, qname.space) < 0) {
        return EJS_ERR;
    }
    return 0;
}


PUBLIC void ecAddFunctionConstants(EcCompiler *cp, EjsPot *obj, int slotNum)
{
    EjsFunction     *fun;

    fun = ejsGetProperty(cp->ejs, obj, slotNum);
    if (fun->resultType) {
        ecAddNameConstant(cp, fun->resultType->qname);
    }
    if (cp->ejs->flags & EJS_FLAG_DOC) {
        ecAddDocConstant(cp, "fun", obj, slotNum);
    }
    ecAddConstants(cp, fun);
    if (fun->activation) {
        ecAddConstants(cp, fun->activation);
    }
}


PUBLIC void ecAddConstants(EcCompiler *cp, EjsAny *block)
{
    Ejs         *ejs;
    EjsName     qname;
    EjsTrait    *trait;
    EjsObj      *vp;
    int         i, numTraits;

    ejs = cp->ejs;
    
    if (VISITED(block)) {
        return;
    }
    SET_VISITED(block, 1);

    numTraits = ejsGetLength(ejs, block);
    for (i = 0; i < numTraits; i++) {
        qname = ejsGetPropertyName(ejs, block, i);
        ecAddNameConstant(cp, qname);
        trait = ejsGetPropertyTraits(ejs, block, i);
        if (trait && trait->type) {
            ecAddNameConstant(cp, trait->type->qname);
        }
        vp = ejsGetProperty(ejs, block, i);
        if (vp != block) {
            if (ejsIsFunction(ejs, vp)) {
                ecAddFunctionConstants(cp, block, i);
            } else if (ejsIsBlock(ejs, vp)) {
                ecAddConstants(cp, vp);
            }
        }
    }
    SET_VISITED(block, 0);
}


PUBLIC int ecAddDocConstant(EcCompiler *cp, cchar *tag, void *vp, int slotNum)
{
    Ejs         *ejs;
    EjsDoc      *doc;
    char        key[32];

    ejs = cp->ejs;

    assure(ejs->doc);
    assure(vp);
    assure(slotNum >= 0);

    fmt(key, sizeof(key), "%s %Lx %d", tag, PTOL(vp), slotNum);
    doc = (EjsDoc*) mprLookupKey(ejs->doc, key);
    if (doc && doc->docString) {
        if (ecAddStringConstant(cp, doc->docString) < 0) {
            assure(0);
            return EJS_ERR;
        }
    }
    return 0;
}


/*
    Add a constant and encode the offset.
 */
PUBLIC int ecAddModuleConstant(EcCompiler *cp, EjsModule *mp, cchar *str)
{
    EjsConstants    *constants;
    MprKey          *kp;
    int             index;

    assure(mp);

    if (str == 0) {
        assure(0);
        return 0;
    }
    constants = mp->constants;
    if (constants->table && (kp = mprLookupKeyEntry(constants->table, str)) != 0) {
        return PTOI(kp->data);
    }
    index = ejsAddConstant(cp->ejs, mp, str);
    // mprLog(0, "%6d %s", index, str);
    mprAddKey(constants->table, str, ITOP(index));
    return index;
}


/****************************** Value Emitters ********************************/
/*
    Emit an encoded string ored with flags. The name index is shifted by 2.
 */
static void encodeTypeName(EcCompiler *cp, EjsString *name, int flags)
{
    int        offset;

    assure(name);

    offset = ecAddModuleConstant(cp, cp->state->currentModule, ejsToMulti(cp->ejs, name));
    if (offset < 0) {
        cp->fatalError = 1;
        assure(offset > 0);
    } else {
        ecEncodeNum(cp, offset << 2 | flags);
    }
}


/*
    Encode a global variable (usually a type). The encoding is untyped: 0, bound type: slot number, unbound or 
    unresolved type: name.
 */
PUBLIC void ecEncodeGlobal(EcCompiler *cp, EjsAny *obj, EjsName qname)
{
    Ejs         *ejs;
    int         slotNum;

    ejs = cp->ejs;
    slotNum = -1;

    if (obj == 0) {
        assure(0);
        ecEncodeNum(cp, EJS_ENCODE_GLOBAL_NOREF);
        return;
    }
    /*
        If binding globals, we can encode the slot number of the type.
     */
    slotNum = ejsLookupProperty(ejs, ejs->global, qname);

    //  MOB - don't bind for Appweb all-in-one. ejs.web can load at different places 
    if (slotNum < ES_global_NUM_CLASS_PROP /* || cp->bind */) {
        if (slotNum >= 0) {
            ecEncodeNum(cp, (slotNum << 2) | EJS_ENCODE_GLOBAL_SLOT);
            return;
        }
    }
    encodeTypeName(cp, qname.name, EJS_ENCODE_GLOBAL_NAME);
    ecEncodeConst(cp, qname.space);
}


/**************************** Value Encoding Routines *************************/
/*
    Reserve a small amount of room sufficient for the next encoding
 */
static int reserveRoom(EcCompiler *cp, int room)
{
    EcCodeGen       *code;

    code = cp->state->code;
    assure(code);

    if (mprGetBufSpace(code->buf) < room) {
        if (mprGrowBuf(code->buf, -1) < 0) {
            cp->fatalError = 1;
            assure(0);
            return MPR_ERR_MEMORY;
        }
    }
    return 0;
}


/*
    Encode an Ejscript instruction operation code
 */
PUBLIC void ecEncodeOpcode(EcCompiler *cp, int code)
{
    assure(code < 240);
    assure(cp);

    cp->lastOpcode = code;
    ecEncodeByte(cp, code);
}


/*
    Encode a <name><namespace> pair
 */
PUBLIC void ecEncodeName(EcCompiler *cp, EjsName qname)
{
    assure(qname.name);

    ecEncodeConst(cp, qname.name);
    ecEncodeConst(cp, qname.space);
}


PUBLIC void ecEncodeConst(EcCompiler *cp, EjsString *sp)
{
    Ejs     *ejs;
    cchar   *str;
    int     offset;

    assure(cp);
    ejs = cp->ejs;

    if (sp == 0) {
        sp = ESV(empty);
    }
    str = ejsToMulti(cp->ejs, sp);
    offset = ecAddModuleConstant(cp, cp->state->currentModule, str);
    if (offset < 0) {
        cp->error = 1;
        cp->fatalError = 1;
        return;
    }
    assure(offset >= 0);
    ecEncodeNum(cp, offset);
}


PUBLIC void ecEncodeByte(EcCompiler *cp, int value)
{
    EcCodeGen   *code;
    uchar       *pc;

    assure(cp);
    code = cp->state->code;

    if (reserveRoom(cp, sizeof(uchar)) < 0) {
        return;
    }
    pc = (uchar*) mprGetBufEnd(code->buf);
    *pc++ = value;
    mprAdjustBufEnd(code->buf, sizeof(uchar));
}


PUBLIC void ecEncodeMulti(EcCompiler *cp, cchar *str)
{
    int    len;

    assure(cp);

    len = (int) strlen(str) + 1;
    assure(len > 0);
    ecEncodeNum(cp, len);
    ecEncodeBlock(cp, (uchar*) str, len);
}


PUBLIC void ecEncodeWideAsMulti(EcCompiler *cp, wchar *str)
{
    char    *mstr;
    ssize   len;

    assure(cp);

    mstr = awtom(str, &len);
    //  UNICODE - why calculate len again?
    len = strlen(mstr) + 1;
    assure(len > 0);
    ecEncodeNum(cp, (int) len);
    ecEncodeBlock(cp, (uchar*) str, (int) len);
}


PUBLIC void ecEncodeNum(EcCompiler *cp, int64 number)
{
    MprBuf      *buf;
    int         len;

    assure(cp);
    buf = cp->state->code->buf;
    if (reserveRoom(cp, sizeof(int64) + 2) < 0) {
        return;
    }
    len = ejsEncodeNum(cp->ejs, (uchar*) mprGetBufEnd(buf), number);
    mprAdjustBufEnd(buf, len);
}


PUBLIC void ecEncodeDouble(EcCompiler *cp, double value)
{
    MprBuf      *buf;
    int         len;

    assure(cp);
    buf = cp->state->code->buf;
    if (reserveRoom(cp, sizeof(double) + 4) < 0) {
        return;
    }
    len = ejsEncodeDouble(cp->ejs, (uchar*) mprGetBufEnd(buf), value);
    mprAdjustBufEnd(buf, len);
}


/*
    Encode a 32-bit number. Always emit exactly 4 bytes.
 */
PUBLIC void ecEncodeInt32(EcCompiler *cp, int number)
{
    MprBuf      *buf;
    int         len;

    assure(cp);
    buf = cp->state->code->buf;

    if (reserveRoom(cp, sizeof(int) / sizeof(char)) < 0) {
        return;
    }
    len = ejsEncodeInt32(cp->ejs, (uchar*) mprGetBufEnd(buf), number);
    assure(len == 4);
    mprAdjustBufEnd(buf, len);
}


PUBLIC void ecEncodeByteAtPos(EcCompiler *cp, int offset, int value)
{
    ejsEncodeByteAtPos(cp->ejs, (uchar*) mprGetBufStart(cp->state->code->buf) + offset, value);
}


PUBLIC void ecEncodeInt32AtPos(EcCompiler *cp, int offset, int value)
{
    if (abs(value) > EJS_ENCODE_MAX_WORD) {
        mprError("Code generation error. Word %d exceeds maximum %d", value, EJS_ENCODE_MAX_WORD);
        cp->fatalError = 1;
        return;
    }
    ejsEncodeInt32AtPos(cp->ejs, (uchar*) mprGetBufStart(cp->state->code->buf) + offset, value);
}


PUBLIC void ecEncodeBlock(EcCompiler *cp, cuchar *buf, int len)
{
    EcCodeGen   *code;

    code = cp->state->code;

    if (reserveRoom(cp, len) < 0) {
        return;
    }
    if (mprPutBlockToBuf(code->buf, (char*) buf, len) != len) {
        cp->fatalError = 1;
    }
}


uint ecGetCodeOffset(EcCompiler *cp)
{
    EcCodeGen   *code;

    code = cp->state->code;
    return (uint) ((uchar*) mprGetBufEnd(code->buf) - (uchar*) mprGetBufStart(code->buf));
}


PUBLIC int ecGetCodeLen(EcCompiler *cp, uchar *mark)
{
    EcCodeGen   *code;

    code = cp->state->code;
    return (int) (((uchar*) mprGetBufEnd(code->buf)) - mark);
}


/*
    Copy the code at "pos" of length "size" the distance specified by "dist". Dist may be postitive or negative.
 */
PUBLIC void ecCopyCode(EcCompiler *cp, uchar *pos, int size, int dist)
{
    mprMemcpy((char*) &pos[dist], size, (char*) pos, size);
}


PUBLIC void ecAdjustCodeLength(EcCompiler *cp, int adj)
{
    EcCodeGen   *code;

    code = cp->state->code;
    mprAdjustBufEnd(code->buf, adj);
}


#if UNUSED && KEEP
static int swapWordField(EcCompiler *cp, int word)
{
    if (mprGetEndian(cp) == MPR_LITTLE_ENDIAN) {
        return word;
    }
    return ((word & 0xFF000000) >> 24) | ((word & 0xFF0000) >> 8) | ((word & 0xFF00) << 8) | ((word & 0xFF) << 24);
}


static int swapShortField(EcCompiler *cp, int word)
{
    if (mprGetEndian(cp) == MPR_LITTLE_ENDIAN) {
        return word;
    }
    return ((word & 0xFF) << 8) | ((word & 0xFF00) >> 8);
}

static int swapCodePoint(EcCompiler *cp, int word)
{
#if BIT_CHAR_LEN == 4
    return swapWordField(cp, word);
#elif BIT_CHAR_LEN == 2
    return swapShortField(cp, word);
#else
    return word;
#endif 
}
#endif /* UNUSED */


/*
    Simple checksum of name and slots. Not meant to be rigorous.
 */
static int sumNum(int value)
{
    return value;
}


static int sumString(EjsString *name)
{
    wchar   *cp;
    int     checksum;

    checksum = 0;
    if (name) {
        for (cp = name->value; *cp; cp++) {
            checksum += *cp;
        }
    }
    return checksum;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
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
