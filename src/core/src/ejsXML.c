/**
    ejsXML.c - XML type.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/****************************** Forward Declarations **************************/
/*
    XML methods
 */
static EjsObj *loadXml(Ejs *ejs, EjsXML *xml, int argc, EjsObj **argv);
static EjsObj *saveXml(Ejs *ejs, EjsXML *xml, int argc, EjsObj **argv);
static EjsObj *xmlToString(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv);
static EjsObj *xml_parent(Ejs *ejs, EjsXML *xml, int argc, EjsObj **argv);

static bool allDigitsForXml(EjsString *name);
static bool deepCompare(EjsXML *lhs, EjsXML *rhs);
static ssize readStringData(MprXml *xp, void *data, char *buf, ssize size);
static ssize readFileData(MprXml *xp, void *data, char *buf, ssize size);

/*********************************** Helpers **********************************/

static EjsXML *createXml(Ejs *ejs, EjsType *type, int size)
{
    return ejsCreateXML(ejs, 0, N(NULL, NULL), NULL, NULL);
}


PUBLIC EjsAny *cloneXml(Ejs *ejs, EjsXML *xml, bool deep)
{
    EjsXML      *root, *elt;
    int         next;

    if (xml == 0) {
        return 0;
    }
    if (xml->kind == EJS_XML_LIST) {
        root = ejsCreateXMLList(ejs, xml->targetObject, xml->targetProperty);
    } else {
        root = ejsCreateXML(ejs, xml->kind, xml->qname, NULL, xml->value);
    }
    if (root == 0) {
        return 0;
    }
    //  TODO - must copy inScopeNamespaces?

    if (xml->attributes) {
        root->attributes = mprCreateList(-1, 0);
        for (next = 0; (elt = (EjsXML*) mprGetNextItem(xml->attributes, &next)) != 0; ) {
            elt = ejsClone(ejs, elt, 1);
            if (elt) {
                elt->parent = root;
                mprAddItem(root->attributes, elt);
            }
        }
    }
    if (xml->elements) {
        root->elements = mprCreateList(-1, 0);
        for (next = 0; (elt = mprGetNextItem(xml->elements, &next)) != 0; ) {
            assert(ejsIsXML(ejs, elt));
            elt = ejsClone(ejs, elt, 1);
            if (elt) {
                elt->parent = root;
                mprAddItem(root->elements, elt);
            }
        }
    }
    if (mprHasMemError(ejs)) {
        return 0;
    }
    return root;
}


/*
    Cast the object operand to a primitive type
 */
static EjsAny *castXml(Ejs *ejs, EjsXML *xml, EjsType *type)
{
    EjsXML      *item;
    EjsObj      *result;
    MprBuf      *buf;

    assert(ejsIsXML(ejs, xml));

    if (type == ESV(XMLList)) {
        return xml;
    }

    switch (type->sid) {
    case S_Object:

    case S_Boolean:
        return ejsCreateBoolean(ejs, 1);

    case S_Number:
        result = castXml(ejs, xml, ESV(String));
        return ejsToNumber(ejs, result);

    case S_String:
        if (xml->kind == EJS_XML_ELEMENT) {
            if (xml->elements == 0) {
                return ESV(empty);
            }
            if (xml->elements && mprGetListLength(xml->elements) == 1) {
                //  TODO - what about PI and comments?
                item = mprGetFirstItem(xml->elements);
                if (item->kind == EJS_XML_TEXT) {
                    return item->value;
                }
            }
        }
        buf = mprCreateBuf(ME_MAX_BUFFER, -1);
        if (ejsXMLToBuf(ejs, buf, xml, -1) < 0) {
            return 0;
        }
        return ejsCreateStringFromAsc(ejs, (char*) buf->start);

    default:
        ejsThrowTypeError(ejs, "Cannot cast to this type");
        return 0;
    }
}


