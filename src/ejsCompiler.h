/*
    ejsCompiler.h - Internal compiler header.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#ifndef _h_EC_COMPILER
#define _h_EC_COMPILER 1

#include    "ejs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************** Defines **********************************/
/*
    Compiler validation modes. From "use standard|strict"
 */
//  MOB DOC
#define PRAGMA_MODE_STANDARD    1               /* Standard unstrict mode */
#define PRAGMA_MODE_STRICT      2               /* Strict mode */

//  MOB DOC
#define STRICT_MODE(cp)         (cp->fileState->strict)

/*
    Variable Kind bits
 */
#define KIND_CONST              0x1
#define KIND_VAR                0x2
#define KIND_LET                0x4

/*
    Phases for AST processing
 */
//  MOB DOC
#define EC_PHASE_DEFINE         0           /* Define types, functions and properties in types */
#define EC_PHASE_CONDITIONAL    1           /* Do conditional processing, hoisting and then type fixups */
#define EC_PHASE_FIXUP          2           /* Fixup type references */
#define EC_PHASE_BIND           3           /* Bind var references to slots and property types */
#define EC_AST_PHASES           4

typedef struct EcLocation {
    MprChar     *source;
    char        *filename;
    int         lineNumber;
    int         column;
} EcLocation;

#define N_ARGS                  1
#define N_ASSIGN_OP             2
#define N_ATTRIBUTES            3
#define N_BINARY_OP             4
#define N_BINARY_TYPE_OP        5
#define N_BLOCK                 6
#define N_BREAK                 7
#define N_CALL                  8
#define N_CASE_ELEMENTS         9
#define N_CASE_LABEL            10
#define N_CATCH                 11
#define N_CATCH_ARG             12
#define N_CATCH_CLAUSES         13
#define N_CLASS                 14
#define N_CONTINUE              15
#define N_DASSIGN               16
#define N_DIRECTIVES            17
#define N_DO                    18
#define N_DOT                   19
#define N_END_FUNCTION          20
#define N_EXPRESSIONS           21
#define N_FIELD                 22
#define N_FOR                   23
#define N_FOR_IN                24
#define N_FUNCTION              25
#define N_GOTO                  26
#define N_HASH                  27
#define N_IF                    28
#define N_LITERAL               29
#define N_MODULE                30
#define N_NEW                   31
#define N_NOP                   32
#define N_OBJECT_LITERAL        33
#define N_PARAMETER             34
#define N_POSTFIX_OP            35
#define N_PRAGMA                36
#define N_PRAGMAS               37
#define N_PROGRAM               38
#define N_QNAME                 39
#define N_REF                   40
#define N_RETURN                41
#define N_SPREAD                42
#define N_SUPER                 43
#define N_SWITCH                44
#define N_THIS                  45
#define N_THROW                 46
#define N_TRY                   47
#define N_TYPE_IDENTIFIERS      48
#define N_UNARY_OP              49
#define N_USE_MODULE            50
#define N_USE_NAMESPACE         51
#define N_VALUE                 52
#define N_VAR                   53
#define N_VAR_DEFINITION        54
#define N_VOID                  55
#define N_WITH                  56

/*
    Ast node define
 */
#if !DOXYGEN
typedef struct EcNode   *Node;
#endif

//  MOB DOC
/*
    Structure for code generation buffers
 */
typedef struct EcCodeGen {
//  MOB DOC
    MprBuf      *buf;                           /* Code generation buffer */
    MprList     *jumps;                         /* Break/continues to patch for this code block */
    MprList     *exceptions;                    /* Exception handlers for this code block */
    EjsDebug    *debug;                         /* Source debug info */ 
    int         jumpKinds;                      /* Kinds of jumps allowed */
    int         breakMark;                      /* Stack item counter for the target for break/continue stmts */
    int         blockMark;                      /* Lexical block counter for the target for break/continue stmts */
    int         stackCount;                     /* Current stack item counter */
    int         blockCount;                     /* Current block counter */
    int         lastLineNumber;                 /* Last line for debug */
} EcCodeGen;


