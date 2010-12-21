/*
    ejsModule.h - Module file format.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#ifndef _h_EJS_MODULE
#define _h_EJS_MODULE 1

#include    "ejsCore.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************* Prototypes *********************************/
/*
    A module file may contain multiple logical modules.

    Module File Format and Layout:

    (N) Numbers are 1-3 little-endian encoded bytes using the 0x80 as the continuation character
    (S) Strings are pointers into the constant pool encoded as number offsets. Strings are UTF-8.
    (T) Types are encoded and ored with the type encoding masks below. Types are either: untyped, 
        unresolved or primitive (builtin). The relevant mask is ored into the lower 2 bits. Slot numbers and
        name tokens are shifted up 2 bits. Zero means untyped.

    ModuleHeader {
        short       magic
        int         fileVersion
        int         version
        int         flags
    }

    Module {
        byte        section
        string      name
        number      version
        number      checksum
        number      constantPoolLength
        block       constantPool
    }

    Dependencies {
        byte        section
        string      moduleName
        number      minVersion
        number      maxVersion
        number      checksum
        byte        flags
    }

    Type {
        byte        section
        string      typeName
        string      namespace
        number      attributes
        number      slot
        type        baseType
        number      numStaticProperties
        number      numInstanceProperties
        number      numInterfaces
        type        Interfaces ...
        ...
    }

    Property {
        byte        section
        string      name
        string      namespace
        number      attributes
        number      slot
        type        property type
    }

    Function {
        byte        section
        string      name
        string      namespace
        number      nextSlotForSetter
        number      attributes
        byte        languageMode
        type        returnType
        number      slot
        number      argCount
        number      defaultArgCount
        number      localCount
        number      exceptionCount
        number      codeLength
        block       code        
    }

    Exception {
        byte        section
        byte        flags
        number      tryStartOffset
        number      tryEndOffset
        number      handlerStartOffset
        number      handlerEndOffset
        number      numOpenBlocks
        type        catchType
    }

    Debug {
        byte        section
        number      countOfLines
        string      fileName
        number      startLine
        number      addressOffset      
        ...
    }

    Block {
        byte        section
        string      name
        number      slot
        number      propCount
    }

    Documentation {
        byte        section
        string      text
    }
 */

/*
    Type encoding masks
 */
#define EJS_ENCODE_GLOBAL_NOREF         0x0
#define EJS_ENCODE_GLOBAL_NAME          0x1
#define EJS_ENCODE_GLOBAL_SLOT          0x2
#define EJS_ENCODE_GLOBAL_MASK          0x3

/*
    Fixup kinds
 */
#define EJS_FIXUP_BASE_TYPE             1
#define EJS_FIXUP_INTERFACE_TYPE        2
#define EJS_FIXUP_RETURN_TYPE           3
#define EJS_FIXUP_TYPE_PROPERTY         4
#define EJS_FIXUP_INSTANCE_PROPERTY     5
#define EJS_FIXUP_LOCAL                 6
#define EJS_FIXUP_EXCEPTION             7

/*
    Number encoding uses one bit per byte plus a sign bit in the first byte
 */ 
#define EJS_ENCODE_MAX_WORD             0x07FFFFFF

typedef struct EjsTypeFixup
{
    int              kind;                       /* Kind of fixup */
    int              slotNum;                    /* Slot of target */
    EjsObj           *target;                    /* Target to fixup */
    EjsName          typeName;                   /* Type name */
    int              typeSlotNum;                /* Type slot number */
} EjsTypeFixup;


#define EJS_MODULE_MAGIC        0xC7DA

/*
    Module file format version
 */
#define EJS_MODULE_VERSION      3
#define EJS_VERSION_FACTOR      1000

/*
    Module content version
 */
#define EJS_COMPAT_VERSION(v1, v2) ((v1 / EJS_VERSION_FACTOR) == (v2 / EJS_VERSION_FACTOR))
#define EJS_MAKE_COMPAT_VERSION(version) (version / EJS_VERSION_FACTOR * EJS_VERSION_FACTOR)
#define EJS_MAKE_VERSION(maj, min, patch) (((((maj) * EJS_VERSION_FACTOR) + (min)) * EJS_VERSION_FACTOR) + (patch))
#define EJS_MAJOR(version)      (((version / EJS_VERSION_FACTOR) / EJS_VERSION_FACTOR) % EJS_VERSION_FACTOR)
#define EJS_MINOR(version)      ((version / EJS_VERSION_FACTOR) % EJS_VERSION_FACTOR)
#define EJS_PATCH(version)      (version % EJS_VERSION_FACTOR)
#define EJS_MAX_VERSION         EJS_MAKE_VERSION(EJS_VERSION_FACTOR-1, EJS_VERSION_FACTOR-1, EJS_VERSION_FACTOR-1)
#define EJS_VERSION             EJS_MAKE_VERSION(BLD_MAJOR_VERSION, BLD_MINOR_VERSION, BLD_PATCH_VERSION)

