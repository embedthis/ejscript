/**
    ejsScope.c - Lookup variables in the scope chain.
  
    This modules provides variable lookup and scope chain management.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************* Code ***********************************/
/*
    Look for a variable by name in the scope chain and return the location in "lookup" and a positive slot number if found. 
    If the name.space is non-empty, then only the given namespace will be used. Otherwise the set of open namespaces will 
    be used. The lookup structure will contain details about the location of the variable.
 */
int ejsLookupScope(Ejs *ejs, EjsName *name, EjsLookup *lookup)
{
    EjsBlock        *block, *thisType;
    EjsState        *state;
    EjsType         *type;
    EjsObj          *prototype, *obj, *thisObj;
    int             slotNum, nthBlock, nthBase;

    mprAssert(ejs);
    mprAssert(name);
    mprAssert(name->name);
    mprAssert(name->space);
    mprAssert(lookup);

    state = ejs->state;
    thisObj = state->fp->function.thisObj;
    thisType = (EjsBlock*) thisObj->type;

    for (nthBlock = 0, block = state->bp; block; block = block->scopeChain, nthBlock++) {
        if (thisType == block) {
            obj = thisObj;
        } else {
            obj = (EjsObj*) block;
        }
        if ((slotNum = ejsLookupVarWithNamespaces(ejs, obj, name, lookup)) >= 0) {
            lookup->nthBlock = nthBlock;
            return slotNum;
        }
        type = block->obj.type;
        for (nthBase = 1; type; type = type->baseType, nthBase++) {
            if ((prototype = type->prototype) == 0 || prototype->skipScope) {
                break;
            }
            if ((slotNum = ejsLookupVarWithNamespaces(ejs, prototype, name, lookup)) >= 0) {
                lookup->nthBlock = nthBlock;
                lookup->nthBase = nthBase;
                return slotNum;
            }
        }
    }
    for (nthBlock = 0, block = state->bp; block; block = block->scopeChain, nthBlock++) {
        if (thisType == block) {
            type = (EjsType*) block;
        } else {
            type = block->obj.type;
        }
        for (nthBase = 1; type; type = type->baseType, nthBase++) {
            if (type->block.obj.skipScope) {
                //  MOB -- continue or break?
                continue;
            }
            if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) type, name, lookup)) >= 0) {
                lookup->nthBase = nthBase;
                lookup->nthBlock = nthBlock;
                return slotNum;
            }
        }
    }
    return -1;
}


/*
    Find a property in an object or its prototype and base classes.
 */
int ejsLookupVar(Ejs *ejs, EjsObj *obj, EjsName *name, EjsLookup *lookup)
{
    EjsType     *type;
    EjsObj      *prototype;
    int         slotNum, nthBase;

    mprAssert(obj);
    mprAssert(obj->type);
    mprAssert(name);

    /*
        OPT - bit field initialization
     */
    lookup->nthBase = 0;
    lookup->nthBlock = 0;
    lookup->trait = 0;

    //  MOB - what about this
    lookup->useThis = 0;
    //  MOB -- not used here
    lookup->instanceProperty = 0;
    //  MOB - what about this
    lookup->ownerIsType = 0;

    if ((slotNum = ejsLookupVarWithNamespaces(ejs, obj, name, lookup)) >= 0) {
        return slotNum;
    }
    type = obj->type;
    for (nthBase = 1; type; type = type->baseType, nthBase++) {
        if ((prototype = type->prototype) == 0 || prototype->skipScope) {
            break;
        }
        if ((slotNum = ejsLookupVarWithNamespaces(ejs, prototype, name, lookup)) >= 0) {
            lookup->nthBase = nthBase;
            return slotNum;
        }
    }
    type = obj->type;
    for (nthBase = 1; type; type = type->baseType, nthBase++) {
        if (type->block.obj.skipScope) {
            //  MOB -- continue or break?
            continue;
        }
        if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) type, name, lookup)) >= 0) {
            lookup->nthBase = nthBase;
            return slotNum;
        }
    }
    return -1;
}


/*
    Find a variable in an object considering namespaces. If the space is "", then search for the property name using
    the set of open namespaces.
 */