//  MOB DOC
typedef struct EcNode {
    char                *kindName;              /* Node kind string */
#if BLD_DEBUG
    char                *tokenName;
#endif
    EjsName             qname;
    EcLocation          loc;                    /* Source code info */
    EjsBlock            *blockRef;              /* Block scope */
    EjsLookup           lookup;                 /* Lookup residuals */
    EjsNamespace        *namespaceRef;          /* Namespace reference */
    Node                left;                   /* children[0] */
    Node                right;                  /* children[1] */
    Node                typeNode;               /* Type of name */
    Node                parent;                 /* Parent node */
    MprList             *namespaces;            /* Namespaces for hoisted variables */
    MprList             *children;

    int                 kind;                   /* Kind of node */
    int                 attributes;             /* Attributes applying to this node */
    int                 tokenId;                /* Lex token */
    int                 groupMask;              /* Token group */
    int                 subId;                  /* Sub token */
    int                 slotNum;                /* Allocated slot for variable */
    int                 jumpLength;             /* Goto length for exceptions */
    int                 seqno;                  /* Unique sequence number */

    uint                blockCreated      : 1;  /* Block object has been created */
    uint                createBlockObject : 1;  /* Create the block object to contain let scope variables */
    uint                enabled           : 1;  /* Node is enabled via conditional definitions */
    int                 literalNamespace  : 1;  /* Namespace is a literal */
    uint                needThis          : 1;  /* Need to push this object */
    uint                needDupObj        : 1;  /* Need to dup the object on stack (before) */
    uint                needDup           : 1;  /* Need to dup the result (after) */
    uint                slotFixed         : 1;  /* Slot fixup has been done */
    uint                specialNamespace  : 1;  /* Using a public|private|protected|internal namespace */

    uchar               *patchAddress;          /* For code patching */
    EcCodeGen           *code;                  /* Code buffer */

    EjsName             *globalProp;            /* Set if this is a global property */
    EjsString           *doc;                   /* Documentation string */

    struct EcCompiler   *cp;                    /* Compiler instance reference */

#if BLD_CC_UNNAMED_UNIONS
    union {
#endif
        struct {
            Node        expression;
            EcCodeGen   *expressionCode;        /* Code buffer for the case expression */
            int         kind;
            int         nextCaseCode;           /* Goto length to the next case statements */
        } caseLabel;

        struct {
            Node        arg;                    /* Catch block argument */
        } catchBlock;

        /*
            Var definitions have one child per variable. Child nodes can be either N_NAME or N_ASSIGN_OP
         */
        struct {
            int         varKind;                /* Variable definition kind */
        } def;

        struct {
            /* Children are the catch clauses */
            Node        tryBlock;               /* Try code */
            Node        catchClauses;           /* Catch clauses */
            Node        finallyBlock;           /* Finally code */
            int         numBlocks;              /* Count of open blocks in the try block */
        } exception;

        struct {
            Node        expr;                   /* Field expression */
            Node        fieldName;              /* Field element name for objects */
            int         fieldKind;              /* value or function */
            int         index;                  /* Array index, set to -1 for objects */
            int         varKind;                /* Variable definition kind (const) */
        } field;

        struct {
            Node        resultType;             /* Function return type */
            Node        body;                   /* Function body */
            Node        parameters;             /* Function formal parameters */
            Node        constructorSettings;    /* Constructor settings */
            EjsFunction *functionVar;           /* Function variable */
            uint        operatorFn    : 1;      /* operator function */
            uint        getter        : 1;      /* getter function */
            uint        setter        : 1;      /* setter function */
            uint        call          : 1;      /* */
            uint        has           : 1;      /* */
            uint        hasRest       : 1;      /* Has rest parameter */
            uint        hasReturn     : 1;      /* Has a return statement */
            uint        isMethod      : 1;      /* Is a class method */
            uint        isConstructor : 1;      /* Is constructor method */
            uint        isDefault     : 1;      /* Is default constructor */
            uint        isExpression  : 1;      /* Is a function expression */
        } function;

        struct {
            Node        iterVar;
            Node        iterGet;
            Node        iterNext;
            Node        body;
            EcCodeGen   *initCode;
            EcCodeGen   *bodyCode;
            int         each;                   /* For each used */
        } forInLoop;

        struct {
            Node        body;
            Node        cond;
            Node        initializer;
            Node        perLoop;
            EcCodeGen   *condCode;
            EcCodeGen   *bodyCode;
            EcCodeGen   *perLoopCode;
        } forLoop;

        struct {
            Node        body;
            Node        expr;
            bool        disabled;
        } hash;

        struct {
            Node         implements;          /* Implemented interfaces */
            Node         constructor;         /* Class constructor */
            MprList      *staticProperties;   /* List of static properties */
            MprList      *instanceProperties; /* Implemented interfaces */
            MprList      *classMethods;       /* Static methods */
            MprList      *methods;            /* Instance methods */
            EjsType      *ref;                /* Type instance ref */
            EjsFunction  *initializer;        /* Initializer function */
            EjsNamespace *publicSpace;
            EjsNamespace *internalSpace;
            EjsString    *extends;            /* Class base class */
            int          isInterface;         /* This is an interface */
        } klass;

        struct {
            EjsObj      *var;               /* Special value */
            MprBuf      *data;              /* XML data */
        } literal;

        struct {
            EjsModule   *ref;               /* Module object */
            EjsString   *name;              /* Module name */
            char        *filename;          /* Module file name */
            int         version;
        } module;

        /*
            Name nodes hold a fully qualified name.
         */
        struct {
            Node        nameExpr;           /* Name expression */
            Node        qualifierExpr;      /* Qualifier expression */
            EjsObj      *nsvalue;           /* Initialization value (MOB - remove) */
            uint        instanceVar  : 1;   /* Instance or static var (if defined in class) */
            uint        isAttribute  : 1;   /* Attribute identifier "@" */
            uint        isDefault    : 1;   /* use default namespace */
            uint        isInternal   : 1;   /* internal namespace */
            uint        isLiteral    : 1;   /* use namespace "literal" */
            uint        isNamespace  : 1;   /* Name is a namespace */
            uint        isRest       : 1;   /* ... rest style args */
            uint        isType       : 1;   /* Name is a type */
            uint        letScope     : 1;   /* Variable is defined in let block scope */
            int         varKind;            /* Variable definition kind */
        } name;

        struct {
            int         callConstructors;   /* Bound type has a constructor */
        } newExpr;

        struct {
            Node        typeNode;           /* Type of object */
            int         isArray;            /* Array literal */
        } objectLiteral;

        struct {
            uint        strict;             /* Strict mode */
        } pragma;

        struct {
            MprList     *dependencies;      /* Accumulated list of dependent modules */
        } program;

        struct {
            Node        node;               /* Actual node reference */
        } ref;

        struct {
            int         blockless;          /* Function expression */
        } ret;

        struct {
            Node        cond;
            Node        thenBlock;
            Node        elseBlock;
            EcCodeGen   *thenCode;
            EcCodeGen   *elseCode;
        } tenary;

        struct {
            int         thisKind;           /* Kind of this. See EC_THIS_ flags */
        } thisNode;

        struct {
            int         minVersion;
            int         maxVersion;
        } useModule;

        struct {
            Node        object;
            Node        statement;
        } with;
#if BLD_CC_UNNAMED_UNIONS
    };
#endif
} EcNode;


