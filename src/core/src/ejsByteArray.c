/*
    ejsByteArray.c - Ejscript ByteArray class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/***************************** Forward Declarations ***************************/

static EjsObj *ba_flush(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv);
static EjsObj *ba_toString(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv);

static int  flushByteArray(Ejs *ejs, EjsByteArray *ap);
static int  getInput(Ejs *ejs, EjsByteArray *ap, int required);
static int  lookupByteArrayProperty(Ejs *ejs, EjsByteArray *ap, EjsName *qname);
 static bool makeRoom(Ejs *ejs, EjsByteArray *ap, int require);

static MPR_INLINE int swap16(EjsByteArray *ap, int a);
static MPR_INLINE int swap32(EjsByteArray *ap, int a);
static MPR_INLINE int64 swap64(EjsByteArray *ap, int64 a);
static MPR_INLINE double swapDouble(EjsByteArray *ap, double a);
static int putByte(EjsByteArray *ap, int value);
static int putInteger(EjsByteArray *ap, int value);
static int putLong(EjsByteArray *ap, int64 value);
static int putShort(EjsByteArray *ap, int value);
static int putString(EjsByteArray *ap, cchar *value, int len);
static int putNumber(EjsByteArray *ap, MprNumber value);
static int putDouble(EjsByteArray *ap, double value);

#define availableBytes(ap)  (((EjsByteArray*) ap)->writePosition - ((EjsByteArray*) ap)->readPosition)
#define room(ap) (ap->length - ap->writePosition)
#define adjustReadPosition(ap, amt) \
    if (1) { \
        ap->readPosition += amt; \
        if (ap->readPosition == ap->writePosition) {    \
            ap->readPosition = ap->writePosition = 0; \
        } \
    } else

/******************************************************************************/
/*
    Cast the object operand to a primitive type
 */
static EjsObj *castByteArrayVar(Ejs *ejs, EjsByteArray *vp, EjsType *type)
{
    switch (type->id) {
    case ES_Boolean:
        return (EjsObj*) ejs->trueValue;

    case ES_Number:
        return (EjsObj*) ejs->zeroValue;

    case ES_String:
        return ba_toString(ejs, vp, 0, 0);

    default:
        ejsThrowTypeError(ejs, "Can't cast to this type");
        return 0;
    }
}


