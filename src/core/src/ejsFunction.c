/**
    ejsFunction.c - Function class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Forwards *********************************/

static void setFunctionAttributes(EjsFunction *fun, int attributes);

/************************************* Code ***********************************/
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
    dest->numArgs = src->numArgs;
    dest->numDefault = src->numDefault;

    /*
        OPT
     */
    dest->staticMethod = src->staticMethod;
    dest->hasReturn = src->hasReturn;
    dest->isConstructor = src->isConstructor;
    dest->isInitializer = src->isInitializer;
    dest->isNativeProc = src->isNativeProc;
    dest->moduleInitializer = src->moduleInitializer;
    dest->rest = src->rest;
    dest->fullScope = src->fullScope;
    dest->strict = src->strict;
    dest->name = src->name;

    if (src->activation) {
        dest->activation = ejsCloneObject(ejs, src->activation, 0);
    }
    ejsSetDebugName(dest, ejsGetDebugName(src));
    return dest;
}


static void destroyFunction(Ejs *ejs, EjsFunction *fun)
{
    ejsFreeVar(ejs, (EjsObj*) fun, ES_Function);
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
    if (fun->archetype) {
        ejsMark(ejs, (EjsObj*) fun->archetype);
    }
    if (fun->thisObj) {
        ejsMark(ejs, fun->thisObj);
    }
    if (fun->resultType) {
        ejsMark(ejs, (EjsObj*) fun->resultType);
    }
}


/*************************************************************************************************************/
/*
    function Function(...[args], body)
 */
static EjsObj *fun_Function(Ejs *ejs, EjsFunction *fun, int argc, EjsObj **argv)
{
#if UNUSED
    EjsArray        *args;
    EjsString       *str;
    MprBuf          *buf;
    cchar           *body, *param, *script;
    int             i, count;
    
    mprAssert(argc > 1);
    args = (EjsArray*) argv[1];
    mprAssert(ejsIsArray(args));

    if (args->length <= 0) {
        ejsThrowArgError(ejs, "Missing function body");
        return 0;
    }
    str = ejsToString(ejs, args->data[args->length - 1]);
    body = ejsGetString(ejs, str);

    buf = mprCreateBuf(ejs, -1, -1);
    mprPutStringToBuf(buf, "function(");
    count = args->length - 1;
    for (i = 0; i < count; i++) {
        str = ejsToString(ejs, args->data[i]);
        param = ejsGetString(ejs, str);
        mprPutStringToBuf(buf, param);
        if (i < (count - 1)) {
            mprPutCharToBuf(buf, ',');
        }
        mprPutStringToBuf(buf, "\n{");
    }
    mprPutStringToBuf(buf, body);
    mprPutStringToBuf(buf, "\n}");

    script = mprGetBufStart(buf);
    if (ejsLoadScriptLiteral(ejs, script, NULL, EC_FLAGS_NO_OUT | EC_FLAGS_DEBUG | EC_FLAGS_THROW | EC_FLAGS_VISIBLE) < 0) {
        //  MOB -- what happens to compiler errors
        return 0;
    }
    fun->body.code = ;
    fun->body.codeLen
#endif
    return (EjsObj*) fun;
}

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


/*
    Return the number of required args.

    function get length(): Number
 */
static EjsObj *fun_length(Ejs *ejs, EjsFunction *fun, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, fun->numArgs);
}


#if ES_Function_name
/*
    function get name(): Number
 */
static EjsObj *fun_name(Ejs *ejs, EjsFunction *fun, int argc, EjsObj **argv)
{
    if (fun->name && *fun->name == '-') {
        return (EjsObj*) ejs->emptyStringValue;
    }
    return (EjsObj*) ejsCreateString(ejs, fun->name);
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
            ejsThrowArgError(ejs, "Scope object must be a class or function");
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
    int numExceptions, EjsType *resultType, int attributes, EjsConst *constants, EjsBlock *scope, int strict)
{
    EjsFunction     *fun;

    if ((fun = ejsCreateSimpleFunction(ejs, name, attributes)) == 0) {
        return 0;
    }
    ejsInitFunction(ejs, fun, name, byteCode, codeLen, numArgs, numDefault, numExceptions, resultType, 
        attributes, constants, scope, strict);
    return fun;
}