static int deleteXmlPropertyByName(Ejs *ejs, EjsXML *xml, EjsName qname)
{
    EjsXML      *item;
    bool        removed;
    int         next;

    removed = 0;

    if (qname.name->value[0] == '@') {
        /* @ and @* */
        if (xml->attributes) {
            for (next = 0; (item = mprGetNextItem(xml->attributes, &next)) != 0; ) {
                assert(qname.name->value[0] == '@');
                if (qname.name->value[1] == '*' || wcmp(item->qname.name->value, &qname.name->value[1]) == 0) {
                    mprRemoveItemAtPos(xml->attributes, next - 1);
                    item->parent = 0;
                    removed = 1;
                    next -= 1;
                }
            }
        }

    } else {
        /* name and * */
        if (xml->elements) {
            for (next = 0; (item = mprGetNextItem(xml->elements, &next)) != 0; ) {
                assert(item->qname.name);
                if (qname.name->value[0] == '*' || ejsCompareString(ejs, item->qname.name, qname.name) == 0) {
                    mprRemoveItemAtPos(xml->elements, next - 1);
                    item->parent = 0;
                    removed = 1;
                    next -= 1;
                }
            }
        }
    }
    return (removed) ? 0 : EJS_ERR;
}


static EjsObj *getXmlNodeName(Ejs *ejs, EjsXML *xml, int argc, EjsObj **argv)
{
    return (EjsObj*) xml->qname.name;
}


/*
    Function to iterate and return the next element name.
    NOTE: this is not a method of Xml. Rather, it is a callback function for Iterator
 */
static EjsObj *nextXmlKey(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
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
static EjsObj *getXmlIterator(Ejs *ejs, EjsObj *xml, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, xml, -1, nextXmlKey, 0, NULL);
}


/*
    Function to iterate and return the next element value.
    NOTE: this is not a method of Xml. Rather, it is a callback function for Iterator
 */
static EjsObj *nextXmlValue(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
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
static EjsObj *getXmlValues(Ejs *ejs, EjsObj *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, ap, -1, nextXmlValue, 0, NULL);
}


static int getXmlPropertyCount(Ejs *ejs, EjsXML *xml)
{
    return mprGetListLength(xml->elements);
}


/*
    Lookup a property by name. There are 7 kinds of lookups:
         prop, @att, [prop], *, @*, .name, .@name
 */
static EjsObj *getXmlPropertyByName(Ejs *ejs, EjsXML *xml, EjsName qname)
{
    EjsXML      *item, *result, *list;
    int         next, nextList;

    result = 0;

    assert(xml->kind < 5);
    if (isdigit((uchar) qname.name->value[0]) && allDigitsForXml(qname.name)) {
        /*
            Consider xml as a list with only one entry == xml. Then return the 0'th entry
         */
        return (EjsObj*) xml;
    }

    if (qname.name->value[0] == '@') {
        /* @ and @* */
        result = ejsCreateXMLList(ejs, xml, qname);
        if (xml->attributes) {
            for (next = 0; (item = mprGetNextItem(xml->attributes, &next)) != 0; ) {
                assert(qname.name->value[0] == '@');
                if (qname.name->value[1] == '*' || wcmp(item->qname.name->value, &qname.name->value[1]) == 0) {
                    result = ejsAppendToXML(ejs, result, item);
                }
            }
        }

    } else if (qname.name->value[0] == '.') {
        /* Decenders (do ..@ also) */
        result = ejsGetXMLDescendants(ejs, xml, qname);

    } else {
        /* name and * */
        result = ejsCreateXMLList(ejs, xml, qname);
        if (xml->elements) {
            for (next = 0; (item = mprGetNextItem(xml->elements, &next)) != 0; ) {
                if (item->kind == EJS_XML_LIST) {
                    list = item;
                    for (nextList = 0; (item = mprGetNextItem(list->elements, &nextList)) != 0; ) {
                        assert(item->qname.name);
                        if (qname.name->value[0] == '*' || ejsCompareString(ejs, item->qname.name, qname.name) == 0) {
                            result = ejsAppendToXML(ejs, result, item);
                        }
                    }

                } else if (item->qname.name) {
                    assert(item->qname.name);
                    if (qname.name->value[0] == '*' || ejsCompareString(ejs, item->qname.name, qname.name) == 0) {
                        result = ejsAppendToXML(ejs, result, item);
                    }
                }
            }
        }
    }
    return (EjsObj*) result;
}