/*
    Various per-node flags
 */
#define EC_THIS_GENERATOR       1
#define EC_THIS_CALLEE          2
#define EC_THIS_TYPE            3
#define EC_THIS_FUNCTION        4

#define EC_SWITCH_KIND_CASE     1   /* Case block */
#define EC_SWITCH_KIND_DEFAULT  2   /* Default block */

/*
    Object (and Array) literal field
 */
#define FIELD_KIND_VALUE        0x1
#define FIELD_KIND_FUNCTION     0x2

#define EC_NUM_NODES            8
#define EC_TAB_WIDTH            4

/*
    Fix clash with arpa/nameser.h
 */
#undef T_NULL

/*
    Lexical tokens (must start at 1)
    ASSIGN tokens must be +1 compared to their non-assignment counterparts.
    (Use genTokens to recreate)
    WARNING: ensure T_MOD and T_MOD_ASSIGN are adjacent. rewriteCompoundAssignment relies on this
 */
#define T_ASSIGN                    1
#define T_AT                        2
#define T_ATTRIBUTE                 3
#define T_BIT_AND                   4
#define T_BIT_AND_ASSIGN            5
#define T_BIT_OR                    6
#define T_BIT_OR_ASSIGN             7
#define T_BIT_XOR                   8
#define T_BIT_XOR_ASSIGN            9
#define T_BREAK                    10
#define T_CALL                     11
#define T_CALLEE                   12
#define T_CASE                     13
#define T_CAST                     14
#define T_CATCH                    15
#define T_CDATA_END                16
#define T_CDATA_START              17
#define T_CLASS                    18
#define T_COLON                    19
#define T_COLON_COLON              20
#define T_COMMA                    21
#define T_CONST                    22
#define T_CONTEXT_RESERVED_ID      23
#define T_CONTINUE                 24
#define T_DEBUGGER                 25
#define T_DECREMENT                26
#define T_DEFAULT                  27
#define T_DELETE                   28
#define T_DIV                      29
#define T_DIV_ASSIGN               30
#define T_DO                       31
#define T_DOT                      32
#define T_DOT_DOT                  33
#define T_DOT_LESS                 34
#define T_DOUBLE                   35
#define T_DYNAMIC                  36
#define T_EACH                     37
#define T_ELIPSIS                  38
#define T_ELSE                     39
#define T_ENUMERABLE               40
#define T_EOF                      41
#define T_EQ                       42
#define T_ERR                      43
#define T_EXTENDS                  44
#define T_FALSE                    45
#define T_FINAL                    46
#define T_FINALLY                  47
#define T_FLOAT                    48
#define T_FOR                      49
#define T_FUNCTION                 50
#define T_GE                       51
#define T_GENERATOR                52
#define T_GET                      53
#define T_GOTO                     54
#define T_GT                       55
#define T_HAS                      56
#define T_HASH                     57
#define T_ID                       58
#define T_IF                       59
#define T_IMPLEMENTS               60
#define T_IN                       61
#define T_INCLUDE                  62
#define T_INCREMENT                63
#define T_INSTANCEOF               64
#define T_INT                      65
#define T_INTERFACE                66
#define T_INTERNAL                 67
#define T_INTRINSIC                68
#define T_IS                       69
#define T_LBRACE                   70
#define T_LBRACKET                 71
#define T_LE                       72
#define T_LET                      73
#define T_LOGICAL_AND              74
#define T_LOGICAL_AND_ASSIGN       75
#define T_LOGICAL_NOT              76
#define T_LOGICAL_OR               77
#define T_LOGICAL_OR_ASSIGN        78
#define T_LOGICAL_XOR              79
#define T_LOGICAL_XOR_ASSIGN       80
#define T_LPAREN                   81
#define T_LSH                      82
#define T_LSH_ASSIGN               83
#define T_LT                       84
#define T_LT_SLASH                 85
#define T_MINUS                    86
#define T_MINUS_ASSIGN             87
#define T_MINUS_MINUS              88
#define T_MODULE                   89
#define T_MOD                      90       // WARNING sorted order manually fixed!!
#define T_MOD_ASSIGN               91
#define T_MUL                      92
#define T_MUL_ASSIGN               93
#define T_NAMESPACE                94
#define T_NATIVE                   95
#define T_NE                       96
#define T_NEW                      97
#define T_NOP                      98
#define T_NULL                     99
#define T_NUMBER                  100
#define T_NUMBER_WORD             101
#define T_OVERRIDE                102
#define T_PLUS                    103
#define T_PLUS_ASSIGN             104
#define T_PLUS_PLUS               105
#define T_PRIVATE                 106
#define T_PROTECTED               107
#define T_PROTOTYPE               108
#define T_PUBLIC                  109
#define T_QUERY                   110
#define T_RBRACE                  111
#define T_RBRACKET                112
#define T_REGEXP                  113
#define T_REQUIRE                 114
#define T_RESERVED_NAMESPACE      115
#define T_RETURN                  116
#define T_RPAREN                  117
#define T_RSH                     118
#define T_RSH_ASSIGN              119
#define T_RSH_ZERO                120
#define T_RSH_ZERO_ASSIGN         121
#define T_SEMICOLON               122
#define T_SET                     123
#define T_SLASH_GT                124
#define T_STANDARD                125
#define T_STATIC                  126
#define T_STRICT                  127
#define T_STRICT_EQ               128
#define T_STRICT_NE               129
#define T_STRING                  130
#define T_SUPER                   131
#define T_SWITCH                  132
#define T_THIS                    133
#define T_THROW                   134
#define T_TILDE                   135
#define T_TO                      136
#define T_TRUE                    137
#define T_TRY                     138
#define T_TYPE                    139
#define T_TYPEOF                  140
#define T_UINT                    141
#define T_UNDEFINED               142
#define T_USE                     143
#define T_VAR                     144
#define T_VOID                    145
#define T_WHILE                   146
#define T_WITH                    147
#define T_XML_COMMENT_END         148
#define T_XML_COMMENT_START       149
#define T_XML_PI_END              150
#define T_XML_PI_START            151
#define T_YIELD                   152

