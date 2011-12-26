/*
    ejsByteArray.c - Ejscript ByteArray class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/***************************** Forward Declarations ***************************/

static EjsObj *ba_flush(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv);
static EjsString *ba_toString(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv);

static int  flushByteArray(Ejs *ejs, EjsByteArray *ap);
static ssize  getInput(Ejs *ejs, EjsByteArray *ap, ssize required);
static int  lookupByteArrayProperty(Ejs *ejs, EjsByteArray *ap, EjsName qname);

static MPR_INLINE int swap16(EjsByteArray *ap, int a);
static MPR_INLINE int swap32(EjsByteArray *ap, int a);
static MPR_INLINE int64 swap64(EjsByteArray *ap, int64 a);
static MPR_INLINE double swapDouble(EjsByteArray *ap, double a);
static int putByte(EjsByteArray *ap, int value);
static int putInteger(EjsByteArray *ap, int value);
static int putLong(EjsByteArray *ap, int64 value);
static int putShort(EjsByteArray *ap, int value);
static int putString(EjsByteArray *ap, EjsString *value, ssize len);
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
static EjsAny *castByteArrayVar(Ejs *ejs, EjsByteArray *vp, EjsType *type)
{
    switch (type->sid) {
    case S_Boolean:
        return ESV(true);

    case S_Number:
        return ESV(zero);

    case S_String:
        return ba_toString(ejs, vp, 0, 0);

    default:
        ejsThrowTypeError(ejs, "Can't cast to this type");
        return 0;
    }
}


