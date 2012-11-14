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


PUBLIC EjsObj *ejsThrowStopIteration(Ejs *ejs)
{
#if FUTURE
    ejs->exception = ejs->iterator;
#else
    ejs->exception = ESV(StopIteration);
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
    assure(ip->nativeNext == 0);

    return ip;
}
#endif


/*********************************** Factory **********************************/
/*
    Create an iterator.
 */
PUBLIC EjsIterator *ejsCreateIterator(Ejs *ejs, EjsAny *obj, int length, void *nativeNext, bool deep, EjsArray *namespaces)
{
    EjsIterator     *ip;

    if ((ip = ejsCreateObj(ejs, ESV(Iterator), 0)) != 0) {
        ip->index = 0;
        ip->indexVar = 0;
        ip->nativeNext = nativeNext;
        ip->target = obj;
        ip->length = length;
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
PUBLIC void ejsCreateIteratorType(Ejs *ejs)
{
    ejsCreateCoreType(ejs, N(EJS_ITERATOR_NAMESPACE, "Iterator"), sizeof(EjsIterator), S_Iterator,  
        ES_iterator_Iterator_NUM_CLASS_PROP, manageIterator, EJS_TYPE_OBJ);
    ejsCreateCoreType(ejs, N(EJS_ITERATOR_NAMESPACE, "StopIteration"), sizeof(EjsError), S_StopIteration, 
        ES_iterator_StopIteration_NUM_CLASS_PROP, manageIterator, EJS_TYPE_OBJ);
}


PUBLIC void ejsConfigureIteratorType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeCoreType(ejs, N(EJS_ITERATOR_NAMESPACE, "Iterator"))) == 0) {
        return;
    }
    prototype = type->prototype;
    ejsBindMethod(ejs, prototype, ES_iterator_Iterator_next, nextIterator);

    if ((type = ejsFinalizeCoreType(ejs, N(EJS_ITERATOR_NAMESPACE, "StopIteration"))) == 0) {
        return;
    }
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