/*
    Group masks
 */
#define G_RESERVED          0x1
#define G_CONREV            0x2
#define G_COMPOUND_ASSIGN   0x4                 /* Eg. <<= */
#define G_OPERATOR          0x8                 /* Operator overload*/

/*
    Attributes (including reserved namespaces)
 */
#define A_FINAL         0x1
#define A_OVERRIDE      0x2
#define A_EARLY         0x4                     /* Early binding */
#define A_DYNAMIC       0x8
#define A_NATIVE        0x10
#define A_PROTOTYPE     0x20
#define A_STATIC        0x40
#define A_ENUMERABLE    0x40

#define EC_INPUT_STREAM "__stdin__"

struct EcStream;
typedef int (*EcStreamGet)(struct EcStream *stream);

/*
    Stream flags
 */
#define EC_STREAM_EOL       0x1                 /* End of line */

//  MOB DOC

typedef struct EcStream {
    struct EcCompiler *compiler;                /* Compiler back reference */
    EcLocation  loc;                            /* Source code debug info */
    EcLocation  lastLoc;                        /* Location info for a prior line */
    EcStreamGet getInput;                       /* Get more input callback */
    MprChar     *buf;                           /* Buffer holding source file */
    MprChar     *nextChar;                      /* Ptr to next input char */
    MprChar     *end;                           /* Ptr to one past end of buf */
    bool        eof;                            /* At end of file */
    int         flags;                          /* Input flags */
} EcStream;


