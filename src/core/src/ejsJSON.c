/**
    ejsJSON.c - JSON encoding and decoding

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Locals ***********************************/

typedef struct JsonState {
    MprChar    *data;
    MprChar    *end;
    MprChar    *next;
    MprChar    *error;
} JsonState;

typedef struct Json {
    MprBuf      *buf;
    EjsObj      *current;
    EjsObj      *options;           /* Options object */
    EjsFunction *replacer;
    char        *indent;
    int         baseClasses;
    int         depth;
    int         hidden;
    int         namespaces;
    int         pretty;
    int         nest;              /* Json serialize nest level */
} Json;

/***************************** Forward Declarations ***************************/

static EjsObj *parseLiteral(Ejs *ejs, JsonState *js);
static EjsObj *parseLiteralInner(Ejs *ejs, MprBuf *buf, JsonState *js);
static EjsString *serialize(Ejs *ejs, EjsAny *vp, Json *json);

/*********************************** Locals ***********************************/
/*
    function deserialize(obj: String, options: Object): Object
 */
EjsObj *g_deserialize(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    mprAssert(argc >=1);
    return ejsDeserialize(ejs, (EjsString*) argv[0]);
}

/*
    function serialize(obj: Object, options: Object = null): String
        Options: baseClasses, depth, indent, hidden, pretty, replacer
 */
static EjsString *g_serialize(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsToJSON(ejs, argv[0], (argc == 2) ? argv[1] : NULL);
}


EjsAny *ejsDeserialize(Ejs *ejs, EjsString *str)
{
    EjsObj      *obj;
    JsonState   js;

    if (!ejsIs(ejs, str, String)) {
        ejsThrowSyntaxError(ejs, "Object is not a string");
        return 0;
    }
    if (str->length == 0) {
        return ESV(empty);
    }
    js.next = js.data = str->value;
    js.end = &js.data[str->length];
    js.error = 0;
    if ((obj = parseLiteral(ejs, &js)) == 0) {
        if (js.error) {
            ejsThrowSyntaxError(ejs, 
                "Can't parse object literal. Error at position %d.\n"
                "===========================\n"
                "Offending text: %w\n"
                "===========================\n"
                "In literal %w"
                "\n===========================\n",
                (int) (js.error - js.data), js.error, js.data);
        } else {
            ejsThrowSyntaxError(ejs, "Can't parse object literal. Undefined error");
        }
        return 0;
    }
    return obj;
}


static EjsObj *parseLiteral(Ejs *ejs, JsonState *js)
{
    mprAssert(js);

    return parseLiteralInner(ejs, mprCreateBuf(0, 0), js);
}


typedef enum Token {
    TOK_ERR,            /* Error */
    TOK_EOF,            /* End of input */
    TOK_LBRACE,         /* { */
    TOK_LBRACKET,       /* [ */
    TOK_RBRACE,         /* } */
    TOK_RBRACKET,       /* ] */
    TOK_COLON,          /* : */
    TOK_COMMA,          /* , */
    TOK_ID,             /* Unquoted ID */
    TOK_QID,            /* Quoted ID */
} Token;


static MprChar *skipComments(MprChar *cp, MprChar *end)
{
    int     inComment;

    for (; cp < end && isspace((int) *cp); cp++) {}

    while (cp < &end[-1]) {
        if (cp < &end[-1] && *cp == '/' && cp[1] == '*') {
            inComment = 1;
            for (cp += 2; cp < &end[-1]; cp++) {
                if (*cp == '*' && cp[1] == '/') {
                    inComment = 0;
                    cp += 2;
                    break;
                }
            }
            if (inComment) {
                return 0;
            }
            while (cp < end && isspace((int) *cp)) cp++;

        } else  if (cp < &end[-1] && *cp == '/' && cp[1] == '/') {
            inComment = 1;
            for (cp += 2; cp < end; cp++) {
                if (*cp == '\n') {
                    inComment = 0;
                    cp++;
                    break;
                }
            }
            if (inComment) {
                return 0;
            }
            while (cp < end && isspace((int) *cp)) cp++;

        } else {
            break;
        }
    }
    return cp;
}


