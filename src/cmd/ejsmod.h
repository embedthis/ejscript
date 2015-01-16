/**
    ejsmod.h - Header for the ejsmod: module file list and slot generation program.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#ifndef _h_EJS_MOD
#define _h_EJS_MOD 1

#include    "ejs.h"

/*********************************** Defines **********************************/
/*
    Listing record structure
 */
typedef struct Lst {
    int         kind;                       /* Record kind */
    EjsModule   *module;                    /* Module holding class, function or property */
    EjsModule   *dependency;                /* Dependant module */
    EjsType     *type;                      /* Class reference */
    EjsObj      *owner;                     /* Owner (type, function, block) for the element */
    EjsFunction *fun;                       /* Relevant function */
    int         numProp;                    /* Number of properties */
    int         slotNum;                    /* Slot number */
    int         attributes;                 /* Property attributes */
    EjsString   *name;                      /* General name (used only for block name) */
    EjsName     typeName;                   /* Property type name */
    EjsName     qname;                      /* Qualified Property name */
} Lst;


/*
    Mod manager control structure
 */
typedef struct EjsMod {
    char        *currentLine;               /* Current input source code line */
    int         currentLineNumber;          /* Current input source line number */
    char        *currentFileName;           /* Current input file name */

    EjsService  *service;                   /* Ejs service manager */
    Ejs         *ejs;                       /* Interpreter handle */
    
    MprList     *lstRecords;                /* Listing records */
    MprList     *packages;                  /* List of packages */
    
    MprList     *blocks;                    /* List of blocks */
    EjsBlock    *currentBlock;              /* Current lexical block being read */

    int         cslots;                     /* Create C slot definitions */
    int         depends;                    /* Print module dependencies */
    int         error;                      /* Unresolved error */
    int         errorCount;                 /* Count of all errors */
    int         exitOnError;                /* Exit if module file errors are detected */
    int         fatalError;                 /* Any a fatal error - Can't continue */
    int         firstGlobal;                /* First global to examine */
    int         listing;                    /* Generate listing file */
    int         memError;                   /* Memory error */
    int         showAsm;                    /* Show assembly bytes */
    int         verbosity;                  /* Verbosity level */
    int         warningCount;               /* Count of all warnings */
    int         warnOnError;                /* Warn if module file errors are detected */

    char        *outputDir;                 /* Directory for slots and listings */
    char        *docDir;                    /* Directory to generate HTML doc */
    bool        html;                       /* Generate HTML doc */
    bool        xml;                        /* Generate XML doc */
    
    char        *path;                      /* Current output file path */
    MprFile     *file;                      /* Current output file handle */
    EjsModule   *module;                    /* Current unit */
    EjsFunction *fun;                       /* Current function to disassemble */
    uchar       *pc;
} EjsMod;


/*
    Image ROM files
 */
typedef struct DocFile {
    cchar           *path;              /* File path */
    uchar           *data;              /* Pointer to file data */
    int             size;               /* Size of file */
    int             inode;              /* Not used */
} DocFile;

extern DocFile docFiles[];

/********************************** Prototypes *******************************/

extern void emListingLoadCallback(Ejs *ejs, int kind, ...);
extern void emnDocLoadCallback(Ejs *ejs, int kind, ...);
extern int  emCreateSlotFiles(EjsMod *mp, EjsModule *up, MprFile *file);
extern int  emCreateDoc(EjsMod *mp);

#endif /* _h_EJS_MOD */

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