/*
    Parse source code from a file
 */
//  MOB DOC
typedef struct EcFileStream {
    EcStream    stream;
    MprFile     *file;
} EcFileStream;


/*
    Parse source code from a memory block
 */
//  MOB DOC
typedef struct EcMemStream {
    EcStream    stream;
} EcMemStream;


/*
    Parse input from the console (or file if using ejsh)
 */
//  MOB DOC
typedef struct EcConsoleStream {
    EcStream    stream;
} EcConsoleStream;


/*
    Program source input tokens
 */
//  MOB DOC
typedef struct EcToken {
    MprChar     *text;                  /* Token text */
    int         length;                 /* Length of text in characters */
    int         size;                   /* Size of text in characters */
    int         tokenId;
    int         subId;
    int         groupMask;
    int         eol;                    /* At the end of the line */
    EcLocation  loc;                    /* Source code debug info */
    struct EcToken *next;               /* Putback and freelist linkage */
    EcStream    *stream;
#if BLD_DEBUG
    char        *name;                  /* Debug token name */
#endif
} EcToken;


/*
    Jump types
 */
#define EC_JUMP_BREAK       0x1
#define EC_JUMP_CONTINUE    0x2
#define EC_JUMP_GOTO        0x4

//  MOB DOC
typedef struct EcJump {
    int             kind;               /* Break, continue */
    int             offset;             /* Code offset to patch */
    EcNode          *node;              /* Owning node */
} EcJump;


/*
    Current parse state. Each non-terminal production has its own state.
    Some state fields are inherited. We keep a linked list from EcCompiler.
 */
