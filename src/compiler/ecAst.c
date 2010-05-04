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
static void     allocName(Ejs *ejs, EjsName *qname);
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
static void     astError(EcCompiler *cp, EcNode *np, char *fmt, ...);
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
static void     astWarn(EcCompiler *cp, EcNode *np, char *fmt, ...);
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
static EjsObj   *getBlockForDefinition(EcCompiler *cp, EcNode *np, EjsObj *block, int attributes);
static EcNode   *getNextAstNode(EcCompiler *cp, EcNode *np, int *next);
static EjsObj   *getTypeProperty(EcCompiler *cp, EjsObj *vp, EjsName *name);
static bool     hoistBlockVar(EcCompiler *cp, EcNode *np);
static void     openBlock(EcCompiler *cp, EcNode *np, EjsBlock *block);
static void     processAstNode(EcCompiler *cp, EcNode *np);
static void     removeProperty(EcCompiler *cp, EjsObj *block, EcNode *np);
static EjsNamespace *resolveNamespace(EcCompiler *cp, EcNode *np, EjsObj *block, bool *modified);
static void     removeScope(EcCompiler *cp);
static int      resolveName(EcCompiler *cp, EcNode *node, EjsObj *vp,  EjsName *name);
static int      resolveProperty(EcCompiler *cp, EcNode *node, EjsType *type, EjsName *name);
static void     setAstDocString(Ejs *ejs, EcNode *np, EjsObj *block616G, int slotNum);
static EjsNamespace *ejsLookupNamespace(Ejs *ejs, cchar *namespace);

#if UNUSED
static int      ecLookupVarWithNamespaces(Ejs *ejs, EjsObj *originalObj, EjsObj *vp, EjsName *name, EjsLookup *lookup);
#endif

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


