/**
    ejsXMLList.c - XMLList type.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/****************************** Forward Declarations **************************/
/*
    XMLList methods
 */

#if KEEP
static EjsObj   *valueOf(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *xlLength(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *toXmlString(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *appendChild(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *attributes(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *child(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *elements(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *comments(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *decendants(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *elements(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *insertChildAfter(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *insertChildBefore(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *replace(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *setName(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
static EjsObj   *text(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv);
#endif

static bool allDigitsForXmlList(EjsString *name);
static EjsXML *resolve(Ejs *ejs, EjsXML *obj);
static EjsXML *shallowCopy(Ejs *ejs, EjsXML *xml);

/*********************************** Helpers **********************************/

static EjsXML *createXmlListVar(Ejs *ejs, EjsType *type, int size)
{
    return (EjsXML*) ejsCreateXMLList(ejs, NULL, N(NULL, NULL));
}


static EjsObj *cloneXmlList(Ejs *ejs, EjsXML *list, bool deep)
{
    EjsXML  *newList;

    //  TODO - implement deep copy
    newList = ejsCreateObj(ejs, TYPE(list), 0);
    if (newList == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    //  TODO incomplete
    return (EjsObj*) newList;
}


/*
    Cast the object operand to a primitive type
 */
static EjsObj *xlCast(Ejs *ejs, EjsXML *vp, EjsType *type)
{
    MprBuf      *buf;
    EjsObj      *result;
    EjsXML      *elt, *item;
    int         next;

    if (type == ESV(XML)) {
        return (EjsObj*) vp;
    }
    switch (type->sid) {
    case S_Object:

    case S_Boolean:
        return (EjsObj*) ejsCreateBoolean(ejs, 1);

    case S_Number:
        result = xlCast(ejs, vp, ESV(String));
        result = (EjsObj*) ejsToNumber(ejs, result);
        return result;

    case S_String:
        buf = mprCreateBuf(ME_MAX_BUFFER, -1);
        if (mprGetListLength(vp->elements) == 1) {
            elt = mprGetFirstItem(vp->elements);
            if (elt->kind == EJS_XML_ELEMENT) {
                if (elt->elements == 0) {
                    return (EjsObj*) ESV(empty);
                }
                if (elt->elements && mprGetListLength(elt->elements) == 1) {
                    //  TODO - what about PI and comments?
                    item = mprGetFirstItem(elt->elements);
                    if (item->kind == EJS_XML_TEXT) {
                        return (EjsObj*) item->value;
                    }
                }
            }
        }
        for (next = 0; (elt = mprGetNextItem(vp->elements, &next)) != 0; ) {
            if (ejsXMLToBuf(ejs, buf, elt, -1) < 0) {
                return 0;
            }
            if (next < vp->elements->length) {
                mprPutStringToBuf(buf, " ");
            }
        }
        return (EjsObj*) ejsCreateStringFromAsc(ejs, (char*) buf->start);

    default:
        ejsThrowTypeError(ejs, "Cannot cast to this type");
        return 0;
    }
}


//  TODO - seems the return code for delete should be boolean?

static int deleteXmlListPropertyByName(Ejs *ejs, EjsXML *list, EjsName qname)
{
    EjsXML      *elt;
    int         index, next;

    if (isdigit((uchar) qname.name->value[0]) && allDigitsForXmlList(qname.name)) {
        index = ejsAtoi(ejs, qname.name, 10);

        elt = (EjsXML*) mprGetItem(list->elements, index);
        if (elt) {
            if (elt->parent) {
                if (elt->kind == EJS_XML_ATTRIBUTE) {
                    ejsDeletePropertyByName(ejs, (EjsObj*) elt->parent, elt->qname);
                } else {
                    //  TODO - let q be the property of parent where parent[q] == x[i]
                    mprRemoveItem(elt->parent->elements, elt);
                    elt->parent = 0;
                }
            }
        }
        //  Spec says return true even if index is out of range. We return 0 for true and < 0 for false.
        //  TODO - should ejs throw?
        return 0;
    }

    for (next = 0; (elt = mprGetNextItem(list->elements, &next)) != 0; ) {
        if (elt->kind == EJS_XML_ELEMENT /* && elt->parent */) {
            ejsDeletePropertyByName(ejs, (EjsObj*) elt /* TODO was elt->parent */, qname);
        }
    }
    return 0;
}


static int getXmlListPropertyCount(Ejs *ejs, EjsXML *list)
{
    return mprGetListLength(list->elements);
}


/*
    Lookup a property by name. There are 7 kinds of lookups:
         prop, @att, [prop], *, @*, .name, .@name
 */
static EjsObj *getXmlListPropertyByName(Ejs *ejs, EjsXML *list, EjsName qname)
{
    EjsXML      *result, *subList, *item;
    int         nextItem;

    /*
        Get the n'th item in the list
     */
    if (isdigit((uchar) qname.name->value[0]) && allDigitsForXmlList(qname.name)) {
        return mprGetItem(list->elements, ejsAtoi(ejs, qname.name, 10));
    }

    result = ejsCreateXMLList(ejs, list, qname);

    /*
        Build a list of all the elements that themselves have a property qname
     */
    for (nextItem = 0; (item = mprGetNextItem(list->elements, &nextItem)) != 0; ) {
        if (item->kind == EJS_XML_ELEMENT) {
            subList = ejsGetPropertyByName(ejs, (EjsObj*) item, qname);
            assert(ejsIsXML(ejs, subList));
            ejsAppendToXML(ejs, result, subList);

        } else {
            //  TODO - do we ever get a list in a list?
            assert(0);
        }
    }
    return (EjsObj*) result;
}


static EjsObj *getXmlListNodeName(Ejs *ejs, EjsXML *xml, int argc, EjsObj **argv)
{
    if (xml->targetProperty.name) {
        return (EjsObj*) xml->targetProperty.name;
    } else if (xml->targetObject) {
        return (EjsObj*) xml->targetObject->qname.name;
    } else {
        return ESV(null);
    }
}



/*
    Function to iterate and return the next element name.
    NOTE: this is not a method of Xml. Rather, it is a callback function for Iterator
 */
static EjsObj *nextXmlListKey(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsXML  *xml;

    xml = (EjsXML*) ip->target;
    if (!ejsIsXML(ejs, xml)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }

    if (ip->index < mprGetListLength(xml->elements)) {
        return (EjsObj*) ejsCreateNumber(ejs, ip->index++);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return the default iterator. This returns the array index names.

    iterator native function get(): Iterator
 */
static EjsObj *getXmlListIterator(Ejs *ejs, EjsObj *xml, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, xml, -1, nextXmlListKey, 0, NULL);
}


/*
    Function to iterate and return the next element value.
    NOTE: this is not a method of Xml. Rather, it is a callback function for Iterator
 */
static EjsObj *nextXmlListValue(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsXML      *xml, *vp;

    xml = (EjsXML*) ip->target;
    if (!ejsIsXML(ejs, xml)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }

    for (; ip->index < mprGetListLength(xml->elements); ip->index++) {
        vp = (EjsXML*) mprGetItem(xml->elements, ip->index);
        if (vp == 0) {
            continue;
        }
        ip->index++;
        return (EjsObj*) vp;
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return an iterator to return the next array element value.

    iterator native function getValues(): Iterator
 */
static EjsObj *getXmlListValues(Ejs *ejs, EjsObj *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, ap, -1, nextXmlListValue, 0, NULL);
}


#if OLD
/*
    Handle all core operators. We currenly handle only === and !==
    TODO. Must implement: +, -, <, >, <=, >=, ==, ===, !=, !==, &, |
 */
static EjsObj *invokeOperator(Ejs *ejs, EjsXML *lhs, int opCode,  EjsXML *rhs)
{
    EjsObj      *l, *r;
    bool        boolResult;

    assert(ejsIsXML(ejs, lhs));
    assert(ejsIsXML(ejs, rhs));

    //  TODO - Complete
    switch (opCode) {
    case EJS_OP_COMPARE_EQ:
    case EJS_OP_COMPARE_STRICTLY_EQ:
        boolResult = (lhs == rhs);
        break;

    case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_STRICTLY_NE:
        boolResult = !(lhs == rhs);
        break;

    default:
        /*
            Cast to strings and re-invoke
         */
        l = ejsCast(ejs, lhs, String);
        r = ejsCast(ejs, rhs, String);
        return ejsInvokeOperator(ejs, l, opCode, r);
    }
    return (EjsObj*) ejsCreateBoolean(ejs, boolResult);
}
#endif


/*
    Set an alpha property by name.
 */
static int setAlphaPropertyByName(Ejs *ejs, EjsXML *list, EjsName qname, EjsObj *value)
{
    EjsXML      *elt, *targetObject;
    int         count;

    targetObject = 0;

    count = ejsGetLength(ejs, (EjsObj*) list);
    if (count > 1) {
        //  TODO - why no error in spec?
        assert(0);
        return 0;
    }

    if (count == 0) {
        /*
            Empty list so resolve the real target object and append it to the list.
         */
        targetObject = resolve(ejs, list);
        if (targetObject == 0) {
            return 0;
        }
        if (ejsGetLength(ejs, (EjsObj*) targetObject) != 1) {
            return 0;
        }
        ejsAppendToXML(ejs, list, targetObject);
    }

    /*
        Update the element
     */
    assert(ejsGetLength(ejs, (EjsObj*) list) == 1);
    elt = mprGetItem(list->elements, 0);                        //  TODO OPT - GetFirstItem
    assert(elt);
    ejsSetPropertyByName(ejs, elt, qname, value);
    return 0;
}


static EjsXML *createElement(Ejs *ejs, EjsXML *list, EjsXML *targetObject, EjsName qname, EjsObj *value)
{
    EjsXML      *elt, *last, *attList;
    int         index;
    int         j;

    if (targetObject && ejsIsXML(ejs, targetObject) && targetObject->kind == EJS_XML_LIST) {

        /*
            If the target is a list it must have 1 element. So switch to it.
            TODO - could we get resolve to do this?
         */
        if (mprGetListLength(targetObject->elements) != 1) {
            /* Spec says so - TODO why no error? */
            return 0;
        }
        targetObject = mprGetFirstItem(targetObject->elements);
    }

    /*
        Return if the target object is not an XML element
     */
    if (!ejsIsXML(ejs, targetObject) || targetObject->kind != EJS_XML_ELEMENT) {
            /* Spec says so - TODO why no error? */
        return 0;
    }

    elt = ejsCreateXML(ejs, EJS_XML_ELEMENT, list->targetProperty, targetObject, NULL);

    if (list->targetProperty.name && list->targetProperty.name->value[0] == '@') {
        elt->kind = EJS_XML_ATTRIBUTE;
        attList = ejsGetPropertyByName(ejs, (EjsObj*) targetObject, list->targetProperty);
        if (attList && mprGetListLength(attList->elements) > 0) {
            /* Spec says so. But this surely means you can't update an attribute? */
            return 0;
        }
    } else if (list->targetProperty.name == NULL || qname.name->value[0] == '*') {
        elt->kind = EJS_XML_TEXT;
        elt->qname.name = 0;
    }

    index = mprGetListLength(list->elements);

    if (elt->kind != EJS_XML_ATTRIBUTE) {
        if (targetObject) {
            if (index > 0) {
                /*
                    Find the place of the last list item in the resolved target object.
                 */
                last = mprGetItem(list->elements, index - 1);
                j = mprLookupItem(targetObject->elements, last);
            } else {
                j = -1;
            }
            if (j < 0) {
                j = mprGetListLength(targetObject->elements) - 1;
            }
            //  TODO - really need to wrap this ejsInsertXML(EjsXML *xml, int index, EjsXML *node)
            if (targetObject->elements == 0) {
                targetObject->elements = mprCreateList(-1, 0);
            }
            /*
                Insert into the target object
             */
            mprInsertItemAtPos(targetObject->elements, j + 1, elt);
        }

        if (ejsIsXML(ejs, value)) {
            if (((EjsXML*) value)->kind == EJS_XML_LIST) {
                elt->qname = ((EjsXML*) value)->targetProperty;
            } else {
                elt->qname = ((EjsXML*) value)->qname;
            }
        }

        /*
            Insert into the XML list
         */
        mprSetItem(list->elements, index, elt);
    }
    return (EjsXML*) mprGetItem(list->elements, index);
}


/*
    Update an existing element
 */
static int updateElement(Ejs *ejs, EjsXML *list, EjsXML *elt, int index, EjsObj *value)
{
    EjsXML      *node;
    int         i, j;

    if (!ejsIsXML(ejs, value)) {
        /* Not XML or XMLList -- convert to string */
        value = ejsCast(ejs, value, String);                //  TODO - seem to be doing this in too many places
    }
    mprSetItem(list->elements, index, value);

    if (elt->kind == EJS_XML_ATTRIBUTE) {
        assert(ejsIs(ejs, value, String));
        i = mprLookupItem(elt->parent->elements, elt);
        assert(i >= 0);
        ejsSetXMLElement(ejs, elt->parent, i, elt);
        //  TODO - why do this. Doesn't above do this?
        ejsSetPropertyByName(ejs, elt->parent, elt->qname, value);
        elt->value = (EjsString*) value;
    }

    if (ejsIsXML(ejs, value) && ((EjsXML*) value)->kind == EJS_XML_LIST) {
        value = (EjsObj*) shallowCopy(ejs, (EjsXML*) value);
        if (elt->parent) {
            index = mprLookupItem(elt->parent->elements, elt);
            assert(index >= 0);
            for (j = 0; j < mprGetListLength(((EjsXML*) value)->elements); j++) {
                mprInsertItemAtPos(elt->parent->elements, index, value);
            }
        }

    } else if (ejsIsXML(ejs, value) || elt->kind != EJS_XML_ELEMENT) {
        if (elt->parent) {
            index = mprLookupItem(elt->parent->elements, elt);
            assert(index >= 0);
            mprSetItem(elt->parent->elements, index, value);
            ((EjsXML*) value)->parent = elt->parent;
            if (ejsIs(ejs, value, String)) {
                node = ejsCreateXML(ejs, EJS_XML_TEXT, N(NULL, NULL), list, (EjsString*) value);
                mprSetItem(list->elements, index, node);
            } else {
                mprSetItem(list->elements, index, value);
            }
        }

    } else {
        ejsSetPropertyByName(ejs, elt, N(NULL, "*"), value);
    }
    return index;
}


/*
    Set a property by name.
 */
static int setXmlListPropertyByName(Ejs *ejs, EjsXML *list, EjsName qname, EjsObj *value)
{
    EjsXML      *elt, *targetObject;
    int         index;

    if (!isdigit((uchar) qname.name->value[0])) {
        return setAlphaPropertyByName(ejs, list, qname, value);
    }

    /*
        Numeric property
     */
    targetObject = 0;
    if (list->targetObject) {
        /*
            Find the real underlying target object. May be an XML object or XMLList if it contains multiple elements.
         */
        targetObject = resolve(ejs, list->targetObject);
        if (targetObject == 0) {
            /* Spec says so - TODO why no error? */
            return 0;
        }
    }
    index = ejsAtoi(ejs, qname.name, 10);
    if (index >= mprGetListLength(list->elements)) {
        /*
            Create, then fall through to update
         */
        elt = createElement(ejs, list, targetObject, qname, value);
        if (elt == 0) {
            return 0;
        }

    } else {
        elt = mprGetItem(list->elements, index);
    }
    assert(elt);
    updateElement(ejs, list, elt, index, value);
    return index;
}


/*
    function parent(): XML
 */
static EjsObj *xl_parent(Ejs *ejs, EjsXML *xml, int argc, EjsObj **argv)
{
    return xml->targetObject ? (EjsObj*) xml->targetObject : (EjsObj*) ESV(null);
}

/******************************** Support Routines **************************/

static bool allDigitsForXmlList(EjsString *name)
{
    wchar       *cp;

    for (cp = name->value; *cp; cp++) {
        if (!isdigit((uchar) *cp) || *cp == '.') {
            return 0;
        }
    }
    return 1;
}


static EjsXML *shallowCopy(Ejs *ejs, EjsXML *xml)
{
    EjsXML      *root, *elt;
    int         next;

    assert(xml->kind == EJS_XML_LIST);

    if (xml == 0) {
        return 0;
    }
    if ((root = ejsCreateXMLList(ejs, xml->targetObject, xml->targetProperty)) == NULL) {
        return 0;
    }
    if (xml->elements) {
        root->elements = mprCreateList(-1, 0);
        for (next = 0; (elt = mprGetNextItem(xml->elements, &next)) != 0; ) {
            assert(ejsIsXML(ejs, elt));
            if (elt) {
                mprAddItem(root->elements, elt);
            }
        }
    }
    if (mprHasMemError()) {
        return 0;
    }
    return root;
}


/*
    Resolve empty XML list objects to an actual XML object. This is used by SetPropertyByName to find the actual
    object to update. This method resolves the value of empty XMLLists. If the XMLList is not empty, the list will
    be returned. If list is empty, this method attempts to create an element based on the list targetObject and
    targetProperty.
 */
static EjsXML *resolve(Ejs *ejs, EjsXML *xml)
{
    EjsXML  *targetObject, *targetPropertyList;

    if (!ejsIsXML(ejs, xml) || xml->kind != EJS_XML_LIST) {
        /* Resolved to an XML object */
        return xml;
    }
    if (mprGetListLength(xml->elements) > 0) {
        /* Resolved to a list of items */
        return xml;
    }
    if (xml->targetObject == 0 || xml->targetProperty.name == NULL || xml->targetProperty.name->value[0] == '*') {
        /* End of chain an no more target objects */
        return 0;
    }
    targetObject = resolve(ejs, xml->targetObject);
    if (targetObject == 0) {
        return 0;
    }
    //  TODO - OPT. targetPropertyList is also being created below.
    targetPropertyList = ejsGetPropertyByName(ejs, (EjsObj*) targetObject, xml->targetProperty);
    if (targetPropertyList == 0) {
        return 0;
    }
    if (ejsGetLength(ejs, (EjsObj*) targetPropertyList) == 0) {
        /*
            Property does not exist in the target.
         */
        if (targetObject->kind == EJS_XML_LIST && ejsGetLength(ejs, (EjsObj*) targetObject) > 1) {
            return 0;
        }
        /*
            Create the property as an element (The text value will be optimized away).
         */
        ejsSetPropertyByName(ejs, targetObject, xml->targetProperty, ESV(empty));
        targetPropertyList = ejsGetPropertyByName(ejs, (EjsObj*) targetObject, xml->targetProperty);
    }
    return targetPropertyList;
}


/************************************ Methods ********************************/

static EjsObj *xmlListConstructor(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
#if FUTURE
    EjsObj      *vp;
    cchar       *str;

    if (argc == 1) {
        vp = argv[0];

        if (ejsIsObject(vp)) {
            /* Convert DOM to XML. Not implemented */;

        } else if (ejsIs(ejs, vp, String)) {
            str = ((EjsString*) vp)->value;
            if (str == 0) {
                return 0;
            }
            if (*str == '<') {
                /* XML Literal */
                return loadXmlString(ejs, (EjsXML*) thisObj, str);

            } else {
                /* Load from file */
                return load(ejs, (EjsXML*) thisObj, argc, argv);
            }
        } else {
            ejsThrowArgError(ejs, "Bad type passed to XML constructor");
            return 0;
        }
    }
#endif
    return (EjsObj*) thisObj;
}


/*
    Convert to a JSON string

    override function toJSON(): String
 */
static EjsObj *xmlListToJson(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    EjsString       *sp;
    MprBuf          *buf;
    cchar           *cp;

    /*
        Quote all quotes
     */
    sp = ejsToString(ejs, vp);
    buf = mprCreateBuf(-1, -1);
    mprPutCharToBuf(buf, '"');
    for (cp = ejsToMulti(ejs, sp); *cp; cp++) {
        if (*cp == '"') {
            mprPutCharToBuf(buf, '\\');
        }
        mprPutCharToBuf(buf, (uchar) *cp);
    }
    mprPutCharToBuf(buf, '"');
    mprAddNullToBuf(buf);
    return (EjsObj*) ejsCreateStringFromAsc(ejs, mprGetBufStart(buf));
}


/*
    Convert the XML object to a string.

    function toString() : String
 */
static EjsObj *xmlListToString(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    return (TYPE(vp)->helpers.cast)(ejs, vp, ESV(String));
}


/*
    Get the length of an array.
    @return Returns the number of items in the array

    public override function get length(): int
 */

static EjsObj *xlLength(Ejs *ejs, EjsXML *xml, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, mprGetListLength(xml->elements));
}


#if FUTURE
/*
    Set the length. TODO - what does this do?
    public override function set length(value: int): void
 */
static EjsObj *setLength(Ejs *ejs, EjsXMLList *xml, int argc, EjsObj **argv)
{
    int         length;

    assert(ejsIsXMLList(ejs, xml));

    if (argc != 1) {
        ejsThrowArgError(ejs, "usage: obj.length = value");
        return 0;
    }
    length = ejsVarToInteger(ejs, argv[0]);

#if KEEP
    if (length < ap->length) {
        for (i = length; i < ap->length; i++) {
            if (ejsSetProperty(ejs, ap, i, ESV(undefined)) < 0) {
                //  TODO - DIAG
                return 0;
            }
        }

    } else if (length > ap->length) {
        if (ejsSetProperty(ejs, ap, length - 1, ESV(undefined)) < 0) {
            //  TODO - DIAG
            return 0;
        }
    }

    ap->length = length;
#endif
    return 0;
}
#endif


/*********************************** Factory **********************************/

PUBLIC EjsXML *ejsCreateXMLList(Ejs *ejs, EjsXML *targetObject, EjsName targetProperty)
{
    EjsXML      *list;

    if ((list = (EjsXML*) ejsAlloc(ejs, ESV(XMLList), 0)) == NULL) {
        return 0;
    }
    list->kind = EJS_XML_LIST;
    list->elements = mprCreateList(-1, 0);
    list->targetObject = targetObject;

    if (targetProperty.name) {
        list->targetProperty.name = targetProperty.name;
    }
    return list;
}


PUBLIC void ejsCreateXMLListType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsCreateCoreType(ejs, N("ejs", "XMLList"), sizeof(EjsXML), S_XMLList, ES_XMLList_NUM_CLASS_PROP,
        ejsManageXML, EJS_TYPE_OBJ);

    /*
        Must not bind as XML uses get/setPropertyByName to defer to user XML elements over XML methods
     */
    type->constructor.block.nobind = 1;

    type->helpers.clone = (EjsCloneHelper) cloneXmlList;
    type->helpers.cast = (EjsCastHelper) xlCast;
    type->helpers.create = (EjsCreateHelper) createXmlListVar;
    type->helpers.getPropertyByName = (EjsGetPropertyByNameHelper) getXmlListPropertyByName;
    type->helpers.getPropertyCount = (EjsGetPropertyCountHelper) getXmlListPropertyCount;
    type->helpers.deletePropertyByName = (EjsDeletePropertyByNameHelper) deleteXmlListPropertyByName;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) ejsInvokeOperatorDefault;
    type->helpers.setPropertyByName = (EjsSetPropertyByNameHelper) setXmlListPropertyByName;
}


PUBLIC void ejsConfigureXMLListType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "XMLList"))) == 0) {
        return;
    }
    prototype = type->prototype;
    ejsBindConstructor(ejs, type, xmlListConstructor);
    ejsBindMethod(ejs, prototype, ES_XMLList_length, xlLength);
    ejsBindMethod(ejs, prototype, ES_XMLList_name, getXmlListNodeName);
    ejsBindMethod(ejs, prototype, ES_XMLList_parent, (EjsProc) xl_parent);
#if FUTURE
    ejsBindMethod(ejs, prototype, "name", name, NULL);
    ejsBindMethod(ejs, prototype, "valueOf", valueOf, NULL);
#endif
    ejsBindMethod(ejs, prototype, ES_XMLList_toJSON, xmlListToJson);
    ejsBindMethod(ejs, prototype, ES_XMLList_toString, xmlListToString);
    ejsBindMethod(ejs, prototype, ES_XMLList_iterator_get, getXmlListIterator);
    ejsBindMethod(ejs, prototype, ES_XMLList_iterator_getValues, getXmlListValues);
}


/*
    @copy   default

    Copyright (c) Embedthis Software. All Rights Reserved.

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
