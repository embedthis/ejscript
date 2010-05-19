/**
    ejsArray.c - Ejscript Array class

    This module implents the standard Array type. It provides the type methods and manages the special "length" property.
    The array elements with numeric indicies are stored in EjsArray.data[]. Non-numeric properties are stored in EjsArray.obj

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/***************************** Forward Declarations ***************************/

static int  checkSlot(Ejs *ejs, EjsArray *ap, int slotNum);
static bool compareArrayElement(Ejs *ejs, EjsObj *v1, EjsObj *v2);
static int growArray(Ejs *ejs, EjsArray *ap, int len);
static int lookupArrayProperty(Ejs *ejs, EjsArray *ap, EjsName *qname);
static EjsObj *pushArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv);
static EjsObj *spliceArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv);
static EjsObj *arrayToString(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv);

static EjsObj *makeIntersection(Ejs *ejs, EjsArray *lhs, EjsArray *rhs);
static EjsObj *makeUnion(Ejs *ejs, EjsArray *lhs, EjsArray *rhs);
static EjsObj *removeArrayElements(Ejs *ejs, EjsArray *lhs, EjsArray *rhs);
static EjsObj *setArrayLength(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv);

/******************************************************************************/
/*
    Create a new array
 */
static EjsArray *createArray(Ejs *ejs, EjsType *type, int numSlots)
{
    EjsArray     *ap;

    ap = (EjsArray*) ejsCreateObject(ejs, ejs->arrayType, 0);
    if (ap == 0) {
        return 0;
    }
    ap->length = 0;
#if FUTURE
    /*
        Clear isObject because we must NOT use direct slot access in the VM
     */ 
    ap->obj.isObject = 0;
#endif
    return ap;
}


/*
    Cast the object operand to a primitive type
 */
static EjsObj *castArray(Ejs *ejs, EjsArray *vp, EjsType *type)
{
    switch (type->id) {
    case ES_Boolean:
        return (EjsObj*) ejs->trueValue;

    case ES_Number:
        return (EjsObj*) ejs->zeroValue;

    case ES_String:
        return arrayToString(ejs, vp, 0, 0);

    default:
        ejsThrowTypeError(ejs, "Can't cast to this type");
        return 0;
    }
}