static EjsByteArray *cloneByteArrayVar(Ejs *ejs, EjsByteArray *ap, bool deep)
{
    EjsByteArray    *newArray;

    if ((newArray = ejsCreateByteArray(ejs, ap->length)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    memcpy(newArray->value, ap->value, ap->length * sizeof(uchar));
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
    if (ejsSetProperty(ejs, ap, slot, ESV(undefined)) < 0) {
        return EJS_ERR;
    }
    return 0;
}


static ssize getByteArrayPropertyCount(Ejs *ejs, EjsByteArray *ap)
{
    return ap->length;
}


static EjsNumber *getByteArrayProperty(Ejs *ejs, EjsByteArray *ap, int slotNum)
{
    if (slotNum < 0 || slotNum >= ap->length) {
        ejsThrowOutOfBoundsError(ejs, "Bad array subscript");
        return 0;
    }
    return ejsCreateNumber(ejs, ap->value[slotNum]);
}


static int lookupByteArrayProperty(struct Ejs *ejs, EjsByteArray *ap, EjsName qname)
{
    int     index;

    if (qname.name == 0 || ! isdigit((int) qname.name->value[0])) {
        return EJS_ERR;
    }
    index = ejsAtoi(ejs, qname.name, 10);
    if (index < ap->length) {
        return index;
    }
    return EJS_ERR;
}


/*
    Cast operands as required for invokeOperator
 */
static EjsAny *coerceByteArrayOperands(Ejs *ejs, EjsObj *lhs, int opcode,  EjsObj *rhs)
{
    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsInvokeOperator(ejs, ba_toString(ejs, (EjsByteArray*) lhs, 0, 0), opcode, rhs);

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, ESV(zero), opcode, rhs);

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
        if (!ejsIsDefined(ejs, rhs)) {
            return ((opcode == EJS_OP_COMPARE_EQ) ? ESV(false): ESV(true));
        } else if (ejsIs(ejs, rhs, Number)) {
            return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIs(ejs, rhs, Number)) {
            return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_STRICTLY_NE:
    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_NULL:
        return ESV(true);

    case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return ESV(false);

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return ESV(undefined);
    }
    return 0;
}


static EjsAny *invokeByteArrayOperator(Ejs *ejs, EjsObj *lhs, int opcode,  EjsObj *rhs)
{
    EjsObj      *result;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if ((result = coerceByteArrayOperands(ejs, lhs, opcode, rhs)) != 0) {
            return result;
        }
    }

    switch (opcode) {

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_GE:
        return ejsCreateBoolean(ejs, (lhs == rhs));

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
    case EJS_OP_COMPARE_LT: case EJS_OP_COMPARE_GT:
        return ejsCreateBoolean(ejs, !(lhs == rhs));

    /*
        Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return ESV(true);

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return ESV(false);

    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return ESV(one);

    /*
        Binary operators
     */
    case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_REM:
    case EJS_OP_SHR: case EJS_OP_USHR: case EJS_OP_XOR:
        return ESV(zero);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
    mprAssert(0);
}


/*
    Create or update an array elements. If slotNum is < 0, then create the next free array slot. If slotNum is greater
    than the array length, grow the array.
 */
static int setByteArrayProperty(struct Ejs *ejs, EjsByteArray *ap, int slotNum,  EjsObj *value)
{
    if (slotNum >= ap->length && ejsGrowByteArray(ejs, ap, slotNum + 1) < 0) {
        return EJS_ERR;
    }
    if (ejsIs(ejs, value, Number)) {
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
    function ByteArray(size: Number = -1, resizable: Boolean = true): ByteArray
 */
static EjsByteArray *ba_ByteArray(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    bool    resizable;
    int     size;

    mprAssert(0 <= argc && argc <= 2);

    size = (argc >= 1) ? ejsGetInt(ejs, argv[0]) : MPR_BUFSIZE;
    if (size <= 0) {
        size = 1;
    }
    resizable = (argc == 2) ? ejsGetBoolean(ejs, argv[1]): 1;
    ap->growInc = (resizable) ? MPR_BUFSIZE : 0;
    ap->endian = mprGetEndian(ejs);
    ap->resizable = 1;
    if (ejsGrowByteArray(ejs, ap, size) < 0) {
        return 0;
    }
    ap->resizable = resizable;
    mprAssert(ap->value);
    return ap;
}


/**
    Determine if the byte array is in async mode
    function get async(): Boolean
 */
static EjsBoolean *ba_async(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return ap->async ? ESV(true) : ESV(false);
}


/**
    Set the async mode 
    function set async(enable: Boolean): Void
 */
static EjsObj *ba_setAsync(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    ap->async = (argv[0] == ESV(true));
    return 0;
}


/**
    Get the number of bytes that are currently available on this stream for reading.
    function get available(): Number
 */
static EjsNumber *ba_available(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) (ap->writePosition - ap->readPosition));
}


/*
    function close(): Void
 */
static EjsObj *ba_close(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "close", NULL, ap);
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
static EjsNumber *ba_copyIn(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    EjsByteArray    *src;
    ssize           i, destOffset, srcOffset, count;

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

    ejsMakeRoomInByteArray(ejs, ap, destOffset + count);
    if ((destOffset + count) > src->length) {
        ejsThrowOutOfBoundsError(ejs, "Insufficient room for data");
        return 0;
    }
    //  TODO OPT - use memmove/memcpy
    for (i = 0; i < count; i++) {
        ap->value[destOffset++] = src->value[srcOffset++];
    }
    return ejsCreateNumber(ejs, (MprNumber) count);
}


/*
    Copy data from the array. Data is copied from the $srcOffset.
    function copyOut(srcOffset: Number, dest: ByteArray, destOffset: Number = 0, count: Number = -1): Number
 */
static EjsNumber *ba_copyOut(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    EjsByteArray    *dest;
    ssize           count;
    int             i, srcOffset, destOffset;

    srcOffset = ejsGetInt(ejs, argv[0]);
    dest = (EjsByteArray*) argv[1];
    destOffset = (argc > 2) ? ejsGetInt(ejs, argv[2]) : 0;
    count = (argc > 3) ? ejsGetInt(ejs, argv[3]) : MAXINT;

    if (srcOffset >= ap->length) {
        ejsThrowOutOfBoundsError(ejs, "Bad source data offset");
        return 0;
    }
    count = min(ap->length - srcOffset, count);
    ejsMakeRoomInByteArray(ejs, dest, destOffset + count);
    if ((destOffset + count) > dest->length) {
        ejsThrowOutOfBoundsError(ejs, "Insufficient room for data");
        return 0;
    }
    //  TODO OPT - use memmove/memcpy
    for (i = 0; i < count; i++) {
        dest->value[destOffset++] = ap->value[srcOffset++];
    }
    return ejsCreateNumber(ejs, (MprNumber) count);
}


/*
    Determine if the system is using little endian byte ordering
    function get endian(): Number
 */
static EjsNumber *endian(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, ap->endian);
}


