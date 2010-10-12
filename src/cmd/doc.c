/**
    doc.c - Documentation generator

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*
    Supported documentation keywords and format.

    The element "one liner" is the first sentance. Rest of description can continue from here and can include embedded html.

    @param argName Description          (Up to next @, case matters on argName)
    @default argName DefaultValue       (Up to next @, case matters on argName)
    @return Sentance                    (Can use return or returns. If sentance starts with lower case, then start 
                                            sentance with "Call returns".
    @event eventName Description        (Up to next @, case matters on eventName)
    @option argName Description         (Up to next @, case matters on argName)
    @throws ExceptionType Explanation   (Up to next @)
    @see Keyword keyword ...            (Case matters)
    @example Description                (Up to next @)
    @stability kind                     (prototype | evolving | stable | mature | deprecated]
    @deprecated version                 Same as @stability deprecated
    @requires ECMA                      (Emit: configuration requires --ejs-ecma)
    @spec                               (ecma-262, ecma-357, ejs-11)
    @hide                               (Hides this entry)
 */

/********************************** Includes **********************************/

#include    "ejsmod.h"

/*********************************** Locals ***********************************/
/*
    Structures used when sorting lists
 */
typedef struct FunRec {
    EjsName         qname;
    EjsFunction     *fun;
    EjsObj          *obj;
    int             slotNum;
    EjsObj          *owner;
    EjsName         ownerName;
    EjsTrait        *trait;
} FunRec;

typedef struct ClassRec {
    EjsName         qname;
    EjsBlock        *block;
    int             slotNum;
    EjsTrait        *trait;
} ClassRec;

typedef struct PropRec {
    EjsName         qname;
    EjsObj          *obj;
    int             slotNum;
    EjsTrait        *trait;
    EjsObj          *vp;
} PropRec;

typedef struct List {
    char        *name;
    MprList     *list;
} List;

/**************************** Forward Declarations ****************************/

static void     addUniqueItem(MprList *list, cchar *item);
static void     addUniqueClass(MprList *list, ClassRec *item);
static MprList  *buildClassList(EjsMod *mp, cchar *namespace);
static void     buildMethodList(EjsMod *mp, MprList *methods, EjsObj *obj, EjsObj *owner, EjsName *ownerName);
static int      compareClasses(ClassRec **c1, ClassRec **c2);
static int      compareFunctions(FunRec **f1, FunRec **f2);
static int      compareProperties(PropRec **p1, PropRec **p2);
static int      compareNames(char **q1, char **q2);
static EjsDoc   *crackDoc(EjsMod *mp, EjsDoc *doc, EjsName *qname);
static MprFile  *createFile(EjsMod *mp, char *name);
static cchar    *demangle(cchar *name);
static void     fixupDoc(Ejs *ejs, EjsDoc *doc);
static char     *fmtAccessors(int attributes);
static char     *fmtAttributes(int attributes, int showAccessors);
static char     *fmtClassUrl(MprCtx ctx, EjsName qname);
static char     *fmtDeclaration(MprCtx ctx, EjsName qname);
static char     *fmtNamespace(EjsName qname);
static char     *fmtSpace(MprCtx ctx, EjsName qname);
static char     *fmtType(MprCtx ctx, EjsName qname);
static char     *fmtTypeReference(MprCtx ctx, EjsName qname);
static char     *fmtModule(cchar *name);
static char     *formatExample(Ejs *ejs, char *example);
static int      generateMethodTable(EjsMod *mp, MprList *methods, EjsObj *obj);
static void     generateClassPage(EjsMod *mp, EjsObj *obj, EjsName *name, EjsTrait *trait, EjsDoc *doc);
static void     generateClassPages(EjsMod *mp);
static void     generateClassPageHeader(EjsMod *mp, EjsObj *obj, EjsName *name, EjsTrait *trait, EjsDoc *doc);
static int      generateClassPropertyTableEntries(EjsMod *mp, EjsObj *obj, int numInhertied);
static int      generateClassGetterTableEntries(EjsMod *mp, EjsObj *obj, int numInherited);
static void     generateClassList(EjsMod *mp, cchar *namespace);
static void     generateContentFooter(EjsMod *mp);
static void     generateContentHeader(EjsMod *mp, cchar *fmt, ... );
static void     generateHomeFrameset(EjsMod *mp);
static void     generateHomeNavigation(EjsMod *mp) ;
static void     generateHomePages(EjsMod *mp);
static void     generateHomeTitle(EjsMod *mp);
static void     generateHtmlFooter(EjsMod *mp);
static void     generateHtmlHeader(EjsMod *mp, cchar *script, cchar *title, ... );
static void     generateImages(EjsMod *mp);
static void     generateOverview(EjsMod *mp);
static void     generateMethod(EjsMod *mp, FunRec *fp);
static void     generateMethodDetail(EjsMod *mp, MprList *methods);
static void     generateNamespace(EjsMod *mp, cchar *namespace);
static void     generateNamespaceClassTable(EjsMod *mp, cchar *namespace);
static int      generateNamespaceClassTableEntries(EjsMod *mp, cchar *namespace);
static void     generateNamespaceList(EjsMod *mp);
static void     generatePropertyTable(EjsMod *mp, EjsObj *obj);
static cchar    *getDefault(EjsDoc *doc, cchar *key);
static EjsDoc   *getDoc(Ejs *ejs, void *block, int slotNum);
static EjsDoc   *getDuplicateDoc(Ejs *ejs, cchar *duplicate);
static void     getKeyValue(char *str, char **key, char **value);
static char     *getFilename(cchar *name);
static int      getPropertyCount(Ejs *ejs, EjsObj *obj);
static bool     match(cchar *last, cchar *key);
static void     prepDocStrings(EjsMod *mp, EjsObj *obj, EjsName *name, EjsTrait *trait, EjsDoc *doc);
static void     out(EjsMod *mp, char *fmt, ...);
static char     *skipAtWord(char *str);

/*********************************** Code *************************************/

int emCreateDoc(EjsMod *mp)
{
    Ejs     *ejs;

    ejs = mp->ejs;

    if (ejs->doc == 0) {
        ejs->doc = mprCreateHash(ejs, EJS_DOC_HASH_SIZE, 0);
        if (ejs->doc == 0) {
            return MPR_ERR_NO_MEMORY;
        }
    }
    generateImages(mp);
    generateClassPages(mp);
    generateHomePages(mp);
    return 0;
}


static void generateImages(EjsMod *mp)
{
    DocFile     *df;
    MprFile     *file;
    char        *path;
    int         rc;

    for (df = docFiles; df->path; df++) {
        path = mprJoinPath(mp, mp->docDir, df->path);
        rc = mprMakeDir(mp, mprGetPathDir(mp, path), 0775, 1);
        file = mprOpen(mp, path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644);
        if (file == 0) {
            mprError(mp, "Can't create %s", path);
            mp->errorCount++;
            mprFree(path);
            return;
        }
        if (mprWrite(file, df->data, df->size) != df->size) {
            mprError(mp->file, "Can't write to buffer");
            mp->errorCount++;
            mprFree(path);
            return;
        }
        mprFree(file);
        mprFree(path);
    }
}


static void generateHomePages(EjsMod *mp)
{
    generateHomeFrameset(mp);
    generateHomeTitle(mp);
    generateHomeNavigation(mp);
    generateNamespaceList(mp);
    generateOverview(mp);
}


static void generateHomeFrameset(EjsMod *mp)
{
    cchar   *script;

    mprFree(mp->file);
    mp->file = createFile(mp, "index.html");
    if (mp->file == 0) {
        return;
    }

    script = "function loaded() { content.location.href = '__overview-page.html'; }";
    generateHtmlHeader(mp, script, "Home");

    out(mp, "<frameset rows='90,*' border='0' onload='loaded()'>\n");
    out(mp, "   <frame src='title.html' name='title' scrolling='no' frameborder='0'>\n");
    out(mp, "   <frameset cols='200,*' border='2' framespacing='0'>\n");
    out(mp, "       <frame src='__navigation-left.html' name='navigation' scrolling='auto' frameborder='1'>\n");
    out(mp, "       <frame src='__overview-page.html' name='content' scrolling='auto' frameborder='1'>\n");
    out(mp, "   </frameset>\n");
    out(mp, "  <noframes><body><p>Please use a frames capable client to view this documentation.</p></body></noframes>");
    out(mp, "</frameset>\n");
    out(mp, "</html>\n");

    mprFree(mp->file);
    mp->file = 0;
}


static void generateHomeTitle(EjsMod *mp)
{
    mprFree(mp->file);
    mp->file = createFile(mp, "title.html");
    if (mp->file == 0) {
        return;
    }
    generateHtmlHeader(mp, NULL, "title");

    out(mp,
        "<body>\n"
        "<div class=\"body\">\n"
        "   <div class=\"top\">\n"
        "       <map name=\"home\" id=\"home\">\n"
        "           <area coords=\"5,15,200,150\" href=\"index.html\" alt=\"doc\"/>\n"
        "       </map>\n"
        "   <div class=\"version\">%s %s</div>\n"
        "   <div class=\"menu\">\n"
        "       <a href=\"http://www.ejscript.org/\" target=\"_top\">Ejscript Home</a>\n"
        "       &gt; <a href=\"index.html\" class=\"menu\" target=\"_top\">Documentation Home</a>\n"
        "   </div>\n"
        "   <div class=\"search\">\n"
        "       <form class=\"smallText\" action=\"search.php\" method=\"post\" name=\"searchForm\" id=\"searchForm\"></form>&nbsp;\n"
        "       <input class=\"smallText\" type=\"text\" name=\"search\" align=\"right\" id=\"searchInput\" size=\"15\" \n"
        "           maxlength=\"50\" value=\"Search\"/>\n"
        "   </div>\n"
        "</div>\n", BLD_NAME, BLD_VERSION);

    generateHtmlFooter(mp);

    mprFree(mp->file);
    mp->file = 0;
}


static void generateHomeNavigation(EjsMod *mp)
{
    mprFree(mp->file);
    mp->file = createFile(mp, "__navigation-left.html");
    if (mp->file == 0) {
        return;
    }

    generateHtmlHeader(mp, NULL, "Navigation");

    out(mp, "<frameset rows='34%%,*' border='1' framespacing='1'>\n");
    out(mp, "  <frame src='__all-namespaces.html' name='namespaces' scrolling='yes' />\n");
    out(mp, "  <frame src='__all-classes.html' name='classes' scrolling='yes' />\n");
    out(mp, "  <noframes><body><p>Please use a frames capable client to view this documentation.</p></body></noframes>");
    out(mp, "</frameset>\n");
    out(mp, "</html>\n");

    mprFree(mp->file);
    mp->file = 0;
}


