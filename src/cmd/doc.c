/**
    doc.c - Documentation generator

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*
    Supported documentation keywords and format.

    The element "one liner" is the first sentance. Rest of description can continue from here and can include embedded html.

    @param argName Description          (Up to next @, case matters on argName)
    @default argName DefaultValue       (Up to next @, case matters on argName)
    @return Sentence                    (Can use return or returns. If sentance starts with lower case, then start 
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

static Ejs *ejs;

/**************************** Forward Declarations ****************************/

static void      addUniqueItem(MprList *list, cchar *item);
static void      addUniqueClass(MprList *list, ClassRec *item);
static MprList   *buildClassList(EjsMod *mp, cchar *namespace);
static void      buildMethodList(EjsMod *mp, MprList *methods, EjsObj *obj, EjsObj *owner, EjsName ownerName);
static void      buildPropertyList(EjsMod *mp, MprList *list, EjsAny *obj, int numInherited);
static int       compareClasses(ClassRec **c1, ClassRec **c2);
static int       compareFunctions(FunRec **f1, FunRec **f2);
static int       compareProperties(PropRec **p1, PropRec **p2);
static int       compareStrings(EjsString **q1, EjsString **q2);
static int       compareNames(char **q1, char **q2);
static EjsDoc    *crackDoc(EjsMod *mp, EjsDoc *doc, EjsName qname);
static MprFile   *createFile(EjsMod *mp, char *name);
static MprKeyValue *createKeyPair(MprChar *key, MprChar *value);
static cchar     *demangle(Ejs *ejs, EjsString *name);
static void      fixupDoc(Ejs *ejs, EjsDoc *doc);
static char      *fmtAccessors(int attributes);
static char      *fmtAttributes(EjsAny *vp, int attributes, int klass);
static char      *fmtClassUrl(Ejs *ejs, EjsName qname);
static char      *fmtDeclaration(Ejs *ejs, EjsName qname);
static char      *fmtNamespace(Ejs *ejs, EjsName qname);
static char      *fmtSpace(Ejs *ejs, EjsName qname);
static char      *fmtType(Ejs *ejs, EjsName qname);
static char      *fmtTypeReference(Ejs *ejs, EjsName qname);
static EjsString *fmtModule(Ejs *ejs, EjsString *name);
static MprChar   *formatExample(Ejs *ejs, EjsString *example);
static int       generateMethodTable(EjsMod *mp, MprList *methods, EjsObj *obj, int instanceMethods);
static void      generateClassPage(EjsMod *mp, EjsObj *obj, EjsName name, EjsTrait *trait, EjsDoc *doc);
static void      generateClassPages(EjsMod *mp);
static void      generateClassPageHeader(EjsMod *mp, EjsObj *obj, EjsName name, EjsTrait *trait, EjsDoc *doc);
static int       generateClassPropertyTableEntries(EjsMod *mp, EjsObj *obj, MprList *properties);
static void      generateClassList(EjsMod *mp, cchar *namespace);
static void      generateContentFooter(EjsMod *mp);
static void      generateContentHeader(EjsMod *mp, cchar *fmt, ... );
static void      generateHomeFrameset(EjsMod *mp);
static void      generateHomeNavigation(EjsMod *mp) ;
static void      generateHomePages(EjsMod *mp);
static void      generateHomeTitle(EjsMod *mp);
static void      generateHtmlFooter(EjsMod *mp);
static void      generateHtmlHeader(EjsMod *mp, cchar *script, cchar *title, ... );
static void      generateImages(EjsMod *mp);
static void      generateOverview(EjsMod *mp);
static void      generateMethod(EjsMod *mp, FunRec *fp);
static void      generateMethodDetail(EjsMod *mp, MprList *methods);
static void      generateNamespace(EjsMod *mp, cchar *namespace);
static void      generateNamespaceClassTable(EjsMod *mp, cchar *namespace);
static int       generateNamespaceClassTableEntries(EjsMod *mp, cchar *namespace);
static void      generateNamespaceList(EjsMod *mp);
static void      generatePropertyTable(EjsMod *mp, EjsObj *obj);
static cchar    *getDefault(EjsDoc *doc, cchar *key);
static EjsDoc   *getDoc(Ejs *ejs, cchar *tag, void *block, int slotNum);
static EjsDoc   *getDuplicateDoc(Ejs *ejs, MprChar *duplicate);
static void      getKeyValue(MprChar *str, MprChar **key, MprChar **value);
static char     *getFilename(cchar *name);
static int       getPropertyCount(Ejs *ejs, EjsObj *obj);
static bool      match(MprChar *last, cchar *key);
static void      prepDocStrings(EjsMod *mp, EjsObj *obj, EjsName name, EjsTrait *trait, EjsDoc *doc);
static void      out(EjsMod *mp, char *fmt, ...);
static MprChar  *skipAtWord(MprChar *str);

/*********************************** Code *************************************/