//  MOB -- rename 
int ejsLookupVarWithNamespaces(Ejs *ejs, EjsObj *obj, EjsName *name, EjsLookup *lookup)
{
    EjsNamespace    *nsp;
    EjsName         qname, qn;
    EjsBlock        *b;
    int             slotNum, next;

    mprAssert(obj);
    mprAssert(name);
    mprAssert(name->name);
    mprAssert(name->space);
    mprAssert(lookup);

    if (name->space[0]) {
        /* Lookup with an explicit namespace */
        slotNum = ejsLookupProperty(ejs, obj, name);

    } else {
        /* Lookup with the set of open namespaces in the current scope */
        qname.name = name->name;
        qname.space = 0;
        /* Special lookup with space == NULL. Means lookup only match if there is only one property of this name */
        if ((slotNum = ejsLookupProperty(ejs, obj, ejsName(&qname, NULL, name->name))) >= 0) {
            qn = ejsGetPropertyName(ejs, obj, slotNum);
            if (name->space[0] && (name->space[0] != qn.space[0] || strcmp(name->space, qn.space) != 0)) {
                /* Unique name match. Name matches, but namespace does not */
                return -1;
            }
            if (qn.space[0]) {
                for (next = -1; (nsp = (EjsNamespace*) ejsGetPrevItem(&ejs->globalBlock->namespaces, &next)) != 0; ) {
                    if (strcmp(nsp->uri, qn.space) == 0) {
                        goto done;
                    }
                }
                //  MOB -- need a fast way to know if the space is a standard reserved namespace or not */
                /* Verify namespace is open */
                for (b = ejs->state->bp; b->scopeChain; b = b->scopeChain) {
                    for (next = -1; (nsp = (EjsNamespace*) ejsGetPrevItem(&b->namespaces, &next)) != 0; ) {
                        if (strcmp(nsp->uri, qn.space) == 0) {
                            goto done;
                        }
                    }
                }
            }

        } else {
            qname = *name;
            for (b = ejs->state->bp; b; b = b->scopeChain) {
                for (next = -1; (nsp = (EjsNamespace*) ejsGetPrevItem(&b->namespaces, &next)) != 0; ) {
                    qname.space = nsp->uri;
                    if ((slotNum = ejsLookupProperty(ejs, obj, &qname)) >= 0) {
                        mprLog(ejs, 0, "Object has multiple properties of the same name \"%s\"", name->name); 
                        goto done;
                    }
                }
            }
        }
    }
done:
    if (slotNum >= 0) {
        lookup->ref = ejsGetProperty(ejs, obj, slotNum);
        lookup->name = *name;
        lookup->obj = obj;
        lookup->slotNum = slotNum;
        lookup->trait = ejsGetTrait(lookup->obj, lookup->slotNum);
    }
    return slotNum;
}


/*
    Get a variable by name. If vp is specified, it contains an explicit object in which to search for the variable name. 
    Otherwise, the full execution scope is consulted. The lookup fields will be set as residuals.
 */
EjsObj *ejsGetVarByName(Ejs *ejs, EjsObj *vp, EjsName *name, EjsLookup *lookup)
{
    EjsObj  *result;
    int     slotNum;

    mprAssert(ejs);
    mprAssert(name);

    //  OPT - really nice to remove this
    if (vp && vp->type->helpers->getPropertyByName) {
        result = (*vp->type->helpers->getPropertyByName)(ejs, vp, name);
        if (result) {
            return result;
        }
    }
    if (vp) {
        slotNum = ejsLookupVar(ejs, vp, name, lookup);
    } else {
        slotNum = ejsLookupScope(ejs, name, lookup);
    }
    if (slotNum < 0) {
        return 0;
    }
    return ejsGetProperty(ejs, lookup->obj, slotNum);
}


void ejsShowBlockScope(Ejs *ejs, EjsBlock *block)
{
#if BLD_DEBUG
    EjsNamespace    *nsp;
    EjsList         *namespaces;
    int             nextNsp;

    mprLog(ejs, 6, "\n  Block scope");
    for (; block; block = block->scopeChain) {
        mprLog(ejs, 6, "    Block \"%s\" 0x%08x", mprGetName(block), block);
        namespaces = &block->namespaces;
        if (namespaces) {
            for (nextNsp = 0; (nsp = (EjsNamespace*) ejsGetNextItem(namespaces, &nextNsp)) != 0; ) {
                mprLog(ejs, 6, "        \"%s\"", nsp->uri);
            }
        }
    }
#endif
}


void ejsShowCurrentScope(Ejs *ejs)
{
#if BLD_DEBUG
    EjsNamespace    *nsp;
    EjsList         *namespaces;
    EjsBlock        *block;
    int             nextNsp;

    mprLog(ejs, 6, "\n  Current scope");
    for (block = ejs->state->bp; block; block = block->scopeChain) {
        mprLog(ejs, 6, "    Block \"%s\" 0x%08x", mprGetName(block), block);
        namespaces = &block->namespaces;
        if (namespaces) {
            for (nextNsp = 0; (nsp = (EjsNamespace*) ejsGetNextItem(namespaces, &nextNsp)) != 0; ) {
                mprLog(ejs, 6, "        \"%s\"", nsp->uri);
            }
        }
    }
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