static void generateNamespaceList(EjsMod *mp)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait;
    EjsName         qname;
    EjsDoc          *doc;
    MprList         *namespaces;
    cchar           *namespace;
    int             count, slotNum, next;

    ejs = mp->ejs;

    mp->file = createFile(mp, "__all-namespaces.html");
    if (mp->file == 0) {
        mp->errorCount++;
        return;
    }
    generateHtmlHeader(mp, NULL, "Namespaces");

    out(mp, "<body>\n");
    out(mp, "<div class='navigation'>\n");
    out(mp, "<h3>Namespaces</h3>\n");
    out(mp, "<table class='navigation' summary='namespaces'>\n");

    /*
        Build a sorted list of namespaces used by classes
     */
    namespaces = mprCreateList(mp);

    count = ejsGetPropertyCount(ejs, ejs->global);
    for (slotNum = 0; slotNum < count; slotNum++) {
        trait = ejsGetTrait(ejs, ejs->global, slotNum);
        if (trait == 0) {
            continue;
        }
        type = ejsGetProperty(ejs, ejs->global, slotNum);
        qname = ejsGetPropertyName(ejs, ejs->global, slotNum);
        if (type == 0 || !ejsIsType(type) || qname.name == 0 || strstr(qname.space, "internal-") != 0) {
            continue;
        }
        doc = getDoc(ejs, ejs->globalBlock, slotNum);
        if (doc && !doc->hide) {
            addUniqueItem(namespaces, fmtNamespace(qname));
        }
    }
    mprSortList(namespaces, (MprListCompareProc) compareNames);

    out(mp, "<tr><td><a href='__all-classes.html' target='classes'>All Namespaces</a></td></tr>\n");

    for (next = 0; (namespace = (cchar*) mprGetNextItem(namespaces, &next)) != 0; ) {
        out(mp, "<tr><td><a href='%s-classes.html' target='classes'>%s</a></td></tr>\n", namespace, namespace);
    }

    out(mp, "</table>\n");
    out(mp, "</div>\n");

    generateHtmlFooter(mp);
    mprFree(mp->file);
    mp->file = 0;

    /*
        Generate namespace overviews and class list files for each namespace
     */
    for (next = 0; (namespace = (cchar*) mprGetNextItem(namespaces, &next)) != 0; ) {
        generateNamespace(mp, namespace);
    }
    generateNamespace(mp, "__all");
    mprFree(namespaces);
}


static void generateNamespace(EjsMod *mp, cchar *namespace)
{
    Ejs         *ejs;
    char        *path;

    ejs = mp->ejs;

    path = mprStrcat(mp, -1, namespace, ".html", NULL);
    mp->file = createFile(mp, path);
    mprFree(path);
    if (mp->file == 0) {
        mp->errorCount++;
        return;
    }
    if (strcmp(namespace, "__all") == 0) {
        generateContentHeader(mp, "All Namespaces");
        generateNamespaceClassTable(mp, namespace);
    } else {
        generateContentHeader(mp, "Namespace %s", namespace);
        generateNamespaceClassTable(mp, namespace);
    }
    generateContentFooter(mp);
    mprFree(mp->file);
    mp->file = 0;

    /*
        Generate an overview page
     */
    generateClassList(mp, namespace);
}


static void generateNamespaceClassTable(EjsMod *mp, cchar *namespace)
{
    Ejs         *ejs;
    int         count;

    ejs = mp->ejs;

    out(mp, "<a name='Classes'></a>\n");

    if (strcmp(namespace, "__all") == 0) {
        out(mp, "<h2 class='classSection'>All Classes</h2>\n");
    } else {
        out(mp, "<h2 class='classSection'>%s Classes</h2>\n", namespace);
    }

    out(mp, "<table class='itemTable' summary='classes'>\n");
    out(mp, "   <tr><th>Class</th><th width='95%%'>Description</th></tr>\n");

    count = generateNamespaceClassTableEntries(mp, namespace);

    if (count == 0) {
        out(mp, "   <tr><td colspan='4'>No properties defined</td></tr>");
    }
    out(mp, "</table>\n\n");
}


/*
    Table of classes in the namespace overview page
 */
static int generateNamespaceClassTableEntries(EjsMod *mp, cchar *namespace)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsDoc          *doc;
    ClassRec        *crec;
    MprList         *classes;
    char            *fmtName;
    int             next, count;

    ejs = mp->ejs;

    classes = buildClassList(mp, namespace);

    for (next = 0; (crec = (ClassRec*) mprGetNextItem(classes, &next)) != 0; ) {
        qname = crec->qname;
        trait = crec->trait;
        fmtName = fmtType(mp, crec->qname);
        out(mp, "   <tr><td><a href='%s' target='content'>%s</a></td>", getFilename(fmtName), qname.name);
        if (crec->block == ejs->globalBlock && mp->firstGlobal == ejsGetPropertyCount(ejs, ejs->global)) {
            continue;
        }
        doc = getDoc(ejs, crec->block ? crec->block : ejs->globalBlock, crec->slotNum);
        if (doc && !doc->hide) {
            out(mp, "<td>%s</td></tr>\n", doc->brief);
        } else {
            out(mp, "<td>&nbsp;</td></tr>\n");
        }
    }
    count = mprGetListCount(classes);
    mprFree(classes);
    return count;
}


static MprList *buildClassList(EjsMod *mp, cchar *namespace)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait;
    EjsDoc          *doc;
    EjsName         qname, gname;
    ClassRec        *crec;
    MprList         *classes;
    int             count, slotNum;

    ejs = mp->ejs;

    /*
        Build a sorted list of classes
     */
    classes = mprCreateList(mp);

    count = ejsGetPropertyCount(ejs, ejs->global);
    for (slotNum = 0; slotNum < count; slotNum++) {
        trait = ejsGetTrait(ejs, ejs->global, slotNum);
        if (trait == 0) {
            continue;
        }
        doc = getDoc(ejs, ejs->globalBlock, slotNum);
        if (doc == 0 || doc->hide) {
            continue;
        }
        type = ejsGetProperty(ejs, ejs->global, slotNum);
        qname = ejsGetPropertyName(ejs, ejs->global, slotNum);
        if (type == 0 || !ejsIsType(type) || qname.name == 0) {
            continue;
        }
        if (strcmp(namespace, "__all") != 0 && strcmp(namespace, fmtNamespace(qname)) != 0) {
            continue;
        }

        /* Suppress the core language types (should not appear as classes) */

        if (strcmp(qname.space, EJS_EJS_NAMESPACE) == 0) {
            if (strcmp(qname.name, "int") == 0 || strcmp(qname.name, "long") == 0 || strcmp(qname.name, "decimal") == 0 ||
                strcmp(qname.name, "boolean") == 0 || strcmp(qname.name, "double") == 0 || 
                strcmp(qname.name, "string") == 0) {
                continue;
            }
        }
        /* Other fixups */
        if (strncmp(qname.space, "internal", 8) == 0 || strcmp(qname.space, "private") == 0) {
            continue;
        }
        crec = (ClassRec*) mprAllocCtx(classes, sizeof(ClassRec));
        crec->qname = qname;
        crec->trait = trait;
        crec->block = ejs->globalBlock;
        crec->slotNum = slotNum;
        addUniqueClass(classes, crec);
    }

    /*
        Add a special type "Global"
     */
    if (strcmp(namespace, "__all") == 0) {
        if (mp->firstGlobal < ejsGetPropertyCount(ejs, ejs->global)) {
            crec = (ClassRec*) mprAllocCtx(classes, sizeof(ClassRec));
            crec->qname.name = EJS_GLOBAL;
            crec->qname.space = EJS_EJS_NAMESPACE;
            crec->block = ejs->globalBlock;
            crec->slotNum = ejsLookupProperty(ejs, ejs->global, ejsName(&gname, EJS_EJS_NAMESPACE, EJS_GLOBAL));
            addUniqueClass(classes, crec);
        }
    }
    mprSortList(classes, (MprListCompareProc) compareClasses);
    return classes;
}


static void generateClassList(EjsMod *mp, cchar *namespace)
{
    Ejs         *ejs;
    MprList     *classes;
    ClassRec    *crec;
    cchar       *className, *fmtName;
    char        *path, script[MPR_MAX_STRING], *cp;
    int         next;

    ejs = mp->ejs;

    path = mprStrcat(mp, -1, namespace, "-classes.html", NULL);
    mp->file = createFile(mp, path);
    mprFree(path);
    if (mp->file == 0) {
        mp->errorCount++;
        return;
    }

    /*
        Create the header and auto-load a namespace overview. We do this here because the class list is loaded
        when the user selects a namespace.
     */
    mprSprintf(mp, script, sizeof(script), "parent.parent.content.location = \'%s.html\';", namespace);
    generateHtmlHeader(mp, script, "%s Class List", namespace);

    out(mp, "<body>\n");
    out(mp, "<div class='navigation'>\n");

    if (strcmp(namespace, "__all") == 0) {
        out(mp, "<h3>All Classes</h3>\n");
    } else {
        out(mp, "<h3>%s Classes</h3>\n", namespace);
    }
    out(mp, "<table class='navigation' summary='classList'>\n");

    classes = buildClassList(mp, namespace);

    for (next = 0; (crec = (ClassRec*) mprGetNextItem(classes, &next)) != 0; ) {
        /*
            Strip namespace portion
         */
        fmtName = fmtType(mp, crec->qname);
        if ((cp = strrchr(fmtName, '.')) != 0) {
            className = ++cp;
        } else {
            className = fmtName;
        }
        if ((cp = strrchr(className, ':')) != 0) {
            className = ++cp;
        }
        out(mp, "<tr><td><a href='%s' target='content'>%s</a></td></tr>\n", getFilename(fmtName), className);
    }

    out(mp, "</table>\n");
    out(mp, "</div>\n");
    out(mp, "&nbsp;<br/>");

    generateHtmlFooter(mp);
    mprFree(mp->file);
    mp->file = 0;
    mprFree(classes);
}


static void generateOverview(EjsMod *mp)
{
    mprFree(mp->file);
    mp->file = createFile(mp, "__overview-page.html");
    if (mp->file == 0) {
        mp->errorCount++;
        return;
    }
    generateContentHeader(mp, "Overview");

    out(mp, "<h1>%s %s</h1>", BLD_NAME, BLD_VERSION);
    out(mp, "<p>Embedthis Ejscript is an implementation of the Javascript (ECMA 262) language.</p>");
    out(mp, "<p>See <a href='http://www.ejscript.org' target='new'>http://www.ejscript.org</a> for "
        "product details and downloads.</p>");
    out(mp, "<h2>Documentation Conventions</h2>");
    out(mp, "<p>APIs are grouped into Namespaces for logical ordering. Within each namespace, classes, methods "
        "and properties are defined. For each method parameters, events and options are described.</p>");
    out(mp, "<h4>Default Values</h4>");
    out(mp, "<p>Method parameters can take default values if an actual parameter is not provided when calling the API. "
        "The default value is listed in the method signature in the form \"name: Type = defaultValue\". The default "
        "value is also listed in the <em>Parameters</em> section.</p>");

    generateContentFooter(mp);

    mprFree(mp->file);
    mp->file = 0;
}


