/**
    ecAst.c - Process AST nodes and define all variables.

    Note on error handling. If a non-recoverable error occurs, then EcCompiler.hasFatalError will be set and
    processing will be aborted. If a recoverable error occurs, then hasError will be set and processing will continue.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"
#include    "ecCompiler.h"

/********************************** Defines ***********************************/
/*
    State level macros. Enter/Leave manage state and inheritance of state.
 */
#undef ENTER
#define ENTER(cp) if (ecEnterState(cp) < 0) { return; } else

#undef LEAVE
#define LEAVE(cp) ecLeaveState(cp)

/***************************** Forward Declarations ***************************/

static void     addGlobalProperty(EcCompiler *cp, EcNode *np, EjsName *qname);
static void     addScope(EcCompiler *cp, EjsBlock *block);
static void     astBinaryOp(EcCompiler *cp, EcNode *np);
static void     astBindName(EcCompiler *cp, EcNode *np);
static void     astBlock(EcCompiler *cp, EcNode *np);
static void     astBreak(EcCompiler *cp, EcNode *np);
static void     astCall(EcCompiler *cp, EcNode *np);
static void     astCaseElements(EcCompiler *cp, EcNode *np);
static void     astCaseLabel(EcCompiler *cp, EcNode *np);
static void     astCatch(EcCompiler *cp, EcNode *np);
static void     astClass(EcCompiler *cp, EcNode *np);
static void     astDirectives(EcCompiler *cp, EcNode *np);
static void     astDot(EcCompiler *cp, EcNode *np);
static void     astDo(EcCompiler *cp, EcNode *np);
static void     astError(EcCompiler *cp, EcNode *np, cchar *fmt, ...);
static void     astExpressions(EcCompiler *cp, EcNode *np);
static void     astField(EcCompiler *cp, EcNode *np);
static void     astFor(EcCompiler *cp, EcNode *np);
static void     astForIn(EcCompiler *cp, EcNode *np);
static void     astFunction(EcCompiler *cp, EcNode *np);
static void     astHash(EcCompiler *cp, EcNode *np);
static void     astIf(EcCompiler *cp, EcNode *np);
static void     astName(EcCompiler *cp, EcNode *np);
static void     astNew(EcCompiler *cp, EcNode *np);
static void     astObjectLiteral(EcCompiler *cp, EcNode *np);
static void     astPostfixOp(EcCompiler *cp, EcNode *np);
static void     astPragmas(EcCompiler *cp, EcNode *np);
static void     astPragma(EcCompiler *cp, EcNode *np);
static void     astProgram(EcCompiler *cp, EcNode *np);
static void     astReturn(EcCompiler *cp, EcNode *np);
static void     astSuper(EcCompiler *cp, EcNode *np);
static void     astSwitch(EcCompiler *cp, EcNode *np);
static void     astThis(EcCompiler *cp, EcNode *np);
static void     astThrow(EcCompiler *cp, EcNode *np);
static void     astTry(EcCompiler *cp, EcNode *np);
static void     astUnaryOp(EcCompiler *cp, EcNode *np);
static void     astModule(EcCompiler *cp, EcNode *np);
static void     astUseNamespace(EcCompiler *cp, EcNode *np);
static void     astVar(EcCompiler *cp, EcNode *np, int varKind, EjsObj *value);
static void     astVarDefinition(EcCompiler *cp, EcNode *np, int *codeRequired, int *instanceCode);
static void     astVoid(EcCompiler *cp, EcNode *np);
static void     astWarn(EcCompiler *cp, EcNode *np, cchar *fmt, ...);
static void     astWith(EcCompiler *cp, EcNode *np);
static void     badAst(EcCompiler *cp, EcNode *np);
static void     bindVariableDefinition(EcCompiler *cp, EcNode *np);
static void     closeBlock(EcCompiler *cp);
static EjsNamespace *createHoistNamespace(EcCompiler *cp, EjsObj *obj);
static EjsModule    *createModule(EcCompiler *cp, EcNode *np);
static EjsFunction *createModuleInitializer(EcCompiler *cp, EcNode *np, EjsModule *mp);
static int      defineParameters(EcCompiler *cp, EcNode *np);
static void     defineVar(EcCompiler *cp, EcNode *np, int varKind, EjsObj *value);
static void     fixupClass(EcCompiler *cp, EjsType *type);
static EjsBlock *getBlockForDefinition(EcCompiler *cp, EcNode *np, EjsBlock *block, int attributes);
static EcNode   *getNextAstNode(EcCompiler *cp, EcNode *np, int *next);
static EjsObj   *getTypeProperty(EcCompiler *cp, EjsObj *vp, EjsName name);
static bool     hoistBlockVar(EcCompiler *cp, EcNode *np);
static void     openBlock(EcCompiler *cp, EcNode *np, EjsBlock *block);
static void     processAstNode(EcCompiler *cp, EcNode *np);
static void     removeProperty(EcCompiler *cp, EjsObj *block, EcNode *np);
static EjsNamespace *resolveNamespace(EcCompiler *cp, EcNode *np, EjsBlock *block, bool *modified);
static void     removeScope(EcCompiler *cp);
static int      resolveName(EcCompiler *cp, EcNode *node, EjsObj *vp,  EjsName name);
static int      resolveProperty(EcCompiler *cp, EcNode *node, EjsType *type, EjsName name);
static void     setAstDocString(Ejs *ejs, EcNode *np, void *vp, int slotNum);
static EjsNamespace *lookupNamespace(Ejs *ejs, EjsString *namespace);

/*********************************************** Code ***********************************************/
/*
    Top level AST node processing.
 */
static int astProcess(EcCompiler *cp, EcNode *np)
{
    Ejs     *ejs;
    int     phase;

    if (ecEnterState(cp) < 0) {
        return EJS_ERR;
    }

    ejs = cp->ejs;
    cp->blockState = cp->state;

    /*
        We do 5 phases over all the nodes: define, conditional, fixup, bind and erase
     */
    for (phase = 0; phase < EC_AST_PHASES && cp->errorCount == 0; phase++) {
        /*
            Looping through the input source files. A single top level node describes the source file.
         */
        cp->phase = phase;
        cp->fileState = cp->state;
        cp->fileState->strict = cp->strict;
        processAstNode(cp, np);
    }
    ecLeaveState(cp);
    cp->fileState = 0;
    cp->blockState = 0;
    cp->error = 0;
    return (cp->errorCount > 0) ? EJS_ERR : 0;
}


int ecAstProcess(EcCompiler *cp)
{
    Ejs         *ejs;
    EcNode      *np;
    int         phase, i, count;

    if (ecEnterState(cp) < 0) {
        return EJS_ERR;
    }

    ejs = cp->ejs;
    cp->blockState = cp->state;

    /*
        We do 5 phases over all the nodes: define, load, fixup, block vars and bind
     */
    for (phase = 0; phase < EC_AST_PHASES && cp->errorCount == 0; phase++) {
        cp->phase = phase;

        /*
            Loop over each source file
         */
        count = mprGetListLength(cp->nodes);
        for (i = 0; i < count && !cp->fatalError; i++) {
            /*
                Looping through the input source files. A single top level node describes the source file.
             */
            np = mprGetItem(cp->nodes, i);
            if (np == 0) {
                continue;
            }
            cp->fileState = cp->state;
            cp->fileState->strict = cp->strict;
            processAstNode(cp, np);
        }
    }
    ecLeaveState(cp);
    cp->fileState = 0;
    cp->blockState = 0;
    cp->error = 0;
    return (cp->errorCount > 0) ? EJS_ERR : 0;
}


static void astArgs(EcCompiler *cp, EcNode *np)
{
    EcNode      *child;
    int         next;

    ENTER(cp);

    mprAssert(np->kind == N_ARGS);

    next = 0;
    while ((child = getNextAstNode(cp, np, &next))) {
        processAstNode(cp, child);
    }
    LEAVE(cp);
}


static void astSpread(EcCompiler *cp, EcNode *np)
{
    EcNode      *child;
    int         next;

    ENTER(cp);

    mprAssert(np->kind == N_SPREAD);
    next = 0;
    while ((child = getNextAstNode(cp, np, &next))) {
        processAstNode(cp, child);
    }
    LEAVE(cp);
}


/*
    Generate an assignment expression
 */
static void astAssignOp(EcCompiler *cp, EcNode *np)
{
    EcState     *state;
    int         rc, next;

    ENTER(cp);

    state = cp->state;
    rc = 0;
    next = 0;

    mprAssert(np->kind == N_ASSIGN_OP);
    mprAssert(np->left);
    mprAssert(np->right);

    if (state->inSettings && cp->phase >= EC_PHASE_BIND) {
        /*
            Assignment in a class initializer. The lhs must be scoped outside the block. The rhs must be scoped inside.
         */
        openBlock(cp, state->currentFunctionNode->function.body, (EjsBlock*) state->currentFunction->activation);
        processAstNode(cp, np->right);
        closeBlock(cp);
    } else {
        processAstNode(cp, np->right);
    }
    state->onLeft = 1;
    processAstNode(cp, np->left);
    LEAVE(cp);
}


/*
    Handle a binary operator. We recursively process left and right nodes.
 */
static void astBinaryOp(EcCompiler *cp, EcNode *np)
{
    ENTER(cp);

    mprAssert(np->kind == N_BINARY_OP);

    if (np->left) {
        processAstNode(cp, np->left);
    }
    if (np->right) {
        processAstNode(cp, np->right);
    }
    LEAVE(cp);
}


static void defineBlock(EcCompiler *cp, EcNode *np)
{
    Ejs         *ejs;
    EcState     *state;
    EjsBlock    *block, *letBlock;
    int         slotNum;

    ejs = cp->ejs;
    state = cp->state;
    letBlock = state->letBlock;

    mprAssert(cp->phase == EC_PHASE_CONDITIONAL);
    mprAssert(np->kind == N_BLOCK || np->kind == N_MODULE);

    block = np->blockRef;

    if (np->createBlockObject) {
        slotNum = ejsDefineProperty(ejs, letBlock, -1, np->qname, TYPE(block), 0, (EjsObj*) block);
        if (slotNum < 0) {
            astError(cp, np, "Can't define block");
        } else {
            np->blockCreated = 1;
            if (letBlock == ejs->global) {
                addGlobalProperty(cp, np, &np->qname);
            }
        }
    }
}


static void bindBlock(EcCompiler *cp, EcNode *np)
{
    Ejs         *ejs;
    EjsBlock    *block;
    int         rc;
    
    mprAssert(cp->phase == EC_PHASE_BIND);
    mprAssert(np->kind == N_BLOCK || np->kind == N_MODULE);

    ejs = cp->ejs;
    block = np->blockRef;
    mprAssert(block);

    rc = resolveName(cp, np, NULL, np->qname);
    if (np->blockCreated) {
        if (! np->createBlockObject) {
            mprAssert(cp->lookup.obj);
            mprAssert(np->lookup.slotNum >= 0);
            ejsDeleteProperty(ejs, np->lookup.obj, np->lookup.slotNum);
            np->blockCreated = 0;
        } else {
            /*
                Mark the parent block as needing to be created to hold this block.
             */
            if (cp->state->prev->letBlockNode) {
                cp->state->prev->letBlockNode->createBlockObject = 1;
            }
        }
    }
}


static void astBlock(EcCompiler *cp, EcNode *np)
{
    EcNode      *child;
    int         next, needBlock;

    ENTER(cp);
    
    needBlock = 0;
    if (cp->phase < EC_PHASE_BIND) {
        needBlock = 1;
    } else if (cp->phase == EC_PHASE_BIND) {
        /*
            Bind the block here before processing the child nodes so we can mark the block as hidden if it will be expunged.
         */
        bindBlock(cp, np);
        needBlock = np->blockCreated;
    }

    /*
        Open block will change state->letBlock which we need preserved in defineBlock. Use ENTER/LEAVE to save and restore.
     */
    ENTER(cp);
    if (needBlock) {
        openBlock(cp, np, NULL);
    }
    next = 0;
    while ((child = getNextAstNode(cp, np, &next))) {
        processAstNode(cp, child);
    }
    if (needBlock) {
        closeBlock(cp);
    }
    LEAVE(cp);

    if (cp->phase == EC_PHASE_CONDITIONAL) {
        /*
            Do define block after the variables have been processed. This allows us to determine if the block is 
            really needed.
         */
        defineBlock(cp, np);

        /* Try to hoist the block object itself */
        if (np->blockCreated && !hoistBlockVar(cp, np)) {
            cp->state->letBlockNode->createBlockObject = 1;
        }
    }
    LEAVE(cp);
}


static void astBreak(EcCompiler *cp, EcNode *np)
{
    mprAssert(np->kind == N_BREAK);
}


static void astCall(EcCompiler *cp, EcNode *np)
{
    EcState         *state;

    mprAssert(np->kind == N_CALL);

    ENTER(cp);
    
    state = cp->state;

    if (state->onLeft) {
        astError(cp, np, "Invalid call expression on the left hand side of assignment");
        LEAVE(cp);
        return;
    }
    if (np->right) {
        mprAssert(np->right->kind == N_ARGS);
        astArgs(cp, np->right);
    }
    processAstNode(cp, np->left);

    /*
        Propagate up the right side qname and lookup.
     */
    if (cp->phase >= EC_PHASE_BIND) {
        if (np->left) {
            np->lookup = np->left->lookup;
            np->qname = np->left->qname;
        }
    }
    LEAVE(cp);
}


static void astCaseElements(EcCompiler *cp, EcNode *np)
{
    EcNode      *child;
    int         next;

    ENTER(cp);

    mprAssert(np->kind == N_CASE_ELEMENTS);

    next = 0;
    while ((child = getNextAstNode(cp, np, &next))) {
        processAstNode(cp, child);
    }
    LEAVE(cp);
}


static void astCaseLabel(EcCompiler *cp, EcNode *np)
{
    EcNode      *child;
    int         next;

    ENTER(cp);

    mprAssert(np->kind == N_CASE_LABEL);

    if (np->caseLabel.kind == EC_SWITCH_KIND_CASE) {
        mprAssert(np->caseLabel.expression);
        processAstNode(cp, np->caseLabel.expression);

    } else {
        mprAssert(np->caseLabel.kind == EC_SWITCH_KIND_DEFAULT);
    }

    /*
        Process the directives for this case label
     */
    next = 0;
    while ((child = getNextAstNode(cp, np, &next))) {
        processAstNode(cp, child);
    }
    LEAVE(cp);
}


static void astCatch(EcCompiler *cp, EcNode *np)
{
    EjsBlock    *block;
    Ejs         *ejs;

    ENTER(cp);

    ejs = cp->ejs;
    block = ejsCreateBlock(cp->ejs, 0);
    ejsSetName(block, MPR_NAME("catch"));
    addScope(cp, block);
    processAstNode(cp, np->left);
    removeScope(cp);
    LEAVE(cp);
}


