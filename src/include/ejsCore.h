/*
    ejsCore.h - Header for the core types.

    The VM provides core types like numbers, strings and objects. This header provides their API.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#ifndef _h_EJS_CORE
#define _h_EJS_CORE 1

#include    "mpr.h"
#include    "http.h"
#include    "ejsTune.h"
#include    "ejsByteCode.h"
#include    "ejsByteCodeTable.h"
#include    "ejs.slots.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************* Defines ************************************/

#if !DOXYGEN
/*
    Forward declare types
 */
struct Ejs;
struct EjsBlock;
struct EjsFrame;
struct EjsFunction;
struct EjsGC;
struct EjsHelpers;
struct EjsLoc;
struct EjsMem;
struct EjsNames;
struct EjsModule;
struct EjsNamespace;
struct EjsObj;
struct EjsPot;
struct EjsService;
struct EjsState;
struct EjsTrait;
struct EjsTraits;
struct EjsType;
struct EjsUri;
struct EjsWorker;
struct EjsXML;
#endif

/*
    Trait, type, function and property attributes. These are sometimes combined into a single attributes word.
 */
#define EJS_TRAIT_BUILTIN               0x1         /**< Property can take a null value */
#define EJS_TRAIT_CAST_NULLS            0x2         /**< Property casts nulls */
#define EJS_TRAIT_DELETED               0x4         /**< Property has been deleted */
#define EJS_TRAIT_GETTER                0x8         /**< Property is a getter */
#define EJS_TRAIT_FIXED                 0x10        /**< Property is not configurable */
#define EJS_TRAIT_HIDDEN                0x20        /**< !Enumerable */
#define EJS_TRAIT_INITIALIZED           0x40        /**< Readonly property has been initialized */
#define EJS_TRAIT_READONLY              0x80        /**< !Writable */
#define EJS_TRAIT_SETTER                0x100       /**< Property is a settter */
#define EJS_TRAIT_THROW_NULLS           0x200       /**< Property rejects null */

#define EJS_PROP_HAS_VALUE              0x400       /**< Property has a value record */
#define EJS_PROP_NATIVE                 0x800       /**< Property is backed by native code */
#define EJS_PROP_STATIC                 0x1000      /**< Class static property */
#define EJS_PROP_ENUMERABLE             0x2000      /**< Property will be enumerable (compiler use only) */

#define EJS_FUN_CONSTRUCTOR             0x4000      /**< Method is a constructor */
#define EJS_FUN_FULL_SCOPE              0x8000      /**< Function needs closure when defined */
#define EJS_FUN_HAS_RETURN              0x10000     /**< Function has a return statement */
#define EJS_FUN_INITIALIZER             0x20000     /**< Type initializer code */
#define EJS_FUN_OVERRIDE                0x40000     /**< Override base type */
#define EJS_FUN_MODULE_INITIALIZER      0x80000     /**< Module initializer */
#define EJS_FUN_REST_ARGS               0x100000    /**< Parameter is a "..." rest */
#define EJS_TRAIT_MASK                  0x1FFFFF    /**< Mask of trait attributes */

/*
    These attributes are never stored in EjsTrait but are often passed in "attributes"
 */
#define EJS_TYPE_CALLS_SUPER            0x200000    /**< Constructor calls super() */
#define EJS_TYPE_HAS_INSTANCE_VARS      0x400000    /**< Type has non-method instance vars (state) */
#define EJS_TYPE_DYNAMIC_INSTANCE       0x800000    /**< Instances are not sealed */
#define EJS_TYPE_FINAL                  0x1000000   /**< Type can't be subclassed */
#define EJS_TYPE_FIXUP                  0x2000000   /**< Type needs to inherit base types properties */
#define EJS_TYPE_HAS_CONSTRUCTOR        0x4000000   /**< Type has a constructor */
#define EJS_TYPE_HAS_TYPE_INITIALIZER   0x80000000  /**< Type has an initializer */
#define EJS_TYPE_IMMUTABLE              0x10000000  /**< Instances are immutable */
#define EJS_TYPE_INTERFACE              0x20000000  /**< Class is an interface */

/*
    Interpreter flags
 */
#define EJS_FLAG_EVENT          0x1         /**< Event pending */
#define EJS_FLAG_NO_INIT        0x8         /**< Don't initialize any modules*/
#define EJS_FLAG_DOC            0x40        /**< Load documentation from modules */
#define EJS_FLAG_NOEXIT         0x200       /**< App should service events and not exit */
#define EJS_FLAG_DYNAMIC        0x400       /**< Make a type that is dynamic itself */
#define EJS_STACK_ARG           -1          /* Offset to locate first arg */

/** 
    Configured numeric type
 */
#define BLD_FEATURE_NUM_TYPE double
typedef BLD_FEATURE_NUM_TYPE MprNumber;

/*  
    Sizes (in bytes) of encoded types in a ByteArray
 */
#define EJS_SIZE_BOOLEAN        1
#define EJS_SIZE_SHORT          2
#define EJS_SIZE_INT            4
#define EJS_SIZE_LONG           8
#define EJS_SIZE_DOUBLE         8
#define EJS_SIZE_DATE           8

/*  
    Reserved and system Namespaces
    The empty namespace is special. When seaching for properties, the empty namespace implies to search all namespaces.
    When properties are defined without a namespace, they are defined in the the empty namespace.
 */
#define EJS_EMPTY_NAMESPACE         ""
#define EJS_BLOCK_NAMESPACE         "-block-"
#define EJS_CONSTRUCTOR_NAMESPACE   "-constructor-"
#define EJS_EJS_NAMESPACE           "ejs"
#define EJS_ITERATOR_NAMESPACE      "iterator"
#define EJS_INIT_NAMESPACE          "-initializer-"
#define EJS_INTERNAL_NAMESPACE      "internal"
#define EJS_META_NAMESPACE          "meta"
#define EJS_PRIVATE_NAMESPACE       "private"
#define EJS_PROTECTED_NAMESPACE     "protected"
#define EJS_PROTOTYPE_NAMESPACE     "-prototype-"
#define EJS_PUBLIC_NAMESPACE        "public"
#define EJS_WORKER_NAMESPACE        "ejs.worker"

/*  
    Flags for fast comparison of namespaces
 */
#define EJS_NSP_PRIVATE         0x1
#define EJS_NSP_PROTECTED       0x2

/*  
    When allocating slots, name hashes and traits, we optimize by rounding up allocations
 */
#define EJS_PROP_ROUNDUP(x) (((x) + EJS_ROUND_PROP - 1) / EJS_ROUND_PROP * EJS_ROUND_PROP)

/*  Property enumeration flags
 */
#define EJS_FLAGS_ENUM_INHERITED 0x1            /**< Enumerate inherited base classes */
#define EJS_FLAGS_ENUM_ALL      0x2             /**< Enumerate non-enumerable and fixture properties */

/*  
    Exception flags and structure
 */
#define EJS_EX_CATCH            0x1             /* Definition is a catch block */
#define EJS_EX_FINALLY          0x2             /* Definition is a finally block */
#define EJS_EX_ITERATION        0x4             /* Definition is an iteration catch block */
#define EJS_EX_INC              4               /* Growth increment for exception handlers */

/*  
    Ejscript return codes.
 */
#define EJS_SUCCESS             MPR_ERR_OK
#define EJS_ERR                 MPR_ERR
#define EJS_EXCEPTION           (MPR_ERR_MAX - 1)

/*  
    Xml defines
 */
#define E4X_MAX_ELT_SIZE        (E4X_BUF_MAX-1)
#define E4X_TEXT_PROPERTY       "-txt"
#define E4X_TAG_NAME_PROPERTY   "-tag"
#define E4X_COMMENT_PROPERTY    "-com"
#define E4X_ATTRIBUTES_PROPERTY "-att"
#define E4X_PI_PROPERTY         "-pi"
#define E4X_PARENT_PROPERTY     "-parent"

#define EJS_XML_FLAGS_TEXT      0x1             /* Node is a text node */
#define EJS_XML_FLAGS_PI        0x2             /* Node is a processing instruction */
#define EJS_XML_FLAGS_COMMENT   0x4             /* Node is a comment */
#define EJS_XML_FLAGS_ATTRIBUTE 0x8             /* Node is an attribute */
#define EJS_XML_FLAGS_ELEMENT   0x10            /* Node is an element */

/*  
    XML node kinds
 */
#define EJS_XML_LIST        1
#define EJS_XML_ELEMENT     2
#define EJS_XML_ATTRIBUTE   3
#define EJS_XML_TEXT        4
#define EJS_XML_COMMENT     5
#define EJS_XML_PROCESSING  6

/*  
    Convenient slot aliases
 */
#define EJSLOT_CONSTRUCTOR          EJSLOT_Object___constructor__

/*  
    Default names
 */
#define EJS_GLOBAL                  "global"
#define EJS_DEFAULT_MODULE          "default"
#define EJS_DEFAULT_MODULE_NAME     EJS_DEFAULT_MODULE EJS_MODULE_EXT
#define EJS_BUILTIN_MODULE_NAME     "ejs"  EJS_MODULE_EXT
#define EJS_DEFAULT_CLASS_NAME      "__defaultClass__"
#define EJS_INITIALIZER_NAME        "__initializer__"

#define EJS_NAME                    "ejs"
#define EJS_MOD                     "ejs.mod"

/*
    File extensions
 */
#define EJS_MODULE_EXT              ".mod"
#define EJS_SOURCE_EXT              ".es"
#define EJS_LISTING_EXT             ".lst"

typedef struct EjsLoc {
    MprChar         *source;
    char            *filename;
    int             lineNumber;
} EjsLoc;

//  MOB -- reorder this file

/************************************************ Unicode ***********************************************/

typedef void EjsAny;

typedef struct EjsObj {
    struct EjsType  *type;
} EjsObj;

typedef struct EjsString {
    struct EjsType   *type;
    struct EjsString *next;              /* Hash chain link when interning */
    struct EjsString *prev;
    size_t           length;
    MprChar          value[0];
} EjsString;

extern void ejsManageString(EjsString *sp, int flags);

/************************************************* Helpers **********************************************/
/**
    Qualified name structure
    @description All names in Ejscript consist of a property name and a name space. Namespaces provide discrete
        spaces to manage and minimize name conflicts. These names will soon be converted to unicode.
    @stability Prototype
    @defgroup EjsName EjsName
    @see EjsName ejsName ejsAllocName ejsDupName ejsCopyName
 */       
typedef struct EjsName {
    EjsString   *name;                          /**< Property name */
    EjsString   *space;                         /**< Property namespace */
} EjsName;

extern void ejsMarkName(EjsName *qname);


/** 
    Allocation and Type Helpers
    @description The type helpers interface defines the set of primitive operations a type must support to
        interact with the virtual machine.
    @ingroup EjsType
 */