//  MOB DOC
typedef struct EcState {
    struct EcState  *next;                  /* State stack */
    uint            blockIsMethod    : 1;   /* Current function is a method */
    uint            captureBreak     : 1;   /* Capture break/continue inside a catch/finally block */
    uint            captureFinally   : 1;   /* Capture break/continue with a finally block */
    uint            conditional      : 1;   /* In branching conditional */
    uint            disabled         : 1;   /* Disable nodes below this scope */
    uint            dupLeft          : 1;   /* Dup left side */
    uint            inClass          : 1;   /* Inside a class declaration */
    uint            inFunction       : 1;   /* Inside a function declaration */
    uint            inHashExpression : 1;   /* Inside a # expression */
    uint            inInterface      : 1;   /* Inside an interface */
    uint            inMethod         : 1;   /* Inside a method declaration */
    uint            inSettings       : 1;   /* Inside constructor settings */
    uint            instanceCode     : 1;   /* Generating instance class code */
    uint            needsValue       : 1;   /* Expression must yield a value */
    uint            noin             : 1;   /* Don't allow "in" */
    uint            onLeft           : 1;   /* On the left of an assignment */
    uint            saveOnLeft       : 1;   /* Saved left of an assignment */
    uint            strict           : 1;   /* Compiler checking mode: Strict, standard*/

    int             blockNestCount;         /* Count of blocks encountered. Used by ejs shell */
    int             namespaceCount;         /* Count of namespaces originally in block. Used to pop namespaces */

    EjsModule       *currentModule;         /* Current open module definition */
    EjsType         *currentClass;          /* Current open class */
    EjsName         currentClassName;       /* Current open class name */
    EcNode          *currentClassNode;      /* Current open class */
    EjsFunction     *currentFunction;       /* Current open method */
    EcNode          *currentFunctionNode;   /* Current open method */
    EcNode          *currentObjectNode;     /* Left object in "." or "[" */
    EcNode          *topVarBlockNode;       /* Top var block node */

    EjsBlock        *letBlock;              /* Block for local block scope declarations */
    EjsBlock        *varBlock;              /* Block for var declarations */
    EjsBlock        *optimizedLetBlock;     /* Optimized let block declarations - may equal ejs->global */
    EcNode          *letBlockNode;          /* Node for the current let block */

    EjsString       *nspace;                /* Namespace for declarations */
    EjsString       *defaultNamespace;      /* Default namespace for new top level declarations. Does not propagate */

    EcCodeGen       *code;                  /* Global and function code buffer */
    EcCodeGen       *staticCodeBuf;         /* Class static level code generation buffer */
    EcCodeGen       *instanceCodeBuf;       /* Class instance level code generation buffer */

    struct EcState  *prevBlockState;        /* Block state stack */
    struct EcState  *breakState;            /* State for breakable blocks */
    struct EcState  *classState;            /* State for current class */
} EcState;


extern int      ecEnterState(struct EcCompiler *cp);
extern void     ecLeaveState(struct EcCompiler *cp);
extern EcNode   *ecLeaveStateWithResult(struct EcCompiler *cp,  struct EcNode *np);
extern int      ecResetModule(struct EcCompiler *cp, struct EcNode *np);
extern void     ecStartBreakableStatement(struct EcCompiler *cp, int kinds);


/*
    Primary compiler control structure
 */
