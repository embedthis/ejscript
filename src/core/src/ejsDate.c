/**
    ejsDate.c - Date type class

    Date/time is store internally as milliseconds since 1970/01/01 GMT

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/***************************** Forward Declarations ***************************/
/*
    TODO - move to ejsNumber.h. But would have to rename fixed() to ejsFixed()
 */
#define fixed(n) ((int64) (floor(n)))

#if BLD_WIN_LIKE
#pragma warning (disable:4244)
#endif

//  TODO this is a generic need. Make an API

#define getNumber(ejs, a) ejsGetNumber(ejs, ejsToNumber(ejs, a))
#define getInt(ejs, a) ((int) ejsGetNumber(ejs, ejsToNumber(ejs, a)))

/******************************************************************************/
/*
    Cast the operand to the specified type

    function cast(type: Type) : Object
 */

static EjsAny *castDate(Ejs *ejs, EjsDate *dp, EjsType *type)
{
    switch (type->sid) {
    case S_Boolean:
        return ESV(true);

    case S_Number:
        return ejsCreateNumber(ejs, (MprNumber) dp->value);

    case S_String:
        /*
            Format:  Tue Jul 15 2012 10:53:23 GMT-0700 (PDT)
         */
        return ejsCreateStringFromAsc(ejs, mprFormatLocalTime("%a %b %d %Y %T GMT%z (%Z)", dp->value));

    default:
        ejsThrowTypeError(ejs, "Can't cast to this type");
        return 0;
    }
    return 0;
}


static EjsDate *cloneDate(Ejs *ejs, EjsDate *dp, int deep)
{
    return ejsCreateDate(ejs, dp->value);
}


/*
    TODO - this is the same as number. Should share code
 */
static EjsAny *coerceDateOperands(Ejs *ejs, EjsAny *lhs, int opcode, EjsAny *rhs)
{
    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        if (ejsIs(ejs, rhs, Void)) {
            return ESV(nan);
        } else if (ejsIs(ejs, rhs, Null)) {
            rhs = ESV(zero);
        } else if (ejsIs(ejs, rhs, Boolean) || ejsIs(ejs, rhs, Number)) {
            return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);
        } else {
            return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);
        }
        break;

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIs(ejs, rhs, String)) {
            return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_STRICTLY_NE:
        return ESV(true);

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return ESV(false);

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_TRUE:
        return (((EjsDate*) lhs)->value ? ESV(true) : ESV(false));

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return (((EjsDate*) lhs)->value ? ESV(false): ESV(true));

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return ESV(false);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return ESV(undefined);
    }
    return 0;
}


static EjsAny *invokeDateOperator(Ejs *ejs, EjsDate *lhs, int opcode, EjsDate *rhs)
{
    EjsAny      *result;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if (!ejsIs(ejs, lhs, Date) || !ejsIs(ejs, rhs, Date)) {
            if ((result = coerceDateOperands(ejs, lhs, opcode, rhs)) != 0) {
                return result;
            }
        }
    }

    switch (opcode) {
    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_STRICTLY_EQ:
        return ejsCreateBoolean(ejs, lhs->value == rhs->value);

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
        return ejsCreateBoolean(ejs, !(lhs->value == rhs->value));

    case EJS_OP_COMPARE_LT:
        return ejsCreateBoolean(ejs, lhs->value < rhs->value);

    case EJS_OP_COMPARE_LE:
        return ejsCreateBoolean(ejs, lhs->value <= rhs->value);

    case EJS_OP_COMPARE_GT:
        return ejsCreateBoolean(ejs, lhs->value > rhs->value);

    case EJS_OP_COMPARE_GE:
        return ejsCreateBoolean(ejs, lhs->value >= rhs->value);

    case EJS_OP_COMPARE_NOT_ZERO:
        return ((lhs->value) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_ZERO:
        return ((lhs->value == 0) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return ESV(false);

    /*
        Unary operators
     */
    case EJS_OP_NEG:
        return ejsCreateNumber(ejs, - (MprNumber) lhs->value);

    case EJS_OP_LOGICAL_NOT:
        return ejsCreateBoolean(ejs, (int) !fixed(lhs->value));

    case EJS_OP_NOT:
        return ejsCreateNumber(ejs, (MprNumber) (~fixed(lhs->value)));

    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsCreateDate(ejs, lhs->value + rhs->value);

    case EJS_OP_AND:
        return ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) & fixed(rhs->value)));

    case EJS_OP_DIV:
        if (rhs->value == 0) {
            ejsThrowArithmeticError(ejs, "Divisor is zero");
            return 0;
        }
        return ejsCreateDate(ejs, lhs->value / rhs->value);

    case EJS_OP_MUL:
        return ejsCreateDate(ejs, lhs->value * rhs->value);

    case EJS_OP_OR:
        return ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) | fixed(rhs->value)));

    case EJS_OP_REM:
        if (rhs->value == 0) {
            ejsThrowArithmeticError(ejs, "Divisor is zero");
            return 0;
        }
        return ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) % fixed(rhs->value)));

    case EJS_OP_SHL:
        return ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) << fixed(rhs->value)));

    case EJS_OP_SHR:
        return ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) >> fixed(rhs->value)));

    case EJS_OP_SUB:
        return ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) - fixed(rhs->value)));

    case EJS_OP_USHR:
        return ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) >> fixed(rhs->value)));

    case EJS_OP_XOR:
        return ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) ^ fixed(rhs->value)));

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
    /* Should never get here */
}


