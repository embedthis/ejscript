/**
    ejsScope.c - Lookup variables in the scope chain.
  
    This modules provides variable lookup and scope chain management.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Forwards *********************************/

static int lookupVarInBlock(Ejs *ejs, EjsBlock *bp, EjsName name, EjsLookup *lookup);

/************************************* Code ***********************************/
/*
    Look for a variable by name in the scope chain and return the location in "lookup" and a positive slot number if found. 
    If the name.space is non-empty, then only the given namespace will be used. Otherwise the set of open namespaces will 
    be used. The lookup structure will contain details about the location of the variable.
 */
int ejsLookupScope(Ejs *ejs, EjsName name, EjsLookup *lookup)
{
    EjsBlock        *bp;
    int             slotNum;

    assert(ejs);
    assert(name.name);
    assert(name.space);
    assert(lookup);

    memset(lookup, 0, sizeof(*lookup));

    //  OPT -- remove nthBlock. Not needed if not binding
    if (ejs->state) {
        for (lookup->nthBlock = 0, bp = ejs->state->bp; bp; bp = bp->scope, lookup->nthBlock++) {
            if ((slotNum = lookupVarInBlock(ejs, bp, name, lookup)) >= 0) {
                return slotNum;
            }
        }
    }
    return lookupVarInBlock(ejs, ejs->global, name, lookup);
}


/*
    Look for a variable by name in the scope chain and return the location in "lookup" and a positive slot number if found. 
    If the name.space is non-empty, then only the given namespace will be used. Otherwise the set of open namespaces will 
    be used. The lookup structure will contain details about the location of the variable.
 */
static int lookupVarInBlock(Ejs *ejs, EjsBlock *bp, EjsName name, EjsLookup *lookup)
{
    EjsFrame        *frame;
    EjsState        *state;
    EjsType         *type;
    EjsObj          *thisObj;
    EjsPot          *prototype;
    int             slotNum, nthBase;

    assert(ejs);
    assert(name.name);
    assert(name.space);
    assert(lookup);

    state = ejs->state;
    slotNum = -1;
    thisObj = state->fp->function.boundThis;

    /* Search simple block */
    lookup->originalObj = bp;
    if ((slotNum = ejsLookupVarWithNamespaces(ejs, bp, name, lookup)) >= 0) {
        return slotNum;
    }
    /* Optimized lookup for frames and types. Simple blocks don't need prototype lookup - so examine the type of block */
    if (ejsIsFrame(ejs, bp)) {
        frame = (EjsFrame*) bp;
        if (thisObj && frame->function.boundThis == thisObj && 
                thisObj != ejs->global && !frame->function.staticMethod && 
                !frame->function.isInitializer) {
            lookup->originalObj = thisObj;
            /* Instance method only */
            if ((slotNum = ejsLookupVarWithNamespaces(ejs, thisObj, name, lookup)) >= 0) {
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
                    if ((slotNum = ejsLookupVarWithNamespaces(ejs, type, name, lookup)) >= 0) {
                        lookup->nthBase = nthBase;
                        return slotNum;
                    }
                }
            }
            thisObj = 0;
        }
    } else if (ejsIsType(ejs, bp)) {
        /* Search base class chain */
        //  OPT -- remove nthBase. Not needed if not binding.
        for (nthBase = 1, type = (EjsType*) bp; type; type = type->baseType, nthBase++) {
            if (type->constructor.block.pot.shortScope) {
                break;
            }
            if ((slotNum = ejsLookupVarWithNamespaces(ejs, type, name, lookup)) >= 0) {
                lookup->nthBase = nthBase;
                return slotNum;
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

    assert(obj);
    assert(lookup);

    memset(lookup, 0, sizeof(*lookup));

    /* Lookup simple object */
    if ((slotNum = ejsLookupVarWithNamespaces(ejs, obj, name, lookup)) >= 0) {
        return slotNum;
    }
    /* Lookup prototype chain */
    for (nthBase = 1, type = TYPE(obj); type; type = type->baseType, nthBase++) {
        if ((prototype = type->prototype) == 0 || prototype->shortScope) {
            break;
        }
        if ((slotNum = ejsLookupVarWithNamespaces(ejs, prototype, name, lookup)) >= 0) {
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
        if ((slotNum = ejsLookupVarWithNamespaces(ejs, type, name, lookup)) >= 0) {
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

    assert(obj);
    assert(name.name);
    assert(name.space);
    assert(lookup);

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
                    /* OPT - Look at global spaces first */
                    for (next = -1; (nsp = mprGetPrevItem(globalSpaces, &next)) != 0; ) {
                        if (nsp->value == target.space) {
                            goto done;
                        }
                    }
                    //  OPT -- need a fast way to know if the space is a standard reserved namespace or not */
                    /* Verify namespace is open */
                    for (b = ejs->state->bp; b; b = b->scope) {
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
                        goto done;
                    }
                }
            }
            for (next = -1; (nsp = mprGetPrevItem(globalSpaces, &next)) != 0; ) {
                qname.space = nsp->value;
                if ((slotNum = ejsLookupProperty(ejs, obj, qname)) >= 0) {
                    goto done;
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

    assert(ejs);

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
#if ME_DEBUG
    EjsNamespace    *nsp;
    MprList         *namespaces;
    int             nextNsp;

    mprDebug("ejs vm", 6, "\n  Block scope");
    for (; block; block = block->scope) {
        mprDebug("ejs vm", 6, "    Block \"%s\" 0x%08x", mprGetName(block), block);
        namespaces = &block->namespaces;
        if (namespaces) {
            for (nextNsp = 0; (nsp = (EjsNamespace*) mprGetNextItem(namespaces, &nextNsp)) != 0; ) {
                mprDebug("ejs vm", 6, "        \"%@\"", nsp->value);
            }
        }
    }
#endif
}


void ejsShowCurrentScope(Ejs *ejs)
{
#if ME_DEBUG
    EjsNamespace    *nsp;
    MprList         *namespaces;
    EjsBlock        *block;
    int             nextNsp;

    mprDebug("ejs vm", 6, "\n  Current scope");
    for (block = ejs->state->bp; block; block = block->scope) {
        mprDebug("ejs vm", 6, "    Block \"%s\" 0x%08x", mprGetName(block), block);
        namespaces = &block->namespaces;
        if (namespaces) {
            for (nextNsp = 0; (nsp = (EjsNamespace*) mprGetNextItem(namespaces, &nextNsp)) != 0; ) {
                mprDebug("ejs vm", 6, "        \"%@\"", nsp->value);
            }
        }
    }
#endif
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
