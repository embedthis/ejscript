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

#define getNumber(ejs, a) ejsGetNumber(ejs, (EjsObj*) ejsToNumber(ejs, ((EjsObj*) a)))

/******************************************************************************/
/*
    Cast the operand to the specified type

    function cast(type: Type) : Object
 */

static EjsObj *castDate(Ejs *ejs, EjsDate *dp, EjsType *type)
{
    struct tm   tm;

    switch (type->sid) {

    case S_Boolean:
        return (EjsObj*) S(true);

    case S_Number:
        return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) dp->value);

    case S_String:
        /*
            Format:  Tue Jul 15 2011 10:53:23 GMT-0700 (PDT)
         */
        mprDecodeLocalTime(&tm, dp->value);
        return (EjsObj*) ejsCreateStringFromAsc(ejs, mprFormatTime("%a %b %d %Y %T GMT%z (%Z)", &tm));

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
static EjsObj *coerceDateOperands(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs)
{
    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        if (ejsIs(ejs, rhs, Void)) {
            return (EjsObj*) S(nan);
        } else if (ejsIs(ejs, rhs, Null)) {
            rhs = (EjsObj*) S(zero);
        } else if (ejsIs(ejs, rhs, Boolean) || ejsIs(ejs, rhs, Number)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);
        } else {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);
        }
        break;

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIs(ejs, rhs, String)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_STRICTLY_NE:
        return (EjsObj*) S(true);

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return (EjsObj*) S(false);

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_TRUE:
        return (EjsObj*) (((EjsDate*) lhs)->value ? S(true) : S(false));

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return (EjsObj*) (((EjsDate*) lhs)->value ? S(false): S(true));

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return (EjsObj*) S(false);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return S(undefined);
    }
    return 0;
}


static EjsObj *invokeDateOperator(Ejs *ejs, EjsDate *lhs, int opcode, EjsDate *rhs)
{
    EjsObj      *result;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if (!ejsIs(ejs, lhs, Date) || !ejsIs(ejs, rhs, Date)) {
            if ((result = coerceDateOperands(ejs, (EjsObj*) lhs, opcode, (EjsObj*) rhs)) != 0) {
                return result;
            }
        }
    }

    switch (opcode) {
    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_STRICTLY_EQ:
        return (EjsObj*) ejsCreateBoolean(ejs, lhs->value == rhs->value);

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
        return (EjsObj*) ejsCreateBoolean(ejs, !(lhs->value == rhs->value));

    case EJS_OP_COMPARE_LT:
        return (EjsObj*) ejsCreateBoolean(ejs, lhs->value < rhs->value);

    case EJS_OP_COMPARE_LE:
        return (EjsObj*) ejsCreateBoolean(ejs, lhs->value <= rhs->value);

    case EJS_OP_COMPARE_GT:
        return (EjsObj*) ejsCreateBoolean(ejs, lhs->value > rhs->value);

    case EJS_OP_COMPARE_GE:
        return (EjsObj*) ejsCreateBoolean(ejs, lhs->value >= rhs->value);

    case EJS_OP_COMPARE_NOT_ZERO:
        return (EjsObj*) ((lhs->value) ? S(true): S(false));

    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ((lhs->value == 0) ? S(true): S(false));

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return (EjsObj*) S(false);

    /*
        Unary operators
     */
    case EJS_OP_NEG:
        return (EjsObj*) ejsCreateNumber(ejs, - (MprNumber) lhs->value);

    case EJS_OP_LOGICAL_NOT:
        return (EjsObj*) ejsCreateBoolean(ejs, (MprNumber) !fixed(lhs->value));

    case EJS_OP_NOT:
        return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) (~fixed(lhs->value)));

    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return (EjsObj*) ejsCreateDate(ejs, lhs->value + rhs->value);

    case EJS_OP_AND:
        return (EjsObj*) ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) & fixed(rhs->value)));

    case EJS_OP_DIV:
        if (rhs->value == 0) {
            ejsThrowArithmeticError(ejs, "Divisor is zero");
            return 0;
        }
        return (EjsObj*) ejsCreateDate(ejs, lhs->value / rhs->value);

    case EJS_OP_MUL:
        return (EjsObj*) ejsCreateDate(ejs, lhs->value * rhs->value);

    case EJS_OP_OR:
        return (EjsObj*) ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) | fixed(rhs->value)));

    case EJS_OP_REM:
        if (rhs->value == 0) {
            ejsThrowArithmeticError(ejs, "Divisor is zero");
            return 0;
        }
        return (EjsObj*) ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) % fixed(rhs->value)));

    case EJS_OP_SHL:
        return (EjsObj*) ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) << fixed(rhs->value)));

    case EJS_OP_SHR:
        return (EjsObj*) ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) >> fixed(rhs->value)));

    case EJS_OP_SUB:
        return (EjsObj*) ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) - fixed(rhs->value)));

    case EJS_OP_USHR:
        return (EjsObj*) ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) >> fixed(rhs->value)));

    case EJS_OP_XOR:
        return (EjsObj*) ejsCreateDate(ejs, (MprNumber) (fixed(lhs->value) ^ fixed(rhs->value)));

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
static EjsObj *date_Date(Ejs *ejs, EjsDate *date, int argc, EjsObj **argv)
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
        vp = ejsGetProperty(ejs, (EjsObj*) args, 0);
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
        vp = ejsGetProperty(ejs, (EjsObj*) args, 0);
        year = getNumber(ejs, vp);
        if (0 <= year && year < 100) {
            year += 1900;
        }
        tm.tm_year = year - 1900;
        if (args->length > 1) {
            vp = ejsGetProperty(ejs, (EjsObj*) args, 1);
            tm.tm_mon = getNumber(ejs, vp);
        }
        if (args->length > 2) {
            vp = ejsGetProperty(ejs, (EjsObj*) args, 2);
            tm.tm_mday = getNumber(ejs, vp);
        } else {
            tm.tm_mday = 1;
        }
        if (args->length > 3) {
            vp = ejsGetProperty(ejs, (EjsObj*) args, 3);
            tm.tm_hour = getNumber(ejs, vp);
        }
        if (args->length > 4) {
            vp = ejsGetProperty(ejs, (EjsObj*) args, 4);
            tm.tm_min = getNumber(ejs, vp);
        }
        if (args->length > 5) {
            vp = ejsGetProperty(ejs, (EjsObj*) args, 5);
            tm.tm_sec = getNumber(ejs, vp);
        }
        date->value = mprMakeTime(&tm);
        if (date->value == -1) {
            ejsThrowArgError(ejs, "Can't construct date from this argument");
        } else if (args->length > 6) {
            vp = ejsGetProperty(ejs, (EjsObj*) args, 6);
            date->value += getNumber(ejs, vp);
        }
    }
    return (EjsObj*) date;
}


