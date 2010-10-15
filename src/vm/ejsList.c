/**
    ejsList.c - Simple static list type.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

#if UNUSED
/****************************** Forward Declarations **************************/

static int growList(Ejs *ejs, EjsList *lp, int incr);

#define CAPACITY(lp) (ejsGetBlockSize(lp) / sizeof(void*))

//  OPT - inline some of these functions as macros

/************************************ Code ************************************/

void ejsInitList(EjsList *lp)
{
    lp->length = 0;
    lp->maxSize = MAXINT;
    lp->items = 0;
}


/*
    Define the list maximum size. If the list has not yet been written to, the initialSize will be observed.
 */
int ejsSetListLimits(Ejs *ejs, EjsList *lp, int initialSize, int maxSize)
{
    int         size;

    if (initialSize <= 0) {
        initialSize = MPR_LIST_INCR;
    }
    if (maxSize <= 0) {
        maxSize = MAXINT;
    }
    size = initialSize * sizeof(void*);

    if (lp->items == 0) {
        if ((lp->items = (void**) ejsAlloc(ejs, size)) == NULL) {
            return MPR_ERR_NO_MEMORY;
        }
    }
    lp->maxSize = maxSize;
    return 0;
}


/*
    Add an item to the list and return the item index.
 */
int ejsAddItem(Ejs *ejs, EjsList *lp, cvoid *item)
{
    int     index, capacity;

    mprAssert(lp);
    mprAssert(lp->length >= 0);

    capacity = CAPACITY(lp->items);
    mprAssert(capacity >= 0);

    if (lp->items == 0 || lp->length >= capacity) {
        if (growList(ejs, lp, 1) < 0) {
            return MPR_ERR_TOO_MANY;
        }
    }
    index = lp->length++;
    lp->items[index] = (void*) item;
    return index;
}


#if UNUSED
int ejsAddItemToSharedList(Ejs *ejs, EjsList *lp, cvoid *item)
{
    EjsList     tmp;

    if (lp->items == NULL || !mprIsParent(ejs, lp->items)) {
        tmp = *lp;
        lp->items = 0;
        lp->length = 0;
        if (ejsCopyList(ejs, lp, &tmp) < 0) {
            return MPR_ERR_NO_MEMORY;
        }
    }
    return ejsAddItem(ejs, lp, item);
}
#endif


EjsList *ejsAppendList(Ejs *ejs, EjsList *list, EjsList *add)
{
    void        *item;
    int         next;

    mprAssert(list);
    mprAssert(list != add);

    for (next = 0; ((item = ejsGetNextItem(add, &next)) != 0); ) {
        if (ejsAddItem(ejs, list, item) < 0) {
            mprFree(list);
            return 0;
        }
    }
    return list;
}


void ejsClearList(EjsList *lp)
{
    int     i;

    mprAssert(lp);

    //  MOB OPT - memset
    for (i = 0; i < lp->length; i++) {
        lp->items[i] = 0;
    }
    lp->length = 0;
}


int ejsCopyList(Ejs *ejs, EjsList *dest, EjsList *src)
{
    void        *item;
    int         next, capacity;

    ejsClearList(dest);

    capacity = CAPACITY(src->items);
    if (ejsSetListLimits(ejs, dest, capacity, src->maxSize) < 0) {
        return MPR_ERR_NO_MEMORY;
    }
    for (next = 0; (item = ejsGetNextItem(src, &next)) != 0; ) {
        if (ejsAddItem(ejs, dest, item) < 0) {
            return MPR_ERR_NO_MEMORY;
        }
    }
    return 0;
}


void *ejsGetItem(EjsList *lp, int index)
{
    mprAssert(lp);

    if (index < 0 || index >= lp->length) {
        return 0;
    }
    return lp->items[index];
}


void *ejsGetLastItem(EjsList *lp)
{
    mprAssert(lp);

    if (lp == 0) {
        return 0;
    }

    if (lp->length == 0) {
        return 0;
    }
    return lp->items[lp->length - 1];
}


void *ejsGetNextItem(EjsList *lp, int *next)
{
    void    *item;
    int     index;

    mprAssert(next);
    mprAssert(*next >= 0);

    if (lp == 0) {
        return 0;
    }
    index = *next;
    if (index < lp->length) {
        item = lp->items[index];
        *next = ++index;
        return item;
    }
    return 0;
}


int ejsGetListCount(EjsList *lp)
{
    if (lp == 0) {
        return 0;
    }

    return lp->length;
}


void *ejsGetPrevItem(EjsList *lp, int *next)
{
    int     index;

    mprAssert(next);

    if (lp == 0) {
        return 0;
    }
    if (*next < 0) {
        *next = lp->length;
    }
    index = *next;
    if (--index < lp->length && index >= 0) {
        *next = index;
        return lp->items[index];
    }
    return 0;
}


int ejsRemoveLastItem(EjsList *lp)
{
    mprAssert(lp);
    mprAssert(lp->length > 0);

    if (lp->length <= 0) {
        return MPR_ERR_NOT_FOUND;
    }
    return ejsRemoveItemAtPos(lp, lp->length - 1);
}


/*
    Remove an index from the list. Return the index where the item resided.
 */
int ejsRemoveItemAtPos(EjsList *lp, int index)
{
    void    **items;
    int     i;

    mprAssert(lp);
    mprAssert(index >= 0);
    mprAssert(lp->length > 0);

    if (index < 0 || index >= lp->length) {
        return MPR_ERR_NOT_FOUND;
    }
    items = lp->items;
    for (i = index; i < (lp->length - 1); i++) {
        items[i] = items[i + 1];
    }
    lp->length--;
    lp->items[lp->length] = 0;
    return index;
}


/*
    Grow the list by the requried increment
 */
static int growList(Ejs *ejs, EjsList *lp, int incr)
{
    int     len, memsize, capacity;

    /*
        Need to grow the list
     */
    capacity = CAPACITY(lp->items);
    mprAssert(capacity >= 0);
    
    if (capacity >= lp->maxSize) {
        if (lp->maxSize == 0) {
            lp->maxSize = INT_MAX;
        } else {
            mprAssert(capacity < lp->maxSize);
            return MPR_ERR_TOO_MANY;
        }
    }

    /*
        If growing by 1, then use the default increment which exponentially grows. Otherwise, assume the caller 
        knows exactly how the list needs to grow.
     */
    len = (incr <= 1) ? (MPR_LIST_INCR + capacity + capacity) : (capacity + incr);
    memsize = len * sizeof(void*);

    /*
        Grow the list of items. Use the existing context for lp->items if it already exists. Otherwise use the list as the
        memory context.
     */
    lp->items = (void**) ejsRealloc(ejs, lp->items, memsize);

    /*
        Zero the new portion (required for no-compact lists)
     */
    memset(&lp->items[capacity], 0, sizeof(void*) * (len - capacity));
    return 0;
}


int ejsLookupItem(EjsList *lp, cvoid *item)
{
    int     i;

    mprAssert(lp);
    
    for (i = 0; i < lp->length; i++) {
        if (lp->items[i] == item) {
            return i;
        }
    }
    return MPR_ERR_NOT_FOUND;
}
#endif


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
