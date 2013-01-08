/*
 *  romFiles -- Compiled Files
 */
#include "mpr.h"

#if BIT_ROM
static uchar _file_1[] = {
    112,114,105,110,116, 40, 39, 72,101,108,108,111, 32, 87,111,114,
    108,100, 39, 41, 10,
    0 };

MprRomInode romFiles[] = {
    { "hello.es", _file_1, 21, 1 },
    { 0, 0, 0, 0 },
};
#endif /* BIT_ROM */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
