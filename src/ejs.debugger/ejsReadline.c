/*
    ejsReadline.c - Readline editing module

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

#if ME_COMPILER_HAS_LIB_EDIT
    #include <histedit.h>
#endif

/*********************************** Locals ***********************************/

#if ME_COMPILER_HAS_LIB_EDIT
static History *cmdHistory;
static EditLine *eh; 
static cchar *prompt;
#endif

/**************************** Forward Declarations ****************************/

/************************************* Code ***********************************/

#if ME_COMPILER_HAS_LIB_EDIT


EditLine *ejsReadlineOpen(MprCtx ctx)
{
    EditLine    *e;
    HistEvent   ev; 

    cmdHistory = history_init(); 
    history(cmdHistory, &ev, H_SETSIZE, 100); 
    e = el_init("ejs", stdin, stdout, stderr); 
    el_set(e, EL_EDITOR, "vi");
    el_set(e, EL_HIST, history, cmdHistory);
    el_source(e, NULL);
    return e;
}


static cchar *issuePrompt(EditLine *e) {
    return prompt;
}


char *ejsReadline(MprCtx ctx, cchar *msg) 
{ 
    HistEvent   ev; 
    cchar       *str; 
    char        *result;
    int         len, count; 
 
    if (eh == NULL) { 
        eh = ejsReadlineOpen(ctx);
    }
    prompt = msg;
    el_set(eh, EL_PROMPT, issuePrompt);
    str = el_gets(eh, &count); 
    if (str && count > 0) { 
        result = strdup(str); 
        len = strlen(result);
        if (result[len - 1] == '\n') {
            result[len - 1] = '\0'; 
        }
        count = history(cmdHistory, &ev, H_ENTER, result); 
        return result; 
    }  
    return NULL; 
} 

#else /* ME_COMPILER_HAS_LIB_EDIT */

char *ejsReadline(MprCtx ctx, cchar *msg)
{
    char    buf[MPR_MAX_STRING];

    printf("%s", msg);
    if (fgets(buf, sizeof(buf) - 1, stdin) == 0) {
        return NULL;
    }
    return strdup(buf);
}
#endif /* ME_COMPILER_HAS_LIB_EDIT */

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