int emCreateDoc(EjsMod *mp)
{
    ejs = mp->ejs;

    if (ejs->doc == 0) {
        ejs->doc = mprCreateHash(EJS_DOC_HASH_SIZE, 0);
        if (ejs->doc == 0) {
            return MPR_ERR_MEMORY;
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

    for (df = docFiles; df->path; df++) {
        path = mprJoinPath(mp->docDir, df->path);
        mprMakeDir(mprGetPathDir(path), 0775, 1);
        file = mprOpenFile(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644);
        if (file == 0) {
            mprError("Can't create %s", path);
            mp->errorCount++;
            return;
        }
        if (mprWriteFile(file, df->data, df->size) != df->size) {
            mprError("Can't write to buffer");
            mp->errorCount++;
            return;
        }
        mprCloseFile(file);
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

    mprCloseFile(mp->file);
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

    mprCloseFile(mp->file);
    mp->file = 0;
}


static void generateHomeTitle(EjsMod *mp)
{
    mprCloseFile(mp->file);
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

    mprCloseFile(mp->file);
    mp->file = 0;
}


static void generateHomeNavigation(EjsMod *mp)
{
    mprCloseFile(mp->file);
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

    mprCloseFile(mp->file);
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
    namespaces = mprCreateList(0, 0);
    count = ejsGetLength(ejs, ejs->global);
    for (slotNum = 0; slotNum < count; slotNum++) {
        trait = ejsGetPropertyTraits(ejs, ejs->global, slotNum);
        if (trait == 0) {
            continue;
        }
        type = ejsGetProperty(ejs, ejs->global, slotNum);
        qname = ejsGetPropertyName(ejs, ejs->global, slotNum);
        if (type == 0 || !ejsIsType(ejs, type) || qname.name == 0 || ejsStartsWithMulti(ejs, qname.space, "internal-")) {
            continue;
        }
        doc = getDoc(ejs, "class", ejs->global, slotNum);
        if (doc && !doc->hide) {
            addUniqueItem(namespaces, fmtNamespace(ejs, qname));
        }
    }
    mprSortList(namespaces, compareNames);

    out(mp, "<tr><td><a href='__all-classes.html' target='classes'>All Namespaces</a></td></tr>\n");

    for (next = 0; (namespace = (cchar*) mprGetNextItem(namespaces, &next)) != 0; ) {
        out(mp, "<tr><td><a href='%s-classes.html' target='classes'>%s</a></td></tr>\n", namespace, namespace);
    }

    out(mp, "</table>\n");
    out(mp, "</div>\n");

    generateHtmlFooter(mp);
    mprCloseFile(mp->file);
    mp->file = 0;

    /*
        Generate namespace overviews and class list files for each namespace
     */
    for (next = 0; (namespace = (cchar*) mprGetNextItem(namespaces, &next)) != 0; ) {
        generateNamespace(mp, namespace);
    }
    generateNamespace(mp, "__all");
}


static void generateNamespace(EjsMod *mp, cchar *namespace)
{
    char    *path;

    path = sjoin(namespace, ".html", NULL);
    mp->file = createFile(mp, path);
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
    mprCloseFile(mp->file);
    mp->file = 0;

    /*
        Generate an overview page
     */
    generateClassList(mp, namespace);
}


static void generateNamespaceClassTable(EjsMod *mp, cchar *namespace)
{
    int         count;

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
    EjsName         qname;
    EjsDoc          *doc;
    ClassRec        *crec;
    MprList         *classes;
    char            *fmtName;
    int             next;

    ejs = mp->ejs;

    classes = buildClassList(mp, namespace);

    for (next = 0; (crec = (ClassRec*) mprGetNextItem(classes, &next)) != 0; ) {
        qname = crec->qname;
        fmtName = fmtType(ejs, crec->qname);
        out(mp, "   <tr><td><a href='%s' target='content'>%@</a></td>", getFilename(fmtName), qname.name);
        if (crec->block == ejs->global && mp->firstGlobal == ejsGetLength(ejs, ejs->global)) {
            continue;
        }
        doc = getDoc(ejs, "class", crec->block ? crec->block : ejs->global, crec->slotNum);
        if (doc && !doc->hide) {
            out(mp, "<td>%w</td></tr>\n", doc->brief);
        } else {
            out(mp, "<td>&nbsp;</td></tr>\n");
        }
    }
    return mprGetListLength(classes);
}


static MprList *buildClassList(EjsMod *mp, cchar *namespace)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait;
    EjsDoc          *doc;
    EjsName         qname;
    ClassRec        *crec;
    MprList         *classes;
    int             count, slotNum;

    ejs = mp->ejs;

    /*
        Build a sorted list of classes
     */
    classes = mprCreateList(0, 0);
    count = ejsGetLength(ejs, ejs->global);
    for (slotNum = 0; slotNum < count; slotNum++) {
        trait = ejsGetPropertyTraits(ejs, ejs->global, slotNum);
        if (trait == 0) {
            continue;
        }
        doc = getDoc(ejs, "class", ejs->global, slotNum);
        if (doc == 0 || doc->hide) {
            continue;
        }
        type = ejsGetProperty(ejs, ejs->global, slotNum);
        qname = ejsGetPropertyName(ejs, ejs->global, slotNum);
        if (type == 0 || !ejsIsType(ejs, type) || qname.name == 0) {
            continue;
        }
        if (strcmp(namespace, "__all") != 0 && strcmp(namespace, fmtNamespace(ejs, qname)) != 0) {
            continue;
        }

        /* Suppress the core language types (should not appear as classes) */

        if (ejsCompareMulti(ejs, qname.space, EJS_EJS_NAMESPACE) == 0) {
            if (ejsCompareMulti(ejs, qname.name, "int") == 0 || 
                ejsCompareMulti(ejs, qname.name, "long") == 0 || ejsCompareMulti(ejs, qname.name, "decimal") == 0 ||
                ejsCompareMulti(ejs, qname.name, "boolean") == 0 || ejsCompareMulti(ejs, qname.name, "double") == 0 || 
                ejsCompareMulti(ejs, qname.name, "string") == 0) {
                continue;
            }
        }
        /* Other fixups */
        if (ejsStartsWithMulti(ejs, qname.space, "internal") || ejsCompareMulti(ejs, qname.space, "private") == 0) {
            continue;
        }
        crec = mprAlloc(sizeof(ClassRec));
        crec->qname = qname;
        crec->trait = trait;
        crec->block = ejs->global;
        crec->slotNum = slotNum;
        addUniqueClass(classes, crec);
    }

    /*
        Add a special type "Global"
     */
    if (strcmp(namespace, "__all") == 0) {
        if (mp->firstGlobal < ejsGetLength(ejs, ejs->global)) {
            crec = mprAlloc(sizeof(ClassRec));
            crec->qname = N(EJS_EJS_NAMESPACE, EJS_GLOBAL);
            crec->block = ejs->global;
            crec->slotNum = ejsLookupProperty(ejs, ejs->global, crec->qname);
            addUniqueClass(classes, crec);
        }
    }
    mprSortList(classes, compareClasses);
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

    path = sjoin(namespace, "-classes.html", NULL);
    mp->file = createFile(mp, path);
    if (mp->file == 0) {
        mp->errorCount++;
        return;
    }

    /*
        Create the header and auto-load a namespace overview. We do this here because the class list is loaded
        when the user selects a namespace.
     */
    mprSprintf(script, sizeof(script), "parent.parent.content.location = \'%s.html\';", namespace);
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
        fmtName = fmtType(ejs, crec->qname);
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
    mprCloseFile(mp->file);
    mp->file = 0;
}


static void generateOverview(EjsMod *mp)
{
    mprCloseFile(mp->file);
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

    mprCloseFile(mp->file);
    mp->file = 0;
}


static void generateHtmlHeader(EjsMod *mp, cchar *script, cchar *fmt, ... )
{
    char        *title;
    va_list     args;

    va_start(args, fmt);
    title = sfmtv(fmt, args);
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
    title = sfmtv(fmt, args);
    va_end(args);

    generateHtmlHeader(mp, NULL, title);
    
    out(mp, "<body>\n<div class='body'>\n\n");
    out(mp, "<div class=\"content\">\n\n");
}


static void generateTerms(EjsMod *mp)
{
    out(mp,
        "<div class=\"terms\">\n"
        "   <p class=\"terms\">\n"
        "       <a href=\"http://www.embedthis.com/\">"
        "       Embedthis Software LLC, 2003-2011. All rights reserved. "
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

    path = mp->path = mprJoinPath(mp->docDir, name);
    file = mprOpenFile(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == 0) {
        mprError("Can't open %s", path);
        mp->errorCount++;
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

    count = ejsGetLength(ejs, ejs->global);
    for (slotNum = mp->firstGlobal; slotNum < count; slotNum++) {
        type = ejsGetProperty(ejs, ejs->global, slotNum);
        qname = ejsGetPropertyName(ejs, ejs->global, slotNum);
        if (type == 0 || !ejsIsType(ejs, type) || qname.name == 0 || ejsStartsWithMulti(ejs, qname.space, "internal-")) {
            continue;
        }
        /*
            Setup the output path, but create the file on demand when output is done
         */
        mprCloseFile(mp->file);
        mp->file = 0;
        mp->path = mprJoinPath(mp->docDir, getFilename(fmtType(ejs, type->qname)));
        if (mp->path == 0) {
            return;
        }
        trait = ejsGetPropertyTraits(ejs, ejs->global, slotNum);
        doc = getDoc(ejs, "class", ejs->global, slotNum);

        if (doc && !doc->hide) {
            generateClassPage(mp, (EjsObj*) type, qname, trait, doc);
        }
        mprCloseFile(mp->file);
        mp->file = 0;
    }

    /*
        Finally do one page specially for "global"
        TODO - Functionalize
     */
    trait = mprAlloc(sizeof(EjsTrait));
    doc = mprAlloc(sizeof(EjsDoc));
    doc->docString = ejsCreateStringFromAsc(ejs, "Global object containing all global functions and variables.");
    doc->returns = doc->example = doc->description = NULL;
    doc->trait = trait;

    mprSprintf(key, sizeof(key), "%Lx %d", PTOL(0), 0);
    mprAddKey(ejs->doc, key, doc);

    slotNum = ejsGetLength(ejs, ejs->global);

    qname = N(EJS_EJS_NAMESPACE, EJS_GLOBAL);
    mp->file = createFile(mp, getFilename(fmtType(ejs, qname)));
    if (mp->file == 0) {
        return;
    }
    generateClassPage(mp, ejs->global, qname, trait, doc);
    mprCloseFile(mp->file);
    mp->file = 0;
}


static void generateClassPage(EjsMod *mp, EjsObj *obj, EjsName name, EjsTrait *trait, EjsDoc *doc)
{
    MprList     *methods;
    int         count;

    prepDocStrings(mp, obj, name, trait, doc);
    if (doc->hide) {
        return;
    }
    generateClassPageHeader(mp, obj, name, trait, doc);
    generatePropertyTable(mp, obj);
    
    methods = mprCreateList(0, 0);
    buildMethodList(mp, methods, obj, obj, name);
    if (ejsIsType(ejs, obj)) {
        buildMethodList(mp, methods, (EjsObj*) ((EjsType*) obj)->prototype, obj, name);
    }
    count = generateMethodTable(mp, methods, obj, 0);
    count += generateMethodTable(mp, methods, obj, 1);
    if (count > 0) {
        generateMethodDetail(mp, methods);
    }
    generateContentFooter(mp);
}


static void prepDocStrings(EjsMod *mp, EjsObj *obj, EjsName qname, EjsTrait *typeTrait, EjsDoc *doc)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait;
    EjsPot          *prototype;
    EjsName         pname;
    EjsDoc          *dp;
    char            *combined;
    int             slotNum, numProp, numInherited;

    ejs = mp->ejs;

    if (doc) {
        crackDoc(mp, doc, qname);
    }
    type = ejsIsType(ejs, obj) ? (EjsType*) obj : 0;

    if (type && type->hasConstructor) {
        slotNum = ejsLookupProperty(ejs, ejs->global, type->qname);
        dp = getDoc(ejs, "fun", ejs->global, slotNum);
        if (dp) {
            crackDoc(mp, dp, type->qname);
        }
    }

    /*
        Loop over all the static properties
     */
    numProp = ejsGetLength(ejs, obj);
    for (slotNum = 0; slotNum < numProp; slotNum++) {
        trait = ejsGetPropertyTraits(ejs, obj, slotNum);
        if (trait == 0) {
            continue;
        }
        dp = getDoc(ejs, NULL, obj, slotNum);
        if (dp) {
            pname = ejsGetPropertyName(ejs, obj, slotNum);
            combined = sfmt("%@.%@", qname.name, pname.name);
            crackDoc(mp, dp, EN(combined)); 
        }
    }

    /*
        Loop over all the instance properties
     */
    if (type) {
        prototype = type->prototype;
        if (prototype) {
            numInherited = type->numInherited;
            if (ejsGetLength(ejs, prototype) > 0) {
                for (slotNum = numInherited; slotNum < prototype->numProp; slotNum++) {
                    trait = ejsGetPropertyTraits(ejs, prototype, slotNum);
                    if (trait == 0) {
                        continue;
                    }
                    doc = getDoc(ejs, NULL, prototype, slotNum);
                    if (doc) {
                        pname = ejsGetPropertyName(ejs, (EjsObj*) prototype, slotNum);
                        crackDoc(mp, doc, qname);
                    }
                }
            }
        }
    }
}


static void generateClassPageHeader(EjsMod *mp, EjsObj *obj, EjsName qname, EjsTrait *trait, EjsDoc *doc)
{
    Ejs         *ejs;
    EjsType     *t, *type;
    EjsString   *modName;
    cchar       *see;
    int         next, count;

    ejs = mp->ejs;

    mprAssert(ejsIsBlock(ejs, obj));

    if (!ejsIsType(ejs, obj)) {
        generateContentHeader(mp, "Global Functions and Variables");
        out(mp, "<a name='top'></a>\n");
        out(mp, "<h1 class='className'>Global Functions and Variables</h1>\n");
    } else {
        generateContentHeader(mp, "Class %@", qname.name);
        out(mp, "<a name='top'></a>\n");
        out(mp, "<h1 class='className'>%@</h1>\n", qname.name);
    }
    out(mp, "<div class='classBlock'>\n");

    if (ejsIsType(ejs, obj)) {
        type = (EjsType*) obj;
        out(mp, "<table class='classHead' summary='%@'>\n", qname.name);

        if (type && type->module) {
            modName = fmtModule(ejs, type->module->name);
            out(mp, "   <tr><td><strong>Module</strong></td><td>%@</td></tr>\n", modName);
        }
        out(mp, "   <tr><td><strong>Definition</strong></td><td>%s class %@</td></tr>\n", 
            fmtAttributes(obj, trait->attributes, 1), qname.name);

        if (type && type->baseType) {
            out(mp, "   <tr><td><strong>Inheritance</strong></td><td>%@", qname.name);
            for (t = type->baseType; t; t = t->baseType) {
                out(mp, " <img src='images/inherit.gif' alt='inherit'/> %s", fmtTypeReference(ejs, t->qname));
            }
        }
        if (doc) {
            if (doc->requires) {
                out(mp, "<tr><td><strong>Requires</strong></td><td>configure --ejs-%s</td></tr>\n", doc->requires);
            }
            if (doc->spec) {
                out(mp, "<tr><td><strong>Specified</strong></td><td>%s</td></tr>\n", doc->spec);
            }
            if (doc->stability) {
                out(mp, "<tr><td><strong>Stability</strong></td><td>%s</td></tr>\n", doc->stability);
            }
            if (doc->example) {
                out(mp, "<tr><td><strong>Example</strong></td><td><pre>%s</pre></td></tr>\n", doc->example);
            }
        }
        out(mp, "       </td></tr>\n");
        out(mp, "</table>\n\n");
    }
    if (doc) {
        out(mp, "<p class='classBrief'>%s</p>\n\n", doc->brief);
        if (doc->description) {
            out(mp, "<p class='classDescription'>%s</p>\n\n", doc->description);
        }

        count = mprGetListLength(doc->see);
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
    EjsObj      *vp;
    EjsPot      *prototype;
    EjsTrait    *trait;
    EjsType     *type;
    int         limit, count, slotNum;

    count = 0;

    limit = ejsGetLength(ejs, obj);
    for (slotNum = 0; slotNum < limit; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        if (vp) {
            trait = ejsGetPropertyTraits(ejs, obj, slotNum);
            if (trait && trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) {
                count++;
            } else if (!ejsIsFunction(ejs, vp)) {
                count++;
            }
        }
    }
    if (ejsIsType(ejs, obj)) {
        type = (EjsType*) obj;
        if (type->prototype) {
            prototype = type->prototype;
            limit = ejsGetLength(ejs, prototype);
            for (slotNum = 0; slotNum < limit; slotNum++) {
                vp = ejsGetProperty(ejs, prototype, slotNum);
                if (vp && !ejsIsFunction(ejs, vp)) {
                    count++;
                }
            }
        }
    }
    return count;
}


static void generatePropertyTable(EjsMod *mp, EjsObj *obj)
{
    Ejs         *ejs;
    EjsType     *type;
    MprList     *list;
    int         count;

    ejs = mp->ejs;

    list = mprCreateList(0, 0);
    buildPropertyList(mp, list, obj, 0);

    type = 0;
    if (ejsIsType(ejs, obj)) {
        type = (EjsType*) obj;
        if (type->prototype) {
            buildPropertyList(mp, list, type->prototype, type->numInherited);
        }
    }
    mprSortList(list, compareProperties);

    out(mp, "<a name='Properties'></a>\n");
    out(mp, "<h2 class='classSection'>Properties</h2>\n");

    if (mprGetListLength(list) > 0) {
        out(mp, "<table class='itemTable' summary='properties'>\n");
        out(mp, "   <tr><th>Qualifiers</th><th>Property</th><th>Type</th><th width='95%%'>Description</th></tr>\n");
        count = generateClassPropertyTableEntries(mp, obj, list);
        out(mp, "</table>\n\n");
    } else {
        out(mp, "   <p>(No own properties defined)</p>");
    }

    if (type && type->baseType) {
        count = getPropertyCount(ejs, (EjsObj*) type->baseType);
        if (count > 0) {
            out(mp, "<p class='inheritedLink'><a href='%s#Properties'><i>Inherited Properties</i></a></p>\n\n",
                fmtClassUrl(ejs, type->baseType->qname));
        }
    }
    out(mp, "<hr />\n");
}


/*
    Generate the entries for class properties. Will be called once for static properties and once for instance properties
 */
static void buildPropertyList(EjsMod *mp, MprList *list, EjsAny *obj, int numInherited)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsDoc          *doc;
    PropRec         *prec;
    int             start, slotNum, numProp;

    ejs = mp->ejs;

    /*
        Loop over all the (non-inherited) properties
     */
    start = (obj == ejs->global) ? mp->firstGlobal : numInherited;
    numProp = ejsGetLength(ejs, obj);
    for (slotNum = start; slotNum < numProp; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        trait = ejsGetPropertyTraits(ejs, obj, slotNum);
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        if (trait) {
            if (trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) {
                doc = getDoc(ejs, "fun", obj, slotNum);
            } else {
                doc = getDoc(ejs, NULL, obj, slotNum);
            }
            if (doc && doc->hide) {
                continue;
            }
        }
        if (vp == 0 || ejsIsType(ejs, vp) || qname.name == 0 || trait == 0) {
            continue;
        }
        if (ejsIsFunction(ejs, vp) && !(trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER))) {
            continue;
        }
        if (ejsCompareMulti(ejs, qname.space, EJS_PRIVATE_NAMESPACE) == 0 || 
            ejsContainsMulti(ejs, qname.space, ",private]")) {
            continue;
        }
        prec = mprAlloc(sizeof(PropRec));
        prec->qname = qname;
        prec->obj = obj;
        prec->slotNum = slotNum;
        prec->trait = trait;
        prec->vp = vp;
        mprAddItem(list, prec);
    }
}


/*
    Generate the entries for class properties. Will be called once for static properties and once for instance properties
 */
static int generateClassPropertyTableEntries(EjsMod *mp, EjsObj *obj, MprList *properties)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsDoc          *doc;
    EjsFunction     *fun;
    PropRec         *prec;
    cchar           *tname;
    int             count, next, attributes;

    ejs = mp->ejs;
    count = 0;

    type = ejsIsType(ejs, obj) ? (EjsType*) obj : 0;

    for (next = 0; (prec = (PropRec*) mprGetNextItem(properties, &next)) != 0; ) {
        vp = prec->vp;
        trait = prec->trait;
        qname = prec->qname;
        if (ejsStartsWithMulti(ejs, qname.space, "internal") || ejsContainsMulti(ejs, qname.space, "private")) {
            continue;
        }
        if (isalpha((int) qname.name->value[0])) {
            out(mp, "<a name='%@'></a>\n", qname.name);
        }
        attributes = trait->attributes;
        if (type && qname.space == type->qname.space) {
            out(mp, "   <tr><td nowrap align='center'>%s</td><td>%@</td>", 
                fmtAttributes(vp, attributes, 0), qname.name);
        } else {
            out(mp, "   <tr><td nowrap align='center'>%s %s</td><td>%@</td>", fmtNamespace(ejs, qname),
                fmtAttributes(vp, attributes, 0), qname.name);
        }
        if (trait->attributes & EJS_TRAIT_GETTER) {
            fun = (EjsFunction*) vp;
            if (fun->resultType) {
                tname = fmtType(ejs, fun->resultType->qname);
                if (scasecmp(tname, "intrinsic::Void") == 0) {
                    out(mp, "<td>&nbsp;</td>");
                } else {
                    out(mp, "<td>%s</td>", fmtTypeReference(ejs, fun->resultType->qname));
                }
            } else {
                out(mp, "<td>&nbsp;</td>");
            }
        } else if (trait->type) {
            out(mp, "<td>%s</td>", fmtTypeReference(ejs, trait->type->qname));
        } else {
            out(mp, "<td>&nbsp;</td>");
        }
        doc = getDoc(ejs, NULL, prec->obj, prec->slotNum);
        if (doc) {
            out(mp, "<td>%s %s</td></tr>\n", doc->brief, doc->description ? doc->description : "");
        } else {
            out(mp, "<td>&nbsp;</td></tr>\n");
        }
        count++;
    }
    return count;
}


static void buildMethodList(EjsMod *mp, MprList *methods, EjsObj *obj, EjsObj *owner, EjsName ownerName)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsFunction     *fun;
    EjsDoc          *doc;
    EjsType         *type;
    FunRec          *fp;
    int             slotNum, numProp, numInherited;

    ejs = mp->ejs;
    if (ejsIsType(ejs, owner) && !ejsIsPrototype(ejs, obj) && ((EjsType*) owner)->hasConstructor) {
        type = (EjsType*) owner;
        slotNum = ejsLookupProperty(ejs, ejs->global, ownerName);
        mprAssert(slotNum >= 0);
        fp = mprAlloc(sizeof(FunRec));
        fp->fun = (EjsFunction*) type;
        fp->obj = ejs->global;
        fp->slotNum = slotNum;
        fp->owner = ejs->global;
        fp->ownerName = type->qname;
        fp->qname = type->qname;
        fp->trait = ejsGetPropertyTraits(ejs, ejs->global, slotNum);
        if (fp->trait) {
            doc = getDoc(ejs, "fun", ejs->global, slotNum);
            if (doc && !doc->hide) {
                mprAddItem(methods, fp);
            }
        }
    }

    numProp = ejsGetLength(ejs, obj);

    numInherited = 0;
    if (ejsIsPrototype(ejs, obj)) {
        numInherited = ((EjsType*) owner)->numInherited;
    }
    slotNum = (obj == ejs->global) ? mp->firstGlobal : numInherited;

    for (; slotNum < numProp; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        trait = ejsGetPropertyTraits(ejs, obj, slotNum);
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        if (ejsIsType(ejs, vp)) {
            doc = getDoc(ejs, "class", obj, slotNum);
            if (doc && doc->hide) {
                continue;
            }
        }
        fun = (EjsFunction*) vp;
        if (trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) {
            continue;
        }
        if (trait) {
            doc = getDoc(ejs, "fun", obj, slotNum);
            if (doc && doc->hide) {
                continue;
            }
        }
        if (vp == 0 || !ejsIsFunction(ejs, vp) || qname.name == 0 || trait == 0) {
            continue;
        }
        if (ejsCompareMulti(ejs, qname.space, EJS_INIT_NAMESPACE) == 0) {
            continue;
        }
        if (ejsCompareMulti(ejs, qname.space, EJS_PRIVATE_NAMESPACE) == 0 || 
                ejsContainsMulti(ejs, qname.space, ",private]")) {
            continue;
        }
        if (ejsStartsWithMulti(ejs, qname.space, "internal")) {
            continue;
        }
        fp = mprAlloc(sizeof(FunRec));
        fp->fun = fun;
        fp->obj = obj;
        fp->slotNum = slotNum;
        fp->owner = owner;
        fp->ownerName = ownerName;
        fp->qname = qname;
        fp->trait = trait;
        mprAddItem(methods, fp);
    }
    mprSortList(methods, compareFunctions);
}