Token getNextJsonToken(MprBuf *buf, MprChar **token, JsonState *js)
{
    MprChar     *start, *cp, *end, *next;
    MprChar     *src, *dest;
    int         quote, tid, c;

    if (buf) {
        mprFlushBuf(buf);
    }
    cp = js->next;
    end = js->end;
    cp = skipComments(cp, end);
    next = cp + 1;
    quote = -1;

    if (*cp == '\0') {
        tid = TOK_EOF;

    } else  if (*cp == '{') {
        tid = TOK_LBRACE;

    } else if (*cp == '[') {
        tid = TOK_LBRACKET;

    } else if (*cp == '}' || *cp == ']') {
        tid = *cp == '}' ? TOK_RBRACE: TOK_RBRACKET;
        while (*++cp && isspace((int) *cp)) ;
        if (*cp == ',' || *cp == ':') {
            cp++;
        }
        next = cp;

    } else {
        if (*cp == '"' || *cp == '\'') {
            tid = TOK_QID;
            quote = *cp++;
            for (start = cp; cp < end; cp++) {
                if (*cp == '\\') {
                    if (cp[1] == quote) {
                        cp++;
                    }
                    continue;
                }
                if (*cp == quote) {
                    break;
                }
            }
            if (*cp != quote) {
                js->error = cp;
                return TOK_ERR;
            }
            if (buf) {
                mprPutBlockToBuf(buf, (char*) start, (cp - start));
            }
            cp++;

        } else if (*cp == '/') {
            tid = TOK_ID;
            for (start = cp++; cp < end; cp++) {
                if (*cp == '\\') {
                    if (cp[1] == '/') {
                        cp++;
                    }
                    continue;
                }
                if (*cp == '/') {
                    break;
                }
            }
            if (*cp != '/') {
                js->error = cp;
                return TOK_ERR;
            }
            if (buf) {
                mprPutBlockToBuf(buf, (char*) start, (cp - start));
            }
            cp++;

        } else {
            tid = TOK_ID;
            for (start = cp; cp < end; cp++) {
                if (*cp == '\\') {
                    continue;
                }
                /* Not an allowable character outside quotes */
                if (!(isalnum((int) *cp) || *cp == '_' || *cp == ' ' || *cp == '-' || *cp == '+' || *cp == '.')) {
                    break;
                }
            }
            if (buf) {
                mprPutBlockToBuf(buf, (char*) start, (int) (cp - start));
            }
        }
        if (buf) {
            mprAddNullToBuf(buf);
        }
        if (*cp == ',' || *cp == ':') {
            cp++;
        } else if (*cp != '}' && *cp != ']' && *cp != '\0' && *cp != '\n' && *cp != '\r' && *cp != ' ') {
            js->error = cp;
            return TOK_ERR;
        }
        next = cp;

        if (buf) {
            for (dest = src = (MprChar*) buf->start; src < (MprChar*) buf->end; ) {
                c = *src++;
                if (c == '\\') {
                    c = *src++;
                    if (c == 'r') {
                        c = '\r';
                    } else if (c == 'n') {
                        c = '\n';
                    } else if (c == 'b') {
                        c = '\b';
                    }
                }
                *dest++ = c;
            }
            *dest = '\0';
            *token = (MprChar*) mprGetBufStart(buf);
        }
    }
    js->next = next;
    return tid;
}


Token peekNextJsonToken(JsonState *js)
{
    JsonState   discard = *js;
    return getNextJsonToken(NULL, NULL, &discard);
}


/*
    Parse an object literal string pointed to by js->next into the given buffer. Update js->next to point
    to the next input token in the object literal. Supports nested object literals.
 */