static EjsType *defineClass(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EjsType         *type;
    EcState         *state;
    EcNode          *constructorNode;
    EjsName         qname;
    EjsNamespace    *nsp;
    char            *name;
    int             fatt, attributes, slotNum;
    
    mprAssert(np->kind == N_CLASS);

    ejs = cp->ejs;
    state = cp->state;
    type = np->klass.ref;
    
    if ((slotNum = ecLookupVar(cp, ejs->global, np->qname)) >= 0) {
        if (cp->fileState->strict) {
            astError(cp, np, "%s Class %s is already defined.", np->qname.space, np->qname.name);
            return 0;
        }
    } else {
        slotNum = ejsGetPropertyCount(ejs, ejs->global);
    }
    attributes = np->attributes | EJS_TYPE_FIXUP;
    if (np->klass.isInterface) {
        attributes |= EJS_TYPE_INTERFACE;
    }
    
    type = ejsCreateType(ejs, np->qname, state->currentModule, NULL, NULL, sizeof(EjsPot), slotNum, 0, 0, attributes);
    if (type == 0) {
        astError(cp, np, "Can't create type %s", type->qname.name);
        return 0;
    }
    type->typeData = np;
    np->klass.ref = type;
    ejsClonePotHelpers(ejs, type);

    nsp = ejsDefineReservedNamespace(ejs, (EjsBlock*) type, &type->qname, EJS_PROTECTED_NAMESPACE);
#if UNUSED
    //  MOB -- these flags should be args to above()
    //  MOB -- need some flag like this to speed up scope searching
    nsp->flags |= EJS_NSP_PROTECTED;
#endif

    nsp = ejsDefineReservedNamespace(ejs, (EjsBlock*) type, &type->qname, EJS_PRIVATE_NAMESPACE);
#if UNUSED
    //  MOB -- need some flag like this to speed up scope searching
    nsp->flags |= EJS_NSP_PRIVATE;
#endif

    /*
        Define a property for the type in global
     */
    slotNum = ejsDefineProperty(ejs, ejs->global, slotNum, np->qname, ejs->typeType, attributes, type);
    if (slotNum < 0) {
        astError(cp, np, "Can't install type %s",  np->qname.name);
        return 0;
    }
    if (!type->isInterface) {
        /*
            Reserve one slot for the static initializer to ensure it is the first non-inherited slot.
            This slot may be reclaimed during fixup if not required. Instance initializers are prepended to the constructor.
         */
        //  MOB -- rethink name
        name = mprAsprintf("-%@-", type->qname.name);
        qname.name = ejsCreateStringFromAsc(ejs, name);
        mprFree(name);
        qname.space = ejsCreateStringFromAsc(ejs, EJS_INIT_NAMESPACE);
        fatt = EJS_TRAIT_HIDDEN | EJS_PROP_STATIC;
        ejsDefineProperty(ejs, (EjsObj*) type, 0, qname, ejs->functionType, fatt, ejs->nullValue);
        constructorNode = np->klass.constructor;
        if (constructorNode && !constructorNode->function.isDefaultConstructor) {
            type->hasConstructor = 1;
        }
#if MOB
        ejsDefineProperty(ejs, (EjsObj*) type, 1, EN("prototype"), ejs->objectType, fatt, type->prototype);
#endif
    }
    return type;
}


static void validateFunction(EcCompiler *cp, EcNode *np, EjsFunction *spec, EjsFunction *fun)
{
}

static void validateClass(EcCompiler *cp, EcNode *np)
{
    Ejs         *ejs;
    EjsType     *type, *iface, *baseType;
    EjsName     qname;
    EjsFunction *fun;
    EjsObj      *vp;
    EcState     *state;
    int         next, i, count;

    ejs = cp->ejs;
    state = cp->state;
    type = np->klass.ref;

    baseType = type->baseType;
    if (baseType && baseType->final) {
        astError(cp, np, "Class \"%s\" is attempting to subclass a final class \"%s\"", type->qname.name, 
            baseType->qname.name);
    }

    /*
        Ensure the class implements all required implemented methods
     */
    for (next = 0; ((iface = (EjsType*) mprGetNextItem(type->implements, &next)) != 0); ) {
        count = ejsGetPropertyCount(ejs, (EjsObj*) iface);
        for (i = 0; i < count; i++) {
            fun = ejsGetProperty(ejs, (EjsObj*) iface, i);
            if (!ejsIsFunction(ejs, fun) || fun->isInitializer) {
                continue;
            }
            qname = ejsGetPropertyName(ejs, (EjsObj*) iface, i);
            vp = ejsGetPropertyByName(ejs, (EjsObj*) type, qname);
            if (vp == 0 || !ejsIsFunction(ejs, vp)) {
                astError(cp, np, "Missing method \"%s\" required by interface \"%s\"", qname.name, iface->qname.name);
            } else {
                validateFunction(cp, np, fun, (EjsFunction*) vp);
            }
        }
    }
    if (type->implements) {
        if (mprGetListLength(type->implements) > 1 || 
                (type->baseType && ejsCompareMulti(ejs, type->baseType->qname.name, "Object") != 0)) {
            //  MOB -- fix. Should support multiple implements
            astError(cp, np, "Only one implements or one extends supported");
        }
    }        
}


/*
    Lookup the set of open namespaces for the required namespace for this class
 */
static void bindClass(EcCompiler *cp, EcNode *np)
{
    Ejs         *ejs;
    EjsType     *type;
    EjsFunction *fun;
    EjsModule   *mp;
    EcState     *state;
    bool        modified;

    ejs = cp->ejs;
    state = cp->state;
    type = np->klass.ref;

    mprAssert(cp->phase == EC_PHASE_BIND);

    if (type->hasInitializer) {
        /*
            Create the static initializer function. Code gen will fill out the code
         */
        mp = state->currentModule;
        fun = ejsCreateFunction(ejs, np->qname.name, NULL, -1, 0, 0, 0, cp->ejs->voidType, EJS_FUN_INITIALIZER, mp, NULL, 
            cp->fileState->strict);
        np->klass.initializer = fun;
        //  MOB -- better to use DefineProperty and set traits for initializer
        ejsSetProperty(ejs, (EjsObj*) type, 0, (EjsObj*) fun);
    }

    modified = 0;
    if (!np->literalNamespace && resolveNamespace(cp, np, (EjsBlock*) ejs->global, &modified) == 0) {
        return;
    }
    if (modified) {
        ejsSetTypeName(ejs, type, np->qname);
    }
    addGlobalProperty(cp, np, &type->qname);

    if (type->hasBaseConstructors) {
        //  MOB --remove - should already be set.
        mprAssert(type->hasConstructor);
        type->hasConstructor = 1;
    }
    if (resolveName(cp, np, ejs->global, type->qname) < 0) {
        return;
    }
    setAstDocString(ejs, np, ejs->global, np->lookup.slotNum);
}


/*
    Process a class node
 */
static void astClass(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EjsType         *type;
    EcState         *state;
    EcNode          *constructor;
    bool            hasInitializer;

    mprAssert(np->kind == N_CLASS);
    
    ENTER(cp);

    ejs = cp->ejs;
    state = cp->state;
    state->classState = state;
    type = np->klass.ref;
    
    if (np->klass.implements) {
        processAstNode(cp, np->klass.implements);
    }        
    if (state->disabled) {
        if (cp->phase == EC_PHASE_CONDITIONAL) {
            removeProperty(cp, ejs->global, np);
        }
        LEAVE(cp);
        return;
    }
    if (cp->phase == EC_PHASE_DEFINE) {
        type = defineClass(cp, np);

    } else if (cp->phase == EC_PHASE_FIXUP) {
         fixupClass(cp, type);

    } else if (cp->phase >= EC_PHASE_BIND) {
        validateClass(cp, np);
        bindClass(cp, np);
    }
    if (cp->error) {
        LEAVE(cp);
        return;
    }
    state->currentClass = type;
    state->currentClassNode = np;
#if UNUSED
    state->currentClassName = type->qname;
#endif
    state->inClass = 1;
    
    //  MOB -- need a way to zero things that should not be inherited
    state->inFunction = 0;
    state->inMethod = 0;
    state->blockIsMethod = 0;
    state->currentFunction = 0;
    state->currentFunctionNode = 0;

    /*
        Add the type to the scope chain and the static initializer if present. Use push frame to make it eaiser to
        pop the type off the scope chain later.
     */
    hasInitializer = 0;
    addScope(cp, (EjsBlock*) type);
    if (np->klass.initializer) {
        openBlock(cp, np, (EjsBlock*) np->klass.initializer);
        hasInitializer++;
    }
    if (cp->phase == EC_PHASE_FIXUP && type->baseType) {
        ejsInheritBaseClassNamespaces(ejs, type, type->baseType);
    }
    state->optimizedLetBlock = (EjsBlock*) type;
    state->letBlock = (EjsBlock*) type;
    state->varBlock = (EjsBlock*) type;

    /*
        Process the class body
     */
    mprAssert(np->left->kind == N_DIRECTIVES);
    processAstNode(cp, np->left);
    
    if (hasInitializer) {
        closeBlock(cp);
    }

    /*
        Only need to do this if this is a default constructor, ie. does not exist in the class body.
     */
    constructor = np->klass.constructor;
    if (constructor && constructor->function.isDefaultConstructor) {
        astFunction(cp, constructor);
    }
    removeScope(cp);
    LEAVE(cp);
}


static void astDassign(EcCompiler *cp, EcNode *np)
{
    EcNode      *child;
    int         next;

    mprAssert(np->kind == N_DASSIGN);

    if (np->objectLiteral.typeNode) {
        processAstNode(cp, np->objectLiteral.typeNode);
    }
    for (next = 0; (child = getNextAstNode(cp, np, &next)) != 0; ) {
        processAstNode(cp, child);
    }
}


static void astDirectives(EcCompiler *cp, EcNode *np)
{
    EcNode      *child;
    Ejs         *ejs;
    int         next;

    mprAssert(np->kind == N_DIRECTIVES);

    ENTER(cp);

    ejs = cp->ejs;
    cp->state->blockNestCount++;
    next = 0;
    while ((child = getNextAstNode(cp, np, &next))) {
        processAstNode(cp, child);
    }
    cp->state->blockNestCount--;
    LEAVE(cp);
}


/*
    Handle a do statement
 */
static void astDo(EcCompiler *cp, EcNode *np)
{
    ENTER(cp);

    mprAssert(np->kind == N_DO);

    if (np->forLoop.cond) {
        processAstNode(cp, np->forLoop.cond);
    }
    if (np->forLoop.body) {
        processAstNode(cp, np->forLoop.body);
    }
    LEAVE(cp);
}


/*
    Handle property dereferencing via "." and "[". This routine will bind a
    name path reference into slot bindings if possible. The dot node is a
    binary node.

            local.a.b.c
            arg.a.b.c
            obj.a.b.c
            static.a.b.c
            any[expression]
            unqualifiedName         - dynamic bound
            expression              - dynamic bound
 */
static void astDot(EcCompiler *cp, EcNode *np)
{
    EcState     *state;
    EcNode      *left;

    mprAssert(np->kind == N_DOT);
    mprAssert(np->left);
    mprAssert(np->right);

    ENTER(cp);

    state = cp->state;
    state->onLeft = 0;
    left = np->left;

    /*
        Optimize to assist with binding. Remove an expressions node which has a sole QNAME.
     */
    if (left && left->kind == N_EXPRESSIONS && left->left && left->left->kind == N_QNAME && left->right == 0) {
        np->left = np->left->left;
    }

    /*
        Process the left of the "."
     */
    processAstNode(cp, np->left);

    state->currentObjectNode = np->left;
    
    /*
        If the right is a terminal node, then assume the parent state's onLeft status
     */
    switch (np->right->kind) {
    case N_QNAME:
/*
    Need to allow obj[fun()] = 7
    case N_EXPRESSIONS: 
 */
    case N_LITERAL:
    case N_OBJECT_LITERAL:
        cp->state->onLeft = cp->state->prev->onLeft;
        break;

    default:
        break;
    }
    processAstNode(cp, np->right);

    /*
        Propagate up the right side qname and lookup.
     */
    if (cp->phase >= EC_PHASE_BIND) {
        np->lookup = np->right->lookup;
        np->qname = np->right->qname;
    }
    LEAVE(cp);
}


/*
    Process an expressions node
 */
static void astExpressions(EcCompiler *cp, EcNode *np)
{
    EcNode      *child;
    int         next;

    mprAssert(np->kind == N_EXPRESSIONS);

    ENTER(cp);

    /*
        No current object when computing an expression. E.g. obj[a + b]
        We don't want obj set as the context object for a or b.
     */
    cp->state->currentObjectNode = 0;

    next = 0;
    while ((child = getNextAstNode(cp, np, &next)) != 0) {
        processAstNode(cp, child);
    }
    /*
        Propagate up the right side qname and lookup.
     */
    if (cp->phase >= EC_PHASE_BIND) {
        child = mprGetLastItem(np->children);
        if (child) {
            np->lookup = child->lookup;
            np->qname = child->qname;
        }
    }
    LEAVE(cp);
}


static EjsFunction *defineFunction(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EcState         *state;
    EcNode          *parameters;
    EjsFunction     *fun;
    EjsBlock        *block;
    int             numArgs;
    int             slotNum;

    mprAssert(np->kind == N_FUNCTION);
    mprAssert(cp->phase == EC_PHASE_DEFINE);

    ejs = cp->ejs;
    state = cp->state;

    if (np->function.isMethod) {
        np->attributes |= EJS_TRAIT_FIXED | EJS_TRAIT_READONLY;
        if (!(np->attributes & EJS_PROP_ENUMERABLE) 
                /* Don't inherit enumerable for functions */
                /* MOB && !(state->currentClassNode->attributes & EJS_PROP_ENUMERABLE) */) {
            np->attributes |= EJS_TRAIT_HIDDEN;
        }
        block = getBlockForDefinition(cp, np, state->varBlock, np->attributes);

    } else {
        block = state->optimizedLetBlock;
        if (state->optimizedLetBlock != state->varBlock) {
            state->letBlockNode->createBlockObject = 1;
        }
    }
    parameters = np->function.parameters;
    numArgs = (parameters) ? mprGetListLength(parameters->children) : 0;
    
    if (np->function.resultType) {
        np->attributes |= np->function.resultType->attributes;
    }
    if (np->function.isConstructor) {
        mprAssert(ejsIsType(ejs, block));
        np->attributes |= EJS_FUN_CONSTRUCTOR;
        fun = (EjsFunction*) block;
        ejsInitFunction(ejs, fun, np->qname.name, NULL, 0, numArgs, 0, 0, NULL, np->attributes, state->currentModule, 
            NULL, cp->fileState->strict);
        
    } else {
        /*
            Create a function object. Don't have code yet so we create without it. Can't resolve the return type yet, so we 
            leave it unset. The numDefault and numExceptions will be fixed when the function is bound.
         */
        fun = ejsCreateFunction(ejs, np->qname.name, NULL, 0, numArgs, 0, 0, NULL, np->attributes, state->currentModule, 
            NULL, cp->fileState->strict);
        if (fun == 0) {
            astError(cp, np, "Can't create function \"%s\"", np->qname.name);
            return 0;
        }  

        /*
            Check if this function has already been defined in this block. Can't check base classes yes. Must wait till 
            bindFunction()
         */
        slotNum = ejsLookupProperty(ejs, block, np->qname);

        if (slotNum >= 0 && cp->fileState->strict) {
            if ((np->function.setter && ejsPropertyHasTrait(ejs, block, slotNum, EJS_TRAIT_SETTER)) ||
                (np->function.getter && ejsPropertyHasTrait(ejs, block, slotNum, EJS_TRAIT_GETTER))) {
                astError(cp, np, "Property \"%s\" is already defined.", np->qname);
                return 0;
            }
        }
        slotNum = ejsDefineProperty(ejs, block, slotNum, np->qname, TYPE(fun), np->attributes, (EjsObj*) fun);
        if (slotNum < 0) {
            astError(cp, np, "Can't define function in type \"%s\"", state->currentClass->qname.name);
            return 0;
        }
    }
    np->function.functionVar = fun;
    fun->activation = (EjsPot*) ejsCreateCompilerFrame(ejs, fun);
    return fun;
}


