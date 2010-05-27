/*
    ejsInterp.c - Virtual Machine Interpreter for Ejscript.

    Copyright (c) All Rights Reserved. See details at the end of the file.

    NEXT
        - Optimize and cache stack.top
        - Fix BAD BINDING
        - Move DEBUG op codes out of line
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/********************************** Inline Code *******************************/
/*
    The stack is a stack of pointers to EjsObj. The top of stack (stack.top) always points to the current top item 
    on the stack. To push a new value, top is incremented then the value is stored. To pop, simply copy the value at 
    top and decrement top ptr.
 */
#define top                     (*state.stack)
#define pop(ejs)                (*state.stack--)

#define push(value)             (*(++(state.stack))) = ((EjsObj*) (value))
#define popString(ejs)          ((EjsString*) pop(ejs))
#define popOutside(ejs)         *(ejs->state->stack)--
#define pushOutside(ejs, value) (*(++(ejs->state->stack))) = ((EjsObj*) (value))

#define FRAME                   state.fp
#define FUNCTION                state.fp.function
#define BLOCK                   state.bp

#define SWAP if (1) { \
        EjsObj *swap = state.stack[0]; \
        state.stack[0] = state.stack[-1]; \
        state.stack[-1] = swap; \
    }

#define TRACE if (1) { \
        FRAME->filename = GET_STRING(); \
        FRAME->lineNumber = GET_INT(); \
        FRAME->currentLine = GET_STRING(); \
    }


static void callFunction(Ejs *ejs, EjsFunction *fun, EjsObj *thisObj, int argc, int stackAdjust);

static MPR_INLINE void getPropertyFromSlot(Ejs *ejs, EjsObj *thisObj, EjsObj *obj, int slotNum) 
{
    EjsFunction     *fun, *value;

    if (ejsHasTrait(obj, slotNum, EJS_TRAIT_GETTER)) {
        fun = ejsGetProperty(ejs, obj, slotNum);
        callFunction(ejs, fun, thisObj, 0, 0);
        if (ejsIsNativeFunction(fun)) {
            pushOutside(ejs, ejs->result);
        } else {
            ejs->state->fp->getter = 1;
        }
        return;
    }
    value = ejsGetProperty(ejs, obj, slotNum);
    if (ejsIsFunction(value)) {
        fun = (EjsFunction*) value;
        if (!fun->thisObj && thisObj) {
            /* Function extraction. Bind the "thisObj" into a clone of the function */
            fun = ejsCloneFunction(ejs, fun, 0);
            fun->thisObj = thisObj;
        }
    }
    pushOutside(ejs, value);
}

#define GET_SLOT(thisObj, obj, slotNum) getPropertyFromSlot(ejs, (EjsObj*) thisObj, (EjsObj*) obj, slotNum)

static MPR_INLINE void checkGetter(Ejs *ejs, EjsObj *value, EjsObj *thisObj, EjsObj *obj, int slotNum) 
{
    EjsFunction     *fun;

    if (ejsIsFunction(value) && !ejsIsType(value)) {
        fun = (EjsFunction*) value;
        if (ejsHasTrait(obj, slotNum, EJS_TRAIT_GETTER)) {
            if (fun->staticMethod) {
                thisObj = obj;
            }
            callFunction(ejs, fun, thisObj, 0, 0);
            if (ejsIsNativeFunction(fun)) {
                pushOutside(ejs, ejs->result);
            } else {
                ejs->state->fp->getter = 1;
            }
            return;
        } else {
            if (!fun->thisObj && thisObj) {
                /* Function extraction. Bind the "thisObj" into a clone of the function */
                fun = ejsCloneFunction(ejs, fun, 0);
                fun->thisObj = thisObj;
                value = (EjsObj*) fun;
            }
        }
    } else if (value == 0) {
        value = ejs->undefinedValue;
    }
    pushOutside(ejs, value);
}

#define CHECK_VALUE(value, thisObj, obj, slotNum) \
    checkGetter(ejs, (EjsObj*) value, (EjsObj*) thisObj, (EjsObj*) obj, slotNum)

/*
    Set a slot value when we don't know if the object is an EjsObj
 */
#define SET_SLOT(thisObj, obj, slotNum, value) \
    storePropertyToSlot(ejs, (EjsObj*) thisObj, (EjsObj*) obj, slotNum, (EjsObj*) value)

/*
    Must clear attentionPc when changing the PC. Otherwise the next instruction will jump to a bad (stale) location.
 */
#define SET_PC(fp, value) \
    if (1) { \
        (fp)->pc = (uchar*) (value); \
        (fp)->attentionPc = 0; \
    } else 

#define GET_BYTE()      *(FRAME)->pc++
#define GET_DOUBLE()    ejsDecodeDouble(ejs, &(FRAME)->pc)
#define GET_INT()       (int) ejsDecodeNum(&(FRAME)->pc)
#define GET_NUM()       ejsDecodeNum(&(FRAME)->pc)
#define GET_NAME()      getNameArg(FRAME)
#define GET_STRING()    getStringArg(FRAME)
#define GET_TYPE()      ((EjsType*) getGlobalArg(ejs, FRAME))
#define GET_WORD()      ejsDecodeWord(&(FRAME)->pc)
#undef THIS
#define THIS            FRAME->function.thisObj
#define FILL(mark)      while (mark < FRAME->pc) { *mark++ = EJS_OP_NOP; }

#if BLD_DEBUG
    static EjsOpCode traceCode(Ejs *ejs, EjsOpCode opcode);
    static int opcount[256];
#else
    #define traceCode(ejs, opcode) opcode
#endif

#if BLD_UNIX_LIKE || VXWORKS
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
static void callProperty(Ejs *ejs, EjsObj *obj, int slotNum, EjsObj *thisObj, int argc, int stackAdjust);
static void checkExceptionHandlers(Ejs *ejs);
static void createExceptionBlock(Ejs *ejs, EjsEx *ex, int flags);
static EjsObj *evalBinaryExpr(Ejs *ejs, EjsObj *lhs, EjsOpCode opcode, EjsObj *rhs);
static inline uint findEndException(Ejs *ejs);
static inline EjsEx *findExceptionHandler(Ejs *ejs, int kind);
static EjsName getNameArg(EjsFrame *fp);
static EjsObj *getNthBase(Ejs *ejs, EjsObj *obj, int nthBase);
static EjsObj *getNthBaseFromBottom(Ejs *ejs, EjsObj *obj, int nthBase);
static EjsObj *getNthBlock(Ejs *ejs, int nth);
static char *getStringArg(EjsFrame *fp);
static EjsObj *getGlobalArg(Ejs *ejs, EjsFrame *fp);
static bool manageExceptions(Ejs *ejs);
static EjsBlock *popExceptionBlock(Ejs *ejs);
static void storeProperty(Ejs *ejs, EjsObj *thisObj, EjsObj *obj, EjsName *name, EjsObj *value, bool dup);
static void storePropertyToSlot(Ejs *ejs, EjsObj *thisObj, EjsObj *obj, int slotNum, EjsObj *value);
static void storePropertyToScope(Ejs *ejs, EjsName *qname, EjsObj *value, bool dup);
static void throwNull(Ejs *ejs);

/************************************* Code ***********************************/
/*
    Virtual Machine byte code evaluation
 */
