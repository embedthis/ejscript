/*
    ejsInterp.c - Virtual Machine Interpreter for Ejscript.

    Copyright (c) All Rights Reserved. See details at the end of the file.

    NEXT
        - Optimize and cache stack.top
        - Fix BAD BINDING
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/********************************** Inline Code *******************************/
/*
    The stack is a stack of pointers to values. The top of stack (stack.top) always points to the current top item 
    on the stack. To push a new value, top is incremented then the value is stored. To pop, simply copy the value at 
    top and decrement top ptr.
 */
#define top                     (*state->stack)

//  TODO - ejs arg not used
#define pop(ejs)                (*state->stack--)

#define push(value)             (*(++(state->stack))) = ((EjsObj*) (value))
#define popString(ejs)          ((EjsString*) pop(ejs))
#define popOutside(ejs)         *(ejs->state->stack)--
#define pushOutside(ejs, value) (*(++(ejs->state->stack))) = ((EjsObj*) (value))

#define FRAME                   state->fp
#define FUNCTION                state->fp.function
#define BLOCK                   state->bp

#define SWAP if (1) { \
        EjsObj *swap = state->stack[0]; \
        state->stack[0] = state->stack[-1]; \
        state->stack[-1] = swap; \
    }

static void callFunction(Ejs *ejs, EjsFunction *fun, EjsAny *thisObj, int argc, int stackAdjust);

static BIT_INLINE void getPropertyFromSlot(Ejs *ejs, EjsAny *thisObj, EjsAny *obj, int slotNum) 
{
    EjsFunction     *fun, *value;

    if (ejsPropertyHasTrait(ejs, obj, slotNum, EJS_TRAIT_GETTER)) {
        fun = ejsGetProperty(ejs, obj, slotNum);
        callFunction(ejs, fun, thisObj, 0, 0);
        if (ejsIsNativeFunction(ejs, fun)) {
            pushOutside(ejs, ejs->result);
        } else {
            ejs->state->fp->getter = 1;
        }
        return;
    }
    value = ejsGetProperty(ejs, obj, slotNum);
    if (ejsIsFunction(ejs, value)) {
        fun = (EjsFunction*) value;
        if (!fun->boundThis && thisObj) {
            /* Function extraction. Bind the "thisObj" into a clone of the function */
            fun = ejsCloneFunction(ejs, fun, 0);
            fun->boundThis = thisObj;
            assert(fun->boundThis != ejs->global);
        }
    }
    pushOutside(ejs, value);
}

#define GET_SLOT(thisObj, obj, slotNum) getPropertyFromSlot(ejs, thisObj, obj, slotNum)

static BIT_INLINE void checkGetter(Ejs *ejs, EjsAny *value, EjsAny *thisObj, EjsAny *obj, int slotNum) 
{
    EjsFunction     *fun;

    if (ejsIsFunction(ejs, value) && !ejsIsType(ejs, value)) {
        fun = (EjsFunction*) value;
        if (ejsPropertyHasTrait(ejs, obj, slotNum, EJS_TRAIT_GETTER)) {
            if (fun->staticMethod) {
                thisObj = obj;
            }
            callFunction(ejs, fun, thisObj, 0, 0);
            if (ejsIsNativeFunction(ejs, fun)) {
                pushOutside(ejs, ejs->result);
            } else {
                ejs->state->fp->getter = 1;
            }
            return;
        } else {
            if (!fun->boundThis && thisObj && thisObj != ejs->global) {
                /* Function extraction. Bind the "thisObj" into a clone of the function */
                /* OPT - this is slow in the a case: a.b.fn */
                fun = ejsCloneFunction(ejs, fun, 0);
                fun->boundThis = thisObj;
                assert(fun->boundThis != ejs->global);
                value = fun;
            }
        }
    } else if (value == 0) {
        value = ESV(undefined);
    }
    pushOutside(ejs, value);
}

#define CHECK_VALUE(value, thisObj, obj, slotNum) checkGetter(ejs, value, thisObj, obj, slotNum)
#define CHECK_GC() if (MPR->heap->mustYield && !(ejs->state->paused)) { mprYield(0); } else 

/*
    Set a slot value when we don't know if the object is an EjsObj
 */
#define SET_SLOT(thisObj, obj, slotNum, value) storePropertyToSlot(ejs, thisObj, obj, slotNum, value)

/*
    Must clear attentionPc when changing the PC. Otherwise the next instruction will jump to a bad (stale) location.
 */
//  TODO -- should not need to clear attentionPc
#define SET_PC(fp, value) \
    if (1) { \
        (fp)->pc = (uchar*) (value); \
        (fp)->attentionPc = 0; \
    } else 

#define GET_BYTE()      *(FRAME)->pc++
#define GET_DOUBLE()    ejsDecodeDouble(ejs, &(FRAME)->pc)
#define GET_INT()       ((int) GET_NUM())

//  TODO OPT - returns 64 bits, but most cases only need 32 bits
#define GET_NUM()       ejsDecodeNum(ejs, &(FRAME)->pc)
#define GET_NAME()      getNameArg(ejs, FRAME)
#define GET_STRING()    getStringArg(ejs, FRAME)
#define GET_TYPE()      ((EjsType*) getGlobalArg(ejs, FRAME))
#define GET_WORD()      ejsDecodeInt32(ejs, &(FRAME)->pc)
#undef THIS
#define THIS            FRAME->function.boundThis
#define FILL(mark)      while (mark < FRAME->pc) { *mark++ = EJS_OP_NOP; }

// #define DEBUG_IDE 1
#if DEBUG_IDE
    static EjsOpCode traceCode(Ejs *ejs, EjsOpCode opcode);
    static int opcount[256];
#else
    #define traceCode(ejs, opcode) opcode
#endif

#if BIT_UNIX_LIKE || (VXWORKS && !BIT_DIAB)
    #define CASE(opcode) opcode
    #define BREAK goto *opcodeJump[opcode = traceCode(ejs, GET_BYTE())]
#else
    /*
        Traditional switch for compilers (looking at you MS) without computed goto.
     */
    #define BREAK break
    #define CASE(opcode) case opcode
#endif

/******************************** Forward Declarations ************************/

static void callInterfaceInitializers(Ejs *ejs, EjsType *type);
static void callProperty(Ejs *ejs, EjsAny *obj, int slotNum, EjsAny *thisObj, int argc, int stackAdjust);
static void checkExceptionHandlers(Ejs *ejs);
static void createExceptionBlock(Ejs *ejs, EjsEx *ex, int flags);
static EjsAny *evalBinaryExpr(Ejs *ejs, EjsAny *lhs, EjsOpCode opcode, EjsAny *rhs);
static uint findEndException(Ejs *ejs);
static EjsEx *findExceptionHandler(Ejs *ejs, int kind);
static EjsName getNameArg(Ejs *ejs, EjsFrame *fp);
static EjsAny *getNthBase(Ejs *ejs, EjsAny *obj, int nthBase);
static EjsAny *getNthBaseFromBottom(Ejs *ejs, EjsAny *obj, int nthBase);
static EjsAny *getNthBlock(Ejs *ejs, int nth);
static EjsString *getString(Ejs *ejs, EjsFrame *fp, int num);
static EjsString *getStringArg(Ejs *ejs, EjsFrame *fp);
static EjsObj *getGlobalArg(Ejs *ejs, EjsFrame *fp);
static EjsBlock *popExceptionBlock(Ejs *ejs);
static bool processException(Ejs *ejs);
static void storeProperty(Ejs *ejs, EjsObj *thisObj, EjsAny *obj, EjsName name, EjsObj *value);
static void storePropertyToSlot(Ejs *ejs, EjsObj *thisObj, EjsAny *obj, int slotNum, EjsObj *value);
static void storePropertyToScope(Ejs *ejs, EjsName qname, EjsObj *value);
static void throwNull(Ejs *ejs);

/************************************* Code ***********************************/
/*
    Virtual Machine byte code evaluation
 */