/*
    Define function parameters during the DEFINE phase.
 */
static int defineParameters(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EjsFunction     *fun;
    EcNode          *nameNode, *child, *parameters;
    int             attributes, next, slotNum, numDefault;

    ejs = cp->ejs;
    parameters = np->function.parameters;
    if (parameters == 0) {
        return 0;
    }
    fun = np->function.functionVar;
    slotNum = 0;
    next = 0;
    numDefault = 0;

    while ((child = getNextAstNode(cp, parameters, &next))) {
        mprAssert(child->kind == N_VAR_DEFINITION);
        attributes = 0;
        nameNode = 0;
        if (child->left->kind == N_QNAME) {
            nameNode = child->left;
        } else if (child->left->kind == N_ASSIGN_OP) {
            numDefault++;
            nameNode = child->left->left;
        }
        attributes |= nameNode->attributes;
        slotNum = ejsDefineProperty(ejs, (EjsObj*) fun->activation, slotNum, nameNode->qname, NULL, attributes, NULL);
        mprAssert(slotNum >= 0);
        /*
            Can assign the lookup information here as these never need fixups.
         */
        nameNode->lookup.slotNum = slotNum;
        nameNode->lookup.obj = (EjsObj*) fun;
        nameNode->lookup.trait = ejsGetPropertyTraits(ejs, fun->activation, slotNum);
        mprAssert(nameNode->lookup.trait);
        slotNum++;
    }
    fun->numDefault = numDefault;
    if (np->function.getter && fun->numArgs != 0) {
        astError(cp, np, "Getter function \"%s\" must not define parameters.", np->qname.name);
    }
    if (np->function.setter && fun->numArgs != 1) {
        astError(cp, np, "Setter function \"%s\" must define exactly one parameter.", np->qname.name);
    }
    return 0;
}


/*
    Bind the function parameter types. Local variables get bound as the block gets traversed.
 */
static void bindParameters(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EcNode          *child, *varNode, *assignNode, *parameters, *localType;
    EjsTrait        *trait;
    EjsFunction     *fun;
    EjsType         *arrayType;
    int             next, slotNum, arraySlot, attributes;

    ejs = cp->ejs;
    fun = np->function.functionVar;
    next = 0;
    parameters = np->function.parameters;

    if (parameters) {
        while ((child = getNextAstNode(cp, parameters, &next))) {
            mprAssert(child->kind == N_VAR_DEFINITION);
            slotNum = next - 1;
            trait = ejsGetPropertyTraits(ejs, fun->activation, slotNum);
            attributes = trait->attributes;
            
            varNode = 0;
            if (child->left->kind == N_QNAME) {
                varNode = child->left;

            } else if (child->left->kind == N_ASSIGN_OP) {
                /*
                    Bind defaulting parameters. Only need to do if there is a body. Native functions ignore this code as they
                    have no body. The lhs must be scoped inside the function. The rhs must be scoped outside.
                 */
                if (np->function.body) {
                    assignNode = child->left;
                    openBlock(cp, np->function.body, (EjsBlock*) fun->activation);
                    processAstNode(cp, assignNode->left);
                    closeBlock(cp);
                    processAstNode(cp, assignNode->right);
                }
                varNode = child->left->left;
            }
            mprAssert(varNode);
            mprAssert(varNode->kind == N_QNAME);

            if (varNode->typeNode == 0) {
                if (varNode->name.isRest) {
                    arraySlot = ejsLookupProperty(ejs, ejs->global, N(EJS_EJS_NAMESPACE, "Array"));
                    mprAssert(arraySlot >= 0);
#if UNUSED
                    ejsSetTraitType(cp->ejs, trait, ejsGetProperty(ejs, ejs->global, arraySlot));
#else
                    arrayType = ejsGetProperty(ejs, ejs->global, arraySlot);
                    ejsSetPropertyTraits(ejs, fun->activation, slotNum, arrayType, -1);
#endif
                    fun->rest = 1;
                }

            } else {
                localType = varNode->typeNode;
                processAstNode(cp, localType);
                if (localType->lookup.ref) {
                    attributes |= (localType->attributes & (EJS_TRAIT_CAST_NULLS | EJS_TRAIT_THROW_NULLS));
#if UNUSED
                    ejsSetTraitAttributes(cp->ejs, trait, attributes);
                    ejsSetTraitType(cp->ejs, trait, (EjsType*) localType->lookup.ref);
#else
                    ejsSetPropertyTraits(cp->ejs, fun->activation, slotNum, (EjsType*) localType->lookup.ref, attributes);
#endif
                }
            }
        }
    }
}


/*
    Utility routine to bind function return type and locals/args
 */
static EjsFunction *bindFunction(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EcNode          *resultTypeNode;
    EcState         *state;
    EjsType         *iface, *currentClass;
    EjsFunction     *fun;
    EjsBlock        *block;
    EjsName         qname;
    bool            modified;
    int             slotNum, next;

    mprAssert(cp->phase >= EC_PHASE_BIND);
    mprAssert(np->kind == N_FUNCTION);
    mprAssert(np->qname.name);

    state = cp->state;
    ejs = cp->ejs;
    fun = np->function.functionVar;
    currentClass = state->currentClass;
    mprAssert(fun);

    if (np->function.isMethod) {
        block = getBlockForDefinition(cp, np, state->varBlock, np->attributes);
    } else {
        block = state->optimizedLetBlock;
    }
    resultTypeNode = np->function.resultType;

    if (cp->phase == EC_PHASE_BIND) {
        /*
            Exclude a literalNamespace as the empty phase as the namespace name is changed for the URI.
            Exclude constructors which are hidden in the virtual constructor namespace.
         */
        if (!np->literalNamespace && !np->function.isConstructor) {
            if (resolveNamespace(cp, np, block, &modified) == 0) {
                return 0;
            }
            if (modified && np->left) {
                np->left->qname = np->qname;
            }
        }
        if ((EjsObj*) block == ejs->global) {
            addGlobalProperty(cp, np, &np->qname);
        }
    }
    
    /*
        Test for clashes with non-overridden methods in base classes.
     */
    if (currentClass && currentClass->baseType) {
        slotNum = ecLookupVar(cp, (EjsObj*) currentClass->baseType, np->qname);
        if (slotNum >= 0 && ejsIsA(ejs, np->lookup.ref, (EjsType*) cp->lookup.obj)) {
            if (!(np->attributes & EJS_FUN_OVERRIDE) && !currentClass->baseType->isInterface) {
                //  MOB - OPT
                if (currentClass->qname.space != ejsCreateStringFromAsc(ejs, EJS_EJS_NAMESPACE) && 
                    currentClass->qname.name != ejsCreateStringFromAsc(ejs, "Type")) {
                    astError(cp, np, 
                        "Function \"%s\" is already defined in a base class. Using \"override\" keyword.", np->qname.name);
                    return 0;
                }
            }

            mprAssert(!ejsLookupProperty(ejs, (EjsObj*) currentClass, np->qname));
            slotNum = -1;
#if MOB && BINDING_ONLY
            /*
                Install the new function into the v-table by overwriting the method from the closest base class.
                Must now define the name of the property and attributes.
             */
#endif
            ejsDefineProperty(ejs, (EjsObj*) block, slotNum, np->qname, 0, np->attributes, (EjsObj*) fun);
        }
    }

    /*
        Test for clashes with non-overridden methods in implemented classes.
     */
    if (state->currentClass && state->currentClass->implements) {
        next = 0;
        while ((iface = (EjsType*) mprGetNextItem(state->currentClass->implements, &next))) {
            slotNum = ecLookupVar(cp, (EjsObj*) iface, np->qname);
            if (slotNum >= 0 && cp->lookup.obj == (EjsObj*) iface) {
                if (!iface->isInterface) {
                    if (!(np->attributes & EJS_FUN_OVERRIDE)) {
                        astError(cp, np, 
                            "Function \"%s\" is already defined in an implemented class. Use the \"override\" keyword.", 
                            np->qname.name);
                        return 0;
                    }

                    /*
                        Install the new function into the v-table by overwriting the inherited implemented method.
                     */
                    ejsDefineProperty(ejs, (EjsObj*) block, slotNum, np->qname, 0, np->attributes, (EjsObj*) fun);
                }
            }
        }
    }

    if (resultTypeNode) {
        //  MOB -- assumes all types are in global. Should do a scope search?
        if (resolveName(cp, resultTypeNode, ejs->global, resultTypeNode->qname) < 0) {
            if (STRICT_MODE(cp)) {
                astError(cp, np, "Can't find type \"%s\". All variables must be declared and typed in strict mode.", 
                    resultTypeNode->qname.name);
            }
        } else {
            resultTypeNode->qname.space = resultTypeNode->lookup.name.space;
        }
    }

    if (!np->function.isConstructor) {
        if (resolveName(cp, np, (EjsObj*) block, np->qname) < 0) {
            astError(cp, np, "Internal error. Can't resolve function %s", np->qname.name);
        }
        if (np->lookup.slotNum >= 0) {
            setAstDocString(ejs, np, np->lookup.obj, np->lookup.slotNum);
        }
    } else {
        qname.space = NULL;
        qname.name = np->qname.name;
        if (resolveName(cp, np, ejs->global, qname) < 0) {
            if (resolveName(cp, np, ejs->global, np->qname) < 0) {
                astError(cp, np, "Internal error. Can't resolve constructor %s", np->qname.name);
            }
        }
        if (np->lookup.slotNum >= 0) {
            setAstDocString(ejs, np, np->lookup.obj, np->lookup.slotNum);
        }
    }

    /*
        Bind the result type. Set the result type in np->trait->type
     */
    if (resultTypeNode) {
        mprAssert(resultTypeNode->lookup.ref == 0 || ejsIsType(ejs, resultTypeNode->lookup.ref));
        fun->resultType = (EjsType*) resultTypeNode->lookup.ref;
    }

    /*
        Optimize away closures
        Global functions need scope for the "internal" namespace. If defined as public, dont need it.
        TODO OPT. Dont set fullScope if public
     */
    if (/* fun->owner == ejs->global || */ np->function.isMethod || np->attributes & EJS_PROP_NATIVE) {
        fun->fullScope = 0;
    } else {
        fun->fullScope = 1;
    }
#if UNUSED
        //  MOB -- refactor
    if (!ejsIsType(ejs, block) || !(np->attributes & EJS_PROP_STATIC)) {
        if (np->function.isMethod) {
            mprAssert(ejsIsBlock(ejs, state->varBlock));
            fun->block.scope = ((EjsBlock*) state->varBlock)->scope;
        } else {
            mprAssert(ejsIsBlock(ejs, state->optimizedLetBlock));
            fun->block.scope = ((EjsBlock*) state->optimizedLetBlock)->scope;
        }
    } else {
        mprAssert(ejsIsBlock(ejs, block));
        fun->block.scope = (EjsBlock*) block;
    }
#else
    if (!np->function.isConstructor) {
        if (np->function.isMethod) {
            fun->block.scope = (EjsBlock*) state->varBlock;
        } else {
            fun->block.scope = (EjsBlock*) state->optimizedLetBlock;
        }
    }
#endif
    return fun;
}


/*
    Process the N_FUNCTION node and bind the return type and parameter types
 */
static void astFunction(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EjsFunction     *fun;
    EjsBlock        *block;
    EcState         *state;

    mprAssert(np->kind == N_FUNCTION);
    ENTER(cp);

    ejs = cp->ejs;
    state = cp->state;
    fun = np->function.functionVar;

    if (state->disabled) {
        if (cp->phase == EC_PHASE_CONDITIONAL) {
            //  MOB -- refactor this somehow
            if (np->function.isMethod) {
                block = getBlockForDefinition(cp, np, state->varBlock, np->attributes);
            } else {
                block = state->optimizedLetBlock;
            }
            removeProperty(cp, (EjsObj*) block, np);
        }
        LEAVE(cp);
        return;
    }

    /*
        Process the function definition (no parameters yet)
     */
    if (cp->phase == EC_PHASE_DEFINE) {
        fun = defineFunction(cp, np);
    } else if (cp->phase >= EC_PHASE_BIND) {
        fun = bindFunction(cp, np);
    }
    if (fun == 0) {
        LEAVE(cp);
        return;
    }
    if (cp->phase == EC_PHASE_DEFINE) {
        defineParameters(cp, np);
    } else if (cp->phase >= EC_PHASE_BIND) {
        bindParameters(cp, np);
    }
    state->currentFunction = fun;
    state->currentFunctionNode = np;
    state->inFunction = 1;
    state->inMethod = state->inMethod || np->function.isMethod;
    state->blockIsMethod = np->function.isMethod;

    state->optimizedLetBlock = (EjsBlock*) fun->activation;
    state->letBlock = (EjsBlock*) fun->activation;
    state->varBlock = (EjsBlock*) fun->activation;

    if (np->function.body) {
        mprAssert(fun->activation);
        mprAssert(ejsIsFrame(ejs, fun->activation));
        openBlock(cp, np->function.body, (EjsBlock*) fun->activation);
        mprAssert(np->function.body->kind == N_DIRECTIVES);
        processAstNode(cp, np->function.body);
        closeBlock(cp);
    }
    /*
        Fixup scope if the class has a static initializer. The static initializer is opened for static initialization
        statements. TODO - refactor this some how.
     */
    if (state->inMethod && state->currentClassNode->klass.initializer) {
        if (fun->block.scope == (EjsBlock*) state->currentClassNode->klass.initializer) {
            fun->block.scope = fun->block.scope->scope;
        }
    }
    if (np->function.constructorSettings) {
        /*
            TODO The constructor settings need special namespace treatment. Consider:
            class Shape {
                var x
                function Shape(arg1) : this.x = arg1 {}
            }
        
            Note the left hand side can use "this" whereas the right hand side must not.
            The right hand side can see the parameters wheres the left hand side must not.
         */
        state->inSettings = 1;
        processAstNode(cp, np->function.constructorSettings);
        state->inSettings = 0;
    }

    /*
        TODO -- No need to add this namespace to be emitted as all function variables are bound (always)
        Process the parameters. Scope for default initialization code for the parameters is as follows:
            left hand side:  inside the function block
            right hand side: outside the function block.
    
        Namespaces are done on each phase because pragmas must apply only from the point of declaration onward 
        (use namespace)
     */
    if (cp->phase >= EC_PHASE_BIND) {
        if (!np->function.hasReturn && (np->function.resultType != 0)) {
            if (fun->resultType == 0 || fun->resultType != ejs->voidType) {
                /*
                    Native classes have no body defined in script, so we can't verify whether or not it has 
                    an appropriate return.
                 */
                if (!(state->currentClass && state->currentClass->isInterface) && !(np->attributes & EJS_PROP_NATIVE)) {
                    /*
                        When building slots for the core VM (empty mode), we can't test ejs->voidType as this won't equal
                        the parsed Void class
                     */
                    if (ejs->initialized || fun->resultType == 0 || 
                            fun->resultType->qname.name != ejs->voidType->qname.name) {
                        astError(cp, np, "Function \"%s\" must return a value",  np->qname.name);
                    }
                }
            }
        }
        if (fun->activation->numProp == 0) {
            /* Activation object not required */
            fun->activation = 0;
        }
    }
    LEAVE(cp);
}