void ejsInitFunction(Ejs *ejs, EjsFunction *fun, cchar *name, cuchar *byteCode, int codeLen, int numArgs, int numDefault, 
    int numExceptions, EjsType *resultType, int attributes, EjsConst *constants, EjsBlock *scope, int strict)
{
    EjsCode         *code;

    if (scope) {
        fun->block.scope = scope;
    }
    fun->block.obj.isFunction = 1;
    fun->numArgs = numArgs;
    fun->numDefault = numDefault;
    fun->resultType = resultType;
    fun->strict = strict;
    code = &fun->body.code;
    code->codeLen = codeLen;
    code->byteCode = (uchar*) byteCode;
    code->numHandlers = numExceptions;
    code->constants = constants;
    setFunctionAttributes(fun, attributes);
}


void ejsDisableFunction(Ejs *ejs, EjsFunction *fun)
{
    fun->block.obj.isFunction = 0;
    fun->isConstructor = 0;
    fun->isInitializer = 0;
    fun->activation = 0;
}


EjsFunction *ejsCreateSimpleFunction(Ejs *ejs, cchar *name, int attributes)
{
    EjsFunction     *fun;

    fun = (EjsFunction*) ejsCreate(ejs, ejs->functionType, 0);
    if (fun == 0) {
        return 0;
    }
    fun->name = mprStrdup(fun, name);
    ejsSetDebugName(fun, fun->name);
    setFunctionAttributes(fun, attributes);
    return fun;
}


static void setFunctionAttributes(EjsFunction *fun, int attributes)
{
    if (attributes & EJS_FUN_CONSTRUCTOR) {
        fun->isConstructor = 1;
    }
    if (attributes & EJS_FUN_INITIALIZER) {
        fun->isInitializer = 1;
    }
    if (attributes & EJS_PROP_NATIVE) {
        fun->isNativeProc = 1;
    }
    if (attributes & EJS_FUN_MODULE_INITIALIZER) {
        fun->moduleInitializer = 1;
    }
    if (attributes & EJS_FUN_REST_ARGS) {
        fun->rest = 1;
    }
    if (attributes & EJS_PROP_STATIC) {
        fun->staticMethod = 1;
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
}


void ejsSetFunctionName(EjsFunction *fun, cchar *name)
{
    fun->name = name;
}


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


//  MOB -- who calls this?
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

    helpers = &type->helpers;
    helpers->create         = (EjsCreateHelper) createFunction;
    helpers->cast           = (EjsCastHelper) castFunction;
    helpers->clone          = (EjsCloneHelper) ejsCloneFunction;
    helpers->destroy        = (EjsDestroyHelper) destroyFunction;
    helpers->mark           = (EjsMarkHelper) ejsMarkFunction;

    nop = ejs->nopFunction = ejsCreateFunction(ejs, "nop", NULL, 0, -1, 0, 0, NULL, EJS_PROP_NATIVE, NULL, NULL, 0);
    nop->body.proc = nopFunction;
    nop->isNativeProc = 1;
}


void ejsConfigureFunctionType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *prototype;

    type = ejs->functionType;
    prototype = type->prototype;

    ejsBindConstructor(ejs, type, (EjsProc) fun_Function);
    ejsBindMethod(ejs, prototype, ES_Function_apply, (EjsProc) fun_applyFunction);
    ejsBindMethod(ejs, prototype, ES_Function_bind, (EjsProc) fun_bindFunction);
    ejsBindMethod(ejs, prototype, ES_Function_boundThis, (EjsProc) fun_boundThis);
    ejsBindMethod(ejs, prototype, ES_Function_call, (EjsProc) fun_call);
    ejsBindMethod(ejs, prototype, ES_Function_length, (EjsProc) fun_length);
#if ES_Function_name
    ejsBindMethod(ejs, prototype, ES_Function_name, (EjsProc) fun_name);
#endif
    ejsBindMethod(ejs, prototype, ES_Function_setScope, (EjsProc) fun_setScope);
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
