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
struct EjsMem;
struct EjsNames;
struct EjsModule;
struct EjsNamespace;
struct EjsObj;
struct EjsService;
struct EjsString;
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
#define EJS_PROP_ENUMERABLE             0x4000      /**< Property will be enumerable (compiler use only) */

#define EJS_FUN_CONSTRUCTOR             0x8000      /**< Method is a constructor */
#define EJS_FUN_FULL_SCOPE              0x10000     /**< Function needs closure when defined */
#define EJS_FUN_HAS_RETURN              0x20000     /**< Function has a return statement */
#define EJS_FUN_INITIALIZER             0x40000     /**< Type initializer code */
#define EJS_FUN_OVERRIDE                0x80000     /**< Override base type */
#define EJS_FUN_MODULE_INITIALIZER      0x100000    /**< Module initializer */
#define EJS_FUN_REST_ARGS               0x200000    /**< Parameter is a "..." rest */
#define EJS_TRAIT_MASK                  0x3FFFFF    /**< Mask of trait attributes */

/*
    These attributes are never stored in EjsTrait but are often passed in "attributes"
 */
#define EJS_TYPE_CALLS_SUPER            0x400000    /**< Constructor calls super() */
#define EJS_TYPE_HAS_INSTANCE_VARS      0x800000    /**< Type has non-method instance vars (state) */
#define EJS_TYPE_DYNAMIC_INSTANCE       0x1000000   /**< Instances are not sealed */
#define EJS_TYPE_FINAL                  0x2000000   /**< Type can't be subclassed */
#define EJS_TYPE_FIXUP                  0x4000000   /**< Type needs to inherit base types properties */
#define EJS_TYPE_HAS_CONSTRUCTOR        0x8000000   /**< Type has a constructor */
#define EJS_TYPE_HAS_TYPE_INITIALIZER   0x10000000  /**< Type has an initializer */
#define EJS_TYPE_IMMUTABLE              0x20000000  /**< Instances are immutable */
#define EJS_TYPE_INTERFACE              0x40000000  /**< Class is an interface */

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
#define EJS_ERR                 MPR_ERR_GENERAL
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

//  MOB -- reorder this file

#if UNUSED
/************************************************* List *************************************************/
//  MOB -- move below
/**
    List type
    @description    The MprList is a dynamic growable array suitable for storing pointers to arbitrary objects.
    @stability      Prototype.
    @see            EjsList mprCreateList mprFree MprBuf
 */
typedef struct EjsList {
    void    **items;                    /* List item data */
    int     length;                     /* Count of used items */
    int     maxSize;                    /* Maximum capacity */
} EjsList;

//  TODO - DOC
extern void     ejsInitList(EjsList *list);
extern int      ejsAddItem(struct Ejs *ejs, EjsList *list, cvoid *item);
#if UNUSED
extern int      ejsAddItemToSharedList(struct Ejs *ejs, EjsList *list, cvoid *item);
#endif
extern EjsList  *ejsAppendList(struct Ejs *ejs, EjsList *list, EjsList *add);
extern int      ejsCopyList(struct Ejs *ejs, EjsList *dest, EjsList *src);
extern void     ejsClearList(EjsList *lp);
extern void     *ejsGetItem(EjsList *lp, int index);
extern void     *ejsGetLastItem(EjsList *lp);
extern int      ejsGetListCount(EjsList *lp);
extern void     *ejsGetNextItem(EjsList *lp, int *lastIndex);
extern void     *ejsGetPrevItem(EjsList *lp, int *lastIndex);
extern int      ejsLookupItem(EjsList *lp, cvoid *item);
extern int      ejsRemoveItemAtPos(EjsList *lp, int index);
extern int      ejsRemoveLastItem(EjsList *lp);
extern int      ejsSetListDetails(struct Ejs *ejs, EjsList *list, int initialSize, int maxSize);
#endif

/************************************************ Unicode ***********************************************/
//  MOB -- tie into MPR definitions

#define BLD_UNICODE_LEN  1
#if BLD_UNICODE_LEN == 4
    typedef int EjsChar;
#elif BLD_UNICODE_LEN == 2
    //  MOB - must be compiled -fshort-wchar
    typedef short EjsChar;
#else
    typedef char EjsChar;
#endif

typedef void EV;

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
    struct EjsString   *name;                          /**< Property name */
    struct EjsString   *space;                         /**< Property namespace */
} EjsName;


/** 
    Allocation and Type Helpers
    @description The type helpers interface defines the set of primitive operations a type must support to
        interact with the virtual machine.
    @ingroup EjsType
 */
typedef struct EjsHelpers {
    EjsManager   manager;
#if UNUSED
    struct EjsType *type;           
#enidf
    /* Used by objects and values */
    EV      *(*cast)(struct Ejs *ejs, EV *ev, struct EjsType *type);
    void    *(*clone)(struct Ejs *ejs, EV *ev, bool deep);
//  MOB -- rename alloc/free
    EV      *(*create)(struct Ejs *ejs, struct EjsType *type, int size);
    int     (*defineProperty)(struct Ejs *ejs, EV *ev, int slotNum, EjsName qname, struct EjsType *propType, 
                int64 attributes, EV *value);
    int     (*deleteProperty)(struct Ejs *ejs, EV *ev, int slotNum);
    int     (*deletePropertyByName)(struct Ejs *ejs, EV *ev, EjsName qname);
//  MOB -- rename alloc/free
    void    (*destroy)(struct Ejs *ejs, EV *ev);
    EV      *(*getProperty)(struct Ejs *ejs, EV *ev, int slotNum);
    EV      *(*getPropertyByName)(struct Ejs *ejs, EV *ev, EjsName qname);
    int     (*getPropertyCount)(struct Ejs *ejs, EV *ev);
    EjsName (*getPropertyName)(struct Ejs *ejs, EV *ev, int slotNum);
    EV      *(*invokeOperator)(struct Ejs *ejs, EV *ev, int opCode, EV *rhs);
    void    (*mark)(struct Ejs *ejs, EV *ev);
    int     (*lookupProperty)(struct Ejs *ejs, EV *ev, EjsName qname);
    int     (*setProperty)(struct Ejs *ejs, EV *ev, int slotNum, EV *value);
    int     (*setPropertyByName)(struct Ejs *ejs, EV *ev, EjsName qname, EV *value);
    int     (*setPropertyName)(struct Ejs *ejs, EV *ev, int slotNum, EjsName qname);
} EjsHelpers;

/************************************************* EjsMem ***********************************************/
/*
    Ejscript memory allocator.  The allocator is a fast, immediate coalescing allocator that will return memory back 
    to the O/S if not required.  It is optimized for frequent allocations of small blocks (< 4K) and uses a scheme of 
    O(1) free queues for fast allocation. Allocations are aligned on 16 byte boundaries on 64-bit systems and on 8 byte 
    boundaries otherwise.
 */
#if BLD_DEBUG
    #define EJS_MEMORY_DEBUG        1                   /* Fill blocks, verifies block integrity. */
    #define EJS_MEMORY_STATS        1                   /* Include memory stats routines */
#else
    #define EJS_MEMORY_DEBUG        0
    #define EJS_MEMORY_STATS        0
#endif

#if MPR_64_BIT
    #define EJS_ALIGN               16
    #define EJS_ALIGN_SHIFT         4
    #define EJS_SIZE_BITS           56
#else
    #define EJS_ALIGN               8
    #define EJS_ALIGN_SHIFT         3
    #define EJS_SIZE_BITS           24
#endif

#define EJS_ALLOC_MAGIC             0xe114e5ae
#define EJS_ALLOC_MIN_SPLIT         (32 + sizeof(EjsMem))
#define EJS_ALLOC_BUCKET_SHIFT      4
#define EJS_ALLOC_NUM_BITS          (sizeof(void*) * 8)
#define EJS_ALLOC_NUM_GROUPS        (EJS_ALLOC_NUM_BITS - EJS_ALLOC_BUCKET_SHIFT - EJS_ALIGN_SHIFT - 1)
#define EJS_ALLOC_NUM_BUCKETS       (1 << EJS_ALLOC_BUCKET_SHIFT)

#define EJS_ALLOC_ALIGN(x)          (((x) + EJS_ALIGN - 1) & ~(EJS_ALIGN - 1))
#define EJS_PAGE_ALIGN(x, psize)    ((((size_t) (x)) + ((size_t) (psize)) - 1) & ~(((size_t) (psize)) - 1))
#define EJS_PAGE_ALIGNED(x, psize)  ((((size_t) (x)) % ((size_t) (psize))) == 0)
#define EJS_BUFSIZE                 4096

/*
    Max/min O/S allocation chunk sizes
 */
#define EJS_REGION_MIN_SIZE         MPR_MEM_CHUNK_SIZE
#if BLD_TUNE == MPR_TUNE_SPEED
    #define EJS_REGION_MAX_SIZE     (4 * 1024 * 1024)
#else
    #define EJS_REGION_MAX_SIZE     EJS_REGION_MIN_SIZE
#endif
#define EJS_ALLOC_RETURN            (32 * 1024)

/*
    Default GC thresholds (not tunable)
 */
#define EJS_MIN_TIME_FOR_GC     300     /* Need 1/3 sec for GC */
#define EJS_SHORT_WORK_QUOTA    50      /* Predict GC short of a full work quota */
    
/*
    GC Object generations
 */
#define EJS_GEN_NEW         0           /* New objects */
#define EJS_GEN_ETERNAL     1           /* Builtin objects that live forever */
#define EJS_MAX_GEN         2           /* Number of generations for object allocation */

/*
    GC Collection modes
 */
#define EJS_GC_ETERNAL      1           /* Collect eternal generation */

typedef int (*EjsManager)(struct Ejs *ejs, void *ptr, int flags);

/*
    Lowest-level memory allocation structure
 */
typedef struct EjsMem {
    struct EjsMem    *next;                     /**< Next in generation or freeq */
    struct EjsMem    *prev;                     /**< Prev in generation or freeq */
    struct EjsMem    *prior;                    /**< Previous in memory (for coalescing) */

    size_t           size       : EJS_SIZE_BITS;/**< Inernal block length including header */

    uint             builtin    : 1;            /**< Object is part of ejs-core */
    uint             dynamic    : 1;            /**< Object may add properties */
    uint             free       : 1;            /**< Block is on a free queue */
    uint             hasType    : 1;            /**< Type field holds a type reference, else marker proc */
    uint             last       : 1;            /**< Block is last in memory region */
    uint             permanent  : 1;            /**< Block is permanent */
    uint             visited    : 1;            /**< Has been traversed */

#if PERHAPS
    uint             cross      : 1;            /**< Cross generational link */
#endif

    union {
        struct EjsType *type;                   /**< Type of block */
        EjsManager   manager;
    };

#if EJS_MEMORY_DEBUG
    uint             magic;                     /**< Unique signature */
    uint             seqno;                     /**< Unique allocations squence */
    struct EjsString *name;                     /**< Debug name */
#endif
} EjsMem;


