/**
    ejsJSON.c - JSON encoding and decoding

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Locals ***********************************/

typedef struct JsonState {
    char    *data;
    char    *end;
    char    *next;
    char    *error;
} JsonState;

/***************************** Forward Declarations ***************************/

static EjsObj *parseLiteral(Ejs *ejs, JsonState *js);
static EjsObj *parseLiteralInner(Ejs *ejs, MprBuf *buf, JsonState *js);

/*********************************** Locals ***********************************/
/*
    Convert a string into an object.
    function deserialize(obj: String, options: Object): Object
 */
EjsObj *deserialize(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    mprAssert(argc >=1);
    return ejsDeserialize(ejs, (EjsString*) argv[0]);
}


EjsObj *ejsDeserialize(Ejs *ejs, EjsString *str)
{
    EjsObj      *obj;
    JsonState   js;
    cchar       *data;

    if (!ejsIsString(str)) {
        ejsThrowSyntaxError(ejs, "Object is not a string");
        return 0;
    }
    data = ejsGetString(ejs, str);
    if (data == 0) {
        return 0;
    } else if (*data == '\0') {
        return (EjsObj*) ejs->emptyStringValue;
    }

    //  MOB -- check that js.data never modifies this
    js.next = js.data = (char*) data;
    js.end = &js.data[str->length];
    js.error = 0;
    if ((obj = parseLiteral(ejs, &js)) == 0) {
        if (js.error) {
            ejsThrowSyntaxError(ejs, 
                "Can't parse object literal. Error at position %d.\n"
                "===========================\n"
                "Offending text: %s\n"
                "===========================\n"
                "In literal %s"
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
    MprBuf      *buf;
    EjsObj      *vp;

    mprAssert(js);

    buf = mprCreateBuf(ejs, 0, 0);
    vp = parseLiteralInner(ejs, buf, js);
    mprFree(buf);
    return vp;
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


static uchar *skipComments(uchar *cp, uchar *end)
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


Token getNextJsonToken(MprBuf *buf, char **token, JsonState *js)
{
    uchar   *start, *cp, *end, *next;
    char    *src, *dest;
    int     quote, tid, c;

    //  MOB -- buf is always set?
    if (buf) {
        mprFlushBuf(buf);
    }
    cp = (uchar*) js->next;
    end = (uchar*) js->end;
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
                js->error = (char*) cp;
                return TOK_ERR;
            }
            if (buf) {
                mprPutBlockToBuf(buf, (char*) start, cp - start);
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
                js->error = (char*) cp;
                return TOK_ERR;
            }
            if (buf) {
                mprPutBlockToBuf(buf, (char*) start, cp - start);
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
                mprPutBlockToBuf(buf, (char*) start, cp - start);
            }
        }
        if (buf) {
            mprAddNullToBuf(buf);
        }
        if (*cp == ',' || *cp == ':') {
            cp++;
        } else if (*cp != '}' && *cp != ']' && *cp != '\0' && *cp != '\n' && *cp != '\r' && *cp != ' ') {
            js->error = (char*) cp;
            return TOK_ERR;
        }
        next = cp;

        if (buf) {
            for (dest = src = buf->start; src < buf->end; ) {
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
            *token = mprGetBufStart(buf);
        }
    }
    js->next = (char*) next;
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
    EjsName     qname;
    EjsObj      *obj, *vp;
    MprBuf      *valueBuf;
    char        *token, *key, *value;
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
        obj = (EjsObj*) ejsCreateSimpleObject(ejs);
    } else {
        return ejsParse(ejs, token, ES_String);
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
            vp = ejsParse(ejs, value, (tid == TOK_QID) ? ES_String: -1);
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
                valueBuf = mprCreateBuf(ejs, 0, 0);
                getNextJsonToken(valueBuf, &value, js);
                if (tid == TOK_QID) {
                    vp = (EjsObj*) ejsCreateString(ejs, value);
                } else {
                    if (strcmp(value, "null") == 0) {
                        vp = ejs->nullValue;
                    } else if (strcmp(value, "undefined") == 0) {
                        vp = ejs->undefinedValue;
                    } else {
                        vp = ejsParse(ejs, value, -1);
                    }
                }
                mprAssert(vp);
                mprFree(valueBuf);
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
            key = mprStrdup(obj, key);
            ejsName(&qname, EJS_EMPTY_NAMESPACE, key);
            if (ejsSetPropertyByName(ejs, obj, &qname, vp) < 0) {
                ejsThrowMemoryError(ejs);
                return 0;
            }
        }
    }
    return obj;
}


/*
    Global function to convert the object to a source code string in JSON format. This is the actual work-horse.
  
    function serialize(obj: Object, options: Object = null)
 */
static EjsObj *serialize(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsToJSON(ejs, argv[0], (argc == 2) ? argv[1] : NULL);
}


void ejsConfigureJSONType(Ejs *ejs)
{
    EjsBlock    *block;

    block = ejs->globalBlock;
    mprAssert(block);

    ejsBindFunction(ejs, block, ES_deserialize, deserialize);
    ejsBindFunction(ejs, block, ES_serialize, serialize);
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
 */