static EjsObj *parseLiteralInner(Ejs *ejs, MprBuf *buf, JsonState *js)
{
    EjsAny      *obj, *vp;
    MprBuf      *valueBuf;
    MprChar     *token, *key, *value;
    int         tid, isArray;

    isArray = 0;

    tid = getNextJsonToken(buf, &token, js);
    if (tid == TOK_ERR || tid == TOK_EOF) {
        return 0;
    }
    if (tid == TOK_LBRACKET) {
        isArray = 1;
        obj = (EjsObj*) ejsCreateArray(ejs, 0);
    } else if (tid == TOK_LBRACE) {
        obj = ejsCreateEmptyPot(ejs);
    } else {
        return ejsParse(ejs, token, S_String);
    }
    if (obj == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    while (1) {
        vp = 0;
        tid = peekNextJsonToken(js);
        if (tid == TOK_ERR) {
            return 0;
        } else if (tid == TOK_EOF) {
            break;
        } else if (tid == TOK_RBRACE || tid == TOK_RBRACKET) {
            getNextJsonToken(buf, &key, js);
            break;
        }
        if (tid == TOK_LBRACKET) {
            /* For array values */
            vp = parseLiteral(ejs, js);
            mprAssert(vp);
            
        } else if (tid == TOK_LBRACE) {
            /* For object values */
            vp = parseLiteral(ejs, js);
            mprAssert(vp);
            
        } else if (isArray) {
            tid = getNextJsonToken(buf, &value, js);
            vp = ejsParse(ejs, value, (tid == TOK_QID) ? S_String: -1);
            mprAssert(vp);
            
        } else {
            getNextJsonToken(buf, &key, js);
            tid = peekNextJsonToken(js);
            if (tid == TOK_ERR) {
                return 0;
            } else if (tid == TOK_EOF) {
                break;
            } else if (tid == TOK_LBRACE || tid == TOK_LBRACKET) {
                vp = parseLiteral(ejs, js);

            } else if (tid == TOK_ID || tid == TOK_QID) {
                valueBuf = mprCreateBuf(0, 0);
                getNextJsonToken(valueBuf, &value, js);
                if (tid == TOK_QID) {
                    vp = ejsCreateString(ejs, value, strlen(value));
                } else {
                    if (mcmp(value, "null") == 0) {
                        vp = ESV(null);
                    } else if (mcmp(value, "undefined") == 0) {
                        vp = ESV(undefined);
                    } else {
                        vp = ejsParse(ejs, value, -1);
                    }
                }
                mprAssert(vp);
            } else {
                getNextJsonToken(buf, &value, js);
                js->error = js->next;
                return 0;
            }
        }
        if (vp == 0) {
            js->error = js->next;
            return 0;
        }
        if (isArray) {
            if (ejsSetProperty(ejs, obj, -1, vp) < 0) {
                ejsThrowMemoryError(ejs);
                return 0;
            }
        } else {
            if (ejsSetPropertyByName(ejs, obj, WEN(key), vp) < 0) {
                ejsThrowMemoryError(ejs);
                return 0;
            }
        }
    }
    return obj;
}


/**
    Get a serialized string representation of a variable using JSON encoding.
    This will look for a "toJSON" function on the specified object. Use ejsSerialize for low level JSON.
    @return Returns a string variable or null if an exception is thrown.
 */
EjsString *ejsToJSON(Ejs *ejs, EjsAny *vp, EjsObj *options)
{
    EjsFunction     *fn;
    EjsString       *result;
    EjsObj          *argv[1];
    int             argc;

    fn = (EjsFunction*) ejsGetPropertyByName(ejs, TYPE(vp)->prototype, N(NULL, "toJSON"));
    if (!ejsIsFunction(ejs, fn) || (fn->isNativeProc && fn->body.proc == (EjsProc) ejsObjToJSON)) {
        result = ejsSerializeWithOptions(ejs, vp, options);
    } else {
        argv[0] = options;
        argc = options ? 1 : 0;
        result = (EjsString*) ejsRunFunction(ejs, fn, vp, argc, argv);
    }
    return result;
}


EjsString *ejsSerializeWithOptions(Ejs *ejs, EjsAny *vp, EjsObj *options)
{
    Json        json;
    EjsObj      *arg;
    EjsString   *result;
    int         i;

    memset(&json, 0, sizeof(Json));
    json.depth = 99;

    if (options) {
        json.options = options;
        if ((arg = ejsGetPropertyByName(ejs, options, EN("baseClasses"))) != 0) {
            json.baseClasses = (arg == ESV(true));
        }
        if ((arg = ejsGetPropertyByName(ejs, options, EN("depth"))) != 0) {
            json.depth = ejsGetInt(ejs, arg);
        }
        if ((arg = ejsGetPropertyByName(ejs, options, EN("indent"))) != 0) {
            if (ejsIs(ejs, arg, String)) {
               json.indent = (char*) ejsToMulti(ejs, arg);
                //  TODO - get another solution to hold
                mprHold(json.indent);
            } else if (ejsIs(ejs, arg, Number)) {
                i = ejsGetInt(ejs, arg);
                if (0 <= i && i < MPR_MAX_STRING) {
                    json.indent = mprAlloc(i + 1);
                    //  TODO - get another solution to hold
                    mprHold(json.indent);
                    memset(json.indent, ' ', i);
                    json.indent[i] = '\0';
                }
            }
        }
        if ((arg = ejsGetPropertyByName(ejs, options, EN("hidden"))) != 0) {
            json.hidden = (arg == ESV(true));
        }
        if ((arg = ejsGetPropertyByName(ejs, options, EN("namespaces"))) != 0) {
            json.namespaces = (arg == ESV(true));
        }
        if ((arg = ejsGetPropertyByName(ejs, options, EN("pretty"))) != 0) {
            json.pretty = (arg == ESV(true));
        }
        json.replacer = ejsGetPropertyByName(ejs, options, EN("replacer"));
        if (!ejsIsFunction(ejs, json.replacer)) {
            json.replacer = NULL;
        }
    }
    result = serialize(ejs, vp, &json);
    //  TODO - get another solution to hold
    mprRelease(json.indent);
    return result;
}


EjsString *ejsSerialize(Ejs *ejs, EjsAny *vp, int flags)
{
    Json    json;

    memset(&json, 0, sizeof(Json));
    json.depth = 99;
    json.pretty = (flags & EJS_JSON_SHOW_PRETTY) ? 1 : 0;
    json.hidden = (flags & EJS_JSON_SHOW_HIDDEN) ? 1 : 0;
    json.namespaces = (flags & EJS_JSON_SHOW_NAMESPACES) ? 1 : 0;
    json.baseClasses = (flags & EJS_JSON_SHOW_SUBCLASSES) ? 1 : 0;
    return serialize(ejs, vp, &json);
}


static EjsString *serialize(Ejs *ejs, EjsAny *vp, Json *json)
{
    EjsName     qname;
    EjsFunction *fn;
    EjsString   *result, *sv;
    EjsTrait    *trait;
    EjsObj      *pp, *obj, *replacerArgs[2];
    MprChar     *cp;
    cchar       *key;
    int         c, isArray, i, count, slotNum;

    /*
        The main code below can handle Arrays, Objects, objects derrived from Object and also native classes with properties.
        All others just use toString.
     */
    count = ejsIsPot(ejs, vp) ? ejsGetLength(ejs, vp) : 0;
    if (count == 0 && TYPE(vp) != ESV(Object) && TYPE(vp) != ESV(Array)) {
        //  OPT - need some flag for this test.
        if (!ejsIsDefined(ejs, vp) || ejsIs(ejs, vp, Boolean) || ejsIs(ejs, vp, Number)) {
            return ejsToString(ejs, vp);
        } else {
            return ejsToLiteralString(ejs, vp);
        }
    }
    obj = vp;
    json->nest++;
    if (json->buf == 0) {
        json->buf = mprCreateBuf(0, 0);
        mprAddRoot(json->buf);
    }
    isArray = ejsIs(ejs, vp, Array);
    mprPutCharToWideBuf(json->buf, isArray ? '[' : '{');
    if (json->pretty || json->indent) {
        mprPutCharToWideBuf(json->buf, '\n');
    }
    if (++ejs->serializeDepth <= json->depth && !VISITED(obj)) {
        SET_VISITED(obj, 1);
        for (slotNum = 0; slotNum < count && !ejs->exception; slotNum++) {
            trait = ejsGetPropertyTraits(ejs, obj, slotNum);
            if (trait && (trait->attributes & (EJS_TRAIT_HIDDEN | EJS_TRAIT_DELETED | EJS_FUN_INITIALIZER | 
                    EJS_FUN_MODULE_INITIALIZER)) && !json->hidden) {
                continue;
            }
            pp = ejsGetProperty(ejs, obj, slotNum);
            if (ejs->exception) {
                SET_VISITED(obj, 0);
                json->nest--;
                return 0;
            }
            if (pp == 0) {
                continue;
            }
            if (isArray) {
                key = itos(slotNum);
                qname.name = ejsCreateStringFromAsc(ejs, key);
                qname.space = ESV(empty);
            } else {
                qname = ejsGetPropertyName(ejs, vp, slotNum);
            }
            if (json->pretty) {
                for (i = 0; i < ejs->serializeDepth; i++) {
                    mprPutStringToWideBuf(json->buf, "  ");
                }
            } else {
                if (json->indent) {
                    mprPutStringToWideBuf(json->buf, json->indent);
                }
            }
            if (!isArray) {
                if (json->namespaces) {
                    if (qname.space != ESV(empty)) {
                        mprPutFmtToWideBuf(json->buf, "\"%@\"::", qname.space);
                    }
                }
//  TODO -- should this be in unicode?  yes?
                mprPutCharToWideBuf(json->buf, '"');
                for (cp = qname.name->value; cp && *cp; cp++) {
                    c = *cp;
                    if (c == '"' || c == '\\') {
                        mprPutCharToWideBuf(json->buf, '\\');
                        mprPutCharToWideBuf(json->buf, c);
                    } else {
                        mprPutCharToWideBuf(json->buf, c);
                    }
                }
                mprPutStringToWideBuf(json->buf, "\":");
                if (json->pretty || json->indent) {
                    mprPutCharToWideBuf(json->buf, ' ');
                }
            }
            fn = (EjsFunction*) ejsGetPropertyByName(ejs, TYPE(pp)->prototype, N(NULL, "toJSON"));
// OPT - check that this is going directly to serialize most of the time
            if (!ejsIsFunction(ejs, fn) || (fn->isNativeProc && fn->body.proc == (EjsProc) ejsObjToJSON)) {
                sv = serialize(ejs, pp, json);
            } else {
                sv = (EjsString*) ejsRunFunction(ejs, fn, pp, 1, &json->options);
            }
            if (sv == 0 || !ejsIs(ejs, sv, String)) {
                if (ejs->exception) {
                    ejsThrowTypeError(ejs, "Can't serialize property %@", qname.name);
                    SET_VISITED(obj, 0);
                    return 0;
                }
            } else {
                if (json->replacer) {
                    replacerArgs[0] = (EjsObj*) qname.name; 
                    replacerArgs[1] = (EjsObj*) sv; 

                    /* function replacer(key: String, value: String): String */
                    sv = ejsRunFunction(ejs, json->replacer, obj, 2, (EjsObj**) replacerArgs);
                }
                mprPutBlockToBuf(json->buf, sv->value, sv->length * sizeof(MprChar));
            }
            if ((slotNum + 1) < count) {
                mprPutCharToWideBuf(json->buf, ',');
            }
            if (json->pretty || json->indent) {
                mprPutCharToWideBuf(json->buf, '\n');
            }
        }
        SET_VISITED(obj, 0);
    }
    --ejs->serializeDepth; 
    if (json->pretty || json->indent) {
        for (i = ejs->serializeDepth; i > 0; i--) {
            mprPutStringToWideBuf(json->buf, "  ");
        }
    }
    mprPutCharToWideBuf(json->buf, isArray ? ']' : '}');
    mprAddNullToWideBuf(json->buf);

    if (--json->nest == 0) {
        result = ejsCreateString(ejs, mprGetBufStart(json->buf), mprGetBufLength(json->buf) / sizeof(MprChar));
        mprRemoveRoot(json->buf);
    } else {
        result = 0;
    }
    return result;
}


void ejsConfigureJSONType(Ejs *ejs)
{
    ejsFinalizeScriptType(ejs, N("ejs", "JSON"), sizeof(EjsPot), ejsManagePot, EJS_TYPE_POT);
    ejsBindFunction(ejs, ejs->global, ES_deserialize, g_deserialize);
    ejsBindFunction(ejs, ejs->global, ES_serialize, g_serialize);
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
    details at: http://embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com
 */