/*
    Handle a for statement
 */
static void astFor(EcCompiler *cp, EcNode *np)
{
    ENTER(cp);

    mprAssert(np->kind == N_FOR);

    if (np->forLoop.initializer) {
        processAstNode(cp, np->forLoop.initializer);
    }
    if (np->forLoop.cond) {
        processAstNode(cp, np->forLoop.cond);
    }
    if (np->forLoop.perLoop) {
        processAstNode(cp, np->forLoop.perLoop);
    }
    if (np->forLoop.body) {
        processAstNode(cp, np->forLoop.body);
    }
    LEAVE(cp);
}


/*
    Handle a for/in statement
 */
static void astForIn(EcCompiler *cp, EcNode *np)
{
    Ejs         *ejs;
    int         rc;

    ENTER(cp);

    mprAssert(np->kind == N_FOR_IN);
    
    ejs = cp->ejs;

    if (np->forInLoop.iterVar) {
        processAstNode(cp, np->forInLoop.iterVar);
    }
    if (np->forInLoop.iterGet) {
        processAstNode(cp, np->forInLoop.iterGet);
    }

    /*
        Link to the iterGet node so we can bind the "next" call.
     */
    if (cp->phase >= EC_PHASE_BIND) {
#if UNUSED
        EjsType     *iteratorType;
        EjsName     qname;
        iteratorType = (EjsType*) ejsGetPropertyByName(ejs, ejs->global, N("iterator", "Iterator"));
        mprAssert(iteratorType);
        if (iteratorType == 0) {
            astError(cp, np, "Can't find Iterator class");
        } else {
            /*
                TODO MOB - this assumes that iterators use Iterator and it is bindable. What if an operator that
                implements an Iterable/Iterator interface
             */
            rc = resolveName(cp, np->forInLoop.iterNext, (EjsObj*) iteratorType->prototype, N("public", "next"));
            if (rc < 0) {
                astError(cp, np, "Can't find Iterator.next method");
            }
        }
        
#else
        np->forInLoop.iterNext->qname = N("public", "next");
        rc = resolveName(cp, np->forInLoop.iterNext, (EjsObj*) ejs->iteratorType->prototype, np->forInLoop.iterNext->qname);
        np->forInLoop.iterNext->lookup.bind = 0;
#endif
    }
    if (np->forInLoop.body) {
        processAstNode(cp, np->forInLoop.body);
    }
    LEAVE(cp);
}


static EjsObj *evalNode(EcCompiler *cp, EcNode *np)
{
    Ejs         *ejs;
    EjsModule   *mp;
    EjsObj      *result;
    int         saveDebug;

    ejs = cp->ejs;
    mprAssert(ejs->exception == 0);
    
    if ((mp = ejsCreateModule(cp->ejs, ejsCreateStringFromAsc(ejs, "__conditional__"), 0, NULL)) == NULL) {
        return 0;
    }
    mp->initializer = createModuleInitializer(cp, np, mp);
    mp->initializer->isInitializer = 1;
    mp->hasInitializer = 1;
    saveDebug = cp->debug;
    cp->debug = 0;

    if (astProcess(cp, np) < 0) {
        cp->debug = saveDebug;
        return 0;
    }
    ecResetParser(cp);
    ecGenConditionalCode(cp, np, mp);
    if (cp->errorCount > 0) {
        cp->debug = saveDebug;
        return 0;
    }
    /*  Install the Config object as a local variable in the initializer function */
    ejsDefineProperty(ejs, mp->initializer->activation, 0, EN("Config"), ejs->typeType, 0, (EjsObj*) ejs->configType);
    ejsSetPropertyName(ejs, (EjsObj*) ejs->configType, ES_Config_Legacy, EN("Legacy"));
    result = ejsRunInitializer(ejs, mp);
    cp->debug = saveDebug;
    if (result == 0) {
        ejsClearException(ejs);
        return 0;
    }
    return result;
}


/*
    Handle an hash statement (conditional compilation)
 */
static void astHash(EcCompiler *cp, EcNode *np)
{
    EjsObj          *result;
    int             savePhase;

    ENTER(cp);

    mprAssert(np->kind == N_HASH);
    mprAssert(np->hash.expr);
    mprAssert(np->hash.body);

    cp->state->inHashExpression = 1;

    if (cp->phase < EC_PHASE_CONDITIONAL) {
        processAstNode(cp, np->hash.expr);

    } else if (cp->phase == EC_PHASE_CONDITIONAL) {
        ENTER(cp);
        savePhase = cp->phase;
        result = evalNode(cp, np->hash.expr);
        cp->phase = savePhase;
        LEAVE(cp);

        if (result) {
            result = (EjsObj*) ejsToBoolean(cp->ejs, result);
            if (result && !ejsGetBoolean(cp->ejs, result)) {
                result = 0;
            }
        }
        if (result == 0) {
            np->hash.disabled = 1;
        }
    }
    if (np->hash.disabled) {
        cp->state->disabled = 1;
    }
    cp->state->inHashExpression = 0;
    processAstNode(cp, np->hash.body);
    LEAVE(cp);
}


/*
    Handle an if statement (tenary node)
 */
static void astIf(EcCompiler *cp, EcNode *np)
{
    ENTER(cp);

    mprAssert(np->kind == N_IF);

    mprAssert(np->tenary.cond);
    mprAssert(np->tenary.thenBlock);

    processAstNode(cp, np->tenary.cond);
    processAstNode(cp, np->tenary.thenBlock);

    if (np->tenary.elseBlock) {
        processAstNode(cp, np->tenary.elseBlock);
    }
    LEAVE(cp);
}


static void astImplements(EcCompiler *cp, EcNode *np)
{
    EcNode      *child;
    int         next;
    
    ENTER(cp);
    
    mprAssert(np->kind == N_TYPE_IDENTIFIERS);
    
    next = 0;
    while ((child = getNextAstNode(cp, np, &next))) {
        processAstNode(cp, child);
    }
    LEAVE(cp);
}


/*
    Generate a name reference. This routine will bind a name path reference into slot bindings if possible.
    The node and its children represent a  name path.
 */
static void astName(EcCompiler *cp, EcNode *np)
{
    if (np->name.qualifierExpr) {
        processAstNode(cp, np->name.qualifierExpr);
    }
    if (np->name.nameExpr) {
        processAstNode(cp, np->name.nameExpr);
    }
    if (cp->phase >= EC_PHASE_BIND) {
        astBindName(cp, np);
        return;
    }
}


static void astBindName(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EjsLookup       *lookup;
    EjsFunction     *fun, *currentFunction;
    EcNode          *left;
    EcState         *state;
    int             rc;

    mprAssert(cp->phase >= EC_PHASE_BIND);
    mprAssert(np->kind == N_QNAME);
    
    if (np->qname.name == 0 || np->name.qualifierExpr || np->name.nameExpr) {
        return;
    }
    ENTER(cp);
    state = cp->state;

    /*
        If resolving a name to the right of a "." or "[", then only search relative to the object to the left of the dot.
     */
    left = state->currentObjectNode;
    ejs = cp->ejs;
    rc = -1;

    if (np->name.isType) {
        rc = resolveName(cp, np, ejs->global, np->qname);
        if (rc < 0) {
            //  NOTE: np->qname.space may be null
            astError(cp, np, "Can't find class \"%@\". Ensure the class is visible.", np->qname.name);
        }

    } else if (left) {
        if (left->kind == N_THIS) {
            /*
                Explicit "this.property"
                TODO - does not yet handle "this function, this callee, this function, this type"
             */
            if (state->currentClass) {
                rc = resolveProperty(cp, np, state->currentClass, np->qname);
                if (rc < 0 && STRICT_MODE(cp)) {
                    astError(cp, np, "Can't find property \"%s\" in this class %s.", np->qname.name, 
                         state->currentClass->qname.name);
                }
            }

        /*
            Do we know the type of the left side?
         */
        } else if (left->lookup.trait && left->lookup.trait->type) {
            /*
                We must handle 2 cases differently:
                    1. obj.property
                    2. Type.property
            
                This is because in the first case, we must extract the type of an object, whereas in the 2nd case,
                we already have the type via an explicit type reference.
             */
            if (left->lookup.ref && (ejsIsType(ejs, left->lookup.ref) 
                        /* UNUSED || ejsIsPrototype(ejs, left->lookup.ref) */)) {
                /*
                    Case 2. Type.property. We have resolved the type reference.
                 */
                np->lookup.ownerIsType = 1;
                rc = resolveProperty(cp, np, (EjsType*) left->lookup.ref, np->qname);
                if (rc < 0 && STRICT_MODE(cp) && !((EjsType*) left->lookup.ref)->dynamicInstance) {
                    astError(cp, np, "Can't find property \"%s\" in class \"%s\".", np->qname.name,
                        ((EjsType*) left->lookup.ref)->qname.name);

                } else if (np->lookup.trait && !(np->lookup.trait->attributes & EJS_PROP_STATIC) &&
                        np->lookup.obj != ejs->global) {
                    if (np->lookup.slotNum >= ES_Object_NUM_CLASS_PROP) {
                        /* Exclude the case of calling a function (constructor) to create a new instance */
                        if (!(left->kind == N_CALL || left->kind == N_EXPRESSIONS)) {
                            astError(cp, np, "Accessing instance level propery \"%s\" without an instance", np->qname.name);
                        }
                    }
                    
                } else if (left->kind == N_CALL) {
                    /*
                        Calling a constructor as a function. This will return an instance
                     */
                    np->lookup.nthBase++;
                }

            } else {
                fun = (EjsFunction*) left->lookup.ref;
                if (fun && ejsIsFunction(ejs, fun)) {
                    /* 
                        Can't use a getter to bind to as the value is determined at run time.
                     */
                    rc = -1;

                } else {

                    /*
                        Case 1: Left side is a normal object. We use the type of the lhs to search for name.
                     */
                    rc = resolveProperty(cp, np, left->lookup.trait->type, np->qname);
                    if (rc == 0) {
                        /*
                            Since we searched above on the type of the object and the lhs is an object, increment nthBase.
                            BUG: but what if lhs is a type? then nthBase is one too many
                         */
                        if (!np->lookup.instanceProperty) {
                            np->lookup.nthBase++;
                        }
                    }
                }
            }

        } else if (left->kind == N_EXPRESSIONS) {
            /* 
                Suppress error message below. We can't know the left because it is an expression. 
                So we can't bind the variable 
             */
            rc = 0;
        }

    } else {
        /*
            No left side, so search the scope chain
         */
        rc = resolveName(cp, np, NULL, np->qname);

        /*
            Check for static function code accessing instance properties or instance methods
         */
        lookup = &np->lookup;
        if (rc == 0 && state->inClass && !state->instanceCode) {
            if (ejsIsPrototype(ejs, lookup->obj) || 
                    (ejsIsType(ejs, lookup->obj) && (lookup->trait && !(lookup->trait->attributes & EJS_PROP_STATIC)))) {
                if (!state->inFunction || (state->currentFunctionNode->attributes & EJS_PROP_STATIC)) {
                    astError(cp, np, "Accessing instance level property \"%s\" without an instance", np->qname.name);
                    rc = -1;
                }
            }
        }
    }

    if (rc < 0) {
#if FUTURE
        //  TODO - need propert expression calculation and propagation
        if (left && left->lookup.trait && left->lookup.trait->type) {
            type = left->lookup.trait->type;
            if (!type->dynamicInstance) {
                astError(cp, np, "Can't find a declaration for \"%s\".", np->qname.name);
            }
        }
#endif
        if (STRICT_MODE(cp) && !cp->error) {
            astError(cp, np, "Can't find a declaration for \"%s\". All variables must be declared and typed in strict mode.",
                np->qname.name);
        }

    } else {
        if (np->lookup.trait) {
            /* Copy traits of property into the reference. Not sure if all are needed */
            np->attributes |= np->lookup.trait->attributes;
        }
    }

    /*
        Disable binding of names in certain cases.
     */
    lookup = &np->lookup;
    
    if (ejsIsFrame(ejs, lookup->obj) && lookup->nthBlock == 0) {
        ;
    } else {
        lookup->bind = 0;
        lookup->useThis = 0;
    }

#if UNUSED
    if (lookup->obj != (EjsObj*) state->currentFunction || ejsIsType(ejs, lookup->obj)) {
        lookup->bind = 0;
        lookup->useThis = 0;
    }
#endif

#if UNUSED
    if (lookup->slotNum >= 0) {
        /*
            Unbind if slot number won't fit in one byte or the object is not a standard Object. The bound op codes 
            require one byte slot numbers.
         */
        if (lookup->slotNum >= 256) {
            lookup->bind = 0;
        }

        if (lookup->obj == ejs->global && !cp->bind) {
            /*
                Unbind non-core globals
             */
            if ((lookup->slotNum >= ES_global_NUM_CLASS_PROP) && !(lookup->ref && BUILTIN(lookup->ref))) {
                lookup->bind = 0;
            }
        }
        if (ejsIsType(ejs, np->lookup.obj)) {
            type = (EjsType*) np->lookup.obj;
            if (type->constructor.block.nobind || type->isInterface) {
                /*
                    Type requires non-bound access. Types that implement interfaces will have different slots.
                 */
                lookup->bind = 0;

            } else if (type->dynamicInstance && !BUILTIN(type)) {
                /*
                    Don't bind non-core dynamic properties
                 */
                lookup->bind = 0;

            } else {
                /*
                    Ugly (but effective) hack just for XML to discriminate between length and length()
                    TODO - refactor away
                 */
                if (type == ejs->xmlType || type == ejs->xmlListType) {
                    if (np->parent == 0 || np->parent->parent == 0 || np->parent->parent->kind != N_CALL) {
                        lookup->bind = 0;
                    }
                }
            }

        } else if (ejsIsPrototype(ejs, np->lookup.obj)) {
            if (!BUILTIN(np->lookup.obj)) {
                lookup->bind = 0;
            }
        }
        if (lookup->trait && lookup->trait->attributes & EJS_TRAIT_GETTER) {
            lookup->bind = 0;
        }
    }
#endif

    /*
        If accessing unbound variables, then the function will require full scope if a closure is ever required.
     */
    currentFunction = state->currentFunction;
    if (lookup->slotNum < 0) {
        if (cp->phase == EC_PHASE_BIND && cp->warnLevel > 5) {
            astWarn(cp, np, "Using unbound variable reference for \"%s\"", np->qname.name);
        }
    }
    LEAVE(cp);
}


static void astNew(EcCompiler *cp, EcNode *np)
{
    EjsType     *type;
    EcNode      *left;

    mprAssert(np->kind == N_NEW);
    mprAssert(np->left);
    mprAssert(np->left->kind == N_QNAME || np->left->kind == N_DOT);
    mprAssert(np->right == 0);

    ENTER(cp);

    left = np->left;
    processAstNode(cp, left);

    if (cp->phase != EC_PHASE_BIND) {
        LEAVE(cp);
        return;
    }
    mprAssert(cp->phase >= EC_PHASE_BIND);

    np->newExpr.callConstructors = 1;

    if (left->lookup.ref) {
        type = (EjsType*) left->lookup.ref;
        if (type && ejsIsType(cp->ejs, type)) {
            /* Type is bound, has no constructor or base class constructors */
            if (!type->hasConstructor && !type->hasBaseConstructors) {
                np->newExpr.callConstructors = 0;
            }
            /*
                Propagate up the left side. Increment nthBase because it is an instance.
             */
            np->qname = left->qname;
            np->lookup = left->lookup;
            np->lookup.trait = mprAllocObj(EjsTrait, NULL);
            np->lookup.trait->type = (EjsType*) np->lookup.ref;
            np->lookup.ref = 0;
            np->lookup.instanceProperty = 1;
        }
    }
    LEAVE(cp);
}


