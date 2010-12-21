/**
    ejsBlock.c - Lexical block

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

//  MOB -- rename to LexBlock
/*********************************** Helpers **********************************/

EjsBlock *ejsCloneBlock(Ejs *ejs, EjsBlock *src, bool deep)
{
    EjsBlock    *dest;

    dest = (EjsBlock*) ejsClonePot(ejs, (EjsObj*) src, deep);

    dest->nobind = src->nobind;
    dest->scope = src->scope;
#if MOB && OPT
    //  This could work if a bit is set in EjsBlock to say inherited
    dest->namespaces = src->namespaces;
#else
    mprInitList(&dest->namespaces);
    mprCopyList(&dest->namespaces, &src->namespaces);
#endif
    return dest;
}


#if BLD_DEBUG
void ejsSetBlockLocation(EjsBlock *block, EjsLoc *loc)
{
    block->loc = *loc; 
}
#endif

/********************************* Namespaces *******************************/

void ejsResetBlockNamespaces(Ejs *ejs, EjsBlock *block)
{
    mprClearList(&block->namespaces);
}


int ejsGetNamespaceCount(EjsBlock *block)
{
    mprAssert(block);
    return block->namespaces.length;
}


void ejsPopBlockNamespaces(EjsBlock *block, int count)
{
    mprAssert(block);
    mprAssert(block->namespaces.length >= count);

    block->namespaces.length = count;
}


int ejsAddNamespaceToBlock(Ejs *ejs, EjsBlock *block, EjsNamespace *nsp)
{
    mprAssert(block);

    if (nsp == 0) {
        ejsThrowTypeError(ejs, "Not a namespace");
        return EJS_ERR;
    }
#if UNUSED && KEEP
    /* Namespaces must be ordered and so can't do this */ 
    for (next = 0; ((existing = (EjsNamespace*) mprGetNextItem(&block->namespaces, &next)) != 0); ) {
        if (existing->value == nsp->value) {
            break;
        }
    }
    if (existing == NULL) {
        mprAddItem(&block->namespaces, nsp);
    }
#else
        mprAddItem(&block->namespaces, nsp);
#endif
    return 0;
}


/*
    Inherit namespaces from base types. Only inherit protected.
 */
void ejsInheritBaseClassNamespaces(Ejs *ejs, EjsType *type, EjsType *baseType)
{
    EjsNamespace    *nsp, *existing;
    EjsBlock        *block;
    MprList         *baseNamespaces;
    int             next, i;

    block = &type->constructor.block;
    baseNamespaces = &baseType->constructor.block.namespaces;

    if (baseNamespaces) {
        for (next = 0; ((nsp = (EjsNamespace*) mprGetNextItem(baseNamespaces, &next)) != 0); ) {
            //  MOB -- must be a better way to do this?
            if (ejsContainsString(ejs, nsp->value, ejs->commaProtString)) {
                for (i = 0; ((existing = (EjsNamespace*) mprGetNextItem(&block->namespaces, &i)) != 0); ) {
                    if (existing->value == nsp->value) {
                        break;
                    }
                }
                //  MOB -- debug to see if duplicates found 
                mprAssert(existing == NULL);
                if (existing == NULL) {
                    mprInsertItemAtPos(&block->namespaces, next - 1, nsp);
                }
            }
        }
    }
}


/*************************************** Factory ***********************************/

EjsBlock *ejsCreateBlock(Ejs *ejs, int size)
{
    EjsBlock        *block;

    block = (EjsBlock*) ejsCreatePot(ejs, ejs->blockType, size);
    if (block == 0) {
        return 0;
    }
    block->pot.shortScope = 1;
    block->pot.isBlock = 1;
    mprInitList(&block->namespaces);
#if BLD_DEBUG
    block->pot.mem = MPR_GET_MEM(block);
#endif
    return block;
}


void ejsManageBlock(EjsBlock *block, int flags)
{
    EjsObj          *item;
    EjsBlock        *b;
    int             next;

    if (block) {
        if (flags & MPR_MANAGE_MARK) {
            ejsManagePot(block, flags);
            mprMark(block->prevException);

            if (block->namespaces.length > 0) {
                mprMark(block->namespaces.items);
                for (next = 0; ((item = (EjsObj*) mprGetNextItem(&block->namespaces, &next)) != 0); ) {
                    mprMark(item);
                }
            }
            for (b = block->scope; b; b = b->scope) {
                mprMark(b);
            }
#if UNUSED
            //  TODO MOB - this should not be required as GC in mark() follows the block caller/prev chain
            for (b = block->prev; b; b = b->prev) {
                mprMark(b);
            }
#endif
            /*
                Don't mark stack as the VM will mark that
                Don't mark locations as they are always inherited from a frame which will mark them for us
             */
        }
    }
}


void ejsCreateBlockType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsCreateNativeType(ejs, N("ejs", "Block"), ES_Block, sizeof(EjsBlock), ejsManageBlock, EJS_POT_HELPERS);
    ejs->blockType = type;
    type->constructor.block.pot.shortScope = 1;

    type->helpers.clone = (EjsCloneHelper) ejsCloneBlock;
    ejs->commaProtString = ejsCreateStringFromAsc(ejs, ",protected");
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
