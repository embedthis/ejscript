/**
    ejsMath.c - Math type class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"
#include     <math.h>

/**************************** Forward Declarations ****************************/

#define fixed(n) ((int64) (floor(n)))

/******************************************************************************/
/*
    function abs(value: Number): Number
 */
static EjsNumber *math_abs(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    //  TODO - can a null slip through?
    return ejsCreateNumber(ejs, (MprNumber) fabs(ejsGetNumber(ejs, argv[0])));
}


/*
    function acos(value: Number): Number
 */
static EjsNumber *math_acos(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) acos(ejsGetNumber(ejs, argv[0])));
}


/*
    function asin(value: Number): Number
 */
static EjsNumber *math_asin(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) asin(ejsGetNumber(ejs, argv[0])));
}


/*
    function atan(value: Number): Number
 */
static EjsNumber *math_atan(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) atan(ejsGetNumber(ejs, argv[0])));
}


/*
    function atan2(x: Number, y: Number): Number
 */
static EjsNumber *math_atan2(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) atan2(ejsGetNumber(ejs, argv[0]), ejsGetNumber(ejs, argv[1])));
}


/*
    function ceil(value: Number): Number
 */
static EjsNumber *math_ceil(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) ceil(ejsGetNumber(ejs, argv[0])));
}


/*
    function cos(value: Number): Number
 */
static EjsNumber *math_cos(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) cos(ejsGetNumber(ejs, argv[0])));
}


/*
    function exp(value: Number): Number
 */
static EjsNumber *math_exp(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) exp(ejsGetNumber(ejs, argv[0])));
}


/*
    function floor(value: Number): Number
 */
static EjsNumber *math_floor(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) floor(ejsGetNumber(ejs, argv[0])));
}


/*
    function log10(value: Number): Number
 */
static EjsNumber *math_log10(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) log10(ejsGetNumber(ejs, argv[0])));
}


/*
    function log(value: Number): Number
 */
static EjsNumber *math_log(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) log(ejsGetNumber(ejs, argv[0])));
}


/*
    function max(x: Number, y: Number): Number
 */
static EjsNumber *math_max(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprNumber   x, y;
    
    x = ejsGetNumber(ejs, argv[0]);
    y = ejsGetNumber(ejs, argv[1]);
    if (x > y) {
        return (EjsNumber*) argv[0];
    }
    return (EjsNumber*) argv[1];
}


/*
    function min(value: Number): Number
 */
static EjsObj *math_min(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprNumber   x, y;
    
    x = ejsGetNumber(ejs, argv[0]);
    y = ejsGetNumber(ejs, argv[1]);
    if (x < y) {
        return argv[0];
    }
    return argv[1];
}


/*
    function pow(x: Number, y: Number): Number
 */
static EjsNumber *math_pow(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprNumber   x, y, result;
    
    x = ejsGetNumber(ejs, argv[0]);
    y = ejsGetNumber(ejs, argv[1]);
    result = pow(x, y);
#if CYGWIN
    /* Cygwin computes (0.0 / -1) == -Infinity */
    if (result < 0 && x == 0.0) {
        result = -result;
    }
#endif
    return ejsCreateNumber(ejs, (MprNumber) result);
}


/*
    function random(value: Number): Number
 */
static EjsNumber *math_random(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprNumber   value;
    uint        uvalue;
    static int  initialized = 0;
    
    if (!initialized) {
#if WINDOWS
        uint seed = (uint) time(0);
        srand(seed);
#elif !MACOSX && !VXWORKS
        srandom(time(0));
#endif
        initialized = 1;
    }
    
#if WINDOWS
{
    errno_t rand_s(uint *value);
    rand_s(&uvalue);
}
#elif LINUX
    uvalue = random();
#elif MACOSX
    uvalue = arc4random();
#else
{
    int64   data[16];
    int     i;
    mprGetRandomBytes((char*) data, sizeof(data), 0);
    uvalue = 0;
    for (i = 0; i < sizeof(data) / sizeof(int64); i++) {
        uvalue += data[i];
    }
}
#endif
    value = ((MprNumber) (uvalue & 0x7FFFFFFF)) / MAXINT;
    return ejsCreateNumber(ejs, value);
}


/*
    function round(value: Number): Number
 */
static EjsNumber *math_round(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprNumber   n;

    n = ejsGetNumber(ejs, argv[0]);
    if (-0.5 <= n && n < 0) {
        n = -0.0;
    } else {
        n += 0.5;
    }
    return ejsCreateNumber(ejs, floor(n));
}


/*
    function sin(value: Number): Number
 */
static EjsNumber *math_sin(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) sin(ejsGetNumber(ejs, argv[0])));
}


/*
    function sqrt(value: Number): Number
 */
static EjsNumber *math_sqrt(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) sqrt(ejsGetNumber(ejs, argv[0])));
}


/*
    function tan(value: Number): Number
 */
static EjsNumber *math_tan(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) tan(ejsGetNumber(ejs, argv[0])));
}


/*********************************** Factory **********************************/

PUBLIC void ejsConfigureMathType(Ejs *ejs)
{
    EjsType     *type;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "Math"), sizeof(EjsPot), ejsManagePot, EJS_TYPE_POT)) == 0) {
        return;
    }
    ejsBindMethod(ejs, type, ES_Math_abs, math_abs);
    ejsBindMethod(ejs, type, ES_Math_acos, math_acos);
    ejsBindMethod(ejs, type, ES_Math_asin, math_asin);
    ejsBindMethod(ejs, type, ES_Math_atan, math_atan);
    ejsBindMethod(ejs, type, ES_Math_atan2, math_atan2);
    ejsBindMethod(ejs, type, ES_Math_ceil, math_ceil);
    ejsBindMethod(ejs, type, ES_Math_cos, math_cos);
    ejsBindMethod(ejs, type, ES_Math_exp, math_exp);
    ejsBindMethod(ejs, type, ES_Math_floor, math_floor);
    ejsBindMethod(ejs, type, ES_Math_log, math_log);
    ejsBindMethod(ejs, type, ES_Math_log10, math_log10);
    ejsBindMethod(ejs, type, ES_Math_max, math_max);
    ejsBindMethod(ejs, type, ES_Math_min, math_min);
    ejsBindMethod(ejs, type, ES_Math_pow, math_pow);
    ejsBindMethod(ejs, type, ES_Math_random, math_random);
    ejsBindMethod(ejs, type, ES_Math_round, math_round);
    ejsBindMethod(ejs, type, ES_Math_sin, math_sin);
    ejsBindMethod(ejs, type, ES_Math_sqrt, math_sqrt);
    ejsBindMethod(ejs, type, ES_Math_tan, math_tan);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