/*
    Set the system encoding to little or big endian.
    function set endian(value: Number): Void
 */
static EjsObj *setEndian(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     endian;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Number));

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
static EjsNumber *nextByteArrayKey(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsByteArray    *ap;

    ap = (EjsByteArray*) ip->target;
    if (!ejsIs(ejs, ap, ByteArray)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }
    if (ip->index < ap->readPosition) {
        ip->index = (int) ap->readPosition;
    }
    if (ip->index < (int) ap->writePosition) {
        return ejsCreateNumber(ejs, ip->index++);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return the default iterator. This returns the array index names.
    iterator native function get(): Iterator
 */
static EjsIterator *ba_get(Ejs *ejs, EjsObj *ap, int argc, EjsObj **argv)
{
    return ejsCreateIterator(ejs, ap, nextByteArrayKey, 0, NULL);
}


/*
    Function to iterate and return the next element value.
    NOTE: this is not a method of Array. Rather, it is a callback function for Iterator
 */
static EjsNumber *nextByteArrayValue(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsByteArray    *ap;

    ap = (EjsByteArray*) ip->target;
    if (!ejsIs(ejs, ap, ByteArray)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }
    if (ip->index < (int) ap->readPosition) {
        ip->index = (int) ap->readPosition;
    }
    if (ip->index < ap->writePosition) {
        return ejsCreateNumber(ejs, ap->value[ip->index++]);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*  
    Return an iterator to return the next array element value.
    iterator native function getValues(): Iterator
 */
static EjsIterator *ba_getValues(Ejs *ejs, EjsObj *ap, int argc, EjsObj **argv)
{
    return ejsCreateIterator(ejs, ap, nextByteArrayValue, 0, NULL);
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
static EjsNumber *ba_getLength(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (int) ap->length);
}


#if KEEP
/*
    Set the length of an array.
    override function set length(value: Number): void
 */
static EjsObj *ba_setLength(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Number));
    mprAssert(ejsIs(ejs, ap, ByteArray));

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
    function resizable(): Boolean
 */
static EjsBoolean *ba_resizable(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return ap->resizable ? ESV(true) : ESV(false);
}


/*
    Read data from the array into another byte array. Data is read from the current read $position pointer.
    Data is written to the write position if offset is -1. Othwise at the given offset. If offset is < 0, the 
    write position is updated.
    function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number
 */
static EjsNumber *ba_read(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    EjsByteArray    *buffer;
    ssize           offset, count;
    int             i;

    mprAssert(1 <= argc && argc <= 3);

    buffer = (EjsByteArray*) argv[0];
    offset = (argc == 2) ? ejsGetInt(ejs, argv[1]) : 0;
    count = (argc == 3) ? ejsGetInt(ejs, argv[2]) : (int) buffer->length;

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
        return ESV(null);
    }
    count = min(availableBytes(ap), count);
    for (i = 0; i < count; i++) {
        buffer->value[offset++] = ap->value[ap->readPosition++];
    }
    buffer->writePosition += count;
    if (ap->emitter && availableBytes(ap) && !ejs->exception) {
        ejsSendEvent(ejs, ap->emitter, "writable", NULL, ap);
    }
    return ejsCreateNumber(ejs, (MprNumber) count);
}


/*
    Read a boolean from the array.
    function readBoolean(): Boolean
 */
static EjsBoolean *ba_readBoolean(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     result;

    if (getInput(ejs, ap, 1) <= 0) {
        /* eof */
        return ESV(null);
    }
    result = ap->value[ap->readPosition];
    adjustReadPosition(ap, 1);
    return ejsCreateBoolean(ejs, result);
}


/*
    Read a byte from the array.
    function readByte(): Number
 */
static EjsNumber *ba_readByte(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     result;

    if (getInput(ejs, ap, 1) <= 0) {
        /* eof */
        return ESV(null);
    }
    result = ap->value[ap->readPosition];
    adjustReadPosition(ap, 1);
    return ejsCreateNumber(ejs, result);
}


/**
    Read a date from the array.
    function readDate(): Date
 */
static EjsDate *ba_readDate(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    double  value;

    if (getInput(ejs, ap, EJS_SIZE_DOUBLE) <= 0) {
        if (availableBytes(ap) > 0) {
            ejsThrowIOError(ejs, "Premanture eof");
            return 0;
        }
        return ESV(null);
    }
    value = * (double*) &ap->value[ap->readPosition];
    value = swapDouble(ap, value);
    adjustReadPosition(ap, sizeof(double));
    return ejsCreateDate(ejs, (MprTime) value);
}


/**
    Read a double from the array. The data will be decoded according to the encoding property.
    function readDouble(): Number
 */
static EjsNumber *ba_readDouble(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    double  value;

    if (getInput(ejs, ap, EJS_SIZE_DOUBLE) <= 0) {
        if (availableBytes(ap) > 0) {
            ejsThrowIOError(ejs, "Premanture eof");
            return 0;
        }
        return ESV(null);
    }
    memcpy(&value, (char*) &ap->value[ap->readPosition], sizeof(double));
    value = swapDouble(ap, value);
    adjustReadPosition(ap, sizeof(double));
    return ejsCreateNumber(ejs, (MprNumber) value);
}


/*
    Read a 32-bit integer from the array. The data will be decoded according to the encoding property.
    function readInteger(): Number
 */
static EjsNumber *ba_readInteger(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     value;

    if (getInput(ejs, ap, EJS_SIZE_INT) <= 0) {
        if (availableBytes(ap) > 0) {
            ejsThrowIOError(ejs, "Premanture eof");
            return 0;
        }
        return ESV(null);
    }
    value = * (int*) &ap->value[ap->readPosition];
    value = swap32(ap, value);
    adjustReadPosition(ap, sizeof(int));
    return ejsCreateNumber(ejs, value);
}


/*
    Read a 64-bit long from the array.The data will be decoded according to the encoding property.
    function readLong(): Number
 */
static EjsNumber *ba_readLong(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int64   value;

    if (getInput(ejs, ap, EJS_SIZE_LONG) <= 0) {
        if (availableBytes(ap) > 0) {
            ejsThrowIOError(ejs, "Premanture eof");
            return 0;
        }
        return ESV(null);
    }
    value = * (int64*) &ap->value[ap->readPosition];
    value = swap64(ap, value);
    adjustReadPosition(ap, sizeof(int64));
    return ejsCreateNumber(ejs, (MprNumber) value);
}


/*
    Get the current read position offset
    function get readPosition(): Number
 */
static EjsNumber *ba_readPosition(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) ap->readPosition);
}


