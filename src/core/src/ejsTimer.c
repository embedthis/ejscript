/*
    ejsTimer.c -- Timer class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/****************************** Forward Declarations **************************/

static int timerCallback(EjsTimer *tp, MprEvent *e);

/*********************************** Methods **********************************/
/*
    Create a new timer

    function Timer(period: Number, callback: Function, oneShot: Boolean = true, drift: Boolean = true)
 */
static EjsVar *constructor(Ejs *ejs, EjsTimer *tp, int argc, EjsVar **argv)
{
    int     flags;

    mprAssert(argc >= 2);
    mprAssert(ejsIsNumber(argv[0]));
    mprAssert(ejsIsFunction(argv[1]));

    tp->ejs = ejs;
    tp->period = ejsGetInt(ejs, argv[0]);
    tp->callback = (EjsFunction*) argv[1];
    tp->oneShot = (argc >= 3) ? ejsGetInt(ejs, argv[2]) : 1;
    tp->drift = (argc >= 4) ? ejsGetInt(ejs, argv[3]) : 1;

    flags = tp->oneShot ? 0 : MPR_EVENT_CONTINUOUS;
    if ((tp->event = mprCreateEvent(ejs->dispatcher, "timer", tp->period, (MprEventProc) timerCallback, tp, flags)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    return 0;
}


/*
    Get the timer drift setting

    function get drift(): Boolean
 */
static EjsVar *getDrift(Ejs *ejs, EjsTimer *tp, int argc, EjsVar **argv)
{
    mprAssert(argc == 0);
    return (EjsVar*) ejsCreateBoolean(ejs, tp->drift);
}


/*
    Set the timer drift setting

    function set drift(period: Boolean): Void
 */
static EjsVar *setDrift(Ejs *ejs, EjsTimer *tp, int argc, EjsVar **argv)
{
    mprAssert(argc == 1 && ejsIsBoolean(argv[0]));
    tp->drift = ejsGetBoolean(ejs, argv[0]);
    return 0;
}


/*
    Get the timer period

    function get period(): Number
 */
static EjsVar *getPeriod(Ejs *ejs, EjsTimer *tp, int argc, EjsVar **argv)
{
    mprAssert(argc == 0);
    return (EjsVar*) ejsCreateNumber(ejs, tp->period);
}


/*
    Set the timer period and restart the timer

    function set period(period: Number): Void
 */
static EjsVar *setPeriod(Ejs *ejs, EjsTimer *tp, int argc, EjsVar **argv)
{
    mprAssert(argc == 1 && ejsIsNumber(argv[0]));

    tp->period = ejsGetInt(ejs, argv[0]);
    mprRescheduleEvent(tp->event, tp->period);
    return 0;
}


/*
    Restart a timer

    function restart(); Void
 */
static EjsVar *restart(Ejs *ejs, EjsTimer *tp, int argc, EjsVar **argv)
{
    mprAssert(argc == 0);
    mprRestartContinuousEvent(tp->event);
    return 0;
}


/*
    Stop a timer

    function stop(): Void
 */
static EjsVar *stop(Ejs *ejs, EjsTimer *tp, int argc, EjsVar **argv)
{
    mprAssert(argc == 0);
    mprRemoveEvent(tp->event);
    return 0;
}

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
    ejsSetProperty(ejs, (EjsVar*) event, ES_Event_data, (EjsVar*) tp);
    ejsSetProperty(ejs, (EjsVar*) event, ES_Event_timestamp, (EjsVar*) ejsCreateDate(ejs, 0));
    return event;
}


static int timerCallback(EjsTimer *tp, MprEvent *e)
{
    Ejs         *ejs;
    EjsObj      *event;
    EjsVar      *arg;

    mprAssert(tp);

    ejs = tp->ejs;
    if ((event = ejsCreateTimerEvent(ejs, tp)) == 0) {
        return 0;
    }
    arg = (EjsVar*) event;
    ejsRunFunction(tp->ejs, tp->callback, NULL, 1, &arg);
    if (tp->ejs->exception) {
        //  TODO must have way that users can catch these
        mprError(tp, 0, "Exception in timer: %s", ejsGetErrorMsg(tp->ejs, 1));
    }
    return 0;
}


/*********************************** Factory **********************************/

void ejsConfigureTimerType(Ejs *ejs)
{
    EjsType     *type;

    ejs->timerEventType = ejsGetTypeByName(ejs, "ejs", "TimerEvent");
    type = ejsGetTypeByName(ejs, "ejs", "Timer");
    type->instanceSize = sizeof(EjsTimer);

    ejsBindMethod(ejs, type, ES_Timer_Timer, (EjsProc) constructor);
    ejsBindMethod(ejs, type, ES_Timer_restart, (EjsProc) restart);
    ejsBindMethod(ejs, type, ES_Timer_stop, (EjsProc) stop);
    ejsBindAccess(ejs, type, ES_Timer_period, (EjsProc) getPeriod, (EjsProc) setPeriod);
    ejsBindAccess(ejs, type, ES_Timer_drift, (EjsProc) getDrift, (EjsProc) setDrift);
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
