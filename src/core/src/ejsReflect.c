/**
    ejsReflect.c - Reflection class and API

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Methods **********************************/
/*
    Constructor

    public function Reflect(o: Object)
 */
static EjsObj *ref_Reflect(Ejs *ejs, EjsReflect *rp, int argc,  EjsObj **argv)
{
    mprAssert(argc == 1);
    rp->subject = argv[0];
    mprAssert(rp->subject->type);
    return (EjsObj*) rp;
}


/*
    Get the base class of the object.

    function get base(): Object
 */
static EjsObj *ref_base(Ejs *ejs, EjsReflect *rp, int argc, EjsObj **argv)
{
    EjsObj      *vp;

    vp = rp->subject;
    if (ejsIsType(vp)) {
        return (EjsObj*) (((EjsType*) vp)->baseType);
    }
    if (vp->type == 0) {
        return ejs->nullValue;
    }
    return (EjsObj*) vp->type;
}


/*
    function get isType(): Boolean
 */
static EjsObj *ref_isType(Ejs *ejs, EjsReflect *rp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, ejsIsType(rp->subject));
}


/*
    Get the type of the object.

    function get type(): Object
 */
static EjsObj *ref_type(Ejs *ejs, EjsReflect *rp, int argc, EjsObj **argv)
{
    EjsObj      *vp;

    vp = rp->subject;
    if (vp->type == 0) {
        return ejs->nullValue;
    }
    return (EjsObj*) vp->type;
}


/*
    Return the type name of a var as a string. If the var is a type, get the base type.
 */
EjsObj *ejsGetTypeName(Ejs *ejs, EjsObj *vp)
{
    EjsType     *type;

    if (vp == 0) {
        return ejs->undefinedValue;
    }
    type = (EjsType*) vp->type;
    if (type == 0) {
        return ejs->nullValue;
    }
    return (EjsObj*) ejsCreateString(ejs, type->qname.name);
}


/*
    Get the type name of a type object

    function get name(): String
 */
static EjsObj *ref_name(Ejs *ejs, EjsReflect *rp, int argc, EjsObj **argv)
{
    EjsType     *type;

    type = (EjsType*) rp->subject;
    if (!ejsIsType(type)) {
        ejsThrowArgError(ejs, "Object is not a type");
        return 0;
    }
    return (EjsObj*) ejsCreateString(ejs, type->qname.name);
}

/*********************************** Globals **********************************/

/*
    function get typeOf(obj): String
 */
static EjsObj *ref_typeOf(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    mprAssert(argc >= 1);
    return (EjsObj*) ejsGetTypeName(ejs, argv[0]);
}


/*
    Get the ecma "typeof" value for an object. Unfortunately, typeof is pretty lame.
 */
EjsObj *ejsGetTypeOf(Ejs *ejs, EjsObj *vp)
{
    if (vp == ejs->undefinedValue) {
        return (EjsObj*) ejsCreateString(ejs, "undefined");

    } else if (vp == ejs->nullValue) {
        /* Yea - I know, ECMAScript is broken */
        return (EjsObj*) ejsCreateString(ejs, "object");

    } if (ejsIsBoolean(vp)) {
        return (EjsObj*) ejsCreateString(ejs, "boolean");

    } else if (ejsIsNumber(vp)) {
        return (EjsObj*) ejsCreateString(ejs, "number");

    } else if (ejsIsString(vp)) {
        return (EjsObj*) ejsCreateString(ejs, "string");

    } else if (ejsIsFunction(vp)) {
        return (EjsObj*) ejsCreateString(ejs, "function");
               
    } else if (ejsIsType(vp)) {
        /* Pretend it is a constructor function */
        return (EjsObj*) ejsCreateString(ejs, "function");
               
    } else {
        return (EjsObj*) ejsCreateString(ejs, "object");
    }
}


/*********************************** Helpers **********************************/

static void markReflectVar(Ejs *ejs, EjsReflect *rp)
{
    //  MOB - probably not needed
    ejsMarkObject(ejs, (EjsObj*) rp);

    if (rp->subject) {
        ejsMark(ejs, rp->subject);
    }
}

/*********************************** Factory **********************************/

void ejsConfigureReflectType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsConfigureNativeType(ejs, EJS_EJS_NAMESPACE, "Reflect", sizeof(EjsReflect));

    type->helpers->mark = (EjsMarkHelper) markReflectVar;

    ejsBindMethod(ejs, type, ES_Reflect_Reflect, (EjsProc) ref_Reflect);
    ejsBindMethod(ejs, type, ES_Reflect_base, (EjsProc) ref_base);
    ejsBindMethod(ejs, type, ES_Reflect_isType, (EjsProc) ref_isType);
    ejsBindMethod(ejs, type, ES_Reflect_name, (EjsProc) ref_name);
    ejsBindMethod(ejs, type, ES_Reflect_type, (EjsProc) ref_type);
    ejsBindFunction(ejs, ejs->globalBlock, ES_typeOf, (EjsProc) ref_typeOf);
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