/*
    Set the current read position offset
    function set readPosition(position: Number): Void
 */
static EjsObj *ba_setReadPosition(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     pos;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Number));

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
static EjsNumber *ba_readShort(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     value;

    if (getInput(ejs, ap, EJS_SIZE_SHORT) <= 0) {
        return ESV(null);
    }
    value = * (short*) &ap->value[ap->readPosition];
    value = swap16(ap, value);
    adjustReadPosition(ap, sizeof(short));
    return ejsCreateNumber(ejs, value);
}


/*
    Read a UTF-8 string from the array. Read data from the read position up to the write position but not more 
    than count characters.

    function readString(count: Number = -1): String
 */
static EjsString *ba_readString(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    EjsString   *result;
    ssize       count;

    count = (argc == 1) ? ejsGetInt(ejs, argv[0]) : -1;

    if (count < 0) {
        if (getInput(ejs, ap, 1) < 0) {
            return ESV(null);
        }
        count = availableBytes(ap);

    } else if (getInput(ejs, ap, count) < 0) {
        return ESV(null);
    }
    count = min(count, availableBytes(ap));
    //  TODO - UNICODE ENCODING
    result = ejsCreateStringFromMulti(ejs, (cchar*) &ap->value[ap->readPosition], count);
    adjustReadPosition(ap, count);
    return result;
}


