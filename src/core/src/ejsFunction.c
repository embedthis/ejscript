/**
    ejsFunction.c - Function class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/******************************************************************************/
/*
    Create a function object.
 */
static EjsFunction *createFunction(Ejs *ejs, EjsType *type, int numSlots)
{
    EjsFunction     *fun;

    /*
        Note: Functions are not pooled, frames are.
     */
    fun = (EjsFunction*) ejsCreateObject(ejs, ejs->functionType, 0);
    if (fun == 0) {
        return 0;
    }
#if UNUSED
    //  MOB -- get rid of owner+slotNum
    fun->slotNum = -1;
#endif
    fun->block.obj.isFunction = 1;
    fun->block.obj.dynamic = 1;
    return fun;
}


/*
    Cast the operand to the specified type

    function cast(type: Type) : Object
 */
static EjsObj *castFunction(Ejs *ejs, EjsFunction *vp, EjsType *type)
{
    switch (type->id) {
    case ES_String:
        return (EjsObj*) ejsCreateString(ejs, "[function Function]");

    case ES_Number:
        return (EjsObj*) ejs->nanValue;

    case ES_Boolean:
        return (EjsObj*) ejs->trueValue;
            
    default:
        ejsThrowTypeError(ejs, "Can't cast type \"%s\"", type->qname.name);
        return 0;
    }
    return 0;
}


EjsFunction *ejsCloneFunction(Ejs *ejs, EjsFunction *src, int deep)
{
    EjsFunction     *dest;

    dest = (EjsFunction*) ejsCloneBlock(ejs, &src->block, deep);
    if (dest == 0) {
        return 0;
    }
    dest->body.code = src->body.code;
    dest->resultType = src->resultType;
    dest->thisObj = src->thisObj;
#if UNUSED
    dest->owner = src->owner;
    dest->slotNum = src->slotNum;
#endif
    dest->numArgs = src->numArgs;
    dest->numDefault = src->numDefault;

    /*
        OPT
     */
    dest->staticMethod = src->staticMethod;
    dest->constructor = src->constructor;
    dest->hasReturn = src->hasReturn;
    dest->isInitializer = src->isInitializer;
    dest->rest = src->rest;
    dest->fullScope = src->fullScope;
    dest->nativeProc = src->nativeProc;
    dest->strict = src->strict;

    if (src->activation) {
        dest->activation = ejsCloneObject(ejs, src->activation, 0);
    }
    ejsSetDebugName(dest, ejsGetDebugName(src));
    return dest;
}


static void destroyFunction(Ejs *ejs, EjsFunction *fun)
{
    ejsFree(ejs, (EjsObj*) fun, ES_Function);
}


static EjsObj *getFunctionProperty(Ejs *ejs, EjsObj *obj, int slotNum)
{
    EjsObj      *vp;

    mprAssert(obj);
    mprAssert(obj->slots);
    mprAssert(slotNum >= 0);

    if (slotNum < 0 || slotNum >= obj->numSlots) {
        ejsThrowReferenceError(ejs, "Property at slot \"%d\" is not found", slotNum);
        return 0;
    }
    vp = obj->slots[slotNum].value.ref;
#if ES_Function_prototype
    if (slotNum == ES_Function_prototype && vp == ejs->nullValue) {
        vp = ejsCreateObject(ejs, ejs->objectType, 0);
        vp->isPrototype = 1;
        ejsSetProperty(ejs, obj, ES_Function_prototype, vp);
    }
#endif
    return vp;
}


/*
    Lookup a property with a namespace qualifier in an object and return the slot if found. Return EJS_ERR if not found.
 */
static int lookupFunctionProperty(struct Ejs *ejs, EjsFunction *fun, EjsName *qname)
{
#if UNUSED
    EjsName     name;
    EjsObj      *prototype;
#endif
    int         slotNum;

    slotNum = (ejs->objectType->helpers->lookupProperty)(ejs, (EjsObj*) fun, qname);

#if UNUSED
    if (slotNum < 0 && qname->name[0] == 'p' && strcmp(qname->name, "prototype") == 0 && qname->space[0] == '\0') {
        prototype = ejsCreatePrototype(ejs, type, 0);
        ejsDefineProperty(ejs, (EjsObj*) fun, ES_Object_prototype, ejsName(&name, "", "prototype"),
            ejs->objectType, 0, prototype);
    }
#endif
    return slotNum;
}