static EjsObj *invokeXmlOperator(Ejs *ejs, EjsXML *lhs, int opcode,  EjsXML *rhs)
{
    EjsObj      *result;

    if ((result = ejsCoerceOperands(ejs, (EjsObj*) lhs, opcode, (EjsObj*) rhs)) != 0) {
        return result;
    }
    switch (opcode) {
    case EJS_OP_COMPARE_EQ:
        return (EjsObj*) ejsCreateBoolean(ejs, deepCompare(lhs, rhs));

    case EJS_OP_COMPARE_NE:
        return (EjsObj*) ejsCreateBoolean(ejs, !deepCompare(lhs, rhs));

    default:
        return ejsInvokeOperatorDefault(ejs, (EjsObj*) lhs, opcode, (EjsObj*) rhs);
    }
}


/*
    Set a property attribute by name.
 */
static int setXmlPropertyAttributeByName(Ejs *ejs, EjsXML *xml, EjsName qname, EjsObj *value)
{
    EjsXML      *elt, *attribute, *xvalue, *lastElt;
    EjsString   *sv;
    EjsName     qn;
    wchar       *str;
    int         index, last, next;

    /*
        Attribute. If the value is an XML list, convert to a space separated string
     */
    xvalue = (EjsXML*) value;
    if (ejsIsXML(ejs, xvalue) && xvalue->kind == EJS_XML_LIST) {
        str = 0;
        for (next = 0; (elt = mprGetNextItem(xvalue->elements, &next)) != 0; ) {
            sv = (EjsString*) ejsCast(ejs, (EjsObj*) elt, String);
            str = mrejoin(str, NULL, " ", sv->value, NULL);
        }
        value = (EjsObj*) ejsCreateString(ejs, str, -1);

    } else {
        value = ejsCast(ejs, value, String);
    }
    assert(ejsIs(ejs, value, String));

    /*
        Find the first attribute that matches. Delete all other attributes of the same name.
     */
    index = 0;
    if (xml->attributes) {
        lastElt = 0;
        for (last = -1, index = -1; (elt = mprGetPrevItem(xml->attributes, &index)) != 0; ) {
            assert(qname.name->value[0] == '@');
            if (wcmp(elt->qname.name->value, &qname.name->value[1]) == 0) {
                if (last >= 0) {
                    mprRemoveItemAtPos(xml->attributes, last);
                }
                last = index;
                lastElt = elt;
            }
        }
        if (lastElt) {
            /*
                Found a match. So replace its value
             */
            lastElt->value = (EjsString*) value;
            return last;

        } else {
            index = mprGetListLength(xml->attributes);
        }
    }
    //  TODO - namespace work to do here

    /*
        Not found. Create a new attribute node
     */
    assert(ejsIs(ejs, value, String));
    qn.space = NULL;
    qn.name = ejsSubstring(ejs, qname.name, 1, -1);
    attribute = ejsCreateXML(ejs, EJS_XML_ATTRIBUTE, qn, xml, (EjsString*) value);
    if (xml->attributes == 0) {
        xml->attributes = mprCreateList(-1, 0);
    }
    mprSetItem(xml->attributes, index, attribute);
    return index;
}


/*
    Create a value node. If the value is an XML node already, we are done. Otherwise, cast the value to a string
    and create a text child node containing the string value.
 */
