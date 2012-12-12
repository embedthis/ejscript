/**
    ejsBlock.c - Lexical block

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Helpers **********************************/

PUBLIC EjsBlock *ejsCloneBlock(Ejs *ejs, EjsBlock *src, bool deep)
{
    EjsBlock    *dest;

    dest = (EjsBlock*) ejsClonePot(ejs, src, deep);

    dest->nobind = src->nobind;
    dest->scope = src->scope;
    mprInitList(&dest->namespaces, MPR_LIST_STABLE);
    mprCopyListContents(&dest->namespaces, &src->namespaces);
    return dest;
}


/********************************* Namespaces *******************************/

PUBLIC void ejsResetBlockNamespaces(Ejs *ejs, EjsBlock *block)
{
    mprClearList(&block->namespaces);
}


PUBLIC int ejsGetNamespaceCount(EjsBlock *block)
{
    assure(block);
    return block->namespaces.length;
}


PUBLIC void ejsPopBlockNamespaces(EjsBlock *block, int count)
{
    assure(block);
    assure(block->namespaces.length >= count);

    block->namespaces.length = count;
}


PUBLIC int ejsAddNamespaceToBlock(Ejs *ejs, EjsBlock *block, EjsNamespace *nsp)
{
    assure(block);

    if (nsp == 0) {
        ejsThrowTypeError(ejs, "Not a namespace");
        return EJS_ERR;
    }
    mprAddItem(&block->namespaces, nsp);
    return 0;
}


/*
    Inherit namespaces from base types. Only inherit protected.
 */
PUBLIC void ejsInheritBaseClassNamespaces(Ejs *ejs, EjsType *type, EjsType *baseType)
{
    EjsNamespace    *nsp, *existing;
    EjsBlock        *block;
    MprList         *baseNamespaces;
    int             next, i;

    block = &type->constructor.block;
    baseNamespaces = &baseType->constructor.block.namespaces;

    if (baseNamespaces) {
        for (next = 0; ((nsp = (EjsNamespace*) mprGetNextItem(baseNamespaces, &next)) != 0); ) {
            //  OPT -- must be a better way to do this?
            if (ejsContainsString(ejs, nsp->value, ESV(commaProt)) >= 0) {
                for (i = 0; ((existing = (EjsNamespace*) mprGetNextItem(&block->namespaces, &i)) != 0); ) {
                    if (existing->value == nsp->value) {
                        break;
                    }
                }
                //  OPT -- debug to see if duplicates found 
                assure(existing == NULL);
                if (existing == NULL) {
                    mprInsertItemAtPos(&block->namespaces, next - 1, nsp);
                }
            }
        }
    }
}


/*************************************** Factory ***********************************/

PUBLIC EjsBlock *ejsCreateBlock(Ejs *ejs, int size)
{
    EjsBlock        *block;

    if ((block = ejsCreatePot(ejs, ESV(Block), size)) == 0) {
        return 0;
    }
    block->pot.shortScope = 1;
    block->pot.isBlock = 1;
    mprInitList(&block->namespaces, MPR_LIST_STABLE);
    return block;
}


PUBLIC void ejsManageBlock(EjsBlock *block, int flags)
{
    EjsObj          *item;
    EjsBlock        *b;
    int             next;

    if (block) {
        if (flags & MPR_MANAGE_MARK) {
            ejsManagePot(block, flags);
            mprMark(block->prevException);

            /*
                Must mark each item of the list as the list itself is not allocated
             */
            mprMark(block->namespaces.items);
            for (next = 0; ((item = (EjsObj*) mprGetNextItem(&block->namespaces, &next)) != 0); ) {
                mprMark(item);
            }
            /* This is the lexical block scope */
            for (b = block->scope; b; b = b->scope) {
#if FUTURE
                if (b->pot.shortScope) {
                    break;
                }
#endif
                mprMark(b);
            }
            /* This is the activation (call) chain */
            for (b = block->prev; b; b = b->prev) {
                mprMark(b);
            }
            /*
                Don't mark stack as the VM will mark that
                Don't mark locations as they are always inherited from a frame which will mark them for us
             */
        }
    }
}


PUBLIC void ejsCreateBlockHelpers(Ejs *ejs)
{
    EjsHelpers      *helpers;

    ejs->service->blockHelpers = ejs->service->potHelpers;
    helpers = &ejs->service->blockHelpers;
    helpers->clone = (EjsCloneHelper) ejsCloneBlock;
}


PUBLIC void ejsInitBlockType(Ejs *ejs, EjsType *type)
{
    type->constructor.block.pot.shortScope = 1;
    ejsAddImmutable(ejs, S_commaProt, EN(",protected"), ejsCreateStringFromAsc(ejs, ",protected"));
}


PUBLIC void ejsConfigureBlockType(Ejs *ejs)
{
    EjsType     *type;
    
    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "Block"))) == 0) {
        return;
    }
    ejsAddImmutable(ejs, S_commaProt, EN(",protected"), ejsCreateStringFromAsc(ejs, ",protected"));
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.

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
