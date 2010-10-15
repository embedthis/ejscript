/**
    ejsException.c - Error Exception class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

static uchar trapByteCode[] = { EJS_OP_ATTENTION };

/************************************ Code ************************************/

void ejsAttention(Ejs *ejs)
{
    EjsFrame    *frame;

    frame = ejs->state->fp;
    if (frame && frame->attentionPc == 0 && !frame->ignoreAttention) {
        frame->attentionPc = frame->pc;
        frame->pc = trapByteCode;
    }
}


void ejsClearAttention(Ejs *ejs)
{
    EjsFrame    *frame;

    frame = ejs->state->fp;

    if (ejs->exception == 0 && frame && frame->attentionPc) {
        frame->pc = frame->attentionPc;
        frame->attentionPc = 0;
        mprAssert(frame->pc);
    }
}


EV *ejsThrowException(Ejs *ejs, EV *error)
{
    mprAssert(error);

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


static EV *createException(Ejs *ejs, EjsType *type, cchar* fmt, va_list fmtArgs)
{
    EjsError    *error;
    EV          *argv[1];
    char        *msg;

    mprAssert(type);

    msg = mprVasprintf(ejs, -1, fmt, fmtArgs);
    argv[0] = ejsCreateStringFromCS(ejs, msg);
    if (argv[0] == 0) {
        mprAssert(argv[0]);
        return 0;
    }
#if UNUSED
    if (!ejs->initialized) {
        if (ejs->empty || ejs->flags & EJS_FLAG_NO_INIT) {
            mprLog(ejs, 5, "Exception: %s", msg);
        } else {
            mprError(ejs, "Exception: %s", msg);
        }
        error = ejsCreateError(ejs, type, argv[0]);
#if OLD
        error = (EjsError*) ejsCreateObject(ejs, type, 0);
        error->message = mprStrdup(error, ejsGetString(ejs, argv[0]));
        ejsFormatStack(ejs, error);
#endif
        return error;
        
    } else {
        error = (EjsError*) ejsCreateInstance(ejs, type, 1, argv);
    }
#else
    if (ejs->errorType->constructor.body.proc) {
        error = (EjsError*) ejsCreateInstance(ejs, type, 1, argv);
    } else {
        error = ejsCreateObject(ejs, type, 0);
    }
#endif
    mprFree(msg);
    return error;
}


EV *ejsCreateException(Ejs *ejs, int slot, cchar *fmt, va_list fmtArgs)
{
    EjsType     *type;
    EV          *error;

    if (ejs->exception) {
#if UNUSED
        char        *buf;
        buf = mprVasprintf(ejs, 0, fmt, fmtArgs);
        mprError(ejs, "Double exception: %s", buf);
        mprFree(buf);
#endif
        return ejs->exception;
    }
    if (ejs->initialized) {
        type = ejsGetProperty(ejs, ejs->global, slot);
    } else {
        type = 0;
    }
    if (type == 0) {
        type = ejs->errorType;
    }
    error = createException(ejs, type, fmt, fmtArgs);
    if (error) {
        ejsThrowException(ejs, error);
    }
    return error;
}


EV *ejsThrowArgError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_ArgError, fmt, fmtArgs);
}


EV *ejsThrowArithmeticError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_ArithmeticError, fmt, fmtArgs);
}


EV *ejsThrowAssertError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_AssertError, fmt, fmtArgs);
}


EV *ejsThrowInstructionError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_InstructionError, fmt, fmtArgs);
}


EV *ejsThrowError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_Error, fmt, fmtArgs);
}


EV *ejsThrowIOError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_IOError, fmt, fmtArgs);
}


EV *ejsThrowInternalError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_InternalError, fmt, fmtArgs);
}


EV *ejsThrowMemoryError(Ejs *ejs)
{
    /*
        Don't do double exceptions for memory errors
     */
    if (ejs->exception == 0) {
        va_list dummy = NULL_INIT;
        return ejsCreateException(ejs, ES_MemoryError, "Memory Error", dummy);
    }
    return ejs->exception;
}


EV *ejsThrowOutOfBoundsError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_OutOfBoundsError, fmt, fmtArgs);
}


EV *ejsThrowReferenceError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_ReferenceError, fmt, fmtArgs);
}


EV *ejsThrowResourceError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_ResourceError, fmt, fmtArgs);
}


EV *ejsThrowStateError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_StateError, fmt, fmtArgs);
}


EV *ejsThrowSyntaxError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_SyntaxError, fmt, fmtArgs);
}


EV *ejsThrowTypeError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_TypeError, fmt, fmtArgs);
}