void ejsMarkFunction(Ejs *ejs, EjsFunction *fun)
{
    ejsMarkBlock(ejs, (EjsBlock*) fun);
    if (fun->activation) {
        ejsMark(ejs, (EjsObj*) fun->activation);
    }
    if (fun->setter) {
        ejsMark(ejs, (EjsObj*) fun->setter);
    }
    if (fun->creator) {
        ejsMark(ejs, (EjsObj*) fun->creator);
    }
#if UNUSED
    if (fun->owner) {
        ejsMark(ejs, fun->owner);
    }
#endif
    if (fun->thisObj) {
        ejsMark(ejs, fun->thisObj);
    }
    if (fun->resultType) {
        ejsMark(ejs, (EjsObj*) fun->resultType);
    }
}


/*************************************************************************************************************/
/*
    function apply(thisObj: Object, args: Array)
 */
static EjsObj *fun_applyFunction(Ejs *ejs, EjsFunction *fun, int argc, EjsObj **argv)
{
    EjsArray        *args;
    EjsObj          *save, *result, *thisObj;
    
    mprAssert(argc > 1);
    args = (EjsArray*) argv[1];
    mprAssert(ejsIsArray(args));

    save = fun->thisObj;
    thisObj = (argv[0] == ejs->nullValue) ? fun->thisObj: argv[0];
    result =  ejsRunFunction(ejs, fun, thisObj, args->length, args->data);
    fun->thisObj = save;
    return result;
}


/*
    function bind(thisObj: Object, overwrite: Boolean = true): Void
 */
static EjsObj *fun_bindFunction(Ejs *ejs, EjsFunction *fun, int argc, EjsObj **argv)
{
    int     overwrite;
    
    mprAssert(argc >= 1);
    overwrite = (argc < 2 || (argv[1] == (EjsObj*) ejs->trueValue));
    if (overwrite || !fun->thisObj) {
        fun->thisObj = argv[0];
    }
    return 0;
}


/*
    function boundThis(): Function
 */
static EjsObj *fun_boundThis(Ejs *ejs, EjsFunction *fun, int argc, EjsObj **argv)
{
    return fun->thisObj;
}


/*
    function call(thisObj, ...args)
 */
static EjsObj *fun_call(Ejs *ejs, EjsFunction *fun, int argc, EjsObj **argv)
{
    mprAssert(argc > 1);
    return fun_applyFunction(ejs, fun, argc, argv);
}


#if UNUSED
/*
    function get prototype(): Object
 */
static EjsObj *fun_prototype(Ejs *ejs, EjsFunction *fun, int argc, EjsObj **argv)
{
    EjsObj      *prototype;
    EjsTrait    *trait;

    mprAssert(ejsIsFunction(fun));

    if ((prototype = ejsCreateObject(ejs, ejs->objectType, 0)) == 0) {
        return 0;
    }
    prototype->isPrototype = 1;
    ejsSetProperty(ejs, (EjsObj*) fun, ES_Function_prototype, prototype);
    if ((trait = ejsGetTrait((EjsObj*) fun, ES_Function_prototype)) != 0) {
        trait->attributes &= ~(EJS_TRAIT_GETTER);
    }
    return prototype;
}
#endif


/*
    function setScope(obj): Void
 */
static EjsObj *fun_setScope(Ejs *ejs, EjsFunction *fun, int argc, EjsObj **argv)
{
    EjsBlock    *scope;

    scope = (EjsBlock*) argv[0];
    if (!ejsIsBlock(scope)) {
        scope = (EjsBlock*) scope->obj.type;
        if (!ejsIsBlock(scope)) {
            ejsThrowArgError(ejs, "scope object must be a class or function");
            return 0;
        }
    }
    fun->block.scope = scope;
    return 0;
}


/*************************************************************************************************************/
/*
    Create a script function. This defines the method traits. It does not create a  method slot. ResultType may
    be null to indicate untyped. NOTE: untyped functions may return a result at their descretion.
 */