static EjsByteArray *cloneByteArrayVar(Ejs *ejs, EjsByteArray *ap, bool deep)
{
    EjsByteArray    *newArray;
    int             i;

    newArray = ejsCreateByteArray(ejs, ap->length);
    if (newArray == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    for (i = 0; i < ap->length; i++) {
        newArray->value[i] = ap->value[i];
    }
    newArray->readPosition = ap->readPosition;
    newArray->writePosition = ap->writePosition;
    return newArray;
}


static int deleteByteArrayProperty(struct Ejs *ejs, EjsByteArray *ap, int slot)
{
    if (slot >= ap->length) {
        ejsThrowOutOfBoundsError(ejs, "Bad subscript");
        return EJS_ERR;
    }
    if ((slot + 1) == ap->length) {
        ap->length--;
        if (ap->readPosition >= ap->length) {
            ap->readPosition = ap->length - 1;
        }
        if (ap->writePosition >= ap->length) {
            ap->writePosition = ap->length - 1;
        }
    }
    if (ejsSetProperty(ejs, ap, slot, ejs->undefinedValue) < 0) {
        return EJS_ERR;
    }
    return 0;
}


static int getByteArrayPropertyCount(Ejs *ejs, EjsByteArray *ap)
{
    return ap->length;
}


static EjsObj *getByteArrayProperty(Ejs *ejs, EjsByteArray *ap, int slotNum)
{
    if (slotNum < 0 || slotNum >= ap->length) {
        ejsThrowOutOfBoundsError(ejs, "Bad array subscript");
        return 0;
    }
    return (EjsObj*) ejsCreateNumber(ejs, ap->value[slotNum]);
}


static int lookupByteArrayProperty(struct Ejs *ejs, EjsByteArray *ap, EjsName *qname)
{
    int     index;

    if (qname == 0 || ! isdigit((int) qname->name[0])) {
        return EJS_ERR;
    }
    index = atoi(qname->name);
    if (index < ap->length) {
        return index;
    }
    return EJS_ERR;
}


/*
    Cast operands as required for invokeOperator
 */
static EjsObj *coerceByteArrayOperands(Ejs *ejs, EjsObj *lhs, int opcode,  EjsObj *rhs)
{
    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsInvokeOperator(ejs, ba_toString(ejs, (EjsByteArray*) lhs, 0, 0), opcode, rhs);

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


static EjsObj *invokeByteArrayOperator(Ejs *ejs, EjsObj *lhs, int opcode,  EjsObj *rhs)
{
    EjsObj      *result;

    if (rhs == 0 || lhs->type != rhs->type) {
        if ((result = coerceByteArrayOperands(ejs, lhs, opcode, rhs)) != 0) {
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

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %s", opcode, lhs->type->qname.name);
        return 0;
    }
    mprAssert(0);
}


static void markByteArrayVar(Ejs *ejs, EjsByteArray *ap)
{
    mprAssert(ejsIsByteArray(ap));

    //  MOB -- not needed
    ejsMarkObject(ejs, (EjsObj*) ap);
    if (ap->emitter) {
        ejsMark(ejs, (EjsObj*) ap->emitter);
    }
    if (ap->listeners) {
        ejsMark(ejs, (EjsObj*) ap->listeners);
    }
}


/*
    Create or update an array elements. If slotNum is < 0, then create the next free array slot. If slotNum is greater
    than the array length, grow the array.
 */
static int setByteArrayProperty(struct Ejs *ejs, EjsByteArray *ap, int slotNum,  EjsObj *value)
{
    if (slotNum >= ap->length) {
        if (ejsGrowByteArray(ejs, ap, slotNum + 1) < 0) {
            return EJS_ERR;
        }
    }
    if (ejsIsNumber(value)) {
        ap->value[slotNum] = ejsGetInt(ejs, value);
    } else {
        ap->value[slotNum] = ejsGetInt(ejs, ejsToNumber(ejs, value));
    }
    if (slotNum >= ap->length) {
        ap->length = slotNum + 1;
    }
    return slotNum;
}


/********************************** Methods *********************************/
/*
    function ByteArray(size: Number = -1, growable: Boolean = true): ByteArray
 */
static EjsObj *ba_ByteArray(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    bool    growable;
    int     size;

    mprAssert(0 <= argc && argc <= 2);

    size = (argc >= 1) ? ejsGetInt(ejs, argv[0]) : MPR_BUFSIZE;
    if (size <= 0) {
        size = 1;
    }
    growable = (argc == 2) ? ejsGetBoolean(ejs, argv[1]): 1;

    if (ejsGrowByteArray(ejs, ap, size) < 0) {
        return 0;
    }
    mprAssert(ap->value);
    ap->growable = growable;
    ap->growInc = MPR_BUFSIZE;
    ap->length = size;
    ap->endian = mprGetEndian(ejs);
    return (EjsObj*) ap;
}


/**
    function get observe(name, listener: Function): Void
 */
static EjsObj *ba_observe(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    ejsAddListener(ejs, &ap->emitter, argv[0], argv[1]);
    return 0;
}


/**
    Determine if the byte array is in async mode
    function get async(): Boolean
 */
static EjsObj *ba_async(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return ap->async ? (EjsObj*) ejs->trueValue : (EjsObj*) ejs->falseValue;
}


/**
    Set the async mode 
    function set async(enable: Boolean): Void
 */
static EjsObj *ba_setAsync(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    ap->async = (argv[0] == (EjsObj*) ejs->trueValue);
    return 0;
}


/**
    Get the number of bytes that are currently available on this stream for reading.
    function get available(): Number
 */
static EjsObj *ba_available(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, ap->writePosition - ap->readPosition);
}


/*
    function close(): Void
 */
static EjsObj *ba_close(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "close", (EjsObj*) ap);
    }
    ap->writePosition = ap->readPosition = 0;
    return 0;
}


/*
    Copy data down and adjust the read/write offset pointers.
    function compact(): Void
 */
static EjsObj *ba_compact(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    mprAssert(argc == 0);

    if (ap->writePosition == ap->readPosition) {
        ap->writePosition = ap->readPosition = 0;

    } else if (ap->readPosition > 0) {
        memmove(ap->value, &ap->value[ap->readPosition], ap->writePosition - ap->readPosition);
        ap->writePosition -= ap->readPosition;
        ap->readPosition = 0;
    }
    return 0;
}


/*
    Copy data into the array. Data is written at the $destOffset.
    function copyIn(destOffset: Number, src: ByteArray, srcOffset: Number = 0, count: Number = -1): Number
 */
static EjsObj *ba_copyIn(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    EjsByteArray    *src;
    int             i, destOffset, srcOffset, count;

    destOffset = ejsGetInt(ejs, argv[0]);
    src = (EjsByteArray*) argv[1];
    srcOffset = (argc > 2) ? ejsGetInt(ejs, argv[2]) : 0;
    count = (argc > 3) ? ejsGetInt(ejs, argv[3]) : MAXINT;

    if (srcOffset >= src->length) {
        ejsThrowOutOfBoundsError(ejs, "Bad source offset");
        return 0;
    }
    if (count < 0) {
        count = MAXINT;
    }
    count = min(src->length - srcOffset, count);

    makeRoom(ejs, ap, destOffset + count);
    if ((destOffset + count) > src->length) {
        ejsThrowOutOfBoundsError(ejs, "Insufficient room for data");
        return 0;
    }
    //  TODO OPT - use memmove/memcpy
    for (i = 0; i < count; i++) {
        ap->value[destOffset++] = src->value[srcOffset++];
    }
    return (EjsObj*) ejsCreateNumber(ejs, count);
}