static int generateMethodTable(EjsMod *mp, MprList *methods, EjsObj *obj, int instanceMethods)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait, *argTrait;
    EjsName         qname, argName;
    EjsDoc          *doc;
    EjsFunction     *fun;
    FunRec          *fp;
    cchar           *defaultValue;
    int             i, count, next, emitTable;

    ejs = mp->ejs;
    type = ejsIsType(ejs, obj) ? ((EjsType*) obj) : 0;

    if (instanceMethods) {
        out(mp, "<a name='InstanceMethods'></a>\n");
        out(mp, "<h2 class='classSection'>%@ Instance Methods</h2>\n", 
            (type) ? type->qname.name : ejsCreateStringFromAsc(ejs, "Global"));
    } else {
        out(mp, "<a name='ClassMethods'></a>\n");
        out(mp, "<h2 class='classSection'>%@ Class Methods</h2>\n", 
            (type) ? type->qname.name : ejsCreateStringFromAsc(ejs, "Global"));
    }
    /*
        Output each method
     */
    count = emitTable = 0;
    for (next = 0; (fp = (FunRec*) mprGetNextItem(methods, &next)) != 0; ) {
        qname = fp->qname;
        trait = fp->trait;
        fun = fp->fun;

        if (!emitTable) {
            out(mp, "<table class='apiIndex' summary='methods'>\n");
            out(mp, "   <tr><th>Qualifiers</th><th width='95%%'>Method</th></tr>\n");
            emitTable = 1;
        }

        if (ejsCompareMulti(ejs, qname.space, EJS_INIT_NAMESPACE) == 0) {
            continue;
        }
        if (instanceMethods) {
            if (trait->attributes & EJS_PROP_STATIC) {
                continue;
            }
        } else {
            if (!(trait->attributes & EJS_PROP_STATIC)) {
                continue;
            }
        }

        if (type && qname.space == type->qname.space) {
            out(mp, "   <tr class='apiDef'><td class='apiType'>%s</td>", fmtAttributes(fun, trait->attributes, 0));
        } else {
            out(mp, "   <tr class='apiDef'><td class='apiType'>%s %s</td>", fmtNamespace(ejs, qname), 
                fmtAttributes(fun, trait->attributes, 0));
        }
        out(mp, "<td><a href='#%@'><b>%s</b></a>(", qname.name, demangle(ejs, qname.name));

        doc = getDoc(ejs, "fun", fp->obj, fp->slotNum);

        for (i = 0; i < (int) fun->numArgs; ) {
            argName = ejsGetPropertyName(ejs, fun->activation, i);
            argTrait = ejsGetPropertyTraits(ejs, fun->activation, i);
            if (argTrait->type) {
                out(mp, "%s: %s", fmtDeclaration(ejs, argName), fmtTypeReference(ejs, argTrait->type->qname));
            } else {
                out(mp, "%s", fmtDeclaration(ejs, argName));
            }
            if (doc) {
                defaultValue = getDefault(doc, ejsToMulti(ejs, argName.name));
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
            out(mp, ": %s", fmtTypeReference(ejs, fun->resultType->qname));
        }
        out(mp, "</tr>");

        if (doc) {
            out(mp, "<tr class='apiBrief'><td>&nbsp;</td><td>%s</td></tr>\n", doc->brief);
        }
        count++;
    }
    if (count == 0) {
        out(mp, "   <p>(No own %s methods defined)</p>", instanceMethods ? "instance" : "class");
    }
    out(mp, "</table>\n\n");
    if (type && type->baseType) {
        out(mp, "<p class='inheritedLink'><a href='%s#InstanceMethods'><i>Inherited Methods</i></a></p>\n\n",
            fmtClassUrl(ejs, type->baseType->qname));
    }
    out(mp, "<hr />\n");
    return count;
}