/*
    function get day(): Number
    Range: 0-6, where 0 is Sunday
 */
static EjsObj *date_day(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_wday);
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
static EjsObj *date_dayOfYear(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_yday);
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
static EjsObj *date_date(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_mday);
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
static EjsObj *date_elapsed(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, mprGetElapsedTime(dp->value));
}


/*
    function format(layout: String): String
 */
static EjsObj *date_format(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateStringFromAsc(ejs, mprFormatTime(ejsToMulti(ejs, argv[0]), &tm));
}


/*
    function formatUTC(layout: String): String
 */
static EjsObj *date_formatUTC(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateStringFromAsc(ejs, mprFormatTime(ejsToMulti(ejs, argv[0]), &tm));
}


/*
    function get fullYear(): Number
    Return year in 4 digits
 */
static EjsObj *date_fullYear(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_year + 1900);
}


/*
    function set fullYear(year: Number): void
    Update the year component using a 4 digit year
 */
static EjsObj *date_set_fullYear(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    tm.tm_year = ejsGetNumber(ejs, argv[0]) - 1900;
    dp->value = mprMakeTime(&tm);
    return 0;
}


/*
    function future(msec: Number): Date
 */
static EjsObj *date_future(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    MprTime     inc;

    inc = ejsGetNumber(ejs, argv[0]);
    return (EjsObj*) ejsCreateDate(ejs, dp->value + inc);
}


/**
    Return the number of minutes between the local computer time and Coordinated Universal Time.
    @return Integer containing the number of minutes between UTC and local time. The offset is positive if
    local time is behind UTC and negative if it is ahead. E.g. American PST is UTC-8 so 420/480 will be retured
    depending on if daylight savings is in effect.

    function getTimezoneOffset(): Number
*/
static EjsObj *date_getTimezoneOffset(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, -mprGetTimeZoneOffset(dp->value) / (MPR_TICKS_PER_SEC * 60));
}


/*
    function getUTCDate(): Number
    Range: 0-31
 */
