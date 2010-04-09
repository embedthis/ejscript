/**
    ejsBlock.c - Lexical block

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Helpers **********************************/

EjsBlock *ejsCreateBlock(Ejs *ejs, int size)
{
    EjsBlock        *block;

    block = (EjsBlock*) ejsCreateObject(ejs, ejs->blockType, size);
    if (block == 0) {
        return 0;
    }
    ejsInitList(&block->namespaces);
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
        for (next = 0; ((item = (EjsObj*) ejsGetNextItem(&block->namespaces, &next)) != 0); ) {
            ejsMark(ejs, item);
        }
    }
    for (b = block->scopeChain; b; b = b->scopeChain) {
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
    dest->scopeChain = src->scopeChain;
    dest->namespaces = src->namespaces;
    return dest;
}

/********************************* Namespaces *******************************/

void ejsResetBlockNamespaces(Ejs *ejs, EjsBlock *block)
{
    ejsClearList(&block->namespaces);
}


int ejsGetNamespaceCount(EjsBlock *block)
{
    mprAssert(block);

    return ejsGetListCount(&block->namespaces);
}


void ejsPopBlockNamespaces(EjsBlock *block, int count)
{
    mprAssert(block);
    mprAssert(block->namespaces.length >= count);

    block->namespaces.length = count;
}


int ejsAddNamespaceToBlock(Ejs *ejs, EjsBlock *block, EjsNamespace *nsp)
{
    EjsFunction     *fun;
    EjsNamespace    *namespace;
    EjsList         *list;
    int             next;

    mprAssert(block);

    if (nsp == 0) {
        ejsThrowTypeError(ejs, "Not a namespace");
        return EJS_ERR;
    }
    fun = (EjsFunction*) block;
    list = &block->namespaces;

    if (ejsIsFunction(fun)) {
        if (fun->isInitializer && fun->owner) {
            block = block->scopeChain;
            list = &block->namespaces;
            /*
                If defining a namespace at the class level (outside functions) use the class itself.
                Initializers only run once so this should only happen once.
             */
            /* TODO OPT - this is only needed when classes implement other classes (Model <= Record) */
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
    ejsAddItemToSharedList(block, list, nsp);
    return 0;
}


/*
    Inherit namespaces from base types. Only inherit protected.
 */
void ejsInheritBaseClassNamespaces(Ejs *ejs, EjsType *type, EjsType *baseType)
{
    EjsNamespace    *nsp;
    EjsBlock        *block;
    EjsList         *baseNamespaces, oldNamespaces;
    int             next;

    block = &type->block;
    oldNamespaces = block->namespaces;
    ejsInitList(&block->namespaces);
    baseNamespaces = &baseType->block.namespaces;

    if (baseNamespaces) {
        for (next = 0; ((nsp = (EjsNamespace*) ejsGetNextItem(baseNamespaces, &next)) != 0); ) {
            if (strstr(nsp->name, ",protected")) {
                ejsAddItem(block, &block->namespaces, nsp);
            }
        }
    }

    if (oldNamespaces.length > 0) {
        for (next = 0; ((nsp = (EjsNamespace*) ejsGetNextItem(&oldNamespaces, &next)) != 0); ) {
            ejsAddItem(block, &block->namespaces, nsp);
        }
    }
}


/*************************************** Factory ***********************************/

void ejsCreateBlockType(Ejs *ejs)
{
    EjsType     *type;

    type = ejs->blockType = ejsCreateNativeType(ejs, "ejs", "Block", ES_Block, sizeof(EjsBlock));

    type->skipScope = 1;
    type->helpers->clone = (EjsCloneHelper) ejsCloneBlock;
    type->helpers->mark = (EjsMarkHelper) ejsMarkBlock;
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