static void VM(Ejs *ejs, EjsFunction *fun, EjsAny *otherThis, int argc, int stackAdjust)
{
    EjsName     qname;
    EjsObj      *result, *vp, *v1, *v2, *obj, *value;
    int         slotNum, nthBase;
    EjsState    *state;
    EjsBlock    *blk;
    EjsObj      *global;
    EjsObj      *vobj, *thisObj;
    EjsString   *nameVar, *spaceVar;
    EjsNumber   *indexVar;
    EjsType     *type;
    EjsLookup   lookup;
    EjsEx       *ex;
    EjsFrame    *newFrame;
    EjsFunction *f1, *f2;
    EjsNamespace *nsp;
    EjsString   *str;
    int         i, offset, count, opcode, attributes, paused;

#if BIT_UNIX_LIKE || (VXWORKS && !BIT_DIAB)
    /*
        Direct threading computed goto processing. Include computed goto jump table.
     */
    #include    "ejsByteGoto.h"
#endif
    assert(ejs);
    assert(!mprHasMemError(ejs));
    assert(!ejs->exception);
    assert(ejs->state->fp == 0 || ejs->state->fp->attentionPc == 0);

    vp = 0;
    slotNum = -1;
    global = ejs->global;

    state = mprAlloc(sizeof(EjsState));
    *state = *ejs->state;
    state->prev = ejs->state;
    state->paused = ejs->state->paused;
    ejs->state = state;

    callFunction(ejs, fun, otherThis, argc, stackAdjust);
    assert(state->fp);
    FRAME->caller = 0;

#if BIT_UNIX_LIKE || (VXWORKS && !BIT_DIAB)
    /*
        Direct threading computed goto processing. Include computed goto jump table.
     */
    BREAK;
#else
    /*
        Traditional switch for compilers (looking at you MS) without computed goto.
     */
    while (1) {
        opcode = (EjsOpCode) GET_BYTE();
        traceCode(ejs, opcode);
        switch (opcode) {
#endif
        /*
            End of a code block. Used to mark the end of a script. Saves testing end of code block in VM loop.
                EndCode
         */
        CASE (EJS_OP_END_CODE):
            /*
                The "ejs" command needs to preserve the current ejs->result for interactive sessions.
             */
            if (ejs->result == 0) {
                // OPT - remove this
                ejs->result = ESV(undefined);
            }
            if (FRAME->getter) {
                push(ejs->result);
            }
            FRAME = 0;
            goto done;

        /*
            Return from a function with a result
                ReturnValue
                Stack before (top)  [value]
                Stack after         []
         */
        CASE (EJS_OP_RETURN_VALUE):
            ejs->result = pop(ejs);
            if (FRAME->caller == 0) {
                goto done;
            }
            state->stack = FRAME->stackReturn;
            if (ejs->result) {
                f1 = &FRAME->function;
                if (FRAME->function.resultType) {
                    type = FRAME->function.resultType;
                    //  TODO remove this voidType
                    if (type != EST(Void) && !ejsIsA(ejs, ejs->result, type)) {
                        if (ejs->result == ESV(null) || ejs->result == ESV(undefined)) {
                            if (FRAME->function.throwNulls) {
                                ejsThrowTypeError(ejs, "Unacceptable null or undefined return value");
                                BREAK;
                            } else if (FRAME->function.castNulls) {
                                ejs->result = ejsCastType(ejs, ejs->result, type);
                                if (ejs->exception) {
                                    BREAK;
                                }
                            }
                        } else {
                            ejs->result = ejsCastType(ejs, ejs->result, type);
                            if (ejs->exception) {
                                BREAK;
                            }
                        }
                    }
                }
            }
            if (FRAME->getter) {
                push(ejs->result);
            }
            state->bp = FRAME->function.block.prev;
            newFrame = FRAME->caller;
            FRAME = newFrame;
            CHECK_GC();
            BREAK;

        /*
            Return from a function without a result
                Return
         */
        CASE (EJS_OP_RETURN):
            ejs->result = ESV(undefined);
            if (FRAME->caller == 0) {
                goto done;
            }
            state->stack = FRAME->stackReturn;
            state->bp = FRAME->function.block.prev;
            newFrame = FRAME->caller;
            FRAME = newFrame;
            CHECK_GC();
            BREAK;

        /*
            Load the catch argument
                PushCatchArg
                Stack before (top)  []
                Stack after         [catchArg]
         */
        CASE (EJS_OP_PUSH_CATCH_ARG):
            push(ejs->exceptionArg);
            ejs->exceptionArg = 0;
            BREAK;

        /*
            Push the function call result
                 PushResult
                Stack before (top)  []
                Stack after         [result]
         */
        CASE (EJS_OP_PUSH_RESULT):
            push(ejs->result);
            BREAK;

        /*
            Save the top of stack and store in the interpreter result register
                SaveResult
                Stack before (top)  [value]
                Stack after         []
         */
        CASE (EJS_OP_SAVE_RESULT):
            ejs->result = pop(ejs);
            BREAK;

        /* Load Constants ----------------------------------------------- */

        /*
            Load a float constant
                LoadDouble          <double>
                Stack before (top)  []
                Stack after         [Double]
         */
        CASE (EJS_OP_LOAD_DOUBLE):
            push(ejsCreateNumber(ejs, GET_DOUBLE()));
            BREAK;

        /*
            Load a signed integer constant (up to 55 bits worth of data)
                LoadInt.64          <int64>
                Stack before (top)  []
                Stack after         [Number]
         */
        CASE (EJS_OP_LOAD_INT):
            push(ejsCreateNumber(ejs, (MprNumber) GET_NUM()));
            BREAK;

        /*
            Load integer constant between 0 and 9
                Load0, Load1, ... Load9
                Stack before (top)  []
                Stack after         [Number]
         */
        CASE (EJS_OP_LOAD_0):
        CASE (EJS_OP_LOAD_1):
        CASE (EJS_OP_LOAD_2):
        CASE (EJS_OP_LOAD_3):
        CASE (EJS_OP_LOAD_4):
        CASE (EJS_OP_LOAD_5):
        CASE (EJS_OP_LOAD_6):
        CASE (EJS_OP_LOAD_7):
        CASE (EJS_OP_LOAD_8):
        CASE (EJS_OP_LOAD_9):
            push(ejsCreateNumber(ejs, opcode - EJS_OP_LOAD_0));
            BREAK;

        /*
            Load the -1 integer constant
                LoadMinusOne
                Stack before (top)  []
                Stack after         [Number]
         */
        CASE (EJS_OP_LOAD_M1):
            push(ejsCreateNumber(ejs, -1));
            BREAK;

        /*
            Load a string constant
                LoadString          <string>
                Stack before (top)  []
                Stack after         [String]
         */
        CASE (EJS_OP_LOAD_STRING):
            push(GET_STRING());
            BREAK;

        /*
            Load a namespace constant
                LoadNamespace       <UriString>
                Stack before (top)  []
                Stack after         [Namespace]
         */
        CASE (EJS_OP_LOAD_NAMESPACE):
            str = GET_STRING();
            push(ejsCreateNamespace(ejs, str));
            BREAK;

        /*
            Load an XML constant
                LoadXML             <xmlString>
                Stack before (top)  []
                Stack after         [XML]
         */
        CASE (EJS_OP_LOAD_XML):
            v1 = ejsCreateObj(ejs, EST(XML), 0);
            str = GET_STRING();
            ejsLoadXMLString(ejs, (EjsXML*) v1, str);
            push(v1);
            BREAK;

        /*
            Load a Regexp constant
                LoadRegExp
                Stack before (top)  []
                Stack after         [RegExp]
         */
        CASE (EJS_OP_LOAD_REGEXP):
#if BIT_PACK_PCRE
            str = GET_STRING();
            v1 = (EjsObj*) ejsParseRegExp(ejs, str);
            push(v1);
#else
            ejsThrowReferenceError(ejs, "RegularExpression support was not included in the build");
#endif
            BREAK;

        /*
            Load a null constant
                LoadNull
                Stack before (top)  []
                Stack after         [Null]
         */
        CASE (EJS_OP_LOAD_NULL):
            push(ESV(null));
            BREAK;

        /*
            Load a void / undefined constant
                LoadUndefined
                Stack before (top)  []
                Stack after         [undefined]
         */
        CASE (EJS_OP_LOAD_UNDEFINED):
            push(ESV(undefined));
            BREAK;

        CASE (EJS_OP_LOAD_THIS):
            push(THIS);
            BREAK;

        CASE (EJS_OP_LOAD_THIS_LOOKUP):
            if (lookup.originalObj) {
                push(lookup.originalObj);
            } else {
                obj = FRAME->function.moduleInitializer ? ejs->global : (EjsObj*) FRAME;
                push(obj);
            }
            BREAK;

        /*
            Load the nth base class of "this"
                LoadThis
                Stack before (top)  []
                Stack after         [baseClass]
         */
        CASE (EJS_OP_LOAD_THIS_BASE):
            vp = getNthBase(ejs, THIS, GET_INT());
            push(vp);
            BREAK;

        /*
            Load the "global" value
                LoadGlobal
                Stack before (tp (op)  []
                Stack after         [global]
         */
        CASE (EJS_OP_LOAD_GLOBAL):
            push(ejs->global);
            BREAK;

        /*
            Load the "true" value
                LoadTrue
                Stack before (top)  []
                Stack after         [true]
         */
        CASE (EJS_OP_LOAD_TRUE):
            push(ESV(true));
            BREAK;

        /*
            Load the "false" value
                LoadFalse
                Stack before (top)  []
                Stack after         [false]
         */
        CASE (EJS_OP_LOAD_FALSE):
            push(ESV(false));
            BREAK;

        /*
            Load a global variable by slot number
                GetGlobalSlot       <slot>
                Stack before (top)  []
                Stack after         [PropRef]
         */
        CASE (EJS_OP_GET_GLOBAL_SLOT):
            GET_SLOT(NULL, global, GET_INT());
            BREAK;

        /*
            Load a local variable by slot number
                GetLocalSlot        <slot>
                Stack before (top)  []
                Stack after         [PropRef]
         */
        CASE (EJS_OP_GET_LOCAL_SLOT):
            GET_SLOT(NULL, FRAME, GET_INT());
            BREAK;

        /*
            Load a local variable in slot 0-9
                GetLocalSlot0, GetLocalSlot1, ... GetLocalSlot9
                Stack before (top)  []
                Stack after         [PropRef]
         */
        CASE (EJS_OP_GET_LOCAL_SLOT_0):
        CASE (EJS_OP_GET_LOCAL_SLOT_1):
        CASE (EJS_OP_GET_LOCAL_SLOT_2):
        CASE (EJS_OP_GET_LOCAL_SLOT_3):
        CASE (EJS_OP_GET_LOCAL_SLOT_4):
        CASE (EJS_OP_GET_LOCAL_SLOT_5):
        CASE (EJS_OP_GET_LOCAL_SLOT_6):
        CASE (EJS_OP_GET_LOCAL_SLOT_7):
        CASE (EJS_OP_GET_LOCAL_SLOT_8):
        CASE (EJS_OP_GET_LOCAL_SLOT_9):
            GET_SLOT(NULL, FRAME, opcode - EJS_OP_GET_LOCAL_SLOT_0);
            BREAK;

        /*
            Load a block scoped variable by slot number
                GetBlockSlot        <slot> <nthBlock>
                Stack before (top)  []
                Stack after         [value]
         */
        CASE (EJS_OP_GET_BLOCK_SLOT):
            slotNum = GET_INT();
            obj = getNthBlock(ejs, GET_INT());
            GET_SLOT(NULL, obj, slotNum);
            BREAK;

        /*
            Load a property in thisObj by slot number
                GetThisSlot         <slot>
                Stack before (top)  []
                Stack after         [value]
         */
        CASE (EJS_OP_GET_THIS_SLOT):
            GET_SLOT(THIS, THIS, GET_INT());
            BREAK;

        /*
            Load a property in slot 0-9
                GetThisSlot0, GetThisSlot1,  ... GetThisSlot9
                Stack before (top)  []
                Stack after         [value]
         */
        CASE (EJS_OP_GET_THIS_SLOT_0):
        CASE (EJS_OP_GET_THIS_SLOT_1):
        CASE (EJS_OP_GET_THIS_SLOT_2):
        CASE (EJS_OP_GET_THIS_SLOT_3):
        CASE (EJS_OP_GET_THIS_SLOT_4):
        CASE (EJS_OP_GET_THIS_SLOT_5):
        CASE (EJS_OP_GET_THIS_SLOT_6):
        CASE (EJS_OP_GET_THIS_SLOT_7):
        CASE (EJS_OP_GET_THIS_SLOT_8):
        CASE (EJS_OP_GET_THIS_SLOT_9):
            GET_SLOT(THIS, THIS, opcode - EJS_OP_GET_THIS_SLOT_0);
            BREAK;

        /*
            Load a property in an object by slot number
                GetObjSlot          <slot>
                Stack before (top)  [obj]
                Stack after         [value]
         */
        CASE (EJS_OP_GET_OBJ_SLOT):
            vp = pop(ejs);
            GET_SLOT(vp, vp, GET_INT());
            BREAK;

        /*
            Load a property in an object from slot 0-9
                GetObjSlot0, GetObjSlot1, ... GetObjSlot9
                Stack before (top)  [obj]
                Stack after         [value]
         */
        CASE (EJS_OP_GET_OBJ_SLOT_0):
        CASE (EJS_OP_GET_OBJ_SLOT_1):
        CASE (EJS_OP_GET_OBJ_SLOT_2):
        CASE (EJS_OP_GET_OBJ_SLOT_3):
        CASE (EJS_OP_GET_OBJ_SLOT_4):
        CASE (EJS_OP_GET_OBJ_SLOT_5):
        CASE (EJS_OP_GET_OBJ_SLOT_6):
        CASE (EJS_OP_GET_OBJ_SLOT_7):
        CASE (EJS_OP_GET_OBJ_SLOT_8):
        CASE (EJS_OP_GET_OBJ_SLOT_9):
            vp = pop(ejs);
            GET_SLOT(vp, vp, opcode - EJS_OP_GET_OBJ_SLOT_0);
            BREAK;

        /*
            Load a variable from a type by slot number
                GetTypeSlot         <slot> <nthBase>
                Stack before (top)  [objRef]
                Stack after         [value]
         */
        CASE (EJS_OP_GET_TYPE_SLOT):
            slotNum = GET_INT();
            thisObj = pop(ejs);
            vp = getNthBase(ejs, thisObj, GET_INT());
            GET_SLOT(thisObj, vp, slotNum);
            BREAK;

        /*
            Load a type variable by slot number from this. NthBase counts from Object up rather than "this" down.
                GetThisTypeSlot     <slot> <nthBaseFromBottom>
                Stack before (top)  []
                Stack after         [value]
         */
        CASE (EJS_OP_GET_THIS_TYPE_SLOT):
            slotNum = GET_INT();
            type = (EjsType*) getNthBaseFromBottom(ejs, THIS, GET_INT());
            if (type == 0) {
                ejsThrowReferenceError(ejs, "Bad base class reference");
            } else {
                GET_SLOT(THIS, type, slotNum);
            }
            BREAK;

        /*
            Load a variable by an unqualified name
                GetScopedName       <qname>
                Stack before (top)  []
                Stack after         [value]
         */
        CASE (EJS_OP_GET_SCOPED_NAME):
            qname = GET_NAME();
            vp = ejsGetVarByName(ejs, NULL, qname, &lookup);
            if (unlikely(vp == 0)) {
                vp = ejsGetVarByName(ejs, NULL, qname, &lookup);
                ejsThrowReferenceError(ejs, "%@ is not defined", qname.name);
            } else {
                CHECK_VALUE(vp, NULL, lookup.obj, lookup.slotNum);
            }
            BREAK;
                
        /*
            Load a variable by an unqualified name expression
                GetScopedNameExpr
                Stack before (top)  [name]
                                    [space]
                Stack after         [value]
         */
        CASE (EJS_OP_GET_SCOPED_NAME_EXPR):
            qname.name = ejsToString(ejs, pop(ejs));
            v1 = pop(ejs);
            if (ejsIs(ejs, v1, Namespace)) {
                qname.space = ((EjsNamespace*) v1)->value;
            } else {
                qname.space = ejsToString(ejs, v1);
            }
            vp = ejsGetVarByName(ejs, NULL, qname, &lookup);
            if (unlikely(vp == 0)) {
                push(ESV(undefined));
            } else {
                CHECK_VALUE(vp, NULL, lookup.obj, lookup.slotNum);
            }
#if DYNAMIC_BINDING
            if (ejs->flags & EJS_FLAG_COMPILER || TYPE(lookup.obj) == EST(Object) || lookup.slotNum >= 4096) {
                BREAK;
            }
            if (lookup.obj == ejs->global) {
                *mark++ = EJS_OP_GET_GLOBAL_SLOT;
                mark += ejsEncodeUint(mark, lookup.slotNum);

            } else if (lookup->obj == state->fp) {
                *mark++ = EJS_OP_GET_LOCAL_SLOT;
                mark += ejsEncodeUint(mark, lookup.slotNum);

            } else if (lookup->obj == state->fp->thisObj) {
                *mark++ = EJS_OP_GET_THIS_SLOT;
                mark += ejsEncodeUint(mark, lookup.slotNum);

            } else if (ejsIsA(ejs, THIS, (EjsType*) lookup.obj)) {
                *mark++ = EJS_OP_GET_BLOCK_SLOT;
                mark += ejsEncodeUint(mark, lookup.slotNum);
                mark += ejsEncodeUint(mark, lookup.nthBlock);

            } else {
                BREAK;
            }
            FILL(mark);
#endif
            BREAK;
                
        /*
            Load a property by property name
                GetObjName          <qname>
                Stack before (top)  [obj]
                Stack after         [result]
         */
        CASE (EJS_OP_GET_OBJ_NAME):
#if DYNAMIC_BINDING
            mark = FRAME->pc - 1;
#endif
            qname = GET_NAME();
            vp = pop(ejs);
            if (vp == ESV(null) || vp == ESV(undefined)) {
                ejsThrowReferenceError(ejs, "Object reference is null");
                BREAK;
            }
            v1 = ejsGetVarByName(ejs, vp, qname, &lookup);
            CHECK_VALUE(v1, vp, lookup.obj, lookup.slotNum);
#if DYNAMIC_BINDING
            if (lookup.slotNum < 0 || lookup.slotNum > 4096 || ejs->flags & EJS_FLAG_COMPILER) {
                BREAK;
            }
            if (lookup.obj == ejs->global) {
                *mark++ = EJS_OP_GET_GLOBAL_SLOT;
                mark += ejsEncodeUint(mark, lookup.slotNum);

            } else if (lookup.obj == (EjsObj*) state.fp) {
                *mark++ = EJS_OP_GET_LOCAL_SLOT;
                mark += ejsEncodeUint(mark, lookup.slotNum);

            } else if (lookup.obj == state.fp->thisObj) {
                *mark++ = EJS_OP_GET_THIS_SLOT;
                mark += ejsEncodeUint(mark, lookup.slotNum);
                
            } else if (ejsIsType(ejs, lookup.obj) && ejsIsA(ejs, THIS, (EjsType*) lookup.obj)) {
                *mark++ = EJS_OP_GET_TYPE_SLOT;
                mark += ejsEncodeUint(mark, lookup.slotNum);
                mark += ejsEncodeUint(mark, lookup.nthBlock + 1);

            } else if ((EjsObj*) TYPE(vp) == lookup.obj) {
                *mark++  = EJS_OP_GET_TYPE_SLOT;
                mark += ejsEncodeUint(mark, lookup.slotNum);
                mark += ejsEncodeUint(mark, lookup.nthBase);

            } else {
                *mark++  = EJS_OP_GET_OBJ_SLOT;
                mark += ejsEncodeUint(mark, lookup.slotNum);
            }
            FILL(mark);
#endif
            BREAK;

#if XXXX
        CASE (EJS_OP_GET_POLY_SLOT):
            vp = pop(ejs);
            slotNum = GET_UINT32();
            type = GET_PTR();
            if (TYPE(vp) != type) {
                GET_SLOT(thisObj, vp, slotNum);
            } else {
                v1 = ejsGetVarByName(ejs, vp, qname, &lookup);
                CHECK_VALUE(v1, vp, lookup.obj, lookup.slotNum);
            }

        /*
            Load a property by property name
                GetObjName          <qname>
                Stack before (top)  [obj]
                Stack after         [result]
         */
        CASE (EJS_OP_GET_OBJ_NAME):
            mark = FRAME->pc - 1;
            qname = GET_NAME();
            vp = pop(ejs);
            if (vp == ESV(null) || vp == ESV(undefined)) {
                ejsThrowReferenceError(ejs, "Object reference is null");
                BREAK;
            }
            v1 = ejsGetVarByName(ejs, vp, qname, &lookup);
            CHECK_VALUE(v1, vp, lookup.obj, lookup.slotNum);

            if (v1 && lookup.obj == vp) {
                *mark++ = EJS_OP_GET_POLY_SLOT;
                *mark++ = lookup.slotNum;
                uint *ui = (uint*) mark;
                *ui++ = lookup.
                uint *ui = (uint*) FRAME->pc;
                *ui = ejsEncodeUint(mark, lookup.slotNum);
            }
            FILL(mark);
            BREAK;
#endif

        /*
            Load a property by property a qualified name expression
                GetObjNameExpr
                Stack before (top)  [name]
                                    [space]
                                    [obj]
                Stack after         [value]
         */
        CASE (EJS_OP_GET_OBJ_NAME_EXPR):
            v1 = pop(ejs);
            v2 = pop(ejs);
            vp = pop(ejs);
            if (vp == 0 || vp == ESV(null) || vp == ESV(undefined)) {
                ejsThrowReferenceError(ejs, "Object reference is null");
                BREAK;
            }
            if (TYPE(vp)->numericIndicies && ejsIs(ejs, v1, Number)) {
                vp = ejsGetProperty(ejs, vp, ejsGetInt(ejs, v1));
                push(vp == 0 ? ESV(null) : vp);
                BREAK;
            } else {
                qname.name = ejsToString(ejs, v1);
                if (ejsIs(ejs, v2, Namespace)) {
                    qname.space = ((EjsNamespace*) v2)->value;
                } else {
                    qname.space = ejsToString(ejs, v2);
                }
                v2 = ejsGetVarByName(ejs, vp, qname, &lookup);
                CHECK_VALUE(v2, NULL, lookup.obj, lookup.slotNum);
                BREAK;
            }


        /* Store ------------------------------- */

        /*
            Store a global variable by slot number
                Stack before (top)  [value]
                Stack after         []
                PutGlobalSlot       <slot>
         */
        CASE (EJS_OP_PUT_GLOBAL_SLOT):
            SET_SLOT(NULL, global, GET_INT(), pop(ejs));
            BREAK;

        /*
            Store a local variable by slot number
                Stack before (top)  [value]
                Stack after         []
                PutLocalSlot        <slot>
         */
        CASE (EJS_OP_PUT_LOCAL_SLOT):
            SET_SLOT(NULL, FRAME, GET_INT(), pop(ejs));
            BREAK;

        /*
            Store a local variable from slot 0-9
                PutLocalSlot0, PutLocalSlot1, ... PutLocalSlot9
                Stack before (top)  [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_LOCAL_SLOT_0):
        CASE (EJS_OP_PUT_LOCAL_SLOT_1):
        CASE (EJS_OP_PUT_LOCAL_SLOT_2):
        CASE (EJS_OP_PUT_LOCAL_SLOT_3):
        CASE (EJS_OP_PUT_LOCAL_SLOT_4):
        CASE (EJS_OP_PUT_LOCAL_SLOT_5):
        CASE (EJS_OP_PUT_LOCAL_SLOT_6):
        CASE (EJS_OP_PUT_LOCAL_SLOT_7):
        CASE (EJS_OP_PUT_LOCAL_SLOT_8):
        CASE (EJS_OP_PUT_LOCAL_SLOT_9):
            SET_SLOT(NULL, FRAME, opcode - EJS_OP_PUT_LOCAL_SLOT_0, pop(ejs));
            BREAK;

        /*
            Store a block variable by slot number
                PutBlockSlot        <slot> <nthBlock>
                Stack before (top)  [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_BLOCK_SLOT):
            slotNum = GET_INT();
            obj = getNthBlock(ejs, GET_INT());
            SET_SLOT(NULL, obj, slotNum, pop(ejs));
            BREAK;

#if FUTURE
        /*
            Store a block variable from slot 0-9
                PutBlockSlot0, PutBlockSlot1, ... PutBlockSlot9 <nthBlock>
                Stack before (top)  [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_BLOCK_SLOT_0):
        CASE (EJS_OP_PUT_BLOCK_SLOT_1):
        CASE (EJS_OP_PUT_BLOCK_SLOT_2):
        CASE (EJS_OP_PUT_BLOCK_SLOT_3):
        CASE (EJS_OP_PUT_BLOCK_SLOT_4):
        CASE (EJS_OP_PUT_BLOCK_SLOT_5):
        CASE (EJS_OP_PUT_BLOCK_SLOT_6):
        CASE (EJS_OP_PUT_BLOCK_SLOT_7):
        CASE (EJS_OP_PUT_BLOCK_SLOT_8):
        CASE (EJS_OP_PUT_BLOCK_SLOT_9):
            slotNum = opcode - EJS_OP_PUT_BLOCK_SLOT_0;
            obj = getNthBlock(ejs, GET_INT());
            SET_SLOT(NULL, obj, slotNum, pop(ejs));
            BREAK;
#endif

        /*
            Store a property by slot number
                PutThisSlot         <slot>
                Stack before (top)  [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_THIS_SLOT):
            slotNum = GET_INT();
            SET_SLOT(THIS, THIS, slotNum, pop(ejs));
            BREAK;

        /*
            Store a property to slot 0-9
                PutThisSlot0, PutThisSlot1, ... PutThisSlot9,
                Stack before (top)  [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_THIS_SLOT_0):
        CASE (EJS_OP_PUT_THIS_SLOT_1):
        CASE (EJS_OP_PUT_THIS_SLOT_2):
        CASE (EJS_OP_PUT_THIS_SLOT_3):
        CASE (EJS_OP_PUT_THIS_SLOT_4):
        CASE (EJS_OP_PUT_THIS_SLOT_5):
        CASE (EJS_OP_PUT_THIS_SLOT_6):
        CASE (EJS_OP_PUT_THIS_SLOT_7):
        CASE (EJS_OP_PUT_THIS_SLOT_8):
        CASE (EJS_OP_PUT_THIS_SLOT_9):
            SET_SLOT(THIS, THIS, opcode - EJS_OP_PUT_THIS_SLOT_0, pop(ejs));
            BREAK;

        /* 
            Store a property by slot number
                PutObjSlot          <slot>
                Stack before (top)  [obj]
                                    [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_OBJ_SLOT):
            vp = pop(ejs);
            SET_SLOT(NULL, vp, GET_INT(), pop(ejs));
            BREAK;

        /*
            Store a property to slot 0-9
                PutObjSlot0, PutObjSlot1, ... PutObjSlot9
                Stack before (top)  [obj]
                                    [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_OBJ_SLOT_0):
        CASE (EJS_OP_PUT_OBJ_SLOT_1):
        CASE (EJS_OP_PUT_OBJ_SLOT_2):
        CASE (EJS_OP_PUT_OBJ_SLOT_3):
        CASE (EJS_OP_PUT_OBJ_SLOT_4):
        CASE (EJS_OP_PUT_OBJ_SLOT_5):
        CASE (EJS_OP_PUT_OBJ_SLOT_6):
        CASE (EJS_OP_PUT_OBJ_SLOT_7):
        CASE (EJS_OP_PUT_OBJ_SLOT_8):
        CASE (EJS_OP_PUT_OBJ_SLOT_9):
            vp = pop(ejs);
            SET_SLOT(NULL, vp, opcode - EJS_OP_PUT_OBJ_SLOT_0, pop(ejs));
            BREAK;

        /*
            Store a variable by an unqualified name
                PutScopedName       <qname>
                Stack before (top)  [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_SCOPED_NAME):
            qname = GET_NAME();
            value = pop(ejs);
            storePropertyToScope(ejs, qname, value);
            BREAK;

        /*
            Store a variable by an unqualified name expression
                PutScopedName 
                Stack before (top)  [name]
                                    [space]
                                    [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_SCOPED_NAME_EXPR):
            qname.name = ejsToString(ejs, pop(ejs));
            v1 = pop(ejs);
            if (ejsIs(ejs, v1, Namespace)) {
                qname.space = ((EjsNamespace*) v1)->value;
            } else {
                qname.space = ejsToString(ejs, v1);
            }
            value = pop(ejs);
            storePropertyToScope(ejs, qname, value);
            BREAK;

        /*
            Store a property by property name to an object
                PutObjName
                Stack before (top)  [objRef]
                                    [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_OBJ_NAME):
            qname = GET_NAME();
            obj = pop(ejs);
            value = pop(ejs);
            storeProperty(ejs, obj, obj, qname, value);
            BREAK;

        /*
            Store a property by a qualified property name expression to an object
                PutObjNameExpr
                Stack before (top)  [nameExpr]
                                    [spaceExpr]
                                    [objRef]
                                    [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_OBJ_NAME_EXPR):
            v1 = pop(ejs);
            v2 = pop(ejs);
            obj = pop(ejs);
            value = pop(ejs);
            //  TODO -- cleanup this too - push into storeProperty
            if (TYPE(obj)->numericIndicies && ejsIs(ejs, v1, Number)) {
                ejsSetProperty(ejs, obj, ejsGetInt(ejs, v1), value);
            } else {
                qname.name = ejsToString(ejs, v1);
                if (ejsIs(ejs, v2, Namespace)) {
                    qname.space = ((EjsNamespace*) v2)->value;
                } else {
                    qname.space = ejsToString(ejs, v2);
                }
                if (qname.name && qname.space) {
                    storeProperty(ejs, obj, obj, qname, value);
                }
            }
            BREAK;

        /*
            Store a type variable by slot number
                PutTypeSlot         <slot> <nthBase>
                Stack before (top)  [obj]
                                    [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_TYPE_SLOT):
            slotNum = GET_INT();
            vobj = pop(ejs);
            vp = getNthBase(ejs, vobj, GET_INT());
            SET_SLOT(vobj, vp, slotNum, pop(ejs));
            BREAK;

        /*
            Store a variable to a slot in the nthBase class of the current "this" object
                PutThisTypeSlot     <slot> <nthBase>
                Stack before (top)  [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_THIS_TYPE_SLOT):
            slotNum = GET_INT();
            type = (EjsType*) getNthBaseFromBottom(ejs, THIS, GET_INT());
            if (type == 0) {
                ejsThrowReferenceError(ejs, "Bad base class reference");
            } else {
                SET_SLOT(THIS, type, slotNum, pop(ejs));
            }
            BREAK;


        /* Function calling and return */

        /*
            Call a function by reference
                Stack before (top)  [args]
                                    [function]
                                    [thisObj]
                Stack after         []
         */
        CASE (EJS_OP_CALL):
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            vp = state->stack[-argc - 1];
            callFunction(ejs, (EjsFunction*) state->stack[-argc], vp, argc, 2);
            BREAK;

        /*
            Call a global function by slot on the given type
                CallGlobalSlot      <slot> <argc>
                Stack before (top)  [args]
                Stack after         []
         */
        CASE (EJS_OP_CALL_GLOBAL_SLOT):
            slotNum = GET_INT();
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            callProperty(ejs, global, slotNum, NULL, argc, 0);
            BREAK;

        /*
            Call a function by slot number on the pushed object
                CallObjSlot         <slot> <argc>
                Stack before (top)  [args]
                                    [obj]
                Stack after         []
         */
        CASE (EJS_OP_CALL_OBJ_SLOT):
            slotNum = GET_INT();
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            vp = state->stack[-argc];
            if (vp == ESV(null) || vp == ESV(undefined)) {
                //  TODO -- refactor
                if (vp && (slotNum == ES_Object_iterator_get || slotNum == ES_Object_iterator_getValues)) {
                    callProperty(ejs, TYPE(vp), slotNum, vp, argc, 1);
                } else {
                    ejsThrowReferenceError(ejs, "Object reference is null or undefined");
                }
            } else {
                callProperty(ejs, TYPE(vp)->prototype, slotNum, vp, argc, 1);
            }
            BREAK;

        /*
            Call a function by slot number on the current this object.
                CallThisSlot        <slot> <argc>
                Stack before (top)  [args]
                Stack after         []
         */
        CASE (EJS_OP_CALL_THIS_SLOT):
            slotNum = GET_INT();
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            obj = (EjsObj*) TYPE(THIS)->prototype;
            callProperty(ejs, obj, slotNum, NULL, argc, 0);
            BREAK;

        /*
            Call a function by slot number on the nth enclosing block
                CallBlockSlot        <slot> <nthBlock> <argc>
                Stack before (top)  [args]
                Stack after         []
         */
        CASE (EJS_OP_CALL_BLOCK_SLOT):
            slotNum = GET_INT();
            obj = getNthBlock(ejs, GET_INT());
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            callProperty(ejs, obj, slotNum, NULL, argc, 0);
            BREAK;

        /*
            Call a function by slot number on an object.
                CallObjInstanceSlot <slot> <argc>
                Stack before (top)  [args]
                                    [obj]
                Stack after         []
         */
        CASE (EJS_OP_CALL_OBJ_INSTANCE_SLOT):
            slotNum = GET_INT();
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            vp = state->stack[-argc];
            if (vp == 0 || vp == ESV(null) || vp == ESV(undefined)) {
                ejsThrowReferenceError(ejs, "Object reference is null");
            } else {
                callProperty(ejs, vp, slotNum, vp, argc, 1);
            }
            BREAK;

        /*
            Call a static function by slot number on the pushed object
                CallObjStaticSlot   <slot> <nthBase> <argc>
                Stack before (top)  [args]
                                    [obj]
                Stack after         []
         */
        CASE (EJS_OP_CALL_OBJ_STATIC_SLOT):
            slotNum = GET_INT();
            nthBase = GET_INT();
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            vp = state->stack[-argc];
            if (vp == ESV(null) || vp == ESV(undefined)) {
                throwNull(ejs);
            } else {
                type = (EjsType*) getNthBase(ejs, vp, nthBase);
                callProperty(ejs, type, slotNum, type, argc, 1);
            }
            BREAK;

        /*
            Call a static function by slot number on the nth base class of the current "this" object
                CallThisStaticSlot  <slot> <nthBase> <argc>
                Stack before (top)  [args]
                Stack after         []
         */
        CASE (EJS_OP_CALL_THIS_STATIC_SLOT):
            slotNum = GET_INT();
            nthBase = GET_INT();
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            type = (EjsType*) getNthBase(ejs, THIS, nthBase);
            if (type == EST(Object)) {
                //  TODO - remove
                ejsThrowReferenceError(ejs, "Bad type reference");
                BREAK;
            }
            callProperty(ejs, type, slotNum, type, argc, 0);
            BREAK;

        /*
            Call a function by name on the pushed object
                CallObjName         <qname> <argc>
                Stack before (top)  [args]
                                    [obj]
                Stack after         []
         */
        CASE (EJS_OP_CALL_OBJ_NAME):
            qname = GET_NAME();
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            vp = state->stack[-argc];
            if (vp == 0) {
                ejsThrowReferenceError(ejs, "%@ is not defined", qname.name);
                throwNull(ejs);
                BREAK;
            }
            slotNum = ejsLookupVar(ejs, vp, qname, &lookup);
            if (slotNum < 0) {
                ejsThrowReferenceError(ejs, "Cannot find function \"%@\"", qname.name);
            } else {
                EjsTrait *trait = ejsGetPropertyTraits(ejs, lookup.obj, slotNum);
                if (trait && trait->attributes & EJS_PROP_STATIC) {
                    vp = lookup.obj;
                }
                callProperty(ejs, lookup.obj, slotNum, vp, argc, 1);
            }
            BREAK;

        /*
            Call a function by name in the current scope. Use existing "this" object if defined.
                CallName            <qname> <argc>
                Stack before (top)  [args]
                Stack after         []
         */
        CASE (EJS_OP_CALL_SCOPED_NAME):
            qname = GET_NAME();
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            slotNum = ejsLookupScope(ejs, qname, &lookup);
            if (slotNum < 0) {
                ejsThrowReferenceError(ejs, "Cannot find function %@", qname.name);
                BREAK;
            }
            fun = ejsGetProperty(ejs, lookup.obj, slotNum);
            if (ejsIsType(ejs, fun)) {
                type = (EjsType*) fun;
                callFunction(ejs, fun, NULL, argc, 0);

            } else if (!ejsIsFunction(ejs, fun)) {
                if (!ejs->exception) {
                    if ((EjsObj*) vp == (EjsObj*) ESV(undefined)) {
                        ejsThrowReferenceError(ejs, "Function is undefined");
                    } else {
                        ejsThrowReferenceError(ejs, "Reference is not a function");
                    }
                }
            } else {
                /*
                    Calculate the "this" to use for the function. If required function is a method in the current 
                    "this" object use the current thisObj. If the lookup.obj is a type, then use it. Otherwise global.
                 */
                if ((vp = fun->boundThis) == 0) {
                    if (lookup.obj == THIS) {
                        vp = THIS;
                    } else if (ejsIsPrototype(ejs, lookup.obj) && ejsIsA(ejs, THIS, lookup.type)) {
                        vp = THIS;
                    } else if (ejsIsType(ejs, lookup.obj)) {
                        vp = lookup.obj;
                    } else {
                        vp = /* lookup.obj */ ejs->global;
                    } 
                }
                callProperty(ejs, lookup.obj, slotNum, vp, argc, 0);
            }
            BREAK;

        /*
            Call a constructor
                CallConstructor     <argc>
                Stack before (top)  [args]
                                    [obj]
                Stack after         [obj]
         */
        CASE (EJS_OP_CALL_CONSTRUCTOR):
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            vp = state->stack[-argc];
            if (vp == 0 || vp == ESV(null) || vp == ESV(undefined)) {
                throwNull(ejs);
                BREAK;
            }
            type = TYPE(vp);
            assert(type);
            if (type && type->constructor.block.pot.isFunction) {
                assert(type->prototype);
                callFunction(ejs, (EjsFunction*) type, vp, argc, 0);
                state->stack[0] = ejs->result;
            }
            BREAK;

        /*
            Call the next constructor
                CallNextConstructor <argc>
                Stack before (top)  [args]
                Stack after         []
         */
        CASE (EJS_OP_CALL_NEXT_CONSTRUCTOR):
            qname = GET_NAME();
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            type = ejsGetPropertyByName(ejs, ejs->global, qname);
            if (type == 0) {
                ejsThrowReferenceError(ejs, "Cannot find constructor %@", qname.name);
            } else {
                assert(type->constructor.block.pot.isFunction);
                callFunction(ejs, (EjsFunction*) type, THIS, argc, 0);
            }
            BREAK;

        /*
            Add a literal namespace to the set of open namespaces for the current block
                AddNamespace <string>
         */
        CASE (EJS_OP_ADD_NAMESPACE):
            str = GET_STRING();
            nsp = ejsCreateNamespace(ejs, str);
            ejsAddNamespaceToBlock(ejs, state->bp, nsp);
            if (ejsContainsAsc(ejs, str, "internal-") >= 0) {
                state->internal = nsp;
            }
            BREAK;

        /*
            Add a namespace expression (reference) to the set of open namespaces for the current block. (use namespace).
                Stack before (top)  [namespace]
                Stack after         []
                AddNamespaceRef
         */
        CASE (EJS_OP_ADD_NAMESPACE_REF):
            ejsAddNamespaceToBlock(ejs, state->bp, (EjsNamespace*) pop(ejs));
            BREAK;

        /*
            Push a new scope block on the scope chain
                OpenBlock <slotNum> <nthBlock>
         */
        CASE (EJS_OP_OPEN_BLOCK):
            slotNum = GET_INT();
            vp = getNthBlock(ejs, GET_INT());
            v1 = ejsGetProperty(ejs, vp, slotNum);
            if (!ejsIsBlock(ejs, v1)) {
                ejsThrowReferenceError(ejs, "Reference is not a block");
                BREAK;
            }
            //  OPT
            blk = ejsCloneBlock(ejs, (EjsBlock*) v1, 0);
            blk->prev = blk->scope = state->bp;
            state->bp = blk;
            blk->stackBase = state->stack;
            mprCopyName(state->bp, v1);
            ejsSetBlockLocation(blk, FRAME->line);
            BREAK;

        /*
            Add a new scope block from the stack onto on the scope chain
                OpenWith
         */
        CASE (EJS_OP_OPEN_WITH):
            vp = pop(ejs);
            blk = ejsCreateBlock(ejs, 0);
            memcpy((void*) blk, vp, TYPE(vp)->instanceSize);
            blk->prev = blk->scope = state->bp;
            state->bp = blk;
            BREAK;

        /*
            Store the top scope block off the scope chain
                CloseBlock
                CloseWith
         */
        CASE (EJS_OP_CLOSE_BLOCK):
            state->bp = state->bp->prev;
            BREAK;

        /*
            Define a class and initialize by calling any static initializer.
                DefineClass <type>
         */
        CASE (EJS_OP_DEFINE_CLASS):
            type = GET_TYPE();
            if (type == 0 || !ejsIsType(ejs, type)) {
                ejsThrowReferenceError(ejs, "Reference is not a class");
            } else {
                type->constructor.block.scope = state->bp;
                if (type && type->hasInitializer) {
                    fun = ejsGetProperty(ejs, type, 0);
                    callFunction(ejs, fun, type, 0, 0);
                    if (type->implements && !ejs->exception) {
                        callInterfaceInitializers(ejs, type);
                    }
                    state->bp = &FRAME->function.block;
                }
            }
            ejs->result = type;
            BREAK;

        /*
            Define a function. This is used only for non-method functions to capture the scope chain.
                DefineFunction <slot> <nthBlock>
         */
        CASE (EJS_OP_DEFINE_FUNCTION):
            qname = GET_NAME();
            if ((slotNum = ejsLookupScope(ejs, qname, &lookup)) >= 0) {
                f1 = ejsGetProperty(ejs, lookup.obj, lookup.slotNum);
            }
            if (slotNum < 0 || !ejsIsFunction(ejs, f1)) {
                ejsThrowReferenceError(ejs, "Reference is not a function");
            } else {
                //  TODO -- fullScope is always true if DEFINE_FUNCTION is emitted
                assert(f1->fullScope);
                if (f1->fullScope) {
                    //  TODO - why exception for global
                    if (lookup.obj != ejs->global) {
                        f2 = ejsCloneFunction(ejs, f1, 0);
                    } else {
                        f2 = f1;
                    }
                    f2->block.scope = state->bp;
                    if (FRAME->function.boundThis != ejs->global) {
                        f2->boundThis = FRAME->function.boundThis;
                    }
                    assert(f2->boundThis != ejs->global);
                    assert(!ejsIsPrototype(ejs, lookup.obj));
                    //  OPT - don't do this for global functions (if f2 == f1 and boundThis not updated (== global))
                    ejsSetProperty(ejs, lookup.obj, lookup.slotNum, f2);
                }
            }
            BREAK;


        /* Exception Handling -------------------------------------------- */

        /*
            Invoke finally blocks before acting on: return, returnValue and break/continue (goto) opcodes.
            These are injected by the compiler.
        
                call_finally
         */
        CASE (EJS_OP_CALL_FINALLY):
            if ((ex = findExceptionHandler(ejs, EJS_EX_FINALLY)) != 0) {
                uchar   *savePC;
                if (FRAME->function.inCatch) {
                    popExceptionBlock(ejs);
                }
                savePC = FRAME->pc;
                createExceptionBlock(ejs, ex, EJS_EX_FINALLY);
                BLOCK->restartAddress = savePC;
            }
            BREAK;

        /*
            Invoke finally blocks before leaving try block. These are injected by the compiler.
        
                goto_finally
         */
        CASE (EJS_OP_GOTO_FINALLY):
            if ((ex = findExceptionHandler(ejs, EJS_EX_FINALLY)) != 0) {
                if (FRAME->function.inCatch) {
                    popExceptionBlock(ejs);
                }
                createExceptionBlock(ejs, ex, EJS_EX_FINALLY);
            }
            BREAK;

        /*
            End of an exception block. Put at the end of the last catch/finally block
                EndException
         */
        CASE (EJS_OP_END_EXCEPTION):
            if (FRAME->function.inException) {
                FRAME->function.inCatch = 0;
                FRAME->function.inException = 0;
                if (BLOCK->restartAddress) {
                    uchar *savePC = BLOCK->restartAddress;
                    popExceptionBlock(ejs);
                    SET_PC(FRAME, savePC);
                } else {
                    offset = findEndException(ejs);
                    SET_PC(FRAME, &FRAME->function.body.code->byteCode[offset]);
                    popExceptionBlock(ejs);
                }
            }
            BREAK;

        /*
            Throw an exception
                Stack before (top)  [exceptionObj]
                Stack after         []
                Throw
         */
        CASE (EJS_OP_THROW):
            ejs->exception = pop(ejs);
            ejsAttention(ejs);
            BREAK;

        /*
            Special circumstances need attention. Exceptions, exiting and garbage collection.
         */
        CASE (EJS_OP_ATTENTION):
            CHECK_GC();
            assert(FRAME->attentionPc);
            if (FRAME->attentionPc) {
                FRAME->pc = FRAME->attentionPc;
                assert(FRAME->pc);
                FRAME->attentionPc = 0;
            }
            if (mprHasMemError(ejs) && !ejs->exception) {
                mprResetMemError(ejs);
                ejsThrowMemoryError(ejs);
            }
            if (ejs->exception && !processException(ejs)) {
                goto done;
            }
            if (ejs->exiting || mprIsStopping()) {
                goto done;
            }
            BREAK;


        /* Stack management ---------------------------------------------- */

        /*
            Pop one item off the stack
                Pop
                Stack before (top)  [value]
                Stack after         []
         */
        CASE (EJS_OP_POP):
            ejs->result = pop(ejs);
            assert(ejs->exception || ejs->result);
            BREAK;

        /*
            Pop N items off the stack
                PopItems            <count>
                Stack before (top)  [value]
                                    [...]
                Stack after         []
         */
        CASE (EJS_OP_POP_ITEMS):
            state->stack -= GET_BYTE();
            BREAK;

        /*
            Duplicate one item on the stack
                Stack before (top)  [value]
                Stack after         [value]
                                    [value]
         */
        CASE (EJS_OP_DUP):
            vp = state->stack[0];
            push(vp);
            BREAK;

        /*
            Duplicate two items on the stack
                Dup2
                Stack before (top)  [value1]
                                    [value2]
                Stack after         [value1]
                                    [value2]
                                    [value1]
                                    [value2]
         */
        CASE (EJS_OP_DUP2):
            v1 = state->stack[-1];
            push(v1);
            v1 = state->stack[0];
            push(v1);
            BREAK;

        /*
            Duplicate one item on the stack
                Stack before (top)  [value]
                Stack after         [value]
                                    [value]
         */
        CASE (EJS_OP_DUP_STACK):
            i = GET_BYTE();
            if (i < 0 || i > 32) {
                ejsThrowTypeError(ejs, "Bad stack index");
            } else {
                vp = state->stack[-i];
                push(vp);
            }
            BREAK;

        /*
            Swap the top two items on the stack
                Swap
                Stack before (top)  [value1]
                                    [value2]
                Stack after         [value2]
                                    [value1]
         */
        CASE (EJS_OP_SWAP):
            SWAP; BREAK;


        /* Branching */

        /*
            Default function argument initialization. Computed goto for functions with more than 256 parameters.
                InitDefault         <tableSize>
         */
        CASE (EJS_OP_INIT_DEFAULT_ARGS): {
            int tableSize, numNonDefault;
            /*
                Use the argc value for the current function. Compare with the number of default args.
             */
            tableSize = (schar) GET_BYTE();
            numNonDefault = FRAME->function.numArgs - FRAME->function.numDefault;
            offset = FRAME->argc - numNonDefault;
            if (offset < 0 || offset > tableSize) {
                offset = tableSize;
            }
            FRAME->pc += ((uint*) FRAME->pc)[offset];
            BREAK;
        }

        /*
            Default function argument initialization. Computed goto for functions with less than 256 parameters.
                InitDefault.8       <tableSize.8>
         */
        CASE (EJS_OP_INIT_DEFAULT_ARGS_8): {
            int tableSize, numNonDefault;
            tableSize = (schar) GET_BYTE();
            numNonDefault = FRAME->function.numArgs - FRAME->function.numDefault - FRAME->function.rest;
            offset = FRAME->argc - numNonDefault;
            if (offset < 0 || offset > tableSize) {
                offset = tableSize;
            }
            FRAME->pc += FRAME->pc[offset];
            BREAK;
        }

        /*
            Spread array/object as individual args
                Pop
                Stack before (top)  [value]
                Stack after         []
         */
        CASE (EJS_OP_SPREAD):
            vp = *state->stack;
            count = ejsGetLength(ejs, vp);
            if (count > 0) {
                vp = pop(ejs);
                for (i = 0; i < count; i++) {
                    push(ejsGetProperty(ejs, vp, i));
                }
                ejs->spreadArgs = count - 1;
            }
            BREAK;

        /*
            Unconditional branch to a new location
                Goto                <offset.32>
         */
        CASE (EJS_OP_GOTO):
            offset = GET_WORD();
            SET_PC(FRAME, &FRAME->pc[offset]);
            CHECK_GC();
            BREAK;

        /*
            Unconditional branch to a new location (8 bit)
                Goto.8              <offset.8>
         */
        CASE (EJS_OP_GOTO_8):
            offset = (schar) GET_BYTE();
            SET_PC(FRAME, &FRAME->pc[offset]);
            CHECK_GC();
            BREAK;

        /*
            Branch to offset if false
                BranchFalse
                Stack before (top)  [boolValue]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_FALSE):
            offset = GET_WORD();
            goto commonBoolBranchCode;

        /*
            Branch to offset if true
                BranchTrue
                Stack before (top)  [boolValue]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_TRUE):
            offset = GET_WORD();
            goto commonBoolBranchCode;

        /*
            Branch to offset if false (8 bit)
                BranchFalse.8
                Stack before (top)  [boolValue]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_FALSE_8):
            opcode = (EjsOpCode) (opcode - EJS_OP_BRANCH_TRUE_8 + EJS_OP_BRANCH_TRUE);
            offset = (schar) GET_BYTE();
            goto commonBoolBranchCode;

        /*
            Branch to offset if true (8 bit)
                BranchTrue.8
                Stack before (top)  [boolValue]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_TRUE_8):
            /* We want sign extension here */
            opcode = (EjsOpCode) (opcode - EJS_OP_BRANCH_TRUE_8 + EJS_OP_BRANCH_TRUE);
            offset = (schar) GET_BYTE();

        /*
            Common boolean branch code
         */
        commonBoolBranchCode:
            v1 = pop(ejs);
            if (v1 == 0 || !ejsIs(ejs, v1, Boolean)) {
                v1 = ejsCast(ejs, v1, Boolean);
                if (ejs->exception) {
                    BREAK;
                }
            }
            if (!ejsIs(ejs, v1, Boolean)) {
                ejsThrowTypeError(ejs, "Result of a comparision must be boolean");
                BREAK;
            }
            if (opcode == EJS_OP_BRANCH_TRUE) {
                if (((EjsBoolean*) v1)->value) {
                    SET_PC(FRAME, &FRAME->pc[offset]);
                }
            } else {
                if (((EjsBoolean*) v1)->value == 0) {
                    SET_PC(FRAME, &FRAME->pc[offset]);
                }
            }
            CHECK_GC();
            BREAK;

        /*
            Branch to offset if [value1] == null
                BranchNull
                Stack before (top)  [boolValue]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_NULL):
            push(ESV(null));
            offset = GET_WORD();
            goto commonBranchCode;

        /*
            Branch to offset if [value1] == undefined
                BranchUndefined
                Stack before (top)  [boolValue]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_UNDEFINED):
            push(ESV(undefined));
            offset = GET_WORD();
            goto commonBranchCode;

        /*
            Branch to offset if [tos] value is zero
                BranchZero
                Stack before (top)  [boolValue]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_ZERO):
            /* Fall through */

        /*
            Branch to offset if [tos] value is not zero
                BranchNotZero
                Stack before (top)  [boolValue]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_NOT_ZERO):
            push(ESV(zero));
            offset = GET_WORD();
            goto commonBranchCode;

        /*
            Branch to offset if [value1] == [value2]
                BranchEQ
                Stack before (top)  [value1]
                Stack before (top)  [value2]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_EQ):

        /*
            Branch to offset if [value1] === [value2]
                BranchStrictlyEQ
                Stack before (top)  [value1]
                Stack after         [value2]
         */
        CASE (EJS_OP_BRANCH_STRICTLY_EQ):

        /*
            Branch to offset if [value1] != [value2]
                BranchNotEqual
                Stack before (top)  [value1]
                                    [value2]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_NE):

        /*
            Branch to offset if [value1] !== [value2]
                BranchStrictlyNE
                Stack before (top)  [value1]
                                    [value2]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_STRICTLY_NE):

        /*
            Branch to offset if [value1] <= [value2]
                BranchLE
                Stack before (top)  [value1]
                                    [value2]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_LE):

        /*
            Branch to offset if [value1] < [value2]
                BranchLT
                Stack before (top)  [value1]
                                    [value2]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_LT):

        /*
            Branch to offset if [value1] >= [value2]
                BranchGE
                Stack before (top)  [value1]
                                    [value2]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_GE):

        /*
            Branch to offset if [value1] > [value2]
                BranchGT
                Stack before (top)  [value1]
                                    [value2]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_GT):
            offset = GET_WORD();
            goto commonBranchCode;

        /*
            Handle all branches here. We convert to a compare opcode and pass to the type to handle.
         */
        commonBranchCode:
            opcode = (EjsOpCode) (opcode - EJS_OP_BRANCH_EQ + EJS_OP_COMPARE_EQ);
            v2 = pop(ejs);
            v1 = pop(ejs);
            result = evalBinaryExpr(ejs, v1, opcode, v2);
            if (!ejsIs(ejs, result, Boolean)) {
                ejsThrowTypeError(ejs, "Result of a comparision must be boolean");
            } else if (((EjsBoolean*) result)->value) {
                SET_PC(FRAME, &FRAME->pc[offset]);
            }
            BREAK;

        /*
            Compare if [value1] == true
                CompareTrue
                Stack before (top)  [value]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_TRUE):

        /*
            Compare if ![value1]
                CompareNotTrue
                Stack before (top)  [value]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_FALSE):
            v1 = pop(ejs);
            result = ejsInvokeOperator(ejs, v1, opcode, 0);
            push(result);
            BREAK;

        /*
            Compare if [value1] == NULL
                CompareNull
                Stack before (top)  [value]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_NULL):
            push(ESV(null));
            goto binaryExpression;

        /*
            Compare if [item] == undefined
                CompareUndefined
                Stack before (top)  [value]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_UNDEFINED):
            push(ESV(undefined));
            goto binaryExpression;

        /*
            Compare if [item] value is zero
                CompareZero
                Stack before (top)  [value]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_ZERO):
            push(ejsCreateNumber(ejs, 0));
            goto binaryExpression;

        /*
            Compare if [tos] value is not zero
                CompareZero
                Stack before (top)  [value]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_NOT_ZERO):
            push(ejsCreateNumber(ejs, 0));
            goto binaryExpression;

        /*
            Compare if [value1] == [item2]
                CompareEQ
                Stack before (top)  [value1]
                                    [value2]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_EQ):

        /*
            Compare if [value1] === [item2]
                CompareStrictlyEQ
                Stack before (top)  [value1]
                                    [value2]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_STRICTLY_EQ):

        /*
            Compare if [value1] != [item2]
                CompareNE
                Stack before (top)  [value1]
                                    [value2]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_NE):

        /*
            Compare if [value1] !== [item2]
                CompareStrictlyNE
                Stack before (top)  [value1]
                                    [value2]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_STRICTLY_NE):

        /*
            Compare if [value1] <= [item2]
                CompareLE
                Stack before (top)  [value1]
                                    [value2]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_LE):

        /*
            Compare if [value1] < [item2]
                CompareStrictlyLT
                Stack before (top)  [value1]
                                    [value2]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_LT):

        /*
            Compare if [value1] >= [item2]
                CompareGE
                Stack before (top)  [value1]
                                    [value2]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_GE):

        /*
            Compare if [value1] > [item2]
                CompareGT
                Stack before (top)  [value1]
                                    [value2]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_GT):

        /*
            Binary expressions
                Stack before (top)  [right]
                                    [left]
                Stack after         [boolean]
         */
        CASE (EJS_OP_ADD):
        CASE (EJS_OP_SUB):
        CASE (EJS_OP_MUL):
        CASE (EJS_OP_DIV):
        CASE (EJS_OP_REM):
        CASE (EJS_OP_SHL):
        CASE (EJS_OP_SHR):
        CASE (EJS_OP_USHR):
        CASE (EJS_OP_AND):
        CASE (EJS_OP_OR):
        CASE (EJS_OP_XOR):
        binaryExpression:
            v2 = pop(ejs);
            v1 = pop(ejs);
            assert(v1);
            ejs->result = evalBinaryExpr(ejs, v1, opcode, v2);
            push(ejs->result);
            BREAK;


        /* Unary operators */

        /*
            Negate a value
                Neg
                Stack before (top)  [value]
                Stack after         [result]
         */
        CASE (EJS_OP_NEG):
            v1 = pop(ejs);
            //  TODO - should this assign to ejs->result
            result = ejsInvokeOperator(ejs, v1, opcode, 0);
            push(result);
            BREAK;

        /*
            Logical not (~value)
                LogicalNot
                Stack before (top)  [value]
                Stack after         [result]
         */
        CASE (EJS_OP_LOGICAL_NOT):
            v1 = pop(ejs);
            if ((v1 = ejsCast(ejs, v1, Boolean)) != 0) {
                result = ejsInvokeOperator(ejs, v1, opcode, 0);
                push(result);
            }
            BREAK;

        /*
            Bitwise not (!value)
                BitwiseNot
                Stack before (top)  [value]
                Stack after         [result]
         */
        CASE (EJS_OP_NOT):
            v1 = pop(ejs);
            result = ejsInvokeOperator(ejs, v1, opcode, 0);
            push(result);
            BREAK;

        /*
            Increment a stack variable
                Inc                 <increment>
                Stack before (top)  [value]
                Stack after         [result]
         */
        CASE (EJS_OP_INC):
            v1 = pop(ejs);
            count = (schar) GET_BYTE();
            result = evalBinaryExpr(ejs, v1, EJS_OP_ADD, ejsCreateNumber(ejs, count));
            push(result);
            BREAK;


        /* Object creation */

        /*
            Create a new object
                New
                Stack before (top)  [type]
                Stack after         [obj]
         */
        CASE (EJS_OP_NEW):
            v1 = pop(ejs);
            if (!ejsIsType(ejs, v1)) {
                if (ejsIsFunction(ejs, v1)) {
                    fun = (EjsFunction*) v1;
                    if (fun->archetype == 0) {
                        if ((fun->archetype = ejsCreateArchetype(ejs, fun, NULL)) == 0) {
                            BREAK;
                        }
                    }
                    obj = ejsCreateObj(ejs, fun->archetype, 0);
                } else {
                    ejsThrowReferenceError(ejs, "Cannot locate type");
                    BREAK;
                }
            } else {
                obj = ejsCreateObj(ejs, (EjsType*) v1, 0);
            }
            push(obj);
            ejs->result = obj;
            BREAK;
                
            /*
             Create a new array literal
             NewArray            <type> <argc>
             Stack before (top)  [<index><value>]
             [<index><value>]
             Stack after         []
             */
        CASE (EJS_OP_NEW_ARRAY):
            paused = ejsBlockGC(ejs);
            type = GET_TYPE();
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            state->t1 = vp = ejsCreateObj(ejs, type, 0);
            for (i = 1 - (argc * 2); i <= 0; ) {
                indexVar = ejsToNumber(ejs, state->stack[i++]);
                if (ejs->exception) BREAK;
                v1 = state->stack[i++];
                if (v1 && indexVar) {
                    ejsSetProperty(ejs, vp, ejsGetInt(ejs, indexVar), v1);
                }
            }
            state->stack -= (argc * 2);
            push(vp);
            state->t1 = 0;
            ejsUnblockGC(ejs, paused);
            BREAK;

        /*
            Create a new object literal
                NewObject           <type> <argc> [<attributes> ...]
                Stack before (top)  [<space><name><value>]
                                    [<space><name><value>]
                Stack after         []
         */
        CASE (EJS_OP_NEW_OBJECT):
            paused = ejsBlockGC(ejs);
            type = GET_TYPE();
            argc = GET_INT();
            argc += ejs->spreadArgs;
            ejs->spreadArgs = 0;
            state->t1 = vp = ejsCreateObj(ejs, type, 0);
            for (i = 1 - (argc * 3); i <= 0; ) {
                spaceVar = ejsToString(ejs, state->stack[i++]);
                if (ejs->exception) BREAK;
                nameVar = ejsToString(ejs, state->stack[i++]);
                if (ejs->exception) BREAK;
                v1 = state->stack[i++];
                attributes = GET_INT();
                if (v1 && nameVar && spaceVar) {
                    EjsName qname = { nameVar, spaceVar };
                    ejsDefineProperty(ejs, vp, -1, qname, NULL, attributes, v1);
                }
            } 
            state->stack -= (argc * 3);
            push(vp);
            state->t1 = 0;
            ejsUnblockGC(ejs, paused);
            BREAK;


        /*
            Reference the super class
                Super
                Stack before (top)  [obj]
                Stack after         [type]
         */
        CASE (EJS_OP_SUPER):
            push(TYPE(FRAME->function.boundThis));
            BREAK;

        /*
            Delete an object property by name expression
                DeleteNameExpr
                Stack before (top)  [name]
                                    [space]
                                    [obj]
                Stack after         [true|false]
         */
        CASE (EJS_OP_DELETE_NAME_EXPR):
            qname.name = ejsToString(ejs, pop(ejs));
            v1 = pop(ejs);
            if (ejsIs(ejs, v1, Namespace)) {
                qname.space = ((EjsNamespace*) v1)->value;
            } else {
                qname.space = ejsToString(ejs, v1);
            }
            vp = pop(ejs);
            slotNum = ejsLookupVar(ejs, vp, qname, &lookup);
            if (slotNum < 0) {
                push(ESV(true));
            } else {
                if (ejsPropertyHasTrait(ejs, lookup.obj, slotNum, EJS_TRAIT_FIXED)) {
                    push(ESV(false));
                } else {
                    ejsDeletePropertyByName(ejs, lookup.obj, lookup.name);
                    push(ESV(true));
                }
            }
            BREAK;

        /*
            Delete an object property from the current scope
                DeleteScopedNameExpr
                Stack before (top)  [name]
                                    [space]
                Stack after         [true|false]
         */
        CASE (EJS_OP_DELETE_SCOPED_NAME_EXPR):
            qname.name = ejsToString(ejs, pop(ejs));
            v1 = pop(ejs);
            if (ejsIs(ejs, v1, Namespace)) {
                qname.space = ((EjsNamespace*) v1)->value;
            } else {
                qname.space = ejsToString(ejs, v1);
            }
            slotNum = ejsLookupScope(ejs, qname, &lookup);
            if (slotNum < 0) {
                push(ESV(true));
            } else {
                if (ejsPropertyHasTrait(ejs, lookup.obj, slotNum, EJS_TRAIT_FIXED)) {
                    push(ESV(false));
                } else {
                    ejsDeletePropertyByName(ejs, lookup.obj, lookup.name);
                    push(ESV(true));
                }
            }
            BREAK;

        /*
            No operation. Does nothing.
                Nop
         */
        CASE (EJS_OP_NOP):
            BREAK;

        /*
            Check if object is a given type
                IsA, Like, InstanceOf
                Stack before (top)  [type]
                                    [obj]
                Stack after         [boolean]
         */
        CASE (EJS_OP_INST_OF):
        CASE (EJS_OP_IS_A):
        CASE (EJS_OP_LIKE):
            type = (EjsType*) pop(ejs);
            v1 = pop(ejs);
            push(ejsCreateBoolean(ejs, ejsIsA(ejs, v1, type)));
            BREAK;

        /*
            Get the type of an object.
                TypeOf              <obj>
                Stack before (top)  [obj]
                Stack after         [string]
         */
        CASE (EJS_OP_TYPE_OF):
            v1 = pop(ejs);
            push(ejsGetTypeOf(ejs, v1));
            BREAK;

        /*
            Cast an object to the given the type. Throw if not castable.
                Cast
                Stack before (top)  [type]
                                    [obj]
                Stack after         [result]
         */
        CASE (EJS_OP_CAST):
            type = (EjsType*) pop(ejs);
            if (!ejsIsType(ejs, type)) {
                ejsThrowTypeError(ejs, "Not a type");
            } else {
                v1 = pop(ejs);
                push(ejsCastType(ejs, v1, type));
            }
            BREAK;

        /*
            Cast to a boolean type
                CastBoolean
                Stack before (top)  [value]
                Stack after         [result]
         */
        CASE (EJS_OP_CAST_BOOLEAN):
            v1 = ejsCast(ejs, pop(ejs), Boolean);
            push(v1);
            BREAK;

        /*
            Test if a given name is the name of a property "in" an object
                Cast
                Stack before (top)  [obj]
                                    [name]
                Stack after         [result]
         */
        CASE (EJS_OP_IN):
            v1 = pop(ejs);
            nameVar = ejsToString(ejs, pop(ejs));
            if (nameVar == 0) {
                ejsThrowTypeError(ejs, "Cannot convert to a name");
            } else {
                EjsName n = { nameVar, NULL };
                slotNum = ejsLookupProperty(ejs, v1, n);
                if (slotNum < 0) {
                    n.space = ESV(empty);
                    slotNum = ejsLookupVar(ejs, v1, n, &lookup);
                    if (slotNum < 0 && ejsIsType(ejs, v1)) {
                        slotNum = ejsLookupVar(ejs, ((EjsType*) v1)->prototype, n, &lookup);
                    }
                }
                push(ejsCreateBoolean(ejs, slotNum >= 0));
            }
            BREAK;

        /*
            Unimplemented op codes
         */
        CASE (EJS_OP_BREAKPOINT):
            assert(0);
            BREAK;

#if !BIT_UNIX_LIKE && !(VXWORKS && !BIT_DIAB)
        }
    }
#endif
    
done:
#if BIT_DEBUG && FUTURE
    if (ejs->initialized) {
        ejsShowOpFrequency(ejs);
    }
#endif
    assert(FRAME == 0 || FRAME->attentionPc == 0);
    ejs->state = ejs->state->prev;
    if (ejs->exception) {
        ejsAttention(ejs);
    }
}


