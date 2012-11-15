/**
    ecState.c - Manage state for the parser

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejsCompiler.h"

/************************************ Code ************************************/

static void manageState(EcState *state, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(state->next);
        mprMark(state->code);
        mprMark(state->currentClass);
        mprMark(state->currentClassNode);
        mprMark(state->currentFunction);
        mprMark(state->currentFunctionNode);
        mprMark(state->currentModule);
        mprMark(state->currentObjectNode);
        mprMark(state->defaultNamespace);
        mprMark(state->instanceCodeBuf);
        mprMark(state->letBlock);
        mprMark(state->letBlockNode);
        mprMark(state->nspace);
        mprMark(state->optimizedLetBlock);
        mprMark(state->staticCodeBuf);
        mprMark(state->topVarBlockNode);
        mprMark(state->varBlock);
    }
}


/*
    Enter a new level. For the parser, this is a new production rule. For the ASP processor or code generator, 
    it is a new AST node. Push old state and setup a new production state
 */
PUBLIC int ecEnterState(EcCompiler *cp)
{
    EcState     *state;

    if ((state = mprAllocBlock(sizeof(EcState), MPR_ALLOC_ZERO | MPR_ALLOC_MANAGER)) == 0) {
        return MPR_ERR_MEMORY;
    }
    mprSetManager(state, (MprManager) manageState);
    if (cp->state) {
        *state = *cp->state;
    }
    state->next = cp->state;
    cp->state = state;
    return 0;
}


PUBLIC void ecLeaveState(EcCompiler *cp)
{
    cp->state = cp->state->next;
}


/*
    Leave a level. Pop the state and pass back the current node.
 */
PUBLIC EcNode *ecLeaveStateWithResult(EcCompiler *cp, EcNode *np)
{
    ecLeaveState(cp);
    if (cp->fatalError || cp->error) {
        return 0;
    }
    return np;
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
    details at: http://embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
