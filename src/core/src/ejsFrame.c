/**
    ejsFrame.c - Activation frame class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/******************************************************************************/

static void manageFrame(EjsFrame *frame, int flags)
{
    assert(frame);
    if (frame) {
        if (flags & MPR_MANAGE_MARK) {
            ejsManageFunction((EjsFunction*) frame, flags);
            mprMark(frame->orig);
            mprMark(frame->caller);
            mprMark(TYPE(frame));
        }
    }
}


static EjsFrame *allocFrame(Ejs *ejs, int numProp)
{
    EjsObj      *obj;
    ssize       size;

    assert(ejs);

    size = sizeof(EjsFrame) + sizeof(EjsProperties) + numProp * sizeof(EjsSlot);
    if ((obj = mprAllocBlock(size, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    mprSetManager(obj, (MprManager) manageFrame);
    SET_TYPE(obj, ESV(Frame));
    ejsSetMemRef(obj);
    return (EjsFrame*) obj;
}


/*
    Create a frame object just for the compiler
 */
PUBLIC EjsFrame *ejsCreateCompilerFrame(Ejs *ejs, EjsFunction *fun)
{
    EjsFrame    *fp;

    if ((fp = ejsCreatePot(ejs, ESV(Frame), 0)) == 0) {
        return 0;
    }
    fp->orig = fun;
    fp->function.name = fun->name;
    fp->function.block.pot.isBlock = 1;
    fp->function.block.pot.isFrame = 1;
    fp->function.isConstructor = fun->isConstructor;
    fp->function.isInitializer = fun->isInitializer;
    fp->function.staticMethod = fun->staticMethod;
    mprSetName(fp, "frame");
    return fp;
}


PUBLIC EjsFrame *ejsCreateFrame(Ejs *ejs, EjsFunction *fun, EjsObj *thisObj, int argc, EjsObj **argv)
{
    EjsFrame    *frame;
    EjsPot      *obj, *activation;
    int         numProp, size, i;

    activation = fun->activation;
    numProp = (activation) ? activation->numProp : 0;
    size = max(numProp, EJS_MIN_FRAME_SLOTS);

    frame = allocFrame(ejs, size);
    obj = (EjsPot*) frame;
    obj->properties = (EjsProperties*) &(((char*) obj)[sizeof(EjsFrame)]);
    obj->properties->size = size;
    obj->numProp = numProp;
    if (activation) {
        //  OPT -- could the function be setup as the prototype and thus avoid doing this?
        //  OPT -- assumes that the function is sealed
        memcpy(obj->properties->slots, activation->properties->slots, numProp * sizeof(EjsSlot));
        ejsIndexProperties(ejs, obj);
    }
    ejsZeroSlots(ejs, &obj->properties->slots[numProp], size - numProp);
    //  OPT - should not need to do this
    SET_DYNAMIC(obj, 1);

    frame->orig = fun;
    frame->function.name = fun->name;
    frame->function.block.pot.isBlock = 1;
    frame->function.block.pot.isFrame = 1;
    frame->function.block.namespaces = fun->block.namespaces;
    frame->function.block.scope = fun->block.scope;
    frame->function.block.prev = fun->block.prev;
    frame->function.block.nobind = fun->block.nobind;

    //  OPT
    frame->function.numArgs = fun->numArgs;
    frame->function.numDefault = fun->numDefault;
    frame->function.castNulls = fun->castNulls;
    frame->function.fullScope = fun->fullScope;
    frame->function.hasReturn = fun->hasReturn;
    frame->function.isConstructor = fun->isConstructor;
    frame->function.isInitializer = fun->isInitializer;
    frame->function.isNativeProc = fun->isNativeProc;
    frame->function.rest = fun->rest;
    frame->function.staticMethod = fun->staticMethod;
    frame->function.strict = fun->strict;
    frame->function.throwNulls = fun->throwNulls;

    frame->function.boundArgs = fun->boundArgs;

    /* NOTE: this can be set to ejs->global in frames */
    frame->function.boundThis = thisObj;
    
    frame->function.resultType = fun->resultType;
    frame->function.body = fun->body;
    frame->pc = fun->body.code->byteCode;
    assert(frame->pc);

    if (argc > 0) {
        frame->argc = argc;
        if ((uint) argc < (fun->numArgs - fun->numDefault - fun->rest) || (uint) argc > fun->numArgs) {
            ejsThrowArgError(ejs, "Incorrect number of arguments");
            return 0;
        }
        for (i = 0; i < argc; i++) {
            frame->function.block.pot.properties->slots[i].value.ref = argv[i];
        }
    }
    //  UNICODE
    mprSetName(frame, fun->name->value);
    return frame;
}


PUBLIC void ejsCreateFrameType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsCreateCoreType(ejs, N("ejs", "Frame"), sizeof(EjsFrame), S_Frame, ES_Frame_NUM_CLASS_PROP,
        manageFrame, EJS_TYPE_POT | EJS_TYPE_DYNAMIC_INSTANCES | EJS_TYPE_MUTABLE_INSTANCES);
    type->constructor.block.pot.shortScope = 1;
    type->configured = 1;
    type->helpers.clone = (EjsCloneHelper) ejsCloneBlock;
}


PUBLIC void ejsConfigureFrameType(Ejs *ejs)
{
    EjsType     *type;

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "Frame"))) == 0) {
        return;
    }
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