/*
    WARNING: this may not complete the store. It may setup a setter function which then requires the VM to execute.
 */
static void storePropertyToSlot(Ejs *ejs, EjsObj *thisObj, EjsAny *obj, int slotNum, EjsObj *value)
{
    EjsFunction     *fun;
    EjsObj          *vp;
    EjsTrait        *trait;

    assert(value);

    if (slotNum < 0 && !DYNAMIC(obj)) {
        ejsThrowTypeError(ejs, "Object is not extendable");
        return;
    }
    trait = ejsGetPropertyTraits(ejs, obj, slotNum);
    if (trait) {
        if (trait->attributes & EJS_TRAIT_SETTER) {
            pushOutside(ejs, value);
            fun = ejsGetProperty(ejs, obj, slotNum);
            assert(fun);
            fun = fun->setter;
            assert(fun);
            callFunction(ejs, fun, thisObj, 1, 0);
            return;
        }
        if (trait->type) {
            if (!ejsIsA(ejs, value, trait->type)) {
                if (value == ESV(null) || value == ESV(undefined)) {
                    if (trait->attributes & EJS_TRAIT_THROW_NULLS) {
                        ejsThrowTypeError(ejs, "Unacceptable null or undefined value");
                        return;
                    } else if (trait->attributes & EJS_TRAIT_CAST_NULLS) {
                        value = ejsCastType(ejs, value, trait->type);
                        if (ejs->exception) {
                            return;
                        }
                    }
                } else {
                    value = ejsCastType(ejs, value, trait->type);
                    if (ejs->exception) {
                        return;
                    }
                }
            }
        }
        if (trait->attributes & EJS_TRAIT_READONLY) {
            EjsName  qname;
            vp = ejsGetProperty(ejs, obj, slotNum);
            if (vp != value && vp != ESV(null) && vp != ESV(undefined)) {
                if (ejsInvokeOperator(ejs, vp, EJS_OP_COMPARE_EQ, value) != ESV(true)) {
                    qname = ejsGetPropertyName(ejs, obj, slotNum);
                    ejsThrowReferenceError(ejs, "Property \"%@\" is not writable", qname.name);
                    return;
                }
            }
        }
    }
    ejsSetProperty(ejs, obj, slotNum, value);
    ejs->result = value;
}