/*
    Copy data from the array. Data is copied from the $srcOffset.
    function copyOut(srcOffset: Number, dest: ByteArray, destOffset: Number = 0, count: Number = -1): Number
 */
static EjsObj *ba_copyOut(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    EjsByteArray    *dest;
    int             i, srcOffset, destOffset, count;

    srcOffset = ejsGetInt(ejs, argv[0]);
    dest = (EjsByteArray*) argv[1];
    destOffset = (argc > 2) ? ejsGetInt(ejs, argv[2]) : 0;
    count = (argc > 3) ? ejsGetInt(ejs, argv[3]) : MAXINT;

    if (srcOffset >= ap->length) {
        ejsThrowOutOfBoundsError(ejs, "Bad source data offset");
        return 0;
    }
    count = min(ap->length - srcOffset, count);
    makeRoom(ejs, dest, destOffset + count);
    if ((destOffset + count) > dest->length) {
        ejsThrowOutOfBoundsError(ejs, "Insufficient room for data");
        return 0;
    }
    //  TODO OPT - use memmove/memcpy
    for (i = 0; i < count; i++) {
        dest->value[destOffset++] = ap->value[srcOffset++];
    }
    return (EjsObj*) ejsCreateNumber(ejs, count);
}


/*
    Determine if the system is using little endian byte ordering
    function get endian(): Number
 */
static EjsObj *endian(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, ap->endian);
}


/*
    Set the system encoding to little or big endian.
    function set endian(value: Number): Void
 */
static EjsObj *setEndian(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     endian;

    mprAssert(argc == 1 && ejsIsNumber(argv[0]));

    endian = ejsGetInt(ejs, argv[0]);
    if (endian != 0 && endian != 1) {
        ejsThrowArgError(ejs, "Bad endian value");
        return 0;
    }
    ap->endian = endian;
    ap->swap = (ap->endian != mprGetEndian(ejs));
    return 0;
}


/*
    Function to iterate and return the next element index.
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsObj *nextByteArrayKey(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsByteArray    *ap;

    ap = (EjsByteArray*) ip->target;
    if (!ejsIsByteArray(ap)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }
    if (ip->index < ap->readPosition) {
        ip->index = ap->readPosition;
    }
    if (ip->index < ap->writePosition) {
        return (EjsObj*) ejsCreateNumber(ejs, ip->index++);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return the default iterator. This returns the array index names.
    iterator native function get(): Iterator
 */
static EjsObj *ba_get(Ejs *ejs, EjsObj *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, ap, (EjsProc) nextByteArrayKey, 0, NULL);
}


/*
    Function to iterate and return the next element value.
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsObj *nextByteArrayValue(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsByteArray    *ap;

    ap = (EjsByteArray*) ip->target;
    if (!ejsIsByteArray(ap)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }
    if (ip->index < ap->readPosition) {
        ip->index = ap->readPosition;
    }
    if (ip->index < ap->writePosition) {
        return (EjsObj*) ejsCreateNumber(ejs, ap->value[ip->index++]);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*  
    Return an iterator to return the next array element value.
    iterator native function getValues(): Iterator
 */
static EjsObj *ba_getValues(Ejs *ejs, EjsObj *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, ap, (EjsProc) nextByteArrayValue, 0, NULL);
}


/*  
    Flush the data in the byte array and reset the read and write position pointers
    function flush(ignored: Number): Void
 */
static EjsObj *ba_flush(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    flushByteArray(ejs, ap);
    ap->writePosition = ap->readPosition = 0;
    return 0;
}


/*
    Get the length of an array.
    @return Returns the number of items in the array
    override function get length(): Number
 */
static EjsObj *ba_getLength(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, ap->length);
}


#if KEEP
/*
    Set the length of an array.
    override function set length(value: Number): void
 */
static EjsObj *ba_setLength(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    mprAssert(argc == 1 && ejsIsNumber(argv[0]));
    mprAssert(ejsIsByteArray(ap));

    ap->length = ejsGetInt(ejs, argv[0]);
    if (ap->readPosition >= ap->length) {
        ap->readPosition = ap->length - 1;
    }
    if (ap->writePosition >= ap->length) {
        ap->writePosition = ap->length - 1;
    }

    return 0;
}
#endif


/*
    Get the length of an array.
    @return Returns the number of items in the array
    function growable(): Boolean
 */
static EjsObj *ba_growable(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, ap->length);
}


/*
    Read data from the array into another byte array. Data is read from the current read $position pointer.
    Data is written to the write position if offset is -1. Othwise at the given offset. If offset is < 0, the 
    write position is updated.
    function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number
 */