static EjsArray *cloneArray(Ejs *ejs, EjsArray *ap, bool deep)
{
    EjsArray    *newArray;
    EjsObj      **dest, **src;
    int         i;

    newArray = (EjsArray*) ejsCloneObject(ejs, (EjsObj*) ap, deep);
    if (newArray == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    if (ap->length > 0) {
        if (growArray(ejs, newArray, ap->length) < 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
        src = ap->data;
        dest = newArray->data;

        if (deep) {
            for (i = 0; i < ap->length; i++) {
                dest[i] = ejsClone(ejs, src[i], 1);
            }
        } else {
            memcpy(dest, src, ap->length * sizeof(EjsObj*));
        }
    }
    return newArray;
}


//  TODO - can remove as free var will do this automatically
static void destroyArray(Ejs *ejs, EjsArray *ap)
{
    mprAssert(ap);

    mprFree(ap->data);
    ap->data = 0;
    ejsFreeVar(ejs, (EjsObj*) ap, -1);
}


/*
    Delete a property and update the length
 */
static int deleteArrayProperty(Ejs *ejs, EjsArray *ap, int slot)
{
    if (slot >= ap->length) {
        mprAssert(0);
        return EJS_ERR;
    }
    if (ejsSetProperty(ejs, (EjsObj*) ap, slot, (EjsObj*) ejs->undefinedValue) < 0) {
        return EJS_ERR;
    }
    if ((slot + 1) == ap->length) {
        ap->length--;
    }
    return 0;
}


/*
    Delete an element by name.
 */
static int deleteArrayPropertyByName(Ejs *ejs, EjsArray *ap, EjsName *qname)
{
    if (isdigit((int) qname->name[0])) {
        return deleteArrayProperty(ejs, ap, atoi(qname->name));
    }
    return (ejs->objectType->helpers.deletePropertyByName)(ejs, (EjsObj*) ap, qname);
}


/*
    Return the number of elements in the array
 */
static int getArrayPropertyCount(Ejs *ejs, EjsArray *ap)
{
    return ap->length;
}


/*
    Get an array element. Slot numbers correspond to indicies.
 */
static EjsObj *getArrayProperty(Ejs *ejs, EjsArray *ap, int slotNum)
{
    if (slotNum < 0 || slotNum >= ap->length) {
        return ejs->undefinedValue;
    }
    return ap->data[slotNum];
}


static EjsObj *getArrayPropertyByName(Ejs *ejs, EjsArray *ap, EjsName *qname)
{
    int     slotNum;

    if (isdigit((int) qname->name[0])) { 
        slotNum = atoi(qname->name);
        if (slotNum < 0 || slotNum >= ap->length) {
            return 0;
        }
        return getArrayProperty(ejs, ap, slotNum);
    }

    /* The "length" property is a method getter */
    if (strcmp(qname->name, "length") == 0) {
        return 0;
    }
    slotNum = (ejs->objectType->helpers.lookupProperty)(ejs, (EjsObj*) ap, qname);
    if (slotNum < 0) {
        return 0;
    }
    return (ejs->objectType->helpers.getProperty)(ejs, (EjsObj*) ap, slotNum);
}


/*
    Lookup an array index.
 */
static int lookupArrayProperty(Ejs *ejs, EjsArray *ap, EjsName *qname)
{
    int     index;

    if (qname == 0 || !isdigit((int) qname->name[0])) {
        return EJS_ERR;
    }
    index = atoi(qname->name);
    if (index < ap->length) {
        return index;
    }

    return EJS_ERR;
}


/*
    Cast operands as required for invokeArrayOperator
 */
static EjsObj *coerceArrayOperands(Ejs *ejs, EjsObj *lhs, int opcode,  EjsObj *rhs)
{
    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsInvokeOperator(ejs, arrayToString(ejs, (EjsArray*) lhs, 0, 0), opcode, rhs);

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, (EjsObj*) ejs->zeroValue, opcode, rhs);

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
        if (ejsIsNull(rhs) || ejsIsUndefined(rhs)) {
            return (EjsObj*) ((opcode == EJS_OP_COMPARE_EQ) ? ejs->falseValue: ejs->trueValue);
        } else if (ejsIsNumber(rhs)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIsNumber(rhs)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_STRICTLY_NE:
    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_NULL:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ejs->falseValue;

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %s", opcode, lhs->type->qname.name);
        return ejs->undefinedValue;
    }
    return 0;
}


static EjsObj *invokeArrayOperator(Ejs *ejs, EjsObj *lhs, int opcode,  EjsObj *rhs)
{
    EjsObj      *result;

    if (rhs == 0 || lhs->type != rhs->type) {
        if ((result = coerceArrayOperands(ejs, lhs, opcode, rhs)) != 0) {
            return result;
        }
    }

    switch (opcode) {

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_GE:
        return (EjsObj*) ejsCreateBoolean(ejs, (lhs == rhs));

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
    case EJS_OP_COMPARE_LT: case EJS_OP_COMPARE_GT:
        return (EjsObj*) ejsCreateBoolean(ejs, !(lhs == rhs));

    /*
        Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ejs->falseValue;

    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return (EjsObj*) ejs->oneValue;

    /*
        Binary operators
     */
    case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_REM:
    case EJS_OP_SHR: case EJS_OP_USHR: case EJS_OP_XOR:
        return (EjsObj*) ejs->zeroValue;

#if EXTENSIONS || 1
    /*
        Operator overload
     */
    case EJS_OP_ADD:
        result = (EjsObj*) ejsCreateArray(ejs, 0);
        pushArray(ejs, (EjsArray*) result, 1, (EjsObj**) &lhs);
        pushArray(ejs, (EjsArray*) result, 1, (EjsObj**) &rhs);
        return result;

    case EJS_OP_AND:
        return (EjsObj*) makeIntersection(ejs, (EjsArray*) lhs, (EjsArray*) rhs);

    case EJS_OP_OR:
        return (EjsObj*) makeUnion(ejs, (EjsArray*) lhs, (EjsArray*) rhs);

    case EJS_OP_SHL:
        return pushArray(ejs, (EjsArray*) lhs, 1, &rhs);

    case EJS_OP_SUB:
        return (EjsObj*) removeArrayElements(ejs, (EjsArray*) lhs, (EjsArray*) rhs);
#endif

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %s", opcode, lhs->type->qname.name);
        return 0;
    }

    mprAssert(0);
}


static void markArrayVar(Ejs *ejs, EjsArray *ap)
{
    EjsObj          *vp;
    int             i;

    mprAssert(ejsIsArray(ap));

    ejsMarkObject(ejs, (EjsObj*) ap);
    for (i = ap->length - 1; i >= 0; i--) {
        if ((vp = ap->data[i]) != 0) {
            ejsMark(ejs, vp);
        }
    }
}


/*
    Create or update an array elements. If slotNum is < 0, then create the next free array slot. If slotNum is greater
    than the array length, grow the array.
 */
static int setArrayProperty(Ejs *ejs, EjsArray *ap, int slotNum,  EjsObj *value)
{
    if ((slotNum = checkSlot(ejs, ap, slotNum)) < 0) {
        return EJS_ERR;
    }
    ap->data[slotNum] = value;
    return slotNum;
}


static int setArrayPropertyByName(Ejs *ejs, EjsArray *ap, EjsName *qname, EjsObj *value)
{
    int     slotNum;

    if (!isdigit((int) qname->name[0])) { 
        /* The "length" property is a method getter */
        if (strcmp(qname->name, "length") == 0) {
            setArrayLength(ejs, ap, 1, &value);
            return ES_Array_length;
        }
        slotNum = (ejs->objectType->helpers.lookupProperty)(ejs, (EjsObj*) ap, qname);
        if (slotNum < 0) {
            slotNum = (ejs->objectType->helpers.setProperty)(ejs, (EjsObj*) ap, slotNum, value);
            if (slotNum < 0) {
                return EJS_ERR;
            }
            if ((ejs->objectType->helpers.setPropertyName)(ejs, (EjsObj*) ap, slotNum, qname) < 0) {
                return EJS_ERR;
            }
            return slotNum;

        } else {
            return (ejs->objectType->helpers.setProperty)(ejs, (EjsObj*) ap, slotNum, value);
        }
    }

    if ((slotNum = checkSlot(ejs, ap, atoi(qname->name))) < 0) {
        return EJS_ERR;
    }
    ap->data[slotNum] = value;
    return slotNum;
}


#if EXTENSIONS || 1
static EjsObj *makeIntersection(Ejs *ejs, EjsArray *lhs, EjsArray *rhs)
{
    EjsArray    *result;
    EjsObj      **l, **r, **resultSlots;
    int         i, j, k;

    result = ejsCreateArray(ejs, 0);
    l = lhs->data;
    r = rhs->data;

    for (i = 0; i < lhs->length; i++) {
        for (j = 0; j < rhs->length; j++) {
            if (compareArrayElement(ejs, l[i], r[j])) {
                resultSlots = result->data;
                for (k = 0; k < result->length; k++) {
                    if (compareArrayElement(ejs, l[i], resultSlots[k])) {
                        break;
                    }
                }
                if (result->length == 0 || k == result->length) {
                    setArrayProperty(ejs, result, -1, l[i]);
                }
            }
        }
    }
    return (EjsObj*) result;
}


static int addUnique(Ejs *ejs, EjsArray *ap, EjsObj *element)
{
    int     i;

    for (i = 0; i < ap->length; i++) {
        if (compareArrayElement(ejs, ap->data[i], element)) {
            break;
        }
    }
    if (i == ap->length) {
        if (setArrayProperty(ejs, ap, -1, element) < 0) {
            return EJS_ERR;
        }
    }
    return 0;
}


static EjsObj *makeUnion(Ejs *ejs, EjsArray *lhs, EjsArray *rhs)
{
    EjsArray    *result;
    EjsObj      **l, **r;
    int         i;

    result = ejsCreateArray(ejs, 0);

    l = lhs->data;
    r = rhs->data;

    for (i = 0; i < lhs->length; i++) {
        addUnique(ejs, result, l[i]);
    }
    for (i = 0; i < rhs->length; i++) {
        addUnique(ejs, result, r[i]);
    }
    return (EjsObj*) result;
}


static EjsObj *removeArrayElements(Ejs *ejs, EjsArray *lhs, EjsArray *rhs)
{
    EjsObj  **l, **r;
    int     i, j, k;

    l = lhs->data;
    r = rhs->data;

    for (j = 0; j < rhs->length; j++) {
        for (i = 0; i < lhs->length; i++) {
            if (compareArrayElement(ejs, l[i], r[j])) {
                for (k = i + 1; k < lhs->length; k++) {
                    l[k - 1] = l[k];
                }
                lhs->length--;
            }
        }
    }
    return (EjsObj*) lhs;
}
#endif


static int checkSlot(Ejs *ejs, EjsArray *ap, int slotNum)
{
    if (slotNum < 0) {
        if (!ap->obj.dynamic) {
            ejsThrowTypeError(ejs, "Object is not dynamic");
            return EJS_ERR;
        }
        slotNum = ap->length;
        if (growArray(ejs, ap, ap->length + 1) < 0) {
            ejsThrowMemoryError(ejs);
            return EJS_ERR;
        }

    } else if (slotNum >= ap->length) {
        if (growArray(ejs, ap, slotNum + 1) < 0) {
            ejsThrowMemoryError(ejs);
            return EJS_ERR;
        }
    }
    return slotNum;
}


/********************************** Methods *********************************/
/*
    Array constructor.

    function Array(...args): Array

    Support the forms:

        var arr = Array();
        var arr = Array(size);
        var arr = Array(elt, elt, elt, ...);
 */

static EjsObj *arrayConstructor(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsArray    *args;
    EjsObj      *arg0, **src, **dest;
    int         size, i;

    mprAssert(argc == 1 && ejsIsArray(argv[0]));

    args = (EjsArray*) argv[0];
    if (args->length == 0) {
        return 0;
    }
    size = 0;
    arg0 = getArrayProperty(ejs, args, 0);

    if (args->length == 1 && ejsIsNumber(arg0)) {
        /*
            x = new Array(size);
         */
        size = ejsGetInt(ejs, arg0);
        if (size > 0 && growArray(ejs, ap, size) < 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }

    } else {

        /*
            x = new Array(element0, element1, ..., elementN):
         */
        size = args->length;
        if (size > 0 && growArray(ejs, ap, size) < 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }

        src = args->data;
        dest = ap->data;
        for (i = 0; i < size; i++) {
            dest[i] = src[i];
        }
    }
    ap->length = size;
    return (EjsObj*) ap;
}


/*
    Append an item to an array

    function append(obj: Object) : Array
 */
static EjsObj *appendArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    if (setArrayProperty(ejs, ap, ap->length, argv[0]) < 0) {
        return 0;
    }
    return (EjsObj*) ap;
}