static EjsXML *createValueNode(Ejs *ejs, EjsXML *elt, EjsObj *value)
{
    EjsXML      *text;
    EjsString   *str;

    if (ejsIsXML(ejs, value)) {
        return (EjsXML*) value;
    }
    if ((str = (EjsString*) ejsCast(ejs, value, String)) == NULL) {
        return 0;
    }
    if (mprGetListLength(elt->elements) == 1) {
        /*
            Update an existing text element
         */
        text = mprGetFirstItem(elt->elements);
        if (text->kind == EJS_XML_TEXT) {
            text->value = str;
            return elt;
        }
    }

    /*
        Create a new text element
     */
    if (str->value[0] != '\0') {
        text = ejsCreateXML(ejs, EJS_XML_TEXT, N(NULL, NULL), elt, str);
        elt = ejsAppendToXML(ejs, elt, text);
    }
    return elt;
}


/*
    Set a property by name
    There are 7 kinds of qname's: prop, @att, [prop], *, @*, .name, .@name
 */
static int setXmlPropertyByName(Ejs *ejs, EjsXML *xml, EjsName qname, EjsObj *value)
{
    EjsXML      *elt, *xvalue, *rp, *lastElt;
    EjsObj      *originalValue;
    int         index, last;

    last = 0;
    lastElt = 0;

    if (isdigit((uchar) qname.name->value[0]) && allDigitsForXml(qname.name)) {
        ejsThrowTypeError(ejs, "Integer indicies for set are not allowed");
        return EJS_ERR;
    }

    if (xml->kind != EJS_XML_ELEMENT) {
        //  TODO spec requires this -- but why? -- surely throw?
        return 0;
    }

    /*
        Massage the value type.
     */
    originalValue = value;

    xvalue = (EjsXML*) value;
    if (ejsIsXML(ejs, xvalue)) {
        if (xvalue->kind == EJS_XML_LIST) {
            value = cloneXml(ejs, xvalue, 1);

        } else if (xvalue->kind == EJS_XML_TEXT || xvalue->kind == EJS_XML_ATTRIBUTE) {
            value = ejsCast(ejs, originalValue, String);

        } else {
            value = cloneXml(ejs, xvalue, 1);
        }
    } else {
        value = ejsCast(ejs, value, String);
    }
    if (qname.name->value[0] == '@') {
        return setXmlPropertyAttributeByName(ejs, xml, qname, value);
    }
    /*
        Delete redundant elements by the same name.
     */
    if (xml->elements) {
        for (last = -1, index = -1; (elt = mprGetPrevItem(xml->elements, &index)) != 0; ) {
            if (qname.name->value[0] == '*' || (elt->kind == EJS_XML_ELEMENT && elt->qname.name == qname.name)) {
                /*
                    Must remove all redundant elements of the same name except the first one
                 */
                if (last >= 0) {
                    rp = mprGetItem(xml->elements, last);
                    rp->parent = 0;
                    mprRemoveItemAtPos(xml->elements, last);
                }
                last = index;
                lastElt = elt;
            }
        }
    }
    if (xml->elements == 0) {
        //  TODO - need routine to do this centrally so we can control the default number of elements in the list?
        xml->elements = mprCreateList(-1, 0);
    }
    elt = lastElt;
    index = last;

    if (qname.name->value[0] == '*') {
        /*
            Special case when called from XMLList to update the value of an element
         */
        xml = createValueNode(ejs, xml, value);

    } else if (elt == 0) {
        /*
            Not found. New node required.
         */
        elt = ejsCreateXML(ejs, EJS_XML_ELEMENT, qname, xml, NULL);
        if (elt == 0) {
            return 0;
        }
        index = mprGetListLength(xml->elements);
        xml = ejsAppendToXML(ejs, xml, createValueNode(ejs, elt, value));

    } else {
        /*
            Update existing element.
         */
        xml = ejsSetXMLElement(ejs, xml, index, createValueNode(ejs, elt, value));
    }

    if (xml == 0) {
        return EJS_ERR;
    }
    return index;
}


/****************************** Support Routines ****************************/
/*
    Deep compare
 */