static void generateHtmlHeader(EjsMod *mp, cchar *script, cchar *fmt, ... )
{
    char        *title;
    va_list     args;

    va_start(args, fmt);
    title = mprVasprintf(mp, -1, fmt, args);
    va_end(args);

    /*
        Header + Style sheet
     */
    out(mp, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
    out(mp, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
    out(mp, "<head>\n   <title>%s</title>\n\n", title);

    out(mp, "   <link rel=\"stylesheet\" type=\"text/css\" href=\"doc.css\" />\n");

    if (script) {
        out(mp, "   <script type=\"text/javascript\">\n      %s\n   </script>\n", script);
    }
    out(mp, "</head>\n\n");
}


static void generateContentHeader(EjsMod *mp, cchar *fmt, ... )
{
    va_list     args;
    char        *title;

    va_start(args, fmt);
    title = mprVasprintf(mp, -1, fmt, args);
    va_end(args);

    generateHtmlHeader(mp, NULL, title);
    mprFree(title);
    
    out(mp, "<body>\n<div class='body'>\n\n");
    out(mp, "<div class=\"content\">\n\n");
}


static void generateTerms(EjsMod *mp)
{
    out(mp,
        "<div class=\"terms\">\n"
        "   <p class=\"terms\">\n"
        "       <a href=\"http://www.embedthis.com/\">"
        "       Embedthis Software LLC, 2003-2010. All rights reserved. "
        "Embedthis is a trademark of Embedthis Software LLC.</a>\n"
        "   </p>\n"
        "</div>");
}


static void generateHtmlFooter(EjsMod *mp)
{
    out(mp, "</body>\n</html>\n");
}


static void generateContentFooter(EjsMod *mp)
{
    generateTerms(mp);
    out(mp, "</div>\n");
    out(mp, "</div>\n");
    generateHtmlFooter(mp);
}


static MprFile *createFile(EjsMod *mp, char *name)
{
    MprFile *file;
    char    *path;

    mprFree(mp->path);
    path = mp->path = mprJoinPath(mp, mp->docDir, name);
    file = mprOpen(mp, path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == 0) {
        mprError(mp, "Can't open %s", path);
        mp->errorCount++;
        mprFree(path);
        return 0;
    }
    return file;
}


/*
    Generate one page per class/type
 */
static void generateClassPages(EjsMod *mp)
{
    Ejs         *ejs;
    EjsType     *type;
    EjsTrait    *trait;
    EjsDoc      *doc;
    EjsName     qname;
    char        key[32];
    int         count, slotNum;

    ejs = mp->ejs;

    count = ejsGetPropertyCount(ejs, ejs->global);
    for (slotNum = mp->firstGlobal; slotNum < count; slotNum++) {
        type = ejsGetProperty(ejs, ejs->global, slotNum);
        qname = ejsGetPropertyName(ejs, ejs->global, slotNum);
        if (type == 0 || !ejsIsType(type) || qname.name == 0 || strstr(qname.space, "internal-") != 0) {
            continue;
        }
        /*
            Setup the output path, but create the file on demand when output is done
         */
        mprFree(mp->file);
        mp->file = 0;
        mp->path = mprJoinPath(mp, mp->docDir, getFilename(fmtType(mp, type->qname)));
        if (mp->path == 0) {
            return;
        }
        trait = ejsGetTrait(ejs, ejs->global, slotNum);
        doc = getDoc(ejs, ejs->globalBlock, slotNum);
        if (doc && !doc->hide) {
            generateClassPage(mp, (EjsObj*) type, &qname, trait, doc);
        }
        mprFree(mp->file);
        mp->file = 0;
    }

    /*
        Finally do one page specially for "global"
        TODO - Functionalize
     */
    trait = mprAllocCtx(mp, sizeof(EjsTrait));
    doc = mprAllocCtx(mp, sizeof(EjsDoc));
    doc->docString = (char*) mprStrdup(doc, "Global object containing all global functions and variables.");
    doc->returns = doc->example = doc->description = "";
    doc->trait = trait;

    mprSprintf(mp, key, sizeof(key), "%Lx %d", PTOL(0), 0);
    mprAddHash(ejs->doc, key, doc);

    slotNum = ejsGetPropertyCount(ejs, ejs->global);

    ejsName(&qname, EJS_EJS_NAMESPACE, EJS_GLOBAL);
    mp->file = createFile(mp, getFilename(fmtType(mp, qname)));
    if (mp->file == 0) {
        return;
    }
#if UNUSED
    type = ejsCreateType(ejs, &qname, NULL, NULL, NULL, sizeof(EjsType), slotNum, ejs->globalBlock->obj.numSlots, 
        0, 0, NULL);
    type->constructor.block = *ejs->globalBlock;
    type->constructor.block.obj.type = ejs->typeType;
    type->constructor.block.obj.isType = 1;
    type->constructor.block.isGlobal = 1;
#endif

    generateClassPage(mp, ejs->global, &qname, trait, doc);

    mprFree(mp->file);
    mp->file = 0;
    mprFree(trait);
    mprFree(doc);
}


static void generateClassPage(EjsMod *mp, EjsObj *obj, EjsName *name, EjsTrait *trait, EjsDoc *doc)
{
    MprList     *methods;
    int         count;

    prepDocStrings(mp, obj, name, trait, doc);
    if (doc->hide) {
        return;
    }
    generateClassPageHeader(mp, obj, name, trait, doc);
    generatePropertyTable(mp, obj);

    methods = mprCreateList(mp);
    buildMethodList(mp, methods, obj, obj, name);
    if (ejsIsType(obj)) {
        buildMethodList(mp, methods, ((EjsType*) obj)->prototype, obj, name);
    }
    count = generateMethodTable(mp, methods, obj);
    if (count > 0) {
        generateMethodDetail(mp, methods);
    }
    generateContentFooter(mp);
    mprFree(methods);
}


static void prepDocStrings(EjsMod *mp, EjsObj *obj, EjsName *qname, EjsTrait *typeTrait, EjsDoc *doc)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait;
    EjsObj          *prototype;
    EjsName         pname;
    EjsDoc          *dp;
    char            *combined;
    int             slotNum, numInherited;

    ejs = mp->ejs;

    if (doc) {
        crackDoc(mp, doc, qname);
    }
    type = ejsIsType(obj) ? (EjsType*) obj : 0;
#if UNUSED
    numInherited = type ? type->numInherited : 0;
#endif

    /*
        Loop over all the static properties
     */
    for (slotNum = 0; slotNum < obj->numSlots; slotNum++) {
        trait = ejsGetTrait(ejs, obj, slotNum);
        if (trait == 0) {
            continue;
        }
#if UNUSED
        if (slotNum < numInherited) {
            fun = ejsGetProperty(ejs, obj, slotNum);
            if (fun && ejsIsFunction(fun) && fun->owner != obj) {
                /* Inherited function */
                continue;
            }
        }
#endif
        dp = getDoc(ejs, obj, slotNum);
        if (dp) {
            pname = ejsGetPropertyName(ejs, obj, slotNum);
            combined = mprAsprintf(mp, -1, "%s.%s", qname->name, pname.name);
            crackDoc(mp, dp, ejsName(&pname, "", combined)); 
        }
    }

    /*
        Loop over all the instance properties
     */
    if (type) {
        prototype = type->prototype;
        if (prototype) {
            numInherited = type->numInherited;
            if (ejsGetPropertyCount(ejs, prototype) > 0) {
                for (slotNum = numInherited; slotNum < prototype->numSlots; slotNum++) {
                    trait = ejsGetTrait(ejs, prototype, slotNum);
                    if (trait == 0) {
                        continue;
                    }
                    doc = getDoc(ejs, prototype, slotNum);
                    if (doc) {
                        pname = ejsGetPropertyName(ejs, (EjsObj*) prototype, slotNum);
                        crackDoc(mp, doc, qname);
                    }
                }
            }
        }
    }
}


static void generateClassPageHeader(EjsMod *mp, EjsObj *obj, EjsName *qname, EjsTrait *trait, EjsDoc *doc)
{
    Ejs         *ejs;
    EjsType     *t, *type;
    cchar       *see, *namespace, *module;
    int         next, count;

    ejs = mp->ejs;

    mprAssert(ejsIsBlock(obj));

    if (!ejsIsType(obj)) {
        generateContentHeader(mp, "Global Functions and Variables");
        out(mp, "<a name='top'></a>\n");
        out(mp, "<h1 class='className'>Global Functions and Variables</h1>\n");
    } else {
        generateContentHeader(mp, "Class %s", qname->name);
        out(mp, "<a name='top'></a>\n");
        out(mp, "<h1 class='className'>%s</h1>\n", qname->name);
    }
    out(mp, "<div class='classBlock'>\n");

    if (ejsIsType(obj)) {
        type = (EjsType*) obj;
        out(mp, "<table class='classHead' summary='%s'>\n", qname->name);

        module = (type && type->module) ? fmtModule(type->module->name) : "";
        if (*module) {
            out(mp, "   <tr><td><strong>Module</strong></td><td>%s</td></tr>\n", module);
        }
        namespace = fmtNamespace(*qname);
        out(mp, "   <tr><td><strong>Definition</strong></td><td>%s class %s</td></tr>\n", 
            fmtAttributes(trait->attributes, 1), qname->name);

        if (type && type->baseType) {
            out(mp, "   <tr><td><strong>Inheritance</strong></td><td>%s", qname->name);
            for (t = type->baseType; t; t = t->baseType) {
                out(mp, " <img src='images/inherit.gif' alt='inherit'/> %s", fmtTypeReference(mp, t->qname));
            }
        }

        if (doc) {
            if (*doc->requires) {
                out(mp, "<tr><td><strong>Requires</strong></td><td>configure --ejs-%s</td></tr>\n", doc->requires);
            }
            if (*doc->spec) {
                out(mp, "<tr><td><strong>Specified</strong></td><td>%s</td></tr>\n", doc->spec);
            }
            if (*doc->stability) {
                out(mp, "<tr><td><strong>Stability</strong></td><td>%s</td></tr>\n", doc->stability);
            }
            if (*doc->example) {
                out(mp, "<tr><td><strong>Example</strong></td><td><pre>%s</pre></td></tr>\n", doc->example);
            }
        }
        out(mp, "       </td></tr>\n");
        out(mp, "</table>\n\n");
    }
    if (doc) {
        out(mp, "<p class='classBrief'>%s</p>\n\n", doc->brief);
        out(mp, "<p class='classDescription'>%s</p>\n\n", doc->description);

        count = mprGetListCount(doc->see);
        if (count > 0) {
            out(mp, "<h3>See Also</h3><p class='detail'>\n");
            for (next = 0; (see = mprGetNextItem(doc->see, &next)) != 0; ) {
                out(mp, "<a href='%s'>%s</a>%s\n", getFilename(see), see, (count == next) ? "" : ", ");
            }
            out(mp, "</p>\n");
        }
    }
    out(mp, "</div>\n\n\n");
    out(mp, "<hr />\n");
}


static int getPropertyCount(Ejs *ejs, EjsObj *obj)
{
    EjsObj      *vp, *prototype;
    EjsTrait    *trait;
    EjsType     *type;
    int         limit, count, slotNum;

    count = 0;

    limit = ejsGetPropertyCount(ejs, obj);
    for (slotNum = 0; slotNum < limit; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        if (vp) {
            trait = ejsGetTrait(ejs, obj, slotNum);
            if (trait && trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) {
                count++;
            } else if (!ejsIsFunction(vp)) {
                count++;
            }
        }
    }
    if (ejsIsType(obj)) {
        type = (EjsType*) obj;
        if (type->prototype) {
            prototype = type->prototype;
            limit = ejsGetPropertyCount(ejs, prototype);
            for (slotNum = 0; slotNum < limit; slotNum++) {
                vp = ejsGetProperty(ejs, prototype, slotNum);
                if (vp && !ejsIsFunction(vp)) {
                    count++;
                }
            }
        }
    }
    return count;
}


static void generatePropertyTable(EjsMod *mp, EjsObj *obj)
{
    Ejs     *ejs;
    EjsType *type;
    int     count;

    ejs = mp->ejs;

    out(mp, "<a name='Properties'></a>\n");
    out(mp, "<h2 class='classSection'>Properties</h2>\n");

    out(mp, "<table class='itemTable' summary='properties'>\n");
    out(mp, "   <tr><th>Qualifiers</th><th>Property</th><th>Type</th><th width='95%%'>Description</th></tr>\n");

    count = generateClassPropertyTableEntries(mp, obj, 0);
    count += generateClassGetterTableEntries(mp, obj, 0);

    type = 0;
    if (ejsIsType(obj)) {
        type = (EjsType*) obj;
        if (type->prototype) {
            count += generateClassPropertyTableEntries(mp, type->prototype, type->numInherited);
            count += generateClassGetterTableEntries(mp, type->prototype, type->numInherited);
        }
    }
    if (count == 0) {
        out(mp, "   <tr><td colspan='4'>No properties defined</td></tr>");
    }
    out(mp, "</table>\n\n");

    if (type && type->baseType) {
        count = getPropertyCount(ejs, (EjsObj*) type->baseType);
        if (count > 0) {
            out(mp, "<p class='inheritedLink'><a href='%s#Properties'><i>Inherited Properties</i></a></p>\n\n",
                fmtClassUrl(type, type->baseType->qname));
        }
    }
    out(mp, "<hr />\n");
}


/*
    Generate the entries for class properties. Will be called once for static properties and once for instance properties
 */
static MprList *buildPropertyList(EjsMod *mp, EjsObj *obj, int numInherited)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsDoc          *doc;
    PropRec         *prec;
    MprList         *list;
    int             start, slotNum;

    ejs = mp->ejs;

    list = mprCreateList(mp);

    /*
        Loop over all the (non-inherited) properties
     */
    start = numInherited;
    if (obj == ejs->global) {
        start = mp->firstGlobal;
    }
    for (slotNum = start; slotNum < obj->numSlots; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        trait = ejsGetTrait(ejs, obj, slotNum);
        if (trait) {
            doc = getDoc(ejs, obj, slotNum);
            if (doc && doc->hide) {
                continue;
            }
        }
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        if (vp == 0 || ejsIsFunction(vp) || ejsIsType(vp) || qname.name == 0 || trait == 0) {
            continue;
        }
        if (strcmp(qname.space, EJS_PRIVATE_NAMESPACE) == 0 || strstr(qname.space, ",private]") != 0) {
            continue;
        }
        prec = mprAllocCtx(list, sizeof(PropRec));
        prec->qname = qname;
        prec->obj = obj;
        prec->slotNum = slotNum;
        prec->trait = trait;
        prec->vp = vp;
        mprAddItem(list, prec);
    }
    mprSortList(list, (MprListCompareProc) compareProperties);
    return list;
}


