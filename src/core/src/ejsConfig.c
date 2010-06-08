/*
    ejsConfig.c -- Config class
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Methods **********************************/

void ejsConfigureConfigType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *vp;
    char        version[16];

    type = ejsConfigureNativeType(ejs, "ejs", "Config", sizeof(EjsObj));
    vp = (EjsObj*) type;
    ejsSetProperty(ejs, vp, ES_Config_Debug, BLD_DEBUG ? (EjsObj*) ejs->trueValue: (EjsObj*) ejs->falseValue);
    ejsSetProperty(ejs, vp, ES_Config_CPU, (EjsObj*) ejsCreateString(ejs, BLD_HOST_CPU));
    ejsSetProperty(ejs, vp, ES_Config_OS, (EjsObj*) ejsCreateString(ejs, BLD_OS));
    ejsSetProperty(ejs, vp, ES_Config_Product, (EjsObj*) ejsCreateString(ejs, BLD_PRODUCT));

    ejsSetProperty(ejs, vp, ES_Config_Title, (EjsObj*) ejsCreateString(ejs, BLD_NAME));
    mprSprintf(ejs, version, sizeof(version), "%s-%s", BLD_VERSION, BLD_NUMBER);
    ejsSetProperty(ejs, vp, ES_Config_Version, (EjsObj*) ejsCreateString(ejs, version));

#if BLD_WIN_LIKE
{
    char    *path;

    path = mprGetAppDir(ejs);
    ejsSetProperty(ejs, vp, ES_Config_BinDir, (EjsObj*) ejsCreateString(ejs, path));
    ejsSetProperty(ejs, vp, ES_Config_ModDir, (EjsObj*) ejsCreateString(ejs, path));
    ejsSetProperty(ejs, vp, ES_Config_LibDir, (EjsObj*) ejsCreateString(ejs, path));
}
#else
    ejsSetProperty(ejs, vp, ES_Config_BinDir, (EjsObj*) ejsCreateString(ejs, BLD_BIN_PREFIX));
    ejsSetProperty(ejs, vp, ES_Config_ModDir, (EjsObj*) ejsCreateString(ejs, BLD_MOD_PREFIX));
    ejsSetProperty(ejs, vp, ES_Config_LibDir, (EjsObj*) ejsCreateString(ejs, BLD_LIB_PREFIX));
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