/*
    Reset the read and write position pointers if there is no available data.
    function reset(): Void
 */
static EjsObj *ba_reset(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    ejsResetByteArray(ejs, ap);
    return 0;
}


/**
    function off(name, observer: Function): Number
 */
static EjsObj *ba_off(Ejs *ejs, EjsByteArray *ap, int argc, EjsAny **argv)
{
    ejsRemoveObserver(ejs, ap->emitter, argv[0], argv[1]);
    return 0;
}


/**
    function get on(name, observer: Function): Void
 */
static EjsObj *ba_on(Ejs *ejs, EjsByteArray *ap, int argc, EjsAny **argv)
{
    ejsAddObserver(ejs, &ap->emitter, argv[0], argv[1]);
    return 0;
}


/**
    Get the number of data bytes that the array can store from the write position till the end of the array.
    function get room(): Number
 */
static EjsNumber *ba_room(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) room(ap));
}


/*
    Convert the byte array data between the read and write positions into a string.
    override function toString(): String
 */
static EjsString *ba_toString(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    //  TODO - UNICODE ENCODING
    return ejsCreateStringFromMulti(ejs, (cchar*) &ap->value[ap->readPosition], availableBytes(ap));
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
    ssize           len, wrote;
    int             i;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Array));

    /*
        Unwrap nested arrays
     */
    args = (EjsArray*) argv[0];
    while (ejsIs(ejs, args, Array) && args->length == 1) {
        vp = ejsGetProperty(ejs, args, 0);
        if (!ejsIs(ejs, vp, Array)) {
            break;
        }
        args = (EjsArray*) vp;
    }
    if (availableBytes(ap) == 0) {
        ap->writePosition = ap->readPosition = 0;
    }
    wrote = 0;

    for (i = 0; i < args->length; i++) {
        vp = ejsGetProperty(ejs, args, i);
        if (vp == 0) {
            continue;
        }
        switch (TYPE(vp)->sid) {
        case S_Boolean:
            if (!ejsMakeRoomInByteArray(ejs, ap, EJS_SIZE_BOOLEAN)) {
                return 0;
            }
            wrote += putByte(ap, ejsGetBoolean(ejs, vp));
            break;

        case S_Date:
            if (!ejsMakeRoomInByteArray(ejs, ap, EJS_SIZE_DOUBLE)) {
                return 0;
            }
            wrote += putNumber(ap, (MprNumber) ((EjsDate*) vp)->value);
            break;

        case S_Number:
            if (!ejsMakeRoomInByteArray(ejs, ap, EJS_SIZE_DOUBLE)) {
                return 0;
            }
            wrote += putNumber(ap, ejsGetNumber(ejs, vp));
            break;

        case S_String:
            if (!ejsMakeRoomInByteArray(ejs, ap, ((EjsString*) vp)->length)) {
                return 0;
            }
            sp = (EjsString*) vp;
            wrote += putString(ap, sp, sp->length);
            break;

        default:
            sp = ejsToString(ejs, vp);
            wrote += putString(ap, sp, sp->length);
            break;

        case S_ByteArray:
            bp = (EjsByteArray*) vp;
            len = availableBytes(bp);
            if (!ejsMakeRoomInByteArray(ejs, ap, len)) {
                return 0;
            }
            /*
                Note: this only copies between the read/write positions of the source byte array
             */
            //  MOB - should use RC value (== len)
            ejsCopyToByteArray(ejs, ap, ap->writePosition, (char*) &bp->value[bp->readPosition], len);
            ap->writePosition += len;
            wrote += len;
            break;
        }
    }
    if (ap->emitter && wrote > 0 && availableBytes(ap) > 0) {
        ejsSendEvent(ejs, ap->emitter, "readable", NULL, ap);
    }
    return ejsCreateNumber(ejs, (MprNumber) wrote);
}