static void astObjectLiteral(EcCompiler *cp, EcNode *np)
{
    EcNode      *child;
    int         next;

    mprAssert(np->kind == N_OBJECT_LITERAL);

    processAstNode(cp, np->objectLiteral.typeNode);
    for (next = 0; (child = getNextAstNode(cp, np, &next)) != 0; ) {
        processAstNode(cp, child);
    }
}


static void astField(EcCompiler *cp, EcNode *np)
{
    if (np->field.fieldKind == FIELD_KIND_VALUE && np->field.expr) {
        processAstNode(cp, np->field.expr);
    }
}


static void astPragmas(EcCompiler *cp, EcNode *np)
{
    Ejs         *ejs;
    EcNode      *child;
    int         next;

    mprAssert(np->kind == N_PRAGMAS);

    ENTER(cp);
    ejs = cp->ejs;

    next = 0;
    while ((child = getNextAstNode(cp, np, &next))) {
        processAstNode(cp, child);
    }
    LEAVE(cp);
}


static void astPragma(EcCompiler *cp, EcNode *np)
{
    mprAssert(np->kind == N_PRAGMA);

    ENTER(cp);
    if (np->pragma.strict) {
        cp->fileState->strict = np->pragma.strict;
    }
    LEAVE(cp);
}



static void astPostfixOp(EcCompiler *cp, EcNode *np)
{
    EcNode      *left;
    
    mprAssert(np->kind == N_POSTFIX_OP);

    ENTER(cp);
    left = np->left;
    if (left->kind == N_LITERAL) {
        astError(cp, np, "Invalid postfix operand");
    } else {
        processAstNode(cp, np->left);
    }
    LEAVE(cp);
}


static void astProgram(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EcState         *state;
    EcNode          *child;
    int             next;

    ENTER(cp);
    ejs = cp->ejs;
    state = cp->state;
    state->nspace = np->qname.name;

    next = 0;
    while ((child = getNextAstNode(cp, np, &next)) != 0) {
        processAstNode(cp, child);
    }
    LEAVE(cp);
}


static void astReturn(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EjsFunction     *fun;
    EcNode          *functionNode;
    EcState         *state;

    ENTER(cp);
    ejs = cp->ejs;
    state = cp->state;

    mprAssert(state->currentFunctionNode->kind == N_FUNCTION);
    state->currentFunctionNode->function.hasReturn = 1;

    if (np->left) {
        processAstNode(cp, np->left);
    }

    if (cp->phase >= EC_PHASE_BIND) {
        mprAssert(state->currentFunction);
        mprAssert(state->currentFunction);
        functionNode = state->currentFunctionNode;
        state->currentFunction->hasReturn = functionNode->function.hasReturn;

        fun = state->currentFunction;
        if (fun->hasReturn) {
            if (np->left) {
                if (fun->resultType && fun->resultType == ejs->voidType) {
                    /*
                        Allow block-less function expressions where a return node was generated by the parser.
                     */
                    if (!np->ret.blockless) {
                        astError(cp, np, "Void function \"%s\" can't return a value", functionNode->qname.name);
                    }
                }

            } else {
                if (fun->resultType && fun->resultType != ejs->voidType) {
                    if (! (!ejs->initialized && fun->resultType->qname.name == ejs->voidType->qname.name)) {
                        astError(cp, np, "Return in function \"%@\" must return a value", functionNode->qname.name);
                    }
                }
            }
        }
    }
    LEAVE(cp);
}


static void astSuper(EcCompiler *cp, EcNode *np)
{
    EcState     *state;

    ENTER(cp);
    state = cp->state;
    if (state->currentObjectNode == 0) {
        if (state->currentFunction == 0) {
            if (cp->phase == EC_PHASE_DEFINE) {
                astError(cp, np, "Can't use unqualified \"super\" outside a method");
            }
            LEAVE(cp);
            return;
        }
        if (!state->currentFunctionNode->function.isConstructor) {
            if (cp->phase == EC_PHASE_DEFINE) {
                astError(cp, np, "Can't use unqualified \"super\" outside a constructor");
            }
            LEAVE(cp);
            return;
        }
        if (cp->phase >= EC_PHASE_BIND) {
            if (state->currentClass->hasBaseConstructors == 0) {
                astError(cp, np, "No base class constructors exist to call via super");
                LEAVE(cp);
                return;
            }
        }
        state->currentClass->callsSuper = 1;
        if (np->left && np->left->kind != N_NOP) {
            processAstNode(cp, np->left);
        }

    } else {
        astError(cp, np, "Can't use unqualified \"super\" outside a method");
    }
    LEAVE(cp);
}


static void astSwitch(EcCompiler *cp, EcNode *np)
{
    EcNode      *child;
    int         next;

    ENTER(cp);

    mprAssert(np->kind == N_SWITCH);
    mprAssert(np->right->kind == N_CASE_ELEMENTS);

    next = 0;
    while ((child = getNextAstNode(cp, np, &next))) {
        processAstNode(cp, child);
    }
    LEAVE(cp);
}


static void astThis(EcCompiler *cp, EcNode *np)
{
    EcState     *state;

    ENTER(cp);
    state = cp->state;

    switch (np->thisNode.thisKind) {
    case EC_THIS_GENERATOR:
        //  TODO - binding not implemented
        break;

    case EC_THIS_CALLEE:
        //  TODO - binding not implemented
        break;

    case EC_THIS_TYPE:
        //  TODO - binding not implemented
        if (!state->inClass) {
            astError(cp, np, "\"this type\" is only valid inside a class");
        } else {
            np->lookup.obj = (EjsObj*) state->currentClass;
            np->lookup.slotNum = 0;
        }
        break;

    case EC_THIS_FUNCTION:
        //  TODO - binding not implemented
        if (!state->inFunction) {
            astError(cp, np, "\"this function\" is not valid outside a function");
        } else {
            np->lookup.obj = (EjsObj*) state->currentFunction;
            np->lookup.slotNum = 0;
        }
        break;

    default:
        np->lookup.obj = (EjsObj*) state->currentClass;
        np->lookup.slotNum = 0;
    }
    LEAVE(cp);
}


static void astThrow(EcCompiler *cp, EcNode *np)
{
    mprAssert(np->left);
    processAstNode(cp, np->left);
}


/*
    Try, catch, finally
 */
static void astTry(EcCompiler *cp, EcNode *np)
{
    Ejs         *ejs;
    EjsBlock    *block;
    EcNode      *child;
    EcState         *state;
    int         next, count;

    ENTER(cp);

    mprAssert(np->kind == N_TRY);
    mprAssert(np->exception.tryBlock);

    ejs = cp->ejs;
    state = cp->state;

    processAstNode(cp, np->exception.tryBlock);

    if (cp->phase == EC_PHASE_BIND) {
        /*
            Calculate the number of lexical blocks in the try block. These must be discarded by the VM when executing
            catch and finally blocks.
         */
        for (count = 0, block = ejs->state->bp->scope; block && !ejsIsFrame(ejs, block); block = block->scope) {
                count++;
        }
        np->exception.numBlocks = count;
    }
    if (np->exception.catchClauses) {
        next = 0;
        while ((child = getNextAstNode(cp, np->exception.catchClauses, &next))) {
            processAstNode(cp, child);
        }
    }
    if (np->exception.finallyBlock) {
        block = ejsCreateBlock(cp->ejs, 0);
        addScope(cp, block);
        processAstNode(cp, np->exception.finallyBlock);
        removeScope(cp);
    }
    LEAVE(cp);
}


/*
    Handle a unary operator.
 */
static void astUnaryOp(EcCompiler *cp, EcNode *np)
{
    ENTER(cp);

    mprAssert(np->kind == N_UNARY_OP);
    mprAssert(np->left);

    if (np->left->kind == N_LITERAL && (np->tokenId == T_PLUS_PLUS || np->tokenId == T_MINUS_MINUS)) {
        astError(cp, np, "Invalid prefix operand");
    } else {
        processAstNode(cp, np->left);
    }
    LEAVE(cp);
}


/*
    Create a module defined via a module directive.
 */
static void astModule(EcCompiler *cp, EcNode *np)
{
    EjsModule       *mp, *core;
    Ejs             *ejs;
    EcState         *state;
    EcNode          *child;
    EjsBlock        *saveChain;
    int             next;

    mprAssert(np->kind == N_MODULE);

    ENTER(cp);

    ejs = cp->ejs;
    state = cp->state;
    
    if (cp->phase == EC_PHASE_DEFINE) {
        mp = createModule(cp, np);
    } else {
        mp = np->module.ref;
        mprAssert(mp);
    }
    if (mp == 0) {
        return;
    }
    mprAssert(mp->initializer);
    mprAssert(mp->initializer->activation);

    /*
        Start a new scope chain for this module. ie. Don't nest modules in the scope chain.
     */
    saveChain = ejs->state->bp->scope;
    ejs->state->bp->scope = mp->scope;

    /*
        Create a block for the module initializer. There is also a child block but that is to hide namespace declarations 
        from other compilation units. Open the block explicitly rather than using astBlock. We do this because we need 
        varBlock to be set to ejs->global and let block to be mp->initializer. The block is really only used to scope 
        namespaces.
     */
    openBlock(cp, np, (EjsBlock*) mp->initializer->activation);
    
    if (cp->phase == EC_PHASE_BIND) {
        /*
            Bind the block here before processing the child nodes so we can mark the block as hidden if it will be expunged.
         */
        bindBlock(cp, np->left);
    }
    
    /*
        Open the child block here so we can set the letBlock and varBlock values inside the block.
     */
    mprAssert(np->left->kind == N_BLOCK);
    openBlock(cp, np->left, NULL);
    
    state->optimizedLetBlock = (EjsBlock*) ejs->global;
    state->varBlock = (EjsBlock*) ejs->global;
    state->letBlock = (EjsBlock*) mp->initializer->activation;
    state->currentModule = mp;

    if (mp->dependencies == 0) {
        mp->dependencies = mprCreateList(ejs);
        core = ejsLookupModule(ejs, ejsCreateStringFromAsc(ejs, "ejs"), 0, 0);
        if (core && core != mp && mprLookupItem(mp->dependencies, core) < 0) {
            mprAddItem(mp->dependencies, core);
        }
    }

    /*
        Skip the first (block) child that was processed manually above.
     */
    for (next = 0; (child = getNextAstNode(cp, np->left, &next)); ) {
        processAstNode(cp, child);
    }
    closeBlock(cp);
    closeBlock(cp);
    
    if (cp->phase == EC_PHASE_CONDITIONAL) {
        /*
            Define block after the variables have been processed. This allows us to determine if the block is really needed.
         */
        defineBlock(cp, np->left);
    }
    ejs->state->bp->scope = saveChain;
    LEAVE(cp);
}


/*
    Use Namespace
 */
static void astUseNamespace(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EjsNamespace    *namespace;
    EcState         *state, *s;

    mprAssert(np->kind == N_USE_NAMESPACE);
    
    ENTER(cp);

    state = cp->state;
    ejs = cp->ejs;
    namespace = 0;

    if (cp->phase == EC_PHASE_CONDITIONAL) {
        /*
            At the define phase, we create a dummy namespace assuming that it will exist somewhere in this block or an 
            outer block. At the fixup phase, we actually resolve the reference to the namespace unless it is a string 
            literal namespace.
         */
        namespace = ejsCreateNamespace(ejs, np->qname.name);
        np->namespaceRef = namespace;

    } else if (cp->phase >= EC_PHASE_BIND) {
        if (np->name.isLiteral) {
            namespace = np->namespaceRef;

        } else {
            /*
                Resolve the real namespace. Must be visible in the current scope (even in standard mode). 
                Then update the URI. URI not used.
             */
            if (resolveName(cp, np, NULL, np->qname) < 0) {
                astError(cp, np, "Can't find namespace \"%s\"", np->qname.name);

            } else {
                //  MOB -- UN BIND
                np->lookup.bind = 0;
                namespace = (EjsNamespace*) np->lookup.ref;
                if (namespace) {
                    np->namespaceRef->value = namespace->value;
                    if (!ejsIsNamespace(ejs, namespace)) {
                        astError(cp, np, "The variable \"%s\" is not a namespace", np->qname.name);
                    } else {
                        np->namespaceRef = namespace;
                    }
                }
            }
            if (namespace && np->name.isDefault) {
                /*
                    Apply the namespace URI to all upper blocks
                    MOB -- not right
                 */
                for (s = cp->state; s; s = s->prev) {
                    s->nspace = namespace->value;
                    if (s == cp->blockState) {
                        break;
                    }
                }
            }
        }
    } else {
        namespace = np->namespaceRef;
    }
    if (namespace) {
        if (state->letBlockNode) {
            state->letBlockNode->createBlockObject = 1;
        }
        if (state->inClass && !state->inFunction) {
            /*
                Must attach to the class itself and not to the outermost block
             */
            ejsAddNamespaceToBlock(ejs, (EjsBlock*) state->currentClass, namespace);
        } else {
            ejsAddNamespaceToBlock(ejs, state->letBlock, namespace);
        }
    }
    LEAVE(cp);
}


/*
    Module depenency
 */
static void astRequire(EcCompiler *cp, EcNode *np)
{
    Ejs         *ejs;
    EjsModule   *currentModule, *mp;
    int         flags;

    mprAssert(np->kind == N_USE_MODULE);
    mprAssert(np->qname.name);

    ENTER(cp);

    ejs = cp->ejs;
    currentModule = cp->state->currentModule;
    mprAssert(currentModule);

    if (cp->phase == EC_PHASE_CONDITIONAL) {
        /*
            Is this a module we are currently compiling?
         */
        mp = ecLookupModule(cp, np->qname.name, np->useModule.minVersion, np->useModule.maxVersion);
        if (mp == 0) {
            /*
                Is this module already loaded by the vm?
             */
            mp = ejsLookupModule(ejs, np->qname.name, np->useModule.minVersion, np->useModule.maxVersion);
            if (mp == 0) {
                flags = cp->fileState->strict ? EJS_LOADER_STRICT : 0;
                if (ejsLoadModule(ejs, np->qname.name, np->useModule.minVersion, np->useModule.maxVersion, flags) < 0) {
                    astError(cp, np, "%s", ejsGetErrorMsg(ejs, 1));
                    cp->fatalError = 1;
                    LEAVE(cp);
                    return;
                }
                mp = ejsLookupModule(ejs, np->qname.name, np->useModule.minVersion, np->useModule.maxVersion);
            }
        }

        if (mp == 0) {
            astError(cp, np, "Can't find required module \"%s\"", np->qname.name);

        } else if (mp != currentModule) {
            if (currentModule->dependencies == 0) {
                currentModule->dependencies = mprCreateList(ejs);
            }
            if (mprLookupItem(currentModule->dependencies, mp) < 0 && 
                    mprAddItem(currentModule->dependencies, mp) < 0) {
                mprAssert(0);
            }
        }
        mprAssert(np->left->kind == N_USE_NAMESPACE);
        np->left->qname.name = mp->vname;
    }
    mprAssert(np->left->kind == N_USE_NAMESPACE);
    processAstNode(cp, np->left);
    LEAVE(cp);
}