/*
    Store a property by name in the given object. Will create if the property does not already exist.
 */
static void storeProperty(Ejs *ejs, EjsObj *thisObj, EjsAny *vp, EjsName qname, EjsObj *value)
{
    EjsLookup       lookup;
    EjsTrait        *trait;
    EjsPot          *pot;
    int             slotNum;

    assert(qname.name);
    assert(vp);

    //  ONLY XML requires this.  NOTE: this bypasses ES5 traits
    //  Alternatively push this whole function down into ejsObject and have all go via setPropertyByName
    
    if (TYPE(vp)->helpers.setPropertyByName) {
        slotNum = (*TYPE(vp)->helpers.setPropertyByName)(ejs, vp, qname, value);
        if (slotNum >= 0) {
            return;
        }
    }
    if ((slotNum = ejsLookupVar(ejs, vp, qname, &lookup)) >= 0) {
        if (lookup.obj != vp) {
            trait = ejsGetPropertyTraits(ejs, lookup.obj, slotNum);
            if (trait->attributes & EJS_TRAIT_SETTER) {
                vp = lookup.obj;
                
            } else if (ejsIsPrototype(ejs, lookup.obj) || trait->attributes & EJS_TRAIT_GETTER) {
                if (TYPE(vp)->hasInstanceVars) {
                    /* The prototype properties have been inherited */
                    assert(ejsIsPot(ejs, vp));
                    slotNum = ejsCheckSlot(ejs, vp, slotNum);
                    pot = (EjsPot*) vp;
                    pot->properties->slots[slotNum].trait = ((EjsPot*) lookup.obj)->properties->slots[slotNum].trait;
                    pot->properties->slots[slotNum].value = ((EjsPot*) lookup.obj)->properties->slots[slotNum].value;
                    slotNum = ejsSetPropertyName(ejs, vp, slotNum, qname);
                } else  {
                    slotNum = -1;
                }
            } else {
                /*
                    This is the fundamental asymetry between load/store. We allow loading properties from static base 
                    types, but do not allow stores. This is essential to stop bleeding of Object static properties into
                    all objects. E.g. Object.create.
                 */
                slotNum = -1;
            }
        }
    }
    if (slotNum < 0) {
        slotNum = ejsSetPropertyName(ejs, vp, slotNum, qname);
    }
    if (!ejs->exception) {
        storePropertyToSlot(ejs, thisObj, vp, slotNum, value);
    }
}