/*
    Write a byte to the array
    function writeByte(value: Number): Void
 */
static EjsObj *ba_writeByte(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    if (!ejsMakeRoomInByteArray(ejs, ap, 1)) {
        return 0;
    }
    putByte(ap, ejsGetInt(ejs, argv[0]));
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "readable", NULL, ap);
    }
    return 0;
}


/*
    Write a short to the array
    function writeShort(value: Number): Void
 */
static EjsObj *ba_writeShort(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    if (!ejsMakeRoomInByteArray(ejs, ap, sizeof(short))) {
        return 0;
    }
    putShort(ap, ejsGetInt(ejs, argv[0]));
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "readable", NULL, ap);
    }
    return 0;
}


/*
    Write a double to the array
    function writeDouble(value: Number): Void
 */
static EjsObj *ba_writeDouble(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    if (!ejsMakeRoomInByteArray(ejs, ap, sizeof(double))) {
        return 0;
    }
    putDouble(ap, ejsGetDouble(ejs, argv[0]));
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "readable", NULL, ap);
    }
    return 0;
}


/*
    Write an integer (32 bits) to the array
    function writeInteger(value: Number): Void
 */

static EjsObj *ba_writeInteger(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    if (!ejsMakeRoomInByteArray(ejs, ap, sizeof(int))) {
        return 0;
    }
    putInteger(ap, ejsGetInt(ejs, argv[0]));
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "readable", NULL, ap);
    }
    return 0;
}


/*
    Write a long (64 bit) to the array
    function writeLong(value: Number): Void
 */
static EjsObj *ba_writeLong(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    if (!ejsMakeRoomInByteArray(ejs, ap, sizeof(int))) {
        return 0;
    }
    putLong(ap, ejsGetInt(ejs, argv[0]));
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "readable", NULL, ap);
    }
    return 0;
}


/*
    Get the current write position offset
    function get writePosition(): Number
 */
static EjsNumber *ba_writePosition(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) ap->writePosition);
}


/*
    Set the current write position offset
    function set writePosition(position: Number): Void
 */
static EjsObj *ba_setWritePosition(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv)
{
    int     pos;

    mprAssert(argc == 1 && ejsIs(ejs, argv[0], Number));

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
        ejsSendEvent(ejs, ap->emitter, "flush", NULL, ap);
    }
    ap->writePosition = ap->readPosition = 0;
    if (ap->emitter) {
        ejsSendEvent(ejs, ap->emitter, "writable", NULL, ap);
    }
    return 0;
}


/*
    Grow the byte array up to the given length, but not over the maximum. Return the length or an error code.
    This routine always throws an exception.
 */
ssize ejsGrowByteArray(Ejs *ejs, EjsByteArray *ap, ssize len)
{
    if (len > ap->length) {
        if (!ap->resizable) {
            ejsThrowResourceError(ejs, "Byte array is too small. Need room for %d bytes.", len);
            return EJS_ERR;
        }
        if ((ap->value = mprRealloc(ap->value, len)) == 0) {
            ejsThrowMemoryError(ejs);
            return EJS_ERR;
        }
        memset(&ap->value[ap->length], 0, len - ap->length);
        ap->growInc = min(ap->growInc * 2, 32 * 1024);
        ap->length = len;
    }
    return ap->length;
}


/*
    Get more input sufficient to satisfy the rquired number of bytes. The required parameter specifies how many bytes 
    must be read. Short fills are not permitted. Return the count of bytes available or 0 if the required number of 
    bytes can't be read. Return -ve on errors.
 */