static MprList *buildGetterList(EjsMod *mp, EjsObj *obj, int numInherited)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsDoc          *doc;
    PropRec         *prec;
    MprList         *list;
    int             slotNum;

    ejs = mp->ejs;

    list = mprCreateList(mp);

    /*
        Loop over all the (non-inherited) properties
     */
    if (obj == ejs->global) {
        slotNum = mp->firstGlobal;
    } else {
        slotNum = numInherited;
    }
    for (; slotNum < obj->numSlots; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        if (!ejsIsFunction(vp)) {
            continue;
        }
        trait = ejsGetTrait(ejs, obj, slotNum);
        if (trait) {
            if (!(trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER))) {
                continue;
            }
            doc = getDoc(ejs, obj, slotNum);
            if (doc && doc->hide) {
                continue;
            }
        }
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        if (vp == 0 || ejsIsType(vp) || qname.name == 0 || trait == 0) {
            continue;
        }
        if (strcmp(qname.space, EJS_PRIVATE_NAMESPACE) == 0 || strstr(qname.space, ",private]") != 0) {
            continue;
        }
        prec = mprAllocCtx(list, sizeof(PropRec));
        prec->qname = qname;
        prec->obj = obj;
        prec->slotNum = slotNum;
        prec->trait = trait;
        prec->vp = vp;
        mprAddItem(list, prec);
    }
    mprSortList(list, (MprListCompareProc) compareProperties);
    return list;
}


/*
    Generate the entries for class properties. Will be called once for static properties and once for instance properties
 */
static int generateClassPropertyTableEntries(EjsMod *mp, EjsObj *obj, int numInherited)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsDoc          *doc;
    MprList         *properties;
    PropRec         *prec;
    int             slotNum, count, next, attributes;

    ejs = mp->ejs;
    count = 0;

    properties = buildPropertyList(mp, obj, numInherited);
    type = ejsIsType(obj) ? (EjsType*) obj : 0;

    for (next = 0; (prec = (PropRec*) mprGetNextItem(properties, &next)) != 0; ) {
        vp = prec->vp;
        trait = prec->trait;
        slotNum = prec->slotNum;
        qname = prec->qname;
        if (strncmp(qname.space, "internal", 8) == 0 || strcmp(qname.space, "private") == 0) {
            continue;
        }
        if (isalpha((int) qname.name[0])) {
            out(mp, "<a name='%s'></a>\n", qname.name);
        }
        attributes = trait->attributes;
        if (type && strcmp(qname.space, type->qname.space) == 0) {
            out(mp, "   <tr><td nowrap align='center'>%s</td><td>%s</td>", 
                fmtAttributes(attributes, 1), qname.name);
        } else {
            out(mp, "   <tr><td nowrap align='center'>%s %s</td><td>%s</td>", fmtNamespace(qname),
                fmtAttributes(attributes, 1), qname.name);
        }
        if (trait->type) {
            out(mp, "<td>%s</td>", fmtTypeReference(mp, trait->type->qname));
        } else {
            out(mp, "<td>&nbsp;</td>");
        }
        doc = getDoc(ejs, prec->obj, prec->slotNum);
        if (doc) {
            out(mp, "<td>%s %s</td></tr>\n", doc->brief, doc->description);
        } else {
            out(mp, "<td>&nbsp;</td></tr>\n");
        }
        count++;
    }
    return count;
}


static int generateClassGetterTableEntries(EjsMod *mp, EjsObj *obj, int numInherited)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsFunction     *fun;
    EjsDoc          *doc;
    MprList         *getters;
    PropRec         *prec;
    cchar           *name, *set, *tname;
    int             slotNum, count, next;

    ejs = mp->ejs;
    count = 0;
    getters  = buildGetterList(mp, obj, numInherited);

    for (next = 0; (prec = (PropRec*) mprGetNextItem(getters, &next)) != 0; ) {
        fun = (EjsFunction*) prec->vp;

        trait = prec->trait;
        slotNum = prec->slotNum;
        qname = prec->qname;

        set = "";
        if (trait->attributes & EJS_TRAIT_SETTER) {
            if (trait->attributes & EJS_TRAIT_GETTER) {
                continue;
            }
        }

        if (strncmp(qname.space, "internal", 8) == 0 || strcmp(qname.space, "private") == 0) {
            continue;
        }
        name = qname.name;
        if (strncmp(name, "set-", 4) == 0) {
            name += 4;
        }
        if (isalpha((int) name[0])) {
            out(mp, "<a name='%s'></a>\n", name);
        }
#if KEEP
        if (strcmp(qname.space, type->qname.space) == 0) {
            out(mp, "   <tr><td nowrap align='left'>%s%s</td><td>%s</td>", fmtAttributes(trait->attributes, 1), set, name);
        } else {
#endif
            out(mp, "   <tr><td nowrap align='left'>%s %s%s</td><td>%s</td>", fmtNamespace(qname),
                fmtAttributes(trait->attributes, 1), set, name);
#if KEEP
        }
#endif
        if (fun->resultType) {
            tname = fmtType(mp, fun->resultType->qname);
            if (mprStrcmpAnyCase(tname, "intrinsic::Void") == 0) {
                out(mp, "<td>&nbsp;</td>");
            } else {
                out(mp, "<td>%s</td>", fmtTypeReference(mp, fun->resultType->qname));
            }
        } else {
            out(mp, "<td>&nbsp;</td>");
        }
        doc = getDoc(ejs, prec->obj, prec->slotNum);
        if (doc) {
            out(mp, "<td>%s %s</td></tr>\n", doc->brief, doc->description);
        } else {
            out(mp, "<td>&nbsp;</td></tr>\n");
        }
        count++;
    }
    return count;
}