/*
    Store a property by name in the scope chain. Will create properties if the given name does not already exist.
 */
static void storePropertyToScope(Ejs *ejs, EjsName qname, EjsObj *value)
{
    EjsFrame        *fp;
    EjsObj          *vp, *thisObj;
    EjsPot          *obj;
    EjsLookup       lookup;
    EjsTrait        *trait;
    int             slotNum;

    fp = ejs->state->fp;

    if ((slotNum = ejsLookupScope(ejs, qname, &lookup)) >= 0) {
        if (ejsIsPrototype(ejs, lookup.obj)) {
            thisObj = vp = (EjsObj*) fp->function.boundThis;
            trait = ejsGetPropertyTraits(ejs, lookup.obj, slotNum);
            if (trait->attributes & EJS_TRAIT_SETTER) {
                vp = lookup.obj;

            } else if (TYPE(vp)->hasInstanceVars && ejsIsPot(ejs, vp)) {
                /* The prototype properties have been inherited */
                assert(ejsIsPot(ejs, vp));
                slotNum = ejsCheckSlot(ejs, (EjsPot*) vp, slotNum);
                obj = (EjsPot*) vp;
                assert(slotNum < obj->numProp);
                assert(slotNum < ((EjsPot*) lookup.obj)->numProp);
                obj->properties->slots[slotNum].trait = ((EjsPot*) lookup.obj)->properties->slots[slotNum].trait;
                obj->properties->slots[slotNum].value = ((EjsPot*) lookup.obj)->properties->slots[slotNum].value;
                slotNum = ejsSetPropertyName(ejs, vp, slotNum, qname);
            } else {
                slotNum = -1;
            }
        } else {
            thisObj = vp = lookup.obj;
        }
    } else {
        thisObj = vp = fp->function.moduleInitializer ? ejs->global : (EjsObj*) fp;
        slotNum = ejsSetPropertyName(ejs, vp, slotNum, qname);
    }
    storePropertyToSlot(ejs, thisObj, vp, slotNum, value);
}