static EjsObj *date_getUTCDate(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_mday);
}


/*
    function getUTCDay(): Number
    Range: 0-6
 */
static EjsObj *date_getUTCDay(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_wday);
}


/*
    function getUTCFullYear(): Number
    Range: 4 digits
 */
static EjsObj *date_getUTCFullYear(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_year + 1900);
}


/*
    function getUTCHours(): Number
    Range: 0-23
 */
static EjsObj *date_getUTCHours(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_hour);
}


/*
    function getUTCMilliseconds(): Number
    Range: 0-999
 */
static EjsObj *date_getUTCMilliseconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, ((int64) dp->value) % MPR_TICKS_PER_SEC);
}


/*
    function getUTCMinutes(): Number
    Range: 0-31
 */
static EjsObj *date_getUTCMinutes(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_min);
}


/*
    function getUTCMonth(): Number
    Range: 1-12
 */
static EjsObj *date_getUTCMonth(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_mon);
}


/*
    function getUTCSeconds(): Number
    Range: 0-59
 */
static EjsObj *date_getUTCSeconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeUniversalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_sec);
}


/*
    function get hours(): Number
    Return hour of day (0-23)
 */
static EjsObj *date_hours(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_hour);
}


/*
    function set hours(hour: Number): void
    Update the hour of the day using a 0-23 hour
 */
static EjsObj *date_set_hours(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    tm.tm_hour = ejsGetNumber(ejs, argv[0]);
    dp->value = mprMakeTime(&tm);
    return 0;
}


/*
    function get milliseconds(): Number
 */
static EjsObj *date_milliseconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, ((int64) dp->value) % MPR_TICKS_PER_SEC);
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
static EjsObj *date_minutes(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_min);
}


/*
    function set minutes(min: Number): void
 */
static EjsObj *date_set_minutes(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    tm.tm_min = ejsGetNumber(ejs, argv[0]);
    dp->value = mprMakeTime(&tm);
    return 0;
}


/*
    function get month(): Number
    Get the month (0-11)
 */
static EjsObj *date_month(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_mon);
}


/*
    function set month(month: Number): void
 */
static EjsObj *date_set_month(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    tm.tm_mon = ejsGetNumber(ejs, argv[0]);
    dp->value = mprMakeTime(&tm);
    return 0;
}


/*
    function nextDay(inc: Number = 1): Date
 */
static EjsObj *date_nextDay(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    MprTime     inc;

    if (argc == 1) {
        inc = ejsGetNumber(ejs, argv[0]);
    } else {
        inc = 1;
    }
    return (EjsObj*) ejsCreateDate(ejs, dp->value + (inc * 86400 * 1000));
}


/*
    static function now(): Number
 */
static EjsObj *date_now(Ejs *ejs, EjsDate *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, mprGetTime());
}


/*
    static function parse(arg: String): Date
 */
static EjsObj *date_parse(Ejs *ejs, EjsDate *unused, int argc, EjsObj **argv)
{
    MprTime     when;

    if (mprParseTime(&when, ejsToMulti(ejs, argv[0]), MPR_LOCAL_TIMEZONE, NULL) < 0) {
        ejsThrowArgError(ejs, "Can't parse date string: %@", ejsToString(ejs, argv[0]));
        return 0;
    }
    return (EjsObj*) ejsCreateNumber(ejs, when);
}


/*
    static function parseDate(arg: String, defaultDate: Date = null): Date
 */
static EjsObj *date_parseDate(Ejs *ejs, EjsDate *unused, int argc, EjsObj **argv)
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
    return (EjsObj*) ejsCreateDate(ejs, when);
}


/*
    static function parseUTCDate(arg: String, defaultDate: Date = null): Date
 */
static EjsObj *date_parseUTCDate(Ejs *ejs, EjsDate *unused, int argc, EjsObj **argv)
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
    return (EjsObj*) ejsCreateDate(ejs, when);
}


/*
    function get seconds(): Number
    Get seconds (0-59)
 */
static EjsObj *date_seconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_sec);
}


/*
    function set seconds(sec: Number): void
 */
