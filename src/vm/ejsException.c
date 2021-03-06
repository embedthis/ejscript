/**
    ejsException.c - Error Exception class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

static uchar trapByteCode[] = { EJS_OP_ATTENTION };

/************************************ Code ************************************/

static EjsAny *createException(Ejs *ejs, EjsType *type, cchar* fmt, va_list fmtArgs)
{
    EjsError    *error;
    EjsAny      *argv[1];
    char        *msg;

    assert(type);
    
#if ME_DEBUG
    /* Breakpoint opportunity */
    if (!ejs->empty) {
        mprNop(0);
    }
#endif
    msg = sfmtv(fmt, fmtArgs);
    argv[0] = ejsCreateStringFromAsc(ejs, msg);
    if (argv[0] == 0) {
        assert(argv[0]);
        return 0;
    }
    if (EST(Error)->constructor.body.proc) {
        error = (EjsError*) ejsCreateInstance(ejs, type, 1, argv);
    } else {
        error = ejsCreatePot(ejs, type, 0);
        ejsSetProperty(ejs, error, ES_Error_message, ejsCreateStringFromAsc(ejs, msg));
    }
    return error;
}


/*
    Redirect the VM to the ATTENTION op code
 */
void ejsAttention(Ejs *ejs)
{
    EjsFrame    *frame;
    uchar       *pc;

    frame = ejs->state->fp;
    if (frame && frame->attentionPc == 0) {
        /*
            Order matters. Setting the pc to the trap byte code will redirect the VM to the ATTENTION op code which
            will call mprLock(ejs->mutex) preventing a race here.
         */
        pc = frame->pc;
        frame->pc = trapByteCode;
        frame->attentionPc = pc;
    }
}


void ejsClearAttention(Ejs *ejs)
{
    EjsFrame    *frame;

    frame = ejs->state->fp;

    if (ejs->exception == 0 && frame && frame->attentionPc) {
        frame->pc = frame->attentionPc;
        frame->attentionPc = 0;
        assert(frame->pc);
    }
}


EjsAny *ejsThrowException(Ejs *ejs, EjsAny *error)
{
    assert(error);

    ejs->exception = error;
    ejsAttention(ejs);
    return error;
}


void ejsClearException(Ejs *ejs)
{
    ejs->exception = 0;
    if (ejs->state->fp) {
        ejsClearAttention(ejs);
    }
}


EjsAny *ejsCreateException(Ejs *ejs, int slot, cchar *fmt, va_list fmtArgs)
{
    EjsType     *type;
    EjsAny      *error;

    if (ejs->exception) {
        mprLog("ejs vm", 0, "Double exception: %s", sfmtv(fmt, fmtArgs));
        return ejs->exception;
    }
    type = (ejs->initialized) ? ejsGetProperty(ejs, ejs->global, slot) : NULL;
    if (type == 0) {
        type = EST(Error);
    }
    error = createException(ejs, type, fmt, fmtArgs);
    if (error) {
        ejsThrowException(ejs, error);
    }
    return error;
}


EjsError *ejsThrowArgError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_ArgError, fmt, fmtArgs);
}


EjsError *ejsThrowArithmeticError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_ArithmeticError, fmt, fmtArgs);
}


EjsError *ejsThrowAssertError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_AssertError, fmt, fmtArgs);
}


EjsError *ejsThrowInstructionError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_InstructionError, fmt, fmtArgs);
}


EjsError *ejsThrowError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_Error, fmt, fmtArgs);
}


EjsError *ejsThrowIOError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_IOError, fmt, fmtArgs);
}


EjsError *ejsThrowInternalError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_InternalError, fmt, fmtArgs);
}


EjsError *ejsThrowMemoryError(Ejs *ejs)
{
    /*
        Don't do double exceptions for memory errors
     */
    if (ejs->exception == 0) {
        static va_list dummy;
        return ejsCreateException(ejs, ES_MemoryError, "Memory Error", dummy);
    }
    return (EjsError*) ejs->exception;
}


EjsError *ejsThrowOutOfBoundsError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_OutOfBoundsError, fmt, fmtArgs);
}


EjsError *ejsThrowReferenceError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_ReferenceError, fmt, fmtArgs);
}


EjsError *ejsThrowResourceError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_ResourceError, fmt, fmtArgs);
}


EjsString *ejsThrowString(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;
    char        *msg;

    assert(fmt);
    va_start(fmtArgs, fmt);
    msg = sfmtv(fmt, fmtArgs);
    va_end(fmtArgs);

    /*
        Throwing a string will not create a stack frame
     */
    ejs->exception = ejsCreateStringFromAsc(ejs, msg);
    ejsAttention(ejs);
    return ejs->exception;
}