/*
    Run the module initializer
 */
EjsObj *ejsRunInitializer(Ejs *ejs, EjsModule *mp)
{
    EjsModule   *dp;
    EjsAny      *result;
    int         next;
    
    if (mp->initialized || !mp->hasInitializer) {
        mp->initialized = 1;
        result = ESV(null);
    } else {
        mp->initialized = 1;
        if (mp->dependencies) {
            for (next = 0; (dp = mprGetNextItem(mp->dependencies, &next)) != 0;) {
                if (dp->hasInitializer && !dp->initialized) {
                    if (ejsRunInitializer(ejs, dp) == 0) {
                        return 0;
                    }
                }
            }
        }
        mprTrace(7, "Running initializer for module %@", mp->name);
        result = ejsRunFunction(ejs, mp->initializer, ejs->global, 0, NULL);
    }
    return result;
}


/*
    Run all initializers for all modules
 */
int ejsRun(Ejs *ejs)
{
    EjsModule   *mp;
    int         next;

    //  OPT - should not examine all modules just to run a script
    for (next = 0; (mp = mprGetNextItem(ejs->modules, &next)) != 0;) {
        if (!mp->initialized) {
            ejs->result = ejsRunInitializer(ejs, mp);
        }
        if (ejsCompareAsc(ejs, mp->name, EJS_DEFAULT_MODULE) == 0) {
            ejsRemoveModule(ejs, mp);
            next--;
        }
        if (ejs->exception) {
            return EJS_ERR;
        }
    }
    return 0;
}


EjsAny *ejsRunFunction(Ejs *ejs, EjsFunction *fun, EjsAny *thisObj, int argc, void *argv)
{
    int     i;
    
    assert(ejs);
    assert(fun);
    assert(ejsIsFunction(ejs, fun));
    if (ejs->exception) {
        mprTrace(0, "STOP");
    }
    assert(ejs->exception == 0);

    if (ejs->exception) {
        return 0;
    }
    ejsClearAttention(ejs);
    
    if (thisObj == 0) {
        thisObj = fun->boundThis ? fun->boundThis : ejs->global;
    }
    if (ejsIsNativeFunction(ejs, fun)) {
        if (fun->body.proc == 0) {
            ejsThrowArgError(ejs, "Native function is not defined");
            return 0;
        }
        /* Push args so they get marked */
        for (i = 0; i < argc; i++) {
            pushOutside(ejs, ((EjsAny**) argv)[i]);
        }
        ejs->result = (fun->body.proc)(ejs, thisObj, argc, argv);
        ejs->state->stack -= argc;
        if (ejs->result == 0) {
            ejs->result = ESV(null);
        }

    } else {
        for (i = 0; i < argc; i++) {
            pushOutside(ejs, ((EjsAny**) argv)[i]);
        }
        VM(ejs, fun, thisObj, argc, 0);
        ejs->state->stack -= argc;
        if (ejs->exiting || mprIsStopping()) {
            ejsAttention(ejs);
        }
    }
    return (ejs->exception) ? 0 : ejs->result;
}


EjsAny *ejsRunFunctionBySlot(Ejs *ejs, EjsAny *thisObj, int slotNum, int argc, void *argv)
{
    EjsFunction     *fun;

    if (thisObj == 0) {
        thisObj = ejs->global;
    }
    if (thisObj == ejs->global) {
        fun = ejsGetProperty(ejs, thisObj, slotNum);
    } else if (ejsIsType(ejs, thisObj)) {
        fun = ejsGetProperty(ejs, thisObj, slotNum);
    } else {
        fun = ejsGetProperty(ejs, TYPE(thisObj)->prototype, slotNum);
    }
    if (fun == 0) {
        ejsThrowReferenceError(ejs, "Cannot find function at slot %d in %N", slotNum, &TYPE(thisObj)->qname);
        return 0;
    }
    return ejsRunFunction(ejs, fun, thisObj, argc, argv);
}


//  TODO - this is inconsistent with ejsRunBySlot. This has a separate container and thisObj, whereas RunBySlot
//  has only one arg

EjsAny *ejsRunFunctionByName(Ejs *ejs, EjsAny *container, EjsName qname, EjsAny *thisObj, int argc, void *argv)
{
    EjsFunction     *fun;
    EjsLookup       lookup;

    if (thisObj == 0) {
        thisObj = ejs->global;
    }
    if (container) {
        if ((fun = ejsGetPropertyByName(ejs, container, qname)) == 0) {
            ejsThrowReferenceError(ejs, "Cannot find function %N", qname);
            return 0;
        }
    } else {
        if (ejsLookupScope(ejs, qname, &lookup) == 0) {
            ejsThrowReferenceError(ejs, "Cannot find function %N", qname);
            return 0;
        }
        fun = ejsGetProperty(ejs, lookup.obj, lookup.slotNum);
    }
    return ejsRunFunction(ejs, fun, thisObj, argc, argv);
}



static void badArgType(Ejs *ejs, EjsFunction *fun, EjsPot *activation, EjsTrait *trait, int index)
{
    EjsName     qname;

    qname = ejsGetPropertyName(ejs, activation, index);
    ejsThrowTypeError(ejs, "Unacceptable null or undefined value for argument \"%@\" in function \"%@\"", qname.name, fun->name);
}


/*
    Validate the args. This routine handles ...rest args and parameter type checking and casts. Returns the new argc 
    or < 0 on errors.
 */