/*
    Clear an array. Remove all elements of the array.

    function clear() : void
 */
static EjsObj *clearArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    ap->length = 0;
    return 0;
}


/*
    Clone an array.

    function clone(deep: Boolean = false) : Array
 */
static EjsArray *cloneArrayMethod(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    bool    deep;

    mprAssert(argc == 0 || ejsIsBoolean(argv[0]));

    deep = (argc == 1) ? ((EjsBoolean*) argv[0])->value : 0;

    return cloneArray(ejs, ap, deep);
}


/*
    Compact an array. Remove all null elements.

    function compact() : Array
 */
static EjsArray *compactArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsObj      **data, **src, **dest;
    int         i;

    data = ap->data;
    src = dest = &data[0];
    for (i = 0; i < ap->length; i++, src++) {
        if (*src == 0 || *src == ejs->undefinedValue || *src == ejs->nullValue) {
            continue;
        }
        *dest++ = *src;
    }
    ap->length = (int) (dest - &data[0]);
    return ap;
}


/*
    Concatenate the supplied elements with the array to create a new array. If any arguments specify an array,
    their elements are catenated. This is a one level deep copy.

    function concat(...args): Array
 */
static EjsObj *concatArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsArray    *args, *newArray, *vpa;
    EjsObj      *vp, **src, **dest;
    int         i, k, next;

    mprAssert(argc == 1 && ejsIsArray(argv[0]));

    args = ((EjsArray*) argv[0]);

    /*
        Guess the new array size. May exceed this if args has elements that are themselves arrays.
     */
    newArray = ejsCreateArray(ejs, ap->length + ((EjsArray*) argv[0])->length);

    src = ap->data;
    dest = newArray->data;

    /*
        Copy the original array
     */
    for (next = 0; next < ap->length; next++) {
        dest[next] = src[next];
    }

    /*
        Copy the args. If any element is itself an array, then flatten it and copy its elements.
     */
    for (i = 0; i < args->length; i++) {
        vp = args->data[i];
        if (ejsIsArray(vp)) {
            vpa = (EjsArray*) vp;
            if (growArray(ejs, newArray, newArray->length + vpa->length - 1) < 0) {
                ejsThrowMemoryError(ejs);
                return 0;
            }
            dest = newArray->data;
            for (k = 0; k < vpa->length; k++) {
                dest[next++] = vpa->data[k];
            }

        } else {
            dest[next++] = vp;
        }
    }
    return (EjsObj*) newArray;
}


