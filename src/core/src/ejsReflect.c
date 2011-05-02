/**
    ejsReflect.c - Reflection class and API

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

#if UNUSED
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
    return rp;
}


/*
    Get the base class of the object.

    function get base(): Object
 */
static EjsObj *ref_base(Ejs *ejs, EjsReflect *rp, int argc, EjsObj **argv)
{
    EjsObj      *vp;

    vp = rp->subject;
    if (ejsIsType(ejs, vp)) {
        return (((EjsType*) vp)->baseType);
    }
    if (vp->type == 0) {
        return S(null);
    }
    return vp->type;
}


/*
    function get isType(): Boolean
 */
static EjsBoolean *ref_isType(Ejs *ejs, EjsReflect *rp, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, ejsIsType(ejs, rp->subject));
}


/*
    Get the type of the object.

    function get type(): Object
 */
static EjsType *ref_type(Ejs *ejs, EjsReflect *rp, int argc, EjsObj **argv)
{
    EjsObj      *vp;

    vp = rp->subject;
    if (vp->type == 0) {
        return S(null);
    }
    return vp->type;
}


/*
    Return the type name of a var as a string. If the var is a type, get the base type.
 */
EjsString *ejsGetTypeName(Ejs *ejs, EjsObj *vp)
{
    EjsType     *type;

    if (vp == 0) {
        return S(undefined);
    }
    type = (EjsType*) vp->type;
    if (type == 0) {
        return S(null);
    }
    return ejsCreateStringFromAsc(ejs, type->qname.name);
}


/*
    Get the type name of a type object

    function get name(): String
 */
static EjsString *ref_name(Ejs *ejs, EjsReflect *rp, int argc, EjsObj **argv)
{
    EjsType     *type;

    type = (EjsType*) rp->subject;
    if (!ejsIsType(ejs, type)) {
        return S(empty);
    }
    return ejsCreateStringFromAsc(ejs, type->qname.name);
}

/*********************************** Globals **********************************/

/*
    function get typeOf(obj): String
 */
static EjsString *ref_typeOf(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    mprAssert(argc >= 1);
    return ejsGetTypeName(ejs, argv[0]);
}


/*
    Get the ecma "typeof" value for an object. Unfortunately, typeof is pretty lame.
 */
EjsString *ejsGetTypeOf(Ejs *ejs, EjsObj *vp)
{
    if (vp == S(undefined)) {
        return ejsCreateStringFromAsc(ejs, "undefined");

    } else if (ejsIs(ejs, vp, Null)) {
        /* Yea - I know, ECMAScript is broken */
        return ejsCreateStringFromAsc(ejs, "object");

    } if (ejsIs(ejs, vp, Boolean)) {
        return ejsCreateStringFromAsc(ejs, "boolean");

    } else if (ejsIs(ejs, vp, Number)) {
        return ejsCreateStringFromAsc(ejs, "number");

    } else if (ejsIs(ejs, vp, String)) {
        return ejsCreateStringFromAsc(ejs, "string");

    } else if (ejsIs(ejs, vp, Function)) {
        return ejsCreateStringFromAsc(ejs, "function");
               
    } else {
        return ejsCreateStringFromAsc(ejs, "object");
    }
}


/*********************************** Factory **********************************/

static void manageReflect(EjsReflect *rp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(rp->subject);
    }
}


void ejsConfigureReflectType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *prototype;

    type = ejsConfigureNativeType(ejs, N("ejs", "Reflect"), sizeof(EjsReflect), manageReflect, EJS_OBJ_HELPERS);
    prototype = type->prototype;

    ejsBindConstructor(ejs, type, (EjsProc) ref_Reflect);
    ejsBindMethod(ejs, prototype, ES_Reflect_base, (EjsProc) ref_base);
    ejsBindMethod(ejs, prototype, ES_Reflect_isType, (EjsProc) ref_isType);
    ejsBindMethod(ejs, prototype, ES_Reflect_name, (EjsProc) ref_name);
#if UNUSED
    ejsBindMethod(ejs, prototype, ES_Reflect_isPrototype, (EjsProc) ref_isPrototype);
    ejsBindMethod(ejs, prototype, ES_Reflect_prototype, (EjsProc) ref_prototype);
#endif
    ejsBindMethod(ejs, prototype, ES_Reflect_type, (EjsProc) ref_type);
    ejsBindFunction(ejs, ejs->globalBlock, ES_typeOf, (EjsProc) ref_typeOf);
}

#else
void dummy_Reflection() {}
#endif


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
