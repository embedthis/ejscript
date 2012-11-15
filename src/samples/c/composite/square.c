/*
    Square.c - Implementation for the Square class. Includes the loadable entry point for the Sample module.
  
    This sample provides the implementation for the Square class native. It defines the native C functions which
    are bound to the JavaScript methods for the Square class. The Square class is created by the runtime as are
    the scripted properties.
  
    The properties in the Square class are real JavaScript properties and are accessible as discrete JavaScript
    objects. See the composite class if you need a more compact representation for properties.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*
    Indent so that genDepend won't warn first time when this file doesn't exist
 */
    #include   "composite.slots.h"

/******************************************************************************/
/*
    The constructor's job is to initialize a bare object instance
  
    function Constructor(height: num, width: num)
 */
static EjsObj *constructor(Ejs *ejs, EjsObj *sp, int argc, EjsObj **argv)
{
    mprAssert(sp);
    mprAssert(argc == 2);

    mprLog(1, "Square()");

    ejsSetProperty(ejs, sp, ES_composite_Square_x, ESV(zero));
    ejsSetProperty(ejs, sp, ES_composite_Square_y, ESV(zero));
    ejsSetProperty(ejs, sp, ES_composite_Square_height, argv[0]);
    ejsSetProperty(ejs, sp, ES_composite_Square_width, argv[1]);
    return sp;
}


/*
    Compute the area of the square
  
    function area(): Number
 */
static EjsNumber *area(Ejs *ejs, EjsObj *sp, int argc, EjsObj **argv)
{
    int     height, width;

    mprAssert(argc == 0);

    mprLog(1, "Square.area()");

    height = ejsGetInt(ejs, ejsGetProperty(ejs, sp, ES_composite_Square_height));
    width = ejsGetInt(ejs, ejsGetProperty(ejs, sp, ES_composite_Square_width));

    return ejsCreateNumber(ejs, height * width);
}


/******************************************************************************/

static int configureSampleTypes(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    mprLog(1, "Loading Sample module\n");

    /*
        Get the Square class object. This will be created from the mod file for us.
     */
    type = ejsGetTypeByName(ejs, N("composite", "Square"));
    if (type == 0) {
        mprError("Can't find type Square");
        return MPR_ERR;
    }
    /*
        Bind the C functions to the JavaScript functions. We use the slot definitions generated
        by ejsmod from Square.es.
     */
    prototype = type->prototype;
    ejsBindConstructor(ejs, type, constructor);
    ejsBindMethod(ejs, prototype, ES_composite_Square_area, area);
    return 0;
}


/*
    Square loadable module entry point. This will be called by the Ejscript loader 
    after the Square.mod file is loaded and before Square initializers are run. 
  
    Module entry points be named [NAME]ModuleInit where "[NAME]" is the name of 
    the module with the first letter mapped to lower case and with any "." characters 
    converted to underscores.
 */
int composite_Init(Ejs *ejs, MprModule *module)
{
    return ejsAddNativeModule(ejs, "composite", configureSampleTypes, _ES_CHECKSUM_composite, EJS_LOADER_ETERNAL);
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