static void buildMethodList(EjsMod *mp, MprList *methods, EjsObj *obj, EjsObj *owner, EjsName *ownerName)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsFunction     *fun;
    EjsDoc          *doc;
    FunRec          *fp;
    int             slotNum, count;

    ejs = mp->ejs;

    if (obj == ejs->global) {
        slotNum = mp->firstGlobal;
    } else {
        slotNum = 0;
    }
    count = 0;
    for (count = 0; slotNum < obj->numSlots; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        trait = ejsGetTrait(ejs, obj, slotNum);

        fun = (EjsFunction*) vp;
        if (trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) {
            continue;
        }
        if (trait) {
            doc = getDoc(ejs, obj, slotNum);
            if (doc && doc->hide) {
                continue;
            }
        }
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        if (vp == 0 || !ejsIsFunction(vp) || qname.name == 0 || trait == 0) {
            continue;
        }
        if (strcmp(qname.space, EJS_INIT_NAMESPACE) == 0) {
            continue;
        }
        if (strcmp(qname.space, EJS_PRIVATE_NAMESPACE) == 0 || strstr(qname.space, ",private]") != 0) {
            continue;
        }
        if (strncmp(qname.space, "internal", 8) == 0) {
            continue;
        }
#if UNUSED && KEEP
        fun = (EjsFunction*) vp;
        if (slotNum < ((EjsType*) obj)->numInherited) {
            if (fun->owner != obj) {
                /* Inherited function */
                continue;
            }
        }
#endif
        fp = mprAllocCtx(methods, sizeof(FunRec));
        fp->fun = fun;
        fp->obj = obj;
        fp->slotNum = slotNum;
        fp->owner = owner;
        fp->ownerName = *ownerName;
        fp->qname = qname;
        fp->trait = trait;
        mprAddItem(methods, fp);
    }
    mprSortList(methods, (MprListCompareProc) compareFunctions);
}


static int generateMethodTable(EjsMod *mp, MprList *methods, EjsObj *obj)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait, *argTrait;
    EjsName         qname, argName;
    EjsDoc          *doc;
    EjsFunction     *fun;
    FunRec          *fp;
    cchar           *defaultValue;
    int             i, count, next;

    ejs = mp->ejs;
    type = ejsIsType(obj) ? ((EjsType*) obj) : 0;

    out(mp, "<a name='Methods'></a>\n");
    out(mp, "<h2 class='classSection'>%s Methods</h2>\n", (type) ? type->qname.name : "Global");
    out(mp, "<table class='apiIndex' summary='methods'>\n");
    out(mp, "   <tr><th>Qualifiers</th><th width='95%%'>Method</th></tr>\n");

    /*
        Output each method
     */
    count = 0;
    for (next = 0; (fp = (FunRec*) mprGetNextItem(methods, &next)) != 0; ) {
        qname = fp->qname;
        trait = fp->trait;
        fun = fp->fun;

        if (strcmp(qname.space, EJS_INIT_NAMESPACE) == 0) {
            continue;
        }

        if (type && strcmp(qname.space, type->qname.space) == 0) {
            out(mp, "   <tr class='apiDef'><td class='apiType'>%s</td>", fmtAttributes(trait->attributes, 1));
        } else {
            out(mp, "   <tr class='apiDef'><td class='apiType'>%s %s</td>", fmtNamespace(qname), 
                fmtAttributes(trait->attributes, 1));
        }
        out(mp, "<td><a href='#%s'><b>%s</b></a>(", qname.name, demangle(qname.name));

        doc = getDoc(ejs, fp->obj, fp->slotNum);

        for (i = 0; i < (int) fun->numArgs; ) {
            argName = ejsGetPropertyName(ejs, fun->activation, i);
            argTrait = ejsGetTrait(ejs, fun->activation, i);
            if (argTrait->type) {
                out(mp, "%s: %s", fmtDeclaration(mp, argName), fmtTypeReference(mp, argTrait->type->qname));
            } else {
                out(mp, "%s", fmtDeclaration(mp, argName));
            }
            if (doc) {
                defaultValue = getDefault(doc, argName.name);
                if (defaultValue) {
                    out(mp, " = %s", defaultValue);
                }
            }
            if (++i < (int) fun->numArgs) {
                out(mp, ", ");
            }
        }
        out(mp, ")");

        if (fun->resultType) {
            out(mp, ": %s", fmtTypeReference(mp, fun->resultType->qname));
        }
        out(mp, "</tr>");

        if (doc) {
            out(mp, "<tr class='apiBrief'><td>&nbsp;</td><td>%s</td></tr>\n", doc->brief);
        }
        count++;
    }
    if (count == 0) {
        out(mp, "   <tr><td colspan='2'>No methods defined</td></tr>\n");
    }
    out(mp, "</table>\n\n");
    if (type && type->baseType) {
        out(mp, "<p class='inheritedLink'><a href='%s#Methods'><i>Inherited Methods</i></a></p>\n\n",
            fmtClassUrl(type, type->baseType->qname));
    }
    out(mp, "<hr />\n");
    return count;
}


static void generateMethodDetail(EjsMod *mp, MprList *methods)
{
    Ejs         *ejs;
    FunRec      *fp;
    int         next;

    ejs = mp->ejs;
    out(mp, "<h2>Method Detail</h2>\n");

    for (next = 0; (fp = (FunRec*) mprGetNextItem(methods, &next)) != 0; ) {
        generateMethod(mp, fp);
    }
}


static void checkArgs(EjsMod *mp, Ejs *ejs, EjsName *ownerName, EjsFunction *fun, EjsName *qname, EjsDoc *doc)
{
    EjsName         argName;
    MprKeyValue     *param;
    cchar           *key;
    int             i, next;

    for (i = 0; i < (int) fun->numArgs; i++) {
        argName = ejsGetPropertyName(ejs, fun->activation, i);
        for (next = 0; (param = mprGetNextItem(doc->params, &next)) != 0; ) {
            key = param->key;
            if (strncmp(key, "...", 3) == 0) {
                key += 3;
            }
            if (strcmp(key, argName.name) == 0) {
                break;
            }
        }
        if (param == 0) { 
            if (mp->warnOnError) {
                mprError(ejs, "Missing documentation for parameter \"%s\" in function \"%s\" in type \"%s\"", 
                     argName.name, qname->name, ownerName->name);
            }
        }
    }
}


static int findArg(Ejs *ejs, EjsFunction *fun, cchar *name)
{
    EjsName     argName;
    int         i;

    if (strncmp(name, "...", 3) == 0) {
        name += 3;
    }
    for (i = 0; i < (int) fun->numArgs; i++) {
        argName = ejsGetPropertyName(ejs, fun->activation, i);
        if (argName.name && strcmp(argName.name, name) == 0) {
            return i;
        }
    }
    return EJS_ERR;
}


/*
    Lookup to see if there is doc about a default value for a parameter
 */
static cchar *getDefault(EjsDoc *doc, cchar *key)
{
    MprKeyValue     *def;
    int             next;

    for (next = 0; (def = mprGetNextItem(doc->defaults, &next)) != 0; ) {
        if (strcmp(def->key, key) == 0) {
            return def->value;
        }
    }
    return 0;
}