static void astWith(EcCompiler *cp, EcNode *np)
{
    Ejs         *ejs;
    EjsLookup   *lookup;
    bool        pushed;

    ENTER(cp);

    ejs = cp->ejs;
    pushed = 0;

    processAstNode(cp, np->with.object);

    if (cp->phase >= EC_PHASE_BIND) {
        processAstNode(cp, np->with.object);
        /*
            To permit early binding, if the object is typed, then add that type to the scope chain.
         */
        lookup = &np->with.object->lookup;
        if (lookup->trait && lookup->trait->type) {
            addScope(cp, (EjsBlock*) lookup->trait->type);
            pushed++;
        }
    }
    processAstNode(cp, np->with.statement);

    if (pushed) {
        removeScope(cp);
    }
    LEAVE(cp);

}


static bool typeIsCompatible(EcCompiler *cp, EjsType *first, EjsType *second)
{
    Ejs     *ejs;

    ejs = cp->ejs;
    if (first == 0 || second == 0) {
        return 1;
    }
    if (first->qname.name == second->qname.name && first->qname.space == second->qname.space) {
        return 1;
    }
    return 0;
}


/*
    Define a variable
 */
static void defineVar(EcCompiler *cp, EcNode *np, int varKind, EjsObj *value)
{
    Ejs             *ejs;
    EjsFunction     *method;
    EjsObj          *obj;
    EcState         *state;
    int             slotNum, attributes;

    ejs = cp->ejs;
    mprAssert(cp->phase == EC_PHASE_DEFINE);

    state = cp->state;
    method = state->currentFunction;

    if (state->inClass) {
        np->attributes |= EJS_TRAIT_FIXED;
        if (!(np->attributes & EJS_PROP_ENUMERABLE) && !(state->currentClassNode->attributes & EJS_PROP_ENUMERABLE)) {
            np->attributes |= EJS_TRAIT_HIDDEN;
        }
        if (ejsContainsMulti(ejs, np->qname.space, ",private")) {
            //  MOB -- TEMP only. Need a reliable flag in attributes for private
            np->attributes |= EJS_TRAIT_HIDDEN;
        }
    }
    attributes = np->attributes;

    /*
        Only create block scope vars if the var block is different to the let block. This converts global let vars to vars.
     */
    np->name.letScope = 0;
    if (varKind & KIND_LET && (state->varBlock != state->optimizedLetBlock)) {
        np->name.letScope = 1;
    }
    if (np->name.letScope) {
        mprAssert(varKind & KIND_LET);
        obj = (EjsObj*) getBlockForDefinition(cp, np, state->optimizedLetBlock, attributes);
        if (ecLookupScope(cp, np->qname) >= 0 && cp->lookup.obj == obj) {
            obj = cp->lookup.obj;
            slotNum = cp->lookup.slotNum;
            if (cp->fileState->strict) {
                astError(cp, np, "Variable \"%s\" is already defined", np->qname.name);
                return;
            }
        } else {
            //  TODO MOB BUG - could / should change context to be obj for the names
            slotNum = ejsDefineProperty(ejs, obj, -1, np->qname, 0, attributes, value);
        }

    } else {
        if (ecLookupVar(cp, (EjsObj*) state->varBlock, np->qname) >= 0) {
            obj = cp->lookup.obj;
            slotNum = cp->lookup.slotNum;
            if (cp->fileState->strict) {
                astError(cp, np, "Variable \"%s\" is already defined.", np->qname.name);
                return;
            }
        }
        /*
            Var declarations are hoisted to the nearest function, class or global block (never nested block scope)
         */
        obj = (EjsObj*) getBlockForDefinition(cp, np, state->varBlock, attributes);
        slotNum = ejsDefineProperty(ejs, obj, -1, np->qname, 0, attributes, value);
    }
    if (slotNum < 0) {
        astError(cp, np, "Can't define variable %s", np->qname.name);
        return;
    }
}


/*
    Hoist a block scoped variable and define in the nearest function, class or global block. This runs during the
    Hoist conditional phase. We hoist the variable by defining with a "-hoisted-%d" namespace which is added to the set of
    Hoist open namespaces. This namespace is only used when compiling and not at runtime. All access to the variable is bound
 */
static bool hoistBlockVar(EcCompiler *cp, EcNode *np)
{
    Ejs         *ejs;
    EcState     *state;
    EjsBlock    *block;
    EjsObj      *obj, *vp;
    int         slotNum, attributes;

    mprAssert(cp->phase == EC_PHASE_CONDITIONAL);

    //  MOB -- all hoisting is currently disabled.
    if (1 || cp->optimizeLevel == 0) {
        return 0;
    }
    ejs = cp->ejs;
    state = cp->state;
    block = (np->kind == N_BLOCK) ? state->letBlock : state->optimizedLetBlock;
    attributes = np->attributes;

    if (state->inClass && state->inFunction) {
        obj = (EjsObj*) state->varBlock;
    } else {
        /*
            Global or class level block
         */
        mprAssert(!state->instanceCode);
        obj = (EjsObj*) state->varBlock;
        attributes |= EJS_PROP_STATIC;
    }

    if (!cp->bind && obj == ejs->global) {
        /* Can't hoist variables to global scope if not binding */
        return 0;
    }

    /*
        Delete the property from the original block. Don't reclaim slot, delete will set to 0.
        This lookup may fail it there were more than one declaration in a block. ie. already hoisted.
     */
    slotNum = ejsLookupProperty(ejs, block, np->qname);
    if (slotNum < 0) {
        mprAssert(state->letBlockNode->namespaceRef);
        np->namespaceRef = state->letBlockNode->namespaceRef;
        np->qname.space = np->namespaceRef->value;
    } else {
        vp = ejsGetProperty(ejs, block, slotNum);
        ejsDeleteProperty(ejs, block, slotNum);

        /*
            Redefine hoisted in the outer var block. Use a unique hoisted namespace to avoid clashes with other
            hoisted variables of the same name. Save the generated namespace for this block and reuse for any
            other hoisted vars from this block.
         */
        if (state->letBlockNode->namespaceRef) {
            np->namespaceRef = state->letBlockNode->namespaceRef;
        } else {
            np->namespaceRef = createHoistNamespace(cp, obj);
            state->letBlockNode->namespaceRef = np->namespaceRef;
        }
        np->qname.space = np->namespaceRef->value;

        slotNum = ejsDefineProperty(ejs, obj, -1, np->qname, 0, attributes, vp);
        if (slotNum < 0) {
            astError(cp, np, "Can't define local variable %s::%s", np->qname.space, np->qname.name);
            return 0;
        }
        if (obj == ejs->global) {
            addGlobalProperty(cp, np, &np->qname);
        }
    }
    np->name.letScope = 0;
    return 1;
}


/*
    Fully bind a variable definition. We already know the owning type and the slot number.
    We now need to  bind the variable type and set the trait reference.
 */
static void bindVariableDefinition(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EjsFunction     *fun;
    EjsBlock        *block;
    EjsTrait        *trait;
    EcState         *state;
    EcNode          *typeNode;
    bool            modified;

    ENTER(cp);

    mprAssert(cp->phase >= EC_PHASE_BIND);

    ejs = cp->ejs;
    state = cp->state;
    fun = state->currentFunction;

    if (np->name.letScope) {
        block = state->optimizedLetBlock;
    } else {
        block = getBlockForDefinition(cp, np, state->varBlock, np->attributes);
    }
    if (/*MOB*/1 || !state->inFunction) {
        if (!np->literalNamespace && resolveNamespace(cp, np, block, &modified) == 0) {
            LEAVE(cp);
            return;
        }
        if (modified && np->left && np->left->left && np->left->left->kind == N_QNAME) {
            /* Fix the namespace of the lhs */
            np->left->left->qname = np->qname;
        }
    }
    if (cp->phase == EC_PHASE_BIND && (EjsObj*) block == ejs->global) {
        addGlobalProperty(cp, np, &np->qname);
    }
    /*
        Check if there is a name clash with any subclasses. Must do after fixup so that the base type has been defined.
        Look in the current type for any public property of the same name.
     */
    if (state->inClass && !state->inFunction && state->currentClass->baseType) {
        if (ecLookupVar(cp, (EjsObj*) state->currentClass->baseType, np->qname) >= 0) {
            astError(cp, np, "Public property %s is already defined in a base class", np->qname.name);
            LEAVE(cp);
            return;
        }
    }
    if (resolveName(cp, np, (EjsObj*) block, np->qname) < 0) {
        astError(cp, np, "Can't find variable \"%s::%s\"", np->qname.space, np->qname.name);
    }
    typeNode = np->typeNode;
    if (typeNode && np->lookup.trait) {
        if (typeNode->lookup.ref) {
            if (!ejsIsType(ejs, typeNode->lookup.ref)) {
                astError(cp, np, "Type is not a type object");
                LEAVE(cp);
                return;
            }
            if (!cp->fileState->strict) {
                /*
                    Allow variable redefinitions providing they are compatible
                 */
                trait = ejsGetPropertyTraits(ejs, np->lookup.obj, np->lookup.slotNum);
                if (!typeIsCompatible(cp, trait->type, (EjsType*) typeNode->lookup.ref)) {
                    astError(cp, np, "Redefinition of \"%s\" is not compatible with prior definition", np->qname.name);
                    LEAVE(cp);
                    return;
                }
            }
#if UNUSED
            ejsSetTraitType(cp->ejs, np->lookup.trait, (EjsType*) typeNode->lookup.ref);
#else
            ejsSetPropertyTraits(cp->ejs, np->lookup.obj, np->lookup.slotNum, (EjsType*) typeNode->lookup.ref, -1);
#endif
        }
    }
    setAstDocString(ejs, np, np->lookup.obj, np->lookup.slotNum);
    np->lookup.bind = 0;
    LEAVE(cp);
}


/*
    Initialize constants here so they can be used for conditional compilation and "use namespace"
 */
static void astInitVar(EcCompiler *cp, EcNode *np)
{
    Ejs         *ejs;
    EcState     *state;
    EcNode      *right;
    int         slotNum;

    ejs = cp->ejs;
    state = cp->state;

    mprAssert(np->left);
    mprAssert(np->left->kind == N_ASSIGN_OP);

    right = np->left->right;
    mprAssert(right);

    if (right->kind == N_LITERAL && !(np->name.varKind & KIND_LET) && !(np->attributes & EJS_PROP_NATIVE)) {
        mprAssert(np->kind == N_VAR);
        mprAssert(right->literal.var);

        /* Exclude class instance variables */
        if (! (state->inClass && !(np->attributes & EJS_PROP_STATIC))) {
            slotNum = ejsLookupProperty(ejs, state->varBlock, np->qname);
            if (cp->phase == EC_PHASE_DEFINE) {
                ejsSetProperty(ejs, state->varBlock, slotNum, right->literal.var);

            } else if (cp->phase >= EC_PHASE_BIND && !np->name.isNamespace && slotNum >= 0) {
                /*
                    Erase the value incase being run in the ejs shell. Must not prematurely define values.
                 */
                ejsSetProperty(ejs, state->varBlock, slotNum, ejs->undefinedValue);
            }
        }
    }
}


static void astVar(EcCompiler *cp, EcNode *np, int varKind, EjsObj *value)
{
    EcState     *state;
    Ejs         *ejs;
    EjsObj      *obj;

    ejs = cp->ejs;
    state = cp->state;

    if (state->disabled) {
        if (cp->phase == EC_PHASE_CONDITIONAL) {
            obj = (EjsObj*) getBlockForDefinition(cp, np, state->varBlock, np->attributes);
            removeProperty(cp, obj, np);
        }
        return;
    }

    state->instanceCode = 0;
    if (state->inClass && !(np->attributes & EJS_PROP_STATIC)) {
        if (state->inMethod) {
            state->instanceCode = 1;
        } else if (state->classState->blockNestCount == (state->blockNestCount - 1)) {
            /*
                Top level var declaration without a static attribute
             */
            state->instanceCode = 1;
        }
    }

    if (np->typeNode) {
        if (np->typeNode->kind != N_QNAME) {
            astError(cp, np, "Bad type name");
            return;
        }
        if (ejsCompareMulti(ejs, np->typeNode->qname.name, "*") != 0) {
            processAstNode(cp, np->typeNode);
        }
    }
    if (cp->phase == EC_PHASE_DEFINE) {
        defineVar(cp, np, varKind, value);

    } else if (cp->phase == EC_PHASE_CONDITIONAL && np->name.letScope) {
        if (!hoistBlockVar(cp, np)) {
            /*
                Unhoisted let scoped variable.
             */
            state->letBlockNode->createBlockObject = 1;
        }

    } else if (cp->phase >= EC_PHASE_BIND) {
        if (np->namespaceRef) {
            /*
                Add any hoist namespaces that were defined in hoistBlockVar in the conditional phase
             */
            ejsAddNamespaceToBlock(ejs, (EjsBlock*) cp->state->optimizedLetBlock, np->namespaceRef);
        }
        bindVariableDefinition(cp, np);
    }
    if (!state->disabled && np->left) {
        astAssignOp(cp, np->left);
        astInitVar(cp, np);
    }
}


static void astVarDefinition(EcCompiler *cp, EcNode *np, int *codeRequired, int *instanceCode)
{
    Ejs         *ejs;
    EcNode      *var;
    EcState     *state;
    int         next;

    mprAssert(np->kind == N_VAR_DEFINITION);

    ENTER(cp);
    ejs = cp->ejs;
    state = cp->state;

    for (next = 0; (var = getNextAstNode(cp, np, &next)); ) {
        if (var->kind == N_VAR) {
            astVar(cp, var, var->name.varKind, var->name.nsvalue);
            if (var->left && !state->disabled) {
                *codeRequired = 1;
                *instanceCode = state->instanceCode;
            }
        } else {
            processAstNode(cp, var);
        }
    }
    LEAVE(cp);
}


/*
    Void type node
 */
static void astVoid(EcCompiler *cp, EcNode *np)
{
    EjsName     qname;
    Ejs         *ejs;

    mprAssert(np->kind == N_VOID);
    ejs = cp->ejs;

    ENTER(cp);

    if (cp->phase >= EC_PHASE_BIND) {
        qname = N(EJS_EJS_NAMESPACE, "Void");
        if (resolveName(cp, np, 0, qname) < 0) {
            astError(cp, np, "Can't find variable \"%s::%s\"", qname.space, qname.name);
        }
    }
    LEAVE(cp);
}


/********************************* Support Code *******************************/
/*
    Create a function to hold the module initialization code. Set a basic scope chain here incase running in ejs.
 */

static EjsFunction *createModuleInitializer(EcCompiler *cp, EcNode *np, EjsModule *mp)
{
    Ejs             *ejs;
    EjsFunction     *fun;

    ejs = cp->ejs;
    
    //  MOB - name shold be EJS_INITIALIZER_NAME
    fun = ejsCreateFunction(ejs, mp->name, 0, -1, 0, 0, 0, ejs->voidType, EJS_FUN_MODULE_INITIALIZER, mp, mp->scope, 
        cp->state->strict);
    if (fun == 0) {
        astError(cp, np, "Can't create initializer function");
        return 0;
    }
    fun->activation = (EjsPot*) ejsCreateCompilerFrame(ejs, fun);
    return fun;
}