/*
    Function to iterate and return the next element name.
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsObj *nextArrayKey(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsArray        *ap;
    EjsObj          *vp, **data;

    ap = (EjsArray*) ip->target;
    if (!ejsIsArray(ap)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }
    data = ap->data;

    for (; ip->index < ap->length; ip->index++) {
        vp = data[ip->index];
        if (vp == 0) {
            continue;
        }
        return (EjsObj*) ejsCreateNumber(ejs, ip->index++);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return the default iterator. This returns the array index names.

    iterator native function get(): Iterator
 */
static EjsObj *getArrayIterator(Ejs *ejs, EjsObj *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, ap, (EjsProc) nextArrayKey, 0, NULL);
}


/*
    Function to iterate and return the next element value.
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsObj *nextArrayValue(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsArray    *ap;
    EjsObj      *vp, **data;

    ap = (EjsArray*) ip->target;
    if (!ejsIsArray(ap)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }

    data = ap->data;
    for (; ip->index < ap->length; ip->index++) {
        vp = data[ip->index];
        if (vp == 0) {
            continue;
        }
        ip->index++;
        return vp;
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return an iterator to return the next array element value.

    iterator native function getValues(): Iterator
 */
static EjsObj *getArrayValues(Ejs *ejs, EjsObj *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, ap, (EjsProc) nextArrayValue, 0, NULL);
}


#if KEEP
static EjsObj *find(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    return 0;
}


/**
    Iterate over all elements in the object and find all elements for which the matching function is true.
    The match is called with the following signature:

        function match(arrayElement: Object, elementIndex: Number, arr: Array): Boolean

    @param match Matching function
    @return Returns a new array containing all matching elements.
 */
static EjsObj *findAll(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsObj      *funArgs[3];
    EjsBoolean  *result;
    EjsArray    *elements;
    int         i;

    mprAssert(argc == 1 && ejsIsFunction(argv[0]));

    elements = ejsCreateArray(ejs, 0);
    if (elements == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }

    for (i = 0; i < ap->length; i++) {
        funArgs[0] = ap->obj.properties.slots[i];               /* Array element */
        funArgs[1] = (EjsObj*) ejsCreateNumber(ejs, i);             /* element index */
        funArgs[2] = (EjsObj*) ap;                                  /* Array */
        result = (EjsBoolean*) ejsRunFunction(ejs, (EjsFunction*) argv[0], 0, 3, funArgs);
        if (result == 0 || !ejsIsBoolean(result) || !result->value) {
            setArrayProperty(ejs, elements, elements->length, ap->obj.properties.slots[i]);
        }
    }
    return (EjsObj*) elements;
}
#endif


static bool compareArrayElement(Ejs *ejs, EjsObj *v1, EjsObj *v2)
{
    if (v1 == v2) {
        return 1;
    }
    if (v1->type != v2->type) {
        return 0;
    }
    if (ejsIsNumber(v1)) {
        return ((EjsNumber*) v1)->value == ((EjsNumber*) v2)->value;
    }
    if (ejsIsString(v1)) {
        return strcmp(((EjsString*) v1)->value, ((EjsString*) v2)->value) == 0;
    }
    return 0;
}


/*
    Search for an item using strict equality "===". This call searches from
    the start of the array for the specified element.
    @return Returns the items index into the array if found, otherwise -1.

    function indexOf(element: Object, startIndex: Number = 0): Number
 */
static EjsObj *indexOfArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsObj      *element;
    int         i, start;

    mprAssert(argc == 1 || argc == 2);

    element = argv[0];
    start = (argc == 2) ? (int) ((EjsNumber*) argv[1])->value : 0;

    if (start < 0) {
        start += ap->length;
    }
    if (start >= ap->length) {
        return (EjsObj*) ejs->minusOneValue;
    }
    if (start < 0) {
        start = 0;
    }
    for (i = start; i < ap->length; i++) {
        if (compareArrayElement(ejs, ap->data[i], element)) {
            return (EjsObj*) ejsCreateNumber(ejs, i);
        }
    }
    return (EjsObj*) ejs->minusOneValue;
}