static void generateMethod(EjsMod *mp, FunRec *fp)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait, *argTrait;
    EjsName         qname, argName, throwName;
    EjsFunction     *fun;
    EjsObj          *obj;
    EjsDoc          *doc;
    EjsLookup       lookup;
    MprKeyValue     *param, *thrown, *option, *event;
    cchar           *defaultValue, *accessorSep, *spaceSep;
    char            *see;
    int             i, count, next, numInherited, slotNum;

    ejs = mp->ejs;
    obj = fp->obj;
    slotNum = fp->slotNum;

    type = ejsIsType(obj) ? (EjsType*) obj : 0;
    fun = (EjsFunction*) ejsGetProperty(ejs, obj, slotNum);

    numInherited = 0;
    if (ejsIsPrototype(obj)) {
        mprAssert(ejsIsType(fp->owner));
        numInherited = ((EjsType*) fp->owner)->numInherited;
    }
    mprAssert(ejsIsFunction(fun));

    qname = ejsGetPropertyName(ejs, obj, slotNum);
    trait = ejsGetTrait(ejs, obj, slotNum);
    
    doc = getDoc(ejs, obj, slotNum);
    if (doc && doc->hide) {
        return;
    }
    if (doc == 0 || doc->brief == 0 || *doc->brief == '\0') {
        /* One extra to not warn about default constructors */
        if (slotNum >= numInherited + 1) {
            if (mp->warnOnError) {
                mprError(mp, "Missing documentation for \"%s.%s\"", fp->ownerName.name, qname.name);
            }
        }
        return;
    }

    if (isalpha((int) qname.name[0])) {
        out(mp, "<a name='%s'></a>\n", qname.name);
    }

    if (type && strcmp(qname.space, type->qname.space) == 0) {
        out(mp, "<div class='api'>\n");
        out(mp, "<div class='apiSig'>%s %s(", fmtAttributes(trait->attributes, 1), qname.name);

    } else {
        accessorSep = (trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) ? " ": "";
        spaceSep = qname.space[0] ? " ": "";
        out(mp, "<div class='api'>\n");
        out(mp, "<div class='apiSig'>%s %s%s %s%s %s(", fmtAttributes(trait->attributes, 0), spaceSep, fmtSpace(mp, qname), 
            accessorSep, fmtAccessors(trait->attributes), demangle(qname.name));
    }

    for (i = 0; i < (int) fun->numArgs;) {
        argName = ejsGetPropertyName(ejs, fun->activation, i);
        argTrait = ejsGetTrait(ejs, fun->activation, i);
        if (argTrait->type) {
            out(mp, "%s: %s", fmtDeclaration(mp, argName), fmtTypeReference(mp, argTrait->type->qname));
        } else {
            out(mp, "%s", fmtDeclaration(mp, argName));
        }
        if (doc) {
            defaultValue = getDefault(doc, argName.name);
            if (defaultValue) {
                out(mp, " = %s", defaultValue);
            }
        }
        if (++i < (int) fun->numArgs) {
            out(mp, ", ");
        }
    }
    out(mp, ")");
    if (fun->resultType) {
        out(mp, ": %s", fmtTypeReference(mp, fun->resultType->qname));
    }
    out(mp, "\n</div>\n");

    if (doc) {
        out(mp, "<div class='apiDetail'>\n<p>%s</p>\n", doc->brief);
        if (doc->description && *doc->description) {
            out(mp, "<dl><dt>Description</dt><dd>%s</dd></dl>\n", doc->description);
        }

        count = mprGetListCount(doc->params);
        if (count > 0) {
            out(mp, "<dl><dt>Parameters</dt>\n");
            out(mp, "<dd><table class='parameters' summary ='parameters'>\n");

            checkArgs(mp, ejs, &fp->ownerName, fun, &qname, doc);
            for (next = 0; (param = mprGetNextItem(doc->params, &next)) != 0; ) {

                defaultValue = getDefault(doc, param->key);
                i = findArg(ejs, fun, param->key);
                if (i < 0) {
                    mprError(mp, "Bad @param reference for \"%s\" in function \"%s\" in type \"%s\"", param->key, 
                        qname.name, fp->ownerName.name);
                } else {
                    argName = ejsGetPropertyName(ejs, fun->activation, i);
                    argTrait = ejsGetTrait(ejs, fun->activation, i);
                    out(mp, "<tr class='param'><td class='param'>");
                    if (argTrait->type) {
                        out(mp, "%s: %s ", fmtDeclaration(mp, argName), fmtTypeReference(mp, argTrait->type->qname));
                    } else {
                        out(mp, "%s ", fmtDeclaration(mp, argName));
                    }
                    out(mp, "</td><td>%s", param->value);
                    if (defaultValue) {
                        if (strstr(param->value, "Not implemented") == NULL) {
                            out(mp, " [default: %s]", defaultValue);
                        }
                    }
                }
                out(mp, "</td></tr>");
            }
            out(mp, "</table></dd>\n");
            out(mp, "</dl>");
        }

        count = mprGetListCount(doc->options);
        if (count > 0) {
            out(mp, "<dl><dt>Options</dt>\n");
            out(mp, "<dd><table class='parameters' summary ='options'>\n");
            for (next = 0; (option = mprGetNextItem(doc->options, &next)) != 0; ) {
                out(mp, "<td class='param'>%s</td><td>%s</td>", option->key, option->value);
                out(mp, "</tr>\n");
            }
            out(mp, "</table></dd>\n");
            out(mp, "</dl>");
        }
        
        count = mprGetListCount(doc->events);
        if (count > 0) {
            out(mp, "<dl><dt>Events</dt>\n");
            out(mp, "<dd><table class='parameters' summary ='events'>\n");
            for (next = 0; (event = mprGetNextItem(doc->events, &next)) != 0; ) {
                out(mp, "<td class='param'>%s</td><td>%s</td>", event->key, event->value);
                out(mp, "</tr>\n");
            }
            out(mp, "</table></dd>\n");
            out(mp, "</dl>");
        }

        if (*doc->returns) {
            out(mp, "<dl><dt>Returns</dt>\n<dd>%s</dd></dl>\n", doc->returns);
        }
        count = mprGetListCount(doc->throws);
        if (count > 0) {
            out(mp, "<dl><dt>Throws</dt><dd>\n");
            for (next = 0; (thrown = (MprKeyValue*) mprGetNextItem(doc->throws, &next)) != 0; ) {
                //  TODO Functionalize
                ejs->state->bp = ejs->globalBlock;
                if ((slotNum = ejsLookupVar(ejs, ejs->global, ejsName(&throwName, "", thrown->key), &lookup)) < 0) {
                    continue;
                }
                throwName = lookup.name;
                out(mp, "<a href='%s'>%s</a>: %s%s\n", getFilename(fmtType(mp, throwName)), thrown->key,
                    thrown->value, (count == next) ? "" : ", ");
            }
            out(mp, "</dd>\n");
            out(mp, "</dl>");
        }
        if (*doc->requires) {
            out(mp, "<dl><dt>Requires</dt>\n<dd>configure --ejs-%s</dd></dl>\n", doc->requires);
        }
        if (*doc->spec) {
            out(mp, "<dl><dt>Specified</dt>\n<dd>%s</dd></dl>\n", doc->spec);
        }
        if (*doc->stability) {
            out(mp, "<dl><dt>Stability</dt>\n<dd>%s</dd></dl>\n", doc->stability);
        }
        if (*doc->example) {
            out(mp, "<dl><dt>Example</dt>\n<dd><pre>%s</pre></dd></dl>\n", doc->example);
        }
        count = mprGetListCount(doc->see);
        if (count > 0) {
            out(mp, "<dl><dt>See Also</dt>\n<dd>\n");
            for (next = 0; (see = mprGetNextItem(doc->see, &next)) != 0; ) {
                out(mp, "<a href='%s'>%s</a>%s\n", getFilename(see), see, (count == next) ? "" : ", ");
            }
            out(mp, "</dd></dl>\n");
        }
        out(mp, "</div>\n");
    }
    out(mp, "</div>\n");
    out(mp, "<hr />\n");
}


static char *fmtAttributes(int attributes, int accessors)
{
    static char attributeBuf[MPR_MAX_STRING];

    attributeBuf[0] = '\0';

    /*
        Order to look best
     */
    if (attributes & EJS_PROP_STATIC) {
        strcat(attributeBuf, "static ");
    }
    if (attributes & EJS_TRAIT_READONLY) {
        strcat(attributeBuf, "const ");
    }
    if (attributes & EJS_TYPE_FINAL) {
        strcat(attributeBuf, "final ");
    }
    if (attributes & EJS_FUN_OVERRIDE) {
        strcat(attributeBuf, "override ");
    }
    if (attributes & EJS_TYPE_DYNAMIC_INSTANCE) {
        strcat(attributeBuf, "dynamic ");
    }
    if (accessors) {
        if (attributes & EJS_TRAIT_GETTER) {
            strcat(attributeBuf, "get ");
        }
        if (attributes & EJS_TRAIT_SETTER) {
            strcat(attributeBuf, "set ");
        }
    }
    return attributeBuf;
}


static char *fmtAccessors(int attributes)
{
    static char attributeBuf[MPR_MAX_STRING];

    attributeBuf[0] = '\0';

    if (attributes & EJS_TRAIT_GETTER) {
        strcat(attributeBuf, "get ");
    }
    if (attributes & EJS_TRAIT_SETTER) {
        strcat(attributeBuf, "set ");
    }
    return attributeBuf;
}


static char *joinLines(char *str)
{
    char    *cp, *np;

    for (cp = str; cp && *cp; cp++) {
        if (*cp == '\n') {
            for (np = &cp[1]; *np; np++) {
                if (!isspace((int) *np)) {
                    break;
                }
            }
            if (!isspace((int) *np)) {
                *cp = ' ';
            }
        }
    }
    return str;
}


/*
    Merge in @duplicate entries
 */
static char *mergeDuplicates(Ejs *ejs, EjsMod *mp, EjsName *qname, EjsDoc *doc, char *spec)
{
    EjsDoc      *dup;
    EjsName     dname;
    char        *next, *duplicate, *mark;

    if ((next = strstr(spec, "@duplicate")) == 0) {
        return spec;
    }
    next = spec = mprStrdup(mp, spec);

    while ((next = strstr(next, "@duplicate")) != 0) {
        mark = next;
        mprStrTok(next, " \t\n\r", &next);
        if ((duplicate = mprStrTok(next, " \t\n\r", &next)) == 0) {
            break;
        }
        if ((dup = getDuplicateDoc(ejs, duplicate)) == 0) {
            mprError(ejs, "Can't find @duplicate directive %s for %s", duplicate, qname->name);
        } else {
            crackDoc(mp, dup, ejsName(&dname, "", duplicate));

            mprCopyList(doc->params, dup->params);
            mprCopyList(doc->options, dup->options);
            mprCopyList(doc->events, dup->events);
            mprCopyList(doc->see, dup->see);
            mprCopyList(doc->throws, dup->throws);

            doc->brief = dup->brief;
            doc->description = dup->description;
            doc->example = dup->example;
            doc->requires = dup->requires;
            doc->returns = dup->returns;
            doc->stability = dup->stability;
            doc->spec = dup->spec;
        }
        memmove(mark, next, strlen(next) + 1);
        next = mark;
    }
    return spec;
}


/*
    Cleanup text. Remove leading comments and "*" that are part of the comment and not part of the doc.
 */
static void prepText(char *str)
{
    char    *dp, *cp;

    dp = cp = str;
    while (isspace((int) *cp) || *cp == '*') {
        cp++;
    }
    while (*cp) {
        if (cp[0] == '\n') {
            *dp++ = '\n';
            for (cp++; (isspace((int) *cp) || *cp == '*'); cp++) {
                if (*cp == '\n') {
                    *dp++ = '\n';
                }
            }
            if (*cp == '\0') {
                cp--;
            }
        } else {
            *dp++ = *cp++;
        }
    }
    *dp = '\0';
}