/*
    Create the required module
 */
static EjsModule *createModule(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EjsModule       *mp;

    ejs = cp->ejs;

    mprAssert(np->kind == N_MODULE);

    if (np->module.version == 0 && cp->modver && ejsCompareMulti(ejs, np->qname.name, EJS_DEFAULT_MODULE) != 0) {
        np->module.version = cp->modver;
    }
    mp = ecLookupModule(cp, np->qname.name, np->module.version, np->module.version);
    if (mp == 0) {
        mp = ejsCreateModule(cp->ejs, np->qname.name, np->module.version, NULL);
        if (mp == 0) {
            astError(cp, np, "Can't create module %s", np->qname.name);
            return 0;
        }
        mp->scope = (EjsBlock*) ejs->global;
        if (ecAddModule(cp, mp) < 0) {
            astError(cp, 0, "Can't insert module");
            return 0;
        }
        /*
            This will prevent the loading of any module that uses this module.
         */
        if (ejsCompareMulti(ejs, mp->name, EJS_DEFAULT_MODULE) != 0) {
            mp->compiling = 1;
        }
    }
    if (mp->initializer == 0 || mp->initializer->activation) {
        mp->initializer = createModuleInitializer(cp, np, mp);
    }
    np->module.ref = mp;

    if (cp->outputFile) {
        np->module.filename = cp->outputFile;
    } else {
        np->module.filename = mprAsprintf("%@%s", np->qname.name, EJS_MODULE_EXT);
    }
    return mp;
}


static EcNode *getNextAstNode(EcCompiler *cp, EcNode *np, int *next)
{
    if (cp->fatalError) {
        return 0;
    }
    if (np == 0 || np->children == 0) {
        return 0;
    }
    return (EcNode*) mprGetNextItem(np->children, next);
}


static void processAstNode(EcCompiler *cp, EcNode *np)
{
    EcState     *state;
    EjsType     *type;
    int         codeRequired, instanceCode;

    ENTER(cp);
    mprAssert(np->parent || np->kind == N_PROGRAM);

    state = cp->state;
    codeRequired = 0;
    instanceCode = state->instanceCode;
    
    switch (np->kind) {
    case N_ARGS:
        astArgs(cp, np);
        codeRequired++;
        break;

    case N_ASSIGN_OP:
        astAssignOp(cp, np);
        codeRequired++;
        break;

    case N_BINARY_OP:
        astBinaryOp(cp, np);
        codeRequired++;
        break;

    case N_BLOCK:
        astBlock(cp, np);
        break;

    case N_BREAK:
        astBreak(cp, np);
        break;

    case N_CALL:
        astCall(cp, np);
        codeRequired++;
        break;

    case N_CASE_ELEMENTS:
        astCaseElements(cp, np);
        codeRequired++;
        break;

    case N_CASE_LABEL:
        astCaseLabel(cp, np);
        codeRequired++;
        break;

    case N_CATCH:
        astCatch(cp, np);
        codeRequired++;
        break;

    case N_CATCH_ARG:
        codeRequired++;
        break;

    case N_CLASS:
        astClass(cp, np);
        type = np->klass.ref;
        codeRequired++;
        break;

    case N_CONTINUE:
        break;

    case N_DASSIGN:
        astDassign(cp, np);
        break;

    case N_DIRECTIVES:
        astDirectives(cp, np);
        break;

    case N_DO:
        astDo(cp, np);
        codeRequired++;
        break;

    case N_DOT:
        astDot(cp, np);
        codeRequired++;
        break;

    case N_END_FUNCTION:
        break;

    case N_EXPRESSIONS:
        astExpressions(cp, np);
        break;

    case N_FOR:
        astFor(cp, np);
        codeRequired++;
        break;

    case N_FOR_IN:
        astForIn(cp, np);
        codeRequired++;
        break;

    case N_FUNCTION:
        astFunction(cp, np);
        break;

    case N_LITERAL:
        codeRequired++;
        break;

    case N_OBJECT_LITERAL:
        astObjectLiteral(cp, np);
        codeRequired++;
        break;

    case N_FIELD:
        astField(cp, np);
        codeRequired++;
        break;

    case N_QNAME:
        astName(cp, np);
        break;

    case N_NEW:
        astNew(cp, np);
        codeRequired++;
        break;

    case N_NOP:
        break;

    case N_POSTFIX_OP:
        astPostfixOp(cp, np);
        codeRequired++;
        break;

    case N_PRAGMAS:
        astPragmas(cp, np);
        break;

    case N_PRAGMA:
        astPragma(cp, np);
        break;

    case N_PROGRAM:
        astProgram(cp, np);
        break;

    case N_REF:
        codeRequired++;
        break;

    case N_RETURN:
        astReturn(cp, np);
        codeRequired++;
        break;

    case N_SPREAD:
        astSpread(cp, np);
        codeRequired++;
        break;

    case N_SUPER:
        astSuper(cp, np);
        codeRequired++;
        break;

    case N_SWITCH:
        astSwitch(cp, np);
        codeRequired++;
        break;

    case N_HASH:
        astHash(cp, np);
        break;

    case N_IF:
        astIf(cp, np);
        codeRequired++;
        break;

    case N_THIS:
        astThis(cp, np);
        codeRequired++;
        break;

    case N_THROW:
        astThrow(cp, np);
        codeRequired++;
        break;

    case N_TRY:
        astTry(cp, np);
        break;

    case N_UNARY_OP:
        astUnaryOp(cp, np);
        codeRequired++;
        break;

    case N_MODULE:
        astModule(cp, np);
        break;
            
    case N_TYPE_IDENTIFIERS:
        astImplements(cp, np);
        break;

    case N_USE_NAMESPACE:
        astUseNamespace(cp, np);
        break;

    case N_USE_MODULE:
        astRequire(cp, np);
        break;

    case N_VAR_DEFINITION:
        astVarDefinition(cp, np, &codeRequired, &instanceCode);
        break;

    case N_VOID:
        astVoid(cp, np);
        break;

    case N_WITH:
        astWith(cp, np);
        break;

    default:
        mprAssert(0);
        badAst(cp, np);
    }
    
    /*
        Determine if classes need initializers. If class code is generated outside of a method, then some form of
        initialization will be required. Either a class constructor, initializer or a global initializer.
     */
    if (cp->phase == EC_PHASE_DEFINE && codeRequired && !state->inMethod && !state->inHashExpression) {
        if (state->inClass && !state->currentClass->isInterface) {
            if (instanceCode) {
                state->currentClass->hasConstructor = 1;
            } else {
                state->currentClass->hasInitializer = 1;
            }
        } else {
            state->currentModule->hasInitializer = 1;
        }
    }
    mprAssert(state == cp->state);
    LEAVE(cp);
}


static void removeProperty(EcCompiler *cp, EjsObj *obj, EcNode *np)
{
    Ejs             *ejs;
    EjsName         *prop;
    MprList         *globals;
    int             next, slotNum;

    mprAssert(obj);
    ejs = cp->ejs;

    if (np->globalProp) {
        globals = cp->state->currentModule->globalProperties;
        mprAssert(globals);

        for (next = 0; ((prop = (EjsName*) mprGetNextItem(globals, &next)) != 0); ) {
            if (np->qname.space == prop->space && np->qname.name == prop->name) {
                mprRemoveItem(globals, prop);
                break;
            }
        }
    }
    slotNum = ejsLookupProperty(ejs, obj, np->qname);
    if (slotNum >= 0) {
        ejsRemovePotProperty(ejs, obj, slotNum);
    }
}


/*
    Fixup all slot definitions in types. When types are first created, they do not reserve space for inherited slots.
    Now that all types should have been resolved, we can reserve room for inherited slots. Override functions also 
    must be removed.
 */
static void fixupClass(EcCompiler *cp, EjsType *type)
{
    Ejs             *ejs;
    EjsType         *baseType, *iface, *typeType;
    EjsFunction     *fun;
    EjsPot          *prototype;
    EjsObj          *obj;
    EjsName         qname;
    EjsTrait        *trait;
    EcNode          *np, *child;
    int             rc, slotNum, attributes, next, hasInstanceVars;

    if (VISITED(type) || !type->needFixup) {
        return;
    }
    type->needFixup = 0;

    mprAssert(cp);
    mprAssert(type);
    mprAssert(ejsIsType(ejs, type));

    ENTER(cp);

    rc = 0;
    ejs = cp->ejs;
    VISITED(type) = 1;
    np = (EcNode*) type->typeData;
    baseType = type->baseType;

    if (baseType == 0) {
        if (np && np->kind == N_CLASS && !np->klass.isInterface) {
            if (np->klass.extends) {
                qname.space = NULL;
                qname.name = np->klass.extends;
                baseType = (EjsType*) getTypeProperty(cp, ejs->global, qname);
            } else {
                if (! (!ejs->initialized && type->qname.name == ejs->objectType->qname.name)) {
                    baseType = (EjsType*) getTypeProperty(cp, ejs->global, N(EJS_EJS_NAMESPACE, "Object"));
                }
            }
        }
    }
    if (np->klass.implements) {
        type->implements = mprCreateList(type);
        next = 0;
        while ((child = getNextAstNode(cp, np->klass.implements, &next))) {
            iface = (EjsType*) getTypeProperty(cp, ejs->global, child->qname);
            if (iface) {
                mprAddItem(type->implements, iface);
            } else {
                astError(cp, np, "Can't find interface \"%s\"", child->qname.name);
                VISITED(type) = 0;
                LEAVE(cp);
                return;
            }
        }
    }
    if (baseType == 0) {
        if (! (!ejs->initialized && type->qname.name == ejs->objectType->qname.name) && !np->klass.isInterface) {
            astError(cp, np, "Can't find base type for %s", type->qname.name);
            VISITED(type) = 0;
            LEAVE(cp);
            return;
        }
    }
    if (baseType) {
        if (baseType->needFixup) {
            fixupClass(cp, baseType);
        }
        if (baseType->hasConstructor) {
            type->hasBaseConstructors = 1;
        }
        if (baseType->hasInitializer) {
            type->hasBaseInitializers = 1;
        }
    }
    if (type->implements) {
        for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
            if (iface->needFixup) {
                fixupClass(cp, iface);
            }
            if (iface->hasConstructor) {
                type->hasBaseConstructors = 1;
            }
            if (iface->hasInitializer) {
                type->hasBaseInitializers = 1;
            }
        }
    }
    if (!type->constructor.block.pot.isPrototype && (EjsObj*) type != ejs->global && !type->isInterface) {
        /*
            Remove the static initializer slot if this class does not require a static initializer
            By convention, it is installed in slot number 1.
         */
        if (type->hasBaseInitializers) {
            type->hasInitializer = 1;
        }
        if (!type->hasInitializer) {
            ejsRemovePotProperty(ejs, (EjsObj*) type, 0);
        }
        /*
            Disable the constructor if this class does not require it
         */
        if (type->hasBaseConstructors) {
            type->hasConstructor = 1;
        }
        if (!type->hasConstructor) {
            np->klass.constructor = 0;
            ejsDisableFunction(ejs, (EjsFunction*) type);
            mprAssert(!(np->attributes & EJS_TYPE_HAS_CONSTRUCTOR));
        }
    }

    /*
        Determine if instances need to copy the prototype properties
     */
    hasInstanceVars = 0;
    prototype = type->prototype;
    for (slotNum = 0; slotNum < prototype->numProp; slotNum++) {
        obj = ejsGetProperty(ejs, prototype, slotNum);
        if (!ejsIsFunction(ejs, obj) && !ejsIsBlock(ejs, obj)) {
            hasInstanceVars = 1;
            break;
        }
    }
    type->hasInstanceVars |= hasInstanceVars;
    if (baseType) {
        type->hasInstanceVars |= baseType->hasInstanceVars;
    }
    ejsFixupType(ejs, type, baseType, 1);
    
    if (ejs->empty) {
        typeType = (EjsType*) getTypeProperty(cp, ejs->global, N(EJS_EJS_NAMESPACE, "Type"));
    } else {
        typeType = ejs->typeType;
    }
    if (typeType == 0) {
        astError(cp, 0, "Can't find Type class");
    }
    if (typeType->needFixup) {
        fixupClass(cp, typeType);
    }
#if UNUSED
    if (type != typeType) {
        ejsBlendTypeProperties(ejs, type, typeType);
    }
#endif

    /*
        Remove the original overridden method. Set the inherited slot to the overridden method. This implements a v-table.
     */
    prototype = type->prototype;
    for (slotNum = type->numInherited; slotNum < prototype->numProp; slotNum++) {
        trait = ejsGetPropertyTraits(ejs, prototype, slotNum);
        if (trait == 0) {
            continue;
        }
        attributes = trait->attributes;
        if (attributes & EJS_FUN_OVERRIDE && type->numInherited > 0) {
            /*
                If the type is not an orphan, it must preserve the slot order dictated by the base class
             */
            fun = ejsGetProperty(ejs, prototype, slotNum);
            mprAssert(fun && ejsIsFunction(ejs, fun));
            qname = ejsGetPropertyName(ejs, prototype, slotNum);
            ejsRemovePotProperty(ejs, prototype, slotNum);
            slotNum--;
            if (resolveName(cp, NULL, (EjsObj*) type, qname) < 0 || cp->lookup.slotNum < 0) {
                astError(cp, 0, "Can't find method \"%s::%s\" in base type of \"%s\" to override", qname.space, qname.name, 
                    type->qname.name);
            } else {
                ejsSetProperty(ejs, prototype, cp->lookup.slotNum, (EjsObj*) fun);
                ejsSetPropertyTraits(cp->ejs, prototype, cp->lookup.slotNum, NULL, attributes);
            }
        }
    }
    VISITED(type) = 0;
    LEAVE(cp);
}


/*
    Lookup the namespace for a definition (np->qname.space).  We look for the namespace variable declaration if it is a user
    defined namespace. Otherwise, we trust that if the set of open namespaces has the namespace -- it must exist.
 */
static EjsNamespace *resolveNamespace(EcCompiler *cp, EcNode *np, EjsBlock *block, bool *modified)
{
    Ejs             *ejs;
    EjsName         qname;
    EjsNamespace    *nspace;
    int             slotNum;

    ejs = cp->ejs;

    if (modified) {
        *modified = 0;
    }
    qname.name = np->qname.space;
    qname.space = 0;
    nspace = (EjsNamespace*) getTypeProperty(cp, 0, qname);
    if (nspace == 0 || !ejsIsNamespace(ejs, nspace)) {
        nspace = lookupNamespace(cp->ejs, np->qname.space);
    }
    if (nspace == 0 && cp->state->nspace == np->qname.space) {
        nspace = ejsCreateNamespace(ejs, np->qname.space);
    }
    if (nspace == 0) {
        if (!np->literalNamespace) {
            astError(cp, np, "Can't find namespace \"%@\"", qname.name);
        }
    } else {
        if (nspace->value != np->qname.space) {
            slotNum = ejsLookupProperty(ejs, block, np->qname);
            mprAssert(slotNum >= 0);
            if (slotNum >= 0) {
                mprFree((char*) np->qname.space);
                /*
                    Change the name to use the namespace URI. This will change the property name and set
                    "modified" so that the caller can modify the derrived names (type->qname)
                 */
                np->qname.space = nspace->value;
                ejsSetPropertyName(ejs, block, slotNum, np->qname);
                if (modified) {
                    *modified = 1;
                }
            }
        }
    }
    //  MOB - nobody uses this except as a return code
    return nspace;
}