static bool deepCompare(EjsXML *lhs, EjsXML *rhs)
{
    EjsXML      *l, *r;
    int         i;

    if (lhs == rhs) {
        return 1;
    }
    //  TODO - must compare all the namespaces?
    if (lhs->kind != rhs->kind) {
        return 0;

    } else  if (lhs->qname.name != rhs->qname.name) {
        return 0;

    } else if (mprGetListLength(lhs->attributes) != mprGetListLength(rhs->attributes)) {
        //  TODO - must compare all the attributes
        return 0;

    } else if (mprGetListLength(lhs->elements) != mprGetListLength(rhs->elements)) {
        //  TODO - must compare all the children
        return 0;

    } else if (lhs->value != rhs->value) {
        return 0;

    } else {
        for (i = 0; i < mprGetListLength(lhs->elements); i++) {
            l = mprGetItem(lhs->elements, i);
            r = mprGetItem(rhs->elements, i);
            if (! deepCompare(l, r)) {
                return 0;
            }
        }
    }
    return 1;
}


PUBLIC EjsXML *ejsGetXMLDescendants(Ejs *ejs, EjsXML *xml, EjsName qname)
{
    EjsXML          *item, *result;
    int             next;

    result = ejsCreateXMLList(ejs, xml, qname);
    if (result == 0) {
        return 0;
    }
    if (qname.name->value[0] == '.' && qname.name->value[1] == '@') {
        if (xml->attributes) {
            for (next = 0; (item = mprGetNextItem(xml->attributes, &next)) != 0; ) {
                if (qname.name->value[2] == '*' || wcmp(item->qname.name->value, &qname.name->value[2]) == 0) {
                    result = ejsAppendToXML(ejs, result, item);
                }
            }
        }
        if (xml->elements) {
            for (next = 0; (item = mprGetNextItem(xml->elements, &next)) != 0; ) {
                result = ejsAppendToXML(ejs, result, ejsGetXMLDescendants(ejs, item, qname));
            }
        }

    } else {
        if (xml->elements) {
            for (next = 0; (item = mprGetNextItem(xml->elements, &next)) != 0; ) {
                if (qname.name->value[0] == '*' || wcmp(item->qname.name->value, &qname.name->value[1]) == 0) {
                    result = ejsAppendToXML(ejs, result, item);
                } else {
                    result = ejsAppendToXML(ejs, result, ejsGetXMLDescendants(ejs, item, qname));
                }
            }
        }
    }
    return result;
}


/************************************ Methods ********************************/
/*
    native function XML(value: Object = null)
 */
static EjsObj *xmlConstructor(Ejs *ejs, EjsXML *thisObj, int argc, EjsObj **argv)
{
    EjsObj      *arg, *vp;
    wchar       *str;

    //  TODO - should be also able to handle a File object

    if (thisObj == 0) {
        /*
            Called as a function - cast the arg
         */
        if (argc > 0){
            if ((arg = ejsCast(ejs, argv[0], String)) == 0) {
                return 0;
            }
        }
        thisObj = ejsCreateXML(ejs, 0, N(NULL, NULL), NULL, NULL);
    }
    if (argc == 0) {
        return (EjsObj*) thisObj;
    }

    arg = argv[0];
    assert(arg);

    if (!ejsIsDefined(ejs, arg)) {
        return (EjsObj*) thisObj;
    }
    arg = ejsCast(ejs, argv[0], String);
    if (arg && ejsIs(ejs, arg, String)) {
        str = ((EjsString*) arg)->value;
        if (str == 0) {
            return 0;
        }
        while (isspace((uchar) *str)) str++;
        if (*str == '<') {
            /* XML Literal */
            ejsLoadXMLString(ejs, thisObj, (EjsString*) arg);

        } else {
            /* Load from file */
            loadXml(ejs, thisObj, argc, argv);
        }
    } else if (arg && ejsIsXML(ejs, arg)) {
        if ((vp = xmlToString(ejs, argv[0], 0, NULL)) != 0) {
            ejsLoadXMLString(ejs, thisObj, (EjsString*) vp);
        }

    } else {
        ejsThrowArgError(ejs, "Bad type passed to XML constructor");
        return 0;
    }
    return (EjsObj*) thisObj;
}