/*
    Section types
 */
#define EJS_SECT_MODULE         1           /* Module section */
#define EJS_SECT_MODULE_END     2           /* End of a module */
#define EJS_SECT_DEBUG          3           /* Module dependency */
#define EJS_SECT_DEPENDENCY     4           /* Module dependency */
#define EJS_SECT_CLASS          5           /* Class definition */
#define EJS_SECT_CLASS_END      6           /* End of class definition */
#define EJS_SECT_FUNCTION       7           /* Function */
#define EJS_SECT_FUNCTION_END   8           /* End of function definition */
#define EJS_SECT_BLOCK          9           /* Nested block */
#define EJS_SECT_BLOCK_END      10          /* End of Nested block */
#define EJS_SECT_PROPERTY       11          /* Property (variable) definition */
#define EJS_SECT_EXCEPTION      12          /* Exception definition */
#define EJS_SECT_DOC            13          /* Documentation for an element */
#define EJS_SECT_MAX            14

/*
    Psudo section types for loader callback
 */
#define EJS_SECT_START          (EJS_SECT_MAX + 1)
#define EJS_SECT_END            (EJS_SECT_MAX + 2)

/*
    Align headers on a 4 byte boundary
 */
#define EJS_HDR_ALIGN           4

/*
    File format is little-endian. All headers are aligned on word boundaries.
 */
typedef struct EjsModuleHdr {
    int32       magic;                      /* Magic number for Ejscript modules */
    int32       fileVersion;                /* Module file format version */
    int32       flags;                      /* Module flags */
} EjsModuleHdr;


typedef struct EjsModule {
    EjsString       *name;                  /* Name of this module */
    EjsString       *vname;                 /* Versioned name */
    char            *path;                  /* Module file path name */
    int             version;                /* Made with EJS_MAKE_VERSION */
    int             minVersion;             /* Minimum version when used as a dependency */
    int             maxVersion;             /* Maximum version when used as a dependency */
    int             checksum;               /* Checksum of slots and names */

    EjsLoadState    *loadState;             /* State while loading */
    MprList         *dependencies;          /* Module file dependencies. List of EjsModules */
    MprFile         *file;                  /* File handle for loading and code generation */

    /*
        Used during code generation
        MOB - move to separate struct
     */
    struct EcCodeGen *code;                 /* Code generation buffer */
    MprList         *globalProperties;      /* List of global properties */
    EjsFunction     *initializer;           /* Initializer method */

    /*
        Used only while loading modules
        MOB move into separate struct
     */
    MprList         *current;               /* Current stack of open objects */
    EjsBlock        *scope;                 /* Lexical scope chain */
    EjsConstants    *constants;             /* Constant pool */
    int             nameToken;              /* */
    int             firstGlobal;            /* First global property */
    int             lastGlobal;             /* Last global property + 1*/
    struct EjsFunction *currentMethod;      /* Current method being loaded */

    uint            compiling       : 1;    /* Module currently being compiled from source */
    uint            configured      : 1;    /* Module types have been configured with native code */

    //  MOB -- used to test if a module should be compiled
    uint            loaded          : 1;    /* Module has been loaded from an external file */
    uint            nativeLoaded    : 1;    /* Backing shared library loaded */
    uint            hasNative       : 1;    /* Has native property definitions */
    uint            hasInitializer  : 1;    /* Has initializer function */
    uint            initialized     : 1;    /* Initializer has run */
    uint            hasError        : 1;    /* Module has a loader error */
    uint            visited         : 1;    /* Module has been traversed */
    int             flags;                  /* Loading flags */
    EjsString       *doc;                   /* Current doc string */
} EjsModule;


typedef int (*EjsNativeCallback)(Ejs *ejs);

typedef struct EjsNativeModule {
    EjsNativeCallback callback;             /* Callback to configure module native types and properties */
    EjsString       *name;                  /* Module name */
    int             checksum;               /* Checksum expected by native code */
    int             flags;                  /* Configuration flags */
} EjsNativeModule;

/*
    Documentation string information
    Element documentation string. The loader will create if required.
 */
