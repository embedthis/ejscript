
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
int ejsLookupScope(Ejs *ejs, EjsName name, EjsLookup *lookup)
{
    EjsFrame        *frame;
    EjsBlock        *bp;
    EjsState        *state;
    EjsType         *type;
    EjsObj          *thisObj;
    EjsPot          *prototype;
    int             slotNum, nthBase;

    mprAssert(ejs);
    mprAssert(name.name);
    mprAssert(name.space);
    mprAssert(lookup);

    state = ejs->state;
    slotNum = -1;
    
    memset(lookup, 0, sizeof(*lookup));
    thisObj = state->fp->function.boundThis;

    //  OPT -- remove nthBlock. Not needed if not binding
    for (lookup->nthBlock = 0, bp = state->bp; bp; bp = bp->scope, lookup->nthBlock++) {
        /* Seach simple object */
        lookup->originalObj = bp;
        if ((slotNum = ejsLookupVarWithNamespaces(ejs, bp, name, lookup)) >= 0) {
            return slotNum;
        }
        if (ejsIsFrame(ejs, bp)) {
            frame = (EjsFrame*) bp;
            if (thisObj && frame->function.boundThis == thisObj && 
                    thisObj != ejs->global && !frame->function.staticMethod && 
                    !frame->function.isInitializer) {
                lookup->originalObj = thisObj;
                /* Instance method only */
                if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) thisObj, name, lookup)) >= 0) {
                    return slotNum;
                }
                /* Search prototype chain */
                for (nthBase = 1, type = TYPE(thisObj); type; type = type->baseType, nthBase++) {
                    if ((prototype = type->prototype) == 0 || prototype->shortScope) {
                        break;
                    }
                    if ((slotNum = ejsLookupVarWithNamespaces(ejs, prototype, name, lookup)) >= 0) {
                        lookup->nthBase = nthBase;
                        lookup->type = type;
                        return slotNum;
                    }
                }
                if (frame->function.isConstructor) {
                    for (nthBase = 1, type = (EjsType*) TYPE(thisObj); type; type = type->baseType, nthBase++) {
                        if (type->constructor.block.pot.shortScope) {
                            break;
                        }
                        if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) type, name, lookup)) >= 0) {
                            lookup->nthBase = nthBase;
                            return slotNum;
                        }
                    }
                }
                thisObj = 0;
            }
        } else if (ejsIsType(ejs, bp)) {
            //  OPT -- remove nthBase. Not needed if not binding.
            /* Search base class chain */
            for (nthBase = 1, type = (EjsType*) bp; type; type = type->baseType, nthBase++) {
                if (type->constructor.block.pot.shortScope) {
                    break;
                }
                if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) type, name, lookup)) >= 0) {
                    lookup->nthBase = nthBase;
                    return slotNum;
                }
            }
        }
    }
    return -1;
}


/*
    Find a property in an object or its prototype and base classes.
 */