int ecAstProcess(EcCompiler *cp, int argc, EcNode **nodes)
{
    Ejs         *ejs;
    EcNode      *np;
    int         phase, i;

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
        for (i = 0; i < argc && !cp->fatalError; i++) {
            /*
                Looping through the input source files. A single top level node describes the source file.
             */
            np = nodes[i];
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


/*
    Generate an assignment expression
 */
static void astAssignOp(EcCompiler *cp, EcNode *np)
{
    EcState     *state;
    EjsFunction *fun;
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
        fun = state->currentFunction;
        openBlock(cp, state->currentFunctionNode->function.body, (EjsBlock*) fun);
#if CHANGE && MOB
        ejsDefineReservedNamespace(cp->ejs, (EjsBlock*) fun, 0, EJS_PRIVATE_NAMESPACE);
#endif
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
    EjsBlock    *block;
    EjsObj      *letBlock;
    int         slotNum;

    ejs = cp->ejs;
    state = cp->state;
    letBlock = state->letBlock;

    mprAssert(cp->phase == EC_PHASE_CONDITIONAL);
    mprAssert(np->kind == N_BLOCK || np->kind == N_MODULE);

    block = np->blockRef;

    if (np->createBlockObject) {
        allocName(ejs, &np->qname);
        slotNum = ejsDefineProperty(ejs, letBlock, -1, &np->qname, block->obj.type, 0, (EjsObj*) block);
        if (slotNum < 0) {
            astError(cp, np, "Can't define block");

        } else {
            np->blockCreated = 1;
            if (letBlock == ejs->global) {
                addGlobalProperty(cp, np, &np->qname);
            }
        }
    } else {
        block->obj.hidden = 1;
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

    rc = resolveName(cp, np, NULL, &np->qname);

    if (np->blockCreated) {
        if (! np->createBlockObject) {
            mprAssert(cp->lookup.obj);
            mprAssert(np->lookup.slotNum >= 0);
            ejsDeleteProperty(ejs, np->lookup.obj, np->lookup.slotNum);
            np->blockCreated = 0;
            np->lookup.ref->hidden = 1;

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
    int         next;

    ENTER(cp);
    
    if (cp->phase == EC_PHASE_BIND) {
        /*
            Bind the block here before processing the child nodes so we can mark the block as hidden if it will be expunged.
         */
        bindBlock(cp, np);
    }

    /*
        Open block will change state->letBlock which we need preserved in defineBlock. Use ENTER/LEAVE to save and restore.
     */
    ENTER(cp);
    openBlock(cp, np, NULL);

    next = 0;
    while ((child = getNextAstNode(cp, np, &next))) {
        processAstNode(cp, child);
    }

    closeBlock(cp);
    LEAVE(cp);

    if (cp->phase == EC_PHASE_CONDITIONAL) {
        /*
            Do define block after the variables have been processed. This allows us to determine if the block is 
            really needed.
         */
        defineBlock(cp, np);

        /*
            Try to hoist the block object itself
         */
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

    ENTER(cp);

    block = ejsCreateBlock(cp->ejs, 0);
    ejsSetDebugName(block, "catch");
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
    EcNode          *constructor;
    EjsName         qname;
    EjsNamespace    *nsp;
    int             fatt, attributes, slotNum;
    
    mprAssert(np->kind == N_CLASS);

    ejs = cp->ejs;
    state = cp->state;
    type = np->klass.ref;
    
    if ((slotNum = ecLookupVar(cp, ejs->global, &np->qname)) >= 0) {
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

    /*
        Create the class type object
     */
    allocName(ejs, &np->qname);
    type = ejsCreateType(ejs, &np->qname, state->currentModule, NULL, sizeof(EjsObj), slotNum, 0, 0, attributes, np);
    if (type == 0) {
        astError(cp, np, "Can't create type %s", type->qname.name);
        return 0;
    }
    np->klass.ref = type;

    nsp = ejsDefineReservedNamespace(ejs, (EjsBlock*) type, &type->qname, EJS_PROTECTED_NAMESPACE);
    nsp->flags |= EJS_NSP_PROTECTED;

    nsp = ejsDefineReservedNamespace(ejs, (EjsBlock*) type, &type->qname, EJS_PRIVATE_NAMESPACE);
    nsp->flags |= EJS_NSP_PRIVATE;

    /*
        Define a property for the type in global
     */
    allocName(ejs, &np->qname);
    slotNum = ejsDefineProperty(ejs, ejs->global, slotNum, &np->qname, ejs->typeType, attributes, (EjsObj*) type);
    if (slotNum < 0) {
        astError(cp, np, "Can't install type %s",  np->qname.name);
        return 0;
    }

    /*
        Reserve two slots for the constructor and static initializer to ensure they are the first two non-inherited slots.
        These slots may be reclaimed during fixup if not required. Instance initializers are prepended to the constructor.
        Set a dummy name for the constructor as it will be defined by calling defineFunction from astClass.
     */
    if (!type->isInterface) {
        fatt = EJS_TRAIT_HIDDEN;
#if UNUSED
        if (type->prototype == 0) {
            type->prototype = (EjsObj*) ejsCreatePrototype(cp->ejs, type, 0);
        }
#endif
        ejsDefineProperty(ejs, type->prototype, 0, ejsName(&qname, "", ""), ejs->functionType, fatt, ejs->nullValue);

        qname.name = type->qname.name;
        qname.space = EJS_INIT_NAMESPACE;
        fatt = EJS_TRAIT_HIDDEN | EJS_PROP_STATIC;
        ejsDefineProperty(ejs, (EjsObj*) type, 0, &qname, ejs->functionType, fatt, ejs->nullValue);

        constructor = np->klass.constructor;
        if (constructor && !constructor->function.isDefaultConstructor) {
            type->hasConstructor = 1;
        }
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
            fun = (EjsFunction*) ejsGetProperty(ejs, (EjsObj*) iface, i);
            if (!ejsIsFunction(fun) || fun->isInitializer) {
                continue;
            }
            qname = ejsGetPropertyName(ejs, (EjsObj*) iface, i);
            vp = ejsGetPropertyByName(ejs, (EjsObj*) type, &qname);
            if (vp == 0 || !ejsIsFunction(vp)) {
                astError(cp, np, "Missing method \"%s\" required by interface \"%s\"", qname.name, iface->qname.name);
            } else {
                validateFunction(cp, np, fun, (EjsFunction*) vp);
            }
        }
    }
    if (type->implements) {
        if (mprGetListCount(type->implements) > 1 || (type->baseType && strcmp(type->baseType->qname.name, "Object") != 0)) {
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

    if (type->hasStaticInitializer) {
        /*
            Create the static initializer function. Code gen will fill out the code. The type must be on the scope chain.
         */
        mp = state->currentModule;
        fun = ejsCreateFunction(ejs, np->qname.name, NULL, -1, 0, 0, 0, cp->ejs->voidType, EJS_FUN_INITIALIZER, 
            mp->constants, NULL, cp->fileState->strict);
        np->klass.initializer = fun;
        //  MOB -- better to use DefineProperty and set traits for initializer
        ejsSetProperty(ejs, (EjsObj*) type, 0, (EjsObj*) fun);
#if UNUSED
        ejsSetFunctionLocation(fun, (EjsObj*) type, 0);
#endif
    }

    modified = 0;
    if (!np->literalNamespace && resolveNamespace(cp, np, ejs->global, &modified) == 0) {
        return;
    }
    if (modified) {
        ejsSetTypeName(ejs, type, &np->qname);
    }
    addGlobalProperty(cp, np, &type->qname);

    if (np->klass.constructor == 0 && type->hasBaseConstructors) {
        mprAssert(type->hasConstructor == 1);
        type->hasConstructor = 1;
    }
    if (resolveName(cp, np, ejs->global, &type->qname) < 0) {
        return;
    }
    /*
        Now that all the properties are defined, make the entire class permanent.
     */
    if (!type->block.obj.dynamic) {
        ejsMakePermanent(ejs, (EjsObj*) type);
        if (type->prototype) {
            ejsMakePermanent(ejs, (EjsObj*) type->prototype);
        }
    }
     setAstDocString(ejs, np, np->lookup.obj, np->lookup.slotNum);
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
    bool            hasStaticInitializer;

    mprAssert(np->kind == N_CLASS);
    
    ENTER(cp);

    ejs = cp->ejs;
    state = cp->state;
    cp->classState = state;
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
    state->currentClassName = type->qname;
    state->inClass = 1;
    state->inFunction = 0;

    /*
        Add the type to the scope chain and the static initializer if present. Use push frame to make it eaiser to
        pop the type off the scope chain later.
     */
    hasStaticInitializer = 0;
    addScope(cp, (EjsBlock*) type);
    if (np->klass.initializer) {
        openBlock(cp, np, (EjsBlock*) np->klass.initializer);
        hasStaticInitializer++;
    }
    if (cp->phase == EC_PHASE_FIXUP && type->baseType) {
        ejsInheritBaseClassNamespaces(ejs, type, type->baseType);
    }
    state->optimizedLetBlock = (EjsObj*) type;
    state->letBlock = (EjsObj*) type;
    state->varBlock = (EjsObj*) type;

    /*
        Process the class body
     */
    mprAssert(np->left->kind == N_DIRECTIVES);
    processAstNode(cp, np->left);

    /*
        Only need to do this if this is a default constructor, ie. does not exist in the class body.
     */
    constructor = np->klass.constructor;
    if (constructor && constructor->function.isDefaultConstructor) {
        astFunction(cp, constructor);
    }
    if (hasStaticInitializer) {
        closeBlock(cp);
    }
    removeScope(cp);
    LEAVE(cp);
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
    EjsObj          *block;
    int             numArgs;
    int             slotNum, attributes;

    mprAssert(np->kind == N_FUNCTION);
    mprAssert(cp->phase == EC_PHASE_DEFINE);

    ejs = cp->ejs;
    state = cp->state;

    if (np->function.isMethod) {
        np->attributes |= EJS_TRAIT_FIXED | EJS_TRAIT_READONLY;
        if (!(np->attributes & EJS_PROP_ENUMERABLE) && !(state->currentClassNode->attributes & EJS_PROP_ENUMERABLE)) {
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
    numArgs = (parameters) ? mprGetListCount(parameters->children) : 0;
    
    if (np->function.resultType) {
        np->attributes |= np->function.resultType->attributes;
    }
    /*
        Create a function object. Don't have code yet so we create without it. Can't resolve the return type yet, so we 
        leave it unset. The numDefault and numExceptions will be fixed when the function is bound.
     */
    fun = ejsCreateFunction(ejs, np->qname.name, NULL, 0, numArgs, 0, 0, NULL, np->attributes, 
        state->currentModule->constants, NULL, cp->fileState->strict);
    if (fun == 0) {
        astError(cp, np, "Can't create function \"%s\"", np->qname.name);
        return 0;
    }    
    np->function.functionVar = fun;

    /*
        Check if this function has already been defined in this block. Can't check base classes yes. Must wait till 
        bindFunction()
     */
    slotNum = ejsLookupProperty(ejs, block, &np->qname);

    if (slotNum >= 0 && cp->fileState->strict) {
        if ((np->function.setter && ejsHasTrait(block, slotNum, EJS_TRAIT_SETTER)) ||
            (np->function.getter && ejsHasTrait(block, slotNum, EJS_TRAIT_GETTER))) {
            astError(cp, np, "Property \"%s\" is already defined.", np->qname);
            return 0;
        }
    }
    attributes = np->attributes;
    if (np->function.isConstructor) {
        slotNum = 0;
        attributes |= EJS_FUN_CONSTRUCTOR;
        np->qname.space = EJS_CONSTRUCTOR_NAMESPACE;
    }
    allocName(ejs, &np->qname);
    slotNum = ejsDefineProperty(ejs, block, slotNum, &np->qname, fun->block.obj.type, attributes, (EjsObj*) fun);
    if (slotNum < 0) {
        astError(cp, np, "Can't define function in type \"%s\"", state->currentClass->qname.name);
        return 0;
    }
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
    EjsName         qname;
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

#if CHANGE && MOB
        ejsName(&qname, EJS_PRIVATE_NAMESPACE, nameNode->qname.name);
#else
        ejsName(&qname, EJS_EMPTY_NAMESPACE, nameNode->qname.name);
#endif
        allocName(ejs, &qname);
        slotNum = ejsDefineProperty(ejs, (EjsObj*) fun, slotNum, &qname, NULL, attributes, NULL);
        mprAssert(slotNum >= 0);

        /*
            We can assign the lookup information here as these never need fixups.
         */
        nameNode->lookup.slotNum = slotNum;
        nameNode->lookup.obj = (EjsObj*) fun;
        nameNode->lookup.trait = ejsGetPropertyTrait(ejs, (EjsObj*) fun, slotNum);
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
    EjsName         qname;
    int             next, slotNum, attributes;

    ejs = cp->ejs;
    fun = np->function.functionVar;
    next = 0;
    parameters = np->function.parameters;

    if (parameters) {
        while ((child = getNextAstNode(cp, parameters, &next))) {
            mprAssert(child->kind == N_VAR_DEFINITION);
            trait = ejsGetPropertyTrait(ejs, (EjsObj*) fun, next - 1);
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

                    //  MOB WARNING - this is for compiler/functions/function-05.tst fun(a = a)

                    openBlock(cp, np->function.body, (EjsBlock*) fun);
#if CHANGE && MOB
                    ejsDefineReservedNamespace(ejs, (EjsBlock*) fun, 0, EJS_PRIVATE_NAMESPACE);
#endif
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
                    ejsName(&qname, EJS_EJS_NAMESPACE, "Array");
                    slotNum = ejsLookupProperty(ejs, ejs->global, &qname);
                    mprAssert(slotNum >= 0);
                    ejsSetTraitType(trait, (EjsType*) ejsGetProperty(ejs, ejs->global, slotNum));
                    fun->rest = 1;
                }

            } else {
                localType = varNode->typeNode;
                processAstNode(cp, localType);
                if (localType->lookup.ref) {
                    ejsSetTraitType(trait, (EjsType*) localType->lookup.ref);
                    attributes |= (localType->attributes & (EJS_TRAIT_CAST_NULLS | EJS_TRAIT_THROW_NULLS));
                    ejsSetTraitAttributes(trait, attributes);
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
    EjsType         *iface;
    EjsFunction     *fun;
    EjsObj          *block;
    int             slotNum, next;

    mprAssert(cp->phase >= EC_PHASE_BIND);
    mprAssert(np->kind == N_FUNCTION);
    mprAssert(np->qname.name);

    state = cp->state;
    ejs = cp->ejs;
    fun = np->function.functionVar;
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
            if (resolveNamespace(cp, np, block, 0) == 0) {
                return 0;
            }
        }
        if (block == ejs->global) {
            addGlobalProperty(cp, np, &np->qname);
        }
    }
    
    /*
        Test for clashes with non-overridden methods in base classes.
     */
    if (state->currentClass && state->currentClass->baseType) {
        slotNum = ecLookupVar(cp, (EjsObj*) state->currentClass->baseType, &np->qname);
        if (slotNum >= 0 && cp->lookup.obj == (EjsObj*) state->currentClass->baseType) {
            if (!(np->attributes & EJS_FUN_OVERRIDE) && !state->currentClass->baseType->isInterface) {
                astError(cp, np, 
                    "Function \"%s\" is already defined in a base class. Try using \"override\" keyword.", np->qname.name);
                return 0;
            }
            /*
                Install the new function into the v-table by overwriting the method from the closest base class.
                Must now define the name of the property and attributes.
             */
            allocName(ejs, &np->qname);
            ejsDefineProperty(ejs, (EjsObj*) block, slotNum, &np->qname, 0, np->attributes, (EjsObj*) fun);
        }
    }

    /*
        Test for clashes with non-overridden methods in base classes and implemented classes.
     */
    if (state->currentClass && state->currentClass->implements) {
        next = 0;
        while ((iface = (EjsType*) mprGetNextItem(state->currentClass->implements, &next))) {
            slotNum = ecLookupVar(cp, (EjsObj*) iface, &np->qname);
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
                    allocName(ejs, &np->qname);
                    ejsDefineProperty(ejs, (EjsObj*) block, slotNum, &np->qname, 0, np->attributes, 
                        (EjsObj*) fun);
                }
            }
        }
    }

    if (resultTypeNode) {
        //  MOB -- assumes all types are in global. Should do a scope search?
        if (resolveName(cp, resultTypeNode, ejs->global, &resultTypeNode->qname) < 0) {
            if (STRICT_MODE(cp)) {
                astError(cp, np, "Can't find type \"%s\". All variables must be declared and typed in strict mode.", 
                    resultTypeNode->qname.name);
            }
        } else {
            resultTypeNode->qname.space = resultTypeNode->lookup.name.space;
        }
    }

    if (resolveName(cp, np, block, &np->qname) < 0) {
        astError(cp, np, "Internal error. Can't bind function %s", np->qname.name);
    }
    if (np->lookup.slotNum >= 0) {
#if UNUSED
        ejsSetFunctionLocation(fun, block, np->lookup.slotNum);
        if (np->function.getter) {
            if (fun->setter) {
                ejsSetFunctionLocation(fun->setter, block, np->lookup.slotNum);
            }
        }
#endif
        setAstDocString(ejs, np, np->lookup.obj, np->lookup.slotNum);
    }

    /*
        Bind the result type. Set the result type in np->trait->type
     */
    if (resultTypeNode) {
        mprAssert(resultTypeNode->lookup.ref == 0 || ejsIsType(resultTypeNode->lookup.ref));
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
#if OLD
    scope = ejs->state->bp->scope;
#endif
    if (ejsIsType(block)) {
        /* Skip the type out of the scope chain. Replace by "this" at run-time in the VM */
        fun->block.scope = ((EjsBlock*) block)->scope;
    } else {
        fun->block.scope = (EjsBlock*) block;
    }
    return fun;
}


/*
    Process the N_FUNCTION node and bind the return type and parameter types
 */
static void astFunction(EcCompiler *cp, EcNode *np)
{
    Ejs             *ejs;
    EjsFunction     *fun;
    EjsObj          *block;
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
            removeProperty(cp, block, np);
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

    /*
        Define and bind the parameters and their types
     */
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

    state->optimizedLetBlock = (EjsObj*) fun;
    state->letBlock = (EjsObj*) fun;
    state->varBlock = (EjsObj*) fun;

    if (np->function.body) {
        openBlock(cp, np->function.body, (EjsBlock*) fun);
#if CHANGE && MOB
        ejsDefineReservedNamespace(ejs, (EjsBlock*) fun, 0, EJS_PRIVATE_NAMESPACE);
#endif
        mprAssert(np->function.body->kind == N_DIRECTIVES);
        processAstNode(cp, np->function.body);
        closeBlock(cp);
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
                    if (ejs->initialized || fun->resultType == 0 || strcmp(fun->resultType->qname.name, "Void") != 0) {
                        astError(cp, np, "Function \"%s\" must return a value",  np->qname.name);
                    }
                }
            }
        }
        ejsCompleteFunction(ejs, fun);
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
    EjsName     qname;
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
#if UNUSED || 1
        EjsType     *iteratorType;
        ejsName(&qname, "iterator", "Iterator");
        iteratorType = (EjsType*) ejsGetPropertyByName(ejs, ejs->global, &qname);
        mprAssert(iteratorType);
        if (iteratorType == 0) {
            astError(cp, np, "Can't find Iterator class");
        } else {
            /*
                TODO MOB - this assumes that iterators use Iterator and it is bindable. What if an operator that
                implements an Iterable/Iterator interface
             */
            ejsName(&qname, "public", "next");
            rc = resolveName(cp, np->forInLoop.iterNext, (EjsObj*) iteratorType->prototype, &qname);
            if (rc < 0) {
                astError(cp, np, "Can't find Iterator.next method");
            }
        }
        
#else
        //  MOB -- can we use this?
        ejsName(&qname, "public", "next");
        resolveName(cp, np->forInLoop.iterNext, (EjsObj*) ejs->iterator, &qname);
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

    ejs = cp->ejs;
    mprAssert(ejs->exception == 0);
    
    mp = ejsCreateModule(cp->ejs, "__conditional__", 0);
    if (mp == 0) {
        return 0;
    }
    mp->initializer = createModuleInitializer(cp, np, mp);
    mp->initializer->isInitializer = 1;
    mp->hasInitializer = 1;

    if (astProcess(cp, np) < 0) {
        return 0;
    }
    ecResetParser(cp);
    ecGenConditionalCode(cp, np, mp);
    if (cp->errorCount > 0) {
        return 0;
    }
    result = ejsRunInitializer(ejs, mp);
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
    EjsType         *type;
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
        rc = resolveName(cp, np, ejs->global, &np->qname);
        if (rc < 0) {
            //  NOTE: np->qname.space may be null
            astError(cp, np, "Can't find class \"%s\". Ensure the class is visible.", np->qname.name);
        }

    } else if (left) {
        if (left->kind == N_THIS) {
            /*
                Explicit "this.property"
                TODO - does not yet handle "this function, this callee, this function, this type"
             */
            if (state->currentClass) {
                rc = resolveProperty(cp, np, state->currentClass, &np->qname);
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
            //  MOB -- is this right to use the IsPrototype on this?
            if (left->lookup.ref && (ejsIsType(left->lookup.ref) || ejsIsPrototype(left->lookup.ref))) {
                /*
                    Case 2. Type.property. We have resolved the type reference.
                 */
                np->lookup.ownerIsType = 1;
                rc = resolveProperty(cp, np, (EjsType*) left->lookup.ref, &np->qname);
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
                if (fun && ejsIsFunction(fun)) {
                    /* 
                        Can't use a getter to bind to as the value is determined at run time.
                     */
                    rc = -1;

                } else {

                    /*
                        Case 1: Left side is a normal object. We use the type of the lhs to search for name.
                     */
                    rc = resolveProperty(cp, np, left->lookup.trait->type, &np->qname);
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
        rc = resolveName(cp, np, NULL, &np->qname);

        /*
            Check for static function code accessing instance properties or instance methods
         */
        lookup = &np->lookup;
        if (rc == 0 && state->inClass && !state->instanceCode) {
            if (lookup->obj->isPrototype || 
                    (ejsIsType(lookup->obj) && (lookup->trait && !(lookup->trait->attributes & EJS_PROP_STATIC)))) {
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

#if MOB || DISABLE_ALL_BINDING || 1
    if (lookup->obj != (EjsObj*) state->currentFunction) {
        lookup->slotNum = -1;
        lookup->obj = 0;
        lookup->useThis = 0;
    }
#endif

    if (lookup->slotNum >= 0) {
        /*
            Unbind if slot number won't fit in one byte or the object is not a standard Object. The bound op codes 
            require one byte slot numbers.
         */
        if (lookup->slotNum >= 256) {
            lookup->slotNum = -1;
        }

        if (lookup->obj == ejs->global && !cp->bind) {
            /*
                Unbind non-core globals
             */
            if ((lookup->slotNum >= ES_global_NUM_CLASS_PROP) && !(lookup->ref && lookup->ref->builtin)) {
                lookup->slotNum = -1;
            }
        }

        if (ejsIsType(np->lookup.obj)) {
            type = (EjsType*) np->lookup.obj;
            if (type->block.nobind || type->isInterface) {
                /*
                    Type requires non-bound access. Types that implement interfaces will have different slots.
                 */
                lookup->slotNum = -1;

            } else if (type->dynamicInstance && !type->block.obj.builtin) {
                /*
                    Don't bind non-core dynamic properties
                 */
                lookup->slotNum = -1;

            } else {
                /*
                    Ugly (but effective) hack just for XML to discriminate between length and length()
                    TODO - refactor away
                 */
                if (type == ejs->xmlType || type == ejs->xmlListType) {
                    if (np->parent == 0 || np->parent->parent == 0 || np->parent->parent->kind != N_CALL) {
                        lookup->slotNum = -1;
                    }
                }
            }

        } else if (ejsIsPrototype(np->lookup.obj)) {
            if (!np->lookup.obj->builtin) {
                lookup->slotNum = -1;
            }
        }
        if (lookup->trait && lookup->trait->attributes & EJS_TRAIT_GETTER) {
            lookup->slotNum = -1;
        }
    }

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
        if (type && ejsIsType(type)) {
            /* Type is bound, has no constructor or base class constructors */
            if (!type->hasConstructor && !type->hasBaseConstructors) {
                np->newExpr.callConstructors = 0;
            }
            
            /*
                Propagate up the left side. Increment nthBase because it is an instance.
             */
            np->qname = left->qname;
            np->lookup = left->lookup;
            np->lookup.trait = mprAllocObj(cp->ejs, EjsTrait);
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
    next = 0;
    while ((child = getNextAstNode(cp, np, &next)) != 0) {
        processAstNode(cp, child);
    }
}


static void astField(EcCompiler *cp, EcNode *np)
{
    if (np->field.fieldKind == FIELD_KIND_VALUE) {
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
    state->namespace = np->qname.name;

    next = 0;
    while ((child = getNextAstNode(cp, np, &next)) != 0) {
        processAstNode(cp, child);
    }
    LEAVE(cp);
}


static void astReturn(EcCompiler *cp, EcNode *np)
{
    EjsFunction     *fun;
    EcNode          *functionNode;
    EcState         *state;

    ENTER(cp);

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
                if (fun->resultType && fun->resultType == cp->ejs->voidType) {
                    /*
                        Allow block-less function expressions where a return node was generated by the parser.
                     */
                    if (!np->ret.blockLess) {
                        astError(cp, np, "Void function \"%s\" can't return a value", functionNode->qname.name);
                    }
                }

            } else {
                if (fun->resultType && fun->resultType != cp->ejs->voidType) {
                    if (! (!cp->ejs->initialized && strcmp(fun->resultType->qname.name, "Void") == 0)) {
                        astError(cp, np, "Return in function \"%s\" must return a value", functionNode->qname.name);
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
        if (!state->currentFunction->constructor) {
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
    case N_THIS_GENERATOR:
        //  TODO - binding not implemented
        break;

    case N_THIS_CALLEE:
        //  TODO - binding not implemented
        break;

    case N_THIS_TYPE:
        //  TODO - binding not implemented
        if (!state->inClass) {
            astError(cp, np, "\"this type\" is only valid inside a class");
        } else {
            np->lookup.obj = (EjsObj*) state->currentClass;
            np->lookup.slotNum = 0;
        }
        break;

    case N_THIS_FUNCTION:
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
        for (count = 0, block = ejs->state->bp->scope; block && !ejsIsFunction(block); block = block->scope) {
            if (!block->obj.hidden) {
                count++;
            }
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
        ejsSetDebugName(block, "finally");
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
    openBlock(cp, np, (EjsBlock*) mp->initializer);
    
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
    
    state->optimizedLetBlock = ejs->global;
    state->varBlock = ejs->global;
    state->letBlock = (EjsObj*) mp->initializer;
    state->currentModule = mp;

    if (mp->dependencies == 0) {
        mp->dependencies = mprCreateList(mp);
        core = ejsLookupModule(ejs, "ejs", 0, 0);
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

    ENTER(cp);

    state = cp->state;

    mprAssert(np->kind == N_USE_NAMESPACE);

    ejs = cp->ejs;
    namespace = 0;

    if (cp->phase == EC_PHASE_CONDITIONAL) {
        /*
            At the define phase, we create a dummy namespace assuming that it will exist somewhere in this block or an 
            outer block. At the fixup phase, we actually resolve the reference to the namespace unless it is a string 
            literal namespace.
         */
        namespace = ejsCreateNamespace(ejs, np->qname.name, np->qname.name);
        np->namespaceRef = namespace;

    } else if (cp->phase >= EC_PHASE_BIND) {
        if (np->useNamespace.isLiteral) {
            namespace = np->namespaceRef;

        } else {
            /*
                Resolve the real namespace. Must be visible in the current scope (even in standard mode). 
                Then update the URI. URI not used.
             */
            if (resolveName(cp, np, NULL, &np->qname) < 0) {
                astError(cp, np, "Can't find namespace \"%s\"", np->qname.name);

            } else {
                namespace = (EjsNamespace*) np->lookup.ref;
                np->namespaceRef->uri = namespace->uri;

                if (!ejsIsNamespace(namespace)) {
                    astError(cp, np, "The variable \"%s\" is not a namespace", np->qname.name);
                } else {
                    np->namespaceRef = namespace;
                }
            }
            if (namespace && np->useNamespace.isDefault) {
                /*
                    Apply the namespace URI to all upper blocks
                 */
                for (s = cp->state; s; s = s->prev) {
                    s->namespace = (char*) namespace->uri;
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
            ejsAddNamespaceToBlock(ejs, (EjsBlock*) state->letBlock, namespace);
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
                    astError(cp, np, "Error loading module \"%s\"\n%s", np->qname.name, ejsGetErrorMsg(ejs, 0));
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
                currentModule->dependencies = mprCreateList(currentModule);
            }
            if (mprLookupItem(currentModule->dependencies, mp) < 0 && mprAddItem(currentModule->dependencies, mp) < 0) {
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
    if (strcmp(first->qname.name, second->qname.name) == 0 && strcmp(first->qname.space, second->qname.space) == 0) {
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
        obj = getBlockForDefinition(cp, np, state->optimizedLetBlock, attributes);
        if (ecLookupScope(cp, &np->qname) >= 0 && cp->lookup.obj == obj) {
            obj = cp->lookup.obj;
            slotNum = cp->lookup.slotNum;
            if (cp->fileState->strict) {
                astError(cp, np, "Variable \"%s\" is already defined", np->qname.name);
                return;
            }
        } else {
            //  TODO MOB BUG - could / should change context to be obj for the names
            allocName(ejs, &np->qname);
            slotNum = ejsDefineProperty(ejs, obj, -1, &np->qname, 0, attributes, value);
        }

    } else {
        if (ecLookupVar(cp, state->varBlock, &np->qname) >= 0) {
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
        obj = getBlockForDefinition(cp, np, state->varBlock, attributes);
        allocName(ejs, &np->qname);
        slotNum = ejsDefineProperty(ejs, obj, -1, &np->qname, 0, attributes, value);
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
    EjsObj      *block, *obj, *vp;
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
        obj = state->varBlock;
    } else {
        /*
            Global or class level block
         */
        mprAssert(!state->instanceCode);
        obj = state->varBlock;
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
    slotNum = ejsLookupProperty(ejs, block, &np->qname);
    if (slotNum < 0) {
        mprAssert(state->letBlockNode->namespaceRef);
        np->namespaceRef = state->letBlockNode->namespaceRef;
        np->qname.space = np->namespaceRef->uri;
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
        np->qname.space = np->namespaceRef->uri;

        allocName(ejs, &np->qname);
        slotNum = ejsDefineProperty(ejs, obj, -1, &np->qname, 0, attributes, vp);
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
    EjsObj          *block;
    EjsTrait        *trait;
    EcState         *state;
    EcNode          *typeNode;

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
    if (!state->inFunction) {
        if (!np->literalNamespace && resolveNamespace(cp, np, block, 0) == 0) {
            LEAVE(cp);
            return;
        }
    }
    if (cp->phase == EC_PHASE_BIND && block == ejs->global) {
        addGlobalProperty(cp, np, &np->qname);
    }
    /*
        Check if there is a name clash with any subclasses. Must do after fixup so that the base type has been defined.
        Look in the current type for any public property of the same name.
     */
    if (state->inClass && !state->inFunction && state->currentClass->baseType) {
        if (ecLookupVar(cp, (EjsObj*) state->currentClass->baseType, &np->qname) >= 0) {
            astError(cp, np, "Public property %s is already defined in a base class", np->qname.name);
            LEAVE(cp);
            return;
        }
    }
    if (resolveName(cp, np, block, &np->qname) < 0) {
        astError(cp, np, "Can't find variable \"%s::%s\"", np->qname.space, np->qname.name);
    }
    typeNode = np->typeNode;
    if (typeNode && np->lookup.trait) {
        if (typeNode->lookup.ref) {
            if (!cp->fileState->strict) {
                /*
                    Allow variable redefinitions providing they are compatible
                 */
                trait = ejsGetPropertyTrait(ejs, np->lookup.obj, np->lookup.slotNum);
                if (!typeIsCompatible(cp, trait->type, (EjsType*) typeNode->lookup.ref)) {
                    astError(cp, np, "Redefinition of \"%s\" is not compatible with prior definition", np->qname.name);
                    LEAVE(cp);
                    return;
                }
            }
            ejsSetTraitType(np->lookup.trait, (EjsType*) typeNode->lookup.ref);
        }
    }
    setAstDocString(ejs, np, np->lookup.obj, np->lookup.slotNum);
    LEAVE(cp);
}


/*
    Define a variable
 */
static void astVar(EcCompiler *cp, EcNode *np, int varKind, EjsObj *value)
{
    EcState     *state;
    Ejs         *ejs;
    EjsObj      *obj;

    ejs = cp->ejs;
    state = cp->state;

    if (state->disabled) {
        if (cp->phase == EC_PHASE_CONDITIONAL) {
            obj = getBlockForDefinition(cp, np, (EjsObj*) state->varBlock, np->attributes);
            removeProperty(cp, obj, np);
        }
        return;
    }

    state->instanceCode = 0;
    if (state->inClass && !(np->attributes & EJS_PROP_STATIC)) {
        if (state->inMethod) {
            state->instanceCode = 1;

        } else if (cp->classState->blockNestCount == (cp->state->blockNestCount - 1)) {
            /*
                Top level var declaration without a static attribute
             */
            state->instanceCode = 1;
        }

    } else {
        mprAssert(state->instanceCode == 0);
    }

    if (np->typeNode) {
        if (np->typeNode->kind != N_QNAME) {
            astError(cp, np, "Bad type name");
            return;
        }
        if (strcmp(np->typeNode->qname.name, "*") != 0) {
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
}


/*
    Define variables
 */
static void astVarDefinition(EcCompiler *cp, EcNode *np, int *codeRequired, int *instanceCode)
{
    Ejs         *ejs;
    EcNode      *child, *var, *right;
    EcState     *state;
    int         next, varKind, slotNum;

    mprAssert(np->kind == N_VAR_DEFINITION);

    ENTER(cp);

    ejs = cp->ejs;
    state = cp->state;
    mprAssert(state);

    varKind = np->def.varKind;

    next = 0;
    while ((child = getNextAstNode(cp, np, &next))) {
        if (child->kind == N_ASSIGN_OP) {
            var = child->left;
        } else {
            var = child;
        }
        astVar(cp, var, np->def.varKind, var->name.value);
        
        if (state->disabled) {
            continue;
        }
        if (child->kind == N_ASSIGN_OP) {
            *instanceCode = state->instanceCode;
            *codeRequired = 1;
        }
        if (child->kind == N_ASSIGN_OP) {
            astAssignOp(cp, child);

            right = child->right;
            mprAssert(right);

            /*
                Define constants here so they can be used for conditional compilation and "use namespace". We erase after the
                conditional phase.
             */
            if (right->kind == N_LITERAL && !(np->def.varKind & KIND_LET) && !(var->attributes & EJS_PROP_NATIVE)) {
                mprAssert(var->kind == N_QNAME);
                mprAssert(right->literal.var);
                /* Exclude class instance variables */
                if (! (state->inClass && !(var->attributes & EJS_PROP_STATIC))) {
                    slotNum = ejsLookupProperty(ejs, state->varBlock, &var->qname);
                    if (cp->phase == EC_PHASE_DEFINE) {
                        ejsSetProperty(ejs, state->varBlock, slotNum, right->literal.var);

                    } else if (cp->phase >= EC_PHASE_BIND && !var->name.isNamespace && slotNum >= 0) {
                        /*
                            Erase the value incase being run in the ejs shell. Must not prematurely define values.
                         */
                        ejsSetProperty(ejs, state->varBlock, slotNum, ejs->nullValue);
                    }
                }
            }
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

    mprAssert(np->kind == N_VOID);

    ENTER(cp);

    if (cp->phase >= EC_PHASE_BIND) {
        qname.name = "Void";
        qname.space = EJS_EJS_NAMESPACE;

        if (resolveName(cp, np, 0, &qname) < 0) {
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
    fun = ejsCreateFunction(ejs, mp->name, 0, -1, 0, 0, 0, ejs->voidType, EJS_FUN_INITIALIZER, mp->constants, mp->scope, 
        cp->state->strict);
    if (fun == 0) {
        astError(cp, np, "Can't create initializer function");
        return 0;
    }
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

    if (np->module.version == 0 && cp->modver && strcmp(np->qname.name, EJS_DEFAULT_MODULE) != 0) {
        np->module.version = cp->modver;
    }
    mp = ecLookupModule(cp, np->qname.name, np->module.version, np->module.version);
    if (mp == 0) {
        mp = ejsCreateModule(cp->ejs, np->qname.name, np->module.version);
        if (mp == 0) {
            astError(cp, np, "Can't create module %s", np->qname.name);
            return 0;
        }
        mp->scope = ejs->globalBlock;
        if (ecAddModule(cp, mp) < 0) {
            astError(cp, 0, "Can't insert module");
            return 0;
        }
        /*
            This will prevent the loading of any module that uses this module.
         */
        if (strcmp(mp->name, EJS_DEFAULT_MODULE) != 0) {
            mp->compiling = 1;
        }
    }

    if (mp->initializer == 0) {
        mp->initializer = createModuleInitializer(cp, np, mp);
    }
    np->module.ref = mp;

    if (cp->outputFile) {
        np->module.filename = cp->outputFile;
    } else {
        np->module.filename = mprStrcat(np, -1, np->qname.name, EJS_MODULE_EXT, NULL);
    }
    return mp;
}


static void astError(EcCompiler *cp, EcNode *np, char *fmt, ...)
{
    va_list     arg;
    char        *msg;

    va_start(arg, fmt);

    if ((msg = mprVasprintf(cp, 0, fmt, arg)) == NULL) {
        msg = "Memory allocation error";
    }
    cp->errorCount++;
    cp->error = 1;
    cp->noout = 1;
    
    if (np) {
        ecReportError(cp, "error", np->filename, np->lineNumber, np->currentLine, np->column, msg);
    } else {
        ecReportError(cp, "error", 0, 0, 0, 0, msg);
    }
    mprFree(msg);
    va_end(arg);
}


static void astWarn(EcCompiler *cp, EcNode *np, char *fmt, ...)
{
    va_list     arg;
    char        *msg;

    va_start(arg, fmt);

    if ((msg = mprVasprintf(cp, 0, fmt, arg)) == NULL) {
        msg = "Memory allocation error";
        cp->errorCount++;
        cp->error = 1;
    }
    cp->warningCount++;
    ecReportError(cp, "warning", np->filename, np->lineNumber, np->currentLine, np->column, msg);
    mprFree(msg);
    va_end(arg);
}


static void badAst(EcCompiler *cp, EcNode *np)
{
    cp->fatalError = 1;
    cp->errorCount++;
    mprError(cp, "Unsupported language feature\nUnknown AST node kind %d",  np->kind);
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

    case N_ARRAY_LITERAL:
        processAstNode(cp, np->left);
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
        // codeRequired++;
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
        /*
            Namespaces by themselves don't required code. Need something to use the namespace.
         */
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
                state->currentClass->hasInitializer = 1;
            } else {
                state->currentClass->hasStaticInitializer = 1;
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
            if (strcmp(np->qname.space, prop->space) == 0 && strcmp(np->qname.name, prop->name) == 0) {
                mprRemoveItem(globals, prop);
                break;
            }
        }
    }
    slotNum = ejsLookupProperty(ejs, obj, &np->qname);
    if (slotNum >= 0) {
        ejsRemoveProperty(ejs, obj, slotNum);
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
    EjsType         *baseType, *iface;
    EjsFunction     *fun;
    EjsObj          *prototype;
    EjsName         qname;
    EjsTrait        *trait;
    EcNode          *np, *child;
    int             rc, slotNum, attributes, next;

    if (type->block.obj.visited || !type->needFixup) {
        return;
    }
    type->needFixup = 0;

    mprAssert(cp);
    mprAssert(type);
    mprAssert(ejsIsType(type));

    ENTER(cp);

    rc = 0;
    ejs = cp->ejs;
    type->block.obj.visited = 1;
    np = (EcNode*) type->typeData;
    baseType = type->baseType;

    if (baseType == 0) {
        if (np && np->kind == N_CLASS && !np->klass.isInterface) {
            if (np->klass.extends) {
                ejsName(&qname, 0, np->klass.extends);
                baseType = (EjsType*) getTypeProperty(cp, ejs->global, &qname);
            } else {
                if (! (!ejs->initialized && strcmp(type->qname.name, "Object") == 0)) {
                    ejsName(&qname, EJS_EJS_NAMESPACE, "Object");
                    baseType = (EjsType*) getTypeProperty(cp, ejs->global, &qname);
                }
            }
        }
    }

    if (np->klass.implements) {
        type->implements = mprCreateList(type);
        next = 0;
        while ((child = getNextAstNode(cp, np->klass.implements, &next))) {
            iface = (EjsType*) getTypeProperty(cp, ejs->global, &child->qname);
            if (iface) {
                mprAddItem(type->implements, iface);
            } else {
                astError(cp, np, "Can't find interface \"%s\"", child->qname.name);
                type->block.obj.visited = 0;
                LEAVE(cp);
                return;
            }
        }
    }

    if (baseType == 0) {
        if (! (!ejs->initialized && strcmp(type->qname.name, "Object") == 0) && !np->klass.isInterface) {
            astError(cp, np, "Can't find base type for %s", type->qname.name);
            type->block.obj.visited = 0;
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
        if (baseType->hasStaticInitializer) {
            type->hasBaseStaticInitializers = 1;
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
            if (iface->hasStaticInitializer) {
                type->hasBaseStaticInitializers = 1;
            }
        }
    }

    if (!type->block.obj.isPrototype && (EjsObj*) type != ejs->global && !type->isInterface) {
        /*
            Remove the static initializer slot if this class does not require a static initializer
            By convention, it is installed in slot number 1.
         */
        if (type->hasBaseStaticInitializers) {
            type->hasStaticInitializer = 1;
        }
        if (!type->hasStaticInitializer) {
            ejsRemoveProperty(ejs, (EjsObj*) type, 0);
        }
        /*
            Remove the constructor slot if this class does not require a constructor. ie. no base classes have constructors
         */
        if (type->hasBaseConstructors) {
            type->hasConstructor = 1;
        }
        if (!type->hasConstructor) {
            if (np && np->klass.constructor && np->klass.constructor->function.isDefaultConstructor) {
                ejsRemoveProperty(ejs, (EjsObj*) type->prototype, 0);
                np->klass.constructor = 0;
            }
        }
    }

    if (!ejs->initialized) {
        if (type->hasConstructor && strcmp("Object", type->qname.name) == 0 && 
                strcmp(EJS_EJS_NAMESPACE, type->qname.space) == 0) {
            astWarn(cp, np, "Object class requires a constructor, but the native class does not implement one.");
        }
    }

    if (baseType) {
        ejsFixupType(ejs, type, baseType, 1);
    }
    
#if NOT_NEEDED
    /*
        Mark all inherited methods implemented for interfaces as implicitly overridden
     */
    for (slotNum = 0; slotNum < type->numInherited; slotNum++) {
        qname = ejsGetPropertyName(ejs, (EjsObj*) type, slotNum);
        fun = (EjsFunction*) ejsGetProperty(ejs, (EjsObj*) type, slotNum);
        if (fun && ejsIsFunction(fun)) {
            owner = (EjsType*) fun->owner;
            if (owner != type && ejsIsType(owner) && owner->isInterface) {
                //  MOB -- remove override - use traits
                fun->override = 1;
            }
        }
    }
#endif
    
    /*
        Remove the original overridden method. Set the inherited slot to the overridden method. This implements a v-table.
     */
    prototype = type->prototype;
    for (slotNum = type->numPrototypeInherited; slotNum < prototype->numSlots; slotNum++) {
        trait = ejsGetPropertyTrait(ejs, prototype, slotNum);
        if (trait == 0) {
            continue;
        }
        attributes = trait->attributes;
        if (attributes & EJS_FUN_OVERRIDE) {
            fun = (EjsFunction*) ejsGetProperty(ejs, prototype, slotNum);
            mprAssert(fun && ejsIsFunction(fun));
            qname = ejsGetPropertyName(ejs, prototype, slotNum);
            ejsRemoveProperty(ejs, prototype, slotNum);
            slotNum--;
            if (resolveName(cp, NULL, prototype, &qname) < 0 || cp->lookup.slotNum < 0) {
                astError(cp, 0, "Can't find method \"%s::%s\" in base type of \"%s\" to override", qname.space, qname.name, 
                    type->qname.name);
            } else {
#if UNUSED
                fun->slotNum = cp->lookup.slotNum;
#endif
                ejsSetProperty(ejs, prototype, cp->lookup.slotNum, (EjsObj*) fun);
                trait = ejsGetTrait(prototype, cp->lookup.slotNum);
                ejsSetTraitAttributes(trait, attributes);
            }
        }
    }
    type->block.obj.visited = 0;
    LEAVE(cp);
}


/*
    Lookup a namespace in the current scope. We look for the namespace variable declaration if it is a user
    defined namespace. Otherwise, we trust that if the set of open namespaces has the namespace -- it must exist.
 */
static EjsNamespace *resolveNamespace(EcCompiler *cp, EcNode *np, EjsObj *block, bool *modified)
{
    Ejs             *ejs;
    EjsName         qname;
    EjsNamespace    *namespace;
    int             slotNum;

    ejs = cp->ejs;

    if (modified) {
        *modified = 0;
    }

    /*
        Resolve the namespace. Must be visible in the current scope. Then update the URI.
     */
    qname.name = np->qname.space;
    qname.space = 0;
    namespace = (EjsNamespace*) getTypeProperty(cp, 0, &qname);
    if (namespace == 0 || !ejsIsNamespace(namespace)) {
        namespace = ejsLookupNamespace(cp->ejs, np->qname.space);
    }
    if (namespace == 0) {
        if (strcmp(cp->state->namespace, np->qname.space) == 0) {
            namespace = ejsCreateNamespace(ejs, np->qname.space, np->qname.space);
        }
    }
    if (namespace == 0) {
        if (!np->literalNamespace) {
            astError(cp, np, "Can't find namespace \"%s\"", qname.name);
        }
    } else {
        if (strcmp(namespace->uri, np->qname.space) != 0) {
            slotNum = ejsLookupProperty(ejs, block, &np->qname);
            mprAssert(slotNum >= 0);
            if (slotNum >= 0) {
                mprFree((char*) np->qname.space);
                /*
                    Change the name to use the namespace URI. This will change the property name and set
                    "modified" so that the caller can modify the type name if block is a type.
                 */
                //  MOB BUG - context not right for names
                np->qname.space = mprStrdup(ejs, namespace->uri);
                ejsSetPropertyName(ejs, block, slotNum, &np->qname);
                if (modified) {
                    *modified = 1;
                }
            }
        }
    }
    return namespace;
}


/*
    Locate a property in context. NOTE this only works for type properties not instance properties.
 */
static EjsObj *getTypeProperty(EcCompiler *cp, EjsObj *vp, EjsName *name)
{
    EcNode      node;

    mprAssert(cp);

    if (resolveName(cp, &node, vp, name) < 0) {
        return 0;
    }
    return node.lookup.ref;
}


static int resolveProperty(EcCompiler *cp, EcNode *np, EjsType *type, EjsName *name)
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
static int resolveName(EcCompiler *cp, EcNode *np, EjsObj *vp, EjsName *qname)
{
    Ejs         *ejs;
    EjsLookup   *lookup;
    EjsType     *type, *currentClass, *tp;
    EcState     *state;
    EjsBlock    *block;

    ejs = cp->ejs;
    state = cp->state;
    lookup = &cp->lookup;

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

    /*
        Revise the nth block to account for blocks that will be erased
     */
    lookup->nthBlock = 0;
    for (block = ejs->state->bp->scope; block; block = block->scope) {
        if ((EjsObj*) block == lookup->obj) {
            break;
        }
        if (ejsIsType(block)) {
            type = (EjsType*) block;
            if ((EjsObj*) type->prototype == lookup->obj) {
                break;
            }
        }
        if (!block->obj.hidden) {
            lookup->nthBlock++;
        }
    }
    if (block == 0) {
        lookup->nthBlock = 0;
    }
    mprAssert(lookup->ref);    
    if (lookup->ref == ejs->nullValue) {
        lookup->ref = 0;
    }

    if ((ejsIsType(lookup->obj) || ejsIsPrototype(lookup->obj)) && state->currentObjectNode == 0) {
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
    EjsName         *prop, *p;
    int             next;

    ejs = cp->ejs;

    up = cp->state->currentModule;
    mprAssert(up);

    prop = mprAllocObjZeroed(cp, EjsName);
    *prop = *qname;

    if (up->globalProperties == 0) {
        up->globalProperties = mprCreateList(up);
    }

    for (next = 0; (p = (EjsName*) mprGetNextItem(up->globalProperties, &next)) != 0; ) {
        if (strcmp(p->name, prop->name) == 0 && strcmp(p->space, prop->space) == 0) {
            return;
        }
    }
    next = mprAddItem(up->globalProperties, prop);
    if (np) {
        np->globalProp = prop;
    }
}


static void setAstDocString(Ejs *ejs, EcNode *np, EjsObj *block, int slotNum)
{
    mprAssert(block);

    if (np->doc && slotNum >= 0 && ejsIsBlock(block)) {
        ejsCreateDoc(ejs, (EjsBlock*) block, slotNum, np->doc);
    }
}


static void addScope(EcCompiler *cp, EjsBlock *block)
{
    block->scope = cp->ejs->state->bp->scope;
    cp->ejs->state->bp->scope = block;
}


static void removeScope(EcCompiler *cp)
{
    EjsBlock    *block;

    block = cp->ejs->state->bp;
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
            if (np->filename == 0) {
                debugName = mprAsprintf(np, -1, "block_%04d", index++);
            } else {
                debugName = mprAsprintf(np, -1, "block_%04d_%d", np->lineNumber, index++);
            }
            block = ejsCreateBlock(cp->ejs, 0);
            ejsSetDebugName(block, debugName);
            np->qname.name = debugName;
            np->qname.space = EJS_BLOCK_NAMESPACE;
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
        mprAssert(block != ejs->globalBlock);
        ejsResetBlockNamespaces(ejs, block);
    }
    
    state->namespaceCount = ejsGetNamespaceCount(block);

    /*
        Special case for the outermost module block. The module (file) block is created to provide a compilation unit
        level scope. However, we do not use the block for the let or var scope, rather we use the global scope.
        Namespaces always use this new block.
     */
    if (! (state->letBlock == ejs->global && np->parent->kind == N_MODULE)) {
        state->optimizedLetBlock = (EjsObj*) block;
    }
    state->letBlock = (EjsObj*) block;
    state->letBlockNode = np;

    /*
        Add namespaces that must apply from the start of the block. Current users: hoisted let vars.
     */
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
    ejsPopBlockNamespaces((EjsBlock*) cp->state->letBlock, cp->state->namespaceCount);
    cp->blockState = cp->state->prevBlockState;
    removeScope(cp);
}


static EjsNamespace *createHoistNamespace(EcCompiler *cp, EjsObj *obj)
{
    EjsNamespace    *namespace;
    Ejs             *ejs;
    EcNode          *letBlockNode;
    char            *spaceName;

    ejs = cp->ejs;
    spaceName = mprAsprintf(cp, -1, "-hoisted-%d", ejsGetPropertyCount(ejs, obj));
    namespace = ejsCreateNamespace(ejs, spaceName, spaceName);

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
static EjsObj *getBlockForDefinition(EcCompiler *cp, EcNode *np, EjsObj *block, int attributes)
{
    EcState     *state;
    EjsType     *type;

    state = cp->state;

    if (ejsIsType(block) && state->inClass) {
        if (!(attributes & EJS_PROP_STATIC) && !state->inFunction &&
                cp->state->blockNestCount <= (cp->classState->blockNestCount + 1)) {
            /*
                Use the prototype object if not static, outside a function and in the top level block.
             */
            type = (EjsType*) block;
#if UNUSED
            if (type->prototype == 0) {
                type->prototype = (EjsObj*) ejsCreatePrototype(cp->ejs, type, 0);
            }
#endif
            block = (EjsObj*) type->prototype;
            if (np->kind == N_QNAME) {
                np->name.instanceVar = 1;
            }
        }
    }
    return block;
}


//  MOB -- remove ejs prefix
static EjsNamespace *ejsLookupNamespace(Ejs *ejs, cchar *namespace)
{
    EjsList         *namespaces;
    EjsNamespace    *nsp;
    EjsBlock        *block;
    int             nextNamespace;

    /*
        Lookup the scope chain considering each block and the open namespaces at that block scope.
     */
    for (block = ejs->state->bp; block; block = block->scope) {
        if (!ejsIsBlock(block)) {
            continue;
        }

        namespaces = &block->namespaces;
        if (namespaces == 0) {
            namespaces = &ejs->globalBlock->namespaces;
        }

#if BLD_DEBUG
        mprLog(ejs, 7, "ejsLookupNamespace in %s", ejsGetDebugName(block));
        for (nextNamespace = 0; (nsp = (EjsNamespace*) ejsGetNextItem(namespaces, &nextNamespace)) != 0; ) {
            mprLog(ejs, 7, "    scope \"%s\"", nsp->uri);
        }
#endif

        mprLog(ejs, 7, "    SEARCH for qname \"%s\"", namespace);
        for (nextNamespace = -1; (nsp = (EjsNamespace*) ejsGetPrevItem(namespaces, &nextNamespace)) != 0; ) {
            if (strcmp(nsp->uri, namespace) == 0) {
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
int ecLookupScope(EcCompiler *cp, EjsName *name)
{
    Ejs             *ejs;
    EjsState        *state;
    EjsBlock        *block;
    EjsType         *type;
    EjsLookup       *lookup;
    EjsObj          *prototype, *obj;
    int             slotNum, nthBlock, nthBase;

    mprAssert(cp);
    mprAssert(name);
    mprAssert(name->name);

    ejs = cp->ejs;
    if (name->space == 0) {
        name->space = "";
    }
    slotNum = -1;
    lookup = &cp->lookup;
    memset(lookup, 0, sizeof(*lookup));

    state = ejs->state;

    for (nthBlock = 0, block = state->bp->scope; block; block = block->scope, nthBlock++) {
        if (ejsIsType(block)) {
            obj = ejsIsType(block) ? ((EjsType*) block)->prototype : (EjsObj*) block;
            if ((slotNum = ejsLookupVarWithNamespaces(ejs, obj, name, lookup)) >= 0) {
                lookup->nthBlock = nthBlock;
                return slotNum;
            }
        } else {
            obj = ejsIsType(block) ? ((EjsType*) block)->prototype : (EjsObj*) block;
            if ((slotNum = ejsLookupVarWithNamespaces(ejs, obj, name, lookup)) >= 0) {
                lookup->nthBlock = nthBlock;
                return slotNum;
            }
        }
        type = block->obj.type;
        for (nthBase = 1; type; type = type->baseType, nthBase++) {
            if ((prototype = type->prototype) == 0 || prototype->shortScope) {
                break;
            }
            if ((slotNum = ejsLookupVarWithNamespaces(ejs, prototype, name, lookup)) >= 0) {
                lookup->nthBlock = nthBlock;
                lookup->nthBase = nthBase;
                return slotNum;
            }
        }
    }
    for (nthBlock = 0, block = state->bp->scope; block; block = block->scope, nthBlock++) {
        type = ejsIsType(block) ? (EjsType*) block : block->obj.type;
        for (nthBase = 1; type; type = type->baseType, nthBase++) {
            if (type->block.obj.shortScope) {
                //  MOB -- continue or break?
                continue;
            }
            if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) type, name, lookup)) >= 0) {
                lookup->nthBase = nthBase;
                lookup->nthBlock = nthBlock;
                return slotNum;
            }
        }
    }
    return -1;
}


int ecLookupVar(EcCompiler *cp, EjsObj *obj, EjsName *name)
{
    if (name->space == 0) {
        //  MOB -- this should be moved back into parser
        name->space = "";
    }
    return ejsLookupVar(cp->ejs, obj, name, &cp->lookup);
}


#if UNUSED
/*
    Find a variable in a block. Scope blocks are provided by the global object, types, functions and statement blocks.
    This is a copy of the code from the VM with the exception that we allow the access to instance methods from
    type objects.
 */
static int ecLookupVarWithNamespaces(Ejs *ejs, EjsObj *originalObj, EjsObj *vp, EjsName *name, EjsLookup *lookup)
{
    EjsNamespace    *nsp;
    EjsName         qname;
    EjsBlock        *b;
    EjsObj          *owner;
    EjsFunction     *ref;
    int             slotNum, nextNsp;

    mprAssert(vp);
    mprAssert(name);
    mprAssert(name->name);
    mprAssert(name->space);
    mprAssert(lookup);

    if ((slotNum = ejsLookupProperty(ejs, vp, name)) < 0 && name->space[0] != EJS_EMPTY_NAMESPACE[0]) {
        /* Explicit namespace - not need to search the set of open namespaces */
        return slotNum;
    }
    if (slotNum >= 0) {
        ref = (EjsFunction*) ejsGetProperty(ejs, vp, slotNum);

        if (ejsIsType(vp) && originalObj != vp && !ejsIsType(originalObj) && (!ejsIsFunction(ref) || ref->staticMethod)) {
            /* Accessing static var or static method from a sub-type of the original instance. 
               ie. Type properties should not be visible to instances */
            ;
        } else {

            /* Not accessing a static method from an instance */
            lookup->name = *name;
            lookup->obj = vp;
            lookup->slotNum = slotNum;
            return slotNum;
        }
    }
    qname = *name;
    for (b = ejs->state->bp; b; b = b->scope) {
        for (nextNsp = -1; (nsp = (EjsNamespace*) ejsGetPrevItem(&b->namespaces, &nextNsp)) != 0; ) {

            if (nsp->flags & EJS_NSP_PROTECTED && vp->isType && ejs->state->fp) {
                /*
                    Protected access. See if the type containing the method we are executing is a sub class of the type 
                    containing the property ie. Can we see protected properties?
                 */
                owner = (EjsObj*) ejs->state->fp->function.owner;
                if (owner && !ejsIsA(ejs, owner, (EjsType*) vp)) {
                    continue;
                }
            }
            qname.space = nsp->uri;
            mprAssert(qname.space);
            if (qname.space) {
                slotNum = ejsLookupProperty(ejs, vp, &qname);
                if (slotNum >= 0) {
                    ref = (EjsFunction*) ejsGetProperty(ejs, vp, slotNum);
                    //  MOB -- since we need to get the trait anyway, better to determine this from the trait
                    lookup->name = qname;
                    lookup->obj = vp;
                    lookup->slotNum = slotNum;
                    return slotNum;
                }
            }
        }
    }
    return -1;
}
#endif


static void allocName(Ejs *ejs, EjsName *qname)
{
    qname->space = mprStrdup(ejs, qname->space);
    qname->name = mprStrdup(ejs, qname->name);
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