EjsFunction *ejsCreateFunction(Ejs *ejs, cchar *name, cuchar *byteCode, int codeLen, int numArgs, int numDefault, 
    int numExceptions, EjsType *resultType, int attributes, EjsConst *constants, EjsBlock *scopeChain, int strict)
{
    EjsFunction     *fun;
    EjsCode         *code;

    fun = (EjsFunction*) ejsCreate(ejs, ejs->functionType, 0);
    if (fun == 0) {
        return 0;
    }
    fun->name = mprStrdup(fun, name);
    ejsSetDebugName(fun, fun->name);
    if (scopeChain) {
        fun->block.scope = scopeChain;
    }
    fun->numArgs = numArgs;
    fun->numDefault = numDefault;
    fun->resultType = resultType;
    fun->strict = strict;

    //  MOB -- convert these all back to a simple bit mask
    if (attributes & EJS_FUN_CONSTRUCTOR) {
        fun->constructor = 1;
    }
    if (attributes & EJS_FUN_REST_ARGS) {
        fun->rest = 1;
    }
    if (attributes & EJS_FUN_INITIALIZER) {
        fun->isInitializer = 1;
    }
    if (attributes & EJS_PROP_STATIC) {
        fun->staticMethod = 1;
    }
    if (attributes & EJS_PROP_NATIVE) {
        fun->nativeProc = 1;
    }
    if (attributes & EJS_FUN_FULL_SCOPE) {
        fun->fullScope = 1;
    }
    if (attributes & EJS_FUN_HAS_RETURN) {
        fun->hasReturn = 1;
    }
    if (attributes & EJS_TRAIT_CAST_NULLS) {
        fun->castNulls = 1;
    }
    if (attributes & EJS_TRAIT_THROW_NULLS) {
        fun->throwNulls = 1;
    }
    code = &fun->body.code;
    code->codeLen = codeLen;
    code->byteCode = (uchar*) byteCode;
    code->numHandlers = numExceptions;
    code->constants = constants;
    return fun;
}


void ejsSetFunctionName(EjsFunction *fun, cchar *name)
{
    fun->name = name;
}


#if UNUSED
void ejsSetFunctionLocation(EjsFunction *fun, EjsObj *obj, int slotNum)
{
    mprAssert(fun);
    mprAssert(obj);

    fun->owner = obj;
    fun->slotNum = slotNum;
}
#endif


EjsEx *ejsAddException(EjsFunction *fun, uint tryStart, uint tryEnd, EjsType *catchType, uint handlerStart,
        uint handlerEnd, int numBlocks, int numStack, int flags, int preferredIndex)
{
    EjsEx           *exception;
    EjsCode         *code;
    int             size;

    mprAssert(fun);

    code = &fun->body.code;

    exception = mprAllocObjZeroed(fun, EjsEx);
    if (exception == 0) {
        mprAssert(0);
        return 0;
    }
    exception->flags = flags;
    exception->tryStart = tryStart;
    exception->tryEnd = tryEnd;
    exception->catchType = catchType;
    exception->handlerStart = handlerStart;
    exception->handlerEnd = handlerEnd;
    exception->numBlocks = numBlocks;
    exception->numStack = numStack;

    if (preferredIndex < 0) {
        preferredIndex = code->numHandlers++;
    }

    if (preferredIndex >= code->sizeHandlers) {
        size = code->sizeHandlers + EJS_EX_INC;
        code->handlers = (EjsEx**) mprRealloc(fun, code->handlers, size * sizeof(EjsEx));
        if (code->handlers == 0) {
            mprAssert(0);
            return 0;
        }
        memset(&code->handlers[code->sizeHandlers], 0, EJS_EX_INC * sizeof(EjsEx)); 
        code->sizeHandlers = size;
    }
    code->handlers[preferredIndex] = exception;
    return exception;
}


void ejsOffsetExceptions(EjsFunction *fun, int offset)
{
    EjsEx           *ex;
    int             i;

    mprAssert(fun);

    for (i = 0; i < fun->body.code.numHandlers; i++) {
        ex = fun->body.code.handlers[i];
        ex->tryStart += offset;
        ex->tryEnd += offset;
        ex->handlerStart += offset;
        ex->handlerEnd += offset;
    }
}