/*********************************** Methods **********************************/
/*
    Date constructor
        Date()
        Date(milliseconds)
        Date(dateString)
        Date(year, month, date, hour, minute, second, msec)
        @param milliseconds Integer representing milliseconds since 1 January 1970 00:00:00 UTC.
        @param dateString String date value in a format recognized by parse().
        @param year Integer value for the year. Should be a Four digit year (e.g. 1998).
        @param month Integer month value (0-11)
        @param date Integer date of the month (1-31)
        @param hour Integer hour value (0-23)
        @param minute Integer minute value (0-59)
        @param second Integer second value (0-59)
        @param msec Integer millisecond value (0-999)
*/
static EjsDate *date_Date(Ejs *ejs, EjsDate *date, int argc, EjsObj **argv)
{
    EjsArray    *args;
    EjsObj      *vp;
    struct tm   tm;
    int         year;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Array));

    args = (EjsArray*) argv[0];

    if (args->length == 0) {
        /* Now */
        date->value = mprGetTime();

    } else if (args->length == 1) {
        vp = ejsGetProperty(ejs, args, 0);
        if (ejsIs(ejs, vp, Number)) {
            /* Milliseconds */
            date->value = ejsGetNumber(ejs, vp);

        } else if (ejsIs(ejs, vp, String)) {
            if (mprParseTime(&date->value, ejsToMulti(ejs, vp), MPR_LOCAL_TIMEZONE, NULL) < 0) {
                ejsThrowArgError(ejs, "Can't parse date string: %@", ejsToString(ejs, vp));
                return 0;
            }
        } else if (ejsIs(ejs, vp, Date)) {
            date->value = ((EjsDate*) vp)->value;

        } else {
            ejsThrowArgError(ejs, "Can't construct date from this argument");
            return 0;
        }

    } else {
        /* Date(year, month, date, hour, minute, second, msec) or any portion thereof */
        memset(&tm, 0, sizeof(tm));
        tm.tm_isdst = -1;
        vp = ejsGetProperty(ejs, args, 0);
        year = getInt(ejs, vp);
        if (0 <= year && year < 100) {
            year += 1900;
        }
        tm.tm_year = year - 1900;
        if (args->length > 1) {
            vp = ejsGetProperty(ejs, args, 1);
            tm.tm_mon = getInt(ejs, vp);
        }
        if (args->length > 2) {
            vp = ejsGetProperty(ejs, args, 2);
            tm.tm_mday = getInt(ejs, vp);
        } else {
            tm.tm_mday = 1;
        }
        if (args->length > 3) {
            vp = ejsGetProperty(ejs, args, 3);
            tm.tm_hour = getInt(ejs, vp);
        }
        if (args->length > 4) {
            vp = ejsGetProperty(ejs, args, 4);
            tm.tm_min = getInt(ejs, vp);
        }
        if (args->length > 5) {
            vp = ejsGetProperty(ejs, args, 5);
            tm.tm_sec = getInt(ejs, vp);
        }
        date->value = mprMakeTime(&tm);
        if (date->value == -1) {
            ejsThrowArgError(ejs, "Can't construct date from this argument");
        } else if (args->length > 6) {
            vp = ejsGetProperty(ejs, args, 6);
            date->value += getNumber(ejs, vp);
        }        
    }
    return date;
}


/*
    function get day(): Number
    Range: 0-6, where 0 is Sunday
 */
static EjsNumber *date_day(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_wday);
}