static void generateMethodDetail(EjsMod *mp, MprList *methods)
{
    FunRec      *fp;
    int         next;

    out(mp, "<h2>Method Detail</h2>\n");
    for (next = 0; (fp = (FunRec*) mprGetNextItem(methods, &next)) != 0; ) {
        generateMethod(mp, fp);
    }
}


static void checkArgs(EjsMod *mp, Ejs *ejs, EjsName ownerName, EjsFunction *fun, EjsName qname, EjsDoc *doc)
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
            if (strcmp(key, ejsToMulti(ejs, argName.name)) == 0) {
                break;
            }
        }
        if (param == 0) { 
            if (mp->warnOnError) {
                mprWarn("Missing documentation for parameter \"%@\" in function \"%@\" in type \"%@\"", 
                     argName.name, qname.name, ownerName.name);
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
        if (argName.name && ejsCompareMulti(ejs, argName.name, name) == 0) {
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
    char            *see, *description, *setType;
    int             i, count, next, slotNum;

    ejs = mp->ejs;
    obj = fp->obj;
    slotNum = fp->slotNum;

    type = ejsIsType(ejs, obj) ? (EjsType*) obj : 0;
    fun = (EjsFunction*) ejsGetProperty(ejs, obj, slotNum);
    mprAssert(ejsIsFunction(ejs, fun));

    qname = ejsGetPropertyName(ejs, obj, slotNum);
    trait = ejsGetPropertyTraits(ejs, obj, slotNum);
    
    doc = getDoc(ejs, "fun", obj, slotNum);
    if (doc && doc->hide) {
        return;
    }
    if (doc == 0 || doc->brief == 0) {
        if (mp->warnOnError) {
            if (!ejsIsType(ejs, fun)) {
                /* Don't warn about default constructors */
                if (mp->warnOnError) {
                    mprWarn("Missing documentation for \"%@.%@\"", fp->ownerName.name, qname.name);
                }
            }
        }
        return;
    }
    if (isalpha((int) qname.name->value[0])) {
        out(mp, "<a name='%@'></a>\n", qname.name);
    }
    if (type && qname.space == type->qname.space) {
        out(mp, "<div class='api'>\n");
        out(mp, "<div class='apiSig'>%s %@(", fmtAttributes(fun, trait->attributes, 0), qname.name);

    } else {
        accessorSep = (trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) ? " ": "";
        spaceSep = qname.space->value[0] ? " ": "";
        out(mp, "<div class='api'>\n");
        out(mp, "<div class='apiSig'>%s %s%s %s%s %s(", 
            fmtAttributes(fun, trait->attributes & ~(EJS_TRAIT_GETTER | EJS_TRAIT_SETTER), 0), 
            spaceSep, fmtSpace(ejs, qname), accessorSep, fmtAccessors(trait->attributes), demangle(ejs, qname.name));
    }

    for (i = 0; i < (int) fun->numArgs; ) {
        argName = ejsGetPropertyName(ejs, fun->activation, i);
        argTrait = ejsGetPropertyTraits(ejs, fun->activation, i);
        if (argTrait->type) {
            out(mp, "%s: %s", fmtDeclaration(ejs, argName), fmtTypeReference(ejs, argTrait->type->qname));
        } else {
            out(mp, "%s", fmtDeclaration(ejs, argName));
        }
        if (doc) {
            defaultValue = getDefault(doc, ejsToMulti(ejs, argName.name));
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
        out(mp, ": %s", fmtTypeReference(ejs, fun->resultType->qname));
    }
    out(mp, "\n</div>\n");

    if (doc) {
        out(mp, "<div class='apiDetail'>\n");
        out(mp, "<dl><dt>Description</dt></dd><dd>%s %s</dd></dl>\n", doc->brief, doc->description ? doc->description : "");
        count = mprGetListLength(doc->params);
        if (count > 0) {
            out(mp, "<dl><dt>Parameters</dt>\n");
            out(mp, "<dd><table class='parameters' summary ='parameters'>\n");

            checkArgs(mp, ejs, fp->ownerName, fun, qname, doc);
            for (next = 0; (param = mprGetNextItem(doc->params, &next)) != 0; ) {
                defaultValue = getDefault(doc, param->key);
                i = findArg(ejs, fun, param->key);
                if (i < 0) {
                    mprError("Bad @param reference for \"%s\" in function \"%@\" in type \"%@\"", param->key, 
                        qname.name, fp->ownerName.name);
                } else {
                    argName = ejsGetPropertyName(ejs, fun->activation, i);
                    argTrait = ejsGetPropertyTraits(ejs, fun->activation, i);
                    out(mp, "<tr class='param'><td class='param'>");
                    description = param->value;
                    setType = 0;
                    if (description && description[0] == ':') {
                        setType = stok(sclone(&description[1]), " ", &description);
                    }
                    if (argTrait->type) {
                        out(mp, "%s: %s ", fmtDeclaration(ejs, argName), fmtTypeReference(ejs, argTrait->type->qname));
                    } else if (setType) {
                        out(mp, "%s: %s", fmtDeclaration(ejs, argName), setType);
                    } else {
                        out(mp, "%s ", fmtDeclaration(ejs, argName));
                    }
                    out(mp, "</td><td>%s", description);
                    if (defaultValue) {
                        if (scontains(description, "Not implemented", -1) == NULL) {
                            out(mp, " [default: %s]", defaultValue);
                        }
                    }
                }
                out(mp, "</td></tr>");
            }
            out(mp, "</table></dd>\n");
            out(mp, "</dl>");
        }

        count = mprGetListLength(doc->options);
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
        
        count = mprGetListLength(doc->events);
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

        if (doc->returns) {
            out(mp, "<dl><dt>Returns</dt>\n<dd>%s</dd></dl>\n", doc->returns);
        }
        count = mprGetListLength(doc->throws);
        if (count > 0) {
            out(mp, "<dl><dt>Throws</dt><dd>\n");
            for (next = 0; (thrown = (MprKeyValue*) mprGetNextItem(doc->throws, &next)) != 0; ) {
                //  TODO Functionalize
                ejs->state->bp = ejs->global;
                if ((slotNum = ejsLookupVar(ejs, ejs->global, EN(thrown->key), &lookup)) < 0) {
                    continue;
                }
                throwName = lookup.name;
                out(mp, "<a href='%s'>%s</a>: %s%s\n", getFilename(fmtType(ejs, throwName)), thrown->key,
                    thrown->value, (count == next) ? "" : ", ");
            }
            out(mp, "</dd>\n");
            out(mp, "</dl>");
        }
        if (doc->requires) {
            out(mp, "<dl><dt>Requires</dt>\n<dd>configure --ejs-%s</dd></dl>\n", doc->requires);
        }
        if (doc->spec) {
            out(mp, "<dl><dt>Specified</dt>\n<dd>%s</dd></dl>\n", doc->spec);
        }
        if (doc->stability) {
            out(mp, "<dl><dt>Stability</dt>\n<dd>%s</dd></dl>\n", doc->stability);
        }
        if (doc->example) {
            out(mp, "<dl><dt>Example</dt>\n<dd><pre>%s</pre></dd></dl>\n", doc->example);
        }
        count = mprGetListLength(doc->see);
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


static char *fmtAttributes(EjsAny *vp, int attributes, int klass)
{
    static char attributeBuf[MPR_MAX_STRING];

    attributeBuf[0] = '\0';

    /*
        Order to look best
     */
    if (attributes & EJS_PROP_STATIC) {
        strcat(attributeBuf, "static ");
    }
    /* Types are can also be constructor functions. Need klass parameter to differentiate */
    if (ejsIsType(ejs, vp) && klass) {
        if (attributes & EJS_TYPE_FINAL) {
            strcat(attributeBuf, "final ");
        }
        if (attributes & EJS_TYPE_DYNAMIC_INSTANCES) {
            strcat(attributeBuf, "dynamic ");
        }
    } else if (ejsIsFunction(ejs, vp)) {
        if (attributes & EJS_FUN_OVERRIDE) {
            strcat(attributeBuf, "override ");
        }
        if (attributes & EJS_TRAIT_GETTER) {
            strcat(attributeBuf, "get ");
        }
        if (attributes & EJS_TRAIT_SETTER) {
            strcat(attributeBuf, "set ");
        }
    } else {
        if (attributes & EJS_TRAIT_READONLY) {
            strcat(attributeBuf, "const ");
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


static MprChar *joinLines(MprChar *str)
{
    MprChar    *cp, *np;

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
static MprChar *mergeDuplicates(Ejs *ejs, EjsMod *mp, EjsName qname, EjsDoc *doc, MprChar *spec)
{
    EjsDoc      *dup;
    MprChar     *next, *duplicate, *mark;

    if ((next = mcontains(spec, "@duplicate", -1)) == 0) {
        return spec;
    }
    next = spec = wclone(spec);

    while ((next = mcontains(next, "@duplicate", -1)) != 0) {
        mark = next;
        mtok(next, " \t\n\r", &next);
        if ((duplicate = mtok(next, " \t\n\r", &next)) == 0) {
            break;
        }
        if ((dup = getDuplicateDoc(ejs, duplicate)) == 0) {
            mprError("Can't find @duplicate directive %s for %s", duplicate, qname.name);
        } else {
            crackDoc(mp, dup, WEN(duplicate));
            mprCopyListContents(doc->params, dup->params);
            mprCopyListContents(doc->options, dup->options);
            mprCopyListContents(doc->events, dup->events);
            mprCopyListContents(doc->see, dup->see);
            mprCopyListContents(doc->throws, dup->throws);
            doc->brief = dup->brief;
            doc->description = dup->description;
            doc->example = dup->example;
            doc->requires = dup->requires;
            doc->returns = dup->returns;
            doc->stability = dup->stability;
            doc->spec = dup->spec;
        }
        memmove(mark, next, wlen(next) + 1);
        next = mark;
    }
    return spec;
}


/*
    Cleanup text. Remove leading comments and "*" that are part of the comment and not part of the doc.
 */
static void prepText(MprChar *str)
{
    MprChar     *dp, *cp;

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


static EjsDoc *crackDoc(EjsMod *mp, EjsDoc *doc, EjsName qname)
{
    Ejs         *ejs;
    EjsDoc      *dup;
    MprKeyValue *pair;
    MprChar     *value, *key, *line, *str, *next, *cp, *token, *nextWord, *word, *duplicate;
    MprChar     *thisBrief, *thisDescription, *start;

    ejs = mp->ejs;

    if (doc->cracked) {
        return doc;
    }
    doc->cracked = 1;
    doc->params = mprCreateList(0, 0);
    doc->options = mprCreateList(0, 0);
    doc->events = mprCreateList(0, 0);
    doc->defaults = mprCreateList(0, 0);
    doc->see = mprCreateList(0, 0);
    doc->throws = mprCreateList(0, 0);

    str = mprMemdup(doc->docString->value, doc->docString->length);
    if (str == NULL) {
        return doc;
    }
    prepText(str);
    if (mcontains(str, "@hide", -1) || mcontains(str, "@hidden", -1)) {
        doc->hide = 1;
    } else if (mcontains(str, "@deprecate", -1) || mcontains(str, "@deprecated", -1)) {
        doc->deprecated = 1;
    }
    str = mergeDuplicates(ejs, mp, qname, doc, str);

    thisDescription = NULL;
    thisBrief = NULL;
    next = str;
    
    if (str[0] != '@') {
        if ((thisBrief = mtok(str, "@", &next)) == 0) {
            thisBrief = NULL;
        } else {
            for (cp = thisBrief; *cp; cp++) {
                if (*cp == '.' && (isspace((int) cp[1]) || *cp == '*')) {
                    cp++;
                    *cp++ = '\0';
                    thisDescription = mtrim(cp, " \t\n", MPR_TRIM_BOTH);
                    break;
                }
            }
        }
    }
    doc->brief = wjoin(doc->brief, thisBrief, NULL);
    doc->description = wjoin(doc->description, thisDescription, NULL);
    mtrim(doc->brief, " \t\r\n", MPR_TRIM_BOTH);
    mtrim(doc->description, " \t\r\n", MPR_TRIM_BOTH);
    mprAssert(doc->brief);
    mprAssert(doc->description);

    /*
        This is what we are parsing:
        One liner is the first sentance. Rest of description can continue from here and can include embedded html
 
        @param argName Description          (Up to next @, case matters on argName)
        @default argName DefaultValue       (Up to next @, case matters on argName)
        @return Sentence                    (Can use return or returns. If sentance starts with lower case, then start 
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
        token = next;
        for (cp = next; cp ; ) {
            mtok(cp, "@", &cp);
            if (cp && cp[-2] == '\\') {
                cp[-1] = '@';
                cp[-2] = ' ';
            } else {
                next = cp;
                break;
            }
        }
        line = skipAtWord(token);

        if (token > &start[2] && token[-2] == '\\') {
            continue;
        }
        if (match(token, "duplicate")) {
            duplicate = mtrim(line, " \t\n", MPR_TRIM_BOTH);
            if ((dup = getDuplicateDoc(ejs, duplicate)) == 0) {
                mprError("Can't find @duplicate directive %s for %@", duplicate, qname.name);
            } else {
                crackDoc(mp, dup, WEN(duplicate));
                mprCopyListContents(doc->params, dup->params);
                mprCopyListContents(doc->options, dup->options);
                mprCopyListContents(doc->events, dup->events);
                mprCopyListContents(doc->see, dup->see);
                mprCopyListContents(doc->throws, dup->throws);
                doc->brief = mrejoin(doc->brief, " ", dup->brief, NULL);
                doc->description = mrejoin(doc->description, " ", dup->description, NULL);
                if (dup->example) {
                    if (doc->example) {
                        doc->example = mrejoin(doc->example, " ", dup->example, NULL);
                    } else {
                        doc->example = dup->example;
                    }
                }
                if (dup->requires) {
                    if (doc->requires) {
                        doc->requires = mrejoin(doc->requires, " ", dup->requires, NULL);
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
                pair = createKeyPair(key, value);
                mprAddItem(doc->events, pair);
            }

        } else if (match(token, "option")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            if (key && *key) {
                pair = createKeyPair(key, value);
                mprAddItem(doc->options, pair);
            }

        } else if (match(token, "param")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            if (key && *key) {
                key = mtrim(key, ".", MPR_TRIM_BOTH);
                pair = createKeyPair(key, value);
                mprAddItem(doc->params, pair);
            }

        } else if (match(token, "default")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            if (key && *key) {
                pair = createKeyPair(key, value);
                mprAddItem(doc->defaults, pair);
            }

        } else if (match(token, "deprecated")) {
            doc->hide = 1;
            doc->deprecated = 1;
            doc->stability = amtow("deprecated", NULL);

        } else if (match(token, "hide") || match(token, "hidden")) {
            doc->hide = 1;

        } else if (match(token, "spec")) {
            doc->spec = mtrim(line, " \t\n", MPR_TRIM_BOTH);
            wlower(doc->spec);

        } else if (match(token, "stability")) {
            doc->stability = mtrim(line, " \t", MPR_TRIM_BOTH);
            wlower(doc->stability);

        } else if (match(token, "requires")) {
            doc->requires = mtrim(line, " \t", MPR_TRIM_BOTH);
            wlower(doc->requires);

        } else if (match(token, "return") || match(token, "returns")) {
            line = joinLines(line);
            doc->returns = mtrim(line, " \t", MPR_TRIM_BOTH);

        } else if (match(token, "throw") || match(token, "throws")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            pair = createKeyPair(key, value);
            mprAddItem(doc->throws, pair);

        } else if (match(token, "see") || match(token, "seeAlso")) {
            nextWord = line;
            do {
                word = nextWord;
                mtok(word, " \t\r\n", &nextWord);
                mprAddItem(doc->see, mtrim(word, " \t", MPR_TRIM_BOTH));
            } while (nextWord && *nextWord);
        }
    }
    fixupDoc(ejs, doc);
    return doc;
}


static MprChar *fixSentence(MprChar *str)
{
    MprChar     *buf;
    size_t      len;

    if (str == 0 || *str == '\0') {
        return 0;
    }
    
    /*
        Copy the string and grow by 1 byte (plus null) to allow for a trailing period.
     */
    len = wlen(str) + 2 * sizeof(MprChar);
    if ((buf = mprAlloc(len)) == 0) {
        return 0;
    }
    wcopy(buf, len, str);
    str = buf;
    str[0] = toupper((int) str[0]);

    /*
        Append a "." if the string does not appear to contain HTML tags
     */
    if (mcontains(str, "</", -1) == 0) {
        /* Trim period and re-add */
        str = mtrim(str, " \t\r\n.", MPR_TRIM_BOTH);
        len = wlen(str);
        if (str[len - 1] != '.') {
            str[len] = '.';
            str[len + 1] = '\0';
        }
    } else {
        str = mtrim(str, " \t\r\n", MPR_TRIM_BOTH);
    }
    return str;
}


static MprChar *formatExample(Ejs *ejs, EjsString *docString)
{
    MprChar     *example, *cp, *end, *buf, *dp;
    int         i, indent;

    if ((example = mcontains(docString->value, "@example", -1)) != 0) {
        example += 8;
        for (cp = example; *cp && *cp != '\n'; cp++) {}
        if (*cp == '\n') {
            cp++;
        }
        example = cp;
        for (end = example; *end; end++) {
            if (*end == '@' && (end == example || end[-1] != '\\')) {
                break;
            }
        }
        for (indent = 0; *cp == '\t' || *cp == ' '; indent++, cp++) {}

        buf = mprAlloc(wlen(example) * 4 + 2);
        for (cp = example, dp = buf; *cp && cp < end; ) {
            for (i = 0; i < indent && *cp && isspace((int) *cp) && *cp != '\n'; i++, cp++) {}
            for (; *cp && *cp != '\n'; ) {
                if (*cp == '<' && cp[1] == '%') {
                    mtow(dp, 5, "&lt;", 4);
                    dp += 4;
                    cp++;
                    *dp++ = *cp++;
                } else if (*cp == '%' && cp[1] == '>') {
                    *dp++ = *cp++;
                    mtow(dp, 5, "&gt;", 4);
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
        for (--dp; dp > example && isspace((int) *dp); dp--) {}
        *++dp = '\0';
        return buf;
    }
    return NULL;
}


static MprChar *wikiFormat(Ejs *ejs, MprChar *start)
{
    EjsLookup   lookup;
    EjsName     qname;
    MprBuf      *buf;
    MprChar     *end, *cp, *klass, *property, *str, *pref, *space;
    ssize       len;
    int         slotNum, sentence;

    if (start == 0 || *start == '\0') {
        return NULL;
    }
    buf = mprCreateBuf(-1, -1);
    end = &start[wlen(start)];
    
    for (str = start; str < end && *str; str++) {
        /*
            FUTURE -- expand this to support basic markdown
         */
        if (str[0] == '\n' && str[1] == '\n') {
            /* Two blank lines forces a blank line in the output */
            mprPutStringToWideBuf(buf, "<br/><br/>");
            str++;

        } else if (*str == '$') {
            if (str[1] == '$') {
                mprPutCharToWideBuf(buf, *str);
                continue;
            }
            if ((str > start && (str[-1] == '$' || str[-1] == '\\'))) {
                /* Remove backquote */
                mprAdjustBufEnd(buf, - (int) sizeof(MprChar));
                mprPutCharToWideBuf(buf, *str);
                continue;
            }
            /* Dollar reference expansion */
            klass = &str[1];
            for (cp = &str[1]; *cp; cp++) {
                if (isspace((int) *cp)) {
                    break;
                }
            }
            len = cp - str;
            str = cp;
            if (isspace((int) *cp)) {
                cp--;
            }
            klass = snclone(klass, len);
            sentence = (klass[wlen(klass) - 1] == '.');
            mprAssert(strcmp(klass, "ejs.web::Request") != 0);

            if (scontains(klass, "::", -1)) {
                space = stok(klass, "::", &klass);
            } else {
                space = "";
            }
            if ((property = wchr(klass, '.')) != 0) {
                *property++ = '\0';
                if (*property == '\0') {
                    property = klass;
                    klass = 0;
                }
            } else {
                property = klass;
                klass = 0;
            }
            pref = strim(property, "(), \t", MPR_TRIM_END);
            klass = mtrim(klass, "., \t", MPR_TRIM_BOTH);
            property = mtrim(property, "., \t", MPR_TRIM_BOTH);

            if (klass) {
                //  TODO Functionalize
                ejs->state->bp = ejs->global;
                if ((slotNum = ejsLookupVar(ejs, ejs->global, N(space, klass), &lookup)) < 0) {
                    if (klass) {
                        mprPutFmtToWideBuf(buf, "%s.%s", klass, property);
                    } else {
                        mprPutStringToBuf(buf, property);
                    }
                } else {
                    qname = lookup.name;
                    if (property) {
                        mprPutFmtToWideBuf(buf, "<a href='%s#%s'>%s.%s</a>", getFilename(fmtType(ejs, qname)), 
                            pref, klass, property);
                    } else {
                        mprPutFmtToWideBuf(buf, "<a href='%s'>%s</a>", getFilename(fmtType(ejs, qname)), klass);
                    }
                }
            } else {
                mprPutFmtToWideBuf(buf, "<a href='#%s'>%s</a>", pref, property);
            }
            if (sentence) {
                mprPutCharToWideBuf(buf, '.');
            }
            mprPutCharToWideBuf(buf, ' ');

        } else {
            mprPutCharToWideBuf(buf, *str);
        }
    }
    mprAddNullToWideBuf(buf);
    return (MprChar*) mprGetBufStart(buf);
}


static void fixupDoc(Ejs *ejs, EjsDoc *doc)
{
    MprKeyValue     *pair;
    int             next;

    doc->brief = wikiFormat(ejs, fixSentence(doc->brief));
    doc->description = wikiFormat(ejs, fixSentence(doc->description));
    doc->returns = wikiFormat(ejs, fixSentence(doc->returns));
    doc->stability = fixSentence(doc->stability);
    doc->requires = wikiFormat(ejs, fixSentence(doc->requires));
    if (doc->spec) {
        if (mcmp(doc->spec, "ejs") == 0) {
            doc->spec = mfmt("ejscript-%d.%d", BLD_MAJOR_VERSION, BLD_MINOR_VERSION);
        }
    } else {
        doc->spec = NULL;
    }
    if (doc->example == 0) {
        doc->example = NULL;
    }
    for (next = 0; (pair = mprGetNextItem(doc->events, &next)) != 0; ) {
        pair->value = wikiFormat(ejs, fixSentence(pair->value));
    }
    for (next = 0; (pair = mprGetNextItem(doc->options, &next)) != 0; ) {
        pair->value = wikiFormat(ejs, fixSentence(pair->value));
    }
    for (next = 0; (pair = mprGetNextItem(doc->params, &next)) != 0; ) {
        pair->value = wikiFormat(ejs, fixSentence(pair->value));
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
        mp->file = mprOpenFile(mp->path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (mp->file == 0) {
            mprError("Can't open %s", mp->path);
            mp->errorCount++;
            return;
        }
    }
    va_start(args, fmt);
    buf = sfmtv(fmt, args);
    if (mprWriteFileString(mp->file, buf) < 0) {
        mprError("Can't write to buffer");
    }
}


static EjsString *fmtModule(Ejs *ejs, EjsString *name)
{
    if (ejsCompareMulti(ejs, name, EJS_DEFAULT_MODULE) == 0) {
        return ESV(empty);
    }
    return name;
}


static char *fmtClassUrl(Ejs *ejs, EjsName qname)
{
    return getFilename(fmtType(ejs, qname));
}


static char *fmtNamespace(Ejs *ejs, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    cchar       *space;
    char        *cp;

    space = ejsToMulti(ejs, qname.space);
    if (space[0] == '[') {
        scopy(buf, sizeof(buf), &space[1]);

    } else {
        scopy(buf, sizeof(buf), space);
    }
    if (buf[strlen(buf) - 1] == ']') {
        buf[strlen(buf) - 1] = '\0';
    }
    if (strcmp(buf, "ejs") == 0) {
        buf[0] = '\0';

    } else if (strcmp(buf, "public") == 0) {
        buf[0] = '\0';

    } else if ((cp = strrchr(buf, ',')) != 0) {
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


static char *fmtType(Ejs *ejs, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *namespace;

    namespace = fmtNamespace(ejs, qname);

    if (strcmp(namespace, EJS_PUBLIC_NAMESPACE) == 0) {
        *namespace = '\0';
    }
    if (*namespace) {
        if (*namespace) {
            mprSprintf(buf, sizeof(buf), "%s::%@", namespace, qname.name);
        } else {
            mprSprintf(buf, sizeof(buf), "%@", qname.name);
        }
    } else {
        mprSprintf(buf, sizeof(buf), "%@", qname.name);
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

    scopy(buf, sizeof(buf), name);

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
    scopy(&buf[strlen(buf)], sizeof(buf) - (int) strlen(buf) - 1, ".html");
    return buf;
}


static char *fmtTypeReference(Ejs *ejs, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *typeName;

    typeName = fmtType(ejs, qname);
    mprSprintf(buf, sizeof(buf), "<a href='%s'>%@</a>", getFilename(typeName), qname.name);
    return buf;
}


static char *fmtSpace(Ejs *ejs, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *namespace;

    namespace = fmtNamespace(ejs, qname);
    if (namespace[0]) {
        mprSprintf(buf, sizeof(buf), "%@", qname.space);
    } else {
        buf[0] = '\0';
    }
    return buf;
}


static char *fmtDeclaration(Ejs *ejs, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *namespace;

    namespace = fmtNamespace(ejs, qname);
    if (namespace[0]) {
        mprSprintf(buf, sizeof(buf), "%@ %s", qname.space, demangle(ejs, qname.name));
    } else {
        mprSprintf(buf, sizeof(buf), "%s", demangle(ejs, qname.name));
    }
    return buf;
}


static bool match(MprChar *last, cchar *key)
{
    int     len;

    mprAssert(last);
    mprAssert(key && *key);

    len = (int) strlen(key);
    return mncmp(last, key, len) == 0;
}


static MprChar *skipAtWord(MprChar *str)
{
    while (!isspace((int) *str) && *str)
        str++;
    while (isspace((int) *str))
        str++;
    return str;
}


static void getKeyValue(MprChar *str, MprChar **key, MprChar **value)
{
    MprChar     *end;

    for (end = str; *end && !isspace((int) *end); end++)
        ;
    if (end) {
        *end = '\0';
    }
    if (key) {
        *key = mtrim(str, " \t", MPR_TRIM_BOTH);
    }
    for (str = end + 1; *str && isspace((int) *str); str++) {
        ;
    }
    if (value) {
        *value = mtrim(str, " \t", MPR_TRIM_BOTH);
    }
}


static int compareProperties(PropRec **p1, PropRec **p2)
{
    return compareStrings(&(*p1)->qname.name, &(*p2)->qname.name);
}


static int compareFunctions(FunRec **f1, FunRec **f2)
{
    return compareStrings(&(*f1)->qname.name, &(*f2)->qname.name);
}


static int compareClasses(ClassRec **c1, ClassRec **c2)
{
    return compareStrings(&(*c1)->qname.name, &(*c2)->qname.name);
}


static cchar *demangle(Ejs *ejs, EjsString *name)
{
    return ejsToMulti(ejs, name);
}


static cchar *demangleCESV(cchar *name)
{
    return name;
}


static int compareNames(char **q1, char **q2)
{
    cchar    *s1, *s2, *cp;

    s1 = demangleCESV(*q1);
    s2 = demangleCESV(*q2);

    /*
        Don't sort on the namespace portions of the name
     */
    if ((cp = strrchr(s1, ':')) != 0) {
        s1 = cp + 1;
    }
    if ((cp = strrchr(s2, ':')) != 0) {
        s2 = cp + 1;
    }
    return scmp(s1, s2);
}


static int compareStrings(EjsString **q1, EjsString **q2)
{
    cchar    *s1, *s2, *cp;

    s1 = demangle(ejs, *q1);
    s2 = demangle(ejs, *q2);

    /*
        Don't sort on the namespace portions of the name
     */
    if ((cp = strrchr(s1, ':')) != 0) {
        s1 = cp + 1;
    }
    if ((cp = strrchr(s2, ':')) != 0) {
        s2 = cp + 1;
    }
    return scmp(s1, s2);
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
    mprAddItem(list, sclone(item));
}


static void addUniqueClass(MprList *list, ClassRec *item)
{
    ClassRec    *p;
    int         next;

    if (item == 0) {
        return;
    }
    for (next = 0; (p = (ClassRec*) mprGetNextItem(list, &next)) != 0; ) {
        if (p->qname.name == item->qname.name) {
            if (p->qname.space == item->qname.space) {
                return;
            }
        }
    }
    mprAddItem(list, item);
}


static EjsDoc *getDoc(Ejs *ejs, cchar *tag, void *obj, int slotNum)
{
    EjsObj      *vp;
    char        key[32];

    vp = ejsGetProperty(ejs, obj, slotNum);
    if (tag == 0) {
        if (ejsIsType(ejs, vp)) {
            tag = "class";
        } else if (ejsIsFunction(ejs, vp)) {
            tag = "fun";
        } else {
            tag = "var";
        }
    }
    mprSprintf(key, sizeof(key), "%s %Lx %d", tag, PTOL(obj), slotNum);
    return mprLookupKey(ejs->doc, key);
}


static EjsDoc *getDuplicateDoc(Ejs *ejs, MprChar *duplicate)
{
    EjsDoc          *doc;
    EjsObj          *vp;
    EjsLookup       lookup;
    MprChar         *space, *klass, *property;
    int             slotNum;

    space = wclone(duplicate);
    if ((klass = mcontains(space, "::", -1)) != 0) {
        *klass = '\0';
        klass += 2;
    } else {
        klass = space;
        space = "";
    }
    if ((property = wchr(klass, '.')) != 0) {
        *property++ = '\0';
    }
    //  TODO Functionalize
    ejs->state->bp = ejs->global;
    if ((slotNum = ejsLookupVar(ejs, ejs->global, WN(space, klass), &lookup)) < 0) {
        return 0;
    }
    if (property == 0) {
        doc = getDoc(ejs, NULL, ejs->global, slotNum);
    } else {
        vp = ejsGetProperty(ejs, ejs->global, slotNum);
        if ((slotNum = ejsLookupVar(ejs, vp, WEN(property), &lookup)) < 0) {
            if (ejsIsType(ejs, vp)) {
                vp = (EjsObj*) ((EjsType*) vp)->prototype;
                if ((slotNum = ejsLookupVar(ejs, vp, WEN(property), &lookup)) < 0) {
                    return 0;
                }
            }
        }
        doc = getDoc(ejs, NULL, (EjsBlock*) vp, slotNum);
    }
    if (doc) {
        if (doc->docString == NULL || doc->docString->value[0] == '\0') {
            mprError("Duplicate entry \"%s\" provides no description", duplicate);
            return 0;
        }
    }
    return doc;
}


static MprKeyValue *createKeyPair(MprChar *key, MprChar *value)
{
    MprKeyValue     *pair;
    
    pair = mprAlloc(sizeof(MprKeyValue));
    if (pair == 0) {
        return 0;
    }
    pair->key = wclone(key);
    pair->value = mtrim(wclone(value), " ", MPR_TRIM_BOTH);
    return pair;
}


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
