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
static EjsObj *math_abs(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    //  TODO - can a null slip through?
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) fabs(ejsGetNumber(ejs, argv[0])));
}


/*
    function acos(value: Number): Number
 */
static EjsObj *math_acos(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprNumber   value;
    
    value = ejsGetNumber(ejs, argv[0]);
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) acos(ejsGetNumber(ejs, argv[0])));
}


/*
    function asin(value: Number): Number
 */
static EjsObj *math_asin(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) asin(ejsGetNumber(ejs, argv[0])));
}


/*
    function atan(value: Number): Number
 */
static EjsObj *math_atan(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) atan(ejsGetNumber(ejs, argv[0])));
}


/*
    function atan2(x: Number, y: Number): Number
 */
static EjsObj *math_atan2(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) atan2(ejsGetNumber(ejs, argv[0]), ejsGetNumber(ejs, argv[1])));
}


/*
    function ceil(value: Number): Number
 */
static EjsObj *math_ceil(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) ceil(ejsGetNumber(ejs, argv[0])));
}


/*
    function cos(value: Number): Number
 */
static EjsObj *math_cos(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) cos(ejsGetNumber(ejs, argv[0])));
}


/*
    function exp(value: Number): Number
 */
static EjsObj *math_exp(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) exp(ejsGetNumber(ejs, argv[0])));
}


/*
    function floor(value: Number): Number
 */
static EjsObj *math_floor(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) floor(ejsGetNumber(ejs, argv[0])));
}


/*
    function log10(value: Number): Number
 */
static EjsObj *math_log10(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) log10(ejsGetNumber(ejs, argv[0])));
}


/*
    function log(value: Number): Number
 */
static EjsObj *math_log(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) log(ejsGetNumber(ejs, argv[0])));
}


/*
    function max(x: Number, y: Number): Number
 */
static EjsObj *math_max(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprNumber   x, y;
    
    x = ejsGetNumber(ejs, argv[0]);
    y = ejsGetNumber(ejs, argv[1]);
    if (x > y) {
        return argv[0];
    }
    return argv[1];
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
static EjsObj *math_pow(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
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
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) result);
}


/*
    function random(value: Number): Number
 */
static EjsObj *math_random(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprNumber   value;
    uint        uvalue;
    static int  initialized = 0;
    
    if (!initialized) {
#if WIN
        uint seed = (uint) time(0);
        srand(seed);
#elif !MACOSX && !VXWORKS
        srandom(time(0));
#endif
        initialized = 1;
    }
    
#if WIN
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
    return (EjsObj*) ejsCreateNumber(ejs, value);
}


/*
    function round(value: Number): Number
 */
static EjsObj *math_round(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprNumber   n;

    n = ejsGetNumber(ejs, argv[0]);
    if (-0.5 <= n && n < 0) {
        n = -0.0;
    } else {
        n += 0.5;
    }
    return (EjsObj*) ejsCreateNumber(ejs, floor(n));
}


/*
    function sin(value: Number): Number
 */
static EjsObj *math_sin(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) sin(ejsGetNumber(ejs, argv[0])));
}


/*
    function sqrt(value: Number): Number
 */
static EjsObj *math_sqrt(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) sqrt(ejsGetNumber(ejs, argv[0])));
}


/*
    function tan(value: Number): Number
 */
static EjsObj *math_tan(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) tan(ejsGetNumber(ejs, argv[0])));
}


/*********************************** Factory **********************************/

void ejsConfigureMathType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsConfigureNativeType(ejs, N("ejs", "Math"), sizeof(EjsObj), NULL, EJS_OBJ_HELPERS);
#if UNUSED
    ejsSetSpecialType(ejs, S_Math, type);
#endif
    ejsBindMethod(ejs, type, ES_Math_abs, (EjsProc) math_abs);
    ejsBindMethod(ejs, type, ES_Math_acos, (EjsProc) math_acos);
    ejsBindMethod(ejs, type, ES_Math_asin, (EjsProc) math_asin);
    ejsBindMethod(ejs, type, ES_Math_atan, (EjsProc) math_atan);
    ejsBindMethod(ejs, type, ES_Math_atan2, (EjsProc) math_atan2);
    ejsBindMethod(ejs, type, ES_Math_ceil, (EjsProc) math_ceil);
    ejsBindMethod(ejs, type, ES_Math_cos, (EjsProc) math_cos);
    ejsBindMethod(ejs, type, ES_Math_exp, (EjsProc) math_exp);
    ejsBindMethod(ejs, type, ES_Math_floor, (EjsProc) math_floor);
    ejsBindMethod(ejs, type, ES_Math_log, (EjsProc) math_log);
    ejsBindMethod(ejs, type, ES_Math_log10, (EjsProc) math_log10);
    ejsBindMethod(ejs, type, ES_Math_max, (EjsProc) math_max);
    ejsBindMethod(ejs, type, ES_Math_min, (EjsProc) math_min);
    ejsBindMethod(ejs, type, ES_Math_pow, (EjsProc) math_pow);
    ejsBindMethod(ejs, type, ES_Math_random, (EjsProc) math_random);
    ejsBindMethod(ejs, type, ES_Math_round, (EjsProc) math_round);
    ejsBindMethod(ejs, type, ES_Math_sin, (EjsProc) math_sin);
    ejsBindMethod(ejs, type, ES_Math_sqrt, (EjsProc) math_sqrt);
    ejsBindMethod(ejs, type, ES_Math_tan, (EjsProc) math_tan);
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