static int validateArgs(Ejs *ejs, EjsFunction *fun, int argc, void *args)
{
    EjsType     *type;
    EjsTrait    *trait;
    EjsArray    *rest;
    EjsPot      *activation;
    EjsObj      *newArg, **argv;
    int         nonDefault, i, limit, numRest;

    assert(ejs->exception == 0);
    assert(ejs->state->fp == 0 || ejs->state->fp->attentionPc == 0);

    argv = (EjsObj**) args;
    activation = fun->activation;
    nonDefault = fun->numArgs - fun->numDefault - fun->rest;

    if (argc < nonDefault) {
        if (!fun->rest || argc != (fun->numArgs - 1)) {
            if (fun->strict || (ejsIsNativeFunction(ejs, fun) && !fun->allowMissingArgs)) {
                ejsThrowArgError(ejs, "Insufficient actual parameters %d. Call requires %d parameter(s).", argc, nonDefault);
                return EJS_ERR;
            } else {
                /* Create undefined values for missing args for script functions */
                for (i = argc; i < nonDefault; i++) {
                    pushOutside(ejs, ESV(undefined));
                }
                argc = nonDefault;
            }
        }
    }
    if ((uint) argc > fun->numArgs && !fun->rest) {
        /*
            Discard excess arguments for scripted functions. No need to discard for native procs. This allows
            ejsDefineGlobalFunction to not have to bother with specifying the number of args for native procs.
         */
        if (!ejsIsNativeFunction(ejs, fun)) {
            ejs->state->stack -=  (argc - fun->numArgs);
            argc = fun->numArgs;
        }
    }

    /*
        Handle rest "..." args
     */
    if (fun->rest && (argc > nonDefault || fun->numDefault == 0)) {
        numRest = argc - fun->numArgs + 1;
        rest = ejsCreateArray(ejs, numRest);
        if (rest == 0) {
            return EJS_ERR;
        }
        for (i = numRest - 1; i >= 0; i--) {
            ejsSetProperty(ejs, rest, i, popOutside(ejs));
        }
        argc = argc - numRest + 1;
        pushOutside(ejs, rest);
        assert((void*) rest == argv[argc-1]);
    }

    /*
        Cast args to the right types
     */
    limit = min((uint) argc, fun->numArgs);
    for (i = 0; i < limit; i++) {
        if ((trait = ejsGetPropertyTraits(ejs, activation, i)) == 0 || trait->type == 0) {
            /* No trait - all to pass */
            continue;
        }
        type = trait->type;
        if (!ejsIsA(ejs, argv[i], type)) {
            if ((argv[i] == ESV(null) || argv[i] == ESV(undefined))) {
                if (trait->attributes & EJS_TRAIT_THROW_NULLS) {
                    badArgType(ejs, fun, activation, trait, i);
                    return EJS_ERR;
                }
                if (!(trait->attributes & EJS_TRAIT_CAST_NULLS)) {
                    continue;
                }
            }
            newArg = ejsCastType(ejs, argv[i], trait->type);
            if (ejs->exception) {
                ejsClearException(ejs);
                badArgType(ejs, fun, activation, trait, i);
                return EJS_ERR;
            }
            argv[i] = newArg;
        }
    }
    return argc;
}


static void callInterfaceInitializers(Ejs *ejs, EjsType *type)
{
    EjsType         *iface;
    EjsFunction     *fun;
    EjsName         qname;
    int             next;

    for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
        if (iface->hasInitializer) {
            qname = ejsGetPropertyName(ejs, iface, 0);
            //  TODO OPT. Could run all 
            fun = ejsGetPropertyByName(ejs, type, qname);
            if (fun && ejsIsFunction(ejs, fun)) {
                callFunction(ejs, fun, type, 0, 0);
            }
        }
    }
}


/*
    Push a block. Used by compiler.
 */
EjsBlock *ejsPushBlock(Ejs *ejs, EjsBlock *original)
{
    EjsBlock    *block;

    assert(!ejsIsFunction(ejs, original));

    block = ejsCloneBlock(ejs, original, 0);
    block->scope = ejs->state->bp;
    block->prev = ejs->state->bp;
    block->stackBase = ejs->state->stack;
    ejs->state->bp = block;
    return block;
}


/*
    Pop a block frame and return to the previous frame.  This pops functions and/or lexical blocks.
 */
EjsBlock *ejsPopBlock(Ejs *ejs)
{
    EjsBlock    *bp;

    bp = ejs->state->bp;
    ejs->state->stack = bp->stackBase;
    return ejs->state->bp = bp->prev;
}


/*
    Pop an exception block
 */
static EjsBlock *popExceptionBlock(Ejs *ejs)
{
    EjsBlock     *prev;

    if ((prev = ejs->state->bp->prev) != 0) {
        if (ejs->exception == 0) {
            ejs->exception = prev->prevException;
            prev->prevException = 0;
            if (ejs->exception) {
                /* Advance by one as checkExceptionHandlers expects the PC to be advanced after parsing the opcode */
                ejs->state->fp->pc++;
                ejsAttention(ejs);
            }
        }
    }
    ejs->state->bp = prev;
    return prev;
}


/*
    Manage exceptions. Bubble up the exception until we find an exception handler for it.
 */
static bool processException(Ejs *ejs)
{
    EjsState        *state;

    state = ejs->state;

    /*
        Check at each function level for a handler to process the exception.
     */
    while (state->fp) {
        checkExceptionHandlers(ejs);
        if (ejs->exception == 0) {
            return 1;
        }
        state->stack = state->fp->stackReturn;
        state->bp = state->fp->function.block.prev;
        state->fp = state->fp->caller;
        ejsClearAttention(ejs);
    }
    return 0;
}


static EjsEx *findExceptionHandler(Ejs *ejs, int kind)
{
    EjsEx       *ex;
    EjsFrame    *fp;
    EjsCode     *code;
    uint        pc;
    int         i;

    ex = 0;
    fp = ejs->state->fp;
    code = fp->function.body.code;
    pc = (uint) (fp->pc - code->byteCode - 1);

    /*
        Exception handlers are sorted with the inner most handlers first.
     */
    for (i = 0; i < code->numHandlers; i++) {
        ex = code->handlers[i];
        if (ex->tryStart <= pc && pc < ex->handlerEnd && (ex->flags & kind)) {
            if (ejsIsType(ejs, ejs->exception) && ((EjsType*) ejs->exception)->sid == S_StopIteration) {
                return ex;
            }
            assert(ex->catchType);
            if (kind == EJS_EX_FINALLY || ex->catchType->sid == S_Void) {
                return ex;
            }
            if (ejsIsA(ejs, ejs->exception, ex->catchType)) {
                return ex;
            }
        }
    }
    return 0;
}


static EjsEx *inHandler(Ejs *ejs, int kind)
{
    EjsEx       *ex;
    EjsFrame    *fp;
    EjsCode     *code;
    uint        pc;
    int         i;
    
    ex = 0;
    fp = ejs->state->fp;
    code = fp->function.body.code;
    pc = (uint) (fp->pc - code->byteCode - 1);
    
    /*
        Exception handlers are sorted with the inner most handlers first.
     */
    for (i = 0; i < code->numHandlers; i++) {
        ex = code->handlers[i];
        if (ex->handlerStart <= pc && pc < ex->handlerEnd && (ex->flags & kind)) {
            return ex;
        }
    }
    return 0;
}


/*
    Find the end of the last catch/finally handler.
 */
static uint findEndException(Ejs *ejs)
{
    EjsFrame    *fp;
    EjsEx       *ex;
    EjsCode     *code;
    uint        offset, pc;
    int         i;

    ex = 0;
    fp = ejs->state->fp;
    code = fp->function.body.code;
    pc = (uint) (fp->pc - code->byteCode - 1);
    offset = 0;

    for (i = 0; i < code->numHandlers; i++) {
        ex = code->handlers[i];
        /*
            Comparison must include try and all catch handlers, incase there are multiple catch handlers
         */
        if (ex->tryStart <= pc && pc < ex->handlerEnd) {
            offset = ex->handlerEnd;
            for (++i; i < code->numHandlers; i++) {
                /* Find the last handler of this try block. Use tryEnd as nested try blocks can start at the same location */
                if (ex->tryEnd == code->handlers[i]->tryEnd) {
                    offset = code->handlers[i]->handlerEnd;
                }
            }
        }
    }
    assert(offset);
    return offset;
}


/*
    Search for an exception handler at this level to process the exception. Return true if the exception is handled.
 */
static void checkExceptionHandlers(Ejs *ejs)
{
    EjsFrame        *fp;
    EjsCode         *code;
    EjsEx           *ex;

    ex = 0;
    fp = ejs->state->fp;
    code = fp->function.body.code;

    if (code->numHandlers == 0) {
        return;
    }
rescan:
    if (!fp->function.inException || (ejs->exception == EST(StopIteration))) {
        /*
            Normal exception in a try block. NOTE: the catch will jump or fall through to the finally block code.
            ie. We won't come here again for the finally code unless there is an exception in the catch block.
            Otherwise, No catch handler at this level and need to bubble up.
         */
        if ((ex = findExceptionHandler(ejs, EJS_EX_CATCH)) != 0) {
            createExceptionBlock(ejs, ex, ex->flags);
            return;
        }

    } else {
        /*
            Exception in a catch or finally block. If in a catch block, must first run the finally
            block before bubbling up. If in a finally block, we are done and upper levels will handle. We can be
            in a finally block and inException == 0. This happens because try blocks jump through directly
            into finally blocks (fast). But we need to check here if we are in the finally block explicitly.
         */
        if ((ex = inHandler(ejs, EJS_EX_FINALLY)) != 0) {
            /*
                If in a finally block, must advance the outer blocks's pc to be outside [tryStart .. finallyStart]
                This prevents this try block from handling this exception again.
             */
            SET_PC(fp, &fp->function.body.code->byteCode[ex->handlerEnd + 1]);
            fp->function.inCatch = fp->function.inException = 0;
            goto rescan;            
        }
    }

    /*
        Exception without a catch block or exception in a catch block. 
     */
    if ((ex = findExceptionHandler(ejs, EJS_EX_FINALLY)) != 0) {
        if (fp->function.inCatch) {
            popExceptionBlock(ejs);
        }
        createExceptionBlock(ejs, ex, EJS_EX_FINALLY);
    } else {
        fp->function.inCatch = fp->function.inException = 0;
    }
    ejsClearAttention(ejs);
}


/*
    Called for catch and finally blocks
 */
static void createExceptionBlock(Ejs *ejs, EjsEx *ex, int flags)
{
    EjsBlock        *block;
    EjsFrame        *fp;
    EjsState        *state;
    int             i, count;

    state = ejs->state;
    fp = state->fp;
    assert(ex);

    ejsClearAttention(ejs);

    if (flags & EJS_EX_ITERATION) {
        /*
            Empty handler is a special case for iteration. We simply do a break to the handler location
            which targets the end of the for/in loop.
         */
        SET_PC(fp, &fp->function.body.code->byteCode[ex->handlerStart]);
        ejs->exception = 0;
        return;
    }

    /*
        Discard all try lexical blocks when running a catch block
     */
    if (flags & EJS_EX_CATCH) {
        for (count = 0, block = state->bp; block != (EjsBlock*) state->fp; block = block->prev) {
            count++;
        }
        count -= ex->numBlocks;
        assert(count >= 0);
        for (i = 0; i < count && count > 0; i++) {
            ejsPopBlock(ejs);
        }
        count = (int) (state->stack - fp->stackBase);
        state->stack -= (count - ex->numStack);
        assert(state->stack >= fp->stackReturn);
    }
    
    /*
        Allocate a new frame in which to execute the handler
     */
    if ((block = ejsCreateBlock(ejs, 0)) == 0) {
        /*  Exception will continue to bubble up */
        return;
    }
    block->prev = block->scope = state->bp;
    block->stackBase = state->stack;
    state->bp = block;

    /*
        Move the PC outside of the try region. If this is a catch block, this allows the finally block to still
        be found. But if this is processing a finally block, the scanning for a matching handler will be forced
        to bubble up.
     */
    SET_PC(fp, &fp->function.body.code->byteCode[ex->handlerStart]);

    if (flags & EJS_EX_CATCH) {
        ejs->exceptionArg = ejs->exception;
        fp->function.inCatch = 1;

    } else {
        /*
            Mask the exception while processing the finally block
         */
        block->prev->prevException = ejs->exception;
        fp->function.inCatch = 0;
        ejsAttention(ejs);
    }
    ejs->exception = 0;
    fp->function.inException = 1;
}


typedef struct OperMap {
    int         opcode;
    cchar       *name;
} OperMap;

static OperMap operMap[] = {
        { EJS_OP_MUL,           "*"     },
        { EJS_OP_DIV,           "/"     },
        { EJS_OP_REM,           "%"     },
        { EJS_OP_COMPARE_LT,    "<"     },
        { EJS_OP_COMPARE_GT,    ">"     },
        { EJS_OP_COMPARE_LE,    "<="    },
        { EJS_OP_COMPARE_GE,    ">="    },
        { 0,                    0       },
};


static int lookupOverloadedOperator(Ejs *ejs, EjsOpCode opcode, EjsAny *lhs)
{
    EjsName     qname;
    int         i;

    for (i = 0; operMap[i].opcode; i++) {
        if (operMap[i].opcode == opcode) {
            qname = ejsName(ejs, "", operMap[i].name);
            break;
        }
    }
    return ejsLookupProperty(ejs, TYPE(lhs), qname);
}


/*
    Evaluate a binary expression.
    OPT -- simplify and move back inline into eval loop.
 */
static EjsAny *evalBinaryExpr(Ejs *ejs, EjsAny *lhs, EjsOpCode opcode, EjsAny *rhs)
{
    EjsAny      *result;
    int         slotNum;

    if (lhs == 0) {
        lhs = ESV(undefined);
    }
    if (rhs == 0) {
        rhs = ESV(undefined);
    }
    result = ejsInvokeOperator(ejs, lhs, opcode, rhs);

    if (result == 0 && ejs->exception == 0) {
        slotNum = lookupOverloadedOperator(ejs, opcode, lhs);
        if (slotNum >= 0) {
            result = ejsRunFunctionBySlot(ejs, lhs, slotNum, 1, &rhs);
        }
    }
    return result;
}


#if FUTURE
/*
    Grow the operand evaluation stack.
    Return a negative error code on memory allocation errors or if the stack grows too big.
 */