//  MOB DOC
typedef struct EcCompiler {
    /*
        Properties ordered to make debugging easier
     */
    int         phase;                      /* Ast processing phase */
    EcState     *state;                     /* Current state */
    EcToken     *peekToken;                 /* Peek ahead token */
    EcToken     *token;                     /* Current input token */

    /*  Lexer */
    MprHashTable *keywords;
    EcStream    *stream;
    EcToken     *putback;                   /* List of active putback tokens */
    char        *docToken;                  /* Last doc token */

    EcState     *fileState;                 /* Top level state for the file */
//  MOB -- these are risky and should be moved into state. A nested block, directive class etc willl modify
    EcState     *directiveState;            /* State for the current directive - used in parse and CodeGen */
    EcState     *blockState;                /* State for the current block */

    EjsLookup   lookup;                     /* Lookup residuals */
    EjsService  *vmService;                 /* VM runtime */
    Ejs         *ejs;                       /* Interpreter instance */
    MprList     *nodes;                     /* Compiled AST nodes */

    /*
        Compiler command line options
     */
    char        *certFile;                  /* Certificate to sign the module file */
    bool        debug;                      /* Run in debug mode */
    bool        doc;                        /* Include documentation strings in output */
    char        *extraFiles;                /* Extra source files to compile */

    MprList     *require;                   /* Required list of modules to pre-load */
    bool        interactive;                /* Interactive use (ejsh) */
    bool        merge;                      /* Merge all dependent modules */
    bool        bind;                       /* Don't bind properties to slots */
    bool        noout;                      /* Don't generate any module output files */
    bool        visibleGlobals;             /* Make globals visible (no namespace) */
    int         optimizeLevel;              /* Optimization factor (0-9) */
    bool        shbang;                     /* Observe #!/path as the first line of a script */
    int         warnLevel;                  /* Warning level factor (0-9) */

    int         strict;                     /* Compiler default strict mode */
    int         lang;                       /* Language compliance level: ecma|plus|fixed */
    char        *outputDir;                 /* Output directory for modules */
    char        *outputFile;                /* Output module file name override */
    MprFile     *file;                      /* Current output file handle */

    int         modver;                     /* Default module version */
    int         parseOnly;                  /* Only parse the code */
    int         strip;                      /* Strip debug symbols */
    int         tabWidth;                   /* For error reporting "^" */

    MprList     *modules;                   /* List of modules to process */
    MprList     *fixups;                    /* Type reference fixups */

    char        *errorMsg;                  /* Aggregated error messages */
    int         error;                      /* Unresolved parse error */
    int         fatalError;                 /* Any a fatal error - Can't continue */
    int         errorCount;                 /* Count of all errors */
    int         warningCount;               /* Count of all warnings */
    int         nextSeqno;                  /* Node sequence numbers */
    int         blockLevel;                 /* Level of nest in blocks */

    /*
        TODO - aggregate these into flags
     */
    int         lastOpcode;                 /* Last opcode encoded */
    int         uid;                        /* Unique identifier generator */
} EcCompiler;

/********************************** Prototypes *******************************/

//  MOB -- reorder
//  MOB DOC
extern int          ecAddModule(EcCompiler *cp, EjsModule *mp);
extern EcNode       *ecAppendNode(EcNode *np, EcNode *child);
extern int          ecAstFixup(EcCompiler *cp, struct EcNode *np);
extern EcNode       *ecChangeNode(EcCompiler *cp, EcNode *np, EcNode *oldNode, EcNode *newNode);
extern void         ecGenConditionalCode(EcCompiler *cp, EcNode *np, EjsModule *up);
extern int          ecCodeGen(EcCompiler *cp);
extern int          ecCompile(EcCompiler *cp, int argc, char **path);
extern EcCompiler   *ecCreateCompiler(struct Ejs *ejs, int flags);
extern void         ecDestroyCompiler(EcCompiler *cp);
extern void         ecInitLexer(EcCompiler *cp);
extern EcNode       *ecCreateNode(EcCompiler *cp, int kind);
extern void         ecFreeToken(EcCompiler *cp, EcToken *token);
extern char         *ecGetErrorMessage(EcCompiler *cp);
extern EjsString    *ecGetInputStreamName(EcCompiler *lp);
extern int          ecGetToken(EcCompiler *cp);
extern int          ecGetRegExpToken(EcCompiler *cp, MprChar *prefix);
extern EcNode       *ecLinkNode(EcNode *np, EcNode *child);