EjsError *ejsThrowStateError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_StateError, fmt, fmtArgs);
}


EjsError *ejsThrowSyntaxError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_SyntaxError, fmt, fmtArgs);
}


EjsError *ejsThrowTypeError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    assert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_TypeError, fmt, fmtArgs);
}


EjsArray *ejsCaptureStack(Ejs *ejs, int uplevels)
{
    EjsFrame        *fp;
    EjsState        *state;
    EjsArray        *stack;
    wchar           *source;
    EjsObj          *frame;
    char            *filename;
    int             index, lineNumber;

    assert(ejs);

    stack = ejsCreateArray(ejs, 0);
    index = 0;
    for (state = ejs->state; state; state = state->prev) {
        for (fp = state->fp; fp; fp = fp->caller) {
            if (uplevels-- <= 0) {
                frame = ejsCreateEmptyPot(ejs);
                if (ejsGetDebugInfo(ejs, (EjsFunction*) fp, fp->pc, &filename, &lineNumber, &source) >= 0) {
                    ejsSetPropertyByName(ejs, frame, EN("filename"), ejsCreatePathFromAsc(ejs, filename));
                    ejsSetPropertyByName(ejs, frame, EN("lineno"), ejsCreateNumber(ejs, lineNumber));
                    ejsSetPropertyByName(ejs, frame, EN("code"), ejsCreateString(ejs, source, wlen(source)));
                } else {
                    ejsSetPropertyByName(ejs, frame, EN("filename"), EST(undefined));
                }
                ejsSetPropertyByName(ejs, frame, EN("func"), fp->function.name);
                ejsSetProperty(ejs, stack, index++, frame);
            }
        }
    }
    return stack;
}


/*
    Get the current exception error. May be an Error object or may be any other object that is thrown.
    Caller must NOT free.
 */
cchar *ejsGetErrorMsg(Ejs *ejs, int withStack)
{
    EjsString   *str, *tag, *msg, *message;
    EjsAny      *stack, *error, *saveException;
    char        *buf, *stackStr;

    error = ejs->exception;
    message = 0;
    stack = 0;
    tag = NULL;

    if (error) {
        tag = TYPE(error)->qname.name;
        if (ejsIs(ejs, error, Error)) {
            message = ejsGetProperty(ejs, error, ES_Error_message);
            if (withStack && ejs->initialized && ejs->state) {
                saveException = ejs->exception;
                ejsClearException(ejs);
                stack = ejsRunFunctionBySlot(ejs, error, ES_Error_formatStack, 0, NULL);
                ejsThrowException(ejs, saveException);
            }

        } else if (ejsIs(ejs, error, String)) {
            tag = ejsCreateStringFromAsc(ejs, "Error");
            message = (EjsString*) error;

        } else if (ejsIs(ejs, error, Number)) {
            tag = ejsCreateStringFromAsc(ejs, "Error");
            message = (EjsString*) error;
            
        } else if (error == EST(StopIteration)) {
            message = ejsCreateStringFromAsc(ejs, "Uncaught StopIteration exception");
        }
    }
    if (message == ESV(null) || message == 0) {
        msg = ejsCreateStringFromAsc(ejs, "Exception");
    } else{
        msg = ejsToString(ejs, message);
    }
    if (ejsIs(ejs, error, Error)) {
        stackStr = (stack) ? ejsToMulti(ejs, stack) : 0;
        if (stackStr && *stackStr) {
            buf = sfmt("%@: %@\nStack:\n%s", tag, msg, (stack) ? ejsToMulti(ejs, stack) : "");
        } else {
            buf = sfmt("%@: %@", tag, msg);
        }

    } else if (message && ejsIs(ejs, message, String)){
        buf = sfmt("%@: %@", tag, msg);

    } else if (message && ejsIs(ejs, message, Number)){
        buf = sfmt("%@: %g", tag, ((EjsNumber*) msg)->value);
        
    } else if (error) {
        EjsObj *saveException = ejs->exception;
        ejs->exception = 0;
        str = ejsToString(ejs, error);
        buf = sclone(ejsToMulti(ejs, str));
        ejs->exception = saveException;

    } else {
        buf = sclone("");
    }
    ejs->errorMsg = buf;
    return buf;
}


bool ejsHasException(Ejs *ejs)
{
    return ejs->exception != 0;
}


EjsObj *ejsGetException(Ejs *ejs)
{
    return ejs->exception;
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