/*
    function set day(day: Number): Void
    Range: 0-6, where 0 is Sunday
*/
static EjsObj *date_set_day(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;
    MprTime     dayDiff, day;

    day = ejsGetNumber(ejs, argv[0]);
    mprDecodeLocalTime(&tm, dp->value);
    dayDiff = day - tm.tm_wday;
    dp->value += dayDiff * 86400 * MPR_TICKS_PER_SEC;
    return 0;
}


/*
    function get dayOfYear(): Number
    Return day of year (0 - 365)
 */
static EjsNumber *date_dayOfYear(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_yday);
}


/*
    function set dayOfYear(day: Number): Void
    Set the day of year (0 - 365)
 */
static EjsObj *date_set_dayOfYear(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;
    MprTime     dayDiff, day;

    day = ejsGetNumber(ejs, argv[0]);
    mprDecodeLocalTime(&tm, dp->value);
    dayDiff = day - tm.tm_yday;
    dp->value += dayDiff * 86400 * MPR_TICKS_PER_SEC;
    return 0;
}


/*
    function get date(): Number
    Return day of month (1-31)
 */
static EjsNumber *date_date(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_mday);
}


/*
    function set date(date: Number): Void
    Range day of month (1-31)
 */
static EjsObj *date_set_date(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;
    MprTime     dayDiff, day;

    day = ejsGetNumber(ejs, argv[0]);
    mprDecodeLocalTime(&tm, dp->value);
    dayDiff = day - tm.tm_mday;
    dp->value += dayDiff * 86400 * MPR_TICKS_PER_SEC;
    return 0;
}


/*
    function get elapsed(): Number
    Get the elapsed time in milliseconds since the Date object was constructed
 */
static EjsNumber *date_elapsed(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, mprGetElapsedTime(dp->value));
}


/*
    function format(layout: String): String
 */
static EjsString *date_format(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, mprFormatLocalTime(ejsToMulti(ejs, argv[0]), dp->value));
}


/*
    function formatUTC(layout: String): String
 */
static EjsString *date_formatUTC(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, mprFormatUniversalTime(ejsToMulti(ejs, argv[0]), dp->value));
}


/*
    function future(msec: Number): Date
 */
static EjsDate *date_future(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    MprTime     inc;

    inc = ejsGetNumber(ejs, argv[0]);
    return ejsCreateDate(ejs, dp->value + inc);
}


/**
    Return the number of minutes between the local computer time and Coordinated Universal Time.
    @return Integer containing the number of minutes between UTC and local time. The offset is positive if
    local time is behind UTC and negative if it is ahead. E.g. American PST is UTC-8 so 420/480 will be retured
    depending on if daylight savings is in effect.

    function getTimezoneOffset(): Number
*/
static EjsNumber *date_getTimezoneOffset(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, -mprGetTimeZoneOffset(dp->value) / (MPR_TICKS_PER_SEC * 60));
}


/*
    function getUTCDate(): Number
    Range: 0-31
 */
static EjsNumber *date_getUTCDate(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_mday);
}


/*
    function getUTCDay(): Number
    Range: 0-6
 */
static EjsNumber *date_getUTCDay(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_wday);
}


/*
    function getUTCFullYear(): Number
    Range: 4 digits
 */
static EjsNumber *date_getUTCFullYear(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_year + 1900);
}


/*
    function getUTCHours(): Number
    Range: 0-23
 */
static EjsNumber *date_getUTCHours(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_hour);
}


/*
    function getUTCMilliseconds(): Number
    Range: 0-999
 */
static EjsNumber *date_getUTCMilliseconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, ((int64) dp->value) % MPR_TICKS_PER_SEC);
}


/*
    function getUTCMinutes(): Number
    Range: 0-31
 */
static EjsNumber *date_getUTCMinutes(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_min);
}


/*
    function getUTCMonth(): Number
    Range: 1-12
 */
static EjsNumber *date_getUTCMonth(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_mon);
}


/*
    function getUTCSeconds(): Number
    Range: 0-59
 */
static EjsNumber *date_getUTCSeconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_sec);
}


/*
    function get hours(): Number
    Return hour of day (0-23)
 */
static EjsNumber *date_hours(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_hour);
}


/*
    function set hours(hour: Number): void
    Update the hour of the day using a 0-23 hour
 */
static EjsObj *date_set_hours(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    tm.tm_hour = (int) ejsGetNumber(ejs, argv[0]);
    dp->value = mprMakeTime(&tm);
    return 0;
}