static EjsObj *loadXml(Ejs *ejs, EjsXML *xml, int argc, EjsObj **argv)
{
    MprFile     *file;
    MprXml      *xp;
    cchar       *filename;

    assert(argc == 1 && ejsIs(ejs, argv[0], String));

    filename = ejsToMulti(ejs, argv[0]);
    file = mprOpenFile(filename, O_RDONLY, 0664);
    if (file == 0) {
        ejsThrowIOError(ejs, "Cannot open: %s", filename);
        return 0;
    }
    //  TODO - convert to open/close
    xp = ejsCreateXmlParser(ejs, xml, filename);
    if (xp == 0) {
        ejsThrowMemoryError(ejs);
        mprCloseFile(file);
        return 0;
    }
    mprXmlSetInputStream(xp, readFileData, (void*) file);

    if (mprXmlParse(xp) < 0 && !ejsHasException(ejs)) {
        ejsThrowIOError(ejs, "Cannot parse XML file: %s\nDetails %s",  filename, mprXmlGetErrorMsg(xp));
    }
    mprCloseFile(file);
    return 0;
}


static EjsObj *saveXml(Ejs *ejs, EjsXML *xml, int argc, EjsObj **argv)
{
    MprBuf      *buf;
    MprFile     *file;
    char        *filename;
    ssize       bytes, len;

    if (argc != 1 || !ejsIs(ejs, argv[0], String)) {
        ejsThrowArgError(ejs, "Bad args. Usage: save(filename);");
        return 0;
    }
    filename = awtom(((EjsString*) argv[0])->value, NULL);

    /*
        Create a buffer to hold the output. All in memory.
     */
    buf = mprCreateBuf(ME_MAX_BUFFER, -1);
    mprPutStringToBuf(buf, "<?xml version=\"1.0\"?>\n");

    if (ejsXMLToBuf(ejs, buf, xml, 0) < 0) {
        return 0;
    }
    file = mprOpenFile(filename,  O_CREAT | O_TRUNC | O_WRONLY | O_TEXT, 0664);
    if (file == 0) {
        ejsThrowIOError(ejs, "Cannot open: %s, %d", filename, mprGetOsError(ejs));
        return 0;
    }
    len = mprGetBufLength(buf);
    bytes = mprWriteFile(file, buf->start, len);
    if (bytes != len) {
        ejsThrowIOError(ejs, "Cannot write to: %s", filename);
        mprCloseFile(file);
        return 0;
    }
    mprWriteFile(file, "\n", 1);
    mprCloseFile(file);
    return 0;
}


/*
    Convert to a JSON string

    override function toJSON(): String
 */
static EjsString *xmlToJson(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
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
    return ejsCreateStringFromAsc(ejs, mprGetBufStart(buf));
}


/*
    Convert the XML object to a string.

    function toString() : String
 */
static EjsObj *xmlToString(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    return (TYPE(obj)->helpers.cast)(ejs, obj, ESV(String));
}


/*
    Get the length of an array.
    @return Returns the number of items in the array

    public override function get length(): int
 */
static EjsObj *xmlLength(Ejs *ejs, EjsXML *xml, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, mprGetListLength(xml->elements));
}


#if KEEP
/*
    Set the length. TODO - what does this do?
    public override function set length(value: int): void
 */
static EjsObj *setLength(Ejs *ejs, EjsXML *xml, int argc, EjsObj **argv)
{
    int         length;

    assert(ejsIsXML(ejs, xml));

    if (argc != 1) {
        ejsThrowArgError(ejs, "usage: obj.length = value");
        return 0;
    }
    length = ejsVarToInteger(ejs, argv[0]);

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
    return 0;
}
#endif