static EjsObj *date_set_seconds(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    tm.tm_sec = ejsGetNumber(ejs, argv[0]);
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
    tm.tm_year = ejsGetNumber(ejs, argv[0]) - 1900;
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
    tm.tm_hour = ejsGetNumber(ejs, argv[0]);
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
    tm.tm_min = ejsGetNumber(ejs, argv[0]);
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
    tm.tm_mon = ejsGetNumber(ejs, argv[0]);
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
    tm.tm_sec = ejsGetNumber(ejs, argv[0]);
    dp->value = mprMakeUniversalTime(&tm);
    if (argc >= 2) {
        dp->value = (dp->value / MPR_TICKS_PER_SEC  * MPR_TICKS_PER_SEC) + ejsGetNumber(ejs, argv[1]);
    }
    return 0;
}


/*
    static function ticks(): Number
 */
static EjsObj *date_ticks(Ejs *ejs, EjsDate *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, mprGetTicks());
}


/*
    Get the number of millseconds since Jan 1, 1970 UTC.
    function get time(): Number
 */
static EjsObj *date_time(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, dp->value);
}


/*
    function set time(value: Number): Number
 */
static EjsObj *date_set_time(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    dp->value = ejsGetNumber(ejs, argv[0]);
    return 0;
}


/**
    Return an ISO formatted date string.
    Sample format: "2006-12-15T23:45:09.33-08:00"
    function toISOString(): String
*/
static EjsObj *date_toISOString(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    EjsObj      *vp;
    struct tm   tm;
    char        *base, *str;

    mprDecodeUniversalTime(&tm, dp->value);
    base = mprFormatTime("%Y-%m-%dT%H:%M:%S", &tm);
    str = mprAsprintf("%s.%03dZ", base, dp->value % MPR_TICKS_PER_SEC);
    vp = (EjsObj*) ejsCreateStringFromAsc(ejs, str);
    return vp;
}


/*
    Serialize using JSON encoding. This uses the ISO date format

    function toJSON()
 */
static EjsObj *date_toJSON(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;
    char        *base, *str;

    mprDecodeUniversalTime(&tm, dp->value);
    base = mprFormatTime("%Y-%m-%dT%H:%M:%S", &tm);
    str = mprAsprintf("\"%sZ\"", base);
    return (EjsObj*) ejsCreateStringFromAsc(ejs, str);
}


/*
    override native function toString(): String
 */
static EjsObj *date_toString(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    return castDate(ejs, dp, ST(String));
}


/*
    Construct a date from UTC values
    function UTC(year, month, date, hour = 0, minute = 0, second = 0, msec = 0): Number
 */
static EjsObj *date_UTC(Ejs *ejs, EjsDate *unused, int argc, EjsObj **argv)
{
    EjsDate     *dp;
    struct tm   tm;
    int         year;

    memset(&tm, 0, sizeof(tm));
    year = getNumber(ejs, argv[0]);
    if (year < 100) {
        year += 1900;
    }
    tm.tm_year = year - 1900;
    if (argc > 1) {
        tm.tm_mon = getNumber(ejs, argv[1]);
    }
    if (argc > 2) {
        tm.tm_mday = getNumber(ejs, argv[2]);
    }
    if (argc > 3) {
        tm.tm_hour = getNumber(ejs, argv[3]);
    }
    if (argc > 4) {
        tm.tm_min = getNumber(ejs, argv[4]);
    }
    if (argc > 5) {
        tm.tm_sec = getNumber(ejs, argv[5]);
    }
    dp = ejsCreateDate(ejs, mprMakeUniversalTime(&tm));
    if (argc > 6) {
        dp->value += getNumber(ejs, argv[6]);
    }
    return (EjsObj*) ejsCreateNumber(ejs, dp->value);
}


/*
    function get year(): Number
 */
static EjsObj *date_year(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    return (EjsObj*) ejsCreateNumber(ejs, tm.tm_year + 1900);
}


/*
    function set year(year: Number): void
 */
static EjsObj *date_set_year(Ejs *ejs, EjsDate *dp, int argc, EjsObj **argv)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, dp->value);
    tm.tm_year = ejsGetNumber(ejs, argv[0]) - 1900;
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

    vp = ejsCreateObj(ejs, S(Date), 0);
    if (vp != 0) {
        vp->value = value;
    }
    return vp;
}


void ejsConfigureDateType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    type = ejsConfigureNativeType(ejs, N("ejs", "Date"), sizeof(EjsDate), NULL, EJS_OBJ_HELPERS);
    ejsSetSpecialType(ejs, S_Date, type);
    prototype = type->prototype;

    type->helpers.cast = (EjsCastHelper) castDate;
    type->helpers.clone = (EjsCloneHelper) cloneDate;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeDateOperator;

    ejsBindMethod(ejs, type, ES_Date_now, (EjsProc) date_now);