/*
    function get milliseconds(): Number
 */
static EjsNumber *date_milliseconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, ((int64) dp->value) % MPR_TICKS_PER_SEC);
}


/*
    function set milliseconds(ms: Number): void
 */
static EjsObj *date_set_milliseconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    dp->value = (dp->value / MPR_TICKS_PER_SEC  * MPR_TICKS_PER_SEC) + ejsGetNumber(ejs, argv[0]);
    return 0;
}


/*
    function get minutes(): Number
 */
static EjsNumber *date_minutes(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_min);
}


/*
    function set minutes(min: Number): void
 */
static EjsObj *date_set_minutes(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    tm.tm_min = (int) ejsGetNumber(ejs, argv[0]);
    dp->value = mprMakeTime(&tm);
    return 0;
}


/*
    function get month(): Number
    Get the month (0-11)
 */
static EjsNumber *date_month(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_mon);
}


/*
    function set month(month: Number): void
 */
static EjsObj *date_set_month(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    tm.tm_mon = (int) ejsGetNumber(ejs, argv[0]);
    dp->value = mprMakeTime(&tm);
    return 0;
}


/*
    function nextDay(inc: Number = 1): Date
 */
static EjsDate *date_nextDay(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    MprTime     inc;

    if (argc == 1) {
        inc = ejsGetNumber(ejs, argv[0]);
    } else {
        inc = 1;
    }
    return ejsCreateDate(ejs, dp->value + (inc * 86400 * 1000));
}


/*
    static function now(): Number
 */
static EjsNumber *date_now(Ejs *ejs, EjsDate *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, mprGetTime());
}


/*
    static function parse(arg: String): Number
 */
static EjsNumber *date_parse(Ejs *ejs, EjsDate *unused, int argc, EjsObj **argv)
{
    MprTime     when;

    if (mprParseTime(&when, ejsToMulti(ejs, argv[0]), MPR_LOCAL_TIMEZONE, NULL) < 0) {
        ejsThrowArgError(ejs, "Can't parse date string: %@", ejsToString(ejs, argv[0]));
        return 0;
    }
    return ejsCreateNumber(ejs, (MprNumber) when);
}


/*
    static function parseDate(arg: String, defaultDate: Date = null): Date
 */
static EjsDate *date_parseDate(Ejs *ejs, EjsDate *unused, int argc, EjsObj **argv)
{
    struct tm   tm, *defaults;
    MprTime     when;

    if (argc >= 2) {
        mprDecodeLocalTime(&tm, ((EjsDate*) argv[1])->value);
        defaults = &tm;
    } else {
        defaults = 0;
    }
    if (mprParseTime(&when, ejsToMulti(ejs, argv[0]), MPR_LOCAL_TIMEZONE, defaults) < 0) {
        ejsThrowArgError(ejs, "Can't parse date string: %@", ejsToString(ejs, argv[0]));
        return 0;
    }
    return ejsCreateDate(ejs, when);
}


/*
    static function parseUTCDate(arg: String, defaultDate: Date = null): Date
 */
static EjsDate *date_parseUTCDate(Ejs *ejs, EjsDate *unused, int argc, EjsObj **argv)
{
    struct tm   tm, *defaults;
    MprTime     when;

    if (argc >= 2) {
        mprDecodeUniversalTime(&tm, ((EjsDate*) argv[1])->value);
        defaults = &tm;
    } else {
        defaults = 0;
    }
    if (mprParseTime(&when, ejsToMulti(ejs, argv[0]), MPR_UTC_TIMEZONE, defaults) < 0) {
        ejsThrowArgError(ejs, "Can't parse date string: %@", ejsToString(ejs, argv[0]));
        return 0;
    }
    return ejsCreateDate(ejs, when);
}


/*
    function get seconds(): Number
    Get seconds (0-59)
 */
static EjsNumber *date_seconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_sec);
}


/*
    function set seconds(sec: Number): void
 */
static EjsObj *date_set_seconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    tm.tm_sec = (int) ejsGetNumber(ejs, argv[0]);
    dp->value = mprMakeTime(&tm);
    return 0;
}


/*
    function setUTCDate(date: Number): Void
    Range month (1-31)
 */
static EjsObj *date_setUTCDate(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;
    MprTime     dayDiff, day;

    day = ejsGetNumber(ejs, argv[0]);
    mprDecodeUniversalTime(&tm, dp->value);
    dayDiff = day - tm.tm_mday;
    dp->value += dayDiff * 86400 * MPR_TICKS_PER_SEC;
    return 0;
}


