/**
    ejsIterator.c - Iterator class

    This provides a high performance iterator construction for native classes.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Code ************************************/
/*
    Call the supplied next() function to return the next enumerable item
 */
static EjsObj *nextIterator(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    if (ip->nativeNext) {
        return (ip->nativeNext)(ejs, ip, argc, argv);
    } else {
        ejsThrowStopIteration(ejs);
        return 0;
    }
}


EjsObj *ejsThrowStopIteration(Ejs *ejs)
{
#if FUTURE
    ejs->exception = ejs->iterator;
#else
    ejs->exception = ST(StopIteration);
#endif
    ejsAttention(ejs);
    return ejs->exception;
}


/*********************************** Methods **********************************/
#if KEEP
/*
    Constructor to create an iterator using a scripted next().

    public function Iterator(obj, f, deep, ...namespaces)
 */
static EjsObj *iteratorConstructor(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    if (argc != 2 || !ejsIsFunction(ejs, argv[1])) {
        ejsThrowArgError(ejs, "usage: Iterator(obj, function)");
        return 0;
    }
    ip->target = argv[0];
    ip->next = (EjsFunction*) argv[1];
    mprAssert(ip->nativeNext == 0);

    return ip;
}
#endif


/*********************************** Factory **********************************/
/*
    Create an iterator.
 */
EjsIterator *ejsCreateIterator(Ejs *ejs, EjsAny *obj, void *nativeNext, bool deep, EjsArray *namespaces)
{
    EjsIterator     *ip;

    ip = ejsCreateObj(ejs, ST(Iterator), 0);
    if (ip) {
        ip->index = 0;
        ip->indexVar = 0;
        ip->nativeNext = nativeNext;
        ip->target = obj;
        ip->deep = deep;
        ip->namespaces = namespaces;
    }
    return ip;
}


static void manageIterator(EjsIterator *ip, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ip->target);
        mprMark(ip->namespaces);
        mprMark(ip->indexVar);
    }
}


/*
    Create the Iterator and StopIteration types
 */
void ejsCreateIteratorType(Ejs *ejs)
{
    EjsType     *type;

    if (ST(Iterator) == 0) {
        type = ejsCreateNativeType(ejs, N(EJS_ITERATOR_NAMESPACE, "Iterator"), sizeof(EjsIterator), S_Iterator,  
            ES_iterator_Iterator_NUM_CLASS_PROP, manageIterator, EJS_OBJ_HELPERS);
    }
    if (ST(StopIteration) == 0) {
        type = ejsCreateNativeType(ejs, N(EJS_ITERATOR_NAMESPACE, "StopIteration"), sizeof(EjsError), S_StopIteration, 
            ES_iterator_StopIteration_NUM_CLASS_PROP, manageIterator, EJS_OBJ_HELPERS);
    }
}


void ejsConfigureIteratorType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;
    static int once = 0;

    if (once++ == 0) {
        type = ST(Iterator);
        prototype = type->prototype;
        ejsBindMethod(ejs, prototype, ES_iterator_Iterator_next, nextIterator);
    }
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