static EjsObj *ba_read(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    EjsByteArray    *buffer;
    int             offset, count, i;

    mprAssert(1 <= argc && argc <= 3);

    buffer = (EjsByteArray*) argv[0];
    offset = (argc == 2) ? ejsGetInt(ejs, argv[1]) : 0;
    count = (argc == 3) ? ejsGetInt(ejs, argv[2]) : buffer->length;

    if (count < 0) {
        count = buffer->length;
    }
    if (offset < 0) {
        offset = buffer->writePosition;
    } else if (offset >= buffer->length) {
        ejsThrowOutOfBoundsError(ejs, "Bad read offset value");
        return 0;
    } else {
        buffer->readPosition = 0;
        buffer->writePosition = 0;
    }
    if (getInput(ejs, ap, 1) <= 0) {
        /* eof */
        return (EjsObj*) ejs->nullValue;
    }
    count = min(availableBytes(ap), count);
    for (i = 0; i < count; i++) {
        buffer->value[offset++] = ap->value[ap->readPosition++];
    }
    buffer->writePosition += count;
    if (ap->emitter && availableBytes(ap) && !ejs->exception) {
        ejsSendEvent(ejs, ap->emitter, "writable", (EjsObj*) ap);
    }
    return (EjsObj*) ejsCreateNumber(ejs, count);
}


/*
    Read a boolean from the array.
    function readBoolean(): Boolean
 */
static EjsObj *ba_readBoolean(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     result;

    if (getInput(ejs, ap, 1) <= 0) {
        /* eof */
        return (EjsObj*) ejs->nullValue;
    }
    result = ap->value[ap->readPosition];
    adjustReadPosition(ap, 1);
    return (EjsObj*) ejsCreateBoolean(ejs, result);
}


/*
    Read a byte from the array.
    function readByte(): Number
 */
static EjsObj *ba_readByte(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     result;

    if (getInput(ejs, ap, 1) <= 0) {
        /* eof */
        return (EjsObj*) ejs->nullValue;
    }
    result = ap->value[ap->readPosition];
    adjustReadPosition(ap, 1);
    return (EjsObj*) ejsCreateNumber(ejs, result);
}


/**
    Read a date from the array.
    function readDate(): Date
 */
static EjsObj *ba_readDate(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    double  value;

    if (getInput(ejs, ap, EJS_SIZE_DOUBLE) <= 0) {
        if (availableBytes(ap) > 0) {
            ejsThrowIOError(ejs, "Premanture eof");
            return 0;
        }
        return (EjsObj*) ejs->nullValue;
    }
    value = * (double*) &ap->value[ap->readPosition];
    value = swapDouble(ap, value);
    adjustReadPosition(ap, sizeof(double));
    return (EjsObj*) ejsCreateDate(ejs, (MprTime) value);
}


/**
    Read a double from the array. The data will be decoded according to the encoding property.
    function readDouble(): Date
 */
static EjsObj *ba_readDouble(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    double  value;

    if (getInput(ejs, ap, EJS_SIZE_DOUBLE) <= 0) {
        if (availableBytes(ap) > 0) {
            ejsThrowIOError(ejs, "Premanture eof");
            return 0;
        }
        return (EjsObj*) ejs->nullValue;
    }
#if OLD
    value = * (double*) &ap->value[ap->readPosition];
#else
    memcpy(&value, (char*) &ap->value[ap->readPosition], sizeof(double));
#endif
    value = swapDouble(ap, value);
    adjustReadPosition(ap, sizeof(double));
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) value);
}


/*
    Read a 32-bit integer from the array. The data will be decoded according to the encoding property.
    function readInteger(): Number
 */
static EjsObj *ba_readInteger(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     value;

    if (getInput(ejs, ap, EJS_SIZE_INT) <= 0) {
        if (availableBytes(ap) > 0) {
            ejsThrowIOError(ejs, "Premanture eof");
            return 0;
        }
        return (EjsObj*) ejs->nullValue;
    }
    value = * (int*) &ap->value[ap->readPosition];
    value = swap32(ap, value);
    adjustReadPosition(ap, sizeof(int));
    return (EjsObj*) ejsCreateNumber(ejs, value);
}


/*
    Read a 64-bit long from the array.The data will be decoded according to the encoding property.
    function readLong(): Number
 */
static EjsObj *ba_readLong(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int64   value;

    if (getInput(ejs, ap, EJS_SIZE_LONG) <= 0) {
        if (availableBytes(ap) > 0) {
            ejsThrowIOError(ejs, "Premanture eof");
            return 0;
        }
        return (EjsObj*) ejs->nullValue;
    }
    value = * (int64*) &ap->value[ap->readPosition];
    value = swap64(ap, value);
    adjustReadPosition(ap, sizeof(int64));
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) value);
}


/*
    Get the current read position offset
    function get readPosition(): Number
 */
static EjsObj *ba_readPosition(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, ap->readPosition);
}


/*
    Set the current read position offset
    function set readPosition(position: Number): Void
 */
static EjsObj *ba_setReadPosition(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     pos;

    mprAssert(argc == 1 && ejsIsNumber(argv[0]));

    pos = ejsGetInt(ejs, argv[0]);
    if (pos < 0 || pos > ap->length) {
        ejsThrowOutOfBoundsError(ejs, "Bad position value");
        return 0;
    }
    if (pos > ap->writePosition) {
        ejsThrowStateError(ejs, "Read position is greater than write position");
    } else {
        ap->readPosition = pos;
    }
    return 0;
}