int ejsLookupVar(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup)
{
    EjsType     *type;
    EjsPot      *prototype;
    int         slotNum, nthBase;

    mprAssert(obj);
    mprAssert(lookup);

    memset(lookup, 0, sizeof(*lookup));

    /* Lookup simple object */
    if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) obj, name, lookup)) >= 0) {
        return slotNum;
    }
    /* Lookup prototype chain */
    for (nthBase = 1, type = TYPE(obj); type; type = type->baseType, nthBase++) {
        if ((prototype = type->prototype) == 0 || prototype->shortScope) {
            break;
        }
        if ((slotNum = ejsLookupVarWithNamespaces(ejs, (EjsObj*) prototype, name, lookup)) >= 0) {
            lookup->nthBase = nthBase;
            return slotNum;
        }
    }
    /* Lookup base-class chain */
    type = ejsIsType(ejs, obj) ? ((EjsType*) obj)->baseType : TYPE(obj);
    for (nthBase = 1; type; type = type->baseType, nthBase++) {
        if (type->constructor.block.pot.shortScope) {
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
int ejsLookupVarWithNamespaces(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup)
{
    EjsNamespace    *nsp;
    EjsName         qname, target;
    EjsString       *space;
    EjsBlock        *b;
    MprList         *globalSpaces;
    int             next, slotNum;

    mprAssert(obj);
    mprAssert(name.name);
    mprAssert(name.space);
    mprAssert(lookup);

    b = (EjsBlock*) ejs->global;
    globalSpaces = &b->namespaces;

    if (name.space->value[0]) {
        /* Lookup with an explicit namespace */
        slotNum = ejsLookupProperty(ejs, obj, name);
        lookup->name = name;

    } else {
        /* 
            Lookup with the set of open namespaces in the current scope 
            Special lookup with space == NULL. Means lookup only match if there is only one property of this name 
         */
        qname.space = NULL;
        qname.name = name.name;
        if ((slotNum = ejsLookupProperty(ejs, obj, qname)) >= 0) {
            if (TYPE(obj)->virtualSlots) {
                lookup->name = name;
            } else {
                target = ejsGetPropertyName(ejs, obj, slotNum);
                lookup->name = target;
                space = name.space;
                if (space->value[0] && space != target.space) {
                    /* Unique name match. Name matches, but namespace does not */
                    slotNum = -1;
                } else if (target.space && target.space->value[0]) {
                    for (next = -1; (nsp = mprGetPrevItem(globalSpaces, &next)) != 0; ) {
                        if (nsp->value == target.space) {
                            goto done;
                        }
                    }
                    //  OPT -- need a fast way to know if the space is a standard reserved namespace or not */
                    /* Verify namespace is open */
                    for (b = ejs->state->bp; b->scope; b = b->scope) {
                        //  OPT. Doing some namespaces multiple times. Fix in compiler.
                        for (next = -1; (nsp = mprGetPrevItem(&b->namespaces, &next)) != 0; ) {
                            if (nsp->value == target.space) {
                                goto done;
                            }
                        }
                    }
                    slotNum = -1;
                }
            }

        } else {
            qname = name;
            for (b = ejs->state->bp; b; b = b->scope) {
                for (next = -1; (nsp = (EjsNamespace*) mprGetPrevItem(&b->namespaces, &next)) != 0; ) {
                    qname.space = nsp->value;
                    if ((slotNum = ejsLookupProperty(ejs, obj, qname)) >= 0) {
                        // mprLog(5, "WARNING: Object has multiple properties of the same name \"%@\"", name.name); 
                        goto done;
                    }
                }
            }
        }
    }
done:
    if (slotNum >= 0) {
        //  OPT MUST GET RID OF THIS. Means that every store does a get
        lookup->ref = ejsGetProperty(ejs, obj, slotNum);
        if (ejs->exception) {
            slotNum = -1;
        } else {
            lookup->obj = obj;
            lookup->slotNum = slotNum;
            lookup->trait = ejsGetPropertyTraits(ejs, lookup->obj, lookup->slotNum);
        }
    }
    return slotNum;
}


/*
    Get a variable by name. If obj is specified, it contains an explicit object in which to search for the variable name. 
    Otherwise, the full execution scope is consulted. The lookup fields will be set as residuals.
 */
EjsAny *ejsGetVarByName(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup)
{
    EjsObj  *result;
    int     slotNum;

    mprAssert(ejs);

    //  OPT - really nice to remove this
    //  OPT -- perhaps delegate the logic below down into a getPropertyByName?
    if (obj && TYPE(obj)->helpers.getPropertyByName) {
        if ((result = (TYPE(obj)->helpers.getPropertyByName)(ejs, obj, name)) != 0) {
            return result;
        }
    }
    if (obj) {
        slotNum = ejsLookupVar(ejs, obj, name, lookup);
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
    MprList         *namespaces;
    int             nextNsp;

    mprLog(6, "\n  Block scope");
    for (; block; block = block->scope) {
        mprLog(6, "    Block \"%s\" 0x%08x", mprGetName(block), block);
        namespaces = &block->namespaces;
        if (namespaces) {
            for (nextNsp = 0; (nsp = (EjsNamespace*) mprGetNextItem(namespaces, &nextNsp)) != 0; ) {
                mprLog(6, "        \"%@\"", nsp->value);
            }
        }
    }
#endif
}


void ejsShowCurrentScope(Ejs *ejs)
{
#if BLD_DEBUG
    EjsNamespace    *nsp;
    MprList         *namespaces;
    EjsBlock        *block;
    int             nextNsp;

    mprLog(6, "\n  Current scope");
    for (block = ejs->state->bp; block; block = block->scope) {
        mprLog(6, "    Block \"%s\" 0x%08x", mprGetName(block), block);
        namespaces = &block->namespaces;
        if (namespaces) {
            for (nextNsp = 0; (nsp = (EjsNamespace*) mprGetNextItem(namespaces, &nextNsp)) != 0; ) {
                mprLog(6, "        \"%@\"", nsp->value);
            }
        }
    }
#endif
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