typedef struct EjsDoc {
    EjsString   *docString;                         /* Original doc string */
    MprChar     *brief;
    MprChar     *description;
    MprChar     *example;
    MprChar     *requires;
    MprChar     *returns;
    MprChar     *stability;                         /* prototype, evolving, stable, mature, deprecated */
    MprChar     *spec;                              /* Where specified */
    struct EjsDoc *duplicate;                       /* From @duplicate directive */
    MprList     *defaults;                          /* Parameter default values */
    MprList     *params;                            /* Function parameters */
    MprList     *options;                           /* Option parameter values */
    MprList     *events;                            /* Option parameter values */
    MprList     *see;
    MprList     *throws;
    EjsTrait    *trait;                             /* Back pointer to trait */
    int         deprecated;                         /* Hide doc if true */
    int         hide;                               /* Hide doc if true */
    int         cracked;                            /* Doc has been cracked and tokenized */
} EjsDoc;


/*
    Loader flags
 */
#define EJS_LOADER_STRICT     0x1
#define EJS_LOADER_NO_INIT    0x2
#define EJS_LOADER_ETERNAL    0x4                   /* Make all types eternal */
#define EJS_LOADER_BUILTIN    0x8                   /* Loading builtins */
#define EJS_LOADER_DEP        0x10                  /* Loading a dependency */

/******************************** Prototypes **********************************/

extern int          ejsAddNativeModule(Ejs *ejs, EjsString *name, EjsNativeCallback callback, int checksum, int flags);
extern EjsNativeModule *ejsLookupNativeModule(Ejs *ejs, EjsString *name);
extern EjsModule    *ejsCreateModule(Ejs *ejs, EjsString *name, int version, EjsConstants *constants);

//  MOB -- would this be better with an ascii name?
extern int          ejsLoadModule(Ejs *ejs, EjsString *name, int minVer, int maxVer, int flags);
extern char         *ejsSearchForModule(Ejs *ejs, cchar *name, int minVer, int maxVer);
extern int          ejsSetModuleConstants(Ejs *ejs, EjsModule *mp, EjsConstants *constants);

extern void         ejsModuleReadBlock(Ejs *ejs, EjsModule *module, char *buf, int len);
extern int          ejsModuleReadByte(Ejs *ejs, EjsModule *module);
extern EjsString    *ejsModuleReadConst(Ejs *ejs, EjsModule *module);
extern int          ejsModuleReadInt(Ejs *ejs, EjsModule *module);
extern int          ejsModuleReadInt32(Ejs *ejs, EjsModule *module);
extern EjsName      ejsModuleReadName(Ejs *ejs, EjsModule *module);
extern int64        ejsModuleReadNum(Ejs *ejs, EjsModule *module);
extern char         *ejsModuleReadMulti(Ejs *ejs, EjsModule *mp);
extern MprChar      *ejsModuleReadMultiAsWide(Ejs *ejs, EjsModule *mp);
extern int          ejsModuleReadType(Ejs *ejs, EjsModule *module, EjsType **typeRef, EjsTypeFixup **fixup, 
                        EjsName *typeName, int *slotNum);

extern double       ejsDecodeDouble(Ejs *ejs, uchar **pp);
extern int          ejsDecodeInt32(Ejs *ejs, uchar **pp);
extern int64        ejsDecodeNum(Ejs *ejs, uchar **pp);

extern int          ejsEncodeByteAtPos(Ejs *ejs, uchar *pos, int value);
extern int          ejsEncodeDouble(Ejs *ejs, uchar *pos, double number);
extern int          ejsEncodeInt32(Ejs *ejs, uchar *pos, int number);
extern int          ejsEncodeNum(Ejs *ejs, uchar *pos, int64 number);
extern int          ejsEncodeInt32AtPos(Ejs *ejs, uchar *pos, int value);

extern double       ejsSwapDouble(Ejs *ejs, double a);
extern int          ejsSwapInt32(Ejs *ejs, int word);
extern int64        ejsSwapInt64(Ejs *ejs, int64 word);

extern char         *ejsGetDocKey(Ejs *ejs, EjsBlock *block, int slotNum, char *buf, int bufsize);
extern EjsDoc       *ejsCreateDoc(Ejs *ejs, void *vp, int slotNum, EjsString *docString);

extern int          ejsAddModule(Ejs *ejs, EjsModule *up);
extern EjsModule    *ejsLookupModule(Ejs *ejs, EjsString *name, int minVersion, int maxVersion);
extern int          ejsRemoveModule(Ejs *ejs, EjsModule *up);

#ifdef __cplusplus
}
#endif
#endif /* _h_EJS_MODULE */

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

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=8 ts=8 expandtab

    @end
 */