/*
    Read a 16-bit short integer from the array. The data will be decoded according to the encoding property.
    function readShort(): Number
 */
static EjsObj *ba_readShort(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     value;

    if (getInput(ejs, ap, EJS_SIZE_SHORT) <= 0) {
        return (EjsObj*) ejs->nullValue;
    }
    value = * (short*) &ap->value[ap->readPosition];
    value = swap16(ap, value);
    adjustReadPosition(ap, sizeof(short));
    return (EjsObj*) ejsCreateNumber(ejs, value);
}


/*
    Read a UTF-8 string from the array. Read data from the read position up to the write position but not more than count characters.
    function readString(count: Number = -1): String
 */
static EjsObj *ba_readString(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    EjsObj  *result;
    int     count;

    count = (argc == 1) ? ejsGetInt(ejs, argv[0]) : -1;

    if (count < 0) {
        if (getInput(ejs, ap, 1) < 0) {
            return (EjsObj*) ejs->nullValue;
        }
        count = availableBytes(ap);

    } else if (getInput(ejs, ap, count) < 0) {
        return (EjsObj*) ejs->nullValue;
    }
    count = min(count, availableBytes(ap));
    result = (EjsObj*) ejsCreateStringWithLength(ejs, (cchar*) &ap->value[ap->readPosition], count);
    adjustReadPosition(ap, count);
    return result;
}


/*
    Reset the read and write position pointers if there is no available data.
    function reset(): Void
 */
static EjsObj *ba_reset(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    mprAssert(argc == 0);

    if (ap->writePosition == ap->readPosition) {
        ap->writePosition = ap->readPosition = 0;
    }
    return 0;
}


/**
    function get removeObserver(name, listener: Function): Number
 */
static EjsObj *ba_removeObserver(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    ejsRemoveListener(ejs, ap->emitter, argv[0], argv[1]);
    return 0;
}


/**
    Get the number of data bytes that the array can store from the write position till the end of the array.
    function get room(): Number
 */
static EjsObj *ba_room(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, room(ap));
}


/*
    Convert the byte array data between the read and write positions into a string.
    override function toString(): String
 */
static EjsObj *ba_toString(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringWithLength(ejs, (cchar*) &ap->value[ap->readPosition], availableBytes(ap));
}


/*
    Write data to the array. Data is written to the current write $position pointer.
    function write(...data): Number
 */
EjsNumber *ejsWriteToByteArray(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    EjsArray        *args;
    EjsByteArray    *bp;
    EjsString       *sp;
    EjsObj          *vp;
    int             i, len, wrote;

    mprAssert(argc == 1 && ejsIsArray(argv[0]));

    /*
        Unwrap nested arrays
     */
    args = (EjsArray*) argv[0];
    while (args && ejsIsArray(args) && args->length == 1) {
        vp = ejsGetProperty(ejs, (EjsVar*) args, 0);
        if (!ejsIsArray(vp)) {
            break;
        }
        args = (EjsArray*) vp;
    }

    if (availableBytes(ap) == 0) {
        ap->writePosition = ap->readPosition = 0;
    }
    wrote = 0;

    for (i = 0; i < args->length; i++) {
        vp = ejsGetProperty(ejs, (EjsVar*) args, i);
        if (vp == 0) {
            continue;
        }
        switch (vp->type->id) {
        case ES_Boolean:
            if (!makeRoom(ejs, ap, EJS_SIZE_BOOLEAN)) {
                return 0;
            }
            wrote += putByte(ap, ejsGetBoolean(ejs, vp));
            break;

        case ES_Date:
            if (!makeRoom(ejs, ap, EJS_SIZE_DOUBLE)) {
                return 0;
            }
            wrote += putNumber(ap, (MprNumber) ((EjsDate*) vp)->value);
            break;

        case ES_Number:
            if (!makeRoom(ejs, ap, EJS_SIZE_DOUBLE)) {
                return 0;
            }
            wrote += putNumber(ap, ejsGetNumber(ejs, vp));
            break;

        case ES_String:
            if (!makeRoom(ejs, ap, ((EjsString*) vp)->length)) {
                return 0;
            }
            sp = (EjsString*) vp;
            wrote += putString(ap, sp->value, sp->length);
            break;

        default:
            sp = ejsToString(ejs, vp);
            wrote += putString(ap, sp->value, sp->length);
            break;

        case ES_ByteArray:
            bp = (EjsByteArray*) vp;
            len = availableBytes(bp);
            if (!makeRoom(ejs, ap, len)) {
                return 0;
            }
            /*
                Note: this only copies between the read/write positions of the source byte array
             */
            ejsCopyToByteArray(ejs, ap, ap->writePosition, (char*) &bp->value[bp->readPosition], len);
            ap->writePosition += len;
            wrote += len;
            break;
        }
    }
    if (ap->emitter && wrote > 0 && availableBytes(ap) > 0) {
        ejsSendEvent(ejs, ap->emitter, "readable", (EjsObj*) ap);
    }
    return ejsCreateNumber(ejs, wrote);
}