typedef struct EjsHelpers {
    /* Used by objects and values */
    EjsAny  *(*cast)(struct Ejs *ejs, EjsAny *obj, struct EjsType *type);
    void    *(*clone)(struct Ejs *ejs, EjsAny *obj, bool deep);
//  MOB -- rename alloc/free
    EjsAny  *(*create)(struct Ejs *ejs, struct EjsType *type, int size);
    int     (*defineProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname, struct EjsType *propType, 
                int64 attributes, EjsAny *value);
    int     (*deleteProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    int     (*deletePropertyByName)(struct Ejs *ejs, EjsAny *obj, EjsName qname);
    EjsAny  *(*getProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    EjsAny  *(*getPropertyByName)(struct Ejs *ejs, EjsAny *obj, EjsName qname);
    int     (*getPropertyCount)(struct Ejs *ejs, EjsAny *obj);
    EjsName (*getPropertyName)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    struct EjsTrait *(*getPropertyTraits)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    EjsAny  *(*invokeOperator)(struct Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);
    int     (*lookupProperty)(struct Ejs *ejs, EjsAny *obj, EjsName qname);
    int     (*setProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum, EjsAny *value);
    int     (*setPropertyByName)(struct Ejs *ejs, EjsAny *obj, EjsName qname, EjsAny *value);
    int     (*setPropertyName)(struct Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname);
    int     (*setPropertyTraits)(struct Ejs *ejs, EjsAny *obj, int slotNum, struct EjsType *type, int attributes);
} EjsHelpers;


//  MOB title
/************************************************* EjsMem ***********************************************/

#define POT(ejs, ptr)    (TYPE(ptr)->isPot)

#define DYNAMIC(ptr)        (MPR_GET_MEM(ptr)->dynamic)
#define VISITED(ptr)        (MPR_GET_MEM(ptr)->visited)
#define BUILTIN(ptr)        (MPR_GET_MEM(ptr)->builtin)
#define TYPE(ptr)           (((EjsObj*) (ptr))->type)

//  MOB dividor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct EjsLoadState {
    MprList         *typeFixups;            /**< Loaded types to fixup */
    int             firstModule;            /**< First module in ejs->modules for this load */
    int             flags;                  /**< Module load flags */
} EjsLoadState;

typedef void (*EjsLoaderCallback)(struct Ejs *ejs, int kind, ...);

/*
    Interned unicode string hash
 */
typedef struct EjsIntern {
    EjsString       *buckets;               /**< Hash buckets and references to link chains of strings (unicode) */
    int             size;                   /**< Size of hash */
    int             count;                  /**< Number of strings */
    int             accesses;
    int             reuse;
} EjsIntern;


/********** Special Values *******************/

#if UNUSED
#define NS_EJS                  1
#define NS_EMPTY                2
#define NS_INTERNAL             3
#define NS_ITERATOR             4
#define NS_PUBLIC               5

#define S_COMMA_PROTECTED       6
#define S_EMPTY                 7
#define S_LENGTH                8
#define S_PUBLIC                9

#define V_FALSE                 10
#define V_INFINITY              11
#define V_ITERATOR              12
#define V_MAX                   13
#define V_MIN                   14
#define V_MINUS_ONE             15
#define V_NAN                   16
#define V_NEGATIVE_INFINITY     17
#define V_NOP                   18
#define V_NULL                  19
#define V_ONE                   20
#define V_TRUE                  21
#define V_UNDEFINED             22
#define V_ZERO                  23
#endif
#define EJS_MAX_SPECIAL         1

extern void ejsSetSpecial(struct Ejs *ejs, int index, EjsAny *value);
extern EjsAny *ejsGetSpecial(struct Ejs *ejs, int index);

#define V(i) ejsGetSpecial(ejs, i)

/**
    Ejsript Interperter Structure
    @description The Ejs structure contains the state for a single interpreter. The #ejsCreate routine may be used
        to create multiple interpreters and returns a reference to be used in subsequent Ejscript API calls.
    @stability Prototype.
    @defgroup Ejs Ejs
    @see ejsCreate, ejsCreateService, ejsAppendSearchPath, ejsSetSearchPath, ejsEvalFile, ejsEvalScript, ejsExit
 */
typedef struct Ejs {
    EjsAny              *exception;         /**< Pointer to exception object */
    EjsAny              *result;            /**< Last expression result */
    struct EjsState     *state;             /**< Current evaluation state and stack */
    struct EjsState     *masterState;       /**< Owns the eval stack */

    struct EjsService   *service;           /**< Back pointer to the service */
    EjsIntern           intern;             /**< Interned Unicode string hash */
    cchar               *bootSearch;        /**< Module search when bootstrapping the VM */
    struct EjsArray     *search;            /**< Module load search path */
    cchar               *className;         /**< Name of a specific class to run for a program */
    cchar               *methodName;        /**< Name of a specific method to run for a program */

    /*
        Essential types
     */
    //  MOB - should these be in special values except for the really common ones?
    struct EjsType      *appType;           /**< App type */
    struct EjsType      *arrayType;         /**< Array type */
    struct EjsType      *blockType;         /**< Block type */
    struct EjsType      *booleanType;       /**< Boolean type */
    struct EjsType      *byteArrayType;     /**< ByteArray type */
    struct EjsType      *configType;        /**< Config type */
    struct EjsType      *dateType;          /**< Date type */
    struct EjsType      *errorType;         /**< Error type */
    struct EjsType      *errorEventType;    /**< ErrorEvent type */
    struct EjsType      *eventType;         /**< Event type */
    struct EjsType      *frameType;         /**< Frame type */
    struct EjsType      *fileType;          /**< File type */
    struct EjsType      *fileSystemType;    /**< FileSystem type */
    struct EjsType      *functionType;      /**< Function type */
    struct EjsType      *httpType;          /**< Http type */
    struct EjsType      *iteratorType;      /**< Iterator type */
    struct EjsType      *mathType;          /**< Math type */
    struct EjsType      *namespaceType;     /**< Namespace type */
    struct EjsType      *nullType;          /**< Null type */
    struct EjsType      *numberType;        /**< Default numeric type */
    struct EjsType      *objectType;        /**< Object type */
    struct EjsType      *pathType;          /**< Path type */
    struct EjsType      *regExpType;        /**< RegExp type */
    struct EjsType      *requestType;       /**< Request type */
    struct EjsType      *sessionType;       /**< Session type object */
    struct EjsType      *stringType;        /**< String type */
    struct EjsType      *socketType;        /**< Socket type */
    struct EjsType      *stopIterationType; /**< StopIteration type */
    struct EjsType      *typeType;          /**< Type type */
    struct EjsType      *uriType;           /**< URI type */
    struct EjsType      *voidType;          /**< Void type */
    struct EjsType      *webType;           /**< Web type */
    struct EjsType      *workerType;        /**< Worker type */
    struct EjsType      *xmlType;           /**< XML type */
    struct EjsType      *xmlListType;       /**< XMLList type */

    /*
        Key values
     */
    EjsAny              *global;            /**< The "global" object */
#if BLD_DEBUG
    struct EjsBlock     *globalBlock;
#endif

    EjsAny              *values[EJS_MAX_SPECIAL];

    //  MOB -- remove most of these
    EjsAny              *falseValue;        /**< The "false" value */
    struct EjsNumber    *infinityValue;     /**< The infinity number value */
    struct EjsIterator  *iterator;          /**< Default iterator */
    struct EjsNumber    *maxValue;          /**< Maximum number value */
    struct EjsNumber    *minValue;          /**< Minimum number value */
    struct EjsNumber    *minusOneValue;     /**< The -1 number value */
    struct EjsNumber    *nanValue;          /**< The "NaN" value if floating point numbers, else zero */
    struct EjsNumber    *negativeInfinityValue; /**< The negative infinity number value */
    struct EjsFunction  *nopFunction;       /**< The NOP function */
    EjsAny              *nullValue;         /**< The "null" value */
    struct EjsNumber    *oneValue;          /**< The 1 number value */
    EjsAny              *trueValue;         /**< The "true" value */
    EjsAny              *undefinedValue;    /**< The "void" value */
    struct EjsNumber    *zeroValue;         /**< The 0 number value */

    //  MOB - need a more scalable solution (index based on first 4 chars only)
    EjsString           *emptyString;       /**< "" */
    EjsString           *lengthString;      /**< "length" */
    EjsString           *publicString;      /**< "public" */
    EjsString           *commaProtString;   /**< ",protected" */

    struct EjsNamespace *emptySpace;        /**< Empty namespace */
    struct EjsNamespace *ejsSpace;          /**< Ejs namespace */
    struct EjsNamespace *iteratorSpace;     /**< Iterator namespace */
    struct EjsNamespace *internalSpace;     /**< Internal namespace */
    struct EjsNamespace *publicSpace;       /**< Public namespace */

    EjsHelpers          objHelpers;         /**< Default EjsObj helpers */
    EjsHelpers          potHelpers;         /**< Helper methods for Pots */
    char                *errorMsg;          /**< Error message */
    cchar               **argv;             /**< Command line args */
    int                 argc;               /**< Count of command line args */
    int                 flags;              /**< Execution flags */
    int                 exitStatus;         /**< Status to exit() */
    int                 firstGlobal;        /**< First global to examine for GC */
    int                 joining;            /**< In Worker.join */
    int                 serializeDepth;     /**< Serialization depth */
    int                 spreadArgs;         /**< Count of spread args */

    uint                compiling: 1;       /**< Currently executing the compiler */
    uint                empty: 1;           /**< Interpreter will be created empty */
    uint                initialized: 1;     /**< Interpreter fully initialized and not empty */
    uint                hasError: 1;        /**< Interpreter has an initialization error */
    uint                exiting: 1;         /**< VM should exit */

    EjsAny              *exceptionArg;      /**< Exception object for catch block */

    MprDispatcher       *dispatcher;        /**< Event dispatcher */
    MprList             *workers;           /**< Worker interpreters */
    MprList             *modules;           /**< Loaded modules */

    void                (*loaderCallback)(struct Ejs *ejs, int kind, ...);
    void                *userData;          /**< User data */

    EjsObj              *coreTypes;         /**< Core type instances */
    MprHashTable        *doc;               /**< Documentation */
    void                *sqlite;            /**< Sqlite context information */

    Http                *http;              /**< Http service object (copy of EjsService.http) */
    HttpLoc             *loc;               /**< Current HttpLocation object for web start scripts */

#if UNUSED
    EjsAny              *sessions;          /**< Session cache */
    int                 sessionTimeout;     /**< Default session timeout */
    MprEvent            *sessionTimer;      /**< Session expiry timer */
#endif
    EjsAny              *applications;      /**< Application cache */
    int                 nextSession;        /**< Session ID counter */
    MprMutex            *mutex;             /**< Multithread locking */
} Ejs;


#if !DOXYGEN
/**
    Native Function signature
    @description This is the calling signature for C Functions.
    @param ejs Ejs reference returned from #ejsCreate
    @param thisObj Reference to the "this" object. (The object containing the method).
    @param argc Number of arguments.
    @param argv Array of arguments.
    @returns Returns a result variable or NULL on errors and exceptions.
    @stability Prototype.
 */
typedef struct EjsObj *(*EjsFun)(Ejs *ejs, EjsAny *thisObj, int argc, EjsObj **argv);

//LEGACY
typedef EjsFun EjsProc;
typedef EjsFun EjsNativeFunction;
#endif

//  TODO is this used?
typedef int (*EjsSortFn)(Ejs *ejs, EjsAny *p1, EjsAny *p2, cchar *name, int order);

/**
    Initialize a Qualified Name structure
    @description Initialize the statically allocated qualified name structure using a name and namespace.
    @param qname Reference to an existing, uninitialized EjsName structure
    @param space Namespace string
    @param name Name string
    @return A reference to the qname structure
    @ingroup EjsName
 */

//  MOB -- NAMING
extern EjsName ejsEmptyWideName(Ejs *ejs, MprChar *name);
extern EjsName ejsEmptyName(Ejs *ejs, cchar *name);
extern EjsName ejsWideName(Ejs *ejs, MprChar *space, MprChar *name);
extern EjsName ejsName(Ejs *ejs, cchar *space, cchar *name);

//  MOB -- NAMING
#define WEN(name) ejsEmptyWideName(ejs, name)
#define EN(name) ejsEmptyName(ejs, name)
#define N(space, name) ejsName(ejs, space, name)
#define WN(space, name) ejsWideName(ejs, space, name)

#if UNUSED
#define NS(space, name) ejsName(ejs, space, name)
#endif

/** 
    Property traits. 
    @description Property traits describe the type and access attributes of a property. The Trait structure
        is used by EjsBlock to describe the attributes of properties defined within a block.
        Note: These traits apply to a property definition and not to the referenced object. ie. two property 
        definitions may have different traits but will refer to the same object.
    @stability Evolving
    @ingroup EjsBlock
 */
typedef struct EjsTrait {
    struct EjsType  *type;                  /**< Property type (prototype) */
    int             attributes;             /**< Modifier attributes */
} EjsTrait;


//  MOB OPT packing
typedef struct EjsSlot {
    EjsName         qname;                  /**< Property name */
    int             hashChain;              /**< Next property in hash chain */
    EjsTrait        trait;                  /**< Property descriptor traits */
    union {
        EjsAny      *ref;                   /**< Property reference */
        MprNumber   *value;                 /**< Immediate number value */
    } value;
} EjsSlot;


typedef struct EjsHash {
    int             size;                   /**< Size of hash */
    int             *buckets;               /**< Hash buckets and head of link chains */
} EjsHash;


typedef struct EjsProperties {
    EjsHash         *hash;                  /**< Hash buckets and head of link chains */
    int             size;                   /**< Current size of slots[] in elements */
    struct EjsSlot  slots[0];               /**< Vector of slots containing property references */
} EjsProperties;


//  MOB revise doc
/** 
    Object Type. Base type for all objects.
    @description The EjsPot type is the foundation for all types, blocks, functions and scripted classes. 
        It provides storage and hashed lookup for properties.
        \n\n
        EjsPot stores properties in an array of slots. These slots store a reference to the property value. 
        Property names are stored in a names hash. Dynamic objects own their own name hash. Sealed object instances 
        of a type, will simply refer to the hash of names owned by the type.
        \n\n
        EjsPots may be either dynamic or sealed. Dynamic objects can grow the number of properties. Sealed 
        objects cannot. Sealed objects will store the slot array as part of the EjsPot memory chunk. Dynamic 
        objects will perform a separate allocation for the slot array so that it can grow.
    @stability Evolving.
    @defgroup EjsPot EjsPot
    @see EjsPot ejsIsPot ejsCreateSimpleObject ejsCreateObject ejsCloneObject ejsGrowObject ejsManageObject
        MOB - change these From Var
        ejsGetVarType ejsAllocObj ejsFreeObj ejsCast ejsClone ejsCreateInstance ejsCreateVar
        ejsDestroyVar ejsDefineProperty ejsDeleteProperty ejsDeletePropertyByName
        ejsGetProperty ejsLookupProperty ejsSetProperty ejsSetPropertyByName ejsSetPropertyName
        ejsSetPropertyTraits ejsDeserialize ejsParseVar
 */
typedef struct EjsPot {
    struct EjsType  *type;                      /**< Type of the object */
#if BLD_HAS_UNNAMED_UNIONS
    union {
        struct {
#endif
            uint    isBlock         : 1;        /**< Instance is a block */
            uint    isFrame         : 1;        /**< Instance is a frame */
            uint    isFunction      : 1;        /**< Instance is a function */
            uint    isPrototype     : 1;        /**< Object is a type prototype object */
            uint    isType          : 1;        /**< Instance is a type object */
            uint    separateHash    : 1;        /**< Object has separate hash memory */
            uint    separateSlots   : 1;        /**< Object has separate slots[] memory */
            uint    shortScope      : 1;        /**< Don't follow type or base classes */
#if BLD_HAS_UNNAMED_UNIONS
        };
        int         bits;
    };
#endif
    EjsProperties   *properties;                /** Object properties */
    //  TODO - OPT - merge numProp with bits above (24 bits)
    int             numProp;                    /** Number of properties */
#if BLD_DEBUG
    MprMem          *mem;                       /**< Pointer to underlying memory block */
#endif
} EjsPot;

extern int ejsIs(EjsAny *obj, int slot);
#define ejsIsPot(ejs, obj) (obj && POT(ejs, obj))

#define ejsGetName(ptr) mprGetName(ptr)
#define ejsSetName(ptr, name) mprSetName(ptr, name)
#define ejsCopyName(dest, src) mprSetName(dest, mprGetName(src))
#if BLD_DEBUG
    #define ejsSetMemRef(obj) if (1) { obj->mem = MPR_GET_MEM(obj); } else 
#else
    #define ejsSetMemRef(obj) 
#endif

/** 
    Allocate a new variable
    @description This will allocate space for a bare variable. This routine should only be called by type factories
        when implementing the createVar helper.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type object from which to create an object instance
    @param extra Size of extra property slots to reserve. This is used for dynamic objects.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
extern EjsAny *ejsAlloc(Ejs *ejs, struct EjsType *type, int extra);

/** 
    Cast a variable to a new type
    @description Cast a variable and return a new variable of the required type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to cast
    @param type Type to cast to
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsAny
 */
extern EjsAny *ejsCast(Ejs *ejs, EjsAny *obj, struct EjsType *type);

/** 
    Clone a variable
    @description Copy a variable and create a new copy. This may do a shallow or deep copy. A shallow copy
        will not copy the property instances, rather it will only duplicate the property reference. A deep copy
        will recursively clone all the properties of the variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to clone
    @param deep Set to true to do a deep copy.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsAny
 */
extern EjsAny *ejsClone(Ejs *ejs, EjsAny *obj, bool deep);

/** 
    Create a new variable instance 
    @description Create a new variable instance and invoke any required constructors with the given arguments.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type from which to create a new instance
    @param argc Count of args in argv
    @param argv Vector of arguments. Each arg is an EjsAny.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsAny
 */
extern EjsAny *ejsCreateInstance(Ejs *ejs, struct EjsType *type, int argc, void *argv);

/** 
    Create a variable
    @description Create a variable of the required type. This invokes the createVar helper method for the specified type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type to cast to
    @param numSlots Size of extra property slots to reserve. This is used for dynamic objects.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsAny
 */
extern EjsAny *ejsCreate(Ejs *ejs, struct EjsType *type, int numSlots);

/** 
    Define a property
    @description Define a property in a variable and give it a name, base type, attributes and default value.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object in which to define a property
    @param slotNum Slot number in the variable for the property. Slots are numbered sequentially from zero. Set to
        -1 to request the next available slot number.
    @param qname Qualified name containing a name and a namespace.
    @param type Base type of the property. Set to ejs->voidType to leave as untyped.
    @param attributes Attribute traits. Useful attributes include:
        @li EJS_FUN_OVERRIDE
        @li EJS_ATTR_CONST
        @li EJS_ATTR_ENUMERABLE
    @param value Initial value of the property
    @return A postitive slot number or a negative MPR error code.
    @ingroup EjsAny
 */
extern int ejsDefineProperty(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname, struct EjsType *type, int64 attributes, 
    EjsAny *value);

/** 
    Delete a property
    @description Delete a variable's property and set its slot to null. The slot is not reclaimed and subsequent properties
        are not compacted.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable in which to delete the property
    @param slotNum Slot number in the variable for the property to delete.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsAny
 */
extern int ejsDeleteProperty(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Delete a property by name
    @description Delete a variable's property by name and set its slot to null. The property is resolved by using 
        ejsLookupProperty with the specified name. Once deleted, the slot is not reclaimed and subsequent properties
        are not compacted.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable in which to delete the property
    @param qname Qualified name for the property including name and namespace.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsAny
 */
extern int ejsDeletePropertyByName(Ejs *ejs, EjsAny *obj, EjsName qname);

/** 
    Get a property
    @description Get a property from a variable at a given slot.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @return The variable property stored at the nominated slot.
    @ingroup EjsAny
 */
extern EjsAny *ejsGetProperty(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Get a count of properties in a variable
    @description Get a property from a variable at a given slot.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @return A positive integer count of the properties stored by the variable. 
    @ingroup EjsAny
 */
extern int ejsGetLength(Ejs *ejs, EjsAny *obj);

//  MOB -- globally change
#define ejsGetPropertyCount(ejs, obj) ejsGetLength(ejs, obj)

/** 
    Get a variable property's name
    @description Get a property name for the property at a given slot in the  variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @return The qualified property name including namespace and name. Caller must not free.
    @ingroup EjsAny
 */
extern EjsName ejsGetPropertyName(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Get a property by name
    @description Get a property from a variable by name.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param qname Qualified name specifying both a namespace and name.
    @return The variable property stored at the nominated slot.
    @ingroup EjsAny
 */
extern EjsAny *ejsGetPropertyByName(Ejs *ejs, EjsAny *obj, EjsName qname);

/** 
    Get a property's traits
    @description Get a property's trait description. The property traits define the properties base type,
        and access attributes.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param slotNum Slot number for the requested property.
    @return A trait structure reference for the property.
    @ingroup EjsAny
 */
extern struct EjsTrait *ejsGetPropertyTraits(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Invoke an opcode on a native type.
    @description Invoke an Ejscript byte code operator on the specified variable given the expression right hand side.
        Native types would normally implement the invokeOperator helper function to respond to this function call.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param opCode Byte ope code to execute
    @param rhs Expression right hand side for binary expression op codes. May be null for other op codes.
    @return The result of the op code or NULL if the opcode does not require a result.
    @ingroup EjsAny
 */
extern EjsAny *ejsInvokeOperator(Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);
extern EjsAny *ejsInvokeOperatorDefault(Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);

/** 
    Lookup a property by name
    @description Search for a property by name in the given variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param qname Qualified name of the property to search for.
    @return The slot number containing the property. Then use #ejsGetProperty to retrieve the property or alternatively
        use ejsGetPropertyByName to lookup and retrieve in one step.
    @ingroup EjsAny
 */
extern int ejsLookupProperty(Ejs *ejs, EjsAny *obj, EjsName qname);

/** 
    Set a property's value
    @description Set a value for a property at a given slot in the specified variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @param value Reference to a value to store.
    @return The slot number of the property updated.
    @ingroup EjsAny
 */
extern int ejsSetProperty(Ejs *ejs, void *obj, int slotNum, void *value);

/** 
    Set a property's value 
    @description Set a value for a property. The property is located by name in the specified variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param qname Qualified property name.
    @param value Reference to a value to store.
    @return The slot number of the property updated.
    @ingroup EjsAny
 */
extern int ejsSetPropertyByName(Ejs *ejs, void *obj, EjsName qname, void *value);

/** 
    Set a property's name 
    @description Set a qualified name for a property at the specified slot in the variable. The qualified name
        consists of a namespace and name - both of which must be persistent. A typical paradigm is for these name
        strings to be owned by the memory context of the variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param slotNum Slot number of the property in the variable.
    @param qname Qualified property name.
    @return The slot number of the property updated.
    @ingroup EjsAny
 */
extern int ejsSetPropertyName(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname);

/** 
    Set a property's traits
    @description Set the traits describing a property. These include the property's base type and access attributes.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param slotNum Slot number of the property in the variable.
    @param type Base type for the property. Set to NULL for an untyped property.
    @param attributes Integer mask of access attributes.
    @return The slot number of the property updated.
    @ingroup EjsAny
 */
extern int ejsSetPropertyTraits(Ejs *ejs, EjsAny *obj, int slotNum, struct EjsType *type, int attributes);

//  TODO - DOC - 
extern EjsAny *ejsDeserialize(Ejs *ejs, EjsString *value);

//  MOB -- should this be EjsString?
extern EjsAny *ejsParse(Ejs *ejs, MprChar *str,  int prefType);
extern void ejsZeroSlots(Ejs *ejs, EjsSlot *slots, int count);

//  MOB -- bad signature
extern void ejsCopySlots(Ejs *ejs, EjsPot *pot, EjsSlot *dest, EjsSlot *src, int count);

/** 
    Create an empty property object
    @description Create a simple object using Object as its base type.
    @param ejs Interpreter instance returned from #ejsCreate
    @return A new object instance
    @ingroup EjsObj
 */
extern EjsAny *ejsCreateEmptyPot(Ejs *ejs);

/** 
    Create an object instance of the specified type
    @description Create a new object using the specified type as a base class. 
        Note: the constructor is not called. If you require the constructor to be invoked, use #ejsCreateInstance
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Base type to use when creating the object instance
    @param size Number of extra slots to allocate when creating the object
    @return A new object instance
    @ingroup EjsObj
 */
extern void *ejsCreatePot(Ejs *ejs, struct EjsType *type, int size);

extern int ejsCompactPot(Ejs *ejs, EjsPot *obj);
extern int ejsInsertPotProperties(Ejs *ejs, EjsPot *pot, int numSlots, int offset);
extern int ejsMakeHash(Ejs *ejs, EjsPot *obj);
extern int ejsPropertyHasTrait(Ejs *ejs, EjsAny *obj, int slotNum, int attributes);
extern int ejsRemovePotProperty(Ejs *ejs, EjsAny *obj, int slotNum);
extern int ejsLookupPotProperty(Ejs *ejs, EjsPot *obj, EjsName qname);
extern EjsName ejsGetPotPropertyName(Ejs *ejs, EjsPot *obj, int slotNum);


/** 
    Copy an object
    @description Copy an object create a new instance. This may do a shallow or deep copy depending on the value of 
        \a deep. A shallow copy will not copy the property instances, rather it will only duplicate the property 
        reference. A deep copy will recursively clone all the properties of the variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param src Source object to copy
    @param deep Set to true to do a deep copy.
    @return A newly allocated object. Caller must not free as the GC will manage the lifecycle of the variable.
    @ingroup EjsObj
 */
extern EjsAny *ejsClonePot(Ejs *ejs, EjsAny *src, bool deep);

/** 
    Grow a pot object
    @description Grow the property storage for an object. Object properties are stored in slots. To store more 
        properties, you need to grow the slots.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object reference to grow
    @param numSlots New minimum count of properties. If size is less than the current number of properties, the call
        will be ignored, i.e. it will not shrink objects.
    @return Zero if successful
    @ingroup EjsObj
 */
extern int ejsGrowPot(Ejs *ejs, EjsPot *obj, int numSlots);

/** 
    Mark an object as currently in use.
    @description Mark an object as currently active so the garbage collector will preserve it. This routine should
        be called by native types that extend EjsObj in their markVar helper.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to mark as currently being used.
    @ingroup EjsObj
 */
extern void ejsManagePot(void *obj, int flags);

extern int      ejsGetSlot(Ejs *ejs, EjsPot *obj, int slotNum);
extern EjsObj   *ejsCoerceOperands(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs);
extern int      ejsComputeStringHashCode(EjsString *name, int size);
extern int      ejsComputeMultiHashCode(cchar *name, int size);
extern int      ejsGetHashSize(int numProp);
extern void     ejsCreatePotHelpers(Ejs *ejs);

//  MOB -- rename
extern void     ejsMakePropertyDontDelete(EjsObj *obj, int dontDelete);
extern int      ejsMakePropertyEnumerable(EjsObj *obj, bool enumerable);
extern void     ejsMakePropertyReadOnly(EjsObj *obj, int readonly);
extern int      ejsRebuildHash(Ejs *ejs, EjsPot *obj);
extern void     ejsResetHash(Ejs *ejs, EjsPot *obj);
extern void     ejsRemoveSlot(Ejs *ejs, EjsPot *slots, int slotNum, int compact);
extern void     ejsSetAllocIncrement(Ejs *ejs, struct EjsType *type, int increment);
extern EjsObj   *ejsToSource(Ejs *ejs, EjsObj *obj, int argc, void *argv);

extern EjsString *ejsObjToString(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv);
extern EjsString *ejsObjToJSON(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv);
extern int ejsBlendObject(Ejs *ejs, EjsObj *dest, EjsObj *src, int overwrite);

/********************************************** String ********************************************/
/** 
    String Class
    @description The String class provides the base class for all strings. Each String object represents a single 
    immutable linear sequence of characters. Strings have operators for: comparison, concatenation, copying, 
    searching, conversion, matching, replacement, and, subsetting.
    \n\n
    Strings are currently sequences of Unicode characters. Depending on the configuration, they may be 8, 16 or 32 bit
    code point values.
    @stability Evolving
    @defgroup EjsString EjsString
    @see EjsString ejsToString ejsCreateString ejsCreateBareString ejsCreateStringWithLength ejsDupString
        ejsVarToString ejsStrdup ejsStrcat ejsIsString
 */
//  MOB -- need definition here

/** 
    Create a string object
    @param ejs Ejs reference returned from #ejsCreate
    @param value C string value to define for the string object. Note: this will soon be changed to unicode.
    @stability Prototype
    @return A string object
    @ingroup EjsString
 */
extern EjsString *ejsCreateString(Ejs *ejs, MprChar *value, size_t len);
extern EjsString *ejsCreateStringFromConst(Ejs *ejs, struct EjsModule *mp, int index);
extern EjsString *ejsCreateStringFromAsc(Ejs *ejs, cchar *value);
extern EjsString *ejsCreateStringFromBytes(Ejs *ejs, cchar *value, size_t len);
extern EjsString *ejsCreateStringFromMulti(Ejs *ejs, cchar *value, size_t len);

/** 
    Create an empty string object. This creates an uninitialized string object of the requrired size. Once initialized,
        the string must be "interned" via $ejsInternString.
    @param ejs Ejs reference returned from #ejsCreate
    @param len Length of space to reserve for future string data
    @return A string object
    @ingroup EjsString
 */
extern EjsString *ejsCreateBareString(Ejs *ejs, size_t len);

//  MOB DOC
extern EjsString *ejsCreateNonInternedString(Ejs *ejs, MprChar *value, size_t len);

/** 
    Intern a string object. This stores the string in the internal string pool. This is required if the string was
    created via ejsCreateBareString. The ejsCreateString routine will intern the string automatcially.
    @param ejs Ejs reference returned from #ejsCreate
    @param len Length of space to reserve for future string data
    @return The internalized string object. NOTE: this may be different to the object passed in, if the string value
        was already present in the intern pool.
    @ingroup EjsString
 */
extern EjsString *ejsInternString(Ejs *ejs, EjsString *sp);
extern EjsString *ejsInternMulti(struct Ejs *ejs, cchar *value, size_t len);
extern EjsString *ejsInternAsc(struct Ejs *ejs, cchar *value, size_t len);
extern EjsString *ejsInternWide(struct Ejs *ejs, MprChar *value, size_t len);
extern void ejsManageIntern(Ejs *ejs, int flags);

extern int       ejsAtoi(Ejs *ejs, EjsString *sp, int radix);
extern EjsString *ejsCatString(Ejs *ejs, EjsString *dest, EjsString *src);
extern EjsString *ejsCatStrings(Ejs *ejs, EjsString *src, ...);
extern EjsString *ejsSubstring(Ejs *ejs, EjsString *src, size_t start, size_t len);
extern int       ejsCompareString(Ejs *ejs, EjsString *s1, EjsString *s2);
extern int       ejsCompareSubstring(Ejs *ejs, EjsString *s1, EjsString *s2, size_t offset, size_t len);
extern EjsString *ejsToLower(Ejs *ejs, EjsString *sp);
extern EjsString *ejsToUpper(Ejs *ejs, EjsString *sp);
extern EjsString *ejsTruncateString(Ejs *ejs, EjsString *sp, size_t len);

//  MIXED modes
extern int       ejsCompareMulti(Ejs *ejs, EjsString *s1, cchar *s2);
extern int       ejsCompareWide(Ejs *ejs, EjsString *s1, MprChar *s2, size_t len);
extern int       ejsContainsChar(Ejs *ejs, EjsString *sp, int charPat);
extern int       ejsContainsMulti(Ejs *ejs, EjsString *sp, cchar *pat);
extern int       ejsContainsString(Ejs *ejs, EjsString *sp, EjsString *pat);
extern int       ejsStartsWithMulti(Ejs *ejs, EjsString *sp, cchar *pat);
extern char      *ejsToMulti(Ejs *ejs, void *obj);
extern EjsString *ejsSprintf(Ejs *ejs, cchar *fmt, ...);

/**
    Convert a variable to a string in JSON format
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Value to cast
    @param options Encoding options. See serialize for details.
    @return A string object
    @ingroup MOB
 */
extern EjsString *ejsToJSON(Ejs *ejs, EjsAny *obj, EjsObj *options);

//  MOB - low level serialize
extern EjsString *ejsSerialize(Ejs *ejs, EjsAny *obj, EjsObj *options);

/** 
    Cast a variable to a string
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to convert
    @return A string object
    @ingroup MOB
 */
extern EjsString *ejsToString(Ejs *ejs, EjsAny *obj);

#if DOXYGEN
    bool ejsIsString(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsString(ejs, obj) (obj && TYPE(obj)->id == ES_String)
#endif

extern EjsString *ejsStringToJSON(Ejs *ejs, EjsObj *obj);

/** 
    Array class
    @description Arrays provide a growable, integer indexed, in-memory store for objects. An array can be treated as a 
        stack (FIFO or LIFO) or a list (ordered). Insertions can be done at the beginning or end of the stack or at an 
        indexed location within a list. The Array class can store objects with numerical indicies and can also store 
        any named properties. The named properties are stored in the obj field, whereas the numeric indexed values are
        stored in the data field. Array extends EjsObj and has all the capabilities of EjsObj.
    @stability Evolving
    @defgroup EjsArray EjsArray
    @see EjsArray ejsCreateArray ejsIsArray
 */
typedef struct EjsArray {
    EjsPot          pot;                /**< Property storage */
    EjsObj          **data;             /**< Array elements */
    int             length;             /**< Array length property */
} EjsArray;


/** 
    Create an array
    @param ejs Ejs reference returned from #ejsCreate
    @param size Initial size of the array
    @return A new array object
    @ingroup EjsArray
 */
extern EjsArray *ejsCreateArray(Ejs *ejs, int size);
extern EjsArray *ejsCloneArray(Ejs *ejs, EjsArray *ap, bool deep);

extern int ejsAddItem(Ejs *ejs, EjsArray *ap, EjsAny *item);
extern int ejsAppendArray(Ejs *ejs, EjsArray *dest, EjsArray *src);
extern void ejsClearArray(Ejs *ejs, EjsArray *ap);
extern int ejsInsertItem(Ejs *ejs, EjsArray *ap, int index, EjsAny *item);
extern void *ejsGetItem(Ejs *ejs, EjsArray *ap, int index);
extern void *ejsGetFirstItem(Ejs *ejs, EjsArray *ap);
extern void *ejsGetLastItem(Ejs *ejs, EjsArray *ap);
extern void *ejsGetNextItem(Ejs *ejs, EjsArray *ap, int *next);
extern void *ejsGetPrevItem(Ejs *ejs, EjsArray *ap, int *next);
extern int ejsLookupItem(Ejs *ejs, EjsArray *lp, EjsAny *item);
extern int ejsRemoveItem(Ejs *ejs, EjsArray *ap, EjsAny *item);
extern int ejsRemoveLastItem(Ejs *ejs, EjsArray *ap);
extern int ejsRemoveItemAtPos(Ejs *ejs, EjsArray *ap, int index);

#if DOXYGEN
    /** 
        Determine if a variable is an array
        @param obj Object to test
        @return True if the variable is an array
        @ingroup EjsArray
     */
    extern bool ejsIsArray(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsArray(ejs, obj) (obj && TYPE(obj)->id == ES_Array)
#endif

/************************************************ Block ********************************************************/

//MOB - add dividors for all classes
/** 
    Block class
    @description The block class is the base class for all program code block scope objects. This is an internal class
        and not exposed to the script programmer.
    Blocks (including types) may describe their properties via traits. The traits store the property 
    type and access attributes and are stored in EjsBlock which is a sub class of EjsObj. See ejsBlock.c for details.
    @stability Evolving
    @defgroup EjsBlock EjsBlock
    @see EjsBlock ejsIsBlock ejsBindFunction
 */
typedef struct EjsBlock {
    EjsPot          pot;                            /**< Property storage */
    MprList         namespaces;                     /**< Current list of namespaces open in this block of properties */
    struct EjsBlock *scope;                         /**< Lexical scope chain for this block */
    struct EjsBlock *prev;                          /**< Previous block in activation chain */

    //  MOB -- OPT and compress / eliminate some of these fields. Every function has these.
    EjsObj          *prevException;                 /**< Previous exception if nested exceptions */
    EjsObj          **stackBase;                    /**< Start of stack in this block */
    uint            breakCatch: 1;                  /**< Return, break or continue in a catch block */
    uint            isGlobal: 1;                    /**< Block is the global block */
    uint            nobind: 1;                      /**< Don't bind to properties in this block */
#if BLD_DEBUG
    EjsLoc          loc;
#endif
} EjsBlock;


#if DOXYGEN
    /** 
        Determine if a variable is a block.
        @description This call tests if the variable is a block.
        @param obj Object to test
        @returns True if the variable is based on EjsBlock
        @ingroup EjsBlock
     */
    extern bool ejsIsBlock(Ejs *ejs, EjsObj *obj);
#else
    #define ejsIsBlock(ejs, obj) (ejsIsPot(ejs, obj) && ((EjsPot*) (obj))->isBlock)
#endif

/** 
    Bind a native C function to a function property
    @description Bind a native C function to an existing javascript function. Functions are typically created
        by compiling a script file of native function definitions into a mod file. When loaded, this mod file 
        will create the function properties. This routine will then bind the specified C function to the 
        function property.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object containing the function property to bind.
    @param slotNum Slot number of the method property
    @param fun Native C function to bind
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
extern int ejsBindFunction(Ejs *ejs, EjsAny *obj, int slotNum, void *fun);

/*  
    This is all an internal API. Native types should probably not be using these routines. Speak up if you find
    you need these routines in your code.
 */

extern int      ejsAddNamespaceToBlock(Ejs *ejs, EjsBlock *blockRef, struct EjsNamespace *nsp);
extern int      ejsAddScope(EjsBlock *block, EjsBlock *scopeBlock);
extern EjsBlock *ejsCreateBlock(Ejs *ejs, int numSlots);

//  TODO - why do we have ejsCloneObject, ejsCloneBlock ... Surely ejsCloneVar is sufficient?
extern EjsBlock *ejsCloneBlock(Ejs *ejs, EjsBlock *src, bool deep);

extern int      ejsCaptureScope(Ejs *ejs, EjsBlock *block, struct EjsArray *scopeChain);
extern int      ejsCopyScope(EjsBlock *block, struct EjsArray *chain);
extern int      ejsGetNamespaceCount(EjsBlock *block);

extern EjsBlock *ejsGetTopScope(EjsBlock *block);
extern void     ejsManageBlock(EjsBlock *block, int flags);
extern void     ejsPopBlockNamespaces(EjsBlock *block, int count);
extern EjsBlock *ejsRemoveScope(EjsBlock *block);
extern void     ejsResetBlockNamespaces(Ejs *ejs, EjsBlock *block);

#if BLD_DEBUG
extern void     ejsSetBlockLocation(EjsBlock *block, EjsLoc *loc);
#else
#define ejsSetBlockLocation(block, loc)
#endif

/** 
    Exception Handler Record
    @description Each exception handler has an exception handler record allocated that describes it.
    @ingroup EjsFunction
 */
typedef struct EjsEx {
// TODO - OPT. Should this be compressed via bit fields for flags Could use short for these offsets.
    struct EjsType  *catchType;             /**< Type of error to catch */
    uint            flags;                  /**< Exception flags */
    uint            tryStart;               /**< Ptr to start of try block */
    uint            tryEnd;                 /**< Ptr to one past the end */
    uint            handlerStart;           /**< Ptr to start of catch/finally block */
    uint            handlerEnd;             /**< Ptr to one past the end */
    uint            numBlocks;              /**< Count of blocks opened before the try block */
    uint            numStack;               /**< Count of stack slots pushed before the try block */
} EjsEx;


#define EJS_INDEX_INCR  256

typedef struct EjsConstants {
    char          *pool;                    /**< Constant pool string data */
    int           poolSize;                 /**< Size of constant pool storage in bytes */
    int           poolLength;               /**< Length of used bytes in constant pool */
    int           indexSize;                /**< Size of index in elements */
    int           indexCount;               /**< Number of constants used in index */
    int           locked;                   /**< No more additions allowed */
    MprHashTable  *table;                   /**< Hash table for fast lookup when compiling */
    EjsString     **index;                  /**< Interned string index */
} EjsConstants;

extern EjsConstants *ejsCreateConstants(Ejs *ejs, int count, size_t size);
extern int ejsGrowConstants(Ejs *ejs, EjsConstants *constants, size_t size);
extern int ejsAddConstant(Ejs *ejs, EjsConstants *constants, cchar *str);

#define EJS_DEBUG_INCR 16

typedef struct EjsLine {
    int         offset;                     /**< Optional PC offsets of each line in function */
    MprChar     *source;                    /**< Program source code. Format: path@line: code */         
} EjsLine;


typedef struct EjsDebug {
    int        size;                        /**< Size of lines[] in elements */
    int        numLines;                    /**< Number of entries in lines[] */
    EjsLine    lines[0];
} EjsDebug;

extern EjsDebug *ejsCreateDebug(Ejs *ejs);
extern int ejsAddDebugLine(Ejs *ejs, EjsDebug **debug, int offset, MprChar *source);
extern int ejsGetDebugInfo(Ejs *ejs, struct EjsFunction *fun, uchar *pc, char **path, int *lineNumber, MprChar **source);

// TODO OPT. Could compress this.
/** 
    Byte code
    @description This structure describes a sequence of byte code for a function. It also defines a set of
        execption handlers pertaining to this byte code.
    @ingroup EjsFunction
 */
typedef struct EjsCode {
    struct EjsModule *module;                /**< Module owning this function */
    EjsDebug         *debug;                 /**< Source code debug information */
    EjsEx            **handlers;             /**< Exception handlers */
    int              codeLen;                /**< Byte code length */
    int              debugOffset;            /**< Offset in mod file for debug info */
    int              numHandlers;            /**< Number of exception handlers */
    int              sizeHandlers;           /**< Size of handlers array */
    uchar            byteCode[0];            /**< Byte code */
} EjsCode;


/** 
    Function class
    @description The Function type is used to represent closures, function expressions and class methods. 
        It contains a reference to the code to execute, the execution scope and possibly a bound "this" reference.
    @stability Evolving
    @defgroup EjsFunction EjsFunction
    @see EjsFunction ejsIsFunction ejsIsNativeFunction ejsIsInitializer ejsCreateFunction ejsCloneFunction
        ejsRunFunctionBySlot ejsRunFunction ejsRunInitializer
 */
typedef struct EjsFunction {
    /*
        A function can store properties like any other object. If it has parameters, it must also must maintain an
        activation object. When compiling, the compiler stores parameters in the normal property "block", it then
        transfers them into the activation block when complete.
     */
    EjsBlock        block;                  /** Run-time properties for local vars */
    EjsPot          *activation;            /** Parameter and local properties */
    EjsString       *name;                  /** Function name */
#if FUTURE
    union {
#endif
        struct EjsFunction *setter;         /**< Setter function for this property */
        struct EjsType  *archetype;         /**< Type to use to create instances */
#if FUTURE && MOB
    } extra;
#endif
    union {
        EjsCode     *code;                  /**< Byte code */
        EjsFun      proc;                   /**< Native function pointer */
    } body;

    struct EjsArray *boundArgs;             /**< Bound "args" */
    EjsAny          *boundThis;             /**< Bound "this" object value */
    struct EjsType  *resultType;            /**< Return type of method */

#if BLD_HAS_UNNAMED_UNIONS
    union {
        struct {
#endif
            uint    numArgs: 8;             /**< Count of formal parameters */
            uint    numDefault: 8;          /**< Count of formal parameters with default initializers */
            uint    allowMissingArgs: 1;    /**< Allow unsufficient args for native functions */
            uint    castNulls: 1;           /**< Cast return values of null */
            uint    fullScope: 1;           /**< Closures must capture full scope */
            uint    hasReturn: 1;           /**< Function has a return stmt */
            uint    inCatch: 1;             /**< Executing catch block */
            uint    inException: 1;         /**< Executing catch/finally exception processing */
            uint    isConstructor: 1;       /**< Function is a constructor */
            uint    isInitializer: 1;       /**< Function is a type initializer */
            uint    isNativeProc: 1;        /**< Function is native procedure */
            uint    moduleInitializer: 1;   /**< Function is a module initializer */
            uint    rest: 1;                /**< Function has a "..." rest of args parameter */
            uint    staticMethod: 1;        /**< Function is a static method */
            uint    strict: 1;              /**< Language strict mode (vs standard) */
            uint    throwNulls: 1;          /**< Return type cannot be null */

#if BLD_HAS_UNNAMED_UNIONS
        };
        int64       bits;
    };
#endif
} EjsFunction;

#if DOXYGEN
    /** 
        Determine if a variable is a function. This will return true if the variable is a function of any kind, including
            methods, native and script functions or initializers.
        @param obj Variable to test
        @return True if the variable is a function
        @ingroup EjsFunction
     */
    //  MOB -- convert all ejsIs to take ejs as arg
    extern bool ejsIsFunction(Ejs *ejs, EjsAny *obj);

    /** 
        Determine if the function is a native function. Functions can be either native - meaning the implementation is
            via a C function, or can be scripted.
        @param obj Object to test
        @return True if the variable is a native function.
        @ingroup EjsFunction
     */
    //  MOB -- convert all ejsIs to take ejs as arg
    extern bool ejsIsNativeFunction(Ejs *ejs, EjsAny *obj);

    /** 
        Determine if the function is an initializer. Initializers are special functions created by the compiler to do
            static and instance initialization of classes during construction.
        @param obj Object to test
        @return True if the variable is an initializer
        @ingroup EjsFunction
     */
    //  MOB -- convert all ejsIs to take ejs as arg
    extern bool ejsIsInitializer(Ejs *ejs, EjsAny *obj);
#else
    //  MOB OPT
    #define ejsIsFunction(ejs, obj)       (obj && POT(ejs, obj) && ((EjsPot*) obj)->isFunction)
    #define ejsIsNativeFunction(ejs, obj) (ejsIsFunction(ejs, obj) && (((EjsFunction*) (obj))->isNativeProc))
    #define ejsIsInitializer(ejs, obj)    (ejsIsFunction(ejs, obj) && (((EjsFunction*) (obj))->isInitializer)
#endif

/** 
    Create a function object
    @description This creates a function object and optionally associates byte code with the function.
    @param ejs Ejs reference returned from #ejsCreate
    @param name Function name used in stack backtraces.
    @param code Pointer to the byte code. The byte code is not copied so this must be a persistent pointer.
    @param codeLen Length of the code.
    @param numArgs Number of formal arguments to the function.
    @param numDefault Number of default args to the function.
    @param numExceptions Number of exception handlers
    @param returnType Return type of the function. Set to NULL for no defined type.
    @param attributes Integer mask of access attributes.
    @param module Reference to the module owning the function.
    @param scope Reference to the chain of blocks that that comprises the lexical scope chain for this function.
    @param strict Run code in strict mode (vs standard).
    @return An initialized function object
    @ingroup EjsFunction
 */
//  MOB - refactor into several functions
extern EjsFunction *ejsCreateFunction(Ejs *ejs, EjsString *name, cuchar *code, int codeLen, int numArgs, int numDefault,
    int numExceptions, struct EjsType *returnType, int attributes, struct EjsModule *module, EjsBlock *scope, 
    int strict);
extern int ejsInitFunction(Ejs *ejs, EjsFunction *fun, EjsString *name, cuchar *code, int codeLen, int numArgs, 
    int numDefault, int numExceptions, struct EjsType *returnType, int attributes, struct EjsModule *module, 
    EjsBlock *scope, int strict);
extern EjsFunction *ejsCreateSimpleFunction(Ejs *ejs, EjsString *name, int attributes);
extern void ejsDisableFunction(Ejs *ejs, EjsFunction *fun);

extern EjsPot *ejsCreateActivation(Ejs *ejs, EjsFunction *fun, int numSlots);
extern void ejsCompleteFunction(Ejs *ejs, EjsFunction *fun);
extern void ejsUseActivation(Ejs *ejs, EjsFunction *fun);

/** 
    Run the initializer for a module
    @description A module's initializer runs global code defined in the module
    @param ejs Ejs reference returned from #ejsCreate
    @param module Module object reference
    @return The last expression result of global code executed
    @ingroup EjsFunction
 */
extern EjsObj *ejsRunInitializer(Ejs *ejs, struct EjsModule *module);

/** 
    Run a function
    @description Run a function with the given actual parameters
    @param ejs Ejs reference returned from #ejsCreate
    @param fn Function object to run
    @param thisObj Object to use as the "this" object when running the function.
    @param argc Count of actual parameters
    @param argv Vector of actual parameters
    @return The return value from the function. If an exception is thrown, NULL will be returned and ejs->exception
        will be set to the exception object.
    @ingroup EjsFunction
 */
extern EjsAny *ejsRunFunction(Ejs *ejs, EjsFunction *fn, EjsAny *thisObj, int argc, void *argv);

/** 
    Run a function by slot number
    @description Run a function identified by slot number with the given actual parameters. This will run the function
        stored at \a slotNum in the \a obj variable. 
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object that holds the function at its "slotNum" slot. Also use this object as the "this" object 
        when running the function.
    @param slotNum Slot number in \a obj that contains the function to run.
    @param argc Count of actual parameters
    @param argv Vector of actual parameters
    @return The return value from the function. If an exception is thrown, NULL will be returned and ejs->exception
        will be set to the exception object.
    @ingroup EjsFunction
 */
extern EjsAny *ejsRunFunctionBySlot(Ejs *ejs, EjsAny *obj, int slotNum, int argc, void *argv);
extern EjsAny *ejsRunFunctionByName(Ejs *ejs, EjsAny *container, EjsName qname, EjsAny *obj, int argc, void *argv);

extern EjsEx *ejsAddException(Ejs *ejs, EjsFunction *mp, uint tryStart, uint tryEnd, struct EjsType *catchType,
    uint handlerStart, uint handlerEnd, int numBlocks, int numStack, int flags, int preferredIndex);
extern EjsFunction *ejsCloneFunction(Ejs *ejs, EjsFunction *src, int deep);
extern int ejsDefineException(Ejs *ejs, struct EjsType *obj, int slot, uint tryOffset,
    uint tryLength, uint handlerOffset, uint handlerLength, int flags);
extern void ejsOffsetExceptions(EjsFunction *mp, int offset);
extern int ejsSetFunctionCode(Ejs *ejs, EjsFunction *fun, struct EjsModule *module, cuchar *byteCode, size_t len, 
    EjsDebug *debug);
extern EjsCode *ejsCreateCode(Ejs *ejs, EjsFunction *fun, struct EjsModule *module, cuchar *byteCode, size_t len, 
    EjsDebug *debug);
extern void ejsManageFunction(EjsFunction *fun, int flags);
extern void ejsShowOpFrequency(Ejs *ejs);

typedef struct EjsFrame {
    EjsFunction     function;               /**< Activation frame for function calls. Stores local variables */
    EjsFunction     *orig;                  /**< Original function frame is based on */
    struct EjsFrame *caller;                /**< Previous invoking frame */
    EjsObj          **stackBase;            /**< Start of stack in this function */
    EjsObj          **stackReturn;          /**< Top of stack to return to */
    uchar           *pc;                    /**< Program counter */
    uchar           *attentionPc;           /**< Restoration PC value after attention */
    //  MOB OPT pack
    int             ignoreAttention;        /**< Ignore attention commands */
    int             getter;                 /**< Frame is a getter */
    uint            argc;                   /**< Actual parameter count */
    int             slotNum;                /**< Slot in owner */
#if BLD_DEBUG
    EjsLoc          loc;
#endif
} EjsFrame;

#if DOXYGEN
    /** 
        Determine if a variable is a frame. Only used internally in the VM.
        @param obj Object to test
        @return True if the variable is a frame. 
        @ingroup EjsFrame
     */
    extern bool ejsIsFrame(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsFrame(ejs, obj) (ejsIsPot(ejs, obj) && ((EjsPot*) (obj))->isFrame)
#endif

extern EjsFrame *ejsCreateFrame(Ejs *ejs, EjsFunction *src, EjsObj *thisObj, int argc, EjsObj **argv);
extern EjsFrame *ejsCreateCompilerFrame(Ejs *ejs, EjsFunction *src);
extern EjsBlock *ejsPopBlock(Ejs *ejs);
extern EjsBlock *ejsPushBlock(Ejs *ejs, EjsBlock *block);

/** 
    Boolean class
    @description The Boolean class provides the base class for the boolean values "true" and "false".
        EjsBoolean is a primitive native type and extends EjsObj. It is still logically an Object, but implements
        Object properties and methods itself. Only two instances of the boolean class are ever created created
        these are referenced as ejs->trueValue and ejs->falseValue.
    @stability Evolving
    @defgroup EjsBoolean EjsBoolean
    @see EjsBoolean ejsCreateBoolean ejsIsBoolean ejsGetBoolean
 */
typedef struct EjsBoolean {
    struct EjsType  *type;              /**< Type of the object */
    bool            value;              /**< Boolean value */
} EjsBoolean;

/** 
    Create a boolean
    @description Create a boolean value. This will not actually create a new boolean instance as there can only ever
        be two boolean instances (true and false). Boolean properties are immutable in Ejscript and so this routine
        will simply return the appropriate pre-created true or false boolean value.
    @param ejs Ejs reference returned from #ejsCreate
    @param value Desired boolean value. Set to 1 for true and zero for false.
    @ingroup EjsBoolean
 */
extern EjsBoolean *ejsCreateBoolean(Ejs *ejs, int value);

/** 
    Cast a variable to a boolean 
    @description
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to cast
    @return A new boolean object
    @ingroup EjsBoolean
 */
extern EjsBoolean *ejsToBoolean(Ejs *ejs, EjsAny *obj);

#if DOXYGEN
    /** 
        Determine if a variable is a boolean
        @param obj Object to test
        @return True if the variable is a boolean
        @ingroup EjsBoolean
     */
    extern bool ejsIsBoolean(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsBoolean(ejs, obj) (obj && TYPE(obj)->id == ES_Boolean)
#endif

/** 
    Get the C boolean value from a boolean object
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Boolean variable to access
    @return True or false
    @ingroup EjsBoolean
 */
extern bool ejsGetBoolean(Ejs *ejs, EjsAny *obj);

/*
    Thse constants match Stream.READ, Stream.WRITE, Stream.BOTH
 */
#define EJS_STREAM_READ     0x1
#define EJS_STREAM_WRITE    0x2
#define EJS_STREAM_BOTH     0x3

/** 
    ByteArray class
    @description ByteArrays provide a growable, integer indexed, in-memory store for bytes. ByteArrays can be used as a 
    simple array type to store and encode data as bytes or they can be used as buffered Streams implementing the Stream 
    interface.
    \n\n
    When used as a simple byte array, the ByteArray class offers a low level set of methods to insert and 
    extract bytes. The index operator [] can be used to access individual bytes and the copyIn and copyOut methods 
    can be used to get and put blocks of data. In this mode, the read and write position properties are ignored. 
    Access to the byte array is from index zero up to the size defined by the length property. When constructed, 
    the ByteArray can be designated as growable, in which case the initial size will grow as required to accomodate 
    data and the length property will be updated accordingly.
    \n\n
    When used as a Stream, the byte array additional write methods to store data at the location specified by the 
    $writePosition property and read methods to read from the $readPosition property. The $available method 
    indicates how much data is available between the read and write position pointers. The $reset method can 
    reset the pointers to the start of the array.  When used with for/in, ByteArrays will iterate or 
    enumerate over the available data between the read and write pointers.
    \n\n
    If numeric values are read or written, they will be encoded according to the value of the endian property 
    which can be set to either LittleEndian or BigEndian. 
    \n\n
    In Stream mode ByteArrays can be configured to run in sync or async mode. Adding observers via the $addObserver
    method will put a stream into async mode. Events will then be issued for close, eof, read and write events.
    @stability Evolving
    @defgroup EjsByteArray EjsByteArray
    @see EjsByteArray ejsIsByteArray ejsCreateByteArray ejsSetByteArrayPositions ejsCopyToByteArray
 */
typedef struct EjsByteArray {
    struct EjsType  *type;              /**< Type of the object */
    EjsObj          *emitter;           /**< Event emitter for listeners */
    uchar           *value;             /**< Data bytes in the array */
    int             async;              /**< Async mode */
    int             endian;             /**< Endian encoding */
    int             length;             /**< Length property */
    int             growInc;            /**< Current read position */
    int             swap;               /**< I/O must swap bytes due to endian byte ordering */
    int             readPosition;       /**< Current read position */
    int             writePosition;      /**< Current write position */
    bool            growable;           /**< Aray is growable */
    EjsObj          *listeners;         /**< Event listeners in async mode */
} EjsByteArray;

#if DOXYGEN
    /** 
        Determine if a variable is a byte array
        @param obj Object to test
        @return True if the variable is a byte array
        @ingroup EjsByteArray
     */
    //  MOB -- convert all ejsIs to take ejs as arg
    extern bool ejsIsByteArray(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsByteArray(ejs, obj) (obj && TYPE(obj)->id == ES_ByteArray)
#endif

/** 
    Create a byte array
    @description Create a new byte array instance.
    @param ejs Ejs reference returned from #ejsCreate
    @param size Initial size of the byte array
    @return A new byte array instance
    @ingroup EjsByteArray
 */
extern EjsByteArray *ejsCreateByteArray(Ejs *ejs, int size);

/** 
    Set the I/O byte array positions
    @description Set the read and/or write positions into the byte array. ByteArrays implement the Stream interface
        and support sequential and random access reading and writing of data in the array. The byte array maintains
        read and write positions that are automatically updated as data is read or written from or to the array. 
    @param ejs Ejs reference returned from #ejsCreate
    @param ba Byte array object
    @param readPosition New read position to set
    @param writePosition New write position to set
    @ingroup EjsByteArray
 */
extern void ejsSetByteArrayPositions(Ejs *ejs, EjsByteArray *ba, int readPosition, int writePosition);

/** 
    Copy data into a byte array
    @description Copy data into a byte array at a specified \a offset. 
    @param ejs Ejs reference returned from #ejsCreate
    @param ba Byte array object
    @param offset Offset in the byte array to which to copy the data.
    @param data Pointer to the source data
    @param length Length of the data to copy
    @return Zero if successful, otherwise a negative MPR error code.
 */
extern int ejsCopyToByteArray(Ejs *ejs, EjsByteArray *ba, int offset, char *data, size_t length);

extern void ejsResetByteArray(EjsByteArray *ba);
extern int ejsGetByteArrayAvailable(EjsByteArray *ba);
extern int ejsGetByteArrayRoom(EjsByteArray *ba);
extern int ejsGrowByteArray(Ejs *ejs, EjsByteArray *ap, int size);

extern struct EjsNumber *ejsWriteToByteArray(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv);
extern bool ejsMakeRoomInByteArray(Ejs *ejs, EjsByteArray *ap, int require);

/** 
    Date class
    @description The Date class is a general purpose class for working with dates and times. 
        is a a primitive native type and extends EjsObj. It is still logically an Object, but implements Object 
        properties and methods itself. 
    @stability Evolving
    @defgroup EjsDate EjsDate
    @see EjsDate EjsIsDate ejsCreateDate
 */
typedef struct EjsDate {
    struct EjsType  *type;              /**< Type of the object */
    MprTime         value;              /**< Time in milliseconds since "1970/01/01 GMT" */
} EjsDate;

#if DOXYGEN
    /** 
        Determine if a variable is a Date
        @param obj Object to test
        @return True if the variable is a date
        @ingroup EjsDate
     */
    bool ejsIsDate(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsDate(ejs, obj) (obj && TYPE(obj)->id == ES_Date)
#endif

/** 
    Create a new date instance
    @param ejs Ejs reference returned from #ejsCreate
    @param value Date/time value to set the new date instance to
    @return An initialized date instance
    @ingroup EjsDate
 */
extern EjsDate *ejsCreateDate(Ejs *ejs, MprTime value);

/** 
    Error classes
    @description Base class for error exception objects. Exception objects are created by programs and by the system 
    as part of changing the normal flow of execution when some error condition occurs. 
    When an exception is created and acted upon ("thrown"), the system transfers the flow of control to a 
    pre-defined instruction stream (the handler or "catch" code). The handler may return processing to the 
    point at which the exception was thrown or not. It may re-throw the exception or pass control up the call stack.
    @stability Evolving.
    @defgroup EjsError EjsError ejsFormatStack ejsGetErrorMsg ejsHasException ejsThrowArgError ejsThrowAssertError
        ejsThrowArithmeticError ejsThrowInstructionError ejsThrowError ejsThrowInternalError ejsThrowIOError
        ejsThrowMemoryError ejsThrowOutOfBoundsError ejsThrowReferenceError ejsThrowResourceError ejsThrowStateError
        ejsThrowStopIteration ejsThrowSyntaxError ejsThrowTypeError
 */
typedef EjsPot EjsError;

#define ejsIsError(ejs, obj) (obj && ejsIsA(ejs, obj, ejs->errorType))

extern EjsError *ejsCreateError(Ejs *ejs, struct EjsType *type, EjsObj *message);
extern EjsArray *ejsCaptureStack(Ejs *ejs, int uplevels);

/* 
    DEPRECATED MOB
    Format the stack backtrace
    @description Return a string containing the current interpreter stack backtrace
    @param ejs Ejs reference returned from #ejsCreate
    @param error Error exception object to analyseo analyseo analyseo analyse
    @return A string containing the stack backtrace. The caller must free.
    @ingroup EjsError
extern char *ejsFormatStack(Ejs *ejs, EjsError *error);
 */

/** 
    Get the interpreter error message
    @description Return a string containing the current interpreter error message
    @param ejs Ejs reference returned from #ejsCreate
    @param withStack Set to 1 to include a stack backtrace in the error message
    @return A string containing the error message. The caller must not free.
    @ingroup EjsError
 */
extern cchar *ejsGetErrorMsg(Ejs *ejs, int withStack);

/** 
    Determine if an exception has been thrown
    @param ejs Ejs reference returned from #ejsCreate
    @return True if an exception has been thrown
    @ingroup EjsError
 */
extern bool ejsHasException(Ejs *ejs);

// TODO - DOC
extern EjsObj *ejsGetException(Ejs *ejs);

/** 
    Throw an argument exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowArgError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an assertion exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowAssertError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an math exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowArithmeticError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an instruction code exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowInstructionError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an general error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an internal error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowInternalError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an IO exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowIOError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an Memory depletion exception
    @param ejs Ejs reference returned from #ejsCreate
    @ingroup EjsError
 */
extern EjsError *ejsThrowMemoryError(Ejs *ejs);

/** 
    Throw an out of bounds exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowOutOfBoundsError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an reference exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowReferenceError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an resource exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowResourceError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an state exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowStateError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an stop iteration exception
    @param ejs Ejs reference returned from #ejsCreate
    @ingroup EjsError
 */
extern EjsObj *ejsThrowStopIteration(Ejs *ejs);

/** 
    Throw an syntax error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowSyntaxError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an type error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowTypeError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);


/** 
    File class
    @description The File class provides a foundation of I/O services to interact with physical files and directories.
    Each File object represents a single file or directory and provides methods for creating, opening, reading, writing 
    and deleting files, and for accessing and modifying information about the file.
    @stability Prototype
    @defgroup EjsFile EjsFile 
    @see EjsFile ejsCreateFile ejsIsFile
 */
typedef struct EjsFile {
    struct EjsType  *type;              /**< Type of the object */
    MprFile         *file;              /**< Open file handle */
    MprPath         info;               /**< Cached file info */
    char            *path;              /**< Filename path */
    char            *modeString;        /**< User supplied mode string */
    int             mode;               /**< Current open mode */
    int             perms;              /**< Posix permissions mask */
#if FUTURE
    cchar           *cygdrive;          /**< Cygwin drive directory (c:/cygdrive) */
    cchar           *newline;           /**< Newline delimiters */
    int             delimiter;          /**< Path delimiter ('/' or '\\') */
    int             hasDriveSpecs;      /**< Paths on this file system have a drive spec */
#endif
} EjsFile;

/** 
    Create a File object
    @description Create a file object associated with the given filename. The filename is not opened, just stored.
    @param ejs Ejs reference returned from #ejsCreate
    @param filename Filename to associate with the file object
    @return A new file object
    @ingroup EjsFile
 */
extern EjsFile *ejsCreateFile(Ejs *ejs, cchar *filename);

extern EjsFile *ejsCreateFileFromFd(Ejs *ejs, int fd, cchar *name, int mode);

#if DOXYGEN
    /** 
        Determine if a variable is a File
        @param ejs Ejs reference returned from #ejsCreate
        @param obj Object to test
        @return True if the variable is a File
        @ingroup File
     */
    extern bool ejsIsFile(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsFile(ejs, obj) (obj && TYPE(obj)->id == ES_File)
#endif


/**
    Path class
    @description The Path class provides file path name services.
    @stability Prototype
    @defgroup EjsPath EjsPath 
    @see EjsFile ejsCreatePath ejsIsPath
 */
typedef struct EjsPath {
    struct EjsType  *type;              /**< Type of the object */
    cchar           *value;             /**< Filename path */
    MprPath         info;               /**< Cached file info */
    MprList         *files;             /**< File list for enumeration */
#if FUTURE
    cchar           *cygdrive;          /**< Cygwin drive directory (c:/cygdrive) */
    cchar           *newline;           /**< Newline delimiters */
    int             delimiter;          /**< Path delimiter ('/' or '\\') */
    int             hasDriveSpecs;      /**< Paths on this file system have a drive spec */
#endif
} EjsPath;


/** 
    Create a Path object
    @description Create a file object associated with the given filename. The filename is not opened, just stored.
    @param ejs Ejs reference returned from #ejsCreate
    @param path Path file name
    @return A new Path object
    @ingroup EjsPath
 */
extern EjsPath *ejsCreatePath(Ejs *ejs, EjsString *path);
extern EjsPath *ejsCreatePathFromAsc(Ejs *ejs, cchar *path);

#if DOXYGEN
    /** 
        Determine if a variable is a Path
        @param ejs Ejs reference returned from #ejsCreate
        @param obj Object to test
        @return True if the variable is a Path
        @ingroup EjsPath
     */
    extern bool ejsIsPath(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsPath(ejs, obj) (obj && TYPE(obj)->id == ES_Path)
#endif

//  MOB DOC
extern EjsPath *ejsToPath(Ejs *ejs, EjsAny *obj);


/** 
    Uri class
    @description The Uri class provides file path name services.
    @stability Prototype
    @defgroup EjsUri EjsUri 
    @see EjsFile ejsCreateUri ejsIsUri
 */
typedef struct EjsUri {
    struct EjsType  *type;              /**< Type of the object */
    HttpUri         *uri;               /**< Decoded URI */
} EjsUri;


/** 
    Create a Uri object
    @description Create a URI object associated with the given URI string.
    @param ejs Ejs reference returned from #ejsCreate
    @param uri Uri string to parse
    @return A new Uri object
    @ingroup EjsUri
 */
extern EjsUri *ejsCreateUri(Ejs *ejs, EjsString *uri);
extern EjsUri *ejsCreateUriFromMulti(Ejs *ejs, cchar *uri);
extern EjsUri *ejsCreateUriFromParts(Ejs *ejs, cchar *scheme, cchar *host, int port, cchar *path, cchar *query, 
        cchar *reference, int complete);

#if DOXYGEN
    /** 
        Determine if a variable is a Uri
        @param ejs Ejs reference returned from #ejsCreate
        @param obj Object to test
        @return True if the variable is a Uri
        @ingroup EjsUri
     */
    extern bool ejsIsUri(Ejs *ejs, EjsAny *obj);
    extern cchar *ejsGetUri(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsUri(ejs, obj) (obj && TYPE(obj)->id == ES_Uri)
    extern cchar *ejsGetUri(Ejs *ejs, EjsAny *obj);
#endif

//  MOB DOC
extern EjsUri *ejsToUri(Ejs *ejs, EjsAny *obj);

/** 
    FileSystem class
    @description The FileSystem class provides file system services.
    @stability Prototype
    @defgroup EjsFileSystem EjsFileSystem 
    @see EjsFile ejsCreateFile ejsIsFile
 */
typedef struct EjsFileSystem {
    struct EjsType  *type;              /**< Type of the object */
    char            *path;              /**< Filename path */
    MprFileSystem   *fs;                /**< MPR file system object */
} EjsFileSystem;


/** 
    Create a FileSystem object
    @description Create a file system object associated with the given pathname.
    @param ejs Ejs reference returned from #ejsCreate
    @param path Path to describe the file system. Can be any path in the file system.
    @return A new file system object
    @ingroup EjsPath
 */
extern EjsFileSystem *ejsCreateFileSystem(Ejs *ejs, cchar *path);

#if DOXYGEN
    /**
        Determine if a variable is a Path
        @param obj Object to test
        @return True if the variable is a FileSystem
        @ingroup EjsFileSystem
     */
    extern bool ejsIsFileSystem(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsFileSystem(ejs, obj) (obj && TYPE(obj)->id == ES_FileSystem)
#endif


extern EjsObj *ejsCreateGlobal(Ejs *ejs);
extern void ejsFreezeGlobal(Ejs *ejs);


/** 
    Http Class
    @description
        Http objects represents a Hypertext Transfer Protocol version 1.1 client connection and are used 
        HTTP requests and capture responses. This class supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
    @stability Prototype
    @defgroup EjsHttp EjsHttp
    @see EjsHttp ejsCreateHttp ejsIsHttp
 */
typedef struct EjsHttp {
    struct EjsType  *type;                      /**< Type of the object */
    EjsObj          *emitter;                   /**< Event emitter */
    EjsByteArray    *data;                      /**< Buffered write data */
    EjsObj          *limits;                    /**< Limits object */
    EjsObj          *responseCache;             /**< Cached response (only used if response() is used) */
    HttpConn        *conn;                      /**< Http connection object */
    MprBuf          *requestContent;            /**< Request body data supplied */
    MprBuf          *responseContent;           /**< Response data */
    char            *uri;                       /**< Target uri */
    char            *method;                    /**< HTTP method */
    char            *keyFile;                   /**< SSL key file */
    char            *certFile;                  /**< SSL certificate file */
    int             closed;                     /**< Http is closed and "close" event has been issued */
    int             error;                      /**< Http errored and "error" event has been issued */
    int             readCount;                  /**< Count of body bytes read */
    int             requestContentCount;        /**< Count of bytes written from requestContent */
    int             writeCount;                 /**< Count of bytes written via write() */
} EjsHttp;


/** 
    Create a new Http object
    @param ejs Ejs reference returned from #ejsCreate
    @return a new Http object
    @ingroup EjsHttp
 */
extern EjsHttp *ejsCreateHttp(Ejs *ejs);

//  MOB - fix up this pattern do don't need both define and function
#if DOXYGEN
    extern bool ejsIsHttp(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsHttpType(ejs, obj) (obj && TYPE(obj)->id == ES_Http)
#endif
extern void ejsSetHttpLimits(Ejs *ejs, HttpLimits *limits, EjsObj *obj, int server);
extern void ejsGetHttpLimits(Ejs *ejs, EjsObj *obj, HttpLimits *limits, int server);

//  MOB - rename SetupHttpTrace
extern int ejsSetupTrace(Ejs *ejs, HttpTrace *trace, EjsObj *options);
void ejsLoadHttpService(Ejs *ejs);


/** 
    Iterator Class
    @description Iterator is a helper class to implement iterators in other native classes
    @stability Prototype
    @defgroup EjsIterator EjsIterator
    @see EjsIterator ejsCreateIterator
 */
typedef struct EjsIterator {
    struct EjsType  *type;              /**< Type of the object */
    EjsObj          *target;            /**< Object to be enumerated */
    EjsFun          nativeNext;         /**< Native next function */
    bool            deep;               /**< Iterator deep (recursively over all properties) */
    EjsArray        *namespaces;        /**< Namespaces to consider in iteration */
    int             index;              /**< Current index */
    EjsObj          *indexVar;          /**< Reference to current item */
} EjsIterator;

/** 
    Create an iterator object
    @description The EjsIterator object is a helper class for native types to implement iteration and enumeration.
    @param ejs Ejs reference returned from #ejsCreate
    @param target Target variable to iterate or enumerate 
    @param next Function to invoke to step to the next element
    @param deep Set to true to do a deep iteration/enumeration
    @param namespaces Reserved and not used. Supply NULL.
    @return A new EjsIterator object
    @ingroup EjsIterator
 */
extern EjsIterator *ejsCreateIterator(Ejs *ejs, EjsAny *target, EjsFun next, bool deep, EjsArray *namespaces);

/** 
    Namespace Class
    @description Namespaces are used to qualify names into discrete spaces.
    @stability Evolving
    @defgroup EjsNamespace EjsNamespace
    @see EjsNamespace ejsIsNamespace ejsCreateNamespace ejsLookupNamespace ejsDefineReservedNamespace 
        ejsCreateReservedNamespace ejsFormatReservedNamespace 
 */
typedef struct EjsNamespace {
    struct EjsType  *type;              /**< Type of the object */
    EjsString       *value;             /**< Textual name of the namespace */
} EjsNamespace;


/** 
    Create a namespace object
    @param ejs Ejs reference returned from #ejsCreate
    @param name Space name to use for the namespace
    @return A new namespace object
    @ingroup EjsNamespace
 */
extern EjsNamespace *ejsCreateNamespace(Ejs *ejs, EjsString *name);

#if DOXYGEN
    /**
        Determine if a variable is a namespace
        @param obj Object to test
        @return True if the variable is a namespace
        @ingroup EjsNamespace
     */
    extern bool ejsIsNamespace(Ejs *ejs, EjsAny *obj)
#else
    #define ejsIsNamespace(ejs, obj) (obj && TYPE(obj)->id == ES_Namespace)
#endif

extern EjsNamespace *ejsDefineReservedNamespace(Ejs *ejs, EjsBlock *block, EjsName *typeName, cchar *name);
extern EjsNamespace *ejsCreateReservedNamespace(Ejs *ejs, EjsName *typeName, EjsString *name);
extern EjsString *ejsFormatReservedNamespace(Ejs *ejs, EjsName *typeName, EjsString *spaceName);

/** 
    Null Class
    @description The Null class provides the base class for the singleton null instance. This instance is stored
        in ejs->nullValue.
    @stability Evolving
    @defgroup EjsNull EjsNull
    @see EjsNull ejsCreateIsNull
 */
typedef EjsAny EjsNull;

/** 
    Determine if a variable is a null
    @return True if a variable is a null
    @ingroup EjsNull
 */
#define ejsIsNull(ejs, obj) (obj && TYPE(obj)->id == ES_Null)
extern EjsNull *ejsCreateNull(Ejs *ejs);

/** 
    Number class
    @description The Number class provide the base class for all numeric values. 
        The primitive number storage data type may be set via the configure program to be either double, float, int
        or int64. 
    @stability Evolving
    @defgroup EjsNumber EjsNumber
    @see EjsNumber ejsToNumber ejsCreateNumber ejsIsNumber ejsGetNumber ejsGetInt ejsGetDouble ejsIsInfinite ejsIsNan
 */
typedef struct EjsNumber {
    struct EjsType  *type;              /**< Type of the object */
    MprNumber       value;              /**< Numeric value */
} EjsNumber;


/** 
    Create a number object
    @param ejs Ejs reference returned from #ejsCreate
    @param value Numeric value to initialize the number object
    @return A number object
    @ingroup EjsNumber
 */
extern EjsNumber *ejsCreateNumber(Ejs *ejs, MprNumber value);

/** 
    Cast a variable to a number
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to cast
    @return A number object
    @ingroup EjsNumber
 */
extern struct EjsNumber *ejsToNumber(Ejs *ejs, EjsAny *obj);

extern bool ejsIsInfinite(MprNumber f);
#if WIN
#define ejsIsNan(f) (_isnan(f))
#elif MACOSX
    #define ejsIsNan(f) isnan(f)
#elif VXWORKS
    #define ejsIsNan(f) isnan(f)
#else
    #define ejsIsNan(f) (f == FP_NAN)
#endif

/** 
    Reflect Class
    @description The Reflect class permits introspection into the type and attributes of objects and properties.
    @stability Evolving
    @defgroup EjsNamespace EjsNamespace
    @see EjsReflect
 */
typedef struct EjsReflect {
    struct EjsType  *type;              /**< Type of the object */
    EjsObj          *subject;           /**< Object under examination */
} EjsReflect;


extern EjsString *ejsGetTypeName(struct Ejs *ejs, EjsAny *obj);
extern EjsString *ejsGetTypeOf(struct Ejs *ejs, EjsAny *obj);

/** 
    RegExp Class
    @description The regular expression class provides string pattern matching and substitution.
    @stability Evolving
    @defgroup EjsRegExp EjsRegExp
    @see EjsRegExp ejsCreateRegExp ejsIsRegExp
 */
typedef struct EjsRegExp {
    struct EjsType  *type;              /**< Type of the object */
    MprChar         *pattern;           /**< Pattern to match */
    void            *compiled;          /**< Compiled pattern */
    bool            global;             /**< Search for pattern globally (multiple times) */
    bool            ignoreCase;         /**< Do case insensitive matching */
    bool            multiline;          /**< Match patterns over multiple lines */
    bool            sticky;
    int             options;            /**< Pattern matching options */
    int             endLastMatch;       /**< End of the last match (one past end) */
    int             startLastMatch;     /**< Start of the last match */
    EjsString       *matched;           /**< Last matched component */
} EjsRegExp;


/** 
    Create a new regular expression object
    @param ejs Ejs reference returned from #ejsCreate
    @param pattern Regular expression pattern string
    @return a EjsRegExp object
    @ingroup EjsRegExp
 */
extern EjsRegExp *ejsCreateRegExp(Ejs *ejs, EjsString *pattern);

EjsString *ejsRegExpToString(Ejs *ejs, EjsRegExp *rp);

#if DOXYGEN
    /** 
        Determine if the variable is a regular expression
        @return True if the variable is a regular expression
        @ingroup EjsRegExp
     */
    extern bool ejsIsRegExp(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsRegExp(ejs, obj) (obj && TYPE(obj)->id == ES_RegExp)
#endif

/**
    Socket Class
    @description
    @stability Prototype
    @defgroup EjsSocket EjsSocket
    @see EjsSocket ejsCreateSocket ejsIsSocket
 */
typedef struct EjsSocket {
    struct EjsType  *type;              /**< Type of the object */
    EjsObj          *emitter;           /**< Event emitter */
    EjsByteArray    *data;              /**< Buffered write data */
    MprSocket       *sock;              /**< Underlying MPR socket object */
    MprWaitHandler  waitHandler;        /**< I/O event wait handler */
    cchar           *address;           /**< Remote address */
    int             port;               /**< Remote port */
    int             async;              /**< In async mode */
    int             mask;               /**< IO event mask */
    MprMutex        *mutex;             /**< Multithread sync */
} EjsSocket;

/** 
    Create a new Socket object
    @param ejs Ejs reference returned from #ejsCreate
    @return a new Socket object
    @ingroup EjsSocket
 */
extern EjsSocket *ejsCreateSocket(Ejs *ejs);

#if DOXYGEN
    extern bool ejsIsSocket(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsSocket(ejs, obj) (obj && TYPE(obj)->id == ES_Socket)
#endif


/** 
    Timer Class
    @description Timers manage the scheduling and execution of Ejscript functions. Timers run repeatedly 
        until stopped by calling the stop method and are scheduled with a granularity of 1 millisecond. 
    @stability Evolving
    @defgroup EjsTimer EjsTimer
    @see EjsTimer
 */
typedef struct EjsTimer {
    struct EjsType  *type;              /**< Type of the object */
    MprEvent        *event;             /**< MPR event for the timer */
    int             drift;              /**< Timer event is allowed to drift if system conditions requrie */
    int             repeat;             /**< Timer repeatedly fires */
    int             period;             /**< Time in msec between invocations */          
    EjsFunction     *callback;          /**< Callback function */
    EjsFunction     *onerror;           /**< onerror function */
    EjsArray        *args;              /**< Callback args */
} EjsTimer;


#define EJS_WORKER_BEGIN        1                   /**< Worker yet to start */
#define EJS_WORKER_STARTED      2                   /**< Worker has started a script */
#define EJS_WORKER_CLOSED       3                   /**< Inside worker has finished */
#define EJS_WORKER_COMPLETE     4                   /**< Worker has completed all messages */

/** 
    Worker Class
    @description The Worker class provides the ability to create new interpreters in dedicated threads
    @stability Prototype
    @defgroup EjsWorker EjsWorker
    @see EjsObj
 */
typedef struct EjsWorker {
    EjsPot          pot;                /**< Property storage */
    char            *name;              /**< Optional worker name */
    Ejs             *ejs;               /**< Interpreter */
    EjsAny          *event;             /**< Current event object */
    struct EjsWorker *pair;             /**< Corresponding worker object in other thread */
    char            *scriptFile;        /**< Script or module to run */
    EjsString       *scriptLiteral;     /**< Literal script string to run */
    int             state;              /**< Worker state */
    int             inside;             /**< Running inside the worker */
    int             complete;           /**< Worker has completed its work */
    int             gotMessage;         /**< Worker has received a message */
    int             terminated;         /**< Worker has had terminate() called */
} EjsWorker;

extern EjsWorker *ejsCreateWorker(Ejs *ejs);

/** 
    Void class
    @description The Void class provides the base class for the singleton "undefined" instance. This instance is stored
        in ejs->undefinedValue..
    @stability Evolving
    @defgroup EjsVoid EjsVoid
    @see EjsVoid
 */

typedef EjsAny EjsVoid;

extern EjsVoid  *ejsCreateUndefined(Ejs *ejs);

#define ejsIsUndefined(ejs, obj) (obj && TYPE(obj)->id == ES_Void)

/*  
    Xml tag state
 */
typedef struct EjsXmlTagState {
    struct EjsXML   *obj;
    //  TODO these two should be XML also
    EjsObj          *attributes;
    EjsObj          *comments;
} EjsXmlTagState;


/*  
    Xml Parser state
 */
typedef struct EjsXmlState {
    //  MOB -- should not be fixed but should be growable
    EjsXmlTagState  nodeStack[EJS_XML_MAX_NODE_DEPTH];
    Ejs             *ejs;
    struct EjsType  *xmlType;
    struct EjsType  *xmlListType;
    int             topOfStack;
    long            inputSize;
    long            inputPos;
//  XX
    cchar           *inputBuf;
    cchar           *filename;
} EjsXmlState;


/** 
    XML class
    @description The XML class and API is based on ECMA-357 -- ECMAScript for XML (E4X). The XML class is a 
    core class in the E4X specification; it provides the ability to load, parse and save XML documents.
    @stability Evolving
    @defgroup EjsXML EjsXML
    @see EjsXML ejsIsXML ejsConfigureXML ejsCreateXML ejsLoadXMLString ejsDeepCopyXML ejsXMLDescendants
 */
typedef struct EjsXML {
    struct EjsType  *type;
    EjsName         qname;              /**< XML node name (e.g. tagName) */
    int             kind;               /**< Kind of XML node */
    MprList         *elements;          /**< List elements or child nodes */
    MprList         *attributes;        /**< Node attributes */
    MprList         *namespaces;        /**< List of namespaces as Namespace objects */
    struct EjsXML   *parent;            /**< Parent node reference (XML or XMLList) */
    struct EjsXML   *targetObject;      /**< XML/XMLList object modified when items inserted into an empty list */
    EjsName         targetProperty;     /**< XML property modified when items inserted into an empty list */
    EjsString       *value;             /**< Value of text|attribute|comment|pi */
    int             flags;
} EjsXML;


#if DOXYGEN
    /** 
        Determine if a variable is an XML object
        @param Object to test
        @return true if the variable is an XML or XMLList object
        @ingroup EjsXML
     */
    extern boolean ejsIsXML(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsXML(ejs, obj) (obj && ((TYPE(obj)->id == ES_XML) || TYPE(obj)->id == ES_XMLList))
#endif

extern EjsXML *ejsCreateXML(Ejs *ejs, int kind, EjsName name, EjsXML *parent, EjsString *value);
extern void  ejsLoadXMLString(Ejs *ejs, EjsXML *xml, EjsString *xmlString);
extern void  ejsLoadXMLFromMulti(Ejs *ejs, EjsXML *xml, cchar *xmlString);
extern EjsXML *ejsConfigureXML(Ejs *ejs, EjsXML *xml, int kind, EjsString *name, EjsXML *parent, EjsString *value);
extern EjsXML *ejsDeepCopyXML(Ejs *ejs, EjsXML *xml);
extern EjsXML *ejsXMLDescendants(Ejs *ejs, EjsXML *xml, EjsName qname);

/*  
    Xml private prototypes
 */
extern void ejsManageXML(EjsXML *xml, int flags);
extern MprXml *ejsCreateXmlParser(Ejs *ejs, EjsXML *xml, cchar *filename);
extern int ejsXMLToString(Ejs *ejs, MprBuf *buf, EjsXML *xml, int indentLevel);
extern EjsXML *ejsAppendToXML(Ejs *ejs, EjsXML *xml, EjsXML *node);
extern EjsXML *ejsSetXML(Ejs *ejs, EjsXML *xml, int index, EjsXML *node);
extern int ejsAppendAttributeToXML(Ejs *ejs, EjsXML *parent, EjsXML *node);
extern EjsXML *ejsCreateXMLList(Ejs *ejs, EjsXML *targetObject, EjsName targetProperty);


extern int ejsAddObserver(Ejs *ejs, EjsObj **emitterPtr, EjsObj *name, EjsObj *listener);
extern int ejsRemoveObserver(Ejs *ejs, EjsObj *emitter, EjsObj *name, EjsObj *listener);
extern int ejsSendEventv(Ejs *ejs, EjsObj *emitter, cchar *name, EjsAny *thisObj, int argc, void *argv);
extern int ejsSendEvent(Ejs *ejs, EjsObj *emitter, cchar *name, EjsAny *thisObj, EjsAny *arg);

/************************************ Accessors **************************************/

/** 
    Determine if a variable is a number
    @param obj Object to examine
    @return True if the variable is a number
    @ingroup EjsNumber
 */
#if DOXYGEN
    bool ejsIsNumber(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsNumber(ejs, obj) (obj && TYPE(obj)->id == ES_Number)
#endif

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to examine
    @return A numeric value
    @ingroup EjsNumber
 */
extern MprNumber ejsGetNumber(Ejs *ejs, EjsAny *obj);

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to examine
    @return An integer value
    @ingroup EjsNumber
 */
extern int ejsGetInt(Ejs *ejs, EjsAny *obj);

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to examine
    @return A double value
    @ingroup EjsNumber
 */
extern double ejsGetDouble(Ejs *ejs, EjsAny *obj);

//  MOB -- rename alloc/free
typedef EjsAny  *(*EjsCreateHelper)(Ejs *ejs, struct EjsType *type, int size);
typedef EjsAny  *(*EjsCastHelper)(Ejs *ejs, EjsAny *obj, struct EjsType *type);
typedef EjsAny  *(*EjsCloneHelper)(Ejs *ejs, EjsAny *obj, bool deep);
typedef int     (*EjsDefinePropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname, struct EjsType *propType, 
                    int64 attributes, EjsAny *value);
typedef int     (*EjsDeletePropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef int     (*EjsDeletePropertyByNameHelper)(Ejs *ejs, EjsAny *obj, EjsName qname);
typedef EjsAny  *(*EjsGetPropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef EjsAny  *(*EjsGetPropertyByNameHelper)(Ejs *ejs, EjsAny *obj, EjsName qname);
typedef struct EjsTrait *(*EjsGetPropertyTraitsHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef int     (*EjsGetPropertyCountHelper)(Ejs *ejs, EjsAny *obj);
typedef EjsName (*EjsGetPropertyNameHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef EjsAny  *(*EjsInvokeOperatorHelper)(Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);
typedef int     (*EjsLookupPropertyHelper)(Ejs *ejs, EjsAny *obj, EjsName qname);
typedef int     (*EjsSetPropertyByNameHelper)(Ejs *ejs, EjsAny *obj, EjsName qname, EjsAny *value);
typedef int     (*EjsSetPropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum, EjsAny *value);
typedef int     (*EjsSetPropertyNameHelper)(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname);
typedef int     (*EjsSetPropertyTraitsHelper)(Ejs *ejs, EjsAny *obj, int slotNum, struct EjsType *type, int attributes);

/** 
    Type class
    @description Classes in Ejscript are represented by instances of an EjsType. 
        Types are templates for creating instances of the given type, but they are also are runtime accessible objects.
        Types contain the static properties and methods for objects and store these in their object slots array. 
        They store the instance properties in the type->instance object. EjsType inherits from EjsBlock, EjsObj 
        and EjsObj. 
    @stability Evolving
    @defgroup EjsType EjsType
    @see EjsType ejsIsType ejsIsProperty ejsCreateType ejsDefineFunction ejsIsA ejsIsTypeSubType 
        ejsBindMethod ejsDefineInstanceProperty ejsGetType
 */
typedef struct EjsType {
    EjsFunction     constructor;                    /**< Constructor function and type properties */
    EjsName         qname;                          /**< Qualified name of the type. Type name and namespace */
    EjsPot          *prototype;                     /**< Prototype for instances when using prototype inheritance (only) */
    EjsHelpers      helpers;                        /**< Type helper methods */
    struct EjsType  *baseType;                      /**< Base class */
    MprManager      manager;                        /**< Manager callback */
    struct Ejs      *ejs;                           /**< Interpreter reference */

    MprList         *implements;                    /**< List of implemented interfaces */
        
    uint            callsSuper           : 1;       /**< Constructor calls super() */
    uint            dynamicInstance      : 1;       /**< Object instances may add properties */
    uint            final                : 1;       /**< Type is final */
    uint            hasBaseConstructors  : 1;       /**< Base types has constructors */
    uint            hasBaseInitializers  : 1;       /**< Base types have initializers */
    uint            hasConstructor       : 1;       /**< Type has a constructor */
    uint            hasInitializer       : 1;       /**< Type has static level initialization code */
    uint            hasInstanceVars      : 1;       /**< Type has non-function instance vars (state) */
    uint            hasMeta              : 1;       /**< Type has meta methods */
    uint            hasScriptFunctions   : 1;       /**< Block has non-native functions requiring namespaces */
    uint            immutable            : 1;       /**< Instances are immutable */
    uint            initialized          : 1;       /**< Static initializer has run */
    uint            isInterface          : 1;       /**< Interface vs class */
    uint            isPot                : 1;       /**< Instances are based on EjsPot */
    uint            needFixup            : 1;       /**< Slots need fixup */
    uint            numericIndicies      : 1;       /**< Instances support direct numeric indicies */
    uint            virtualSlots         : 1;       /**< Properties are not stored in slots[] */
    
    //  MOB -- pack with above?
    ushort          numInherited;                   /**< Number of inherited prototype properties */
    ushort          instanceSize;                   /**< Size of instances in bytes */
    ushort          id;                             /**< Unique type id */
    struct EjsModule *module;                       /**< Module owning the type - stores the constant pool */
    void            *typeData;                      /**< Type specific data */
} EjsType;


#if DOXYGEN
    /** 
        Determine if a variable is an type
        @param obj Object to test
        @return True if the variable is a type
        @ingroup EjsType
     */
    extern bool ejsIsType(Ejs *ejs, EjsAny *obj);

    /** 
        Determine if a variable is a prototype object. Types store the template for instance properties in a prototype object
        @param obj Object to test
        @return True if the variable is a prototype object.
        @ingroup EjsType
     */
    extern bool ejsIsPrototype(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsType(ejs, obj)       (obj && ejsIsPot(ejs, obj) && (((EjsPot*) (obj))->isType))
    #define ejsIsPrototype(ejs, obj)  (obj && ejsIsPot(ejs, obj) && (((EjsPot*) (obj))->isPrototype))
#endif

/** 
    Create a new type object
    @description Create a new type object 
    @param ejs Ejs reference returned from #ejsCreate
    @param name Qualified name to give the type. This name is merely referenced by the type and must be persistent.
        This name is not used to define the type as a global property.
    @param up Reference to a module that will own the type. Set to null if not owned by any module.
    @param baseType Base type for this type.
    @param prototype Prototype object instance properties of this type.
    @param size Size of instances. This is the size in bytes of an instance object.
    @param slotNum Slot number that the type will be installed at. This is used by core types to define a unique type ID. 
        For non-core types, set to -1.
    @param numTypeProp Number of type (class) properties for the type. These include static properties and methods.
    @param numInstanceProp Number of instance properties.
    @param attributes Attribute mask to modify how the type is initialized.
    @param data
    @ingroup EjsType EjsType
 */
extern EjsType *ejsCreateType(Ejs *ejs, EjsName name, struct EjsModule *up, EjsType *baseType, EjsPot *prototype,
    int size, int typeId, int numTypeProp, int numInstanceProp, int64 attributes);
extern EjsType *ejsConfigureType(Ejs *ejs, EjsType *type, struct EjsModule *up, EjsType *baseType, 
    int numTypeProp, int numInstanceProp, int64 attributes);

#define EJS_OBJ_HELPERS 1
#define EJS_POT_HELPERS  2

extern EjsType  *ejsCreateNativeType(Ejs *ejs, EjsName qname, int id, int size, void *manager, int helpers);
extern EjsType  *ejsConfigureNativeType(Ejs *ejs, EjsName qname, int size, void *manager, int helpers);

extern EjsObj *ejsCreatePrototype(Ejs *ejs, EjsType *type, int numProp);
extern EjsType *ejsCreateArchetype(Ejs *ejs, struct EjsFunction *fun, EjsPot *prototype);

/** 
    Define a global function
    @description Define a global public function and bind it to the C native function. This is a simple one liner
        to define a public global function. The more typical paradigm to define functions is to create a script file
        of native method definitions and and compile it. This results in a mod file that can be loaded which will
        create the function/method definitions. Then use #ejsBindMethod to associate a C function with a property.
    @ingroup EjsType
 */
//  XX
extern int ejsDefineGlobalFunction(Ejs *ejs, EjsString *name, EjsFun fn);


/** 
    Test if an variable is an instance of a given type
    @description Perform an "is a" test. This tests if a variable is a direct instance or subclass of a given base type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param target Target variable to test.
    @param type Type to compare with the target
    @return True if target is an instance of "type" or an instance of a subclass of "type".
    @ingroup EjsType
 */
extern bool ejsIsA(Ejs *ejs, EjsAny *target, EjsType *type);

/** 
    Test if a type is a derived type of a given base type.
    @description Test if a type subclasses a base type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param target Target type to test.
    @param baseType Base class to see if the target subclasses it.
    @return True if target is a "baseType" or a subclass of "baseType".
    @ingroup EjsType
 */
extern bool ejsIsTypeSubType(Ejs *ejs, EjsType *target, EjsType *baseType);

/** 
    Bind a native C function to a method property
    @description Bind a native C function to an existing javascript method. Method functions are typically created
        by compiling a script file of native method definitions into a mod file. When loaded, this mod file will create
        the method properties. This routine will then bind the specified C function to the method property.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Type containing the function property to bind.
    @param slotNum Slot number of the method property
    @param fn Native C function to bind
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
extern int ejsBindMethod(Ejs *ejs, EjsAny *obj, int slotNum, void *fn);
extern int ejsBindAccess(Ejs *ejs, EjsAny *obj, int slotNum, void *getter, void *setter);
extern void ejsBindConstructor(Ejs *ejs, EjsType *type, void *nativeProc);

/** 
    Define an instance property
    @description Define an instance property on a type. This routine should not normally be called manually. Instance
        properties are best created by creating a script file of native property definitions and then loading the resultant
        mod file.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type in which to create the instance property
    @param slotNum Instance slot number in the type that will hold the property. Set to -1 to allocate the next available
        free slot.
    @param name Qualified name for the property including namespace and name.
    @param propType Type of the instance property.
    @param attributes Integer mask of access attributes.
    @param value Initial value of the instance property.
    @return The slot number used for the property.
    @ingroup EjsType
 */
extern int ejsDefineInstanceProperty(Ejs *ejs, EjsType *type, int slotNum, EjsName name, EjsType *propType, 
    int attributes, EjsAny *value);

/** 
    Get a type
    @description Get the type installed at the given slot number. All core-types are installed a specific global slots.
        When Ejscript is built, these slots are converted into C program defines of the form: ES_TYPE where TYPE is the 
        name of the type concerned. For example, you can get the String type object via:
        @pre
        ejsGetType(ejs, ES_String)
    @param ejs Interpreter instance returned from #ejsCreate
    @param slotNum Slot number of the type to retrieve. Use ES_TYPE defines. 
    @return A type object if successful or zero if the type could not be found
    @ingroup EjsType
 */
extern EjsType  *ejsGetType(Ejs *ejs, int slotNum);

extern EjsType  *ejsGetTypeByName(Ejs *ejs, EjsName qname);

#define VSPACE(space) space "-" BLD_VNUM
#define ejsGetVType(ejs, space, name) ejsGetTypeByName(ejs, space "-" BLD_VNUM, name)

extern int      ejsCompactClass(Ejs *ejs, EjsType *type);
extern int      ejsCopyBaseProperties(Ejs *ejs, EjsType *type, EjsType *baseType);
extern void     ejsDefineTypeNamespaces(Ejs *ejs, EjsType *type);
extern int      ejsFixupType(Ejs *ejs, EjsType *type, EjsType *baseType, int makeRoom);
extern void     ejsInitializeBlockHelpers(EjsHelpers *helpers);

extern void     ejsSetTypeName(Ejs *ejs, EjsType *type, EjsName qname);
extern void     ejsTypeNeedsFixup(Ejs *ejs, EjsType *type);
extern int      ejsGetTypeSize(Ejs *ejs, EjsType *type);
extern EjsPot   *ejsGetPrototype(Ejs *ejs, EjsAny *obj);

/******************************** Private Prototypes **********************************/

extern int      ejsBootstrapTypes(Ejs *ejs);
extern void     ejsCreateArrayType(Ejs *ejs);
extern void     ejsCreateBlockType(Ejs *ejs);
extern void     ejsCreateBooleanType(Ejs *ejs);
extern void     ejsCreateConfigType(Ejs *ejs);
extern void     ejsCreateErrorType(Ejs *ejs);
extern void     ejsCreateFrameType(Ejs *ejs);
extern void     ejsCreateFunctionType(Ejs *ejs);
extern void     ejsCreateGlobalBlock(Ejs *ejs);
extern void     ejsCreateIteratorType(Ejs *ejs);
extern void     ejsCreateNamespaceType(Ejs *ejs);
extern void     ejsCreateNullType(Ejs *ejs);
extern void     ejsCreateNumberType(Ejs *ejs);
extern void     ejsCreateObjectType(Ejs *ejs);
extern void     ejsCreateRegExpType(Ejs *ejs);
extern void     ejsCreateTypeType(Ejs *ejs);
extern void     ejsCreateVoidType(Ejs *ejs);
extern void     ejsCreateXMLType(Ejs *ejs);
extern void     ejsCreateXMLListType(Ejs *ejs);

extern void     ejsInitStringType(Ejs *ejs, EjsType *type);

/*  
    Native type configuration
 */
extern void     ejsConfigureAppType(Ejs *ejs);
extern void     ejsConfigureArrayType(Ejs *ejs);
extern void     ejsConfigureBooleanType(Ejs *ejs);
extern void     ejsConfigureByteArrayType(Ejs *ejs);
extern void     ejsConfigureDateType(Ejs *ejs);
extern void     ejsConfigureSqliteTypes(Ejs *ejs);
extern void     ejsConfigureDebugType(Ejs *ejs);
extern void     ejsConfigureErrorType(Ejs *ejs);
extern void     ejsConfigureEventType(Ejs *ejs);
extern void     ejsConfigureGCType(Ejs *ejs);
extern void     ejsConfigureGlobalBlock(Ejs *ejs);
extern void     ejsConfigureFileType(Ejs *ejs);
extern void     ejsConfigureFileSystemType(Ejs *ejs);
extern void     ejsConfigureFunctionType(Ejs *ejs);
extern void     ejsConfigureHttpType(Ejs *ejs);
extern void     ejsConfigureIteratorType(Ejs *ejs);
extern void     ejsConfigureJSONType(Ejs *ejs);
extern void     ejsConfigureLoggerType(Ejs *ejs);
extern void     ejsConfigureNamespaceType(Ejs *ejs);
extern void     ejsConfigureMemoryType(Ejs *ejs);
extern void     ejsConfigureMathType(Ejs *ejs);
extern void     ejsConfigureNumberType(Ejs *ejs);
extern void     ejsConfigureNullType(Ejs *ejs);
extern void     ejsConfigureObjectType(Ejs *ejs);
extern void     ejsConfigurePathType(Ejs *ejs);
extern void     ejsConfigureReflectType(Ejs *ejs);
extern void     ejsConfigureRegExpType(Ejs *ejs);
extern void     ejsConfigureStringType(Ejs *ejs);
extern void     ejsConfigureSocketType(Ejs *ejs);
extern void     ejsConfigureSystemType(Ejs *ejs);
extern void     ejsConfigureTimerType(Ejs *ejs);
extern void     ejsConfigureTypes(Ejs *ejs);
extern void     ejsConfigureUriType(Ejs *ejs);
extern void     ejsConfigureVoidType(Ejs *ejs);
extern void     ejsConfigureWorkerType(Ejs *ejs);
extern void     ejsConfigureXMLType(Ejs *ejs);
extern void     ejsConfigureXMLListType(Ejs *ejs);

extern void     ejsCreateCoreNamespaces(Ejs *ejs);
extern int      ejsCopyCoreTypes(Ejs *ejs);
extern int      ejsDefineCoreTypes(Ejs *ejs);
extern int      ejsDefineErrorTypes(Ejs *ejs);
extern void     ejsInheritBaseClassNamespaces(Ejs *ejs, EjsType *type, EjsType *baseType);
extern void     ejsServiceEvents(Ejs *ejs, int timeout, int flags);
extern void     ejsSetSqliteMemCtx(MprThreadLocal *tls);
extern void     ejsSetSqliteTls(MprThreadLocal *tls);

#if BLD_FEATURE_EJS_ALL_IN_ONE || BLD_FEATURE_STATIC
extern int      ejs_events_Init(Ejs *ejs);
extern int      ejs_xml_Init(Ejs *ejs);
extern int      ejs_io_Init(Ejs *ejs);
extern int      ejs_sys_Init(Ejs *ejs);
#if BLD_FEATURE_SQLITE
extern int      ejs_db_sqlite_Init(Ejs *ejs);
#endif
extern int      ejs_web_Init(Ejs *ejs);
#endif

/* 
    Move some ejsWeb.h declarations here so handlers can just include ejs.h whether they are using the
    all-in-one ejs.h or the pure ejs.h
 */
extern HttpStage *ejsAddWebHandler(Http *http);
extern int ejsHostHttpServer(HttpConn *conn);

extern int ejs_db_sqlite_Init(Ejs *ejs);
extern int ejs_web_init(Ejs *ejs);

/**
    VM Evaluation state. 
    The VM Stacks grow forward in memory. A push is done by incrementing first, then storing. ie. *++top = value
    A pop is done by extraction then decrement. ie. value = *top--
    @ingroup EjsVm
 */
typedef struct EjsState {
    struct EjsFrame     *fp;                /* Current Frame function pointer */
    struct EjsBlock     *bp;                /* Current block pointer */
    EjsObj              **stack;            /* Top of stack (points to the last element pushed) */
    EjsObj              **stackBase;        /* Pointer to start of stack mem */
    struct EjsState     *prev;              /* Previous state */
    struct EjsNamespace *internal;          /* Current internal namespace */
    int                 stackSize;          /* Stack size */
} EjsState;


/**
    Lookup State.
    @description Location information returned when looking up properties.
    @ingroup EjsVm
 */
typedef struct EjsLookup {
    int             slotNum;                /* Final slot in obj containing the variable reference */
    uint            nthBase;                /* Property on Nth super type -- count from the object */
    uint            nthBlock;               /* Property on Nth block in the scope chain -- count from the end */
    EjsType         *type;                  /* Type containing property (if on a prototype obj) */
    //  MOB -- check all these being used
    uint            instanceProperty;       /* Property is an instance property */
    //  MOB -- check all these being used
    uint            ownerIsType;            /* Original object owning the property is a type */
    uint            useThis;                /* Property accessible via "this." */
    EjsAny          *obj;                   /* Final object / Type containing the variable */
    EjsAny          *originalObj;           /* Original object used for the search */
    EjsAny          *ref;                   /* Actual property reference */
    struct EjsTrait *trait;                 /* Property trait describing the property */
    struct EjsName  name;                   /* Name and namespace used to find the property */
    int             bind;                   /* Whether to bind to this lookup */
} EjsLookup;


/**
    Ejscript Service structure
    @description The Ejscript service manages the overall language runtime. It 
        is the factory that creates interpreter instances via #ejsCreate.
    @ingroup EjsService
 */
typedef struct EjsService {
    EjsObj          *(*loadScriptLiteral)(Ejs *ejs, EjsString *script, cchar *cache);
    EjsObj          *(*loadScriptFile)(Ejs *ejs, cchar *path, cchar *cache);
    MprList         *vmlist;
    MprHashTable    *nativeModules;
    Http            *http;
    MprMutex        *mutex;             /**< Multithread locking */
} EjsService;

extern EjsService *ejsGetService();
extern int ejsInitCompiler(EjsService *service);
extern void ejsAttention(Ejs *ejs);
extern void ejsClearAttention(Ejs *ejs);

/*********************************** Prototypes *******************************/
/**
    Open the Ejscript service
    @description One Ejscript service object is required per application. From this service, interpreters
        can be created.
    @return An ejs service object
    @ingroup Ejs
 */
extern EjsService *ejsCreateService();

/**
    Create an ejs virtual machine 
    @description Create a virtual machine interpreter object to evalute Ejscript programs. Ejscript supports multiple 
        interpreters. One interpreter can be designated as a master interpreter and then it can be cloned by supplying 
        the master interpreter to this call. A master interpreter provides the standard system types and clone interpreters 
        can quickly be created an utilize the master interpreter's types. This saves memory and speeds initialization.
    @param search Module search path to use. Set to NULL for the default search path.
    @param require Optional list of required modules to load. If NULL, the following modules will be loaded:
        ejs, ejs.io, ejs.events, ejs.xml, ejs.sys and ejs.unix.
    @param argc Count of command line argumements in argv
    @param argv Command line arguments
    @param flags Optional flags to modify the interpreter behavior. Valid flags are:
        @li    EJS_FLAG_COMPILER       - Interpreter will compile code from source
        @li    EJS_FLAG_NO_EXE         - Don't execute any code. Just compile.
        @li    EJS_FLAG_DOC            - Load documentation from modules
        @li    EJS_FLAG_NOEXIT         - App should service events and not exit unless explicitly instructed
    @return A new interpreter
    @ingroup Ejs
 */
extern Ejs *ejsCreateVm(cchar *search, MprList *require, int argc, cchar **argv, int flags);

/**
    Create a search path array. This can be used in ejsCreateVm.
    @description Create and array of search paths.
    @param ejs Ejs interpreter
    @param searchPath Search path string. This is a colon (or semicolon on Windows) separated string of directories.
    @return An array of search paths
    @ingroup Ejs
 */
struct EjsArray *ejsCreateSearchPath(Ejs *ejs, cchar *searchPath);

/**
    Set the module search path
    @description Set the ejs module search path. The search path is by default set to the value of the EJSPATH
        environment directory. Ejsript will search for modules by name. The search strategy is:
        Given a name "a.b.c", scan for:
        @li File named a.b.c
        @li File named a/b/c
        @li File named a.b.c in EJSPATH
        @li File named a/b/c in EJSPATH
        @li File named c in EJSPATH

    Ejs will search for files with no extension and also search for modules with a ".mod" extension. If there is
    a shared library of the same name with a shared library extension (.so, .dll, .dylib) and the module requires 
    native code, then the shared library will also be loaded.
    @param ejs Ejs interpreter
    @param search Array of search paths
    @ingroup Ejs
 */
extern void ejsSetSearchPath(Ejs *ejs, struct EjsArray *search);
extern void ejsInitSearchPath(Ejs *ejs);

/**
    Evaluate a file
    @description Evaluate a file containing an Ejscript. This requires linking with the Ejscript compiler library (libec). 
    @param path Filename of the script to evaluate
    @return Return zero on success. Otherwise return a negative Mpr error code.
    @ingroup Ejs
 */
extern int ejsEvalFile(cchar *path);

/*
    Flags for LoadScript and compiling
 */
#define EC_FLAGS_BIND            0x1                    /* Bind global references and type/object properties */
#define EC_FLAGS_DEBUG           0x2                    /* Generate symbolic debugging information */
#define EC_FLAGS_MERGE           0x8                    /* Merge all output onto one output file */
#define EC_FLAGS_NO_OUT          0x10                   /* Don't generate any output file */
#define EC_FLAGS_PARSE_ONLY      0x20                   /* Just parse source. Don't generate code */
#define EC_FLAGS_THROW           0x40                   /* Throw errors when compiling. Used for eval() */
#define EC_FLAGS_VISIBLE         0x80                   /* Make global vars visible to all */
#define EC_FLAGS_DOC             0x100                  /* Parse inline doc */

//  TODO - DOC
extern int ejsLoadScriptFile(Ejs *ejs, cchar *path, cchar *cache, int flags);
extern int ejsLoadScriptLiteral(Ejs *ejs, EjsString *script, cchar *cache, int flags);

/**
    Evaluate a module
    @description Evaluate a module containing compiled Ejscript.
    @param path Filename of the module to evaluate.
    @return Return zero on success. Otherwise return a negative Mpr error code.
    @ingroup Ejs
 */
extern int ejsEvalModule(cchar *path);

/**
    Evaluate a script
    @description Evaluate a script. This requires linking with the Ejscript compiler library (libec). 
    @param script Script to evaluate
    @return Return zero on success. Otherwise return a negative Mpr error code.
    @ingroup Ejs
 */
extern int ejsEvalScript(cchar *script);

/**
    Instruct the interpreter to exit.
    @description This will instruct the interpreter to cease interpreting any further script code.
    @param ejs Interpeter object returned from #ejsCreate
    @param status Reserved and ignored
    @ingroup Ejs
 */
extern void ejsExit(Ejs *ejs, int status);

/**
    Get the hosting handle
    @description The interpreter can store a hosting handle. This is typically a web server object if hosted inside
        a web server
    @param ejs Interpeter object returned from #ejsCreate
    @return Hosting handle
    @ingroup Ejs
 */
extern void *ejsGetHandle(Ejs *ejs);

/**
    Run a script
    @description Run a script that has previously ben compiled by ecCompile
    @param ejs Interpeter object returned from #ejsCreate
    @return Zero if successful, otherwise a non-zero Mpr error code.
 */
extern int ejsRun(Ejs *ejs);

/**
    Throw an exception
    @description Throw an exception object 
    @param ejs Interpeter object returned from #ejsCreate
    @param error Exception argument object.
    @return The exception argument for chaining.
    @ingroup Ejs
 */
extern EjsAny *ejsThrowException(Ejs *ejs, EjsAny *error);
extern void ejsClearException(Ejs *ejs);

/**
    Report an error message using the MprLog error channel
    @description This will emit an error message of the format:
        @li program:line:errorCode:SEVERITY: message
    @param ejs Interpeter object returned from #ejsCreate
    @param fmt Is an alternate printf style format to emit if the interpreter has no valid error message.
    @param ... Arguments for fmt
    @ingroup Ejs
 */
extern void ejsReportError(Ejs *ejs, char *fmt, ...);

extern EjsAny *ejsCastOperands(Ejs *ejs, EjsAny *lhs, int opcode, EjsAny *rhs);
extern int ejsCheckModuleLoaded(Ejs *ejs, cchar *name);
extern void ejsClearExiting(Ejs *ejs);
extern EjsAny *ejsCreateException(Ejs *ejs, int slot, cchar *fmt, va_list fmtArgs);
extern EjsAny *ejsGetVarByName(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup);
extern int ejsInitStack(Ejs *ejs);
extern void ejsLog(Ejs *ejs, cchar *fmt, ...);

extern int ejsLookupVar(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup);
extern int ejsLookupVarWithNamespaces(Ejs *ejs, EjsObj *obj, EjsName name, EjsLookup *lookup);

extern int ejsLookupScope(Ejs *ejs, EjsName name, EjsLookup *lookup);
#if UNUSED
extern void ejsMemoryNotifier(int flags, size_t size);
#endif
extern int ejsRunProgram(Ejs *ejs, cchar *className, cchar *methodName);
extern void ejsSetHandle(Ejs *ejs, void *handle);
extern void ejsShowCurrentScope(Ejs *ejs);
extern void ejsShowStack(Ejs *ejs, EjsFunction *fp);
extern void ejsShowBlockScope(Ejs *ejs, EjsBlock *block);
extern int ejsStartMprLogging(char *logSpec);
extern void ejsCreateObjHelpers(Ejs *ejs);
extern void ejsCloneObjHelpers(Ejs *ejs, EjsType *type);
extern void ejsClonePotHelpers(Ejs *ejs, EjsType *type);
extern void ejsCloneBlockHelpers(Ejs *ejs, EjsType *type);
extern int  ejsParseModuleVersion(cchar *name);

extern void ejsLockVm(Ejs *ejs);
extern void ejsUnlockVm(Ejs *ejs);
extern void ejsLockService(Ejs *ejs);
extern void ejsUnlockService(Ejs *ejs);

#ifdef __cplusplus
}
#endif
#endif /* _h_EJS_CORE */

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