extern EjsModule    *ecLookupModule(EcCompiler *cp, EjsString *name, int minVersion, int maxVersion);
extern int          ecLookupScope(EcCompiler *cp, EjsName name);
extern int          ecLookupVar(EcCompiler *cp, EjsAny *vp, EjsName name);
extern EcNode       *ecParseWarning(EcCompiler *cp, char *fmt, ...);
extern int          ecPeekToken(EcCompiler *cp);
extern int          ecPutSpecificToken(EcCompiler *cp, EcToken *token);
extern int          ecPutToken(EcCompiler *cp);
extern void         ecError(EcCompiler *cp, cchar *severity, EcLocation *loc, cchar *fmt, ...);
extern void         ecErrorv(EcCompiler *cp, cchar *severity, EcLocation *loc, cchar *fmt, va_list args);
extern void         ecResetInput(EcCompiler *cp);
extern EcNode       *ecResetError(EcCompiler *cp, EcNode *np, bool eatInput);
extern int          ecRemoveModule(EcCompiler *cp, EjsModule *mp);
extern void         ecResetParser(EcCompiler *cp);
extern int          ecResetModuleList(EcCompiler *cp);
extern int          ecOpenConsoleStream(EcCompiler *cp, EcStreamGet gets, cchar *contents);
extern int          ecOpenFileStream(EcCompiler *cp, cchar *path);
extern int          ecOpenMemoryStream(EcCompiler *cp, cchar *contents, ssize len);
extern void         ecCloseStream(EcCompiler *cp);
extern void         ecSetOptimizeLevel(EcCompiler *cp, int level);
extern void         ecSetWarnLevel(EcCompiler *cp, int level);
extern void         ecSetStrictMode(EcCompiler *cp, int on);
extern void         ecSetTabWidth(EcCompiler *cp, int width);
extern void         ecSetOutputDir(EcCompiler *cp, cchar *outputDir);
extern void         ecSetOutputFile(EcCompiler *cp, cchar *outputFile);
extern void         ecSetCertFile(EcCompiler *cp, cchar *certFile);
extern EcToken      *ecTakeToken(EcCompiler *cp);
extern int          ecAstProcess(struct EcCompiler *cp);
extern void         *ecCreateStream(EcCompiler *cp, ssize size, cchar *filename, void *manager);
extern void         ecSetStreamBuf(EcStream *sp, cchar *contents, ssize len);
extern EcNode       *ecParseFile(EcCompiler *cp, char *path);
extern void         ecManageStream(EcStream *sp, int flags);
extern void         ecMarkLocation(EcLocation *loc);
extern void         ecSetRequire(EcCompiler *cp, MprList *modules);


/*
    Module file creation routines.
 */
extern void     ecAddFunctionConstants(EcCompiler *cp, EjsPot *obj, int slotNum);
extern void     ecAddConstants(EcCompiler *cp, EjsAny *obj);
extern int      ecAddStringConstant(EcCompiler *cp, EjsString *sp);
extern int      ecAddCStringConstant(EcCompiler *cp, cchar *str);
extern int      ecAddNameConstant(EcCompiler *cp, EjsName qname);
extern int      ecAddDocConstant(EcCompiler *cp, cchar *tag, void *vp, int slotNum);
extern int      ecAddModuleConstant(EcCompiler *cp, EjsModule *up, cchar *str);
extern int      ecCreateModuleHeader(EcCompiler *cp);
extern int      ecCreateModuleSection(EcCompiler *cp);


/*
    Encoding emitter routines
 */
extern void      ecEncodeBlock(EcCompiler *cp, cuchar *buf, int len);
extern void      ecEncodeByte(EcCompiler *cp, int value);
extern void      ecEncodeByteAtPos(EcCompiler *cp, int offset, int value);
extern void      ecEncodeConst(EcCompiler *cp, EjsString *sp);
extern void      ecEncodeDouble(EcCompiler *cp, double value);
extern void      ecEncodeGlobal(EcCompiler *cp, EjsAny *obj, EjsName qname);
extern void      ecEncodeInt32(EcCompiler *cp, int value);
extern void      ecEncodeInt32AtPos(EcCompiler *cp, int offset, int value);
extern void      ecEncodeNum(EcCompiler *cp, int64 number);
extern void      ecEncodeName(EcCompiler *cp, EjsName qname);
extern void      ecEncodeMulti(EcCompiler *cp, cchar *str);
extern void      ecEncodeWideAsMulti(EcCompiler *cp, MprChar *str);
extern void      ecEncodeOpcode(EcCompiler *cp, int value);

extern void     ecCopyCode(EcCompiler *cp, uchar *pos, int size, int dist);
extern uint     ecGetCodeOffset(EcCompiler *cp);
extern int      ecGetCodeLen(EcCompiler *cp, uchar *mark);
extern void     ecAdjustCodeLength(EcCompiler *cp, int adj);

#ifdef __cplusplus
}
#endif
#endif /* _h_EC_COMPILER */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.

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

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