/*
    Write a byte to the array
    function writeByte(value: Number): Void
 */
static EjsObj *ba_writeByte(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    if (!makeRoom(ejs, ap, 1)) {
        return 0;
    }
    putByte(ap, ejsGetInt(ejs, argv[0]));
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "readable", (EjsObj*) ap);
    }
    return 0;
}


/*
    Write a short to the array
    function writeShort(value: Number): Void
 */
static EjsObj *ba_writeShort(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    if (!makeRoom(ejs, ap, sizeof(short))) {
        return 0;
    }
    putShort(ap, ejsGetInt(ejs, argv[0]));
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "readable", (EjsObj*) ap);
    }
    return 0;
}


/*
    Write a double to the array
    function writeDouble(value: Number): Void
 */
static EjsObj *ba_writeDouble(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    if (!makeRoom(ejs, ap, sizeof(double))) {
        return 0;
    }
    putDouble(ap, ejsGetDouble(ejs, argv[0]));
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "readable", (EjsObj*) ap);
    }
    return 0;
}


/*
    Write an integer (32 bits) to the array
    function writeInteger(value: Number): Void
 */

static EjsObj *ba_writeInteger(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    if (!makeRoom(ejs, ap, sizeof(int))) {
        return 0;
    }
    putInteger(ap, ejsGetInt(ejs, argv[0]));
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "readable", (EjsObj*) ap);
    }
    return 0;
}


/*
    Write a long (64 bit) to the array
    function writeLong(value: Number): Void
 */
static EjsObj *ba_writeLong(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    if (!makeRoom(ejs, ap, sizeof(int))) {
        return 0;
    }
    putLong(ap, ejsGetInt(ejs, argv[0]));
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "readable", (EjsObj*) ap);
    }
    return 0;
}


/*
    Get the current write position offset
    function get writePosition(): Number
 */
static EjsObj *ba_writePosition(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, ap->writePosition);
}


/*
    Set the current write position offset
    function set writePosition(position: Number): Void
 */
static EjsObj *ba_setWritePosition(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     pos;

    mprAssert(argc == 1 && ejsIsNumber(argv[0]));

    pos = ejsGetInt(ejs, argv[0]);
    if (pos < 0 || pos > ap->length) {
        ejsThrowStateError(ejs, "Write position is outside bounds of array");
        return 0;
    }
    if (pos < ap->readPosition) {
        ejsThrowStateError(ejs, "Write position is less than read position");
    } else {
        ap->writePosition = pos;
    }
    return 0;
}


/*********************************** Support **********************************/
/*
    Flush the array. Issue a "flush" event. Flushing attempts to write pending data before resetting the array.
 */
static int flushByteArray(Ejs *ejs, EjsByteArray *ap)
{
    if (ap->emitter && availableBytes(ap) && !ejs->exception) {
        ejsSendEvent(ejs, ap->emitter, "flush", (EjsObj*) ap);
    }
    ap->writePosition = ap->readPosition = 0;
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "writable", (EjsObj*) ap);
    }
    return 0;
}


int ejsGrowByteArray(Ejs *ejs, EjsByteArray *ap, int len)
{
    if (len > ap->length) {
        ap->value = mprRealloc(ap, ap->value, len);
        if (ap->value == 0) {
            ejsThrowMemoryError(ejs);
            return EJS_ERR;
        }
        memset(&ap->value[ap->length], 0, len - ap->length);
        ap->growInc = min(ap->growInc * 2, 32 * 1024);
        ap->length = len;
    }
    return 0;
}


/*
    Get more input sufficient to satisfy the rquired number of bytes. The required parameter specifies how many bytes 
    must be read. Short fills are not permitted. Return the count of bytes available or 0 if the required number of 
    bytes can't be read. Return -ve on errors.
 */
static int getInput(Ejs *ejs, EjsByteArray *ap, int required)
{
    if (availableBytes(ap) == 0) {
        ap->writePosition = ap->readPosition = 0;
    }
    if (availableBytes(ap) < required && !ejs->exception) {
        if (ap->emitter) {
            ejsSendEvent(ejs, ap->emitter, "writable", (EjsObj*) ap);
        }
    }
    if (availableBytes(ap) < required) {
        return 0;
    }
    return availableBytes(ap);
}


static bool makeRoom(Ejs *ejs, EjsByteArray *ap, int require)
{
    int     newLen;

    if (room(ap) < require) {
        if (ap->emitter) {
            ejsSendEvent(ejs, ap->emitter, "readable", (EjsObj*) ap);
        }
        if (room(ap) < require) {
            newLen = max(ap->length + require, ap->length + ap->growInc);
            if (!ap->growable || ejsGrowByteArray(ejs, ap, newLen) < 0) {
                if (!ejs->exception) {
                    ejsThrowResourceError(ejs, "Byte array is too small");
                }
                return 0;
            }
        }
    }
    return 1;
}


static MPR_INLINE int swap16(EjsByteArray *ap, int a)
{
    if (!ap->swap) {
        return a;
    }
    return (a & 0xFF) << 8 | (a & 0xFF00 >> 8);
}


