/**
    ejsScope.c - Lookup variables in the scope chain.
  
    This modules provides scope chain management including lookup, get and set services for variables. It will 
    lookup variables using the current execution variable scope and the set of open namespaces.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************* Code ***********************************/
/*
    Look for a variable by name in the scope chain and return the location in "lookup" and a positive slot number if found. 
    If the name.space is non-null/non-empty, then only the given namespace will be used. otherwise the set of open 
    namespaces will be used. The lookup structure will contain details about the location of the variable.
 */
int ejsLookupScope(Ejs *ejs, EjsName *name, EjsLookup *lookup)
{
    EjsFrame        *fp;
    EjsBlock        *block;
    EjsState        *state;
    int             slotNum, nth;

    mprAssert(ejs);
    mprAssert(name);
    mprAssert(lookup);

    slotNum = -1;
    state = ejs->state;
    fp = state->fp;

    if (fp->function.thisObj) {
        if ((slotNum = ejsLookupVar(ejs, fp->function.thisObj, name, lookup)) >= 0) {
            lookup->slotNum = slotNum;
            return slotNum;
        }
    }
    
    /*
        Look for the name in the scope chain considering each block scope. LookupVar will consider base classes and 
        namespaces. Don't search the last scope chain entry which will be global. For cloned interpreters, global 
        will belong to the master interpreter, so we must do that explicitly below to get the right global.
     */
    for (nth = 0, block = state->bp; block->scopeChain; block = block->scopeChain) {

        //  MOB -- rationalize with the code above
        if (fp->function.thisObj && block == (EjsBlock*) fp->function.thisObj->type) {
            /*
                This will lookup the instance and all base classes
             */
            if ((slotNum = ejsLookupVar(ejs, fp->function.thisObj, name, lookup)) >= 0) {
                lookup->nthBlock = nth;
                break;
            }
            
        } else {
            if ((slotNum = ejsLookupVar(ejs, (EjsObj*) block, name, lookup)) >= 0) {
                lookup->nthBlock = nth;
                break;
            }
        }
        nth++;
    }
    if (slotNum < 0 && ((slotNum = ejsLookupVar(ejs, ejs->global, name, lookup)) >= 0)) {
        lookup->nthBlock = nth;
    }
    lookup->slotNum = slotNum;
    return slotNum;
}


/*
    Find a property in an object or type and its base classes.
 */
int ejsLookupVar(Ejs *ejs, EjsObj *obj, EjsName *name, EjsLookup *lookup)
{
    EjsType     *type;
    EjsObj      *vp;
    int         slotNum;

    mprAssert(obj);
    mprAssert(obj->type);
    mprAssert(name);

    /*
        OPT - bit field initialization
     */
    lookup->nthBase = 0;
    lookup->nthBlock = 0;
    lookup->useThis = 0;
    lookup->instanceProperty = 0;
    lookup->ownerIsType = 0;
    lookup->trait = 0;
    lookup->slotNum = -1;
    vp = obj;

    /*
        Search through the inheritance chain of base classes. nthBase counts the subtypes that must be traversed. 
     */
    for (slotNum = -1, lookup->nthBase = 0; vp; lookup->nthBase++) {
        if ((slotNum = ejsLookupVarWithNamespaces(ejs, obj, vp, name, lookup)) >= 0) {
            break;
        }
        if (ejsIsFrame(vp)) break;
        vp = (vp->isType) ? (EjsObj*) ((EjsType*) vp)->baseType: (EjsObj*) vp->type;
        type = (EjsType*) vp;
        if (type == 0 || type->skipScope) {
            break;
        }
    }
    if (slotNum < 0 && obj->type->prototype) {
        vp = obj->type->prototype;
        for (lookup->nthBase = 0; vp; lookup->nthBase++) {
            if ((slotNum = ejsLookupVarWithNamespaces(ejs, obj, vp, name, lookup)) >= 0) {
                break;
            }
            //  MOB -- fix (Object prototype loops)
            if (vp == vp->type->prototype) {
                break;
            }
            vp = vp->type->prototype;
        }
    } 
    if (slotNum >= 0) {
        lookup->trait = ejsGetTrait(lookup->obj, lookup->slotNum);
    }
    return lookup->slotNum = slotNum;
}


/*
    Find a variable in a block. Scope blocks are provided by the global object, types, functions and statement blocks.
 */
int ejsLookupVarWithNamespaces(Ejs *ejs, EjsObj *originalObj, EjsObj *vp, EjsName *name, EjsLookup *lookup)
{
    EjsNamespace    *nsp;
    EjsName         qname;
    EjsBlock        *b;
    EjsObj          *owner;
    EjsFunction     *ref;
    int             slotNum, nextNsp;

    mprAssert(vp);
    mprAssert(name);
    mprAssert(name->name);
    mprAssert(name->space);
    mprAssert(lookup);

    if ((slotNum = ejsLookupProperty(ejs, vp, name)) < 0 && name->space[0] != EJS_EMPTY_NAMESPACE[0]) {
        return slotNum;
    }
    if (slotNum >= 0) {
        ref = (EjsFunction*) ejsGetProperty(ejs, vp, slotNum);
        
        if (ejsIsType(vp) && originalObj != vp && !ejsIsType(originalObj) && (!ejsIsFunction(ref) || ref->staticMethod)) {
             /* 
                Accessing a static var or static method from a sub-type of the original instance.
                i.e. Type properties should not be visible to instances.
              */
            ;
        } else {
        
            if (ejsIsFunction(ref) && !ref->staticMethod && ejsIsType(originalObj) && vp != (EjsObj*) ejs->objectType) {
                /* 
                    Accessing an instance method from a type and the method is not in Object
                    i.e. Instance methods should not be visible to Type objects.
                 */
                ;
                
            } else {
                lookup->name = *name;
                lookup->obj = vp;
                lookup->slotNum = slotNum;
                return slotNum;
            }
        }
    }
    qname = *name;
    for (b = ejs->state->bp; b; b = b->scopeChain) {
        for (nextNsp = -1; (nsp = (EjsNamespace*) ejsGetPrevItem(&b->namespaces, &nextNsp)) != 0; ) {

            if (nsp->flags & EJS_NSP_PROTECTED && vp->isType && ejs->state->fp) {
                /*
                    Protected access. See if the type containing the method we are executing is a sub class of the type 
                    containing the property ie. Can we see protected properties?
                 */
                owner = (EjsObj*) ejs->state->fp->function.owner;
                if (owner && !ejsIsA(ejs, owner, (EjsType*) vp)) {
                    continue;
                }
            }
            qname.space = nsp->uri;
            mprAssert(qname.space);
            if (qname.space) {
                slotNum = ejsLookupProperty(ejs, vp, &qname);
                if (slotNum >= 0) {
                    //  MOB -- since we need to get the trait anyway, better to determine this from the trait
                    ref = (EjsFunction*) ejsGetProperty(ejs, vp, slotNum);
                    if (ejsIsType(vp) && originalObj != vp && !ejsIsType(originalObj) && 
                            (!ejsIsFunction(ref) || ref->staticMethod)) {
                        /* Accessing static property or static method from an instance */
                        continue;
                    }
                    if (ejsIsFunction(ref) && !ref->staticMethod && ejsIsType(originalObj) && 
                            vp != (EjsObj*) ejs->objectType) {
                        mprNop();
                        continue;
                    }
                    lookup->name = qname;
                    lookup->obj = vp;
                    lookup->slotNum = slotNum;
                    return slotNum;
                }
            }
        }
    }
    return -1;
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