/*
    Manager flags
 */
#define EJS_MANAGE_FREE     0x1                 /**< Block being freed. Free dependant resources */
#define EJS_MANAGE_MARK     0x2                 /**< Block being marked by GC. Mark dependant resources */

//  MOB -- rename EJS_GET_MEM
#define EJS_GET_MEM(ptr) ((EjsMem*) (((cchar*) (ptr)) - sizeof(EjsMem)))
#define EJS_GET_PTR(bp)  ((cchar*) (((cchar*) (bp)) + sizeof(EjsMem)))

//  MOB --- prefixes?
#define OBJECT(ejs, ptr)    (TYPE(ptr) == ejs->objectType)
#define MANAGER(ptr)        (EJS_GET_MEM(ptr)->manager)
#define TYPE(ptr)           (EJS_GET_MEM(ptr)->type)
#define VISITED(ptr)        (EJS_GET_MEM(ptr)->visited)
#define BUILTIN(ptr)        (EJS_GET_MEM(ptr)->builtin)
#define DYNAMIC(ptr)        (EJS_GET_MEM(ptr)->dynamic)
#define SHORT_SCOPE(ptr)    (EJS_GET_MEM(ptr)->shortScope)

/**
    Block structure when on a free list. This overlays EjsMem and replaces generational forw/back
 */
typedef struct EjsFreeMem {
    struct EjsFreeMem *next;                /* Free list forward chain */
    struct EjsFreeMem *prev;                /* Free list backward chain */
#if BLD_MEMORY_STATS
    /* These only exist in the queue header */
    size_t          size;                   /* Min size of block in queue */
    uint            count;                  /* Number of blocks on the queue */
#endif
} EjsFreeMem;


typedef struct EjsStrings {
    EjsMem          **buckets;              /**< Hash buckets and references to link chains */
    int             size;                   /**< Size of hash */
    int             count;                  /**< Number of strings */
} EjsStrings;


/*
    Heap control structure
 */
typedef struct EjsHeap {
    struct Ejs      *ejs;
    EjsFreeMem      free[MPR_ALLOC_NUM_GROUPS * MPR_ALLOC_NUM_BUCKETS];
    EjsFreeMem      *freeEnd;
    EjsStrings      strings;                /**< Interned strings hash */
    size_t          groupMap;
    size_t          bucketMap[MPR_ALLOC_NUM_GROUPS];
    EjsMem          generations[EJS_MAX_GEN];
    EjsMem          *gen;                   /**< Allocating generation */
    struct EjsType  *type;                  /**< Type with helpers for non-object allocations */
    
    int             collecting;             /**< Running garbage collection */
    int             degraded;               /**< Have exceeded redlineMemory */
    int             enabled;                /**< GC is enabled */
    int             gcRequired;             /**< Garbage collection is now required */
    int             nextSeqno;              /**< Next sequence number */
    int             pageSize;               /**< O/S page size */
    int             regionSize;             /**< Virtual memory allocation size */
    int             workQuota;              /**< Quota of work before GC */
    int             workDone;               /**< Count of allocations to determining if GC needed */

#if BLD_MEMORY_STATS
    size_t          bytesAllocated;         /* Bytes currently allocated */
    size_t          bytesFree;              /* Bytes currently free on heap queues */
    uint64          allocs;                 /* Count of times a block was split Calls to allocate memory from the O/S */
    uint64          joins;                  /* Count of times a block was joined (coalesced) with its neighbours */
    uint64          requests;               /* Count of memory allocation requests */
    uint64          reuse;                  /* Count of times a block was reused from a free queue */
    uint64          splits;                 /* Count of times a block was split */
    uint64          unpins;                 /* Count of times a block was unpinned and released back to the O/S */
#endif
} EjsHeap;


extern void ejsCheckBlock(EjsMem *bp);
extern int ejsIsBlkValid(EjsMem *bp);

typedef struct EjsLoadState {
    struct EjsArray *typeFixups;            /**< Loaded types to fixup */
    int             firstModule;            /**< First module in ejs->modules for this load */
    int             flags;                  /**< Module load flags */
} EjsLoadState;

typedef void (*EjsLoaderCallback)(struct Ejs *ejs, int kind, ...);

/**
    Ejsript Interperter Structure
    @description The Ejs structure contains the state for a single interpreter. The #ejsCreate routine may be used
        to create multiple interpreters and returns a reference to be used in subsequent Ejscript API calls.
    @stability Prototype.
    @defgroup Ejs Ejs
    @see ejsCreate, ejsCreateService, ejsAppendSearchPath, ejsSetSearchPath, ejsEvalFile, ejsEvalScript, ejsExit
 */
