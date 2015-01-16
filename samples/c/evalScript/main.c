/*
    main.c - Simple main program to load and run a module
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Code ************************************/

MAIN(evalScript, int argc, char **argv, char **envp)
{
    if (argc != 2) {
        fprintf(stderr, "usage: main \"literalScript\"\n");
        exit(1);
    }

    /*
        Evaluate the command line
     */
    if (ejsEvalScript(argv[1]) < 0) {
        fprintf(stderr, "Cannot evaluate %s\n", argv[1]);
        exit(1);
    }
    return 0;
}

/*
    @copy   default

    Copyright (c) Embedthis Software. All Rights Reserved.

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