static MPR_INLINE int swap32(EjsByteArray *ap, int a)
{
    if (!ap->swap) {
        return a;
    }
    return (a & 0xFF) << 24 | (a & 0xFF00 << 8) | (a & 0xFF0000 >> 8) | (a & 0xFF000000 >> 16);
}


static MPR_INLINE int64 swap64(EjsByteArray *ap, int64 a)
{
    int64   low, high;

    if (!ap->swap) {
        return a;
    }
    low = a & 0xFFFFFFFF;
    high = (a >> 32) & 0xFFFFFFFF;
    return  (low & 0xFF) << 24 | (low & 0xFF00 << 8) | (low & 0xFF0000 >> 8) | (low & 0xFF000000 >> 16) |
            ((high & 0xFF) << 24 | (high & 0xFF00 << 8) | (high & 0xFF0000 >> 8) | (high & 0xFF000000 >> 16)) << 32;
}


static MPR_INLINE double swapDouble(EjsByteArray *ap, double a)
{
    int64   low, high;

    if (!ap->swap) {
        return a;
    }
    low = ((int64) a) & 0xFFFFFFFF;
    high = (((int64) a) >> 32) & 0xFFFFFFFF;
    return  (double) ((low & 0xFF) << 24 | (low & 0xFF00 << 8) | (low & 0xFF0000 >> 8) | (low & 0xFF000000 >> 16) |
            ((high & 0xFF) << 24 | (high & 0xFF00 << 8) | (high & 0xFF0000 >> 8) | (high & 0xFF000000 >> 16)) << 32);
}


static int putByte(EjsByteArray *ap, int value)
{
    ap->value[ap->writePosition++] = (char) value;
    return sizeof(char);
}


static int putShort(EjsByteArray *ap, int value)
{
    value = swap16(ap, value);

    *((short*) &ap->value[ap->writePosition]) = (short) value;
    ap->writePosition += sizeof(short);
    return sizeof(short);
}


static int putInteger(EjsByteArray *ap, int value)
{
    value = swap32(ap, value);

    *((int*) &ap->value[ap->writePosition]) = (int) value;
    ap->writePosition += sizeof(int);
    return sizeof(int);
}


static int putLong(EjsByteArray *ap, int64 value)
{
    value = swap64(ap, value);

    *((int64*) &ap->value[ap->writePosition]) = value;
    ap->writePosition += sizeof(int64);
    return sizeof(int64);
}


static int putDouble(EjsByteArray *ap, double value)
{
    value = swapDouble(ap, value);

#if OLD
    *((double*) &ap->value[ap->writePosition]) = value;
#else
    memcpy((char*) &ap->value[ap->writePosition], &value, sizeof(double));
#endif
    ap->writePosition += sizeof(double);
    return sizeof(double);
}


/*
    Write a number in the default number encoding
 */
static int putNumber(EjsByteArray *ap, MprNumber value)
{
    return putDouble(ap, value);
}


static int putString(EjsByteArray *ap, cchar *value, int len)
{
    mprMemcpy(&ap->value[ap->writePosition], room(ap), value, len);
    ap->writePosition += len;
    return len;
}

/********************************* Public Support API *****************************/

void ejsResetByteArray(EjsByteArray *ba)
{
    if (ba->writePosition == ba->readPosition) {
        ba->writePosition = ba->readPosition = 0;
    }
}


void ejsSetByteArrayPositions(Ejs *ejs, EjsByteArray *ba, int readPosition, int writePosition)
{
    if (readPosition >= 0) {
        ba->readPosition = readPosition;
    }
    if (writePosition >= 0) {
        ba->writePosition = writePosition;
    }
}


int ejsCopyToByteArray(Ejs *ejs, EjsByteArray *ba, int offset, char *data, int length)
{
    int     i;

    mprAssert(ba);
    mprAssert(data);

    if (!makeRoom(ejs, ba, offset + length)) {
        return EJS_ERR;
    }
    if (ba->length < (offset + length)) {
        return EJS_ERR;
    }
    for (i = 0; i < length; i++) {
        ba->value[offset++] = data[i];
    }
    return 0;
}


int ejsGetByteArrayAvailable(EjsByteArray *ba)
{
    return availableBytes(ba);
}


int ejsGetByteArrayRoom(EjsByteArray *ba)
{
    return room(ba);
}

/*********************************** Factory **********************************/

EjsByteArray *ejsCreateByteArray(Ejs *ejs, int size)
{
    EjsByteArray    *ap;

    /*
        No need to invoke constructor
     */
    ap = (EjsByteArray*) ejsCreate(ejs, ejs->byteArrayType, 0);
    if (ap == 0) {
        return 0;
    }
    if (size <= 0) {
        size = MPR_BUFSIZE;
    }
    if (ejsGrowByteArray(ejs, ap, size) < 0) {
        return 0;
    }
    ap->length = size;
    ap->async = -1;
    ap->growable = 1;
    ap->growInc = MPR_BUFSIZE;
    ap->endian = mprGetEndian(ejs);
    ejsSetDebugName(ap, "ByteArray instance");
    return ap;
}