#if ES_Date_ticks
    ejsBindAccess(ejs, type, ES_Date_ticks, (EjsProc) date_ticks, NULL);
#endif
    ejsBindMethod(ejs, type, ES_Date_parseDate, (EjsProc) date_parseDate);
    ejsBindMethod(ejs, type, ES_Date_parseUTCDate, (EjsProc) date_parseUTCDate);
    ejsBindMethod(ejs, type, ES_Date_parse, (EjsProc) date_parse);
    ejsBindMethod(ejs, type, ES_Date_UTC, (EjsProc) date_UTC);

    ejsBindConstructor(ejs, type, (EjsProc) date_Date);
    ejsBindAccess(ejs, prototype, ES_Date_day, (EjsProc) date_day, (EjsProc) date_set_day);
    ejsBindAccess(ejs, prototype, ES_Date_dayOfYear, (EjsProc) date_dayOfYear, (EjsProc) date_set_dayOfYear);
    ejsBindAccess(ejs, prototype, ES_Date_date, (EjsProc) date_date, (EjsProc) date_set_date);
    ejsBindMethod(ejs, prototype, ES_Date_elapsed, (EjsProc) date_elapsed);
    ejsBindMethod(ejs, prototype, ES_Date_format, (EjsProc) date_format);
    ejsBindMethod(ejs, prototype, ES_Date_formatUTC, (EjsProc) date_formatUTC);
    ejsBindAccess(ejs, prototype, ES_Date_fullYear, (EjsProc) date_fullYear, (EjsProc) date_set_fullYear);
    ejsBindMethod(ejs, prototype, ES_Date_future, (EjsProc) date_future);
    ejsBindMethod(ejs, prototype, ES_Date_getTimezoneOffset, (EjsProc) date_getTimezoneOffset); 
    ejsBindMethod(ejs, prototype, ES_Date_getUTCDate, (EjsProc) date_getUTCDate);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCDay, (EjsProc) date_getUTCDay);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCFullYear, (EjsProc) date_getUTCFullYear);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCHours, (EjsProc) date_getUTCHours);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCMilliseconds, (EjsProc) date_getUTCMilliseconds);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCMinutes, (EjsProc) date_getUTCMinutes);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCMonth, (EjsProc) date_getUTCMonth);
    ejsBindMethod(ejs, prototype, ES_Date_getUTCSeconds, (EjsProc) date_getUTCSeconds);
    ejsBindAccess(ejs, prototype, ES_Date_hours, (EjsProc) date_hours, (EjsProc) date_set_hours);
    ejsBindAccess(ejs, prototype, ES_Date_milliseconds, (EjsProc) date_milliseconds, (EjsProc) date_set_milliseconds);
    ejsBindAccess(ejs, prototype, ES_Date_minutes, (EjsProc) date_minutes, (EjsProc) date_set_minutes);
    ejsBindAccess(ejs, prototype, ES_Date_month, (EjsProc) date_month, (EjsProc) date_set_month);
    ejsBindMethod(ejs, prototype, ES_Date_nextDay, (EjsProc) date_nextDay);
    ejsBindAccess(ejs, prototype, ES_Date_seconds, (EjsProc) date_seconds, (EjsProc) date_set_seconds);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCDate, (EjsProc) date_setUTCDate);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCFullYear, (EjsProc) date_setUTCFullYear);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCHours, (EjsProc) date_setUTCHours);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCMilliseconds, (EjsProc) date_setUTCMilliseconds);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCMinutes, (EjsProc) date_setUTCMinutes);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCMonth, (EjsProc) date_setUTCMonth);
    ejsBindMethod(ejs, prototype, ES_Date_setUTCSeconds, (EjsProc) date_setUTCSeconds);
    ejsBindAccess(ejs, prototype, ES_Date_time, (EjsProc) date_time, (EjsProc) date_set_time);
    ejsBindMethod(ejs, prototype, ES_Date_toJSON, (EjsProc) date_toJSON);
    ejsBindMethod(ejs, prototype, ES_Date_toISOString, (EjsProc) date_toISOString);
    ejsBindMethod(ejs, prototype, ES_Date_toString, (EjsProc) date_toString);
    ejsBindAccess(ejs, prototype, ES_Date_year, (EjsProc) date_year, (EjsProc) date_set_year);
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