/*
   function setUTCFullYear(y: Number): void
 */
static EjsObj *date_setUTCFullYear(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    tm.tm_year = (int) ejsGetNumber(ejs, argv[0]) - 1900;
    dp->value = mprMakeUniversalTime(&tm);
    return 0;
}


/*
    function setUTCHours(h: Number): void
 */
static EjsObj *date_setUTCHours(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    tm.tm_hour = (int) ejsGetNumber(ejs, argv[0]);
    dp->value = mprMakeUniversalTime(&tm);
    return 0;
}


/*
    function setUTCMilliseconds(ms: Number): void
 */
static EjsObj *date_setUTCMilliseconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    /* Same as set_milliseconds */
    dp->value = (dp->value / MPR_TICKS_PER_SEC  * MPR_TICKS_PER_SEC) + ejsGetNumber(ejs, argv[0]);
    return 0;
}


/*
    function setUTCMinutes(min: Number): void
 */
static EjsObj *date_setUTCMinutes(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    tm.tm_min = (int) ejsGetNumber(ejs, argv[0]);
    dp->value = mprMakeUniversalTime(&tm);
    return 0;
}


/*
    function setUTCMonth(mon: Number): void
 */
static EjsObj *date_setUTCMonth(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    tm.tm_mon = (int) ejsGetNumber(ejs, argv[0]);
    dp->value = mprMakeUniversalTime(&tm);
    return 0;
}


/*
    function setUTCSeconds(sec: Number, msec: Number = null): void
 */
static EjsObj *date_setUTCSeconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    tm.tm_sec = (int) ejsGetNumber(ejs, argv[0]);
    dp->value = mprMakeUniversalTime(&tm);
    if (argc >= 2) {
        dp->value = (dp->value / MPR_TICKS_PER_SEC  * MPR_TICKS_PER_SEC) + ejsGetNumber(ejs, argv[1]);
    }
    return 0;
}


/*
    static function ticks(): Number
 */
static EjsNumber *date_ticks(Ejs *ejs, EjsDate *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, mprGetTicks());
}


/*
    Get the number of millseconds since Jan 1, 1970 UTC.
    function get time(): Number
 */
static EjsNumber *date_time(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, dp->value);
}


/*
    function set time(value: Number): Number
 */
static EjsNumber *date_set_time(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    dp->value = ejsGetNumber(ejs, argv[0]);
    return 0;
}


/**
    Return an ISO formatted date string.
    Sample format: "2006-12-15T23:45:09.33-08:00"
    function toISOString(): String
*/
static EjsString *date_toISOString(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    char    *base, *str;

    base = mprFormatUniversalTime("%Y-%m-%dT%H:%M:%S", dp->value);
    str = sfmt("%s.%03dZ", base, dp->value % MPR_TICKS_PER_SEC);
    return ejsCreateStringFromAsc(ejs, str);
}


/*
    Serialize using JSON encoding. This uses the ISO date format

    function toJSON(): String
 */
static EjsString *date_toJSON(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    char    *base, *str;

    base = mprFormatUniversalTime("%Y-%m-%dT%H:%M:%S", dp->value);
    str = sfmt("\"%sZ\"", base);
    return ejsCreateStringFromAsc(ejs, str);
}


/*
    override native function toString(): String
 */
static EjsString *date_toString(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return castDate(ejs, dp, ESV(String));
}


/*
    Construct a date from UTC values
    function UTC(year, month, date, hour = 0, minute = 0, second = 0, msec = 0): Number
 */
static EjsNumber *date_UTC(Ejs *ejs, EjsDate *unused, int argc, EjsObj **argv)
{
    EjsDate     *dp;
    struct tm   tm;
    int         year;

    memset(&tm, 0, sizeof(tm));
    year = getInt(ejs, argv[0]);
    if (year < 100) {
        year += 1900;
    }
    tm.tm_year = year - 1900;
    if (argc > 1) {
        tm.tm_mon = getInt(ejs, argv[1]);
    }
    if (argc > 2) {
        tm.tm_mday = getInt(ejs, argv[2]);
    }
    if (argc > 3) {
        tm.tm_hour = getInt(ejs, argv[3]);
    }
    if (argc > 4) {
        tm.tm_min = getInt(ejs, argv[4]);
    }
    if (argc > 5) {
        tm.tm_sec = getInt(ejs, argv[5]);
    }
    dp = ejsCreateDate(ejs, mprMakeUniversalTime(&tm));
    if (argc > 6) {
        dp->value += getNumber(ejs, argv[6]);
    }
    return ejsCreateNumber(ejs, dp->value);
}


