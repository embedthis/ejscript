/**
    ejsBlock.c - Lexical block

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

//  MOB -- rename to LexBlock
/*********************************** Helpers **********************************/

EjsBlock *ejsCreateBlock(Ejs *ejs, int size)
{
    EjsBlock        *block;

    block = (EjsBlock*) ejsCreateObject(ejs, ejs->blockType, size);
    if (block == 0) {
        return 0;
    }
    SHORT_SCOPE(block) = 1;
#if UNUSED
    ejsInitList(&block->namespaces);
#endif
    return block;
}


void ejsMarkBlock(Ejs *ejs, EjsBlock *block)
{
    EjsObj          *item;
    EjsBlock        *b;
    int             next;

    ejsMarkObject(ejs, (EjsObj*) block);
    if (block->prevException) {
        ejsMark(ejs, (EjsObj*) block->prevException);
    }
    if (block->namespaces.length > 0) {
        for (next = 0; ((item = (EjsObj*) ejsGetNextItem(ejs, &block->namespaces, &next)) != 0); ) {
            ejsMark(ejs, item);
        }
    }
    for (b = block->scope; b; b = b->scope) {
        ejsMark(ejs, (EjsObj*) b);
    }
    //  TODO MOB - this should not be required as GC in mark() follows the block caller/prev chain
    for (b = block->prev; b; b = b->prev) {
        ejsMark(ejs, (EjsObj*) b);
    }
}


EjsBlock *ejsCloneBlock(Ejs *ejs, EjsBlock *src, bool deep)
{
    EjsBlock    *dest;

    dest = (EjsBlock*) ejsCloneObject(ejs, (EjsObj*) src, deep);

    dest->nobind = src->nobind;
    dest->scope = src->scope;
#if MOB && OPT
    dest->namespaces = src->namespaces;
#else
#if UNUSED
    ejsInitList(&dest->namespaces);
    ejsCopyList(ejs, &dest->namespaces, &src->namespaces);
#else
    ejsAppendArray(ejs, &dest->namespaces, &src->namespaces);
#endif
#endif
    return dest;
}

/********************************* Namespaces *******************************/

void ejsResetBlockNamespaces(Ejs *ejs, EjsBlock *block)
{
    ejsClearArray(ejs, &block->namespaces);
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
#if UNUSED
    //  TODO OPT need a routine or option to only add unique namespaces.   
    EjsFunction     *fun;
    fun = (EjsFunction*) block;

    list = &block->namespaces;
    EjsNamespace    *namespace;
    int             next;
    if (ejsIsFunction(ejs, fun)) {
        if (fun->isInitializer) {
            block = block->scope;
            list = &block->namespaces;
            for (next = 0; (namespace = ejsGetNextItem(list, &next)) != 0; ) {
                if (strcmp(namespace->name, nsp->name) == 0) {
                    /* Already there */
                    return 0;
                }
            }
            if (block->obj.master && ejs->master) {
                nsp = ejsCreateNamespace(ejs->master, mprStrdup(ejs->master, nsp->name), mprStrdup(ejs->master, nsp->uri));
            }
        }
    }
#endif
#if MOB && OPT
    ejsAddItemToSharedList(ejs, list, nsp);
#else
    ejsAddItem(ejs, &block->namespaces, nsp);
#endif
    return 0;
}


/*
    Inherit namespaces from base types. Only inherit protected.
 */
void ejsInheritBaseClassNamespaces(Ejs *ejs, EjsType *type, EjsType *baseType)
{
    EjsNamespace    *nsp;
    EjsBlock        *block;
    EjsArray        *baseNamespaces, *oldNamespaces;
    int             next;

    block = &type->constructor.block;
    oldNamespaces = &block->namespaces;
#if UNUSED
    ejsInitList(&block->namespaces);
#endif
    baseNamespaces = &baseType->constructor.block.namespaces;

    if (baseNamespaces) {
        for (next = 0; ((nsp = (EjsNamespace*) ejsGetNextItem(ejs, baseNamespaces, &next)) != 0); ) {
            //  MOB -- must be a better way to do this?
            if (ejsContainsString(ejs, nsp->name, ejs->commaProtString)) {
                ejsAddItem(ejs, &block->namespaces, nsp);
            }
        }
    }
    if (oldNamespaces->length > 0) {
        for (next = 0; ((nsp = (EjsNamespace*) ejsGetNextItem(ejs, oldNamespaces, &next)) != 0); ) {
            ejsAddItem(ejs, &block->namespaces, nsp);
        }
    }
}


/*************************************** Factory ***********************************/

void ejsCreateBlockType(Ejs *ejs)
{
    EjsType     *type;

    type = ejs->blockType = ejsCreateNativeType(ejs, "ejs", "Block", ES_Block, sizeof(EjsBlock));
    SHORT_SCOPE(type) = 1;
    ejsCloneObjectHelpers(ejs, type);

    type->helpers.clone = (EjsCloneHelper) ejsCloneBlock;
    type->helpers.mark = (EjsMarkHelper) ejsMarkBlock;

    ejs->commaProtString = ejsCreateStringFromCS(ejs, ",protected");
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
