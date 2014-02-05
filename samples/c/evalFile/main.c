/*
    main.c - Simple main program to load and run a module
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Code ************************************/

int main(int argc, char **argv)
{
    /*
        Load the script and evaluate
     */
    if (ejsEvalFile("test.es") < 0) {
        fprintf(stderr, "Cannot load test.es\n");
        exit(1);
    }
    return 0;
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