static void VM(Ejs *ejs, EjsFunction *fun, EjsObj *otherThis, int argc, int stackAdjust)
{
    EjsState    state;
    EjsName     qname;
    EjsObj      *result, *vp, *v1, *v2, *obj, *value;
    int         slotNum, nthBase;

    EjsBlock    *blk;
    EjsObj      *vobj, *global, *thisObj;
    EjsString   *nameVar, *spaceVar;
    EjsType     *type;
    EjsLookup   lookup;
    EjsEx       *ex;
    EjsFrame    *newFrame;
    EjsFunction *f1, *f2;
    EjsNamespace *nsp;
    char        *str;
    uchar       *mark;
    int         i, offset, count, opcode, attributes;

#if BLD_UNIX_LIKE || VXWORKS 
    /*
        Direct threading computed goto processing. Include computed goto jump table.
     */
    #include    "ejsByteGoto.h"
#endif
    mprAssert(ejs);
    mprAssert(!mprHasAllocError(ejs));
    mprAssert(!ejs->exception);
    mprAssert(ejs->state->fp == 0 || ejs->state->fp->attentionPc == 0);

    vp = 0;
    slotNum = -1;
    global = (EjsObj*) ejs->global;

    state = *ejs->state;
    state.prev = ejs->state;
    ejs->state = &state;

    callFunction(ejs, fun, otherThis, argc, stackAdjust);
    FRAME->caller = 0;
    FRAME->currentLine = 0;
    FRAME->filename = 0;
    FRAME->lineNumber = 0;

    mprAssert(!FRAME->attentionPc);

#if BLD_UNIX_LIKE || VXWORKS 
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
            Symbolic source code debug information
                Debug <filename> <lineNumber> <sourceLine>
         */
        CASE (EJS_OP_DEBUG):
            TRACE; BREAK;

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
                ejs->result = ejs->undefinedValue;
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
            state.stack = FRAME->stackReturn;
            if (ejs->result) {
                f1 = &FRAME->function;
                if (FRAME->function.resultType) {
                    type = FRAME->function.resultType;
                    //  MOB remove this voidType
                    if (type != ejs->voidType && !ejsIsA(ejs, ejs->result, type)) {
                        if (ejs->result == ejs->nullValue || ejs->result == ejs->undefinedValue) {
                            if (FRAME->function.throwNulls) {
                                ejsThrowTypeError(ejs, "Unacceptable null or undefined return value");
                                BREAK;
                            } else if (FRAME->function.castNulls) {
                                ejs->result = ejsCast(ejs, ejs->result, type);
                                if (ejs->exception) {
                                    BREAK;
                                }
                            }
                        } else {
                            ejs->result = ejsCast(ejs, ejs->result, type);
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
            state.bp = FRAME->function.block.prev;
            newFrame = FRAME->caller;
            FRAME = newFrame;
            BREAK;

        /*
            Return from a function without a result
                Return
         */
        CASE (EJS_OP_RETURN):
            ejs->result = ejs->undefinedValue;
            if (FRAME->caller == 0) {
                goto done;
            }
            state.stack = FRAME->stackReturn;
            state.bp = FRAME->function.block.prev;
            newFrame = FRAME->caller;
            FRAME = newFrame;
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
            str = GET_STRING();
            push(ejsCreateString(ejs, str));
            BREAK;

        /*
            Load a namespace constant
                LoadNamespace       <UriString>
                Stack before (top)  []
                Stack after         [Namespace]
         */
        CASE (EJS_OP_LOAD_NAMESPACE):
            str = GET_STRING();
            push(ejsCreateNamespace(ejs, str, str));
            BREAK;


        /*
            Load an XML constant
                LoadXML             <xmlString>
                Stack before (top)  []
                Stack after         [XML]
         */
        CASE (EJS_OP_LOAD_XML):
            v1 = (EjsObj*) ejsCreate(ejs, ejs->xmlType, 0);
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
            str = GET_STRING();
            v1 = (EjsObj*) ejsCreateRegExp(ejs, str);
            push(v1);
            BREAK;

        /*
            Load a null constant
                LoadNull
                Stack before (top)  []
                Stack after         [Null]
         */
        CASE (EJS_OP_LOAD_NULL):
            push(ejs->nullValue);
            BREAK;

        /*
            Load a void / undefined constant
                LoadUndefined
                Stack before (top)  []
                Stack after         [undefined]
         */
        CASE (EJS_OP_LOAD_UNDEFINED):
            push(ejs->undefinedValue);
            BREAK;

        /*
            Load the "this" value
                LoadThis
                Stack before (top)  []
                Stack after         [this]
         */
        CASE (EJS_OP_LOAD_THIS):
            push(THIS);
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
            push(ejs->trueValue);
            BREAK;

        /*
            Load the "false" value
                LoadFalse
                Stack before (top)  []
                Stack after         [false]
         */
        CASE (EJS_OP_LOAD_FALSE):
            push(ejs->falseValue);
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
            obj = (EjsObj*) getNthBlock(ejs, GET_INT());
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
            thisObj = (EjsObj*) pop(ejs);
            vp = getNthBase(ejs, (EjsObj*) thisObj, GET_INT());
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
                //  MOB -- is this the right "this". Should not be an instance?
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
            mark = FRAME->pc - 1;
            qname = GET_NAME();
            vp = ejsGetVarByName(ejs, NULL, &qname, &lookup);
            if (unlikely(vp == 0)) {
                ejsThrowReferenceError(ejs, "%s is not defined", qname.name);
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
            mark = FRAME->pc - 1;
            qname.name = ejsToString(ejs, pop(ejs))->value;
            v1 = pop(ejs);
            if (ejsIsNamespace(v1)) {
                qname.space = ((EjsNamespace*) v1)->uri;
            } else {
                qname.space = ejsToString(ejs, v1)->value;
            }
            vp = ejsGetVarByName(ejs, NULL, &qname, &lookup);
            if (unlikely(vp == 0)) {
                ejsThrowReferenceError(ejs, "%s is not defined", qname.name);
            } else {
                CHECK_VALUE(vp, NULL, lookup.obj, lookup.slotNum);
            }
#if DYNAMIC_BINDING
            if (ejs->flags & EJS_FLAG_COMPILER || lookup.obj->type == ejs->objectType || lookup.slotNum >= 4096) {
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
            //  TODO OPT - remove vp == 0
            if (vp == 0 || vp == ejs->nullValue || vp == ejs->undefinedValue) {
                ejsThrowReferenceError(ejs, "Object reference is null");
                BREAK;
            }
            v1 = ejsGetVarByName(ejs, vp, &qname, &lookup);
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
                
            } else if (ejsIsType(lookup.obj) && ejsIsA(ejs, THIS, (EjsType*) lookup.obj)) {
                *mark++ = EJS_OP_GET_TYPE_SLOT;
                mark += ejsEncodeUint(mark, lookup.slotNum);
                mark += ejsEncodeUint(mark, lookup.nthBlock + 1);

            } else if ((EjsObj*) vp->type == lookup.obj) {
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
            if (vp->type != type) {
                GET_SLOT(thisObj, vp, slotNum);
            } else {
                v1 = ejsGetVarByName(ejs, vp, &qname, &lookup);
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
            if (vp == ejs->nullValue || vp == ejs->undefinedValue) {
                ejsThrowReferenceError(ejs, "Object reference is null");
                BREAK;
            }
            v1 = ejsGetVarByName(ejs, vp, &qname, &lookup);
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
            if (vp == 0 || vp == ejs->nullValue || vp == ejs->undefinedValue) {
                ejsThrowReferenceError(ejs, "Object reference is null");
                BREAK;
            }
            if (vp->type->numericIndicies && ejsIsNumber(v1)) {
                vp = ejsGetProperty(ejs, vp, ejsGetInt(ejs, v1));
                push(vp == 0 ? ejs->nullValue : vp);
                BREAK;
            } else {
                qname.name = ejsToString(ejs, v1)->value;
                if (ejsIsNamespace(v2)) {
                    qname.space = ((EjsNamespace*) v2)->uri;
                } else {
                    qname.space = ejsToString(ejs, v2)->value;
                }
                v2 = ejsGetVarByName(ejs, vp, &qname, &lookup);
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
            obj = (EjsObj*) getNthBlock(ejs, GET_INT());
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
            obj = (EjsObj*) getNthBlock(ejs, GET_INT());
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
            storePropertyToScope(ejs, &qname, value, 0);
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
            qname.name = ejsToString(ejs, pop(ejs))->value;
            v1 = pop(ejs);
            if (ejsIsNamespace(v1)) {
                qname.space = ((EjsNamespace*) v1)->uri;
            } else {
                qname.space = ejsToString(ejs, v1)->value;
            }
            value = pop(ejs);
            storePropertyToScope(ejs, &qname, value, 1);
            BREAK;

        /*
            Store a property by property name to an object
                PutObjName
                MOB - is this right order?
                Stack before (top)  [objRef]
                                    [value]
                Stack after         []
         */
        CASE (EJS_OP_PUT_OBJ_NAME):
            qname = GET_NAME();
            obj = pop(ejs);
            value = pop(ejs);
            storeProperty(ejs, obj, obj, &qname, value, 0);
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
            //  MOB -- cleanup this too - push into storeProperty
            if (obj->type->numericIndicies && ejsIsNumber(v1)) {
                ejsSetProperty(ejs, obj, ejsGetInt(ejs, v1), value);
            } else {
                qname.name = ejsToString(ejs, v1)->value;
                if (ejsIsNamespace(v2)) {
                    qname.space = ((EjsNamespace*) v2)->uri;
                } else {
                    qname.space = ejsToString(ejs, v2)->value;
                }
                if (qname.name && qname.space) {
                    storeProperty(ejs, obj, obj, &qname, value, 1);
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
            vp = state.stack[-argc - 1];
            callFunction(ejs, (EjsFunction*) state.stack[-argc], vp, argc, 2);
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
            vp = state.stack[-argc];
            if (vp == ejs->nullValue || vp == ejs->undefinedValue) {
                //  MOB -- refactor
                if (vp && (slotNum == ES_Object_iterator_get || slotNum == ES_Object_iterator_getValues)) {
                    callProperty(ejs, (EjsObj*) vp->type, slotNum, vp, argc, 1);
                } else {
                    ejsThrowReferenceError(ejs, "Object reference is null or undefined");
                }
            } else {
                //  MOB -- need a function to invoke 
                callProperty(ejs, (EjsObj*) vp->type->prototype, slotNum, vp, argc, 1);
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
            obj = (EjsObj*) THIS->type->prototype;
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
            obj = (EjsObj*) getNthBlock(ejs, GET_INT());
            argc = GET_INT();
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
            vp = state.stack[-argc];
            if (vp == 0 || vp == ejs->nullValue || vp == ejs->undefinedValue) {
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
            vp = state.stack[-argc];
            if (vp == ejs->nullValue || vp == ejs->undefinedValue) {
                throwNull(ejs);
            } else {
                type = (EjsType*) getNthBase(ejs, vp, nthBase);
                callProperty(ejs, (EjsObj*) type, slotNum, (EjsObj*) type, argc, 1);
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
            type = (EjsType*) getNthBase(ejs, THIS, nthBase);
            if (type == ejs->objectType) {
                //  TODO - remove
                ejsThrowReferenceError(ejs, "Bad type reference");
                BREAK;
            }
            callProperty(ejs, (EjsObj*) type, slotNum, (EjsObj*) type, argc, 0);
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
            vp = state.stack[-argc];
            if (vp == 0) {
                ejsThrowReferenceError(ejs, "%s is not defined", qname.name);
                throwNull(ejs);
                BREAK;
            }
            slotNum = ejsLookupVar(ejs, (EjsObj*) vp, &qname, &lookup);
            if (slotNum < 0) {
                ejsThrowReferenceError(ejs, "Can't find function \"%s\"", qname.name);
            } else {
                EjsTrait *trait = ejsGetTrait(lookup.obj, slotNum);
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
            slotNum = ejsLookupScope(ejs, &qname, &lookup);
            if (slotNum < 0) {
                ejsThrowReferenceError(ejs, "Can't find method %s", qname.name);
                BREAK;
            }
            fun = ejsGetProperty(ejs, lookup.obj, slotNum);
            if (ejsIsType(fun)) {
                type = (EjsType*) fun;
                callFunction(ejs, fun, NULL, argc, 0);

            } else if (!ejsIsFunction(fun)) {
                if ((EjsObj*) vp == (EjsObj*) ejs->undefinedValue) {
                    ejsThrowReferenceError(ejs, "Function is undefined");
                } else {
                    ejsThrowReferenceError(ejs, "Reference is not a function");
                }
            } else {
                /*
                    Calculate the "this" to use for the function. If required function is a method in the current 
                    "this" object use the current thisObj. If the lookup.obj is a type, then use it. Otherwise global.
                 */
                if ((vp = fun->thisObj) == 0) {
                    if (lookup.obj == THIS) {
                        vp = THIS;
                    } else if (lookup.obj->isPrototype && ejsIsA(ejs, THIS, lookup.type)) {
                        vp = THIS;
                    } else if (ejsIsType(lookup.obj)) {
                        vp = lookup.obj;
                    } else {
                        vp = ejs->global;
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
            vp = state.stack[-argc];
            if (vp == 0 || vp == ejs->nullValue || vp == ejs->undefinedValue) {
                throwNull(ejs);
                BREAK;
            }
            type = vp->type;
            mprAssert(type);
            if (type && type->constructor.block.obj.isFunction) {
                mprAssert(type->prototype);
                callFunction(ejs, (EjsFunction*) type, (EjsObj*) vp, argc, 0);
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
            type = ejsGetPropertyByName(ejs, ejs->global, &qname);
            if (type == 0) {
                ejsThrowReferenceError(ejs, "Can't find constructor %s", qname.name);
            } else {
                mprAssert(type->constructor.block.obj.isFunction);
                callFunction(ejs, (EjsFunction*) type, THIS, argc, 0);
            }
            BREAK;

        /*
            Add a literal namespace to the set of open namespaces for the current block
                AddNamespace <string>
         */
        CASE (EJS_OP_ADD_NAMESPACE):
            str = GET_STRING();
            nsp = ejsCreateNamespace(ejs, str, str);
            ejsAddNamespaceToBlock(ejs, state.bp, nsp);
            if (strstr(str, "internal-")) {
                state.internal = nsp;
            }
            BREAK;

        /*
            Add a namespace expression (reference) to the set of open namespaces for the current block. (use namespace).
                Stack before (top)  [namespace]
                Stack after         []
                AddNamespaceRef
         */
        CASE (EJS_OP_ADD_NAMESPACE_REF):
            ejsAddNamespaceToBlock(ejs, state.bp, (EjsNamespace*) pop(ejs));
            BREAK;

        /*
            Push a new scope block on the scope chain
                OpenBlock <slotNum> <nthBlock>
         */
        CASE (EJS_OP_OPEN_BLOCK):
            slotNum = GET_INT();
            vp = getNthBlock(ejs, GET_INT());
            v1 = ejsGetProperty(ejs, vp, slotNum);
            if (!ejsIsBlock(v1)) {
                ejsThrowReferenceError(ejs, "Reference is not a block");
                BREAK;
            }
            //  OPT
            blk = ejsCloneBlock(ejs, (EjsBlock*) v1, 0);
            blk->prev = blk->scope = state.bp;
            state.bp = blk;
            blk->stackBase = state.stack;
            ejsSetDebugName(state.bp, mprGetName(v1));
            BREAK;

        /*
            Add a new scope block from the stack onto on the scope chain
                OpenWith
         */
        CASE (EJS_OP_OPEN_WITH):
            vp = pop(ejs);
            blk = ejsCreateBlock(ejs, 0);
            ejsSetDebugName(blk, "with");
            //  MOB -- looks bugged. Can overwrite block.
            memcpy((void*) blk, vp, vp->type->instanceSize);
            blk->prev = blk->scope = state.bp;
            state.bp = blk;
            BREAK;

        /*
            Store the top scope block off the scope chain
                CloseBlock
                CloseWith
         */
        CASE (EJS_OP_CLOSE_BLOCK):
            state.bp = state.bp->prev;
            BREAK;

        /*
            Define a class and initialize by calling any static initializer.
                DefineClass <type>
         */
        CASE (EJS_OP_DEFINE_CLASS):
            type = GET_TYPE();
            if (type == 0 || !ejsIsType(type)) {
                ejsThrowReferenceError(ejs, "Reference is not a class");
            } else {
                type->constructor.block.scope = state.bp;
                if (type && type->hasInitializer) {
                    fun = ejsGetProperty(ejs, (EjsObj*) type, 0);
                    callFunction(ejs, fun, (EjsObj*) type, 0, 0);
                    if (type->implements && !ejs->exception) {
                        callInterfaceInitializers(ejs, type);
                    }
                    state.bp = &FRAME->function.block;
                }
            }
            ejs->result = (EjsObj*) type;
            BREAK;

        /*
            Define a function. This is used only for non-method functions to capture the scope chain.
                DefineFunction <slot> <nthBlock>
         */
        CASE (EJS_OP_DEFINE_FUNCTION):
            qname = GET_NAME();
            if ((slotNum = ejsLookupScope(ejs, &qname, &lookup)) >= 0) {
                f1 = ejsGetProperty(ejs, lookup.obj, lookup.slotNum);
            }
            if (slotNum < 0 || !ejsIsFunction(f1)) {
                ejsThrowReferenceError(ejs, "Reference is not a function");
            } else if (f1->fullScope) {
                if (lookup.obj != ejs->global) {
                    f2 = ejsCloneFunction(ejs, f1, 0);
                } else {
                    f2 = f1;
                }
                f2->block.scope = state.bp;
                f2->thisObj = FRAME->function.thisObj;
                mprAssert(!lookup.obj->isPrototype);
                ejsSetProperty(ejs, lookup.obj, lookup.slotNum, (EjsObj*) f2);
            }
            BREAK;


        /* Exception Handling -------------------------------------------- */

        /*
            Invoke finally blocks before acting on: return, returnValue and break (goto) opcodes.
            Injected by the compiler prior to break, continue and return statements. Also at the end of a try block
            if there is a finally block.
        
                finally
         */
        CASE (EJS_OP_FINALLY):
            if ((ex = findExceptionHandler(ejs, EJS_EX_FINALLY)) != 0) {
                if (FRAME->function.inCatch) {
                    popExceptionBlock(ejs);
                    push(FRAME->pc);
                    createExceptionBlock(ejs, ex, EJS_EX_FINALLY);
                    BLOCK->breakCatch = 1;
                } else {
                    createExceptionBlock(ejs, ex, EJS_EX_FINALLY);
                }
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
                if (BLOCK->breakCatch) {
                    /* Restart the original instruction (return, break, continue) */
                    popExceptionBlock(ejs);
                    SET_PC(FRAME, pop(ejs));
                } else {
                    offset = findEndException(ejs);
                    SET_PC(FRAME, &FRAME->function.body.code.byteCode[offset]);
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
            mprAssert(FRAME->attentionPc);
            FRAME->pc = FRAME->attentionPc;
            mprAssert(FRAME->pc);
            FRAME->attentionPc = 0;
            if (mprHasAllocError(ejs) && !ejs->exception) {
                mprResetAllocError(ejs);
                ejsThrowMemoryError(ejs);
            }
            if (ejs->exiting || mprIsExiting(ejs)) {
                goto done;
            }
            if (ejs->gcRequired) {
                ejsCollectGarbage(ejs, EJS_GEN_NEW);
            }
            if (ejs->exception && !manageExceptions(ejs)) {
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
            mprAssert(ejs->exception || ejs->result);
            BREAK;

        /*
            Pop N items off the stack
                PopItems            <count>
                Stack before (top)  [value]
                                    [...]
                Stack after         []
         */
        CASE (EJS_OP_POP_ITEMS):
            state.stack -= GET_BYTE();
            BREAK;

        /*
            Duplicate one item on the stack
                Stack before (top)  [value]
                Stack after         [value]
                                    [value]
         */
        CASE (EJS_OP_DUP):
            vp = state.stack[0];
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
            v1 = state.stack[-1];
            push(v1);
            v1 = state.stack[0];
            push(v1);
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
            numNonDefault = FRAME->function.numArgs - FRAME->function.numDefault;
            offset = FRAME->argc - numNonDefault;
            if (offset < 0 || offset > tableSize) {
                offset = tableSize;
            }
            FRAME->pc += FRAME->pc[offset];
            BREAK;
        }

        /*
            Unconditional branch to a new location
                Goto                <offset.32>
         */
        CASE (EJS_OP_GOTO):
            offset = GET_WORD();
            SET_PC(FRAME, &FRAME->pc[offset]);
            BREAK;

        /*
            Unconditional branch to a new location (8 bit)
                Goto.8              <offset.8>
         */
        CASE (EJS_OP_GOTO_8):
            offset = (schar) GET_BYTE();
            SET_PC(FRAME, &FRAME->pc[offset]);
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
            if (v1 == 0 || !ejsIsBoolean(v1)) {
                v1 = ejsCast(ejs, v1, ejs->booleanType);
                if (ejs->exception) {
                    BREAK;
                }
            }
            if (!ejsIsBoolean(v1)) {
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
            BREAK;

        /*
            Branch to offset if [value1] == null
                BranchNull
                Stack before (top)  [boolValue]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_NULL):
            push(ejs->nullValue);
            offset = GET_WORD();
            goto commonBranchCode;

        /*
            Branch to offset if [value1] == undefined
                BranchUndefined
                Stack before (top)  [boolValue]
                Stack after         []
         */
        CASE (EJS_OP_BRANCH_UNDEFINED):
            push(ejs->undefinedValue);
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
            push(ejs->zeroValue);
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
            if (!ejsIsBoolean(result)) {
                ejsThrowTypeError(ejs, "Result of a comparision must be boolean");
            } else {
                if (((EjsBoolean*) result)->value) {
                    SET_PC(FRAME, &FRAME->pc[offset]);
                }
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
            push(ejs->nullValue);
            goto binaryExpression;

        /*
            Compare if [item] == undefined
                CompareUndefined
                Stack before (top)  [value]
                Stack after         [boolean]
         */
        CASE (EJS_OP_COMPARE_UNDEFINED):
            push(ejs->undefinedValue);
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
            mprAssert(v1);
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
            v1 = ejsCast(ejs, v1, ejs->booleanType);
            result = ejsInvokeOperator(ejs, v1, opcode, 0);
            push(result);
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
            result = evalBinaryExpr(ejs, v1, EJS_OP_ADD, (EjsObj*) ejsCreateNumber(ejs, count));
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
            if (!ejsIsType(v1)) {
                if (ejsIsFunction(v1)) {
                    fun = (EjsFunction*) v1;
                    if (fun->archetype == 0) {
                        if ((fun->archetype = ejsCreateArchetype(ejs, fun, NULL)) == 0) {
                            BREAK;
                        }
                    }
                    obj = ejsCreate(ejs, fun->archetype, 0);
                } else {
                    ejsThrowReferenceError(ejs, "Can't locate type");
                    BREAK;
                }
            } else {
                obj = ejsCreate(ejs, (EjsType*) v1, 0);
            }
            push(obj);
            ejs->result = obj;
            BREAK;

        /*
            Create a new object literal
                NewObject           <type> <argc> [<attributes> ...]
                Stack before (top)  [<space><name><value>]
                                    [<space><name><value>]
                Stack after         []
         */
        CASE (EJS_OP_NEW_OBJECT):
            FRAME->ignoreAttention = 1;
            type = GET_TYPE();
            argc = GET_INT();
            vp = (EjsObj*) ejsCreateObject(ejs, type, 0);
            for (i = 1 - (argc * 3); i <= 0; ) {
                spaceVar = ejsToString(ejs, state.stack[i++]);
                if (ejs->exception) BREAK;
                nameVar = ejsToString(ejs, state.stack[i++]);
                if (ejs->exception) BREAK;
                v1 = state.stack[i++];
                attributes = GET_INT();
                if (v1 && nameVar && spaceVar) {
                    //  TODO MOB - allow types
                    ejsName(&qname, mprStrdup(vp, spaceVar->value), mprStrdup(vp, nameVar->value));
                    ejsDefineProperty(ejs, vp, -1, &qname, NULL, attributes, v1);
                }
            }
            state.stack -= (argc * 3);
            push(vp);
            FRAME->ignoreAttention = 0;
            BREAK;

        /*
            Reference the super class
                Super
                Stack before (top)  [obj]
                Stack after         [type]
         */
        CASE (EJS_OP_SUPER):
            push(FRAME->function.thisObj->type);
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
            qname.name = ejsToString(ejs, pop(ejs))->value;
            v1 = pop(ejs);
            if (ejsIsNamespace(v1)) {
                qname.space = ((EjsNamespace*) v1)->uri;
            } else {
                qname.space = ejsToString(ejs, v1)->value;
            }
            vp = pop(ejs);
            slotNum = ejsLookupVar(ejs, vp, &qname, &lookup);
#if ECMA || 1
            if (slotNum < 0) {
                push(ejs->trueValue);
            } else {
                if (!lookup.obj->dynamic || ejsHasTrait(lookup.obj, slotNum, EJS_TRAIT_FIXED)) {
                    push(ejs->falseValue);
                } else {
                    ejsDeletePropertyByName(ejs, lookup.obj, &lookup.name);
                    push(ejs->trueValue);
                }
            }
#else
            if (slotNum < 0) {
                ejsThrowReferenceError(ejs, "Property \"%s\" does not exist", qname.name);
            } else {
                if (!lookup.obj->dynamic) {
                    //  MOB -- probably can remove this and rely on fixed below as per ecma spec
                    ejsThrowTypeError(ejs, "Can't delete properties in a non-dynamic object");
                } else if (ejsHasTrait(lookup.obj, slotNum, EJS_TRAIT_FIXED)) {
                    ejsThrowTypeError(ejs, "Property \"%s\" is not deletable", qname.name);
                } else {
                    ejsDeletePropertyByName(ejs, lookup.obj, &lookup.name);
                }
            }
#endif
            BREAK;

        /*
            Delete an object property from the current scope
                DeleteScopedNameExpr
                Stack before (top)  [name]
                                    [space]
                Stack after         [true|false]
         */
        CASE (EJS_OP_DELETE_SCOPED_NAME_EXPR):
            qname.name = ejsToString(ejs, pop(ejs))->value;
            v1 = pop(ejs);
            if (ejsIsNamespace(v1)) {
                qname.space = ((EjsNamespace*) v1)->uri;
            } else {
                qname.space = ejsToString(ejs, v1)->value;
            }
            slotNum = ejsLookupScope(ejs, &qname, &lookup);
#if ECMA || 1
            if (slotNum < 0) {
                push(ejs->trueValue);
            } else {
                if (!lookup.obj->dynamic || ejsHasTrait(lookup.obj, slotNum, EJS_TRAIT_FIXED)) {
                    push(ejs->falseValue);
                } else {
                    ejsDeletePropertyByName(ejs, lookup.obj, &lookup.name);
                    push(ejs->trueValue);
                }
            }
#else
            if (slotNum < 0) {
                ejsThrowReferenceError(ejs, "Property \"%s\" does not exist", qname.name);
            } else {
                if (!lookup.obj->dynamic) {
                    //  MOB -- probably can remove this and rely on fixed below as per ecma spec
                    ejsThrowTypeError(ejs, "Can't delete properties in a non-dynamic object");
                } else if (ejsHasTrait(lookup.obj, slotNum, EJS_TRAIT_FIXED)) {
                    ejsThrowTypeError(ejs, "Property \"%s\" is not deletable", qname.name);
                } else {
                    ejsDeletePropertyByName(ejs, lookup.obj, &lookup.name);
                }
            }
#endif
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
            if (!ejsIsType(type)) {
                ejsThrowTypeError(ejs, "Not a type");
            } else {
                v1 = pop(ejs);
                push(ejsCast(ejs, v1, type));
            }
            BREAK;

        /*
            Cast to a boolean type
                CastBoolean
                Stack before (top)  [value]
                Stack after         [result]
         */
        CASE (EJS_OP_CAST_BOOLEAN):
            v1 = ejsCast(ejs, pop(ejs), ejs->booleanType);
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
                ejsThrowTypeError(ejs, "Can't convert to a name");
            } else {
                ejsName(&qname, "", nameVar->value);
                slotNum = ejsLookupProperty(ejs, v1, &qname);
                if (slotNum < 0) {
                    //  MOB -- Reconsider
                    slotNum = ejsLookupVar(ejs, v1, &qname, &lookup);
                    if (slotNum < 0 && ejsIsType(v1)) {
                        slotNum = ejsLookupVar(ejs, (EjsObj*) ((EjsType*) v1)->prototype, &qname, &lookup);
                    }
                }
                push(ejsCreateBoolean(ejs, slotNum >= 0));
            }
            BREAK;

        /*
            Unimplemented op codes
         */
        CASE (EJS_OP_BREAKPOINT):
            mprAssert(0);
            BREAK;

#if !BLD_UNIX_LIKE && !VXWORKS
        }
    }
#endif
    
done:
#if BLD_DEBUG && FUTURE
    if (ejs->initialized) {
        ejsShowOpFrequency(ejs);
    }
#endif
    mprAssert(FRAME == 0 || FRAME->attentionPc == 0);
    ejs->state = ejs->state->prev;;
    if (ejs->exception) {
        ejsAttention(ejs);
    }
}


/*
    WARNING: this may not complete the store. It may setup a setter function which then requires the VM to execute.
 */
static void storePropertyToSlot(Ejs *ejs, EjsObj *thisObj, EjsObj *obj, int slotNum, EjsObj *value)
{
    EjsFunction     *fun;
    EjsObj          *vp;
    EjsTrait        *trait;

    mprAssert(value);

    if (slotNum < 0 && !obj->dynamic) {
        ejsThrowTypeError(ejs, "Object is not extendable");
        return;
    }
    trait = ejsGetTrait(obj, slotNum);
    if (trait) {
        if (trait->attributes & EJS_TRAIT_SETTER) {
            pushOutside(ejs, value);
            fun = ejsGetProperty(ejs, obj, slotNum);
            fun = fun->setter;
            callFunction(ejs, fun, thisObj, 1, 0);
            return;
        }
        if (trait->type) {
            if (!ejsIsA(ejs, value, trait->type)) {
                if (value == ejs->nullValue || value == ejs->undefinedValue) {
                    if (trait->attributes & EJS_TRAIT_THROW_NULLS) {
                        ejsThrowTypeError(ejs, "Unacceptable null or undefined value");
                        return;
                    } else if (trait->attributes & EJS_TRAIT_CAST_NULLS) {
                        value = ejsCast(ejs, value, trait->type);
                        if (ejs->exception) {
                            return;
                        }
                    }
                } else {
                    value = ejsCast(ejs, value, trait->type);
                    if (ejs->exception) {
                        return;
                    }
                }
            }
        }
        if (trait->attributes & EJS_TRAIT_READONLY) {
            EjsName         qname;
            vp = ejsGetProperty(ejs, obj, slotNum);
            if (vp != ejs->nullValue && vp != ejs->undefinedValue) {
                qname = ejsGetPropertyName(ejs, obj, slotNum);
                ejsThrowReferenceError(ejs, "Property \"%s\" is not writable", qname.name);
                return;
            }
        }
    }
    ejsSetProperty(ejs, (EjsObj*) obj, slotNum, (EjsObj*) value);
    ejs->result = value;
}


/*
    Store a property by name in the given object. Will create if the property does not already exist.
 */
static void storeProperty(Ejs *ejs, EjsObj *thisObj, EjsObj *obj, EjsName *qname, EjsObj *value, bool dupName)
{
    EjsLookup       lookup;
    EjsTrait        *trait;
    int             slotNum;

    mprAssert(qname);
    mprAssert(qname->name);
    mprAssert(obj);

    //  MOB -- ONLY XML requires this.  NOTE: this bypasses ES5 traits
    //  Alternatively push this whole function down into ejsObject and have all go via setPropertyByName
    
    if (obj->type->helpers.setPropertyByName) {
        slotNum = (*obj->type->helpers.setPropertyByName)(ejs, obj, qname, value);
        if (slotNum >= 0) {
            return;
        }
    }
    if ((slotNum = ejsLookupVar(ejs, obj, qname, &lookup)) >= 0) {
        if (lookup.obj != obj) {
            trait = ejsGetTrait(lookup.obj, slotNum);
            if (trait->attributes & EJS_TRAIT_SETTER) {
                obj = lookup.obj;
                
                //  MOB - REFACTOR. Just for prototype() getter in Object.prototype
            } else if (lookup.obj->isPrototype || trait->attributes & EJS_TRAIT_GETTER) {
                if (obj->type->hasInstanceVars) {
                    /* The prototype properties have been inherited */
                    slotNum = ejsGetSlot(ejs, obj, slotNum);
                    obj->slots[slotNum].trait = lookup.obj->slots[slotNum].trait;
                    obj->slots[slotNum].value = lookup.obj->slots[slotNum].value;
                    slotNum = ejsSetPropertyName(ejs, obj, slotNum, qname);
                } else  {
                    slotNum = -1;
                }
            } else {
#if UNUSED
                //  MOB -- this is allowing a obj.prop = x, where prop is in a static base type 
                obj = lookup.obj;
#else
                /*
                    This is the fundamental asymetry between load/store. We allow loading properties from static base 
                    types, but do not allow stores. This is essential to stop bleeding of Object static properties into
                    all objects. E.g. Object.create.
                 */
                slotNum = -1;
#endif
            }
        }
    }
    if (slotNum < 0) {
        if (dupName) {
            qname->name = mprStrdup(obj, qname->name);
            qname->space = mprStrdup(obj, qname->space);
        }
        slotNum = ejsSetPropertyName(ejs, obj, slotNum, qname);
    }
    if (!ejs->exception) {
        storePropertyToSlot(ejs, thisObj, obj, slotNum, value);
    }
}


/*
    Store a property by name in the scope chain. Will create properties if the given name does not already exist.
 */
static void storePropertyToScope(Ejs *ejs, EjsName *qname, EjsObj *value, bool dup)
{
    EjsFrame        *fp;
    EjsObj          *obj, *thisObj;
    EjsLookup       lookup;
    EjsTrait        *trait;
    int             slotNum;

    mprAssert(qname);

    fp = ejs->state->fp;

    if ((slotNum = ejsLookupScope(ejs, qname, &lookup)) >= 0) {
        if (lookup.obj->isPrototype) {
            thisObj = obj = (EjsObj*) fp->function.thisObj;
            trait = ejsGetTrait(lookup.obj, slotNum);
            if (trait->attributes & EJS_TRAIT_SETTER) {
                obj = lookup.obj;

            //  MOB -- surely this should be done universally if found on a prototype?
            } else if (obj->type->hasInstanceVars) {
                /* The prototype properties have been inherited */
                slotNum = ejsGetSlot(ejs, obj, slotNum);
                obj->slots[slotNum].trait = lookup.obj->slots[slotNum].trait;
                obj->slots[slotNum].value = lookup.obj->slots[slotNum].value;
                slotNum = ejsSetPropertyName(ejs, obj, slotNum, qname);
            } else {
                slotNum = -1;
            }
        } else {
            thisObj = obj = lookup.obj;
        }

    } else {
        thisObj = obj = fp->function.moduleInitializer ? ejs->global : (EjsObj*) fp;
        if (dup) {
            qname->name = mprStrdup(obj, qname->name);
            qname->space = mprStrdup(obj, qname->space);
        }
        slotNum = ejsSetPropertyName(ejs, obj, slotNum, qname);
    }
    storePropertyToSlot(ejs, thisObj, obj, slotNum, value);
}


/*
    Run the module initializer
 */
EjsObj *ejsRunInitializer(Ejs *ejs, EjsModule *mp)
{
    EjsModule   *dp;
    EjsObj      *result;
    int         next;

    if (mp->initialized || !mp->hasInitializer) {
        mp->initialized = 1;
        return ejs->nullValue;
    }
    mp->initialized = 1;

    if (mp->dependencies) {
        for (next = 0; (dp = (EjsModule*) mprGetNextItem(mp->dependencies, &next)) != 0;) {
            if (dp->hasInitializer && !dp->initialized) {
                if (ejsRunInitializer(ejs, dp) == 0) {
                    return 0;
                }
            }
        }
    }
    mprLog(ejs, 6, "Running initializer for module %s", mp->name);
    result = ejsRunFunction(ejs, mp->initializer, ejs->global, 0, NULL);

    //  TODO - not calling MakePermanent so is this really needed?
    ejsMakeTransient(ejs, (EjsObj*) mp->initializer);
    return result;
}


/*
    Run all initializers for all modules
 */
int ejsRun(Ejs *ejs)
{
    EjsModule   *mp;
    int         next;

    /*
        This is used by ejs to interpret scripts. OPT. Should not run through old modules every time
     */
    for (next = 0; (mp = (EjsModule*) mprGetNextItem(ejs->modules, &next)) != 0;) {
        if (mp->initialized) {
            continue;
        }
        if (ejsRunInitializer(ejs, mp) == 0) {
            return EJS_ERR;
        }
    }
    return 0;
}


EjsObj *ejsRunFunction(Ejs *ejs, EjsFunction *fun, EjsObj *thisObj, int argc, EjsObj **argv)
{
    int         i;
    
    mprAssert(ejs);
    mprAssert(fun);
    mprAssert(ejsIsFunction(fun));
    mprAssert(ejs->exception == 0);

    if (ejs->exception) {
        return 0;
    }
    ejsClearAttention(ejs);
    
    if (thisObj == 0) {
        if ((thisObj = fun->thisObj) == 0) {
            thisObj = ejs->state->fp->function.thisObj;
        }
    }    
    if (ejsIsNativeFunction(fun)) {
        if (fun->body.proc == 0) {
            ejsThrowArgError(ejs, "Native function is not defined");
            return 0;
        }
        ejs->result = (fun->body.proc)(ejs, thisObj, argc, argv);
        if (ejs->result == 0) {
            ejs->result = ejs->nullValue;
        }

    } else {
        for (i = 0; i < argc; i++) {
            pushOutside(ejs, argv[i]);
        }
        VM(ejs, fun, thisObj, argc, 0);
        ejs->state->stack -= argc;
        if (ejs->exiting || mprIsExiting(ejs)) {
            ejsAttention(ejs);
        }
    }
    return (ejs->exception) ? 0 : ejs->result;
}


//  MOB - can only be used to run instance methods -- rename to clarify

EjsObj *ejsRunFunctionBySlot(Ejs *ejs, EjsObj *thisObj, int slotNum, int argc, EjsObj **argv)
{
    EjsFunction     *fun;

    if (thisObj == 0) {
        thisObj = ejs->global;
    }
    if (thisObj == ejs->global) {
        fun = ejsGetProperty(ejs, thisObj, slotNum);
    } else if (ejsIsType(thisObj)) {
        fun = ejsGetProperty(ejs, thisObj, slotNum);
    } else {
        fun = ejsGetProperty(ejs, thisObj->type->prototype, slotNum);
    }
    if (fun == 0) {
        ejsThrowReferenceError(ejs, "Can't find function at slot %d in %s::%s", slotNum, thisObj->type->qname.space, 
            thisObj->type->qname.name);
        return 0;
    }
    return ejsRunFunction(ejs, fun, thisObj, argc, argv);
}


EjsObj *ejsRunFunctionByName(Ejs *ejs, EjsObj *container, EjsName *qname, EjsObj *thisObj, int argc, EjsObj **argv)
{
    EjsFunction     *fun;

    if (thisObj == 0) {
        thisObj = ejs->global;
    }
    if ((fun = ejsGetPropertyByName(ejs, container, qname)) == 0) {
        ejsThrowReferenceError(ejs, "Can't find function %s::%s", qname->space, qname->name);
        return 0;
    }
    return ejsRunFunction(ejs, fun, thisObj, argc, argv);
}



static void badArgType(Ejs *ejs, EjsObj *activation, EjsTrait *trait, int index)
{
    EjsName     qname;

    qname = ejsGetPropertyName(ejs, activation, index);
    ejsThrowTypeError(ejs, "Unacceptable null or undefined value for arg \"%s\" (pos: %d)", qname.name, index);
}


/*
    Validate the args. This routine handles ...rest args and parameter type checking and casts. Returns the new argc 
    or < 0 on errors.
 */
static int validateArgs(Ejs *ejs, EjsFunction *fun, int argc, EjsObj **argv)
{
    EjsType     *type;
    EjsTrait    *trait;
    EjsObj      *activation, *newArg;
    EjsArray    *rest;
    int         nonDefault, i, limit, numRest;

    mprAssert(ejs->exception == 0);
    mprAssert(ejs->state->fp == 0 || ejs->state->fp->attentionPc == 0);

    nonDefault = fun->numArgs - fun->numDefault;
    activation = fun->activation;

    if (argc < nonDefault) {
        if (!fun->rest || argc != (fun->numArgs - 1)) {
            if (fun->strict) {
                ejsThrowArgError(ejs, "Insufficient actual parameters. Call requires %d parameter(s).", nonDefault);
                return EJS_ERR;
            } else {
                /* Create undefined values for missing args */
                for (i = argc; i < nonDefault; i++) {
                    pushOutside(ejs, ejs->undefinedValue);
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
        if (!ejsIsNativeFunction(fun)) {
            ejs->state->stack -=  (argc - fun->numArgs);
            argc = fun->numArgs;
        }
    }

    /*
        Handle rest "..." args
     */
    if (fun->rest) {
        numRest = argc - fun->numArgs + 1;
        rest = ejsCreateArray(ejs, numRest);
        if (rest == 0) {
            return EJS_ERR;
        }
        for (i = numRest - 1; i >= 0; i--) {
            ejsSetProperty(ejs, (EjsObj*) rest, i, popOutside(ejs));
        }
        argc = argc - numRest + 1;
        pushOutside(ejs, rest);
    }

    /*
        Cast args to the right types
     */
    limit = min((uint) argc, fun->numArgs);
    for (i = 0; i < limit; i++) {
        if ((trait = ejsGetTrait(activation, i)) == 0 || trait->type == 0) {
            /* No trait - all to pass */
            continue;
        }
        type = trait->type;
        if (!ejsIsA(ejs, argv[i], type)) {
            if ((argv[i] == ejs->nullValue || argv[i] == ejs->undefinedValue)) {
                if (trait->attributes & EJS_TRAIT_THROW_NULLS) {
                    badArgType(ejs, activation, trait, i);
                    return EJS_ERR;
                }
                if (!(trait->attributes & EJS_TRAIT_CAST_NULLS)) {
                    continue;
                }
            }
            newArg = ejsCast(ejs, argv[i], trait->type);
            if (ejs->exception) {
                ejsClearException(ejs);
                badArgType(ejs, activation, trait, i);
                return EJS_ERR;
            }
            argv[i] = newArg;
        }
    }
    return argc;
}


#if UNUSED
/*
    Call a type constructor function and create a new object.
 */
static void callConstructor(Ejs *ejs, EjsType *type, int argc, int stackAdjust)
{
    EjsObj      *obj;

    mprAssert(ejsIsType(type));
    mprAssert(ejs->exception == 0);
    mprAssert(ejs->state->fp->attentionPc == 0);

    obj = ejsCreate(ejs, type, 0);
    ejsClearAttention(ejs);
    
    mprAssert(type->constructor.block.obj.isFunction);
    if (ejsIsNativeFunction(type)) {
        mprAssert(ejs->state->fp == 0 || ejs->state->fp->attentionPc == 0);    
        callFunction(ejs, (EjsFunction*) type, obj, argc, stackAdjust);
    } else {
        VM(ejs, (EjsFunction*) type, obj, argc, stackAdjust);
        ejs->state->stack -= (argc + stackAdjust);
        if (ejs->exiting || mprIsExiting(ejs)) {
            ejsAttention(ejs);
        }
    }
    ejs->result = obj;
}
#endif


#if UNUSED
/*
    Find the right base class to use as "this" for a static method
 */
static EjsObj *getStaticThis(Ejs *ejs, EjsType *type, int slotNum)
{
    while (type) {
        if (slotNum >= type->numInherited) {
            break;
        }
        type = type->baseType;
    }
    return (EjsObj*) type;
}
#endif


static void callInterfaceInitializers(Ejs *ejs, EjsType *type)
{
    EjsType         *iface;
    EjsFunction     *fun;
    EjsName         qname;
    int             next;

    for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
        if (iface->hasInitializer) {
            qname = ejsGetPropertyName(ejs, (EjsObj*) iface, 0);
            //  TODO OPT. Could run all 
            fun = ejsGetPropertyByName(ejs, (EjsObj*) type, &qname);
            if (fun && ejsIsFunction(fun)) {
                callFunction(ejs, fun, (EjsObj*) type, 0, 0);
            }
        }
    }
}


/*
    Push a block. Used by compiler.
    MOB -- move back to the compiler
 */
EjsBlock *ejsPushBlock(Ejs *ejs, EjsBlock *original)
{
    EjsBlock    *block;

    mprAssert(!ejsIsFunction(original));

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
    Pop an exception block.
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
static bool manageExceptions(Ejs *ejs)
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


static inline EjsEx *findExceptionHandler(Ejs *ejs, int kind)
{
    EjsEx       *ex;
    EjsFrame    *fp;
    EjsCode     *code;
    uint        pc;
    int         i;

    ex = 0;
    fp = ejs->state->fp;
    code = &fp->function.body.code;
    pc = (uint) (fp->pc - code->byteCode - 1);

    /*
        Exception handlers are sorted with the inner most handlers first.
     */
    for (i = 0; i < code->numHandlers; i++) {
        ex = code->handlers[i];
        if (ex->tryStart <= pc && pc < ex->handlerEnd && (ex->flags & kind)) {
            if (ejs->exception == (EjsObj*) ejs->iterator || kind == EJS_EX_FINALLY || ex->catchType == ejs->voidType || 
                    /* MOB - This test is here because stopIteration throws a type and ejsIsA works only for instances */
                    ejs->exception == (EjsObj*) ex->catchType || ejsIsA(ejs, (EjsObj*) ejs->exception, ex->catchType)) {
                return ex;
            }
        }
    }
    return 0;
}

static inline EjsEx *inHandler(Ejs *ejs, int kind)
{
    EjsEx       *ex;
    EjsFrame    *fp;
    EjsCode     *code;
    uint        pc;
    int         i;
    
    ex = 0;
    fp = ejs->state->fp;
    code = &fp->function.body.code;
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
static inline uint findEndException(Ejs *ejs)
{
    EjsFrame    *fp;
    EjsEx       *best, *ex;
    EjsCode     *code;
    uint        offset, pc;
    int         i;

    ex = 0;
    fp = ejs->state->fp;
    code = &fp->function.body.code;
    pc = (uint) (fp->pc - code->byteCode - 1);
    offset = 0;

    for (best = 0, i = 0; i < code->numHandlers; i++) {
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
    mprAssert(offset);
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
    uint            pc;

    ex = 0;
    fp = ejs->state->fp;
    code = &fp->function.body.code;

    if (code->numHandlers == 0) {
        return;
    }

    /*
        The PC is always one advanced from the throwing instruction. ie. the PC has advanced past the offending 
        instruction so reverse by one.
     */
    pc = (uint) (fp->pc - code->byteCode - 1);
    mprAssert(pc >= 0);

rescan:
    if (!fp->function.inException || (ejs->exception == (EjsObj*) ejs->stopIterationType)) {
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
            SET_PC(fp, &fp->function.body.code.byteCode[ex->handlerEnd + 1]);
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
    EjsCode         *code;
    int             i, count;

    state = ejs->state;
    fp = state->fp;
    code = &state->fp->function.body.code;
    mprAssert(ex);

    ejsClearAttention(ejs);

    if (flags & EJS_EX_ITERATION) {
        /*
            Empty handler is a special case for iteration. We simply do a break to the handler location
            which targets the end of the for/in loop.
         */
        SET_PC(fp, &fp->function.body.code.byteCode[ex->handlerStart]);
        ejs->exception = 0;
        return;
    }

    /*
        Discard all lexical blocks defined inside the try block
     */
    if (!fp->function.inCatch) {
        for (count = 0, block = state->bp; block != (EjsBlock*) state->fp; block = block->prev) {
            count++;
        }
        count -= ex->numBlocks;
        mprAssert(count >= 0);
        for (i = 0; i < count && count > 0; i++) {
            ejsPopBlock(ejs);
        }
        count = (state->stack - fp->stackReturn);
        state->stack -= (count - ex->numStack);
        mprAssert(state->stack >= fp->stackReturn);
    }
    
    /*
        Allocate a new frame in which to execute the handler
     */
    block = ejsCreateBlock(ejs, 0);
    ejsSetDebugName(block, "exception");
    if (block == 0) {
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
    SET_PC(fp, &fp->function.body.code.byteCode[ex->handlerStart]);

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


static int lookupOverloadedOperator(Ejs *ejs, EjsOpCode opcode, EjsObj *lhs)
{
    EjsName     qname;
    int         i;

    for (i = 0; operMap[i].opcode; i++) {
        if (operMap[i].opcode == opcode) {
            ejsName(&qname, "", operMap[i].name);
            break;
        }
    }
    return ejsLookupProperty(ejs, (EjsObj*) lhs->type, &qname);
}


/*
    Evaluate a binary expression.
    OPT -- simplify and move back inline into eval loop.
 */
static EjsObj *evalBinaryExpr(Ejs *ejs, EjsObj *lhs, EjsOpCode opcode, EjsObj *rhs)
{
    EjsObj      *result;
    int         slotNum;

    if (lhs == 0) {
        lhs = ejs->undefinedValue;
    }
    if (rhs == 0) {
        rhs = ejs->undefinedValue;
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


int ejsInitStack(Ejs *ejs)
{
    EjsState    *state;

    state = ejs->state = ejs->masterState = mprAllocObjZeroed(ejs, EjsState);

    /*
        Allocate the stack
     */
    state->stackSize = MPR_PAGE_ALIGN(EJS_STACK_MAX, mprGetPageSize(ejs));

    /*
        This will allocate memory virtually for systems with virutal memory. Otherwise, it will just use malloc.
        TODO - create a guard page
     */
    state->stackBase = mprMapAlloc(ejs, state->stackSize, MPR_MAP_READ | MPR_MAP_WRITE);
    if (state->stackBase == 0) {
        mprSetAllocError(ejs);
        return EJS_ERR;
    }
    state->stack = &state->stackBase[-1];
    return 0;
}


#if FUTURE
/*
    Grow the operand evaluation stack.
    Return a negative error code on memory allocation errors or if the stack grows too big.
 */
int ejsGrowStack(Ejs *ejs, int incr)
{
    EjsStack *sp;
    EjsFrame *frame;
    EjsObj **bottom;
    int i, size, moveBy;

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
        if (sp->top >= &sp->bottom[EJS_STACK_MAX]) {
            return MPR_ERR_NO_MEMORY;
        }
        size = (sizeof(EjsObj*) * incr);
        sp->bottom = (EjsObj**) mprAlloc(sp, size);
        /*
            Push always begins with an increment of sp->top. Initially, sp_bottom points to the first (future) element.
         */
        sp->top = &sp->bottom[-1];
    }

    if (sp->bottom == 0) {
        return MPR_ERR_NO_MEMORY;
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
    //  TODO - should pass status back
    ejs->exiting = 1;
}


static EjsName getNameArg(EjsFrame *fp)
{
    EjsName     qname;

    qname.name = getStringArg(fp);
    qname.space = getStringArg(fp);
    return qname;
}


/*
    Get an interned string. String constants are stored as token offsets into the constant pool. The pool
    contains null terminated UTF-8 strings.
 */
static char *getStringArg(EjsFrame *fp)
{
    int     number;

    number = (int) ejsDecodeNum(&fp->pc);

    mprAssert(fp->function.body.code.constants);
    return &fp->function.body.code.constants->pool[number];
}


static EjsObj *getGlobalArg(Ejs *ejs, EjsFrame *fp)
{
    EjsObj      *vp;
    EjsName     qname;
    int         t, slotNum;

    t = (int) ejsDecodeNum(&fp->pc);
    if (t < 0) {
        return 0;
    }

    slotNum = -1;
    qname.name = 0;
    qname.space = 0;
    vp = 0;

    /*
        OPT. Could this encoding be optimized?
     */
    switch (t & EJS_ENCODE_GLOBAL_MASK) {
    default:
        mprAssert(0);
        return 0;

    case EJS_ENCODE_GLOBAL_NOREF:
        return 0;

    case EJS_ENCODE_GLOBAL_SLOT:
        slotNum = t >> 2;
        if (0 <= slotNum && slotNum < ejsGetPropertyCount(ejs, ejs->global)) {
            vp = ejsGetProperty(ejs, ejs->global, slotNum);
        }
        break;

    case EJS_ENCODE_GLOBAL_NAME:
        qname.name = &fp->function.body.code.constants->pool[t >> 2];
        if (qname.name == 0) {
            mprAssert(0);
            return 0;
        }
        qname.space = getStringArg(fp);
        if (qname.space == 0) {
            return 0;
        }
        if (qname.name) {
            vp = ejsGetPropertyByName(ejs, ejs->global, &qname);
        }
        break;
    }
    return vp;
}


static void callProperty(Ejs *ejs, EjsObj *obj, int slotNum, EjsObj *thisObj, int argc, int stackAdjust)
{
    EjsTrait    *trait;
    EjsFunction *fun;

    //  MOB -- rethink this.
    fun = ejsGetProperty(ejs, obj, slotNum);
    trait = ejsGetTrait(obj, slotNum);
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
static void callFunction(Ejs *ejs, EjsFunction *fun, EjsObj *thisObj, int argc, int stackAdjust)
{
    EjsState        *state;
    EjsFrame        *fp;
    EjsType         *type;
    EjsObj          **argv;

    mprAssert(fun);
    mprAssert(ejs->exception == 0);
    mprAssert(ejs->state->fp == 0 || ejs->state->fp->attentionPc == 0);    

    state = ejs->state;

    if (unlikely(ejsIsType(fun))) {
        type = (EjsType*) fun;
#if OLD
        //MOB callConstructor(ejs, (EjsType*) fun, argc, stackAdjust);
        obj = ejsCreate(ejs, type, 0);
        ejsClearAttention(ejs);
        if (!ejsIsNativeFunction(type)) {
            VM(ejs, (EjsFunction*) type, obj, argc, stackAdjust);
            ejs->state->stack -= (argc + stackAdjust);
            if (ejs->exiting || mprIsExiting(ejs)) {
                ejsAttention(ejs);
            }
        }
        ejs->result = obj;
        return;
#else
        if (thisObj == NULL) {
            thisObj = ejsCreate(ejs, type, 0);
        }
        ejs->result = thisObj;
        if (!type->hasConstructor) {
            ejs->state->stack -= (argc + stackAdjust);
            if (ejs->exiting || mprIsExiting(ejs)) {
                ejsAttention(ejs);
            }
            return;
        }
#endif
        
    } else if (!ejsIsFunction(fun)) {
        if ((EjsObj*) fun == (EjsObj*) ejs->undefinedValue) {
            ejsThrowReferenceError(ejs, "Function is undefined");
            return;
        } else {
            ejsThrowReferenceError(ejs, "Reference is not a function");
            return;
        }
    }
    if (thisObj == 0) {
        if ((thisObj = fun->thisObj) == 0) {
            thisObj = state->fp->function.thisObj;
        } 
    } 
#if UNUSED
    if (fun->staticMethod && !ejsIsType(thisObj)) {
        /*
            Calling a static method via an instance object
         */
        thisObj = getStaticThis(ejs, thisObj->type, fun->slotNum);
    }
#endif
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
    if (ejsIsNativeFunction(fun)) {
        if (fun->body.proc == 0) {
            ejsThrowInternalError(ejs, "Native function \"%s\" is not defined", fun->name);
            return;
        }
        ejsClearAttention(ejs);
        ejs->result = (fun->body.proc)(ejs, thisObj, argc, argv);
        if (ejs->result == 0) {
            ejs->result = ejs->nullValue;
        }
        state->stack -= (argc + stackAdjust);

    } else {
        fp = ejsCreateFrame(ejs, fun, thisObj, argc, argv);
        fp->function.block.prev = state->bp;
        fp->caller = state->fp;
        fp->stackBase = state->stack;
        fp->stackReturn = state->stack - argc - stackAdjust;
        state->fp = fp;
        state->bp = (EjsBlock*) fp;
        ejsClearAttention(ejs);
    }
}


static void throwNull(Ejs *ejs)
{
    ejsThrowReferenceError(ejs, "Object reference is null");
}


/*
    Object can be an instance or a type. If an instance, then step to the immediate base type to begin the count.
 */
static EjsObj *getNthBase(Ejs *ejs, EjsObj *obj, int nthBase)
{
    EjsType     *type;

    if (obj) {
        if (ejsIsType(obj) || obj == ejs->global) {
            type = (EjsType*) obj;
        } else {
            type = obj->type;
            nthBase--;
        }
        for (; type && nthBase > 0; type = type->baseType) {
            nthBase--;
        }
        if (nthBase > 0) {
            ejsThrowReferenceError(ejs, "Can't find correct base class");
            return 0;
        }
        obj = (EjsObj*) type;
    }
    return obj;
}


static EjsObj *getNthBaseFromBottom(Ejs *ejs, EjsObj *obj, int nthBase)
{
    EjsType     *type, *tp;
    int         count;

    if (obj) {
        if (ejsIsType(obj) || obj == ejs->global) {
            type = (EjsType*) obj;
        } else {
            type = obj->type;
        }
        for (count = 0, tp = type->baseType; tp; tp = tp->baseType) {
            count++;
        }
        nthBase = count - nthBase;
        for (; type && nthBase > 0; type = type->baseType) {
            nthBase--;
        }
        obj = (EjsObj*) type;
    }
    return obj;
}


static EjsObj *getNthBlock(Ejs *ejs, int nth)
{
    EjsBlock    *block;

    mprAssert(ejs);
    mprAssert(nth >= 0);

    for (block = ejs->state->bp; block && --nth >= 0; ) {
        /* TODO - this is done for loading scripts into ejs. Really the compiler should remove these blocks */
#if UNUSED
        mprAssert(block->obj.hidden == 0);
        if (block->obj.hidden) nth++;
#endif
        block = block->scope;
    }
    return (EjsObj*) block;
}


/*
    Enter a mesage into the log file
 */
void ejsLog(Ejs *ejs, const char *fmt, ...)
{
    va_list     args;
    char        buf[MPR_MAX_LOG_STRING];

    va_start(args, fmt);
    mprVsprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);

    mprLog(ejs, 0, "%s", buf);
}


void ejsShowStack(Ejs *ejs, EjsFunction *fp)
{
    char    *stack;
    
    stack = ejsFormatStack(ejs, NULL);
    mprLog(ejs, 7, "Stack\n%s", stack);
    mprFree(stack);
}


#if FUTURE

- Separate file

//  MOB - move into the mpr
#if BLD_CC_EDITLINE
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


static void manageBreakpoint(Ejs *ejs)
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

    //  TODO - should have a switch to turn this on / off
    //  OPT - compiler should strip '\n' from currentLine and we should explicitly add it here
    optable = ejsGetOptable(ejs);
    mprLog(ejs, 7, "%0s %04d: [%d] %02x: %-35s # %s:%d %s",
        mprGetCurrentThreadName(fp), offset, (int) (state->stack - fp->stackReturn),
        (uchar) opcode, optable[opcode].name, fp->filename, fp->lineNumber, fp->currentLine);
    if (stop && once++ == 0) {
        mprSleep(ejs, 0);
    }
    mprAssert(state->stack >= fp->stackReturn);
}
#endif


#if BLD_DEBUG
#if UNUSED
void ejsShowOpFrequency(Ejs *ejs)
{
    EjsOptable      *optable;
    int             i;

    if (mprGetLogLevel(ejs) < 6) {
        return;
    }
    optable = ejsGetOptable(ejs);
    mprLog(ejs, 0, "Opcode Frequency");
    for (i = 0; i < 256 && optable[i].name; i++) {
        mprLog(ejs, 6, "%4d %24s %8d", (uchar) i, optable[i].name, opcount[i]);
    }
}
#endif


int ejsOpCount = 0;
static EjsOpCode traceCode(Ejs *ejs, EjsOpCode opcode)
{
    EjsFrame        *fp;
    EjsState        *state;
    EjsOptable      *optable;
    int             len;
    int             offset;
    static int      once = 0;
    static int      stop = 1;
#if UNUSED
    static int      showFrequency = 1;
#endif

    state = ejs->state;
    fp = state->fp;
    opcount[opcode]++;

    if (ejs->initialized && opcode != EJS_OP_DEBUG) {
        //  OPT - should strip '\n' in the compiler
        if (fp->currentLine) {
            len = (int) strlen(fp->currentLine) - 1;
            if (fp->currentLine[len] == '\n') {
                ((char*) fp->currentLine)[len] = '\0';
            }
        }
        offset = (int) (fp->pc - fp->function.body.code.byteCode) - 1;
        if (offset < 0) {
            offset = 0;
        }
        //  TODO - should have a switch to turn this on / off
        //  OPT - compiler should strip '\n' from currentLine and we should explicitly add it here
        optable = ejsGetOptable(ejs);
        mprLog(ejs, 7, "%0s %04d: [%d] %02x: %-35s # %s:%d %s",
            mprGetCurrentThreadName(fp), offset, (int) (state->stack - fp->stackReturn),
            (uchar) opcode, optable[opcode].name, fp->filename, fp->lineNumber, fp->currentLine);
#if UNUSED
        if (showFrequency && ((once % 1000) == 999)) {
            ejsShowOpFrequency(ejs);
        }
#endif
        if (stop && once++ == 0) {
            mprSleep(ejs, 0);
        }
        mprAssert(state->stack >= fp->stackReturn);
    }
    ejsOpCount++;
    return opcode;
}
#endif


/*
    Cleanup defines for all-in-one builds
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
#undef TRACE
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