/*
    Insert elements. Insert elements at the specified position. Negative indicies are measured from the end of the array.
    @return Returns a the original array.

    function insert(pos: Number, ...args): Array
 */
static EjsObj *insertArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsArray    *args;
    EjsObj      **src, **dest;
    int         i, pos, delta, oldLen, endInsert;

    mprAssert(argc == 2 && ejsIsArray(argv[1]));

    pos = ejsGetInt(ejs, argv[0]);
    if (pos < 0) {
        pos += ap->length;
    }
    if (pos < 0) {
        pos = 0;
    }
    if (pos >= ap->length) {
        pos = ap->length;
    }
    args = (EjsArray*) argv[1];

    oldLen = ap->length;
    if (growArray(ejs, ap, ap->length + args->length) < 0) {
        return 0;
    }

    delta = args->length;
    dest = ap->data;
    src = args->data;

    endInsert = pos + delta;
    for (i = ap->length - 1; i >= endInsert; i--) {
        dest[i] = dest[i - delta];
    }
    for (i = 0; i < delta; i++) {
        dest[pos++] = src[i];
    }

    return (EjsObj*) ap;
}


/*
    Joins the elements in the array into a single string.
    @param sep Element separator.
    @return Returns a string.

    function join(sep: String = undefined): String
 */
static EjsObj *joinArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsString       *result, *sep;
    EjsObj          *vp;
    int             i;

    if (argc == 1) {
        sep = (EjsString*) argv[0];
    } else {
        sep = 0;
    }

    result = ejsCreateString(ejs, "");
    for (i = 0; i < ap->length; i++) {
        vp = ap->data[i];
        if (vp == 0 || ejsIsUndefined(vp) || ejsIsNull(vp)) {
            continue;
        }
        if (i > 0 && sep) {
            ejsStrcat(ejs, result, (EjsObj*) sep);
        }
        ejsStrcat(ejs, result, vp);
    }
    return (EjsObj*) result;
}


/*
    Search for an item using strict equality "===". This call searches from
    the end of the array for the specified element.
    @return Returns the items index into the array if found, otherwise -1.

    function lastIndexOf(element: Object, fromIndex: Number = 0): Number
 */
static EjsObj *lastArrayIndexOf(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsObj          *element;
    int             i, start;

    mprAssert(argc == 1 || argc == 2);

    element = argv[0];
    start = ((argc == 2) ? (int) ((EjsNumber*) argv[1])->value : ap->length - 1);
    if (start < 0) {
        start += ap->length;
    }
    if (start >= ap->length) {
        start = ap->length - 1;
    }
    if (start < 0) {
        return (EjsObj*) ejs->minusOneValue;
    }
    for (i = start; i >= 0; i--) {
        if (compareArrayElement(ejs, ap->data[i], element)) {
            return (EjsObj*) ejsCreateNumber(ejs, i);
        }
    }
    return (EjsObj*) ejs->minusOneValue;
}


/*
    Get the length of an array.
    @return Returns the number of items in the array

    override function get length(): Number
 */
static EjsObj *getArrayLength(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, ap->length);
}


/*
    Set the length of an array.

    override function set length(value: Number): void
 */
static EjsObj *setArrayLength(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsObj      **data, **dest;
    int         length;

    mprAssert(argc == 1 && ejsIsNumber(argv[0]));
    mprAssert(ejsIsArray(ap));

    length = (int) ((EjsNumber*) argv[0])->value;
    if (length < 0) {
        length = 0;
    }
    if (length > ap->length) {
        if (growArray(ejs, ap, length) < 0) {
            return 0;
        }
        data = ap->data;
        for (dest = &data[ap->length]; dest < &data[length]; dest++) {
            *dest = 0;
        }
    }
    ap->length = length;
    return 0;
}


/*
    Remove and return the last value in the array.
    @return Returns the last element in the array.

    function pop(): Object
 */
static EjsObj *popArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    if (ap->length == 0) {
        return (EjsObj*) ejs->undefinedValue;
    }
    return ap->data[--ap->length];
}


/*
    Append items to the end of the array.
    @return Returns the new length of the array.

    function push(...items): Number
 */
static EjsObj *pushArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsArray    *args;
    EjsObj      **src, **dest;
    int         i, oldLen;

    mprAssert(argc == 1 && ejsIsArray(argv[0]));

    args = (EjsArray*) argv[0];

    oldLen = ap->length;
    if (growArray(ejs, ap, ap->length + args->length) < 0) {
        return 0;
    }
    dest = ap->data;
    src = args->data;
    for (i = 0; i < args->length; i++) {
        dest[i + oldLen] = src[i];
    }
    return (EjsObj*) ejsCreateNumber(ejs, ap->length);
}


/*
    Reverse the order of the objects in the array. The elements are reversed in the original array.
    @return Returns a reference to the array.

    function reverse(): Array
 */
static EjsObj *reverseArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsObj  *tmp, **data;
    int     i, j;

    if (ap->length <= 1) {
        return (EjsObj*) ap;
    }

    data = ap->data;
    i = (ap->length - 2) / 2;
    j = (ap->length + 1) / 2;

    for (; i >= 0; i--, j++) {
        tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
    }
    return (EjsObj*) ap;
}


/*
    Remove and return the first value in the array.
    @return Returns the first element in the array.

    function shift(): Object
 */