typedef struct Ejs {
    EV                  *exception;         /**< Pointer to exception object */
    EV                  *result;            /**< Last expression result */
    struct EjsState     *state;             /**< Current evaluation state and stack */
    struct EjsState     *masterState;       /**< Owns the eval stack */

    struct EjsService   *service;           /**< Back pointer to the service */
    EjsHeap             *heap;              /**< Ejs Allocation heap */
    cchar               *bootSearch;        /**< Module search when bootstrapping the VM */
    struct EjsArray     *search;            /**< Module load search path */
//  XX
    cchar               *className;         /**< Name of a specific class to run for a program */
    cchar               *methodName;        /**< Name of a specific method to run for a program */

    /*
        Essential types
     */
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
    //  MOB - should all these values be EV (don't have two globals)

    struct EjsObj       *global;            /**< The "global" object as an EjsObj */
    struct EjsBlock     *globalBlock;       /**< The "global" object as an EjsBlock */

    EV                  *falseValue;        /**< The "false" value */
    struct EjsNumber    *infinityValue;     /**< The infinity number value */
    struct EjsIterator  *iterator;          /**< Default iterator */
    struct EjsNumber    *maxValue;          /**< Maximum number value */
    struct EjsNumber    *minValue;          /**< Minimum number value */
    struct EjsNumber    *minusOneValue;     /**< The -1 number value */
    struct EjsNumber    *nanValue;          /**< The "NaN" value if floating point numbers, else zero */
    struct EjsNumber    *negativeInfinityValue; /**< The negative infinity number value */
    struct EjsFunction  *nopFunction;       /**< The NOP function */
    EV                  *nullValue;         /**< The "null" value */
    struct EjsNumber    *oneValue;          /**< The 1 number value */
    EV                  *trueValue;         /**< The "true" value */
    EV                  *undefinedValue;    /**< The "void" value */
    struct EjsNumber    *zeroValue;         /**< The 0 number value */
    struct EjsFunction  *memoryCallback;    /**< Memory.readline callback */

    //  MOB - need a more scalable solution (index based on first 4 chars only)
    struct EjsString    *emptyString;       /**< "" */
    struct EjsString    *lengthString;      /**< "length" */
    struct EjsString    *publicString;      /**< "public" */
    struct EjsString    *commaProtString;   /**< ",protected" */

    struct EjsNamespace *emptySpace;        /**< Empty namespace */
    struct EjsNamespace *ejsSpace;          /**< Ejs namespace */
    struct EjsNamespace *iteratorSpace;     /**< Iterator namespace */
    struct EjsNamespace *internalSpace;     /**< Internal namespace */
    struct EjsNamespace *publicSpace;       /**< Public namespace */

    char                *castTemp;          /**< Temporary string for casting */
    char                *errorMsg;          /**< Error message */
    cchar               **argv;             /**< Command line args */
    int                 argc;               /**< Count of command line args */
    int                 flags;              /**< Execution flags */
    int                 exitStatus;         /**< Status to exit() */
    int                 serializeDepth;     /**< Serialization depth */
    int                 joining;            /**< In Worker.join */
    int                 spreadArgs;         /**< Count of spread args */

    uint                compiling: 1;       /**< Currently executing the compiler */
    uint                empty: 1;           /**< Interpreter will be created empty */
    uint                initialized: 1;     /**< Interpreter fully initialized and not empty */
    uint                hasError: 1;        /**< Interpreter has an initialization error */
    uint                exiting: 1;         /**< VM should exit */

    EV                  *exceptionArg;      /**< Exception object for catch block */

    MprDispatcher       *dispatcher;        /**< Event dispatcher */
    struct EjsArray     *workers;           /**< Worker interpreters */
    struct EjsArray     *modules;           /**< Loaded modules */

    void                (*loaderCallback)(struct Ejs *ejs, int kind, ...);
    void                *userData;          /**< User data */

    struct EjsObj       *coreTypes;         /**< Core type instances */
#if UNUSED
    MprHashTable        *standardSpaces;    /**< Hash of standard namespaces (global namespaces) */
#endif
    MprHashTable        *doc;               /**< Documentation */
    void                *sqlite;            /**< Sqlite context information */

    Http                *http;              /**< Http service object (copy of EjsService.http) */
    HttpLoc             *loc;               /**< Current HttpLocation object for web start scripts */

#if UNUSED
    EV                  *sessions;          /**< Session cache */
    int                 sessionTimeout;     /**< Default session timeout */
    MprEvent            *sessionTimer;      /**< Session expiry timer */
#endif
    struct EjsType      *sessionType;       /**< Session type object */
    EV                  *applications;      /**< Application cache */
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
typedef struct EV *(*EjsFun)(Ejs *ejs, EV *thisObj, int argc, EV **argv);

//LEGACY
typedef EjsFun EjsProc;
typedef EjsFun EjsNativeFunction;
#endif

//  TODO is this used?
typedef int (*EjsSortFn)(Ejs *ejs, EV *p1, EV *p2, cchar *name, int order);

/**
    Initialize a Qualified Name structure
    @description Initialize the statically allocated qualified name structure using a name and namespace.
    @param qname Reference to an existing, uninitialized EjsName structure
    @param space Namespace string
    @param name Name string
    @return A reference to the qname structure
    @ingroup EjsName
 */

//  MOB -- rationalize to allow both CStrings and Strings
extern EjsName ejsName(Ejs *ejs, cchar *space, cchar *name);

#define EN(name) ejsName(ejs, "", name)
#define N(space, name) ejsName(ejs, space, name)
#define NS(space, name) ejsName(ejs, space, name)

#if UNUSED
/**
    Allocate and Initialize  a Qualified Name structure
    @description Create and initialize a qualified name structure using a name and namespace.
    @param ctx Any memory context returned by mprAlloc
    @param space Namespace string
    @param name Name string
    @return A reference to an allocated EjsName structure. Caller must free.
    @ingroup EjsName
 */
//  XX
extern EjsName *ejsAllocName(MprCtx ctx, cchar *space, cchar *name);

//  MOB -- review
extern EjsName *ejsDupName(MprCtx ctx, EjsName qname);
extern EjsName ejsCopyName(MprCtx ctx, EjsName qname);
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


typedef struct EjsSlot {
    EjsName         qname;                  /**< Property name */
    int             hashChain;              /**< Next property in hash chain */
    EjsTrait        trait;                  /**< Property descriptor traits */
    union {
        EV          *ref;                   /**< Property reference */
        MprNumber   *number;                /**< Immediate number value */
    } value;
} EjsSlot;

//  XX - use this. Rename EjsProperties .size, .count
typedef struct EjsSlots {
    struct EjsSlot  *slots;                 /**< Vector of slots containing property references */
    int             sizeSlots;              /**< Current size of traits[] and slots[] */
    int             numSlots;               /**< Number of properties in traits/slots */
} EjsSlots;


typedef struct EjsHash {
    int             size;                   /**< Size of hash */
    int             *buckets;               /**< Hash buckets and head of link chains */
} EjsHash;


/** 
    Object Type. Base type for all objects.
    @description The EjsObj type is the foundation for all types, blocks, functions and scripted classes. 
        It provides storage and hashed lookup for properties.
        \n\n
        EjsObj stores properties in an array of slots. These slots store a reference to the property value. 
        Property names are stored in a names hash. Dynamic objects own their own name hash. Sealed object instances 
        of a type, will simply refer to the hash of names owned by the type.
        \n\n
        EjsObjs may be either dynamic or sealed. Dynamic objects can grow the number of properties. Sealed 
        objects cannot. Sealed objects will store the slot array as part of the EjsObj memory chunk. Dynamic 
        objects will perform a separate allocation for the slot array so that it can grow.
    @stability Evolving.
    @defgroup EjsObj EjsObj
    @see EjsObj ejsIsObject ejsCreateSimpleObject ejsCreateObject ejsCloneObject ejsGrowObject ejsMarkObject
        TODO - change these From Var
        ejsGetVarType ejsAllocObj ejsFreeObj ejsCast ejsClone ejsCreateInstance ejsCreateVar
        ejsDestroyVar ejsDefineProperty ejsDeleteProperty ejsDeletePropertyByName
        ejsGetProperty ejsLookupProperty ejsSetProperty ejsSetPropertyByName ejsSetPropertyName
        ejsSetPropertyTrait ejsDeserialize ejsParseVar
 */
typedef struct EjsObj {
#if BLD_HAS_UNNAMED_UNIONS
    union {
        struct {
#endif
            uint    isBlock         :  1;       /**< Instance is a block */
            uint    isFrame         :  1;       /**< Instance is a frame */
            uint    isFunction      :  1;       /**< Instance is a function */
            uint    isPrototype     :  1;       /**< Object is a type prototype object */
            uint    isType          :  1;       /**< Instance is a type object */
            uint    separateSlots   :  1;       /**< Object has separate slots[] memory */
            uint    shortScope      : 1;        /**< Don't follow type or base classes */
#if BLD_HAS_UNNAMED_UNIONS
        };
        int         bits;
    };
#endif

//  XX -- convert to EjsSlots *properties;
    struct EjsSlot  *slots;                     /**< Vector of slots containing property references */
    int             sizeSlots;                  /**< Current size of traits[] and slots[] */
    int             numSlots;                   /**< Number of properties in traits/slots */
    EjsHash         *hash;                      /**< Hash buckets and head of link chains */
#if BLD_DEBUG
    EjsMem          *mem;                       /**< Pointer to underlying memory block */
#endif
} EjsObj;

#if UNUSED
/*  
    Compare if a variable is an instance or sub-type of a given type described by the type's global slot.
 */
#define ejsIs(ev, slot)   _ejsIs(ev, slot)
#endif
extern int ejsIs(EV *ev, int slot);
#define ejsIsObject(ejs, ev) (ev && OBJECT(ejs, ev))

#if UNUSED
#if DOXYGEN
    /** 
        Get a variables type
        @description Get the base type for a variable
        @param obj Object reference
        @returns A reference to the variables type object
        @ingroup EjsObj
     */
    extern EjsType *ejsGetVarType(EjsObj *obj);
#else
    #define ejsGetVarType(obj)       (obj->type)
#endif
#endif

extern void *ejsAlloc(Ejs *ejs, size_t size);
extern void *ejsAllocWithManager(Ejs *ejs, size_t size, EjsManager manager);
#define ejsAllocStruct(ctx, type, manager) ejsAllocWithManager(ctx, sizeof(type), (EjsManager) manager))


extern EV *ejsAllocValue(Ejs *ejs, struct EjsType *type, int extra);

extern void *ejsRealloc(Ejs *ejs, void *ptr, size_t size);
extern void *ejsAllocAndFree(Ejs *ejs, void *ptr, size_t size);
extern void ejsFree(Ejs *ejs, void *ptr);
extern size_t ejsGetBlockSize(cvoid *ptr); 
#define ejsAllocVar ejsAllocObj

extern void *ejsDup(Ejs *ejs, void *ptr, bool deep);

#if BLD_DEBUG || DOXYGEN
extern struct EjsString *ejsGetDebugName(void *ptr);

//  MOB -- should the default be char*
extern void ejsSetDebugName(void *ptr, struct EjsString *name);
extern void ejsCopyDebugName(void *dest, cvoid *src);
#else
#define ejsGetDebugName(ptr)
#define ejsSetDebugName(ptr)
#define ejsCopyDebugName(dest, src)
#endif

#if UNUSED
//  MOB -- Rename to SetCSDebugName
#define ejsSetLiteralDebugName(ptr, name) ejsSetDebugName(ptr, ejsCreateStringFromCS(ejs, name))
#endif

#define ejsFreeVar(ejs, obj, id) ejsFree(ejs, obj)

//  MOB -- ideal to delete ejsAllocVar + ejsFreeVar
#if UNUSED
/** 
    Allocate a new variable
    @description This will allocate space for a bare variable. This routine should only be called by type factories
        when implementing the createVar helper.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type object from which to create an object instance
    @param size Size of extra property slots to reserve. This is used for dynamic objects.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EV
 */
extern EV *ejsAllocVar(Ejs *ejs, struct EjsType *type, int size);
#endif

/**
    Free a new variable
    @description This should typically only be called by the destroyVar type helper which is invoked by the GC when
        a variable is no longer needed. It should not be called by normal code.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to free
    @param pool Optional pool id. Set to -1 for defaults.
    @ingroup EV
 */
extern void ejsFreeVar(Ejs *ejs, EV *ev, int pool);

/** 
    Cast a variable to a new type
    @description Cast a variable and return a new variable of the required type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to cast
    @param type Type to cast to
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EV
 */
extern EV *ejsCast(Ejs *ejs, EV *ev, struct EjsType *type);

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
    @ingroup EV
 */
extern EV *ejsClone(Ejs *ejs, EV *ev, bool deep);

/** 
    Create a new variable instance 
    @description Create a new variable instance and invoke any required constructors with the given arguments.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type from which to create a new instance
    @param argc Count of args in argv
    @param argv Vector of arguments. Each arg is an EV.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EV
 */
extern EV *ejsCreateInstance(Ejs *ejs, struct EjsType *type, int argc, EV **argv);

/** 
    Create a variable
    @description Create a variable of the required type. This invokes the createVar helper method for the specified type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type to cast to
    @param numSlots Size of extra property slots to reserve. This is used for dynamic objects.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EV
 */
extern EV *ejsCreate(Ejs *ejs, struct EjsType *type, int numSlots);

#if UNUSED
/** 
    Destroy a variable
    @description Destroy a variable of the required type. This invokes the destroyVar helper method for the specified type.
        The default action for the destroyVar helper is to simply invoke ejsFreeObj which will return the variable
        storage to a type pool or return the memory to the heap.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Varaible to destroy
    @ingroup EV
 */
extern void ejsDestroy(Ejs *ejs, EV *ev);
#endif

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
    @ingroup EV
 */
extern int ejsDefineProperty(Ejs *ejs, EV *ev, int slotNum, EjsName qname, struct EjsType *type, 
    int64 attributes, EV *value);

/** 
    Delete a property
    @description Delete a variable's property and set its slot to null. The slot is not reclaimed and subsequent properties
        are not compacted.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable in which to delete the property
    @param slotNum Slot number in the variable for the property to delete.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EV
 */
extern int ejsDeleteProperty(Ejs *ejs, EV *ev, int slotNum);

/** 
    Delete a property by name
    @description Delete a variable's property by name and set its slot to null. The property is resolved by using 
        ejsLookupProperty with the specified name. Once deleted, the slot is not reclaimed and subsequent properties
        are not compacted.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable in which to delete the property
    @param qname Qualified name for the property including name and namespace.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EV
 */
extern int ejsDeletePropertyByName(Ejs *ejs, EV *ev, EjsName qname);

/** 
    Get a property
    @description Get a property from a variable at a given slot.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @return The variable property stored at the nominated slot.
    @ingroup EV
 */
extern EV *ejsGetProperty(Ejs *ejs, EV *ev, int slotNum);

/** 
    Get a count of properties in a variable
    @description Get a property from a variable at a given slot.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @return A positive integer count of the properties stored by the variable. 
    @ingroup EV
 */
extern int ejsGetLength(Ejs *ejs, EV *ev);
//  MOB -- globally change
#define ejsGetPropertyCount(ejs, ev) ejsGetLength(ejs, ev)

/** 
    Get a variable property's name
    @description Get a property name for the property at a given slot in the  variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param ev Object to examine
    @param slotNum Slot number for the requested property.
    @return The qualified property name including namespace and name. Caller must not free.
    @ingroup EV
 */
extern EjsName ejsGetPropertyName(Ejs *ejs, EV *ev, int slotNum);

/** 
    Get a property by name
    @description Get a property from a variable by name.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param qname Qualified name specifying both a namespace and name.
    @return The variable property stored at the nominated slot.
    @ingroup EV
 */
extern EV *ejsGetPropertyByName(Ejs *ejs, EV *ev, EjsName qname);

/** 
    Get a property's traits
    @description Get a property's trait description. The property traits define the properties base type,
        and access attributes.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param slotNum Slot number for the requested property.
    @return A trait structure reference for the property.
    @ingroup EV
 */
extern struct EjsTrait *ejsGetPropertyTrait(Ejs *ejs, EV *ev, int slotNum);

/** 
    Invoke an opcode on a native type.
    @description Invoke an Ejscript byte code operator on the specified variable given the expression right hand side.
        Native types would normally implement the invokeOperator helper function to respond to this function call.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param opCode Byte ope code to execute
    @param rhs Expression right hand side for binary expression op codes. May be null for other op codes.
    @return The result of the op code or NULL if the opcode does not require a result.
    @ingroup EV
 */
extern EV *ejsInvokeOperator(Ejs *ejs, EV *ev, int opCode, EV *rhs);

/** 
    Lookup a property by name
    @description Search for a property by name in the given variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param qname Qualified name of the property to search for.
    @return The slot number containing the property. Then use #ejsGetProperty to retrieve the property or alternatively
        use ejsGetPropertyByName to lookup and retrieve in one step.
    @ingroup EV
 */
extern int ejsLookupProperty(Ejs *ejs, EV *ev, EjsName qname);

/** 
    Mark a variable as currently in use.
    @description Mark a variables as currently active so the garbage collector will preserve it. This routine should
        be called by native types in their markVar helper.
    @param ejs Interpreter instance returned from #ejsCreate
    @param ptr Pointer to allocated block to mark
    @ingroup EV
 */
extern void ejsMark(Ejs *ejs, EV *ptr);

/** 
    Set a property's value
    @description Set a value for a property at a given slot in the specified variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @param value Reference to a value to store.
    @return The slot number of the property updated.
    @ingroup EV
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
    @ingroup EV
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
    @ingroup EV
 */
extern int ejsSetPropertyName(Ejs *ejs, EV *ev, int slotNum, EjsName qname);

/** 
    Set a property's traits
    @description Set the traits describing a property. These include the property's base type and access attributes.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param slotNum Slot number of the property in the variable.
    @param type Base type for the property. Set to NULL for an untyped property.
    @param attributes Integer mask of access attributes.
    @return The slot number of the property updated.
    @ingroup EV
 */
extern int ejsSetPropertyTrait(Ejs *ejs, EV *ev, int slotNum, struct EjsType *type, int attributes);

//  TODO - DOC - 
extern EV *ejsDeserialize(Ejs *ejs, struct EjsString *value);

//  TODO - XX MOB UNICODE - should take EjsString - lots of change in ejsJSON
extern EV *ejsParse(Ejs *ejs, cchar *str,  int prefType);
extern void ejsZeroSlots(Ejs *ejs, EjsSlot *slots, int count);
extern void ejsCopySlots(Ejs *ejs, EjsObj *obj, EjsSlot *dest, EjsSlot *src, int count);

/** 
    Create a simple object
    @description Create a simple object using Object as its base type.
    @param ejs Interpreter instance returned from #ejsCreate
    @return A new object instance
    @ingroup EjsObj
 */
extern EjsObj *ejsCreateSimpleObject(Ejs *ejs);

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
extern void *ejsCreateObject(Ejs *ejs, struct EjsType *type, int size);

extern int ejsInsertGrowObject(Ejs *ejs, EjsObj *obj, int numSlots, int offset);
extern int ejsRemoveProperty(Ejs *ejs, EjsObj *obj, int slotNum);
extern int ejsGetOwnNames(Ejs *ejs, EjsObj *obj, int sizeNames);
extern int ejsMakeObjHash(Ejs *ejs, EjsObj *obj);
extern void ejsClearObjHash(EjsObj *obj);

extern int ejsGetOwnTraits(Ejs *ejs, EV *ev, int sizeTraits);
extern void ejsSetTraitType(Ejs *ejs, struct EjsTrait *trait, struct EjsType *type);
extern void ejsSetTraitAttributes(Ejs *ejs, struct EjsTrait *trait, int attributes);
extern EjsTrait *ejsGetTrait(Ejs *ejs, void *obj, int slotNum);
extern int ejsSetTraitDetails(Ejs *ejs, void *obj, int slotNum, struct EjsType *type, int attributes);

//  MOB -- should take ejs as first arg
extern int ejsHasTrait(EjsObj *obj, int slotNum, int attributes);
extern int ejsGetTraitAttributes(EjsObj *obj, int slotNum);
extern struct EjsType *ejsGetTraitType(EjsObj *obj, int slotNum);
extern int ejsBlendObject(Ejs *ejs, EjsObj *dest, EjsObj *src, int overwrite);
extern int ejsCompactObject(Ejs *ejs, EjsObj *obj);
extern int ejsLookupObjectProperty(struct Ejs *ejs, EjsObj *obj, EjsName qname);
extern EjsName ejsGetObjectPropertyName(Ejs *ejs, EjsObj *obj, int slotNum);


//  TODO - inconsistent naming vs ejsCloneVar (clone vs copy)
//
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
extern void *ejsCloneObject(Ejs *ejs, void *src, bool deep);

/** 
    Grow an object
    @description Grow the property storage for an object. Object properties are stored in slots. To store more 
        properties, you need to grow the slots.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object reference to grow
    @param numSlots New minimum count of properties. If size is less than the current number of properties, the call
        will be ignored, i.e. it will not shrink objects.
    @return Zero if successful
    @ingroup EjsObj
 */
extern int ejsGrowObject(Ejs *ejs, EjsObj *obj, int numSlots);

/** 
    Mark an object as currently in use.
    @description Mark an object as currently active so the garbage collector will preserve it. This routine should
        be called by native types that extend EjsObj in their markVar helper.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to mark as currently being used.
    @ingroup EjsObj
 */
extern void     ejsMarkObject(Ejs *ejs, void *obj);

extern int      ejsGetSlot(Ejs *ejs, EjsObj *obj, int slotNum);
extern EjsObj   *ejsCoerceOperands(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs);
extern int      ejsComputeStringHashCode(struct EjsString *name, int size);
extern int      ejsComputeCStringHashCode(cchar *name, int size);
extern int      ejsGetHashSize(int numProp);
extern void     ejsCreateObjectHelpers(Ejs *ejs);
extern int      ejsInsertGrowObject(Ejs *ejs, EjsObj *obj, int size, int offset);
extern void     ejsMakePropertyDontDelete(EjsObj *obj, int dontDelete);
extern int      ejsMakePropertyEnumerable(EjsObj *obj, bool enumerable);
extern void     ejsMakePropertyReadOnly(EjsObj *obj, int readonly);
extern EjsObj   *ejsObjectOperator(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs);
extern int      ejsRebuildHash(Ejs *ejs, EjsObj *obj);
extern void     ejsResetHash(Ejs *ejs, EjsObj *obj);
extern void     ejsRemoveSlot(Ejs *ejs, EjsObj *slots, int slotNum, int compact);
extern void     ejsSetAllocIncrement(Ejs *ejs, struct EjsType *type, int increment);
extern EjsObj   *ejsToSource(Ejs *ejs, EjsObj *obj, int argc, EV **argv);

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
        ejsVarToString ejsStrdup ejsStrcat ejsIsString ejsGetString
 */
typedef struct EjsString {
    int         length;                         /* Length of the string (excluding the trailing null) */
    EjsChar     value[0];
} EjsString;

#define CS(s) ejsCreateStringFromCS(ejs, s)

#define EJS_TRIM_START  0x1
#define EJS_TRIM_END    0x2
#define EJS_TRIM_BOTH   0x3

//  MOB - doc
//  MOB -- consistencty. ejsCreateStringFromCS vs ContainsCString
extern int ejsAtoi(Ejs *ejs, EjsString *sp, int radix);
extern EjsString *ejsCatString(Ejs *ejs, EjsString *dest, EjsString *src);
extern EjsString *ejsCatStrings(Ejs *ejs, EjsString *src, ...);
extern int ejsContainsCString(Ejs *ejs, EjsString *sp, cchar *pat);
extern int ejsContainsUString(Ejs *ejs, EjsString *sp, EjsChar *pat);
extern int ejsCompareString(Ejs *ejs, EjsString *s1, EjsString *s2);
extern int ejsCompareCString(Ejs *ejs, EjsString *s1, cchar *s2);
extern int ejsCompareUString(Ejs *ejs, EjsString *s1, EjsChar *s2);
extern int ejsCompareString(Ejs *ejs, EjsString *s1, EjsString *s2);
extern int ejsContainsChar(Ejs *ejs, EjsString *sp, int charPat);
extern int ejsContainsString(Ejs *ejs, EjsString *sp, EjsString *pat);
extern int ejsStartsWithCString(Ejs *ejs, EjsString *sp, cchar *pat);
//  MOB -- rename to ejsGetCS
extern cchar *ejsGetString(Ejs *ejs, void *obj);
extern EjsString *ejsInternString(Ejs *ejs, EjsString *sp);
extern EjsString *ejsSprintf(Ejs *ejs, cchar *fmt, ...);
extern EjsString *ejsSubString(Ejs *ejs, EjsString *src, int start, int len);
extern EjsString *ejsToLower(Ejs *ejs, EjsString *sp);
extern char *ejsToMulti(Ejs *ejs, EjsString *sp);
extern EjsString *ejsToUni(Ejs *ejs, cchar* src);
extern EjsString *ejsToUpper(Ejs *ejs, EjsString *sp);
extern EjsString *ejsTrimString(Ejs *ejs, EjsString *sp, cchar *pat, int flags);
extern EjsString *ejsTruncateString(Ejs *ejs, EjsString *sp, int len);


/** 
    Create a string object
    @param ejs Ejs reference returned from #ejsCreate
    @param value C string value to define for the string object. Note: this will soon be changed to unicode.
    @stability Prototype
    @return A string object
    @ingroup EjsString
 */
extern EjsString *ejsCreateStringFromCS(Ejs *ejs, cchar *value);

/** 
    Create a string object and free the argument
    @param ejs Ejs reference returned from #ejsCreate
    @param value C string value to define for the string object. Note: this will soon be changed to unicode.
    @stability Prototype
    @return A string object
    @ingroup EjsString
 */
extern EjsString *ejsCreateStringAndFree(Ejs *ejs, char *value);

/** 
    Create an empty string object. This creates an uninitialized string object of the requrired size. Once initialized,
    the string must be "interned" via $ejsInternString.
    @param ejs Ejs reference returned from #ejsCreate
    @param len Length of space to reserve for future string data
    @return A string object
    @ingroup EjsString
 */
extern EjsString *ejsCreateBareString(Ejs *ejs, int len);

/** 
    Create a string and reserve extra room.
    @param ejs Ejs reference returned from #ejsCreate
    @param value C string value to define for the string object. Note: this will soon be changed to unicode.
    @param len Length of the string storage to allocate.
    @return A string object
    @ingroup EjsString
 */
//  XX
extern EjsString *ejsCreateStringWithLength(Ejs *ejs, cchar *value, int len);

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

/**
    Convert a variable to a string in JSON format
    @param ejs Ejs reference returned from #ejsCreate
    @param ev Value to cast
    @param options Encoding options. See serialize for details.
    @return A string object
    @ingroup MOB
 */
extern EjsString *ejsToJSON(Ejs *ejs, EV *ev, EV *options);

/** 
    Cast a variable to a string
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to convert
    @return A string object
    @ingroup MOB
 */
extern EjsString *ejsToString(Ejs *ejs, EV *ev);

#if DOXYGEN
    bool ejsIsString(Ejs *ejs, EV *obj);
#else
    #define ejsIsString(ejs, obj) (obj && TYPE(obj) == ejs->stringType)
#endif

extern EjsString *ejsStringToJSON(Ejs *ejs, EV *ev);

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
    EjsObj          obj;                /**< Extends Object */
    EV              **data;             /**< Array elements */
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

extern int ejsAddItem(Ejs *ejs, EjsArray *ap, EV *item);
extern int ejsAppendArray(Ejs *ejs, EjsArray *dest, EjsArray *src);
extern void ejsClearArray(Ejs *ejs, EjsArray *ap);
extern int ejsInsertItem(Ejs *ejs, EjsArray *ap, int index, EV *item);
extern void *ejsGetItem(Ejs *ejs, EjsArray *ap, int index);
extern void *ejsGetFirstItem(Ejs *ejs, EjsArray *ap);
extern void *ejsGetLastItem(Ejs *ejs, EjsArray *ap);
extern void *ejsGetNextItem(Ejs *ejs, EjsArray *ap, int *next);
extern void *ejsGetPrevItem(Ejs *ejs, EjsArray *ap, int *next);
extern int ejsLookupItem(Ejs *ejs, EjsArray *lp, EV *item);
extern int ejsRemoveItem(Ejs *ejs, EjsArray *ap, EV *item);
extern int ejsRemoveLastItem(Ejs *ejs, EjsArray *ap);
extern int ejsRemoveItemAtPos(Ejs *ejs, EjsArray *ap, int index);

#if DOXYGEN
    /** 
        Determine if a variable is an array
        @param obj Object to test
        @return True if the variable is an array
        @ingroup EjsArray
     */
    extern bool ejsIsArray(Ejs *ejs, EV *ev);
#else
    #define ejsIsArray(ejs, obj) (obj && TYPE(obj) == ejs->arrayType)
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
//  MOB -- rename to LexBlock
typedef struct EjsBlock {
    EjsObj          obj;                            /**< Extends Object - Property storage */
    EjsArray        namespaces;                     /**< Current list of namespaces open in this block of properties */
    struct EjsBlock *scope;                         /**< Lexical scope chain for this block */
    struct EjsBlock *prev;                          /**< Previous block in activation chain */

    //  MOB -- OPT and compress / eliminate some of these fields. Every function has these.
    EV              *prevException;                 /**< Previous exception if nested exceptions */
    EV              **stackBase;                    /**< Start of stack in this block */
    uint            breakCatch: 1;                  /**< Return, break or continue in a catch block */
    uint            isGlobal: 1;                    /**< Block is the global block */
    uint            nobind: 1;                      /**< Don't bind to properties in this block */
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
    #define ejsIsBlock(ejs, obj) (ejsIsObject(ejs, obj) && ((EjsObj*) (obj))->isBlock)
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
extern int ejsBindFunction(Ejs *ejs, EV *obj, int slotNum, EV *fun);

/*  
    This is all an internal API. Native types should probably not be using these routines. Speak up if you find
    you need these routines in your code.
 */

extern int      ejsAddNamespaceToBlock(Ejs *ejs, EjsBlock *blockRef, struct EjsNamespace *nsp);
extern int      ejsAddScope(MprCtx ctx, EjsBlock *block, EjsBlock *scopeBlock);
extern EjsBlock *ejsCreateBlock(Ejs *ejs, int numSlots);

//  TODO - why do we have ejsCloneObject, ejsCloneBlock ... Surely ejsCloneVar is sufficient?
extern EjsBlock *ejsCloneBlock(Ejs *ejs, EjsBlock *src, bool deep);

extern int      ejsCaptureScope(Ejs *ejs, EjsBlock *block, struct EjsArray *scopeChain);
extern int      ejsCopyScope(EjsBlock *block, struct EjsArray *chain);
extern int      ejsGetNamespaceCount(EjsBlock *block);

extern EjsBlock *ejsGetTopScope(EjsBlock *block);
extern void     ejsMarkBlock(Ejs *ejs, EjsBlock *block);
extern void     ejsPopBlockNamespaces(EjsBlock *block, int count);
extern EjsBlock *ejsRemoveScope(EjsBlock *block);
extern void     ejsResetBlockNamespaces(Ejs *ejs, EjsBlock *block);

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


// TODO OPT. Could compress this.
/** 
    Byte code
    @description This structure describes a sequence of byte code for a function. It also defines a set of
        execption handlers pertaining to this byte code.
    @ingroup EjsFunction
 */
typedef struct EjsCode {
    uchar           *byteCode;              /**< Byte code */
    struct EjsConst *constants;             /**< Constant pool. Reference to module constant pool */
    struct EjsEx    **handlers;             /**< Exception handlers */
    int             codeLen;                /**< Byte code length */
    int             numHandlers;            /**< Number of exception handlers */
    int             sizeHandlers;           /**< Size of handlers array */
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
    EjsBlock        block;                  /** Activation block for local vars */
    EjsObj          *activation;            /** Activation properties (parameters + locals) */
    EjsString       *name;                  /** Function name */
#if FUTURE && MOB
    union {
#endif
        struct EjsFunction *setter;         /**< Setter function for this property */
        struct EjsType  *archetype;         /**< Type to use to create instances */
#if FUTURE && MOB
    } extra;
#endif

    union {
        EjsCode     code;                   /**< Byte code */
        EjsFun      proc;                   /**< Native function pointer */
    } body;

    struct EjsArray *boundArgs;             /**< Bound "args" */
    EV              *boundThis;             /**< Bound "this" object value */
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
    extern bool ejsIsFunction(Ejs *ejs, EV *ev);

    /** 
        Determine if the function is a native function. Functions can be either native - meaning the implementation is
            via a C function, or can be scripted.
        @param obj Object to test
        @return True if the variable is a native function.
        @ingroup EjsFunction
     */
    //  MOB -- convert all ejsIs to take ejs as arg
    extern bool ejsIsNativeFunction(Ejs *ejs, EV *ev);

    /** 
        Determine if the function is an initializer. Initializers are special functions created by the compiler to do
            static and instance initialization of classes during construction.
        @param obj Object to test
        @return True if the variable is an initializer
        @ingroup EjsFunction
     */
    //  MOB -- convert all ejsIs to take ejs as arg
    extern bool ejsIsInitializer(Ejs *ejs, EV *ev);
#else
    #define ejsIsFunction(ejs, ev)       (ev && OBJECT(ejs, ev) && ((EjsObj*) ev)->isFunction)
    #define ejsIsNativeFunction(ejs, ev) (ejsIsFunction(ejs, ev) && (((EjsFunction*) (ev))->isNativeProc))
    #define ejsIsInitializer(ejs, ev)    (ejsIsFunction(ejs, ev) && (((EjsFunction*) (ev))->isInitializer)
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
    @param constants Reference to the module constant pool. Some byte code opcodes contain references into the
        constant pool
    @param scope Reference to the chain of blocks that that comprises the lexical scope chain for this function.
    @param strict Run code in strict mode (vs standard).
    @return An initialized function object
    @ingroup EjsFunction
 */
extern EjsFunction *ejsCreateFunction(Ejs *ejs, EjsString *name, cuchar *code, int codeLen, int numArgs, int numDefault,
    int numExceptions, struct EjsType *returnType, int attributes, struct EjsConst *constants, EjsBlock *scope, 
    int strict);
extern void ejsInitFunction(Ejs *ejs, EjsFunction *fun, EjsString *name, cuchar *code, int codeLen, int numArgs, 
    int numDefault, int numExceptions, struct EjsType *returnType, int attributes, struct EjsConst *constants, 
    EjsBlock *scope, int strict);
extern EjsFunction *ejsCreateSimpleFunction(Ejs *ejs, EjsString *name, int attributes);
extern void ejsDisableFunction(Ejs *ejs, EjsFunction *fun);

extern EjsObj *ejsCreateActivation(Ejs *ejs, EjsFunction *fun, int numSlots);
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
extern EV *ejsRunInitializer(Ejs *ejs, struct EjsModule *module);

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
extern EV *ejsRunFunction(Ejs *ejs, EjsFunction *fn, EV *thisObj, int argc, EV **argv);

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
extern EV *ejsRunFunctionBySlot(Ejs *ejs, EV *obj, int slotNum, int argc, EV **argv);
extern EV *ejsRunFunctionByName(Ejs *ejs, EV *container, EjsName qname, EV *obj, int argc, EV **argv);

extern EjsEx *ejsAddException(Ejs *ejs, EjsFunction *mp, uint tryStart, uint tryEnd, struct EjsType *catchType,
    uint handlerStart, uint handlerEnd, int numBlocks, int numStack, int flags, int preferredIndex);
extern EjsFunction *ejsCloneFunction(Ejs *ejs, EjsFunction *src, int deep);
extern int ejsDefineException(Ejs *ejs, struct EjsType *obj, int slot, uint tryOffset,
    uint tryLength, uint handlerOffset, uint handlerLength, int flags);
extern void ejsOffsetExceptions(EjsFunction *mp, int offset);
extern int  ejsSetFunctionCode(Ejs *ejs, EjsFunction *mp, uchar *byteCode, int len);
extern void ejsMarkFunction(Ejs *ejs, EjsFunction *fun);
extern void ejsShowOpFrequency(Ejs *ejs);
#if UNUSED
extern int ejsLookupFunctionProperty(Ejs *ejs, EjsFunction *fun, EjsName qname);
#endif

typedef struct EjsFrame {
    EjsFunction     function;               /**< Activation frame for function calls. Stores local variables */
    EjsFunction     *orig;                  /**< Original function frame is based on */
    struct EjsFrame *caller;                /**< Previous invoking frame */
    EV              **stackBase;            /**< Start of stack in this function */
    EV              **stackReturn;          /**< Top of stack to return to */
    int             slotNum;                /**< Slot in owner */
    uchar           *pc;                    /**< Program counter */
    uchar           *attentionPc;           /**< Restoration PC value after attention */
    int             ignoreAttention;        /**< Ignore attention commands */
    int             lineNumber;             /**< Source code line number */
    EjsString       *currentLine;           /**< Current source code line */
    EjsString       *filename;              /**< Source code file name */
    int             getter;                 /**< Frame is a getter */
    uint            argc;                   /**< Actual parameter count */
} EjsFrame;

#if DOXYGEN
    /** 
        Determine if a variable is a frame. Only used internally in the VM.
        @param obj Object to test
        @return True if the variable is a frame. 
        @ingroup EjsFrame
     */
    extern bool ejsIsFrame(Ejs *ejs, EV *ev);
#else
    #define ejsIsFrame(ejs, obj) (ejsIsObject(ejs, obj) && ((EjsObj*) (obj))->isFrame)
#endif

extern EjsFrame *ejsCreateFrame(Ejs *ejs, EjsFunction *src, EV *thisObj, int argc, EV **argv);
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
extern EjsBoolean *ejsToBoolean(Ejs *ejs, EV *ev);

#if DOXYGEN
    /** 
        Determine if a variable is a boolean
        @param obj Object to test
        @return True if the variable is a boolean
        @ingroup EjsBoolean
     */
    extern bool ejsIsBoolean(Ejs *ejs, EV *ev);
#else
    #define ejsIsBoolean(ejs, obj) (obj && TYPE(obj) == ejs->booleanType)
#endif

/** 
    Get the C boolean value from a boolean object
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Boolean variable to access
    @return True or false
    @ingroup EjsBoolean
 */
extern bool ejsGetBoolean(Ejs *ejs, EV *ev);

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
    extern bool ejsIsByteArray(Ejs *ejs, EV *ev);
#else
    #define ejsIsByteArray(ejs, ev) (ev && TYPE(ev) == ejs->byteArrayType)
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
extern int ejsCopyToByteArray(Ejs *ejs, EjsByteArray *ba, int offset, char *data, int length);

extern void ejsResetByteArray(EjsByteArray *ba);
extern int ejsGetByteArrayAvailable(EjsByteArray *ba);
extern int ejsGetByteArrayRoom(EjsByteArray *ba);
extern int ejsGrowByteArray(Ejs *ejs, EjsByteArray *ap, int size);

extern struct EjsNumber *ejsWriteToByteArray(Ejs *ejs, EjsByteArray *ap, int argc, EV **argv);
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
    MprTime         value;              /**< Time in milliseconds since "1970/01/01 GMT" */
} EjsDate;

#if DOXYGEN
    /** 
        Determine if a variable is a Date
        @param obj Object to test
        @return True if the variable is a date
        @ingroup EjsDate
     */
    bool ejsIsDate(Ejs *ejs, EV *ev);
#else
    #define ejsIsDate(ejs, ev) (ev && TYPE(ev) == ejs->dateType)
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
//  MOB -- perhaps remove and just use EjsObj. Ie. no longer native
typedef struct EjsError {
    EjsObj          obj;                /**< Extends Object */
} EjsError;

#define ejsIsError(ejs, ev) (ev && ejsIsA(ejs, ev, ejs->byteArrayType))

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
extern EV *ejsGetException(Ejs *ejs);

/** 
    Throw an argument exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowArgError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an assertion exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowAssertError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an math exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowArithmeticError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an instruction code exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowInstructionError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an general error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an internal error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowInternalError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an IO exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowIOError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an Memory depletion exception
    @param ejs Ejs reference returned from #ejsCreate
    @ingroup EjsError
 */
extern EV *ejsThrowMemoryError(Ejs *ejs);

/** 
    Throw an out of bounds exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowOutOfBoundsError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an reference exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowReferenceError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an resource exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowResourceError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an state exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowStateError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an stop iteration exception
    @param ejs Ejs reference returned from #ejsCreate
    @ingroup EjsError
 */
extern EV *ejsThrowStopIteration(Ejs *ejs);

/** 
    Throw an syntax error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowSyntaxError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an type error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EV *ejsThrowTypeError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);


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
    extern bool ejsIsFile(Ejs *ejs, EV *ev);
#else
    #define ejsIsFile(ejs, ev) (ev && TYPE(ev) == ejs->fileType)
#endif


/**
    Path class
    @description The Path class provides file path name services.
    @stability Prototype
    @defgroup EjsPath EjsPath 
    @see EjsFile ejsCreatePath ejsIsPath
 */
typedef struct EjsPath {
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
//  MOB -- reverse
extern EjsPath *ejsCreatePath(Ejs *ejs, EjsString *path);
extern EjsPath *ejsCreatePathFromCS(Ejs *ejs, cchar *path);
extern EjsPath *ejsCreatePathAndFree(Ejs *ejs, char *path);

#if DOXYGEN
    /** 
        Determine if a variable is a Path
        @param ejs Ejs reference returned from #ejsCreate
        @param obj Object to test
        @return True if the variable is a Path
        @ingroup EjsPath
     */
    extern bool ejsIsPath(Ejs *ejs, EV *ev);
#else
    #define ejsIsPath(ejs, ev) (ev && TYPE(ev) == ejs->pathType)
#endif

//  MOB DOC
extern EjsPath *ejsToPath(Ejs *ejs, EV *ev);


/** 
    Uri class
    @description The Uri class provides file path name services.
    @stability Prototype
    @defgroup EjsUri EjsUri 
    @see EjsFile ejsCreateUri ejsIsUri
 */
typedef struct EjsUri {
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
extern EjsUri *ejsCreateUriFromCS(Ejs *ejs, cchar *uri);
extern EjsUri *ejsCreateUriAndFree(Ejs *ejs, char *uri);
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
    extern bool ejsIsUri(Ejs *ejs, EV *ev);
    extern cchar *ejsGetUri(Ejs *ejs, EV *ev);
#else
    #define ejsIsUri(ejs, ev) (ev && TYPE(ev) == ejs->uriType)
    extern cchar *ejsGetUri(Ejs *ejs, EV *ev);
#endif

//  MOB DOC
extern EjsUri *ejsToUri(Ejs *ejs, EV *ev);

/** 
    FileSystem class
    @description The FileSystem class provides file system services.
    @stability Prototype
    @defgroup EjsFileSystem EjsFileSystem 
    @see EjsFile ejsCreateFile ejsIsFile
 */
typedef struct EjsFileSystem {
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
    extern bool ejsIsFileSystem(Ejs *ejs, EV *ev);
#else
    #define ejsIsFileSystem(ejs, ev) (ev && TYPE(ev) == ejs->fileSystemType)
#endif


/** 
    EjsGlobal cass
    @description The Global class is the base class for the global object. The global object is the top level
        scoping object.
    @stability Stable
    @defgroup EjsGlobal EjsGlobal
 */
typedef EjsObj EjsGlobal;

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
    Ejs             *ejs;                       /**< Convenience access to ejs interpreter instance */
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

#if DOXYGEN
    extern bool ejsIsHttp(Ejs *ejs, EV *ev);
#else
    #define ejsIsHttpType(ejs, ev) (ev && TYPE(ev) == ejs->httpType)
#endif
extern void ejsSetHttpLimits(Ejs *ejs, HttpLimits *limits, EjsObj *obj, int server);
extern void ejsGetHttpLimits(Ejs *ejs, EjsObj *obj, HttpLimits *limits, int server);
extern int ejsSetupTrace(Ejs *ejs, MprCtx ctx, HttpTrace *trace, EjsObj *options);


/** 
    Iterator Class
    @description Iterator is a helper class to implement iterators in other native classes
    @stability Prototype
    @defgroup EjsIterator EjsIterator
    @see EjsIterator ejsCreateIterator
 */
typedef struct EjsIterator {
#if UNUSED
    EjsObj              var;                /**< Extends Object - Property storage (not used) */
#endif
    EV                  *target;            /**< Object to be enumerated */
    EjsFun              nativeNext;         /**< Native next function */
    bool                deep;               /**< Iterator deep (recursively over all properties) */
    EjsArray            *namespaces;        /**< Namespaces to consider in iteration */
    int                 index;              /**< Current index */
    EV                  *indexVar;          /**< Reference to current item */
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
extern EjsIterator *ejsCreateIterator(Ejs *ejs, EV *target, EjsFun next, bool deep, EjsArray *namespaces);

/** 
    Namespace Class
    @description Namespaces are used to qualify names into discrete spaces.
    @stability Evolving
    @defgroup EjsNamespace EjsNamespace
    @see EjsNamespace ejsIsNamespace ejsCreateNamespace ejsLookupNamespace ejsDefineReservedNamespace 
        ejsCreateReservedNamespace ejsFormatReservedNamespace 
 */
typedef struct EjsNamespace {
    EjsString   *name;              /**< Textual name of the namespace */
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
    extern bool ejsIsNamespace(Ejs *ejs, EV *ev)
#else
    #define ejsIsNamespace(ejs, ev) (ev && TYPE(ev) == ejs->namespaceType)
#endif

extern EjsNamespace *ejsDefineReservedNamespace(Ejs *ejs, EjsBlock *block, EjsName *typeName, EjsString *name);
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
typedef struct EjsNull {
} EjsNull;

/** 
    Determine if a variable is a null
    @return True if a variable is a null
    @ingroup EjsNull
 */
#define ejsIsNull(ejs, ev) (ev == ejs->nullValue)
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
    MprNumber   value;              /**< Numeric value */
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
extern struct EjsNumber *ejsToNumber(Ejs *ejs, EV *ev);

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
    EjsObj      *subject;           /**< Object under examination */
} EjsReflect;


extern struct EjsString *ejsGetTypeName(struct Ejs *ejs, EV *ev);
extern struct EjsString *ejsGetTypeOf(struct Ejs *ejs, EV *ev);

/** 
    RegExp Class
    @description The regular expression class provides string pattern matching and substitution.
    @stability Evolving
    @defgroup EjsRegExp EjsRegExp
    @see EjsRegExp ejsCreateRegExp ejsIsRegExp
 */
typedef struct EjsRegExp {
#if UNUSED
    EjsObj          var;                /**< Extends Object - Property storage */
#endif
    char            *pattern;           /**< Pattern to match with */
    void            *compiled;          /**< Compiled pattern */
    bool            global;             /**< Search for pattern globally (multiple times) */
    bool            ignoreCase;         /**< Do case insensitive matching */
    bool            multiline;          /**< Match patterns over multiple lines */
    bool            sticky;
    int             options;            /**< Pattern matching options */
    int             endLastMatch;       /**< End of the last match (one past end) */
    int             startLastMatch;     /**< Start of the last match */
    struct EjsString *matched;          /**< Last matched component */
} EjsRegExp;


/** 
    Create a new regular expression object
    @param ejs Ejs reference returned from #ejsCreate
    @param pattern Regular expression pattern string
    @return a EjsRegExp object
    @ingroup EjsRegExp
 */
extern EjsRegExp *ejsCreateRegExp(Ejs *ejs, EjsString *pattern);

struct EjsString *ejsRegExpToString(Ejs *ejs, EjsRegExp *rp);

#if DOXYGEN
    /** 
        Determine if the variable is a regular expression
        @return True if the variable is a regular expression
        @ingroup EjsRegExp
     */
    extern bool ejsIsRegExp(Ejs *ejs, EV *ev);
#else
    #define ejsIsRegExp(ejs, ev) (ev && TYPE(ev) == ejs->regExpType)
#endif

/**
    Socket Class
    @description
    @stability Prototype
    @defgroup EjsSocket EjsSocket
    @see EjsSocket ejsCreateSocket ejsIsSocket
 */
typedef struct EjsSocket {
    EjsObj          *emitter;                   /**< Event emitter */
    EjsByteArray    *data;                      /**< Buffered write data */
    MprSocket       *sock;                      /**< Underlying MPR socket object */
    MprWaitHandler  waitHandler;                /**< I/O event wait handler */
    Ejs             *ejs;                       /**< Convenience access to ejs interpreter instance */
    cchar           *address;                   /**< Remote address */
    int             port;                       /**< Remote port */
    int             async;                      /**< In async mode */
    int             mask;                       /**< IO event mask */
    MprMutex        *mutex;                     /**< Multithread sync */
} EjsSocket;

/** 
    Create a new Socket object
    @param ejs Ejs reference returned from #ejsCreate
    @return a new Socket object
    @ingroup EjsSocket
 */
extern EjsSocket *ejsCreateSocket(Ejs *ejs);

#if DOXYGEN
    extern bool ejsIsSocket(Ejs *ejs, EV *ev);
#else
    #define ejsIsSocket(ejs, ev) (ev && TYPE(ev) == ejs->socketType)
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
    Ejs             *ejs;                           /**< Need interpreter reference in callback */
    MprEvent        *event;                         /**< MPR event for the timer */
    int             drift;                          /**< Timer event is allowed to drift if system conditions requrie */
    int             repeat;                         /**< Timer repeatedly fires */
    int             period;                         /**< Time in msec between invocations */          
    EjsFunction     *callback;                      /**< Callback function */
    EjsFunction     *onerror;                       /**< onerror function */
    EjsArray        *args;                          /**< Callback args */
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
    char            *name;                          /**< Optional worker name */
    Ejs             *ejs;                           /**< Interpreter */
    EV              *event;                         /**< Current event object */
    struct EjsWorker *pair;                         /**< Corresponding worker object in other thread */
    char            *scriptFile;                    /**< Script or module to run */
    EjsString       *scriptLiteral;                 /**< Literal script string to run */
    int             state;                          /**< Worker state */
    int             inside;                         /**< Running inside the worker */
    int             complete;                       /**< Worker has completed its work */
    int             gotMessage;                     /**< Worker has received a message */
    int             terminated;                     /**< Worker has had terminate() called */
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

//  MOB -- should not be a type. Should be NULL.
typedef struct EjsVoid {
} EjsVoid;

extern EjsVoid  *ejsCreateUndefined(Ejs *ejs);

#define ejsIsUndefined(ejs, ev) (ejs->undefinedValue == ev)


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
    //  MOB -- is this obj needed?
    EjsObj          obj;                /**< Extends Object - Property storage (unused) */
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
    extern boolean ejsIsXML(Ejs *ejs, EV *ev);
#else
    #define ejsIsXML(ejs, ev) (ev && ((TYPE(ev) == ejs->xmlType) || TYPE(ev) == ejs->xmlListType))
#endif

extern EjsXML *ejsCreateXML(Ejs *ejs, int kind, EjsName name, EjsXML *parent, EjsString *value);
extern void  ejsLoadXMLString(Ejs *ejs, EjsXML *xml, EjsString *xmlString);
extern void  ejsLoadXMLCString(Ejs *ejs, EjsXML *xml, cchar *xmlString);
extern EjsXML *ejsConfigureXML(Ejs *ejs, EjsXML *xml, int kind, EjsString *name, EjsXML *parent, EjsString *value);
extern EjsXML *ejsDeepCopyXML(Ejs *ejs, EjsXML *xml);
extern EjsXML *ejsXMLDescendants(Ejs *ejs, EjsXML *xml, EjsName qname);

/*  
    Xml private prototypes
 */
extern void ejsMarkXML(Ejs *ejs, EjsXML *xml);
extern MprXml *ejsCreateXmlParser(Ejs *ejs, EjsXML *xml, cchar *filename);
extern int ejsXMLToString(Ejs *ejs, MprBuf *buf, EjsXML *xml, int indentLevel);
extern EjsXML *ejsAppendToXML(Ejs *ejs, EjsXML *xml, EjsXML *node);
extern EjsXML *ejsSetXML(Ejs *ejs, EjsXML *xml, int index, EjsXML *node);
extern int ejsAppendAttributeToXML(Ejs *ejs, EjsXML *parent, EjsXML *node);
extern EjsXML *ejsCreateXMLList(Ejs *ejs, EjsXML *targetObject, EjsName targetProperty);


extern int ejsAddObserver(Ejs *ejs, EjsObj **emitterPtr, EjsObj *name, EjsObj *listener);
extern int ejsRemoveObserver(Ejs *ejs, EjsObj *emitter, EjsObj *name, EjsObj *listener);
extern int ejsSendEventv(Ejs *ejs, EjsObj *emitter, cchar *name, EV *thisObj, int argc, EV **argv);
extern int ejsSendEvent(Ejs *ejs, EjsObj *emitter, cchar *name, EV *thisObj, EV *arg);

/************************************ Accessors **************************************/

/** 
    Determine if a variable is a number
    @param obj Object to examine
    @return True if the variable is a number
    @ingroup EjsNumber
 */
#if DOXYGEN
    bool ejsIsNumber(Ejs *ejs, EV *ev);
#else
    #define ejsIsNumber(ejs, obj) (obj && TYPE(obj) == ejs->numberType)
#endif

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to examine
    @return A numeric value
    @ingroup EjsNumber
 */
extern MprNumber ejsGetNumber(Ejs *ejs, EV *ev);

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to examine
    @return An integer value
    @ingroup EjsNumber
 */
extern int ejsGetInt(Ejs *ejs, EV *ev);

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to examine
    @return A double value
    @ingroup EjsNumber
 */
extern double ejsGetDouble(Ejs *ejs, EV *ev);

//  MOB -- rename alloc/free
typedef EV      *(*EjsCreateHelper)(Ejs *ejs, struct EjsType *type, int size);
typedef void    (*EjsDestroyHelper)(Ejs *ejs, EV *ev);
typedef EV      *(*EjsCastHelper)(Ejs *ejs, EV *ev, struct EjsType *type);
typedef EV      *(*EjsCloneHelper)(Ejs *ejs, EV *ev, bool deep);
typedef int     (*EjsDefinePropertyHelper)(Ejs *ejs, EV *ev, int slotNum, EjsName qname, struct EjsType *propType, 
                    int64 attributes, EV *value);
typedef int     (*EjsDeletePropertyHelper)(Ejs *ejs, EV *ev, int slotNum);
typedef int     (*EjsDeletePropertyByNameHelper)(Ejs *ejs, EV *ev, EjsName qname);
typedef EV      *(*EjsGetPropertyHelper)(Ejs *ejs, EV *ev, int slotNum);
typedef EV      *(*EjsGetPropertyByNameHelper)(Ejs *ejs, EV *ev, EjsName qname);
typedef int     (*EjsGetPropertyCountHelper)(Ejs *ejs, EV *ev);
typedef EjsName (*EjsGetPropertyNameHelper)(Ejs *ejs, EV *ev, int slotNum);
typedef int     (*EjsLookupPropertyHelper)(Ejs *ejs, EV *ev, EjsName qname);
typedef EV      *(*EjsInvokeOperatorHelper)(Ejs *ejs, EV *ev, int opCode, EV *rhs);
typedef void    (*EjsMarkHelper)(Ejs *ejs, EV *ev);
typedef int     (*EjsSetPropertyByNameHelper)(Ejs *ejs, EV *ev, EjsName qname, EV *value);
typedef int     (*EjsSetPropertyHelper)(Ejs *ejs, EV *ev, int slotNum, EV *value);
typedef int     (*EjsSetPropertyNameHelper)(Ejs *ejs, EV *ev, int slotNum, EjsName qname);

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
    EjsObj          *prototype;                     /**< Prototype for instances when using prototype inheritance (only) */
    EjsHelpers      helpers;                        /**< Type helper methods */
    struct EjsType  *baseType;                      /**< Base class */

    //  MOB Array?
    MprList         *implements;                    /**< List of implemented interfaces */
        
    uint            callsSuper              :  1;   /**< Constructor calls super() */
    uint            dynamicInstance         :  1;   /**< Object instances may add properties */
    uint            final                   :  1;   /**< Type is final */
    uint            hasBaseConstructors     :  1;   /**< Base types has constructors */
    uint            hasBaseInitializers     :  1;   /**< Base types have initializers */
    uint            hasConstructor          :  1;   /**< Type has a constructor */
    uint            hasInitializer          :  1;   /**< Type has static level initialization code */
    uint            hasInstanceVars         :  1;   /**< Type has non-function instance vars (state) */
    uint            hasMeta                 :  1;   /**< Type has meta methods */
    uint            hasScriptFunctions      :  1;   /**< Block has non-native functions requiring namespaces */
    uint            immutable               :  1;   /**< Instances are immutable */
    uint            initialized             :  1;   /**< Static initializer has run */
    uint            isInterface             :  1;   /**< Interface vs class */
    uint            needFixup               :  1;   /**< Slots need fixup */
    uint            numericIndicies         :  1;   /**< Instances support direct numeric indicies */
    uint            virtualSlots            :  1;   /**< Properties are not stored in slots[] */
    
    //  MOB -- pack with above?
    int             numInherited;                   /**< Number of inherited prototype properties */
    short           id;                             /**< Unique type id */
    ushort          instanceSize;                   /**< Size of instances in bytes */
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
    extern bool ejsIsType(Ejs *ejs, EV *ev);

    /** 
        Determine if a variable is a prototype object. Types store the template for instance properties in a prototype object
        @param obj Object to test
        @return True if the variable is a prototype object.
        @ingroup EjsType
     */
    extern bool ejsIsPrototype(Ejs *ejs, EV *ev);
#else
    #define ejsIsType(ejs, ev)       (ev && ejsIsObject(ejs, ev) && (((EjsObj*) (ev))->isType))
    #define ejsIsPrototype(ejs, ev)  (ev && ejsIsObject(ejs, ev) && (((EjsObj*) (ev))->isPrototype))
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
extern EjsType *ejsCreateType(Ejs *ejs, EjsName name, struct EjsModule *up, EjsType *baseType, EjsObj *prototype,
    int size, int slotNum, int numTypeProp, int numInstanceProp, int64 attributes, void *data);

extern EjsType *ejsConfigureType(Ejs *ejs, EjsType *type, struct EjsModule *up, EjsType *baseType, 
    int numTypeProp, int numInstanceProp, int64 attributes);
extern void ejsCompleteType(Ejs *ejs, EjsType *type);

extern EjsObj *ejsCreatePrototype(Ejs *ejs, EjsType *type, int numProp);
extern EjsType *ejsCreateArchetype(Ejs *ejs, struct EjsFunction *fun, EjsObj *prototype);

/** 
    Define a global function
    @description Define a global public function and bind it to the C native function. This is a simple one liner
        to define a public global function. The more typical paradigm to define functions is to create a script file
        of native method definitions and and compile it. This results in a mod file that can be loaded which will
        create the function/method definitions. Then use #ejsBindMethod to associate a C function with a property.
    @ingroup EjsType
 */
//  XX
extern int ejsDefineGlobalFunction(Ejs *ejs, cchar *name, EjsFun fn);


/** 
    Test if an variable is an instance of a given type
    @description Perform an "is a" test. This tests if a variable is a direct instance or subclass of a given base type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param target Target variable to test.
    @param type Type to compare with the target
    @return True if target is an instance of "type" or an instance of a subclass of "type".
    @ingroup EjsType
 */
extern bool ejsIsA(Ejs *ejs, EV *target, EjsType *type);

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
extern int ejsBindMethod(Ejs *ejs, EV *obj, int slotNum, EV *fn);
extern int ejsBindAccess(Ejs *ejs, EV *obj, int slotNum, EV *getter, EV *setter);
extern void ejsBindConstructor(Ejs *ejs, EjsType *type, EV *nativeProc);

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
    int attributes, EV *value);

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

//  XX
extern EjsType  *ejsGetTypeByName(Ejs *ejs, cchar *space, cchar *name);

#define VSPACE(space) space "-" BLD_VNUM
#define ejsGetVType(ejs, space, name) ejsGetTypeByName(ejs, space "-" BLD_VNUM, name)

extern int      ejsCompactClass(Ejs *ejs, EjsType *type);
extern int      ejsCopyBaseProperties(Ejs *ejs, EjsType *type, EjsType *baseType);
extern void     ejsDefineTypeNamespaces(Ejs *ejs, EjsType *type);
extern int      ejsFixupType(Ejs *ejs, EjsType *type, EjsType *baseType, int makeRoom);
extern int      ejsBlendTypeProperties(Ejs *ejs, EjsType *type, EjsType *typeType);
extern int      ejsGetTypePropertyAttributes(Ejs *ejs, EV *obj, int slot);
extern void     ejsInitializeBlockHelpers(EjsHelpers *helpers);

extern void     ejsSetTypeName(Ejs *ejs, EjsType *type, EjsName qname);
extern void     ejsTypeNeedsFixup(Ejs *ejs, EjsType *type);
extern int      ejsGetTypeSize(Ejs *ejs, EjsType *type);

extern EjsType  *ejsCreateCoreType(Ejs *ejs, EjsName name, EjsType *extendsType, int size, int slotNum, 
    int numTypeProp, int numInstanceProp, int64 attributes);

//  XX
extern EjsType  *ejsCreateNativeType(Ejs *ejs, cchar *space, cchar *name, int id, int size);
extern EjsType  *ejsConfigureNativeType(Ejs *ejs, cchar *space, cchar *name, int size);

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
extern void     ejsCreateStringType(Ejs *ejs);
extern void     ejsCreateTypeType(Ejs *ejs);
extern void     ejsCreateVoidType(Ejs *ejs);
extern void     ejsCreateXMLType(Ejs *ejs);
extern void     ejsCreateXMLListType(Ejs *ejs);

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
extern void     ejsInitializeDefaultHelpers(EjsHelpers *helpers);
extern void     ejsInitializeFunctionHelpers(EjsHelpers *helpers, int all);
extern void     ejsServiceEvents(Ejs *ejs, int timeout, int flags);
extern void     ejsSetSqliteMemCtx(MprThreadLocal *tls, MprCtx ctx);
extern void     ejsSetSqliteTls(MprThreadLocal *tls);

#if BLD_FEATURE_EJS_ALL_IN_ONE || BLD_FEATURE_STATIC
extern int      ejs_events_Init(MprCtx ctx);
extern int      ejs_xml_Init(MprCtx ctx);
extern int      ejs_io_Init(MprCtx ctx);
extern int      ejs_sys_Init(MprCtx ctx);
#if BLD_FEATURE_SQLITE
extern int      ejs_db_sqlite_Init(MprCtx ctx);
#endif
extern int      ejs_web_Init(MprCtx ctx);
#endif

/* 
    Move some ejsWeb.h declarations here so handlers can just include ejs.h whether they are using the
    all-in-one ejs.h or the pure ejs.h
 */
extern HttpStage *ejsAddWebHandler(Http *http);
extern int ejsHostHttpServer(HttpConn *conn);

extern int ejs_db_sqlite_Init(MprCtx ctx);
extern int ejs_web_init(MprCtx ctx);

/**
    VM Evaluation state. 
    The VM Stacks grow forward in memory. A push is done by incrementing first, then storing. ie. *++top = value
    A pop is done by extraction then decrement. ie. value = *top--
    @ingroup EjsVm
 */
typedef struct EjsState {
    struct EjsFrame     *fp;                /* Current Frame function pointer */
    struct EjsBlock     *bp;                /* Current block pointer */
    EV                  **stack;            /* Top of stack (points to the last element pushed) */
    EV                  **stackBase;        /* Pointer to start of stack mem */
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
    EV              *obj;                   /* Final object / Type containing the variable */
    EV              *originalObj;           /* Original object used for the search */
    EV              *ref;                   /* Actual property reference */
    struct EjsTrait *trait;                 /* Property trait describing the property */
    struct EjsName  name;                   /* Name and namespace used to find the property */
    int             bind;                   /* Whether to bind to this lookup */
} EjsLookup;


/******************************** Internal GC API ********************************/

extern int      ejsSetGeneration(Ejs *ejs, int generation);
extern void     ejsAnalyzeGlobal(Ejs *ejs);
extern int      ejsCreateAllocService(Ejs *ejs);
extern int      ejsCreateStringService(Ejs *ejs);
extern int      ejsCreateGCService(Ejs *ejs);
extern void     ejsDestroyGCService(Ejs *ejs);
extern int      ejsIsTimeForGC(Ejs *ejs, int timeTillNextEvent);
extern void     ejsCollectEverything(Ejs *ejs);
extern void     ejsCollectGarbage(Ejs *ejs, int gen);
extern int      ejsEnableGC(Ejs *ejs, bool on);
extern void     ejsGracefulDegrade(Ejs *ejs);
extern void     ejsPrintAllocReport(Ejs *ejs);
extern void     ejsMakeEternalPermanent(Ejs *ejs);
extern void     ejsMakePermanent(Ejs *ejs, void *ptr);
extern void     ejsMakeTransient(Ejs *ejs, void *ptr);

/**
    Ejscript Service structure
    @description The Ejscript service manages the overall language runtime. It 
        is the factory that creates interpreter instances via #ejsCreate.
    @ingroup EjsService
 */
typedef struct EjsService {
    EV              *(*loadScriptLiteral)(Ejs *ejs, EjsString *script, cchar *cache);
    EV              *(*loadScriptFile)(Ejs *ejs, cchar *path, cchar *cache);
    MprHashTable    *nativeModules;
    Http            *http;
    MprMutex        *mutex;             /**< Multithread locking */
} EjsService;

extern EjsService *ejsGetService(MprCtx ctx);
extern int ejsInitCompiler(EjsService *service);
extern void ejsAttention(Ejs *ejs);
extern void ejsClearAttention(Ejs *ejs);

/*********************************** Prototypes *******************************/
/**
    Open the Ejscript service
    @description One Ejscript service object is required per application. From this service, interpreters
        can be created.
    @param ctx Any memory context returned by mprAlloc
    @return An ejs service object
    @ingroup Ejs
 */
extern EjsService *ejsCreateService(MprCtx ctx);

/**
    Create an ejs virtual machine 
    @description Create a virtual machine interpreter object to evalute Ejscript programs. Ejscript supports multiple 
        interpreters. One interpreter can be designated as a master interpreter and then it can be cloned by supplying 
        the master interpreter to this call. A master interpreter provides the standard system types and clone interpreters 
        can quickly be created an utilize the master interpreter's types. This saves memory and speeds initialization.
    @param ctx Any memory context returned by mprAlloc
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
extern Ejs *ejsCreateVm(MprCtx ctx, cchar *search, MprList *require, int argc, cchar **argv, int flags);

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
extern EV *ejsThrowException(Ejs *ejs, EV *error);
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

extern EV *ejsCastOperands(Ejs *ejs, EV *lhs, int opcode, EV *rhs);
extern int ejsCheckModuleLoaded(Ejs *ejs, cchar *name);
extern void ejsClearExiting(Ejs *ejs);
extern EV *ejsCreateException(Ejs *ejs, int slot, cchar *fmt, va_list fmtArgs);
extern EjsArray *ejsGetModuleList(Ejs *ejs);
extern EV *ejsGetVarByName(Ejs *ejs, EV *obj, EjsName name, EjsLookup *lookup);
extern int ejsInitStack(Ejs *ejs);
extern void ejsLog(Ejs *ejs, cchar *fmt, ...);

extern int ejsLookupVar(Ejs *ejs, EV *obj, EjsName name, EjsLookup *lookup);
extern int ejsLookupVarWithNamespaces(Ejs *ejs, EV *obj, EjsName name, EjsLookup *lookup);

extern int ejsLookupScope(Ejs *ejs, EjsName name, EjsLookup *lookup);
extern void ejsMemoryFailure(MprCtx ctx, size_t size, size_t total, bool granted);
extern int ejsRunProgram(Ejs *ejs, cchar *className, cchar *methodName);
extern void ejsSetHandle(Ejs *ejs, void *handle);
extern void ejsShowCurrentScope(Ejs *ejs);
extern void ejsShowStack(Ejs *ejs, EjsFunction *fp);
extern void ejsShowBlockScope(Ejs *ejs, EjsBlock *block);
extern int ejsStartMprLogging(Mpr *mpr, char *logSpec);
extern void ejsCloneDefaultHelpers(Ejs *ejs, EjsType *type);
extern void ejsCloneObjectHelpers(Ejs *ejs, EjsType *type);
extern void ejsCloneBlockHelpers(Ejs *ejs, EjsType *type);
extern int  ejsParseModuleVersion(cchar *name);

extern void ejsLockVm(Ejs *ejs);
extern void ejsUnlockVm(Ejs *ejs);

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