static EjsDoc *crackDoc(EjsMod *mp, EjsDoc *doc, EjsName *qname)
{
    Ejs         *ejs;
    EjsDoc      *dup;
    EjsName     dname;
    MprKeyValue *pair;
    char        *value, *key, *line, *str, *next, *cp, *token, *nextWord, *word, *duplicate;
    char        *thisBrief, *thisDescription, *start;

    ejs = mp->ejs;

    mprAssert(strstr(qname->name, "missingDoc") == 0);
    
    if (doc->cracked) {
        return doc;
    }
    doc->cracked = 1;
    doc->params = mprCreateList(doc);
    doc->options = mprCreateList(doc);
    doc->events = mprCreateList(doc);
    doc->defaults = mprCreateList(doc);
    doc->see = mprCreateList(doc);
    doc->throws = mprCreateList(doc);

    str = mprStrdup(doc, doc->docString);
    if (str == NULL) {
        return doc;
    }
    prepText(str);
    if (strstr(str, "@hide") || strstr(str, "@hidden")) {
        doc->hide = 1;
    } else if (strstr(str, "@deprecate") || strstr(str, "@deprecated")) {
        doc->deprecated = 1;
    }
    doc->description = "";
    doc->brief = "";
    str = mergeDuplicates(ejs, mp, qname, doc, str);

    thisDescription = "";
    thisBrief = "";
    next = str;
    
    if (str[0] != '@') {
        if ((thisBrief = mprStrTok(str, "@", &next)) == 0) {
            thisBrief = "";
        } else {
            for (cp = thisBrief; *cp; cp++) {
                if (*cp == '.' && (isspace((int) cp[1]) || *cp == '*')) {
                    cp++;
                    *cp++ = '\0';
                    thisDescription = mprStrTrim(cp, " \t\n");
                    break;
                }
            }
        }
    }
    doc->brief = mprStrcat(doc, -1, doc->brief, thisBrief, NULL);
    doc->description = mprStrcat(doc, -1, doc->description, thisDescription, NULL);
    mprStrTrim(doc->brief, " \t\r\n");
    mprStrTrim(doc->description, " \t\r\n");
    mprAssert(doc->brief);
    mprAssert(doc->description);

    /*
        This is what we are parsing:
        One liner is the first sentance. Rest of description can continue from here and can include embedded html
 
        @param argName Description          (Up to next @, case matters on argName)
        @default argName DefaultValue       (Up to next @, case matters on argName)
        @return Sentance                    (Can use return or returns. If sentance starts with lower case, then start 
                                                sentance with "Call returns".
        @option argName Description         (Up to next @, case matters on argName)
        @event eventName Description        (Up to next @, case matters on argName)
        @throws ExceptionType Explanation   (Up to next @)
        @see Keyword keyword ...            (Case matters)
        @example Description                (Up to next @)
        @stability kind                     (prototype | evolving | stable | mature | deprecated]
        @deprecated version                 Same as @stability deprecated
        @requires ECMA                      (Emit: configuration requires --ejs-ecma)
        @spec                               (ecma-262, ecma-357, ejs-11)
        @hide                               (Hides this entry)

        Only functions can use return and param.
     */
    start = next;
    while (next) {
        token = mprStrTok(next, "@", &next);
        line = skipAtWord(token);

        if (token > &start[2] && token[-2] == '\\') {
            continue;
        }
        if (match(token, "duplicate")) {
            duplicate = mprStrTrim(line, " \t\n");
            if ((dup = getDuplicateDoc(ejs, duplicate)) == 0) {
                mprError(ejs, "Can't find @duplicate directive %s for %s", duplicate, qname->name);
            } else {
                crackDoc(mp, dup, ejsName(&dname, "", duplicate));
                mprCopyList(doc->params, dup->params);
                mprCopyList(doc->options, dup->options);
                mprCopyList(doc->events, dup->events);
                mprCopyList(doc->see, dup->see);
                mprCopyList(doc->throws, dup->throws);
                doc->brief = mprStrcat(doc, -1, doc->brief, " ", dup->brief, NULL);
                doc->description = mprStrcat(doc, -1, doc->description, " ", dup->description, NULL);
                if (dup->example) {
                    if (doc->example) {
                        doc->example = mprStrcat(doc, -1, doc->example, " ", dup->example, NULL);
                    } else {
                        doc->example = dup->example;
                    }
                }
                if (dup->requires) {
                    if (doc->requires) {
                        doc->requires = mprStrcat(doc, -1, doc->requires, " ", dup->requires, NULL);
                    } else {
                        doc->requires = dup->requires;
                    }
                }
                if (dup->returns && doc->returns == 0) {
                    doc->returns = dup->returns;
                }
                if (dup->stability && doc->stability == 0) {   
                    doc->stability = dup->stability;
                }
                if (dup->spec && doc->spec == 0) {
                    doc->spec = dup->spec;
                }
            }

        } else if (match(token, "example") || match(token, "examples")) {
            doc->example = formatExample(ejs, doc->docString);

        } else if (match(token, "event")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            if (key && *key) {
                pair = mprCreateKeyPair(doc->events, key, value);
                mprAddItem(doc->events, pair);
            }

        } else if (match(token, "option")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            if (key && *key) {
                pair = mprCreateKeyPair(doc->options, key, value);
                mprAddItem(doc->options, pair);
            }

        } else if (match(token, "param")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            if (key && *key) {
                key = mprStrTrim(key, ".");
                pair = mprCreateKeyPair(doc->params, key, value);
                mprAddItem(doc->params, pair);
            }

        } else if (match(token, "default")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            if (key && *key) {
                pair = mprCreateKeyPair(doc->defaults, key, value);
                mprAddItem(doc->defaults, pair);
            }

        } else if (match(token, "deprecated")) {
            doc->hide = 1;
            doc->deprecated = 1;
            doc->stability = "deprecated";

        } else if (match(token, "hide") || match(token, "hidden")) {
            doc->hide = 1;

        } else if (match(token, "spec")) {
            doc->spec = mprStrTrim(line, " \t\n");
            mprStrLower(doc->spec);

        } else if (match(token, "stability")) {
            doc->stability = mprStrTrim(line, " \t");
            mprStrLower(doc->stability);

        } else if (match(token, "requires")) {
            doc->requires = mprStrTrim(line, " \t");
            mprStrLower(doc->requires);

        } else if (match(token, "return") || match(token, "returns")) {
            line = joinLines(line);
            doc->returns = mprStrTrim(line, " \t");

        } else if (match(token, "throw") || match(token, "throws")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            pair = mprCreateKeyPair(doc->throws, key, value);
            mprAddItem(doc->throws, pair);

        } else if (match(token, "see") || match(token, "seeAlso")) {
            nextWord = line;
            do {
                word = nextWord;
                mprStrTok(word, " \t\r\n", &nextWord);
                mprAddItem(doc->see, mprStrTrim(word, " \t"));
            } while (nextWord && *nextWord);
        }
    }
    fixupDoc(ejs, doc);
    return doc;
}


static char *fixSentance(MprCtx ctx, char *str)
{
    char    *buf;
    int     len;

    if (str == 0 || *str == '\0') {
        return "";
    }
    
    /*
        Copy the string and grow by 1 byte (plus null) to allow for a trailing period.
     */
    len = (int) strlen(str) + 2;
    buf = mprAlloc(ctx, len);
    if (str == 0) {
        return "";
    }
    mprStrcpy(buf, len, str);
    str = buf;
    str[0] = toupper((int) str[0]);

    /*
        We can safely patch one past the end as we always have new lines and white space before the next token or 
        end of comment.
     */
    str = mprStrTrim(str, " \t\r\n.");

    /*
        Add a "." if the string does not appear to contain HTML tags
     */
    if (strstr(str, "</") == 0) {
        len = (int) strlen(str);
        if (str[len - 1] != '.') {
            str[len] = '.';
            str[len+1] = '\0';
        }
    }
    return str;
}


static char *formatExample(Ejs *ejs, char *docString)
{
    char    *example, *cp, *end, *buf, *dp;
    int     i, indent;

    if ((example = strstr(docString, "@example")) != 0) {
        example += 8;
        for (cp = example; *cp && *cp != '\n'; cp++) {}
        if (*cp == '\n') {
            cp++;
        }
        example = cp;
        if ((end = strchr(cp, '@')) != 0) {
            *end = '\0';
        }
        for (indent = 0; *cp == '\t' || *cp == ' '; indent++, cp++) {}

        buf = mprAlloc(ejs, (int) strlen(example) * 4 + 2);
        for (cp = example, dp = buf; *cp; ) {
            for (i = 0; i < indent && *cp; i++, cp++) {}
            for (; *cp && *cp != '\n'; ) {
                if (*cp == '<' && cp[1] == '%') {
                    strcpy(dp, "&lt;");
                    dp += 4;
                    cp++;
                    *dp++ = *cp++;
                } else if (*cp == '%' && cp[1] == '>') {
                    *dp++ = *cp++;
                    strcpy(dp, "&gt;");
                    dp += 4;
                    cp++;
                } else {
                    *dp++ = *cp++;
                }
            }
            if (*cp == '\n') {
                *dp++ = *cp++;
            }
            *dp = '\0';
        }
        return buf;
    }
    return "";
}


static char *wikiFormat(Ejs *ejs, char *start)
{
    EjsLookup   lookup;
    EjsName     qname;
    MprBuf      *buf;
    char        *end, *cp, *klass, *property, *str;
    int         slotNum, sentence;

    if (start == 0 || *start == '\0') {
        return "";
    }
    buf = mprCreateBuf(ejs, -1, -1);

    end = &start[strlen(start)];
    for (str = start; str < end && *str; str++) {
        /*
            MOB -- expand this to support basic markdown
         */
        if (str[0] == '\n' && str[1] == '\n') {
            /* Two blank lines forces a blank line in the output */
            mprPutStringToBuf(buf, "<br/><br/>");
            str++;

        } else if (*str == '$') {
            /* Dollar reference expansion*/
            klass = 0;
            property = 0;
            klass = &str[1];
            for (cp = klass; *cp; cp++) {
                if (isspace((int) *cp)) {
                    *cp++ = '\0';
                    break;
                }
            }
            sentence = (klass[strlen(klass) - 1] == '.');

            if ((property = strchr(str, '.')) != 0) {
                *property++ = '\0';
                if (*property == '\0') {
                    property = &str[1];
                    klass = 0;
                }
                str = &property[strlen(property)];
            } else {
                str = &str[strlen(str)];
                if (islower((int) *klass)) {
                    property = klass;
                    klass = 0;
                }
            }
            if (klass) {
                klass = mprStrTrim(klass, ".");
                property = mprStrTrim(property, ".");
                //  TODO Functionalize
                ejs->state->bp = ejs->globalBlock;
                if ((slotNum = ejsLookupVar(ejs, ejs->global, ejsName(&qname, "", klass), &lookup)) < 0) {
                    continue;
                }
                qname = lookup.name;
                if (property) {
                    mprPutFmtToBuf(buf, "<a href='%s#%s'>%s.%s</a>", getFilename(fmtType(ejs, qname)), 
                        property, klass, property);
                } else {
                    mprPutFmtToBuf(buf, "<a href='%s'>%s</a>", getFilename(fmtType(ejs, qname)), klass);
                }
            } else {
                mprPutFmtToBuf(buf, "<a href='#%s'>%s</a>", property, property);
            }
            if (sentence) {
                mprPutCharToBuf(buf, '.');
            }
            mprPutCharToBuf(buf, ' ');

        } else {
            mprPutCharToBuf(buf, *str);
        }
    }
    mprAddNullToBuf(buf);
    return mprStealBuf(ejs, buf);
}


static void fixupDoc(Ejs *ejs, EjsDoc *doc)
{
    MprKeyValue     *pair;
    int             next;

    doc->brief = wikiFormat(ejs, fixSentance(doc, doc->brief));
    doc->description = wikiFormat(ejs, fixSentance(doc, doc->description));
    doc->returns = wikiFormat(ejs, fixSentance(doc, doc->returns));
    doc->stability = fixSentance(doc, doc->stability);
    doc->requires = wikiFormat(ejs, fixSentance(doc, doc->requires));
    if (doc->spec) {
        if (strcmp(doc->spec, "ejs") == 0) {
            doc->spec = mprAsprintf(doc, -1, "ejscript-%d.%d", BLD_MAJOR_VERSION, BLD_MINOR_VERSION);
        }
    } else {
        doc->spec = "";
    }
    if (doc->example == 0) {
        doc->example = "";
    }
    for (next = 0; (pair = mprGetNextItem(doc->events, &next)) != 0; ) {
        pair->value = wikiFormat(ejs, fixSentance(doc, pair->value));
    }
    for (next = 0; (pair = mprGetNextItem(doc->options, &next)) != 0; ) {
        pair->value = wikiFormat(ejs, fixSentance(doc, pair->value));
    }
    for (next = 0; (pair = mprGetNextItem(doc->params, &next)) != 0; ) {
        pair->value = wikiFormat(ejs, fixSentance(doc, pair->value));
    }
    /*
        Don't fix the default value
     */
}