static EjsObj *shiftArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsObj      *result, **data;
    int         i;

    if (ap->length == 0) {
        return ejs->undefinedValue;
    }
    data = ap->data;
    result = data[0];
    for (i = 1; i < ap->length; i++) {
        data[i - 1] = data[i];
    }
    ap->length--;
    return result;
}


/*
    Create a new array by taking a slice from an array.

    function slice(start: Number, end: Number, step: Number = 1): Array
 */
static EjsObj *sliceArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsArray    *result;
    EjsObj      **src, **dest;
    int         start, end, step, i, j, len;

    mprAssert(1 <= argc && argc <= 3);

    start = ejsGetInt(ejs, argv[0]);
    if (argc >= 2) {
        end = ejsGetInt(ejs, argv[1]);
    } else {
        end = ap->length;
    }
    if (argc == 3) {
        step = ejsGetInt(ejs, argv[2]);
    } else {
        step = 1;
    }
    if (step == 0) {
        step = 1;
    }

    if (start < 0) {
        start += ap->length;
    }
    if (start < 0) {
        start = 0;
    } else if (start >= ap->length) {
        start = ap->length;
    }

    if (end < 0) {
        end += ap->length;
    }
    if (end < 0) {
        end = 0;
    } else if (end >= ap->length) {
        end = ap->length;
    }

    /*
        This may allocate too many elements if step is > 0, but length will still be correct.
     */
    result = ejsCreateArray(ejs, end - start);
    if (result == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    src = ap->data;
    dest = result->data;

    len = 0;
    if (step > 0) {
        for (i = start, j = 0; i < end; i += step, j++) {
            dest[j] = src[i];
            len++;
        }

    } else {
        for (i = start, j = 0; i > end; i += step, j++) {
            dest[j] = src[i];
            len++;
        }
    }
    result->length = len;
    return (EjsObj*) result;
}


/*
    Quick sort partition
 */
static int partition(Ejs *ejs, EjsArray *array, EjsFunction *compare, int direction, int p, int r)
{
    EjsObj      *tmp, *x, *argv[3];
    EjsString   *sx, *so;
    EjsNumber   *result;
    int         i, j, order;

    x = array->data[r];
    sx = 0;

    if (compare) {
        if ((argv[1] = (EjsObj*) ejsCreateNumber(ejs, r)) == 0) {
            return 0;
        }
    } else {
        if ((sx = ejsToString(ejs, x)) == 0) {
            return 0;
        }
    }
    j = p - 1;

    for (i = p; i < r; i++) {
        if (compare) {
            argv[0] = (EjsObj*) array;
            argv[2] = (EjsObj*) ejsCreateNumber(ejs, i);
            result = (EjsNumber*) ejsRunFunction(ejs, compare, NULL, 3, argv);
            if (!ejsIsNumber(result)) {
                return 0;
            }
            order = ejsGetInt(ejs, result);

        } else {
            if ((so = ejsToString(ejs, array->data[i])) == 0) {
                return 0;
            }
            order = strcmp(sx->value, so->value);
        }
        order *= direction;
        if (order > 0) {
            j = j + 1;
            tmp = array->data[j];
            array->data[j] = array->data[i];
            array->data[i] = tmp;
        }
    }
    array->data[r] = array->data[j + 1];
    array->data[j + 1] = x;
    return j + 1;
}


void quickSort(Ejs *ejs, EjsArray *ap, EjsFunction *compare, int direction, int p, int r)
{
    int     q;

    if (p < r && !ejs->exception) {
        q = partition(ejs, ap, compare, direction, p, r);
        quickSort(ejs, ap, compare, direction, p, q - 1);
        quickSort(ejs, ap, compare, direction, q + 1, r);
    }
}


/**
    Sort the array using the supplied compare function

    function sort(compare: Function = null, direction: Number = 1): Array

    Where compare is defined as:
        function compare(a,b): Number
 */
static EjsObj *sortArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsFunction     *compare;
    int             direction;

    if (ap->length <= 1) {
        return (EjsObj*) ap;
    }
    compare = (EjsFunction*) ((argc == 1) ? argv[0]: NULL);
    if ((EjsObj*) compare == ejs->nullValue) {
        compare = 0;
    }
    if (compare && !ejsIsFunction(compare)) {
        ejsThrowArgError(ejs, "Compare argument is not a function");
        return 0;
    }
    direction = (argc == 2) ? ejsGetInt(ejs, argv[1]) : 1;
    quickSort(ejs, ap, compare, direction, 0, ap->length - 1);
    return (EjsObj*) ap;
}


/*
    Insert, remove or replace array elements. Return the removed elements.

    function splice(start: Number, deleteCount: Number, ...values): Array

 */