EjsArray *ejsCaptureStack(Ejs *ejs, int uplevels)
{
    EjsFrame        *fp;
    EjsState        *state;
    EjsArray        *stack;
    EV              *frame;
    EjsString       *line;
    int             index;

    mprAssert(ejs);

    stack = ejsCreateArray(ejs, 0);
    index = 0;
    for (state = ejs->state; state; state = state->prev) {
        for (fp = state->fp; fp; fp = fp->caller) {
            if (uplevels-- <= 0) {
                if (fp->currentLine == 0) {
                    line = ejs->emptyString;
                }
                frame = ejsCreateSimpleObject(ejs);
                if (fp->filename) {
                    ejsSetPropertyByName(ejs, frame, N("", "filename"), ejsCreatePath(ejs, fp->filename));
                } else {
                    ejsSetPropertyByName(ejs, frame, N("", "filename"), ejs->undefinedValue);
                }
                ejsSetPropertyByName(ejs, frame, N("", "lineno"), ejsCreateNumber(ejs, fp->lineNumber));
                ejsSetPropertyByName(ejs, frame, N("", "func"), fp->function.name);
                ejsSetPropertyByName(ejs, frame, N("", "code"), line);
                ejsSetProperty(ejs, stack, index++, frame);
            }
        }
    }
    return stack;
}


#if UNUSED
char *ejsFormatStack(Ejs *ejs, EjsError *error)
{
    EjsFrame        *fp;
    EjsState        *state;
    cchar           *line, *codeSep;
    char            *backtrace, *traceLine;
    int             level, len;

    mprAssert(ejs);

    backtrace = 0;
    len = 0;
    level = 0;

    for (state = ejs->state; state; state = state->prev) {
        for (fp = state->fp; fp; fp = fp->caller) {
            if (fp->currentLine == 0) {
                line = "";
            } else {
                for (line = fp->currentLine; *line && isspace((int) *line); line++) {
                    ;
                }
            }
            codeSep = (*line) ? "->" : "";

            if (error && backtrace == 0) {
                error->filename = mprStrdup(error, fp->filename);
                error->lineNumber = fp->lineNumber;
            }
            if ((traceLine = mprAsprintf(ejs, MPR_MAX_STRING, " [%02d] %s, %s, line %d %s %s\n",
                    level++, fp->filename ? fp->filename : "script", fp->function.name,
                    fp->lineNumber, codeSep, line)) == NULL) {
                break;
            }
            backtrace = (char*) mprRealloc(ejs, backtrace, len + (int) strlen(traceLine) + 1);
            if (backtrace == 0) {
                return 0;
            }
            memcpy(&backtrace[len], traceLine, strlen(traceLine) + 1);
            len += (int) strlen(traceLine);
            mprFree(traceLine);
        }
    }
    if (error) {
        error->stack = backtrace;
    }
    return backtrace;
}
#endif


/*
    Get the current exception error. May be an Error object or may be any other object that is thrown.
    Caller must NOT free.
 */
cchar *ejsGetErrorMsg(Ejs *ejs, int withStack)
{
    EjsString   *str, *tag, *msg;
    EV          *message, *stack, *error, *saveException;
    char        *buf;

    error = ejs->exception;
    message = stack = 0;
    tag = NULL;

    if (error) {
        tag = TYPE(error)->qname.name;
        if (ejsIsA(ejs, error, ejs->errorType)) {
            message = ejsGetProperty(ejs, error, ES_Error_message);
            if (withStack) {
                saveException = ejs->exception;
                ejsClearException(ejs);
                stack = ejsRunFunctionBySlot(ejs, error, ES_Error_formatStack, 0, NULL);
                ejs->exception = saveException;
            }

        } else if (ejsIsString(ejs, error)) {
            tag = ejsCreateStringFromCS(ejs, "Error");
            message = error;

        } else if (ejsIsNumber(ejs, error)) {
            tag = ejsCreateStringFromCS(ejs, "Error");
            message = error;
            
        } else if (error == ejs->stopIterationType) {
            message = ejsCreateStringFromCS(ejs, "Uncaught StopIteration exception");
        }
    }
    if (message == ejs->nullValue || message == 0) {
        msg = ejsCreateStringFromCS(ejs, "Exception");
    } else{
        msg = ejsToString(ejs, message);
    }
    if (ejsIsA(ejs, error, ejs->errorType)) {
        buf = mprAsprintf(ejs, -1, "%S: %S\nStack:\n%s", tag, msg, (stack) ? ejsGetString(ejs, stack) : "");

    } else if (message && ejsIsString(ejs, message)){
        buf = mprAsprintf(ejs, -1, "%S: %S", tag, msg);

    } else if (message && ejsIsNumber(ejs, message)){
        buf = mprAsprintf(ejs, -1, "%S: %g", tag, msg);
        
    } else if (error) {
        EV *saveException = ejs->exception;
        ejs->exception = 0;
        str = ejsToString(ejs, error);
        buf = mprStrdup(ejs, ejsGetString(ejs, str));
        ejs->exception = saveException;

    } else {
        buf = mprStrdup(ejs, "");
    }
    mprFree(ejs->errorMsg);
    ejs->errorMsg = buf;
    return buf;
}


bool ejsHasException(Ejs *ejs)
{
    return ejs->exception != 0;
}


EV *ejsGetException(Ejs *ejs)
{
    return ejs->exception;
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