int ejsGrowStack(Ejs *ejs, int incr)
{
    EjsStack    *sp;
    EjsFrame    *frame;
    EjsObj      **bottom;
    int         i, size, moveBy;

    sp = ejs->stack;
    sp->ejs = ejs;

    incr = max(incr, EJS_STACK_INC);

    if (sp->bottom) {
        /*
            Grow an existing stack
         */
        size = sp->size + (sizeof(EjsObj*) * incr);
        bottom = (EjsObj**) mprRealloc(sp, sp->bottom, size);
        //  OPT - don't zeroed?
        memset(&bottom[sp->size], 0, (size - sp->size) * sizeof(EjsObj*));
        moveBy = (int) ((char*) bottom - (char*) sp->bottom);
        sp->top = (EjsObj**) ((char*) sp->top + moveBy);
        sp->bottom = bottom;

        /*
            Adjust all the argv pointers.
         */
        for (frame = ejs->frame; frame; frame = frame->prev) {
            if (frame->argv) {
                frame->argv = (EjsObj**) ((char*) frame->argv + moveBy);
            }
            frame->prevStackTop = (EjsObj**) ((char*) frame->prevStackTop + moveBy);
        }

    } else {
        /*
            Allocate a stack
         */
        if (sp->top >= &sp->bottom[BIT_MAX_EJS_STACK]) {
            return MPR_ERR_MEMORY;
        }
        size = (sizeof(EjsObj*) * incr);
        sp->bottom = (EjsObj**) mprAlloc(size);
        /*
            Push always begins with an increment of sp->top. Initially, sp_bottom points to the first (future) element.
         */
        sp->top = &sp->bottom[-1];
    }

    if (sp->bottom == 0) {
        return MPR_ERR_MEMORY;
    }

    sp->end = &sp->bottom[size / sizeof(EjsObj*)];
    sp->size = size;

    for (i = 1; i <= incr; i++) {
        sp->top[i] = 0;
    }
    return 0;
}
#endif


/*
    Exit the script
 */
void ejsExit(Ejs *ejs, int status)
{
    ejs->exiting = 1;
    mprSignalDispatcher(ejs->dispatcher);
}


static EjsName getNameArg(Ejs *ejs, EjsFrame *fp)
{
    EjsName     qname;

    qname.name = getStringArg(ejs, fp);
    qname.space = getStringArg(ejs, fp);
    return qname;
}


static EjsString *getString(Ejs *ejs, EjsFrame *fp, int num)
{
    return ejsCreateStringFromConst(ejs, fp->function.body.code->module, num);
}


static EjsString *getStringArg(Ejs *ejs, EjsFrame *fp)
{
    return getString(ejs, fp, (int) ejsDecodeNum(ejs, &fp->pc));
}


static EjsObj *getGlobalArg(Ejs *ejs, EjsFrame *fp)
{
    EjsObj      *obj;
    EjsName     qname;
    int         t, slotNum;

    t = (int) ejsDecodeNum(ejs, &fp->pc);
    if (t < 0) {
        return 0;
    }
    slotNum = -1;
    qname.name = 0;
    qname.space = 0;
    obj = 0;

    /*
        OPT. Could this encoding be optimized?
     */
    switch (t & EJS_ENCODE_GLOBAL_MASK) {
    default:
        assert(0);
        return 0;

    case EJS_ENCODE_GLOBAL_NOREF:
        return 0;

    case EJS_ENCODE_GLOBAL_SLOT:
        slotNum = t >> 2;
        if (0 <= slotNum && slotNum < ejsGetLength(ejs, ejs->global)) {
            obj = ejsGetProperty(ejs, ejs->global, slotNum);
        }
        break;

    case EJS_ENCODE_GLOBAL_NAME:
        qname.name = getString(ejs, fp, t >> 2);
        if (qname.name == 0) {
            assert(0);
            return 0;
        }
        qname.space = getStringArg(ejs, fp);
        if (qname.space == 0) {
            return 0;
        }
        if (qname.name) {
            obj = ejsGetPropertyByName(ejs, ejs->global, qname);
        }
        break;
    }
    return obj;
}


static void callProperty(Ejs *ejs, EjsAny *obj, int slotNum, EjsAny *thisObj, int argc, int stackAdjust)
{
    EjsTrait    *trait;
    EjsFunction *fun;

    fun = ejsGetProperty(ejs, obj, slotNum);
    trait = ejsGetPropertyTraits(ejs, obj, slotNum);
    if (trait && trait->attributes & EJS_TRAIT_GETTER) {
        fun = (EjsFunction*) ejsRunFunction(ejs, fun, thisObj, 0, NULL);
        if (ejs->exception) {
            return;
        }
    }
    callFunction(ejs, fun, thisObj, argc, stackAdjust);
}


/*
    Call a function. Supports both native and scripted functions. If native, the function is fully 
    invoked here. If scripted, a new frame is created and the pc adjusted to point to the new function.
 */
static void callFunction(Ejs *ejs, EjsFunction *fun, EjsAny *thisObj, int argc, int stackAdjust)
{
    EjsState        *state;
    EjsFrame        *fp;
    EjsType         *type;
    EjsObj          **argv;
    EjsObj          **sp;
    int             count, i, fstate;

    assert(fun);
    assert(ejs->exception == 0);
    assert(ejs->state->fp == 0 || ejs->state->fp->attentionPc == 0);  

    state = ejs->state;

    if (unlikely(ejsIsType(ejs, fun))) {
        type = (EjsType*) fun;
        if (thisObj == NULL) {
            thisObj = ejsCreateObj(ejs, type, 0);
        }
        ejs->result = thisObj;
        if (!type->hasConstructor) {
            ejs->state->stack -= (argc + stackAdjust);
            if (ejs->exiting || mprIsStopping()) {
                ejsAttention(ejs);
            }
            return;
        }
        
    } else if (!ejsIsFunction(ejs, fun)) {
        if (fun == ESV(undefined)) {
            ejsThrowReferenceError(ejs, "Function is undefined");
            return;
        } else {
            ejsThrowReferenceError(ejs, "Reference is not a function");
            return;
        }
    }
    if (thisObj == 0) {
        if ((thisObj = fun->boundThis) == 0) {
            thisObj = state->fp->function.boundThis;
        } 
    } 
    if (fun->boundArgs) {
        assert(ejsIs(ejs, fun->boundArgs, Array));
        count = fun->boundArgs->length;
        sp = &state->stack[1 - argc];
        for (i = argc - 1; i >= 0; i--) {
            sp[i + count] = sp[i];
        }
        for (i = 0; i < count; i++) {
            *sp++ = fun->boundArgs->data[i];
        }
        state->stack += count;
        argc += count;
    }
    
    assert(ejs->spreadArgs == 0);
    argc += ejs->spreadArgs;
    ejs->spreadArgs = 0;
    
    /*
        Validate the args. Cast to the right type, handle rest args and return with argc adjusted.
     */
    argv = NULL;
    if (argc > 0 || fun->numArgs || fun->rest) {
        argv = &(state->stack[1 - argc]);
        if ((argc = validateArgs(ejs, fun, argc, argv)) < 0) {
            return;
        }
    }
    if (ejs->exception) {
        return;
    }
    if (ejsIsNativeFunction(ejs, fun)) {
        if (fun->body.proc == 0) {
            ejsThrowInternalError(ejs, "Native function is not defined: %@", fun->name);
            return;
        }
        ejsClearAttention(ejs);
        fstate = state->paused;
        ejs->result = (fun->body.proc)(ejs, thisObj, argc, argv);
        state->paused = fstate;
        if (ejs->result == 0) {
            ejs->result = ESV(null);
        }
        state->stack -= (argc + stackAdjust);

    } else {
        if (fun->body.code && fun->body.code->debug) {
            assert(fun->body.code->debug->magic == EJS_DEBUG_MAGIC);
        }
        assert(thisObj);
        if ((fp = ejsCreateFrame(ejs, fun, thisObj, argc, argv)) == 0) {
            return;
        }
        fp->function.block.prev = state->bp;
        fp->caller = state->fp;
        fp->stackBase = state->stack;
        fp->stackReturn = state->stack - argc - stackAdjust;
        state->fp = fp;
        state->bp = (EjsBlock*) fp;
        ejsClearAttention(ejs);
    }
    assert(ejs->state->fp);
}


static void throwNull(Ejs *ejs)
{
    ejsThrowReferenceError(ejs, "Object reference is null");
}


/*
    Object can be an instance or a type. If an instance, then step to the immediate base type to begin the count.
 */
static EjsAny *getNthBase(Ejs *ejs, EjsAny *vp, int nthBase)
{
    EjsType     *type;

    if (vp) {
        if (ejsIsType(ejs, vp) || vp == ejs->global) {
            type = (EjsType*) vp;
        } else {
            type = TYPE(vp);
            nthBase--;
        }
        for (; type && nthBase > 0; type = type->baseType) {
            nthBase--;
        }
        if (nthBase > 0) {
            ejsThrowReferenceError(ejs, "Cannot find correct base class");
            return 0;
        }
        vp = type;
    }
    return vp;
}


static EjsAny *getNthBaseFromBottom(Ejs *ejs, EjsAny *vp, int nthBase)
{
    EjsType     *type, *tp;
    int         count;

    if (vp) {
        if (ejsIsType(ejs, vp) || vp == ejs->global) {
            type = (EjsType*) vp;
        } else {
            type = TYPE(vp);
        }
        for (count = 0, tp = type->baseType; tp; tp = tp->baseType) {
            count++;
        }
        nthBase = count - nthBase;
        for (; type && nthBase > 0; type = type->baseType) {
            nthBase--;
        }
        vp = type;
    }
    return vp;
}


static EjsAny *getNthBlock(Ejs *ejs, int nth)
{
    EjsBlock    *block;

    assert(ejs);
    assert(nth >= 0);

    for (block = ejs->state->bp; block && --nth >= 0; ) {
        /* TODO - this is done for loading scripts into ejs. Really the compiler should remove these blocks */
        block = block->scope;
    }
    return block;
}


/*
    Enter a mesage into the log file
 */
void ejsLog(Ejs *ejs, cchar *fmt, ...)
{
    va_list     args;
    char        buf[BIT_MAX_BUFFER];

    va_start(args, fmt);
    fmtv(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    mprLog(0, "%s", buf);
}


#if FUTURE
#if BIT_HAS_LIB_EDIT
static History  *cmdHistory;
static EditLine *eh; 
static cchar    *prompt;

static cchar *issuePrompt(EditLine *e) {
    return prompt;
}

static EditLine *initEditLine()
{
    EditLine    *e;
    HistEvent   ev; 

    cmdHistory = history_init(); 
    history(cmdHistory, &ev, H_SETSIZE, 100); 
    e = el_init("ejs", stdin, stdout, stderr); 
    el_set(e, EL_EDITOR, "vi");
    el_set(e, EL_HIST, history, cmdHistory);
    el_source(e, NULL);
    return e;
}


/*  
    Prompt for input with the level of current nest (block nest depth)
 */
static char *readline(cchar *msg) 
{ 
    HistEvent   ev; 
    cchar       *str; 
    char        *result;
    int         len, count; 
 
    if (eh == NULL) { 
        eh = initEditLine();
    }
    prompt = msg;
    el_set(eh, EL_PROMPT, issuePrompt);
    str = el_gets(eh, &count); 
    if (str && count > 0) { 
        result = strdup(str); 
        len = strlen(result);
        if (result[len - 1] == '\n') {
            result[len - 1] = '\0'; 
        }
        count = history(cmdHistory, &ev, H_ENTER, result); 
        return result; 
    }  
    return NULL; 
} 

#else

static char *readline(cchar *msg)
{
    char    buf[MPR_MAX_STRING];

    printf("%s", msg);
    if (fgets(buf, sizeof(buf) - 1, stdin) == 0) {
        return NULL;
    }
    return strdup(buf);
}
#endif


typedef struct EjsBreakpoint {
    cchar   *filename;
    int     lineNumber;
    int     opcode;    
} EjsBreakpoint;


static void bkpt(Ejs *ejs)
{
    EjsFrame        *fp;
    EjsState        *state;
    EjsBreakpoints  *breakpoints;
    EjsOptable      *optable;
    int             len;
    uint            offset;
    static int      once = 0;
    static int      stop = 1;

    state = ejs->state;
    fp = state->fp;
    opcount[opcode]++;

    breakpoints = ejs->breakpoints;

    offset = (uint) (fp->pc - fp->function.body.code.byteCode) - 1;
    if (offset < 0) {
        offset = 0;
    }
    str = readline("edb> ");

    Display source with current line highlighted

    optable = ejsGetOptable();
    if (mprGetLogLevel(ejs) > 7) {
        mprPrintf(ejs, "%0s %04d: [%d] %02x: %-35s # %s:%d %@",
            mprGetCurrentThreadName(fp), offset, (int) (state->stack - fp->stackReturn),
            (uchar) opcode, optable[opcode].name, fp->filename, fp->lineNumber, fp->currentLine);
    if (stop && once++ == 0) {
        mprNap(0);
    }
    assert(state->stack >= fp->stackReturn);
}
#endif


#if DEBUG_IDE
/*
    This code is only active when building in debug mode and debugging in an IDE
 */
static int ejsOpCount = 0;
static int doDebug = 1;

static EjsOpCode traceCode(Ejs *ejs, EjsOpCode opcode)
{
    EjsFrame        *fp;
    EjsState        *state;
    int             offset;
#if FUTURE
    EjsOptable      *optable;
    static int      showFrequency = 1;
#endif

    state = ejs->state;
    fp = state->fp;
    opcount[opcode]++;
    // assert(ejs->exception || (state->stack >= fp->stackReturn));

    if (1 || (ejs->initialized && doDebug)) {
        offset = (int) (fp->pc - fp->function.body.code->byteCode) - 1;
        if (offset < 0) {
            offset = 0;
        }
        fp->line = ejsGetDebugLine(ejs, (EjsFunction*) fp, fp->pc);
#if FUTURE
        optable = ejsGetOptable();
        if (showFrequency && ((once % 1000) == 999)) {
            ejsShowOpFrequency(ejs);
        }
#endif
        // assert(ejs->exception || (state->stack >= fp->stackReturn));
    }
    ejsOpCount++;
    return opcode;
}


#if FUTURE
void ejsShowOpFrequency(Ejs *ejs)
{
    EjsOptable      *optable;
    int             i;

    if (mprGetLogLevel(ejs) < 6) {
        return;
    }
    optable = ejsGetOptable();
    mprTrace(0, "Opcode Frequency");
    for (i = 0; i < 256 && optable[i].name; i++) {
        mprTrace(6, "%4d %24s %8d", (uchar) i, optable[i].name, opcount[i]);
    }
}
#endif

#endif /* BIT_DEBUG */


/*
    Cleanup defines for combo builds
 */
#undef top
#undef pop
#undef push
#undef popString
#undef popOutside
#undef pushOutside
#undef FRAME
#undef FUNCTION
#undef BLOCK
#undef SWAP
#undef GET_SLOT
#undef SET_SLOT
#undef GET_BYTE
#undef GET_DOUBLE
#undef GET_INT
#undef GET_NUM
#undef GET_NAME
#undef GET_STRING
#undef GET_TYPE
#undef GET_WORD
#undef THIS
#undef FILL
#undef CASE
#undef BREAK

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
