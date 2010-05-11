/**
    ejsFrame.c - Activation frame class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/******************************************************************************/

static void destroyFrame(Ejs *ejs, EjsFrame *frame)
{
    //  MOB -- does this now mean that the last arg can be deleted?
    ejsFree(ejs, (EjsObj*) frame, ES_Frame);
}


static void markFrame(Ejs *ejs, EjsFrame *frame)
{
    ejsMarkFunction(ejs, (EjsFunction*) frame);
    if (frame->caller) {
        ejsMark(ejs, (EjsObj*) frame->caller);
    }
    /* Marking the stack is done in ejsGarbage.c:mark() */
}


/*************************************************************************************************************/

static EjsFrame *allocFrame(Ejs *ejs, int numSlots)
{
    EjsObj      *obj;
    uint        size;

    mprAssert(ejs);

    size = numSlots * sizeof(EjsSlot) + sizeof(EjsFrame);
    if ((obj = (EjsObj*) mprAllocZeroed(ejsGetAllocCtx(ejs), size)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    obj->type = ejs->frameType;
    obj->master = (ejs->master == 0);
    ejsAddToGcStats(ejs, obj, ES_Frame);
    return (EjsFrame*) obj;
}


EjsFrame *ejsCreateFrame(Ejs *ejs, EjsFunction *fun, EjsObj *thisObj, int argc, EjsObj **argv)
{
    EjsFrame    *frame;
    EjsObj      *obj, *activation;
    int         numSlots, sizeSlots, i;

    activation = fun->activation;
    numSlots = (activation) ? activation->numSlots : 0;
    sizeSlots = max(numSlots, EJS_MIN_FRAME_SLOTS);

    if (sizeSlots > EJS_MIN_FRAME_SLOTS || (frame = (EjsFrame*) ejsAllocPooled(ejs, ES_Frame)) == 0) {
        frame = allocFrame(ejs, sizeSlots);
    }
    obj = (EjsObj*) frame;
    obj->slots = (EjsSlot*) &(((char*) obj)[sizeof(EjsFrame)]);
    obj->sizeSlots = sizeSlots;
    obj->numSlots = numSlots;
    if (activation) {
        //  MOB -- could the function be setup as the prototype and thus avoid doing this?
        //  MOB -- assumes that the function is sealed
        memcpy(obj->slots, activation->slots, numSlots * sizeof(EjsSlot));
        ejsMakeObjHash(obj);
    }
    ejsZeroSlots(ejs, &obj->slots[numSlots], sizeSlots - numSlots);
    obj->dynamic = 1;

    frame->function.name = fun->name;
    frame->function.block.obj.isFrame = 1;
    frame->function.block.namespaces = fun->block.namespaces;
    frame->function.block.scope = fun->block.scope;
    frame->function.block.prev = fun->block.prev;
    frame->function.block.breakCatch = fun->block.breakCatch;
    frame->function.block.nobind = fun->block.nobind;

#if BLD_HAS_UNNAMED_UNIONS
    frame->function.bits = fun->bits;
#else
    frame->function.numArgs = fun->numArgs;
    frame->function.numDefault = fun->numDefault;
    frame->function.castNulls = fun->castNulls;
    frame->function.constructor = fun->constructor;
    frame->function.fullScope = fun->fullScope;
    frame->function.hasReturn = fun->hasReturn;
    frame->function.isInitializer = fun->isInitializer;
    frame->function.nativeProc = fun->nativeProc;
    frame->function.override = fun->override;
    frame->function.rest = fun->rest;
    frame->function.staticMethod = fun->staticMethod;
    frame->function.strict = fun->strict;
    frame->function.throwNulls = fun->throwNulls;
#endif
    frame->function.thisObj = thisObj;
    frame->function.resultType = fun->resultType;
    frame->function.body = fun->body;
    frame->pc = fun->body.code.byteCode;

    if (argc > 0) {
        frame->argc = argc;
        if ((uint) argc < (fun->numArgs - fun->numDefault) || (uint) argc > fun->numArgs) {
            ejsThrowArgError(ejs, "Incorrect number of arguments");
            return 0;
        }
        for (i = 0; i < argc; i++) {
            frame->function.block.obj.slots[i].value.ref = argv[i];
        }
    }
    ejsSetDebugName(frame, ejsGetDebugName(fun));
    return frame;
}


void ejsCreateFrameType(Ejs *ejs)
{
    EjsType         *type;
    EjsTypeHelpers  *helpers;

    type = ejs->frameType = ejsCreateNativeType(ejs, "ejs", "Frame", ES_Frame, sizeof(EjsFrame));
    type->orphan = 1;
    type->block.obj.shortScope = 1;

    helpers = &type->helpers;
    helpers->destroy = (EjsDestroyHelper) destroyFrame;
    helpers->mark    = (EjsMarkHelper) markFrame;
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