void ejsConfigureByteArrayType(Ejs *ejs)
{
    EjsType         *type;
    EjsTypeHelpers  *helpers;
    EjsObj          *prototype;

    type = ejs->byteArrayType = ejsConfigureNativeType(ejs, "ejs", "ByteArray", sizeof(EjsByteArray));
    type->numericIndicies = 1;
    type->virtualSlots = 1;
    prototype = type->prototype;

    helpers = &type->helpers;
    helpers->cast = (EjsCastHelper) castByteArrayVar;
    helpers->clone = (EjsCloneHelper) cloneByteArrayVar;
    helpers->getProperty = (EjsGetPropertyHelper) getByteArrayProperty;
    helpers->getPropertyCount = (EjsGetPropertyCountHelper) getByteArrayPropertyCount;
    helpers->deleteProperty = (EjsDeletePropertyHelper) deleteByteArrayProperty;
    helpers->invokeOperator = (EjsInvokeOperatorHelper) invokeByteArrayOperator;
    helpers->mark = (EjsMarkHelper) markByteArrayVar;
    helpers->lookupProperty = (EjsLookupPropertyHelper) lookupByteArrayProperty;
    helpers->setProperty = (EjsSetPropertyHelper) setByteArrayProperty;
    
    ejsBindConstructor(ejs, type, (EjsProc) ba_ByteArray);
    ejsBindMethod(ejs, prototype, ES_ByteArray_observe, (EjsProc) ba_observe);
    ejsBindMethod(ejs, prototype, ES_ByteArray_available, (EjsProc) ba_available);
    ejsBindAccess(ejs, prototype, ES_ByteArray_async, (EjsProc) ba_async, (EjsProc) ba_setAsync);
    ejsBindMethod(ejs, prototype, ES_ByteArray_close, (EjsProc) ba_close);
    ejsBindMethod(ejs, prototype, ES_ByteArray_compact, (EjsProc) ba_compact);
    ejsBindMethod(ejs, prototype, ES_ByteArray_copyIn, (EjsProc) ba_copyIn);
    ejsBindMethod(ejs, prototype, ES_ByteArray_copyOut, (EjsProc) ba_copyOut);
    ejsBindMethod(ejs, prototype, ES_ByteArray_flush, (EjsProc) ba_flush);
    ejsBindMethod(ejs, prototype, ES_ByteArray_growable, (EjsProc) ba_growable);
    ejsBindMethod(ejs, prototype, ES_ByteArray_length, (EjsProc) ba_getLength);
    ejsBindMethod(ejs, prototype, ES_ByteArray_iterator_get, (EjsProc) ba_get);
    ejsBindMethod(ejs, prototype, ES_ByteArray_iterator_getValues, (EjsProc) ba_getValues);
    ejsBindAccess(ejs, prototype, ES_ByteArray_endian, (EjsProc) endian, (EjsProc) setEndian);
    ejsBindMethod(ejs, prototype, ES_ByteArray_read, (EjsProc) ba_read);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readBoolean, (EjsProc) ba_readBoolean);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readByte, (EjsProc) ba_readByte);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readDate, (EjsProc) ba_readDate);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readDouble, (EjsProc) ba_readDouble);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readInteger, (EjsProc) ba_readInteger);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readLong, (EjsProc) ba_readLong);
    ejsBindAccess(ejs, prototype, ES_ByteArray_readPosition, (EjsProc) ba_readPosition,(EjsProc) ba_setReadPosition);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readShort, (EjsProc) ba_readShort);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readString, (EjsProc) ba_readString);
    ejsBindMethod(ejs, prototype, ES_ByteArray_removeObserver, (EjsProc) ba_removeObserver);
    ejsBindMethod(ejs, prototype, ES_ByteArray_reset, (EjsProc) ba_reset);
    ejsBindMethod(ejs, prototype, ES_ByteArray_room, (EjsProc) ba_room);
    ejsBindMethod(ejs, prototype, ES_ByteArray_toString, (EjsProc) ba_toString);
    ejsBindMethod(ejs, prototype, ES_ByteArray_write, (EjsProc) ejsWriteToByteArray);
    ejsBindMethod(ejs, prototype, ES_ByteArray_writeByte, (EjsProc) ba_writeByte);
    ejsBindMethod(ejs, prototype, ES_ByteArray_writeShort, (EjsProc) ba_writeShort);
    ejsBindMethod(ejs, prototype, ES_ByteArray_writeInteger, (EjsProc) ba_writeInteger);
    ejsBindMethod(ejs, prototype, ES_ByteArray_writeLong, (EjsProc) ba_writeLong);
    ejsBindMethod(ejs, prototype, ES_ByteArray_writeDouble, (EjsProc) ba_writeDouble);
    ejsBindAccess(ejs, prototype, ES_ByteArray_writePosition, (EjsProc) ba_writePosition, (EjsProc) ba_setWritePosition);
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