/*
    function parent(): XML
 */
static EjsObj *xml_parent(Ejs *ejs, EjsXML *xml, int argc, EjsObj **argv)
{
    return (xml->parent && xml != xml->parent) ? (EjsObj*) xml->parent : (EjsObj*) ESV(null);
}

/********************************** Support **********************************/
/*
    Set an indexed element to an XML value
 */
PUBLIC EjsXML *ejsSetXMLElement(Ejs *ejs, EjsXML *xml, int index, EjsXML *node)
{
    EjsXML      *old;

    if (xml == 0 || node == 0) {
        return 0;
    }
    if (xml->elements == 0) {
        xml->elements = mprCreateList(-1, 0);

    } else {
        old = (EjsXML*) mprGetItem(xml->elements, index);
        if (old && old != node) {
            old->parent = 0;
        }
    }

    if (xml->kind != EJS_XML_LIST) {
        node->parent = xml;
    }
    mprSetItem(xml->elements, index, node);
    return xml;
}


PUBLIC EjsXML *ejsAppendToXML(Ejs *ejs, EjsXML *xml, EjsXML *node)
{
    EjsXML      *elt;
    int         next;

    if (xml == 0 || node == 0) {
        return 0;
    }
    if (xml->elements == 0) {
        xml->elements = mprCreateList(-1, 0);
    }
    if (node->kind == EJS_XML_LIST) {
        for (next = 0; (elt = mprGetNextItem(node->elements, &next)) != 0; ) {
            if (xml->kind != EJS_XML_LIST) {
                elt->parent = xml;
            }
            mprAddItem(xml->elements, elt);
        }
        xml->targetObject = node->targetObject;
        xml->targetProperty = node->targetProperty;

    } else {
        if (xml->kind != EJS_XML_LIST) {
            node->parent = xml;
        }
        mprAddItem(xml->elements, node);
    }
    return xml;
}


PUBLIC int ejsAppendAttributeToXML(Ejs *ejs, EjsXML *parent, EjsXML *node)
{
    if (parent->attributes == 0) {
        parent->attributes = mprCreateList(-1, 0);
    }
    node->parent = parent;
    return mprAddItem(parent->attributes, node);
}


static ssize readFileData(MprXml *xp, void *data, char *buf, ssize size)
{
    assert(xp);
    assert(data);
    assert(buf);
    assert(size > 0);

    return mprReadFile((MprFile*) data, buf, size);
}


static ssize readStringData(MprXml *xp, void *data, char *buf, ssize size)
{
    EjsXmlState *parser;
    ssize       len, rc;

    assert(xp);
    assert(buf);
    assert(size > 0);

    parser = (EjsXmlState*) xp->parseArg;

    if (parser->inputPos < parser->inputSize) {
        len = min(size, (parser->inputSize - parser->inputPos));
        rc = mprMemcpy(buf, size, &parser->inputBuf[parser->inputPos], len);
        parser->inputPos += len;
        return rc;
    }
    return 0;
}


static bool allDigitsForXml(EjsString *name)
{
    wchar       *cp;

    for (cp = name->value; *cp; cp++) {
        if (!isdigit((uchar) *cp) || *cp == '.') {
            return 0;
        }
    }
    return 1;
}


/*********************************** Factory **********************************/

PUBLIC EjsXML *ejsCreateXML(Ejs *ejs, int kind, EjsName qname, EjsXML *parent, EjsString *value)
{
    EjsXML      *xml;

    if ((xml = (EjsXML*) ejsAlloc(ejs, ESV(XML), 0)) == NULL) {
        return 0;
    }
    if (qname.name) {
        xml->qname = qname;
    }
    xml->kind = kind;
    xml->parent = parent;
    if (value) {
        xml->value = value;
    }
    return xml;
}


