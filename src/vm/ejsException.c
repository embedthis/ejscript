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


EjsObj *ejsThrowException(Ejs *ejs, EjsObj *obj)
{
    mprAssert(obj);

    ejs->exception = obj;
    ejsAttention(ejs);
    return obj;
}


void ejsClearException(Ejs *ejs)
{
    ejs->exception = 0;
    if (ejs->state->fp) {
        ejs->state->fp->attentionPc = 0;
    }
}


static EjsObj *createException(Ejs *ejs, EjsType *type, cchar* fmt, va_list fmtArgs)
{
    EjsError    *error;
    EjsObj      *argv[1];
    char        *msg;

    mprAssert(type);

    msg = mprVasprintf(ejs, -1, fmt, fmtArgs);
    argv[0] = (EjsObj*) ejsCreateString(ejs, msg);
    if (argv[0] == 0) {
        mprAssert(argv[0]);
        return 0;
    }
    if (!ejs->initialized) {
        if (ejs->empty || ejs->flags & EJS_FLAG_NO_INIT) {
            mprLog(ejs, 5, "Exception: %s", msg);
        } else {
            mprError(ejs, "Exception: %s", msg);
        }
        error = (EjsError*) ejsCreateObject(ejs, type, 0);
        error->message = mprStrdup(error, ejsGetString(ejs, argv[0]));
        ejsFormatStack(ejs, error);
        return (EjsObj*) error;
        
    } else {
        error = (EjsError*) ejsCreateInstance(ejs, type, 1, argv);
    }
    mprFree(msg);
    return (EjsObj*) error;
}


EjsObj *ejsCreateException(Ejs *ejs, int slot, cchar *fmt, va_list fmtArgs)
{
    EjsType     *type;
    EjsObj      *error;
    char        *buf;

    if (ejs->exception) {
        buf = mprVasprintf(ejs, 0, fmt, fmtArgs);
        mprError(ejs, "Double exception: %s", buf);
        mprFree(buf);
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


EjsObj *ejsThrowArgError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_ArgError, fmt, fmtArgs);
}


EjsObj *ejsThrowArithmeticError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_ArithmeticError, fmt, fmtArgs);
}


EjsObj *ejsThrowAssertError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_AssertError, fmt, fmtArgs);
}


EjsObj *ejsThrowInstructionError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_InstructionError, fmt, fmtArgs);
}


EjsObj *ejsThrowError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_Error, fmt, fmtArgs);
}


EjsObj *ejsThrowIOError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_IOError, fmt, fmtArgs);
}


EjsObj *ejsThrowInternalError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_InternalError, fmt, fmtArgs);
}


EjsObj *ejsThrowMemoryError(Ejs *ejs)
{
    /*
        Don't do double exceptions for memory errors
     */
    if (ejs->exception == 0) {
        va_list dummy = NULL_INIT;
        return ejsCreateException(ejs, ES_MemoryError, NULL, dummy);
    }
    return ejs->exception;
}


EjsObj *ejsThrowOutOfBoundsError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_OutOfBoundsError, fmt, fmtArgs);
}


EjsObj *ejsThrowReferenceError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_ReferenceError, fmt, fmtArgs);
}


EjsObj *ejsThrowResourceError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_ResourceError, fmt, fmtArgs);
}


EjsObj *ejsThrowStateError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_StateError, fmt, fmtArgs);
}


EjsObj *ejsThrowSyntaxError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_SyntaxError, fmt, fmtArgs);
}


EjsObj *ejsThrowTypeError(Ejs *ejs, cchar *fmt, ...)
{
    va_list     fmtArgs;

    mprAssert(fmt);
    va_start(fmtArgs, fmt);
    return ejsCreateException(ejs, ES_TypeError, fmt, fmtArgs);
}


/*
    Format the stack backtrace
 */
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


/*
    Public routine to set the error message. Caller MUST NOT free.
 */
char *ejsGetErrorMsg(Ejs *ejs, int withStack)
{
    EjsObj      *message, *stack, *error;
    cchar       *name;
    char        *buf;

#if UNUSED
    if (!ejs->initialized) {
        return "";
    }
#endif
    error = (EjsObj*) ejs->exception;
    message = stack = 0;
    name = 0;

    if (error) {
        name = error->type->qname.name;
        if (ejsIsA(ejs, error, ejs->errorType)) {
            message = ejsGetProperty(ejs, error, ES_Error_message);
            stack = ejsGetProperty(ejs, error, ES_Error_stack);

        } else if (ejsIsString(error)) {
            name = "Error";
            message = error;

        } else if (ejsIsNumber(error)) {
            name = "Error";
            message = error;
            
        } else if (error == (EjsObj*) ejs->stopIterationType) {
            name = "StopIteration";
            message = (EjsObj*) ejsCreateString(ejs, "Uncaught StopIteration exception");
        }
    }
    if (!withStack) {
        stack = 0;
    }

    if (stack && ejsIsString(stack) && message && ejsIsString(message)){
        buf = mprAsprintf(ejs, -1, "%s Exception: %s\nStack:\n%s", name, ((EjsString*) message)->value, 
            ((EjsString*) stack)->value);

    } else if (message && ejsIsString(message)){
        buf = mprAsprintf(ejs, -1, "%s: %s", name, ((EjsString*) message)->value);

    } else if (message && ejsIsNumber(message)){
        buf = mprAsprintf(ejs, -1, "%s: %d", name, ((EjsNumber*) message)->value);
        
    } else {
        if (error) {
            buf = mprStrdup(ejs, "Unknown exception object type");
        } else {
            buf = mprStrdup(ejs, "");
        }
    }
    mprFree(ejs->errorMsg);
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