/*
    function get year(): Number
 */
static EjsNumber *date_year(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return ejsCreateNumber(ejs, tm.tm_year + 1900);
}


/*
    function set year(year: Number): void
 */
static EjsObj *date_set_year(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    tm.tm_year = (int) ejsGetNumber(ejs, argv[0]) - 1900;
    dp->value = mprMakeTime(&tm);
    return 0;
}

/*********************************** Factory **********************************/
/*
    Create an initialized date object. Set to the current time if value is zero.
 */

EjsDate *ejsCreateDate(Ejs *ejs, MprTime value)
{
    EjsDate *vp;

    vp = ejsCreateObj(ejs, ESV(Date), 0);
    if (vp != 0) {
        vp->value = value;
    }
    return vp;
}


void ejsConfigureDateType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "Date"), sizeof(EjsDate), 0, 
            EJS_TYPE_OBJ | EJS_TYPE_MUTABLE_INSTANCES)) == 0) {
        return;
    }
    type->helpers.cast = (EjsCastHelper) castDate;
    type->helpers.clone = (EjsCloneHelper) cloneDate;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeDateOperator;

    ejsBindMethod(ejs, type, ES_Date_now, date_now);
    ejsBindAccess(ejs, type, ES_Date_ticks, date_ticks, NULL);
    ejsBindMethod(ejs, type, ES_Date_parseDate, date_parseDate);
    ejsBindMethod(ejs, type, ES_Date_parseUTCDate, date_parseUTCDate);
    ejsBindMethod(ejs, type, ES_Date_parse, date_parse);
    ejsBindMethod(ejs, type, ES_Date_UTC, date_UTC);

    prototype = type->prototype;
    ejsBindConstructor(ejs, type, date_Date);
    ejsBindAccess(ejs, prototype, ES_Date_day, date_day, date_set_day);
    ejsBindAccess(ejs, prototype, ES_Date_dayOfYear, date_dayOfYear, date_set_dayOfYear);
    ejsBindAccess(ejs, prototype, ES_Date_date, date_date, date_set_date);
    ejsBindMethod(ejs, prototype, ES_Date_elapsed, date_elapsed);
    ejsBindMethod(ejs, prototype, ES_Date_format, date_format);
    ejsBindMethod(ejs, prototype, ES_Date_formatUTC, date_formatUTC);
    ejsBindMethod(ejs, prototype, ES_Date_future, date_future);
    ejsBindMethod(ejs, prototype, ES_Date_getTimezoneOffset, date_getTimezoneOffset); 
    ejsBindMethod(ejs, prototype, ES_Date_getUTCDate, date_getUTCDate);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCDay, date_getUTCDay);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCFullYear, date_getUTCFullYear);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCHours, date_getUTCHours);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCMilliseconds, date_getUTCMilliseconds);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCMinutes, date_getUTCMinutes);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCMonth, date_getUTCMonth);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCSeconds, date_getUTCSeconds);
    ejsBindAccess(ejs, prototype, ES_Date_hours, date_hours, date_set_hours);
    ejsBindAccess(ejs, prototype, ES_Date_milliseconds, date_milliseconds, date_set_milliseconds);
    ejsBindAccess(ejs, prototype, ES_Date_minutes, date_minutes, date_set_minutes);
    ejsBindAccess(ejs, prototype, ES_Date_month, date_month, date_set_month);
    ejsBindMethod(ejs, prototype, ES_Date_nextDay, date_nextDay);
    ejsBindAccess(ejs, prototype, ES_Date_seconds, date_seconds, date_set_seconds);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCDate, date_setUTCDate);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCFullYear, date_setUTCFullYear);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCHours, date_setUTCHours);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCMilliseconds, date_setUTCMilliseconds);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCMinutes, date_setUTCMinutes);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCMonth, date_setUTCMonth);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCSeconds, date_setUTCSeconds);
    ejsBindAccess(ejs, prototype, ES_Date_time, date_time, date_set_time);
    ejsBindMethod(ejs, prototype, ES_Date_toJSON, date_toJSON);
    ejsBindMethod(ejs, prototype, ES_Date_toISOString, date_toISOString);
    ejsBindMethod(ejs, prototype, ES_Date_toString, date_toString);
    ejsBindAccess(ejs, prototype, ES_Date_year, date_year, date_set_year);
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
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
