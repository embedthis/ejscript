/**
    ejsFrame.c - Activation frame class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/******************************************************************************/

static void manageFrame(EjsFrame *frame, int flags)
{
    if (frame) {
        if (flags & MPR_MANAGE_MARK) {
            ejsManageFunction((EjsFunction*) frame, flags);
            mprMark(frame->orig);
            mprMark(frame->caller);
            mprMark(((EjsObj*) frame)->type);
            /* Marking the stack is done in ejsGarbage.c:mark() */
#if BLD_DEBUG
            mprMark(frame->loc.source);
            mprMark(frame->loc.filename);
#endif
        }
    }
}


static EjsFrame *allocFrame(Ejs *ejs, int numProp)
{
    EjsObj      *obj;
    uint        size;

    mprAssert(ejs);

    size = sizeof(EjsFrame) + sizeof(EjsProperties) + numProp * sizeof(EjsSlot);
    if ((obj = mprAllocBlock(size, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    mprSetManager(obj, manageFrame);
    TYPE(obj) = ejs->frameType;
    return (EjsFrame*) obj;
}


/*
    Create a frame object just for the compiler
 */
EjsFrame *ejsCreateCompilerFrame(Ejs *ejs, EjsFunction *fun)
{
    EjsFrame    *fp;

    fp = (EjsFrame*) ejsCreate(ejs, ejs->frameType, 0);
    if (fp == 0) {
        return 0;
    }
    fp->orig = fun;
    fp->function.name = fun->name;
    fp->function.block.pot.isBlock = 1;
    fp->function.block.pot.isFrame = 1;
    fp->function.isConstructor = fun->isConstructor;
    fp->function.isInitializer = fun->isInitializer;
    fp->function.staticMethod = fun->staticMethod;
    ejsSetName(fp, MPR_NAME("frame"));
    return fp;
}


EjsFrame *ejsCreateFrame(Ejs *ejs, EjsFunction *fun, EjsObj *thisObj, int argc, EjsObj **argv)
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
        //  MOB -- could the function be setup as the prototype and thus avoid doing this?
        //  MOB -- assumes that the function is sealed
        memcpy(obj->properties->slots, activation->properties->slots, numProp * sizeof(EjsSlot));
        ejsMakeHash(ejs, obj);
    }
    ejsZeroSlots(ejs, &obj->properties->slots[numProp], size - numProp);
    DYNAMIC(obj) = 1;

    frame->orig = fun;
    frame->function.name = fun->name;
    frame->function.block.pot.isBlock = 1;
    frame->function.block.pot.isFrame = 1;
    frame->function.block.namespaces = fun->block.namespaces;
    frame->function.block.scope = fun->block.scope;
    frame->function.block.prev = fun->block.prev;
    frame->function.block.breakCatch = fun->block.breakCatch;
    frame->function.block.nobind = fun->block.nobind;

    //  MOB -- check these
    //  MOB - OPT
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
    frame->function.boundThis = fun->boundThis;
    if (thisObj) {
        frame->function.boundThis = thisObj;
    }
    frame->function.resultType = fun->resultType;
    frame->function.body = fun->body;
    frame->pc = fun->body.code->byteCode;
    mprAssert(frame->pc);

    if (argc > 0) {
        frame->argc = argc;
        if ((uint) argc < (fun->numArgs - fun->numDefault) || (uint) argc > fun->numArgs) {
            ejsThrowArgError(ejs, "Incorrect number of arguments");
            return 0;
        }
        for (i = 0; i < argc; i++) {
            frame->function.block.pot.properties->slots[i].value.ref = argv[i];
        }
    }
    ejsCopyName(frame, fun);
#if BLD_DEBUG
    frame->function.block.pot.mem = MPR_GET_MEM(frame);
#endif
    return frame;
}


void ejsCreateFrameType(Ejs *ejs)
{
    EjsType     *type;

    type = ejs->frameType = ejsCreateNativeType(ejs, N("ejs", "Frame"), ES_Frame, sizeof(EjsFrame), manageFrame, 
        EJS_POT_HELPERS);
    type->constructor.block.pot.shortScope = 1;
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
