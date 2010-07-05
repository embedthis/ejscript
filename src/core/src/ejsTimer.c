/*
    ejsTimer.c -- Timer class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Methods **********************************/
/*
    Create a new timer

    function Timer(period: Number, callback: Function, ...args)
 */
static EjsObj *constructor(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc >= 2);
    mprAssert(ejsIsNumber(argv[0]));
    mprAssert(ejsIsFunction(argv[1]));
    mprAssert(ejsIsArray(argv[2]));

    tp->ejs = ejs;
    tp->period = ejsGetInt(ejs, argv[0]);
    tp->callback = (EjsFunction*) argv[1];
    tp->args = (EjsArray*) argv[2];
    tp->repeat = 0;
    tp->drift = 1;
    return 0;
}


/*
    function get drift(): Boolean
 */
static EjsObj *getDrift(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 0);
    return (EjsObj*) ejsCreateBoolean(ejs, tp->drift);
}


/*
    function set drift(period: Boolean): Void
 */
static EjsObj *setDrift(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 1 && ejsIsBoolean(argv[0]));
    tp->drift = ejsGetBoolean(ejs, argv[0]);
    return 0;
}


/*
    function get period(): Number
 */
static EjsObj *getPeriod(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 0);
    return (EjsObj*) ejsCreateNumber(ejs, tp->period);
}


/*
    function set period(period: Number): Void
 */
static EjsObj *setPeriod(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 1 && ejsIsNumber(argv[0]));

    tp->period = ejsGetInt(ejs, argv[0]);
    return 0;
}


/*
    function get repeat(): Boolean
 */
static EjsObj *getRepeat(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 0);
    return (EjsObj*) ejsCreateBoolean(ejs, tp->repeat);
}


/*
    function set repeat(enable: Boolean): Void
 */
static EjsObj *setRepeat(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    mprAssert(argc == 1 && ejsIsNumber(argv[0]));

    tp->repeat = ejsGetBoolean(ejs, argv[0]);
    if (tp->event) {
        mprEnableContinuousEvent(tp->event, tp->repeat);
    }
    return 0;
}


static int timerCallback(EjsTimer *tp, MprEvent *e)
{
    Ejs         *ejs;

    mprAssert(tp);
    mprAssert(tp->args);

    ejs = tp->ejs;
    //  MOB -- this obj
    ejsRunFunction(tp->ejs, tp->callback, NULL, tp->args->length, tp->args->data);
    if (ejs->exception) {
#if FUTURE
        if (tp->onerror) {
            EjsString   *msg;
            msg = ejsCreateString(ejs, ejsGetErrorMsg(ejs, 1));
            ejsRunFunction(tp->ejs, tp->onerror, NULL, 1, &msg);
        } else
#endif
        mprError(tp, "Exception in timer: %s", ejsGetErrorMsg(ejs, 1));
    }
    return 0;
}


/*
    function start(): Void
 */
static EjsObj *start(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
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
    return 0;
}


/*
    function stop(): Void
 */
static EjsObj *stop(Ejs *ejs, EjsTimer *tp, int argc, EjsObj **argv)
{
    if (tp->event) {
        mprRemoveEvent(tp->event);
    }
    return 0;
}

#if UNUSED
/*********************************** Support **********************************/
/*
    This creates a timer event object, but does not schedule it.
 */
EjsObj *ejsCreateTimerEvent(Ejs *ejs, EjsTimer *tp)
{
    EjsObj       *event;

    if ((event = ejsCreateObject(ejs, ejs->timerEventType, 0)) == 0) {
        return 0;
    }
    ejsSetProperty(ejs, (EjsObj*) event, ES_Event_data, (EjsObj*) tp);
    ejsSetProperty(ejs, (EjsObj*) event, ES_Event_timestamp, (EjsObj*) ejsCreateDate(ejs, 0));
    return event;
}

#endif /* UNUSED */

/*********************************** Factory **********************************/

void ejsConfigureTimerType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *prototype;

    type = ejsGetTypeByName(ejs, "ejs", "Timer");
    type->instanceSize = sizeof(EjsTimer);
    prototype = type->prototype;

    ejsBindConstructor(ejs, type, (EjsProc) constructor);
#if ES_Timer_start
    ejsBindMethod(ejs, prototype, ES_Timer_start, (EjsProc) start);
#endif
    ejsBindMethod(ejs, prototype, ES_Timer_stop, (EjsProc) stop);

    ejsBindAccess(ejs, prototype, ES_Timer_drift, (EjsProc) getDrift, (EjsProc) setDrift);
    ejsBindAccess(ejs, prototype, ES_Timer_period, (EjsProc) getPeriod, (EjsProc) setPeriod);
#if ES_Timer_repeat
    ejsBindAccess(ejs, prototype, ES_Timer_repeat, (EjsProc) getRepeat, (EjsProc) setRepeat);
#endif
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