PUBLIC EjsXML *ejsConfigureXML(Ejs *ejs, EjsXML *xml, int kind, EjsString *name, EjsXML *parent, EjsString *value)
{
    xml->qname.name = name;
    xml->kind = kind;
    xml->parent = parent;
    if (value) {
        xml->value = value;
    }
    return xml;
}


/*
    Support routine. Not an class method
 */
PUBLIC void ejsLoadXMLString(Ejs *ejs, EjsXML *xml, EjsString *xmlString)
{
    EjsXmlState *parser;
    MprXml      *xp;

    xp = ejsCreateXmlParser(ejs, xml, "string");
    parser = mprXmlGetParseArg(xp);
    parser->inputBuf = ejsToMulti(ejs, xmlString);
    parser->inputSize = slen(parser->inputBuf);
    mprXmlSetInputStream(xp, readStringData, (void*) 0);

    if (mprXmlParse(xp) < 0 && !ejsHasException(ejs)) {
        ejsThrowSyntaxError(ejs, "Cannot parse XML string: %s", mprXmlGetErrorMsg(xp));
    }
}


PUBLIC void ejsLoadXMLAsc(Ejs *ejs, EjsXML *xml, cchar *xmlString)
{
    ejsLoadXMLString(ejs, xml, ejsCreateStringFromAsc(ejs, xmlString));
}


PUBLIC void ejsManageXML(EjsXML *xml, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(xml->parent);
        mprMark(xml->targetObject);
        mprMark(xml->attributes);
        mprMark(xml->elements);
        mprMark(xml->namespaces);
        mprMark(xml->qname.name);
        mprMark(xml->qname.space);
        mprMark(xml->value);
    }
}


PUBLIC void ejsCreateXMLType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsCreateCoreType(ejs, N("ejs", "XML"), sizeof(EjsXML), S_XML, ES_XML_NUM_CLASS_PROP, ejsManageXML, 
        EJS_TYPE_OBJ);

    /*
        Must not bind as XML uses get/setPropertyByName to defer to user XML elements over XML methods
     */
    type->constructor.block.nobind = 1;

    type->helpers.clone = (EjsCloneHelper) cloneXml;
    type->helpers.cast = (EjsCastHelper) castXml;
    type->helpers.create = (EjsCreateHelper) createXml;
    type->helpers.getPropertyByName = (EjsGetPropertyByNameHelper) getXmlPropertyByName;
    type->helpers.getPropertyCount = (EjsGetPropertyCountHelper) getXmlPropertyCount;
    type->helpers.deletePropertyByName = (EjsDeletePropertyByNameHelper) deleteXmlPropertyByName;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeXmlOperator;
    type->helpers.setPropertyByName = (EjsSetPropertyByNameHelper) setXmlPropertyByName;
}


PUBLIC void ejsConfigureXMLType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "XML"))) == 0) {
        return;
    }
    prototype = type->prototype;
    ejsBindConstructor(ejs, type, xmlConstructor);
    ejsBindMethod(ejs, prototype, ES_XML_length, xmlLength);
    ejsBindMethod(ejs, prototype, ES_XML_load, loadXml);
    ejsBindMethod(ejs, prototype, ES_XML_save, saveXml);
    ejsBindMethod(ejs, prototype, ES_XML_name, getXmlNodeName);
    ejsBindMethod(ejs, prototype, ES_XML_parent, (EjsProc) xml_parent);

    /*
        Override these methods
     */
    ejsBindMethod(ejs, prototype, ES_XML_toString, xmlToString);
    ejsBindMethod(ejs, prototype, ES_XML_toJSON, xmlToJson);
    ejsBindMethod(ejs, prototype, ES_XML_iterator_get, getXmlIterator);
    ejsBindMethod(ejs, prototype, ES_XML_iterator_getValues, getXmlValues);
#if FUTURE
    ejsBindMethod(ejs, prototype, ES_XML_parent, parent);
    ejsBindMethod(ejs, prototype, "valueOf", valueOf, NULL);
#endif
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