/*
    Locate a property in context. NOTE this only works for type properties not instance properties.
 */
static EjsObj *getTypeProperty(EcCompiler *cp, EjsObj *vp, EjsName name)
{
    EcNode      node;

    mprAssert(cp);

    if (resolveName(cp, &node, vp, name) < 0) {
        return 0;
    }
    return node.lookup.ref;
}


static int resolveProperty(EcCompiler *cp, EcNode *np, EjsType *type, EjsName name)
{
    EcNode      node;

    mprAssert(cp);

    if (resolveName(cp, &node, (EjsObj*) type->prototype, name) < 0) {
        if (resolveName(cp, &node, (EjsObj*) type, name) < 0) {
            return -1;
        }
    }
    //  MOB -- should return slotNum
    return 0;
}


/*
    Locate a property via lookup and determine the best way to address the property.
 */
static int resolveName(EcCompiler *cp, EcNode *np, EjsObj *vp, EjsName qname)
{
    Ejs         *ejs;
    EjsLookup   *lookup;
    EjsType     *type, *currentClass, *tp;
    EcState     *state;
    EjsBlock    *block;

    ejs = cp->ejs;
    state = cp->state;
    lookup = &cp->lookup;

    cp->lookup.bind = 0;

    if (vp) {
        //  MOB -- was anyspace
        if (ecLookupVar(cp, vp, qname) < 0) {
            return EJS_ERR;
        }

    } else {
        if (ecLookupScope(cp, qname) < 0) {
            return EJS_ERR;
        }
    }
    cp->lookup.bind = 1;

    /*
        Revise the nth block to account for blocks that will be erased
     */
    lookup->nthBlock = 0;
    for (block = ejs->state->bp->scope; block; block = block->scope) {
        if ((EjsObj*) block == lookup->obj) {
            break;
        }
        if (ejsIsType(ejs, block)) {
            type = (EjsType*) block;
            if ((EjsObj*) type->prototype == lookup->obj) {
                break;
            }
        }
        lookup->nthBlock++;
    }
    if (block == 0) {
        lookup->nthBlock = 0;
    }
    mprAssert(lookup->ref);    
    if (lookup->ref == ejs->nullValue) {
        lookup->ref = 0;
    }

    if ((ejsIsType(ejs, lookup->obj) || ejsIsPrototype(ejs, lookup->obj)) && state->currentObjectNode == 0) {
        mprAssert(lookup->obj != ejs->global);
        //  NOTE: could potentially do this for static properties as well
        if (lookup->trait && lookup->slotNum >= 0) {
            /*
                class instance or method properties
             */
            type = (EjsType*) lookup->obj;
            currentClass = state->currentClass;
            if (currentClass) {
                mprAssert(state->inClass);
                for (tp = currentClass; tp; tp = tp->baseType) {
                    if ((EjsObj*) tp == lookup->obj || (EjsObj*) tp->prototype == lookup->obj) {
                        /*
                            Method code or class level instance initialization code. This is code that is a subtype of the 
                            type owning the property, so we can use the thisObj to access it.
                         */
                        if (state->inClass) {
                            lookup->useThis = 1;
                        }
                    }
                }
            }
        }
    }

    if (np) {
        np->lookup = cp->lookup;
    }
    //  MOB -- should return slotNum
    return 0;
}


/*
    Wrap the define property routine. Need to keep a module to property mapping
 */
static void addGlobalProperty(EcCompiler *cp, EcNode *np, EjsName *qname)
{
    Ejs             *ejs;
    EjsModule       *up;
    EjsName         *p;
    int             next;

    ejs = cp->ejs;

    up = cp->state->currentModule;
    mprAssert(up);

    if (up->globalProperties == 0) {
        up->globalProperties = mprCreateList(up);
    }
    //  TODO OPT - should this be a hash?
    for (next = 0; (p = (EjsName*) mprGetNextItem(up->globalProperties, &next)) != 0; ) {
        if (p->name == qname->name && p->space == qname->space) {
            return;
        }
    }
    next = mprAddItem(up->globalProperties, qname);
    if (np) {
        np->globalProp = qname;
    }
}


static void setAstDocString(Ejs *ejs, EcNode *np, void *vp, int slotNum)
{
    mprAssert(vp);
    mprAssert(slotNum >= 0);

    if (np->doc && vp && slotNum >= 0) {
        ejsCreateDoc(ejs, vp, slotNum, np->doc);
    }
}


static void addScope(EcCompiler *cp, EjsBlock *block)
{
    mprAssert(block != cp->ejs->state->bp);

    //  MOB -- why is there a "Compiler" object on the top at all times?
    block->scope = cp->ejs->state->bp->scope;
    cp->ejs->state->bp->scope = block;
    mprAssert(block != block->scope);
}


static void removeScope(EcCompiler *cp)
{
    EjsBlock    *block;

    block = cp->ejs->state->bp;
    mprAssert(block != block->scope);
    block->scope = block->scope->scope;
}


/*
    Create a new lexical block scope and open it
 */
static void openBlock(EcCompiler *cp, EcNode *np, EjsBlock *block)
{
    Ejs             *ejs;
    EcState         *state;
    EjsNamespace    *namespace;
    char            *debugName;
    int             next;

    ejs = cp->ejs;
    state = cp->state;

    if (cp->phase == EC_PHASE_DEFINE) {
        if (block == 0) {
            static int index = 0;
            if (np->loc.filename == 0) {
                debugName = mprAsprintf("block_%04d", index++);
            } else {
                debugName = mprAsprintf("block_%04d_%d", np->loc.lineNumber, index++);
            }
            block = ejsCreateBlock(cp->ejs, 0);
            np->qname = N(EJS_BLOCK_NAMESPACE, debugName);
        }
        np->blockRef = block;

    } else {
        /*
            Must reset the namespaces each phase. This is because pragmas must apply from the point of use in a block onward
            only. Except for hoisted variable namespaces which must apply from the start of the block. They are applied below
         */
        if (block == 0) {
            block = np->blockRef;
        }
        if (!ejsIsType(ejs, block) && block != ejs->global) {
            ejsResetBlockNamespaces(ejs, block);
        }
    }
    state->namespaceCount = ejsGetNamespaceCount(block);

    /*
        Special case for the outermost module block. The module (file) block is created to provide a compilation unit
        level scope. However, we do not use the block for the let or var scope, rather we use the global scope.
        Namespaces always use this new block.
     */
    if (! (state->letBlock == ejs->global && np->parent->kind == N_MODULE)) {
        state->optimizedLetBlock = block;
    }
    state->letBlock = block;
    state->letBlockNode = np;

    /*
        Add namespaces that must apply from the start of the block. Current users: hoisted let vars.
     */
    //  MOB -- this seems to always be null
    mprAssert(np->namespaces == NULL);
    if (np->namespaces) {
        for (next = 0; (namespace = (EjsNamespace*) mprGetNextItem(np->namespaces, &next)) != 0; ) {
            ejsAddNamespaceToBlock(ejs, block, namespace);
        }
    }
    /*
        Mark the state corresponding to the last opened block
     */
    state->prevBlockState = cp->blockState;
    cp->blockState = state;
    addScope(cp, block);
}


static void closeBlock(EcCompiler *cp)
{
    EjsBlock    *block;
    EcState     *state;
    
    state = cp->state;
    
    block = cp->ejs->state->bp->scope;
    ejsPopBlockNamespaces(block, state->namespaceCount);
    cp->blockState = state->prevBlockState;
    removeScope(cp);
}


static EjsNamespace *createHoistNamespace(EcCompiler *cp, EjsObj *obj)
{
    EjsNamespace    *namespace;
    Ejs             *ejs;
    EcNode          *letBlockNode;
    char            *spaceName;

    ejs = cp->ejs;
    spaceName = mprAsprintf("-hoisted-%d", ejsGetPropertyCount(ejs, obj));
    namespace = ejsCreateNamespace(ejs, ejsCreateStringFromAsc(ejs, spaceName));

    letBlockNode = cp->state->letBlockNode;
    if (letBlockNode->namespaces == 0) {
        letBlockNode->namespaces = mprCreateList(letBlockNode);
    }
    mprAddItem(letBlockNode->namespaces, namespace);
    ejsAddNamespaceToBlock(ejs, (EjsBlock*) cp->state->optimizedLetBlock, namespace);
    return namespace;
}


/*
    Determine the block in which to define a variable.
 */
static EjsBlock *getBlockForDefinition(EcCompiler *cp, EcNode *np, EjsBlock *block, int attributes)
{
    EcState     *state;
    EjsType     *type;

    state = cp->state;

    if (ejsIsType(cp->ejs, block) && state->inClass) {
        if (!(attributes & EJS_PROP_STATIC) && !state->inFunction &&
                state->blockNestCount <= (state->classState->blockNestCount + 1)) {
            /*
                Use the prototype object if not static, outside a function and in the top level block.
             */
            type = (EjsType*) block;
            if (!(np->kind == N_FUNCTION && np->function.isConstructor)) {
                block = (EjsBlock*) type->prototype;
            }
            if (np->kind == N_QNAME || np->kind == N_VAR) {
                np->name.instanceVar = 1;
            }
        }
    }
    return block;
}


static EjsNamespace *lookupNamespace(Ejs *ejs, EjsString *nspace)
{
    MprList         *namespaces;
    EjsNamespace    *nsp;
    EjsBlock        *block;
    int             nextNamespace;

    /*
        Lookup the scope chain considering each block and the open namespaces at that block scope.
     */
    for (block = ejs->state->bp; block; block = block->scope) {
        if (!ejsIsBlock(ejs, block)) {
            continue;
        }
        namespaces = &block->namespaces;
        for (nextNamespace = -1; (nsp = (EjsNamespace*) mprGetPrevItem(namespaces, &nextNamespace)) != 0; ) {
            if (nsp->value == nspace) {
                return nsp;
            }
        }
    }
    return 0;
}


/*
    Look for a variable by name in the scope chain and return the location in "cp->lookup" and a positive slot 
    number if found.  If the name.space is non-null/non-empty, then only the given namespace will be used. 
    otherwise the set of open namespaces will be used. The lookup structure will contain details about the location 
    of the variable.
 */
int ecLookupScope(EcCompiler *cp, EjsName name)
{
    Ejs             *ejs;
    EjsFunction     *fun;
    EjsBlock        *bp;
    EjsState        *state;
    EjsType         *type;
    EjsPot          *prototype;
    EjsLookup       *lookup;
    int             slotNum, nthBase;

    mprAssert(cp);
    mprAssert(name.name);

    ejs = cp->ejs;
    if (name.space == NULL) {
        name.space = ejs->emptyString;
    }
    lookup = &cp->lookup;
    state = ejs->state;
    slotNum = -1;
    
    memset(lookup, 0, sizeof(*lookup));

    //  MOB -- remove nthBlock. Not needed if not binding
    //  MOB -- should start one in to step over Compiler block
    for (lookup->nthBlock = 0, bp = state->bp; bp; bp = bp->scope, lookup->nthBlock++) {
        /* Seach simple object */
        if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) bp, name, lookup)) >= 0) {
            return slotNum;
        }
        if (ejsIsFrame(ejs, bp)) {
            fun = (EjsFunction*) bp;
            if (cp->state->inMethod && !fun->staticMethod && !fun->isInitializer) {
                /* Instance method only */
                /* Search prototype chain */
                for (nthBase = 1, type = cp->state->currentClass; type; type = type->baseType, nthBase++) {
                    if ((prototype = type->prototype) == 0 || prototype->shortScope) {
                        break;
                    }
                    if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) prototype, name, lookup)) >= 0) {
                        lookup->nthBase = nthBase;
                        return slotNum;
                    }
                }
            }
        } else if (ejsIsType(ejs, bp)) {
            if (cp->state->inClass && !cp->state->inFunction) {
                /* MOBXX Instance level initialization code. Should really be inside a constructor */
                for (nthBase = 1, type = cp->state->currentClass; type; type = type->baseType, nthBase++) {
                    if ((prototype = type->prototype) == 0 || prototype->shortScope) {
                        break;
                    }
                    if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) prototype, name, lookup)) >= 0) {
                        lookup->nthBase = nthBase;
                        return slotNum;
                    }
                }
            }
            //  MOB -- remove nthBase. Not needed if not binding.
            /* Search base class chain */
            for (nthBase = 1, type = (EjsType*) bp; type; type = type->baseType, nthBase++) {
                if (type->constructor.block.pot.shortScope) {
                    break;
                }
                if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) type, name, lookup)) >= 0) {
                    lookup->nthBase = nthBase;
                    return slotNum;
                }
            }
        }
    }
    return -1;
}


int ecLookupVar(EcCompiler *cp, EjsObj *obj, EjsName name)
{
    Ejs         *ejs;
    EjsLookup   *lookup;
    EjsType     *type;
    EjsPot      *prototype;
    int         slotNum, nthBase;

    mprAssert(obj);
    
    ejs = cp->ejs;
    lookup = &cp->lookup;
    if (name.space == NULL) {
        name.space = ejs->emptyString;
    }
    memset(lookup, 0, sizeof(*lookup));

    /* Lookup simple object */
    if ((slotNum = ejsLookupVarWithNamespaces(ejs, obj, name, lookup)) >= 0) {
        return slotNum;
    }
    /* Lookup prototype chain */
    type = ejsIsType(ejs, obj) ? ((EjsType*) obj) : TYPE(obj);
    for (nthBase = 1; type; type = type->baseType, nthBase++) {
        if ((prototype = type->prototype) == 0 || prototype->shortScope) {
            break;
        }
        if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) prototype, name, lookup)) >= 0) {
            lookup->nthBase = nthBase;
            return slotNum;
        }
    }
    /* Lookup base-class chain */
    type = ejsIsType(ejs, obj) ? ((EjsType*) obj)->baseType : TYPE(obj);
    for (nthBase = 1; type; type = type->baseType, nthBase++) {
        if (type->constructor.block.pot.shortScope) {
            //  MOB -- continue or break?
            continue;
        }
        if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) type, name, lookup)) >= 0) {
            lookup->nthBase = nthBase;
            return slotNum;
        }
    }
    return -1;
}


static void astError(EcCompiler *cp, EcNode *np, cchar *fmt, ...)
{
    EcLocation  *loc;
    va_list     args;

    va_start(args, fmt);
    cp->errorCount++;
    cp->error = 1;
    cp->noout = 1;
    
    if (np) {
        loc = &np->loc;
        ecErrorv(cp, "Error", loc, fmt, args);
    } else {
        ecError(cp, "Error", NULL, fmt, args);
    }
    va_end(args);
}


static void astWarn(EcCompiler *cp, EcNode *np, cchar *fmt, ...)
{
    va_list     args;
    EcLocation  *loc;

    va_start(args, fmt);

    cp->warningCount++;
    loc = &np->loc;
    ecError(cp, "Warning", loc, fmt, args);
    va_end(args);
}


static void badAst(EcCompiler *cp, EcNode *np)
{
    cp->fatalError = 1;
    cp->errorCount++;
    mprError("Unsupported language feature\nUnknown AST node kind %d",  np->kind);
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