/*
    Set the byte code for a script function
 */
int ejsSetFunctionCode(EjsFunction *fun, uchar *byteCode, int len)
{
    mprAssert(fun);
    mprAssert(byteCode);
    mprAssert(len >= 0);

    byteCode = (uchar*) mprMemdup(fun, byteCode, len);
    if (byteCode == 0) {
        return EJS_ERR;
    }
    fun->body.code.codeLen = len;
    mprFree(fun->body.code.byteCode);
    fun->body.code.byteCode = (uchar*) byteCode;
    return 0;
}


static EjsObj *nopFunction(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    return ejs->undefinedValue;
}


void ejsCompleteFunction(Ejs *ejs, EjsFunction *fun)
{
    int     numSlots;

    numSlots = fun->block.obj.numSlots;
    if (numSlots > 0 && fun->activation == 0) {
        fun->activation = ejsCreateActivation(ejs, fun, numSlots);
        ejsCopySlots(ejs, (EjsObj*) fun, fun->activation->slots, fun->block.obj.slots, numSlots, 0);
        ejsZeroSlots(ejs, fun->block.obj.slots, numSlots);
        ejsClearObjHash((EjsObj*) fun);
        fun->block.obj.numSlots = 0;
    }
}


void ejsUseActivation(Ejs *ejs, EjsFunction *fun)
{
    EjsObj  *activation;
    int     numSlots;

    if ((activation = fun->activation) == 0) {
        return;
    }
    numSlots = activation->numSlots;
    if (numSlots > 0) {
        ejsGrowObject(ejs, (EjsObj*) fun, numSlots);
        ejsCopySlots(ejs, (EjsObj*) fun, fun->block.obj.slots, fun->activation->slots, numSlots, 0);
        fun->block.obj.numSlots = numSlots;
    }
}


EjsObj *ejsCreateActivation(Ejs *ejs, EjsFunction *fun, int numSlots)
{
    EjsObj  *activation;

    activation = ejsCreateObject(ejs, ejs->objectType, numSlots);
    ejsSetDebugName(activation, ejsGetDebugName(fun));
    return activation;
}


/********************************** Factory **********************************/

void ejsCreateFunctionType(Ejs *ejs)
{
    EjsType         *type;
    EjsTypeHelpers  *helpers;
    EjsFunction     *nop;

    type = ejs->functionType = ejsCreateNativeType(ejs, "ejs", "Function", ES_Function, sizeof(EjsFunction));

    helpers = type->helpers;
    helpers->create         = (EjsCreateHelper) createFunction;
    helpers->cast           = (EjsCastHelper) castFunction;
    helpers->clone          = (EjsCloneHelper) ejsCloneFunction;
    helpers->destroy        = (EjsDestroyHelper) destroyFunction;
    helpers->getProperty    = (EjsGetPropertyHelper) getFunctionProperty;
    helpers->mark           = (EjsMarkHelper) ejsMarkFunction;
    helpers->lookupProperty = (EjsLookupPropertyHelper) lookupFunctionProperty;

    nop = ejs->nopFunction = ejsCreateFunction(ejs, "nop", NULL, 0, -1, 0, 0, NULL, EJS_PROP_NATIVE, NULL, NULL, 0);
    nop->body.proc = nopFunction;
    nop->nativeProc = 1;
}


void ejsConfigureFunctionType(Ejs *ejs)
{
    EjsType     *type;

    type = ejs->functionType;
    ejsBindMethod(ejs, type, ES_Function_apply, (EjsProc) fun_applyFunction);
    ejsBindMethod(ejs, type, ES_Function_bind, (EjsProc) fun_bindFunction);
    ejsBindMethod(ejs, type, ES_Function_boundThis, (EjsProc) fun_boundThis);
#if UNUSED
    ejsBindMethod(ejs, type, ES_Function_prototype, (EjsProc) fun_prototype);
#endif
    ejsBindMethod(ejs, type, ES_Function_setScope, (EjsProc) fun_setScope);
    ejsBindMethod(ejs, type, ES_Function_call, (EjsProc) fun_call);
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