static EjsObj *spliceArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsArray    *result, *values;
    EjsObj      **data, **dest, **items;
    int         start, deleteCount, i, delta, endInsert, oldLen;

    mprAssert(1 <= argc && argc <= 3);
    
    start = ejsGetInt(ejs, argv[0]);
    deleteCount = ejsGetInt(ejs, argv[1]);
    values = (EjsArray*) argv[2];

    if (ap->length == 0) {
        if (deleteCount <= 0) {
            return (EjsObj*) ap;
        }
        ejsThrowArgError(ejs, "Array is empty");
        return 0;
    }
    if (start < 0) {
        start += ap->length;
    }
    if (start < 0) {
        start = 0;
    }
    if (start >= ap->length) {
        start = ap->length - 1;
    }

    if (deleteCount < 0) {
        deleteCount = ap->length - start + 1;
    }
    if (deleteCount > ap->length) {
        deleteCount = ap->length;
    }

    result = ejsCreateArray(ejs, deleteCount);
    if (result == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }

    data = ap->data;
    dest = result->data;
    items = values->data;

    /*
        Copy removed items to the result
     */
    for (i = 0; i < deleteCount; i++) {
        dest[i] = data[i + start];
    }

    oldLen = ap->length;
    delta = values->length - deleteCount;
    
    if (delta > 0) {
        /*
            Make room for items to insert
         */
        if (growArray(ejs, ap, ap->length + delta) < 0) {
            return 0;
        }
        data = ap->data;
        endInsert = start + delta;
        for (i = ap->length - 1; i >= endInsert; i--) {
            data[i] = data[i - delta];
        }
        
    } else {
        ap->length += delta;
    }

    /*
        Copy in new values
     */
    for (i = 0; i < values->length; i++) {
        data[start + i] = items[i];
    }

    /*
        Remove holes
     */
    if (delta < 0) {
        for (i = start + values->length; i < oldLen; i++) {
            data[i] = data[i - delta];
        }
    }
    return (EjsObj*) result;
}


#if ES_Object_toLocaleString && FUTURE
/*
    Convert the array to a single localized string each member of the array
    has toString called on it and the resulting strings are concatenated.
    Currently just calls toString.

    function toLocaleString(): String
 */
static EjsObj *toLocaleString(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    return arrayToString(ejs, ap, argc, argv);
}
#endif


/*
    Convert the array to a single string each member of the array has toString called on it and the resulting strings 
    are concatenated.

    override function toString(): String
 */
static EjsObj *arrayToString(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsString       *result;
    EjsObj          *vp;
    int             i, rc;

    result = ejsCreateString(ejs, "");
    if (result == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }

    for (i = 0; i < ap->length; i++) {
        vp = ap->data[i];
        rc = 0;
        if (i > 0) {
            rc = ejsStrcat(ejs, result, (EjsObj*) ejsCreateString(ejs, ","));
        }
        if (vp != 0 && vp != ejs->undefinedValue && vp != ejs->nullValue) {
            rc = ejsStrcat(ejs, result, vp);
        }
        if (rc < 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
    }
    return (EjsObj*) result;
}


/*
    Return an array with duplicate elements removed where duplicates are detected by using "==" (ie. content equality, 
    not strict equality).

    function unique(): Array
 */
static EjsObj *uniqueArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv)
{
    EjsObj      **data;
    int         i, j, k;

    data = ap->data;

    for (i = 0; i < ap->length; i++) {
        for (j = i + 1; j < ap->length; j++) {
            if (compareArrayElement(ejs, data[i], data[j])) {
                for (k = j + 1; k < ap->length; k++) {
                    data[k - 1] = data[k];
                }
                ap->length--;
                j--;
            }
        }
    }
    return (EjsObj*) ap;
}


/*
    function unshift(...args): Array
 */
static EjsVar *unshiftArray(Ejs *ejs, EjsArray *ap, int argc, EjsVar **argv)
{
    EjsArray    *args;
    EjsObj      **src, **dest;
    int         i, delta, oldLen, endInsert;

    mprAssert(argc == 1 && ejsIsArray(argv[0]));

    args = (EjsArray*) argv[0];
    if (args->length <= 0) {
        return (EjsObj*) ap;
    }
    oldLen = ap->length;
    if (growArray(ejs, ap, ap->length + args->length) < 0) {
        return 0;
    }
    delta = args->length;
    dest = ap->data;
    src = args->data;

    endInsert = delta;
    for (i = ap->length - 1; i >= endInsert; i--) {
        dest[i] = dest[i - delta];
    }
    for (i = 0; i < delta; i++) {
        dest[i] = src[i];
    }
    return (EjsObj*) ap;
}

/*********************************** Support **********************************/

static int growArray(Ejs *ejs, EjsArray *ap, int len)
{
    EjsObj      **dp;
    int         i, size, count, factor;

    mprAssert(ap);

    if (len <= 0) {
        return 0;
    }
    if (len <= ap->length) {
        return EJS_ERR;
    }
    size = mprGetBlockSize(ap->data) / sizeof(EjsObj*);

    /*
        Allocate or grow the data structures.
     */
    if (len > size) {
        if (size > EJS_LOTSA_PROP) {
            /*
                Looks like a big object so grow by a bigger chunk
             */
            factor = max(size / 4, EJS_ROUND_PROP);
            count = (len + factor) / factor * factor;
        } else {
            count = len;
        }
        count = EJS_PROP_ROUNDUP(count);
        if (ap->data == 0) {
            mprAssert(ap->length == 0);
            mprAssert(count > 0);
            ap->data = (EjsObj**) mprAllocZeroed(ap, sizeof(EjsObj*) * count);
            if (ap->data == 0) {
                return EJS_ERR;
            }
        } else {
            mprAssert(size > 0);
            ap->data = (EjsObj**) mprRealloc(ap, ap->data, sizeof(EjsObj*) * count);
            if (ap->data == 0) {
                return EJS_ERR;
            }
            dp = &ap->data[ap->length];
            for (i = ap->length; i < count; i++) {
                *dp++ = ejs->nullValue;
            }
        }
    }
    ap->length = len;
    return 0;
}


/*********************************** Factory **********************************/