static ssize getInput(Ejs *ejs, EjsByteArray *ap, ssize required)
{
    if (availableBytes(ap) == 0) {
        ap->writePosition = ap->readPosition = 0;
    }
    if (availableBytes(ap) < required && !ejs->exception) {
        if (ap->emitter) {
            ejsSendEvent(ejs, ap->emitter, "writable", NULL, ap);
        }
    }
    if (availableBytes(ap) < required) {
        return 0;
    }
    return availableBytes(ap);
}


bool ejsMakeRoomInByteArray(Ejs *ejs, EjsByteArray *ap, ssize require)
{
    ssize   newLen;

    /*
        MOB - should this do ejsResetByteArray if empty
     */
    if (room(ap) < require) {
        if (ap->emitter && availableBytes(ap)) {
            ejsSendEvent(ejs, ap->emitter, "readable", NULL, ap);
        }
        if (room(ap) < require) {
            newLen = max(ap->length + require, ap->length + ap->growInc);
            if (ejsGrowByteArray(ejs, ap, newLen) < 0) {
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


static int putString(EjsByteArray *ap, EjsString *str, ssize len)
{
    //  TODO -- this must do encoding
    mprMemcpy(&ap->value[ap->writePosition], room(ap), str->value, len);
    ap->writePosition += len;
    return (int) len;
}

/********************************* Public Support API *****************************/

void ejsResetByteArray(Ejs *ejs, EjsByteArray *ba)
{
    if (ba->writePosition == ba->readPosition) {
        ba->writePosition = ba->readPosition = 0;
    }
}


void ejsSetByteArrayPositions(Ejs *ejs, EjsByteArray *ba, ssize readPosition, ssize writePosition)
{
    if (readPosition >= 0) {
        ba->readPosition = readPosition;
    }
    if (writePosition >= 0) {
        ba->writePosition = writePosition;
    }
}


ssize ejsCopyToByteArray(Ejs *ejs, EjsByteArray *ba, ssize offset, cchar *data, ssize length)
{
    int     i;

    mprAssert(ba);
    mprAssert(data);

    if (offset < 0) {
        offset = ba->writePosition;
    }
    if (!ejsMakeRoomInByteArray(ejs, ba, offset + length)) {
        return EJS_ERR;
    }
    if (ba->length < (offset + length)) {
        return EJS_ERR;
    }
    for (i = 0; i < length; i++) {
        ba->value[offset++] = data[i];
    }
    return length;
}


ssize ejsGetByteArrayAvailableData(EjsByteArray *ba)
{
    return availableBytes(ba);
}


ssize ejsGetByteArrayRoom(EjsByteArray *ba)
{
    return room(ba);
}

/*********************************** Factory **********************************/

EjsByteArray *ejsCreateByteArray(Ejs *ejs, ssize size)
{
    EjsByteArray    *ap;

    /*
        No need to invoke constructor
     */
    ap = ejsCreateObj(ejs, ESV(ByteArray), 0);
    if (ap == 0) {
        return 0;
    }
    if (size <= 0) {
        size = MPR_BUFSIZE;
    }
    ap->async = -1;
    ap->resizable = 1;
    ap->growInc = MPR_BUFSIZE;
    ap->endian = mprGetEndian(ejs);
    if (ejsGrowByteArray(ejs, ap, size) < 0) {
        return 0;
    }
    return ap;
}


static void manageByteArray(EjsByteArray *ap, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ap->emitter);
        mprMark(ap->value);
        mprMark(TYPE(ap));
    }
}


void ejsConfigureByteArrayType(Ejs *ejs)
{
    EjsType     *type;
    EjsHelpers  *helpers;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "ByteArray"), sizeof(EjsByteArray), manageByteArray, 
            EJS_TYPE_OBJ | EJS_TYPE_NUMERIC_INDICIES | EJS_TYPE_VIRTUAL_SLOTS | EJS_TYPE_MUTABLE_INSTANCES)) == 0) {
        return;
    }
    helpers = &type->helpers;
    helpers->cast = (EjsCastHelper) castByteArrayVar;
    helpers->clone = (EjsCloneHelper) cloneByteArrayVar;
    helpers->deleteProperty = (EjsDeletePropertyHelper) deleteByteArrayProperty;
    helpers->getProperty = (EjsGetPropertyHelper) getByteArrayProperty;
    helpers->getPropertyCount = (EjsGetPropertyCountHelper) getByteArrayPropertyCount;
    helpers->invokeOperator = (EjsInvokeOperatorHelper) invokeByteArrayOperator;
    helpers->lookupProperty = (EjsLookupPropertyHelper) lookupByteArrayProperty;
    helpers->setProperty = (EjsSetPropertyHelper) setByteArrayProperty;
    
    prototype = type->prototype;
    ejsBindConstructor(ejs, type, ba_ByteArray);
    ejsBindMethod(ejs, prototype, ES_ByteArray_on, ba_on);
    ejsBindMethod(ejs, prototype, ES_ByteArray_available, ba_available);
    ejsBindAccess(ejs, prototype, ES_ByteArray_async, ba_async, ba_setAsync);
    ejsBindMethod(ejs, prototype, ES_ByteArray_close, ba_close);
    ejsBindMethod(ejs, prototype, ES_ByteArray_compact, ba_compact);
    ejsBindMethod(ejs, prototype, ES_ByteArray_copyIn, ba_copyIn);
    ejsBindMethod(ejs, prototype, ES_ByteArray_copyOut, ba_copyOut);
    ejsBindAccess(ejs, prototype, ES_ByteArray_endian, endian, setEndian);
    ejsBindMethod(ejs, prototype, ES_ByteArray_flush, ba_flush);
    ejsBindMethod(ejs, prototype, ES_ByteArray_resizable, ba_resizable);
    ejsBindMethod(ejs, prototype, ES_ByteArray_length, ba_getLength);
    ejsBindMethod(ejs, prototype, ES_ByteArray_iterator_get, ba_get);
    ejsBindMethod(ejs, prototype, ES_ByteArray_iterator_getValues, ba_getValues);
    ejsBindMethod(ejs, prototype, ES_ByteArray_off, ba_off);
    ejsBindMethod(ejs, prototype, ES_ByteArray_read, ba_read);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readBoolean, ba_readBoolean);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readByte, ba_readByte);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readDate, ba_readDate);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readDouble, ba_readDouble);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readInteger, ba_readInteger);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readLong, ba_readLong);
    ejsBindAccess(ejs, prototype, ES_ByteArray_readPosition, ba_readPosition,ba_setReadPosition);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readShort, ba_readShort);
    ejsBindMethod(ejs, prototype, ES_ByteArray_readString, ba_readString);
    ejsBindMethod(ejs, prototype, ES_ByteArray_reset, ba_reset);
    ejsBindMethod(ejs, prototype, ES_ByteArray_room, ba_room);
    ejsBindMethod(ejs, prototype, ES_ByteArray_toString, ba_toString);
    ejsBindMethod(ejs, prototype, ES_ByteArray_write, ejsWriteToByteArray);
    ejsBindMethod(ejs, prototype, ES_ByteArray_writeByte, ba_writeByte);
    ejsBindMethod(ejs, prototype, ES_ByteArray_writeShort, ba_writeShort);
    ejsBindMethod(ejs, prototype, ES_ByteArray_writeInteger, ba_writeInteger);
    ejsBindMethod(ejs, prototype, ES_ByteArray_writeLong, ba_writeLong);
    ejsBindMethod(ejs, prototype, ES_ByteArray_writeDouble, ba_writeDouble);
    ejsBindAccess(ejs, prototype, ES_ByteArray_writePosition, ba_writePosition, ba_setWritePosition);
}


/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
  
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.
  
    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://embedthis.com/downloads/gplLicense.html
  
    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com
  
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