static void out(EjsMod *mp, char *fmt, ...)
{
    va_list     args;
    char        *buf;

    if (mp->file == 0) {
        mp->file = mprOpen(mp, mp->path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (mp->file == 0) {
            mprError(mp, "Can't open %s", mp->path);
            mp->errorCount++;
            return;
        }
    }
    va_start(args, fmt);
    buf = mprVasprintf(mp, -1, fmt, args);
    if (mprWriteString(mp->file, buf) < 0) {
        mprError(mp->file, "Can't write to buffer");
    }
    mprFree(buf);
}


static char *fmtModule(cchar *name)
{
    static char buf[MPR_MAX_STRING];

    mprStrcpy(buf, sizeof(buf), name);
    if (strcmp(buf, EJS_DEFAULT_MODULE) == 0) {
        buf[0] = '\0';
    }
    return buf;
}


static char *fmtClassUrl(MprCtx ctx, EjsName qname)
{
    return getFilename(fmtType(ctx, qname));
}


static char *fmtNamespace(EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *cp;

    if (qname.space[0] == '[') {
        mprStrcpy(buf, sizeof(buf), &qname.space[1]);

    } else {
        mprStrcpy(buf, sizeof(buf), qname.space);
    }
    if (buf[strlen(buf) - 1] == ']') {
        buf[strlen(buf) - 1] = '\0';
    }

    if ((cp = strrchr(buf, ',')) != 0) {
        ++cp;
        if (strcmp(cp, EJS_PUBLIC_NAMESPACE) == 0) {
            strcpy(buf, EJS_PUBLIC_NAMESPACE);

        } else if (strcmp(cp, EJS_PRIVATE_NAMESPACE) == 0 || strcmp(cp, EJS_CONSTRUCTOR_NAMESPACE) == 0 ||
                   strcmp(cp, EJS_INIT_NAMESPACE) == 0) {
            /*
                Suppress "private" as they are the default for namespaces and local vars
             */
            buf[0] = '\0';
        }
    }

    if (strcmp(buf, EJS_PRIVATE_NAMESPACE) == 0 || strcmp(buf, EJS_CONSTRUCTOR_NAMESPACE) == 0 ||
           strcmp(buf, EJS_INIT_NAMESPACE) == 0) {
        buf[0] = '\0';
    }
    return buf;
}


static char *fmtType(MprCtx ctx, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *namespace;

    namespace = fmtNamespace(qname);

    if (strcmp(namespace, EJS_PUBLIC_NAMESPACE) == 0) {
        *namespace = '\0';
    }
    if (*namespace) {
        if (*namespace) {
            mprSprintf(ctx, buf, sizeof(buf), "%s::%s", namespace, qname.name);
        } else {
            mprSprintf(ctx, buf, sizeof(buf), "%s", qname.name);
        }
    } else {
        mprSprintf(ctx, buf, sizeof(buf), "%s", qname.name);
    }
    return buf;
}


/*
    Map lower case names with a "l-" prefix for systems with case insensitive names
 */
static char *getFilename(cchar *name)
{
    static char buf[MPR_MAX_STRING];
    char        *cp, *sp;

    mprStrcpy(buf, sizeof(buf), name);

    if ((cp = strstr(buf, "::")) != 0) {
        *cp++ = '-';
        if (islower((int) cp[1])) {
            *cp++ = '-';
            for (sp = cp; *sp; ) {
                *cp++ = *sp++;
            }

        } else {
            for (sp = cp + 1; *sp; ) {
                *cp++ = *sp++;
            }
        }
        *cp = '\0';
    }
    mprStrcpy(&buf[strlen(buf)], sizeof(buf) - (int) strlen(buf) - 1, ".html");
    return buf;
}


static char *fmtTypeReference(MprCtx ctx, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *typeName;

    typeName = fmtType(ctx, qname);
    mprSprintf(ctx, buf, sizeof(buf), "<a href='%s'>%s</a>", getFilename(typeName), qname.name);
    return buf;
}


static char *fmtSpace(MprCtx ctx, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *namespace;

    namespace = fmtNamespace(qname);

    if (namespace[0]) {
        mprSprintf(ctx, buf, sizeof(buf), "%s", qname.space);
    }
    return buf;
}


static char *fmtDeclaration(MprCtx ctx, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *namespace;

    namespace = fmtNamespace(qname);

    if (namespace[0]) {
        mprSprintf(ctx, buf, sizeof(buf), "%s %s", qname.space, demangle(qname.name));
    } else {
        mprSprintf(ctx, buf, sizeof(buf), "%s", demangle(qname.name));
    }
    return buf;
}


static bool match(cchar *last, cchar *key)
{
    int     len;

    mprAssert(last);
    mprAssert(key && *key);

    len = (int) strlen(key);
    return strncmp(last, key, len) == 0;
}


static char *skipAtWord(char *str)
{
    while (!isspace((int) *str) && *str)
        str++;
    while (isspace((int) *str))
        str++;
    return str;
}


#if UNUSED && KEEP
static char *getType(char *str, char *typeName, int typeSize)
{
    char    *end, *cp;
    int     i;

    for (end = str; *end && isspace((int) *end); end++)
        ;
    for (; *end && !isspace((int) *end); end++)
        ;
    typeSize--;
    for (i = 0, cp = str; i < typeSize && cp < end; cp++, i++) {
        typeName[i] = *cp;
    }
    typeName[i] = '\0';

    for (; *end && isspace((int) *end); end++)
        ;
    return end;
}
#endif


static void getKeyValue(char *str, char **key, char **value)
{
    char    *end;

    for (end = str; *end && !isspace((int) *end); end++)
        ;
    if (end) {
        *end = '\0';
    }
    if (key) {
        *key = mprStrTrim(str, " \t");
    }
    for (str = end + 1; *str && isspace((int) *str); str++) {
        ;
    }
    if (value) {
        *value = mprStrTrim(str, " \t");
    }
}


static int compareProperties(PropRec **p1, PropRec **p2)
{
    return compareNames((char**) &(*p1)->qname.name, (char**) &(*p2)->qname.name);
}


static int compareFunctions(FunRec **f1, FunRec **f2)
{
    return compareNames((char**) &(*f1)->qname.name, (char**) &(*f2)->qname.name);
}


static int compareClasses(ClassRec **c1, ClassRec **c2)
{
    return compareNames((char**) &(*c1)->qname.name, (char**) &(*c2)->qname.name);
}


static cchar *demangle(cchar *name)
{
    if (strncmp(name, "set-", 4) == 0) {
        return &name[4];
    }
    return name;
}

static int compareNames(char **q1, char **q2)
{
    char    *s1, *s2, *cp;

    s1 = *q1;
    s2 = *q2;

    s1 = (char*) demangle(s1);
    s2 = (char*) demangle(s2);

    /*
        Don't sort on the namespace portions of the name
     */
    if ((cp = strrchr(s1, ':')) != 0) {
        s1 = cp + 1;
    }
    if ((cp = strrchr(s2, ':')) != 0) {
        s2 = cp + 1;
    }

    return mprStrcmpAnyCase(s1, s2);
}


static void addUniqueItem(MprList *list, cchar *item)
{
    cchar   *p;
    int     next;

    if (item == 0 || *item == '\0') {
        return;
    }
    for (next = 0; (p = mprGetNextItem(list, &next)) != 0; ) {
        if (strcmp(p, item) == 0) {
            return;
        }
    }
    mprAddItem(list, mprStrdup(list, item));
}


static void addUniqueClass(MprList *list, ClassRec *item)
{
    ClassRec    *p;
    int         next;

    if (item == 0) {
        return;
    }
    for (next = 0; (p = (ClassRec*) mprGetNextItem(list, &next)) != 0; ) {
        if (strcmp(p->qname.name, item->qname.name) == 0) {
            if (item->qname.space && p->qname.space && strcmp(p->qname.space, item->qname.space) == 0) {
                return;
            }
        }
    }
    mprAddItem(list, item);
}


static EjsDoc *getDoc(Ejs *ejs, void *vp, int slotNum)
{
    char        key[32];

    mprSprintf(ejs, key, sizeof(key), "%Lx %d", PTOL(vp), slotNum);
    return (EjsDoc*) mprLookupHash(ejs->doc, key);
}


static EjsDoc *getDuplicateDoc(Ejs *ejs, cchar *duplicate)
{
    EjsName         qname;
    EjsDoc          *doc;
    EjsObj          *vp;
    EjsLookup       lookup;
    char            *space, *dup, *klass, *property;
    int             slotNum;

    dup = space = mprStrdup(ejs, duplicate);
    if ((klass = strstr(space, "::")) != 0) {
        *klass = '\0';
        klass += 2;
    } else {
        klass = space;
        space = "";
    }
    if ((property = strchr(klass, '.')) != 0) {
        *property++ = '\0';
    }
    //  TODO Functionalize
    ejs->state->bp = ejs->globalBlock;
    if ((slotNum = ejsLookupVar(ejs, ejs->global, ejsName(&qname, space, klass), &lookup)) < 0) {
        mprFree(dup);
        return 0;
    }
    if (property == 0) {
        doc = getDoc(ejs, ejs->globalBlock, slotNum);
    } else {
        vp = ejsGetProperty(ejs, ejs->global, slotNum);
        if ((slotNum = ejsLookupVar(ejs, vp, ejsName(&qname, "", property), &lookup)) < 0) {
            if (ejsIsType(vp)) {
                vp = ((EjsType*) vp)->prototype;
                if ((slotNum = ejsLookupVar(ejs, vp, &qname, &lookup)) < 0) {
                    mprFree(dup);
                    return 0;
                }
            }
#if UNUSED
            if ((slotNum = ejsLookupVar(ejs, vp, ejsName(&qname, space, property), &lookup)) < 0) {
                if ((slotNum = ejsLookupVar(ejs, vp, ejsName(&qname, space, property), &lookup)) < 0) {
                    /* Last chance - Not ideal */
                    space = mprAsprintf(ejs, -1, "%s-%s", space, BLD_VNUM);
                    if ((slotNum = ejsLookupVar(ejs, vp, ejsName(&qname, space, property), &lookup)) < 0) {
                        mprFree(dup);
                        return 0;
                    }
                }
            }
#endif
        }
        doc = getDoc(ejs, (EjsBlock*) vp, slotNum);
    }
    if (doc) {
        if (doc->docString == NULL || *doc->docString == '\0') {
            mprError(ejs, "Duplicate entry %s provides no description", duplicate);
            mprFree(dup);
            return 0;
        }
    }
    mprFree(dup);
    return doc;
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
