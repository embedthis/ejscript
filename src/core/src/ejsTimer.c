/*
    ejsTimer.c -- Timer class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Code ************************************/
/*
    Create a new timer

    function Timer(period: Number, callback: Function, ...args)
 */
static EjsTimer *timer_constructor(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc >= 2);
    mprAssert(ejsIs(ejs, argv[0], Number));
    mprAssert(ejsIsFunction(ejs, argv[1]));
    mprAssert(ejsIs(ejs, argv[2], Array));

    tp->period = ejsGetInt(ejs, argv[0]);
    tp->callback = (EjsFunction*) argv[1];
    tp->args = (EjsArray*) argv[2];
    tp->repeat = 0;
    tp->drift = 1;
    tp->ejs = ejs;
    return tp;
}


/*
    function get drift(): Boolean
 */
static EjsBoolean *timer_get_drift(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 0);
    return ejsCreateBoolean(ejs, tp->drift);
}


/*
    function set drift(period: Boolean): Void
 */
static EjsObj *timer_set_drift(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Boolean));
    tp->drift = ejsGetBoolean(ejs, argv[0]);
    return 0;
}


/*
    function get onerror(): Function
 */
static EjsFunction *timer_get_onerror(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 0);
    return tp->onerror;
}


/*
    function set onerror(callback: Function): Void
 */
static EjsObj *timer_set_onerror(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    tp->onerror = (EjsFunction*) argv[0];
    return 0;
}


/*
    function get period(): Number
 */
static EjsNumber *timer_get_period(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 0);
    return ejsCreateNumber(ejs, tp->period);
}


/*
    function set period(period: Number): Void
 */
static EjsObj *timer_set_period(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Number));

    tp->period = ejsGetInt(ejs, argv[0]);
    return 0;
}


/*
    function get repeat(): Boolean
 */
static EjsBoolean *timer_get_repeat(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 0);
    return ejsCreateBoolean(ejs, tp->repeat);
}


/*
    function set repeat(enable: Boolean): Void
 */
static EjsObj *timer_set_repeat(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Boolean));

    tp->repeat = ejsGetBoolean(ejs, argv[0]);
    if (tp->event) {
        mprEnableContinuousEvent(tp->event, tp->repeat);
    }
    return 0;
}


static int timerCallback(EjsTimer *tp, MprEvent *e)
{
    Ejs         *ejs;
    EjsObj      *thisObj, *error;

    mprAssert(tp);
    mprAssert(tp->args);
    mprAssert(tp->callback);

    ejs = tp->ejs;
    thisObj = (tp->callback->boundThis) ? tp->callback->boundThis : tp;
    ejsRunFunction(ejs, tp->callback, thisObj, tp->args->length, tp->args->data);
    if (ejs->exception) {
        if (tp->onerror) {
            error = ejs->exception;
            ejsClearException(ejs);
            ejsRunFunction(ejs, tp->onerror, thisObj, 1, &error);
        } else {
            mprError("Uncaught exception in timer\n%s", ejsGetErrorMsg(ejs, 1));
            ejsClearException(ejs);
        }
    }
    if (!tp->repeat) {
        mprRemoveRoot(tp);
        tp->event = 0;
        tp->ejs = 0;
    }
    return 0;
}


/*
    function start(): Timer
 */
static EjsTimer *timer_start(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    int     flags;

    if (tp->event == 0) {
        flags = tp->repeat ? MPR_EVENT_CONTINUOUS : 0;
        tp->event = mprCreateEvent(ejs->dispatcher, "timer", tp->period, (MprEventProc) timerCallback, tp, flags);
        if (tp->event == 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
    }
    return tp;
}


/*
    function stop(): Void
 */
static EjsObj *timer_stop(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    if (tp->event) {
        mprRemoveEvent(tp->event);
        mprRemoveRoot(tp);
        tp->event = 0;
    }
    return 0;
}

/*********************************** Helpers **********************************/

static void manageTimer(EjsTimer *tp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(tp->ejs);
        mprMark(tp->event);
        mprMark(tp->callback);
        mprMark(tp->onerror);
        mprMark(tp->args);

    } else if (flags & MPR_MANAGE_FREE) {
        if (tp->event) {
            mprRemoveEvent(tp->event);
        }
    }
}


/*********************************** Factory **********************************/

void ejsConfigureTimerType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "Timer"), sizeof(EjsTimer), manageTimer,
            EJS_TYPE_OBJ | EJS_TYPE_MUTABLE_INSTANCES)) == 0) {
        return;
    }
    prototype = type->prototype;
    ejsBindConstructor(ejs, type, timer_constructor);
    ejsBindMethod(ejs, prototype, ES_Timer_start, timer_start);
    ejsBindMethod(ejs, prototype, ES_Timer_stop, timer_stop);

    ejsBindAccess(ejs, prototype, ES_Timer_drift, timer_get_drift, timer_set_drift);
    ejsBindAccess(ejs, prototype, ES_Timer_period, timer_get_period, timer_set_period);
    ejsBindAccess(ejs, prototype, ES_Timer_onerror, timer_get_onerror, timer_set_onerror);
    ejsBindAccess(ejs, prototype, ES_Timer_repeat, timer_get_repeat, timer_set_repeat);
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details.

    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