EjsArray *ejsCreateArray(Ejs *ejs, int size)
{
    EjsArray    *ap;

    /*
        No need to invoke constructor
     */
    ap = (EjsArray*) ejsCreateObject(ejs, ejs->arrayType, 0);
    if (ap != 0) {
        ap->length = 0;
        if (size > 0 && growArray(ejs, ap, size) < 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
    }
    ejsSetDebugName(ap, "array instance");
    return ap;
}


void ejsCreateArrayType(Ejs *ejs)
{
    EjsType         *type;
    EjsTypeHelpers  *helpers;

    type = ejs->arrayType = ejsCreateNativeType(ejs, "ejs", "Array", ES_Array, sizeof(EjsArray));
    type->numericIndicies = 1;
    type->virtualSlots = 1;

    helpers = &type->helpers;
    helpers->cast = (EjsCastHelper) castArray;
    helpers->clone = (EjsCloneHelper) cloneArray;
    helpers->create = (EjsCreateHelper) createArray;
    helpers->destroy = (EjsDestroyHelper) destroyArray;
    helpers->getProperty = (EjsGetPropertyHelper) getArrayProperty;
    helpers->getPropertyCount = (EjsGetPropertyCountHelper) getArrayPropertyCount;
    helpers->getPropertyByName = (EjsGetPropertyByNameHelper) getArrayPropertyByName;
    helpers->deleteProperty = (EjsDeletePropertyHelper) deleteArrayProperty;
    helpers->deletePropertyByName = (EjsDeletePropertyByNameHelper) deleteArrayPropertyByName;
    helpers->invokeOperator = (EjsInvokeOperatorHelper) invokeArrayOperator;
    helpers->lookupProperty = (EjsLookupPropertyHelper) lookupArrayProperty;
    helpers->mark = (EjsMarkHelper) markArrayVar;
    helpers->setProperty = (EjsSetPropertyHelper) setArrayProperty;
    helpers->setPropertyByName = (EjsSetPropertyByNameHelper) setArrayPropertyByName;
}


void ejsConfigureArrayType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *prototype;

    type = ejsGetTypeByName(ejs, "ejs", "Array");
    prototype = type->prototype;

    /*
        We override some Object methods
     */
    ejsBindConstructor(ejs, type, (EjsProc) arrayConstructor);
    ejsBindMethod(ejs, prototype, ES_Array_iterator_get, getArrayIterator);
    ejsBindMethod(ejs, prototype, ES_Array_iterator_getValues, getArrayValues);
    ejsBindMethod(ejs, prototype, ES_Array_clone, (EjsProc) cloneArrayMethod);
    ejsBindMethod(ejs, prototype, ES_Array_toString, (EjsProc) arrayToString);
    ejsBindMethod(ejs, prototype, ES_Array_append, (EjsProc) appendArray);
    ejsBindMethod(ejs, prototype, ES_Array_clear, (EjsProc) clearArray);
    ejsBindMethod(ejs, prototype, ES_Array_compact, (EjsProc) compactArray);
    ejsBindMethod(ejs, prototype, ES_Array_concat, (EjsProc) concatArray);
    ejsBindMethod(ejs, prototype, ES_Array_indexOf, (EjsProc) indexOfArray);
    ejsBindMethod(ejs, prototype, ES_Array_insert, (EjsProc) insertArray);
    ejsBindMethod(ejs, prototype, ES_Array_join, (EjsProc) joinArray);
    ejsBindMethod(ejs, prototype, ES_Array_lastIndexOf, (EjsProc) lastArrayIndexOf);
    ejsBindAccess(ejs, prototype, ES_Array_length, (EjsProc) getArrayLength, (EjsProc) setArrayLength);
    ejsBindMethod(ejs, prototype, ES_Array_pop, (EjsProc) popArray);
    ejsBindMethod(ejs, prototype, ES_Array_push, (EjsProc) pushArray);
    ejsBindMethod(ejs, prototype, ES_Array_reverse, (EjsProc) reverseArray);
    ejsBindMethod(ejs, prototype, ES_Array_shift, (EjsProc) shiftArray);
    ejsBindMethod(ejs, prototype, ES_Array_slice, (EjsProc) sliceArray);
    ejsBindMethod(ejs, prototype, ES_Array_sort, (EjsProc) sortArray);
    ejsBindMethod(ejs, prototype, ES_Array_splice, (EjsProc) spliceArray);
    ejsBindMethod(ejs, prototype, ES_Array_unique, (EjsProc) uniqueArray);
    ejsBindMethod(ejs, prototype, ES_Array_unshift, (EjsProc) unshiftArray);

#if FUTURE
    ejsBindMethod(ejs, prototype, ES_Array_toLocaleString, toLocaleString);
    ejsBindMethod(ejs, prototype, ES_Array_toJSONString, toJSONString);
    ejsBindMethod(ejs, prototype, ES_Array_LBRACKET, operLBRACKET);
    ejsBindMethod(ejs, prototype, ES_Array_AND, operAND);
    ejsBindMethod(ejs, prototype, ES_Array_EQ, operEQ);
    ejsBindMethod(ejs, prototype, ES_Array_GT, operGT);
    ejsBindMethod(ejs, prototype, ES_Array_LT, operLT);
    ejsBindMethod(ejs, prototype, ES_Array_LSH, operLSH);
    ejsBindMethod(ejs, prototype, ES_Array_MINUS, operMINUS);
    ejsBindMethod(ejs, prototype, ES_Array_OR, operOR);
    ejsBindMethod(ejs, prototype, ES_Array_AND, operAND);
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
