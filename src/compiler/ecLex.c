/**
    ecLex.c - Lexical analyzer

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*
    TODO - must preserve inter-token white space for XML literals
 */

/********************************** Includes **********************************/

#include    "ejsCompiler.h"

/*********************************** Locals ***********************************/

typedef struct ReservedWord {
    void    *name;
    int     groupMask;
    int     tokenId;
    int     subId;
} ReservedWord;


/*
    Reserved keyword table
    The "true", "false", "null" and "undefined" are handled as global variables
 */
static ReservedWord keywords[] =
{
  { "break",            G_RESERVED,         T_BREAK,                    0, },
  { "case",             G_RESERVED,         T_CASE,                     0, },
  { "cast",             G_CONREV,           T_CAST,                     0, },
  { "catch",            G_RESERVED,         T_CATCH,                    0, },
  { "class",            G_RESERVED,         T_CLASS,                    0, },
  { "const",            G_CONREV,           T_CONST,                    0, },
  { "continue",         G_RESERVED,         T_CONTINUE,                 0, },
  { "default",          G_CONREV,           T_DEFAULT,                  0, },
  { "delete",           G_RESERVED,         T_DELETE,                   0, },
  { "do",               G_RESERVED,         T_DO,                       0, },
  { "dynamic",          G_CONREV,           T_ATTRIBUTE,                T_DYNAMIC, },
  { "each",             G_CONREV,           T_EACH,                     0, },
  { "else",             G_RESERVED,         T_ELSE,                     0, },
  { "enumerable",       G_CONREV,           T_ATTRIBUTE,                T_ENUMERABLE, },
  { "extends",          G_RESERVED,         T_EXTENDS,                  0, },
  { "false",            G_RESERVED,         T_FALSE,                    0, },
  { "final",            G_CONREV,           T_ATTRIBUTE,                T_FINAL, },
  { "finally",          G_RESERVED,         T_FINALLY,                  0, },
  { "for",              G_RESERVED,         T_FOR,                      0, },
  { "function",         G_RESERVED,         T_FUNCTION,                 0, },
  { "get",              G_CONREV,           T_GET,                      0, },
  { "goto",             G_CONREV,           T_GOTO,                     0, },
  { "if",               G_RESERVED,         T_IF,                       0, },
  { "implements",       G_CONREV,           T_IMPLEMENTS,               0, },
  { "in",               G_RESERVED,         T_IN,                       0, },
#if UNUSED
  { "include",          G_CONREV,           T_INCLUDE,                  0, },
#endif
  { "instanceof",       G_RESERVED,         T_INSTANCEOF,               0, },
  { "interface",        G_CONREV,           T_INTERFACE,                0, },
  { "internal",         G_CONREV,           T_RESERVED_NAMESPACE,       T_INTERNAL, },
  { "intrinsic",        G_CONREV,           T_RESERVED_NAMESPACE,       T_INTRINSIC, },
  { "is",               G_CONREV,           T_IS,                       0, },
  { "let",              G_CONREV,           T_LET,                      0, },
  { "module",           G_CONREV,           T_MODULE,                   0, },
  { "namespace",        G_CONREV,           T_NAMESPACE,                0, },
  { "native",           G_CONREV,           T_ATTRIBUTE,                T_NATIVE, },
  { "new",              G_RESERVED,         T_NEW,                      0, },
  { "null",             G_RESERVED,         T_NULL,                     0, },
  { "override",         G_CONREV,           T_ATTRIBUTE,                T_OVERRIDE, },
  { "private",          G_CONREV,           T_RESERVED_NAMESPACE,       T_PRIVATE, },
  { "protected",        G_CONREV,           T_RESERVED_NAMESPACE,       T_PROTECTED, },
  { "public",           G_CONREV,           T_RESERVED_NAMESPACE,       T_PUBLIC, },
  { "require",          G_CONREV,           T_REQUIRE,                  0, },
  { "return",           G_RESERVED,         T_RETURN,                   0, },
  { "set",              G_CONREV,           T_SET,                      0, },
  { "standard",         G_CONREV,           T_STANDARD,                 0, },
  { "static",           G_CONREV,           T_ATTRIBUTE,                T_STATIC, },
  { "strict",           G_CONREV,           T_STRICT,                   0, },
  { "super",            G_RESERVED,         T_SUPER,                    0, },
  { "switch",           G_RESERVED,         T_SWITCH,                   0, },
  { "this",             G_RESERVED,         T_THIS,                     0, },
  { "throw",            G_RESERVED,         T_THROW,                    0, },
  { "to",               G_CONREV,           T_TO,                       0, },
  { "true",             G_RESERVED,         T_TRUE,                     0, },
  { "try",              G_RESERVED,         T_TRY,                      0, },
  { "typeof",           G_RESERVED,         T_TYPEOF,                   0, },
  { "var",              G_RESERVED,         T_VAR,                      0, },
  { "undefined",        G_CONREV,           T_UNDEFINED,                0, },
  { "use",              G_CONREV,           T_USE,                      0, },
  { "void",             G_RESERVED,         T_VOID,                     0, },
  { "while",            G_RESERVED,         T_WHILE,                    0, },
  { "with",             G_RESERVED,         T_WITH,                     0, },

#if UNUSED && KEEP
  /*
        Reserved but not implemented
   */
  { "abstract",         G_RESERVED,         T_ABSTRACT,                 0, },
  { "callee",           G_CONREV,           T_CALLEE,                   0, },
  { "enum",             G_RESERVED,         T_ENUM,                     T_ENUM, },
  { "generator",        G_CONREV,           T_GENERATOR,                0, },
  { "has",              G_CONREV,           T_HAS,                      0, },
  { "like",             G_CONREV,           T_LIKE,                     0, },
  { "readonly",         G_RESERVED,         T_ATTRIBUTE,                T_READONLY, },
  { "synchronized",     G_RESERVED,         T_ATTRIBUTE,                T_SYNCHRONIZED, },
  { "type",             G_CONREV,           T_TYPE,                     0, },
  { "volatile",         G_CONREV,           T_VOLATILE,                 0, },
  { "yield",            G_CONREV,           T_YIELD,                    0, },
#endif
  { 0,                  0,                  0, },
};

/***************************** Forward Declarations ***************************/

static int  addCharToToken(EcToken *tp, int c);
static int  addFormattedStringToToken(EcToken *tp, char *fmt, ...);
static int  addStringToToken(EcToken *tp, char *str);
static int  decodeNumber(EcCompiler *cp, int radix, int length);
static int  finalizeToken(EcToken *tp);
static int  initializeToken(EcToken *tp, EcStream *stream);
static int  setTokenID(EcToken *tp, int tokenId, int subId, int groupMask);
static int  makeNumberToken(EcCompiler *cp, EcToken *tp, int c);
static int  makeAlphaToken(EcCompiler *cp, EcToken *tp, int c);
static int  getComment(EcCompiler *cp, EcToken *tp, int c);
static int  getNextChar(EcStream *stream);
static int  makeQuotedToken(EcCompiler *cp, EcToken *tp, int c);
static int  makeSubToken(EcToken *tp, int c, int tokenId, int subId, int groupMask);
static int  makeToken(EcToken *tp, int c, int tokenId, int groupMask);
static void putBackChar(EcStream *stream, int c);

/************************************ Code ************************************/

void ecInitLexer(EcCompiler *cp)
{
    ReservedWord    *rp;
    int             size;

    size = sizeof(keywords) / sizeof(ReservedWord);
    if ((cp->keywords = mprCreateHash(size, MPR_HASH_UNICODE | MPR_HASH_STATIC_KEYS | MPR_HASH_STATIC_VALUES)) == 0) {
        return;
    }
    for (rp = keywords; rp->name; rp++) {
#if BIT_CHAR_LEN > 1
        rp->name = amtow(cp->keywords, rp->name, NULL);
#endif
        mprAddKey(cp->keywords, rp->name, rp);
    }
}


static void manageToken(EcToken *tp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(tp->text);
        ecMarkLocation(&tp->loc);
    }
}


static EcToken *getLexToken(EcCompiler *cp)
{
    EcToken     *tp;

    if ((tp = cp->putback) != 0) {
        cp->putback = tp->next;
        cp->token = tp;
    } else {
        if ((cp->token = mprAllocObj(EcToken, manageToken)) == 0) {
            return 0;
        }
        initializeToken(cp->token, cp->stream);
    }
    return cp->token;
}


int ecGetToken(EcCompiler *cp)
{
    EcToken     *tp;
    EcStream    *stream;
    int         c;

    if ((tp = getLexToken(cp)) == NULL) {
        return T_ERR;
    }
    if (tp->tokenId) {
        return tp->tokenId;
    }
    stream = cp->stream;

    while (1) {
        c = getNextChar(stream);
        /*
            Overloadable operators
            + - ~ * / % < > <= >= == << >> >>> & | === != !==
            TODO FUTURE, we could allow also:  ".", "[", "(" and unary !, ^
         */
        switch (c) {
        default:
            if (isdigit((uchar) c)) {
                return makeNumberToken(cp, tp, c);

            } else if (c == '\\') {
                c = getNextChar(stream);
                if (c == '\n') {
                    break;
                }
                putBackChar(stream, c);
                c = '\n';
            }
            if (isalpha((uchar) c) || c == '_' || c == '\\' || c == '$') {
                return makeAlphaToken(cp, tp, c);
            }
            return makeToken(tp, 0, T_ERR, 0);

        case -1:
            return makeToken(tp, 0, T_ERR, 0);

        case 0:
            if (stream->flags & EC_STREAM_EOL) {
                return makeToken(tp, 0, T_NOP, 0);
            }
            return makeToken(tp, 0, T_EOF, 0);

        case ' ':
        case '\f':
        case '\t':
        case '\v':
        case 0xA0:      /* No break space */
            break;

        case '\r':
        case '\n':
            break;

        case '"':
        case '\'':
            return makeQuotedToken(cp, tp, c);

        case '#':
            return makeToken(tp, c, T_HASH, 0);

        case '[':
            //  EJS extension to consider this an operator
            return makeToken(tp, c, T_LBRACKET, G_OPERATOR);

        case ']':
            return makeToken(tp, c, T_RBRACKET, 0);

        case '(':
            //  EJS extension to consider this an operator
            return makeToken(tp, c, T_LPAREN, G_OPERATOR);

        case ')':
            return makeToken(tp, c, T_RPAREN, 0);

        case '{':
            return makeToken(tp, c, T_LBRACE, 0);

        case '}':
            return makeToken(tp, c, T_RBRACE, 0);

        case '@':
            return makeToken(tp, c, T_AT, 0);

        case ';':
            return makeToken(tp, c, T_SEMICOLON, 0);

        case ',':
            return makeToken(tp, c, T_COMMA, 0);

        case '?':
            return makeToken(tp, c, T_QUERY, 0);

        case '~':
            return makeToken(tp, c, T_TILDE, G_OPERATOR);

        case '+':
            c = getNextChar(stream);
            if (c == '+') {
                addCharToToken(tp, '+');
                return makeToken(tp, c, T_PLUS_PLUS, G_OPERATOR);
            } else if (c == '=') {
                addCharToToken(tp, '+');
                return makeSubToken(tp, c, T_ASSIGN, T_PLUS_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);
            }
            putBackChar(stream, c);
            return makeToken(tp, '+', T_PLUS, G_OPERATOR);

        case '-':
            c = getNextChar(stream);
            if (isdigit((uchar) c)) {
                putBackChar(stream, c);
                return makeToken(tp, '-', T_MINUS, G_OPERATOR);

            } else if (c == '-') {
                addCharToToken(tp, '-');
                return makeToken(tp, c, T_MINUS_MINUS, G_OPERATOR);

            } else if (c == '=') {
                addCharToToken(tp, '-');
                return makeSubToken(tp, c, T_ASSIGN, T_MINUS_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);
            }
            putBackChar(stream, c);
            return makeToken(tp, '-', T_MINUS, G_OPERATOR);

        case '*':
            c = getNextChar(stream);
            if (c == '=') {
                addCharToToken(tp, '*');
                return makeSubToken(tp, c, T_ASSIGN, T_MUL_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);
            }
            putBackChar(stream, c);
            return makeToken(tp, '*', T_MUL, G_OPERATOR);

        case '/':
            c = getNextChar(stream);
            if (c == '=') {
                addCharToToken(tp, '/');
                return makeSubToken(tp, c, T_ASSIGN, T_DIV_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);

            } else if (c == '>') {
                addCharToToken(tp, '/');
                return makeToken(tp, c, T_SLASH_GT, G_OPERATOR);

            } else if (c == '*' || c == '/') {
                /*
                    C and C++ comments
                 */
                if (getComment(cp, tp, c) < 0) {
                    return tp->tokenId;
                }
                /*
                    Doc comments are: [slash]**. The second "*' becomes the first char of the comment.
                    Don't regard: [slash]*** (three stars) as a comment.
                 */
                if (cp->doc) {
                    if (tp->text && tp->text[0] == '*' && tp->text[1] != '*') {
                        cp->docToken = mprMemdup(tp->text, tp->length * sizeof(wchar));
                    }
                }
                initializeToken(tp, stream);
                break;
            }
            putBackChar(stream, c);
            return makeToken(tp, '/', T_DIV, G_OPERATOR);

        case '%':
            c = getNextChar(stream);
            if (c == '=') {
                addCharToToken(tp, '%');
                return makeSubToken(tp, c, T_ASSIGN, T_MOD_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);
            }
            putBackChar(stream, c);
            return makeToken(tp, '%', T_MOD, G_OPERATOR);

        case '.':
            c = getNextChar(stream);
            if (c == '.') {
                c = getNextChar(stream);
                if (c == '.') {
                    addStringToToken(tp, "..");
                    return makeToken(tp, c, T_ELIPSIS, 0);
                }
                putBackChar(stream, c);
                addCharToToken(tp, '.');
                return makeToken(tp, '.', T_DOT_DOT, 0);
#if FUTURE
            } else if (c == '<') {
                addCharToToken(tp, '.');
                return makeToken(tp, c, T_DOT_LESS, 0);
#endif
            } else if (isdigit((uchar) c)) {
                putBackChar(stream, c);
                return makeNumberToken(cp, tp, '.');
            }
            putBackChar(stream, c);
            //  EJS extension to consider this an operator
            return makeToken(tp, '.', T_DOT, G_OPERATOR);

        case ':':
            c = getNextChar(stream);
            if (c == ':') {
                addCharToToken(tp, ':');
                return makeToken(tp, c, T_COLON_COLON, 0);
            }
            putBackChar(stream, c);
            return makeToken(tp, ':', T_COLON, 0);

        case '!':
            c = getNextChar(stream);
            if (c == '=') {
                c = getNextChar(stream);
                if (c == '=') {
                    addStringToToken(tp, "!=");
                    return makeToken(tp, c, T_STRICT_NE, G_OPERATOR);
                }
                putBackChar(stream, c);
                addCharToToken(tp, '!');
                return makeToken(tp, '=', T_NE, G_OPERATOR);
            }
            putBackChar(stream, c);
            return makeToken(tp, '!', T_LOGICAL_NOT, G_OPERATOR);

        case '&':
            c = getNextChar(stream);
            if (c == '&') {
                addCharToToken(tp, '&');
                c = getNextChar(stream);
                if (c == '=') {
                    addCharToToken(tp, '&');
                    return makeSubToken(tp, '=', T_ASSIGN, T_LOGICAL_AND_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);
                }
                putBackChar(stream, c);
                return makeToken(tp, '&', T_LOGICAL_AND, G_OPERATOR);

            } else if (c == '=') {
                addCharToToken(tp, '&');
                return makeSubToken(tp, c, T_ASSIGN, T_BIT_AND_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);
            }
            putBackChar(stream, c);
            return makeToken(tp, '&', T_BIT_AND, G_OPERATOR);

        case '<':
            c = getNextChar(stream);
            if (c == '=') {
                addCharToToken(tp, '<');
                return makeToken(tp, c, T_LE, G_OPERATOR);
            } else if (c == '<') {
                c = getNextChar(stream);
                if (c == '=') {
                    addStringToToken(tp, "<<");
                    return makeSubToken(tp, c, T_ASSIGN, T_LSH_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);
                }
                putBackChar(stream, c);
                addCharToToken(tp, '<');
                return makeToken(tp, c, T_LSH, G_OPERATOR);

            } else if (c == '/') {
                addCharToToken(tp, '<');
                return makeToken(tp, c, T_LT_SLASH, 0);
            }
            putBackChar(stream, c);
            return makeToken(tp, '<', T_LT, G_OPERATOR);

        case '=':
            c = getNextChar(stream);
            if (c == '=') {
                c = getNextChar(stream);
                if (c == '=') {
                    addStringToToken(tp, "==");
                    return makeToken(tp, c, T_STRICT_EQ, G_OPERATOR);
                }
                putBackChar(stream, c);
                addCharToToken(tp, '=');
                return makeToken(tp, c, T_EQ, G_OPERATOR);
            }
            putBackChar(stream, c);
            return makeToken(tp, '=', T_ASSIGN, G_OPERATOR);

        case '>':
            c = getNextChar(stream);
            if (c == '=') {
                addCharToToken(tp, '<');
                return makeToken(tp, c, T_GE, G_OPERATOR);
            } else if (c == '>') {
                c = getNextChar(stream);
                if (c == '=') {
                    addStringToToken(tp, ">>");
                    return makeSubToken(tp, c, T_ASSIGN, T_RSH_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);
                } else if (c == '>') {
                    c = getNextChar(stream);
                    if (c == '=') {
                        addStringToToken(tp, ">>>");
                        return makeSubToken(tp, c, T_ASSIGN, T_RSH_ZERO_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);
                    }
                    putBackChar(stream, c);
                    addStringToToken(tp, ">>");
                    return makeToken(tp, '>', T_RSH_ZERO, G_OPERATOR);
                }
                putBackChar(stream, c);
                addCharToToken(tp, '>');
                return makeToken(tp, '>', T_RSH, G_OPERATOR);
            }
            putBackChar(stream, c);
            return makeToken(tp, '>', T_GT, G_OPERATOR);

        case '^':
            c = getNextChar(stream);
            if (c == '^') {
                addCharToToken(tp, '^');
                c = getNextChar(stream);
                if (c == '=') {
                    addCharToToken(tp, '^');
                    return makeSubToken(tp, '=', T_ASSIGN, T_LOGICAL_XOR_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);
                }
                putBackChar(stream, c);
                return makeToken(tp, '^', T_LOGICAL_XOR, G_OPERATOR);

            } else if (c == '=') {
                addCharToToken(tp, '^');
                return makeSubToken(tp, '=', T_ASSIGN, T_BIT_XOR_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);
            }
            putBackChar(stream, c);
            return makeToken(tp, '^', T_BIT_XOR, G_OPERATOR);

        case '|':
            c = getNextChar(stream);
            if (c == '|') {
                addCharToToken(tp, '|');
                c = getNextChar(stream);
                if (c == '=') {
                    addCharToToken(tp, '|');
                    return makeSubToken(tp, '=', T_ASSIGN, T_LOGICAL_OR_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);
                }
                putBackChar(stream, c);
                return makeToken(tp, '|', T_LOGICAL_OR, G_OPERATOR);

            } else if (c == '=') {
                addCharToToken(tp, '|');
                return makeSubToken(tp, '=', T_ASSIGN, T_BIT_OR_ASSIGN, G_OPERATOR | G_COMPOUND_ASSIGN);

            }
            putBackChar(stream, c);
            return makeToken(tp, '|', T_BIT_OR, G_OPERATOR);
        }
    }
}


int ecGetRegExpToken(EcCompiler *cp, wchar *prefix)
{
    EcToken     *token, *tp;
    EcStream    *stream;
    wchar       *pp;
    int         c;

    stream = cp->stream;
    tp = token = cp->token;
    mprAssert(tp != 0);

    initializeToken(tp, stream);

    for (pp = prefix; pp && *pp; pp++) {
        addCharToToken(tp, *pp);
    }
    while (1) {
        c = getNextChar(stream);
        switch (c) {
        case -1:
            return makeToken(tp, 0, T_ERR, 0);

        case 0:
            if (stream->flags & EC_STREAM_EOL) {
                return makeToken(tp, 0, T_NOP, 0);
            }
            return makeToken(tp, 0, T_EOF, 0);

        case '/':
            addCharToToken(tp, '/');
            while (1) {
                c = getNextChar(stream);
                if (c != 'g' && c != 'i' && c != 'm' && c != 'y' && c != 'x' && c != 'X' && c != 'U' && c != 's') {
                    putBackChar(stream, c);
                    break;
                }
                addCharToToken(tp, c);
            }
            return makeToken(tp, 0, T_REGEXP, 0);

        case '\\':
            c = getNextChar(stream);
            if (c == '\r' || c == '\n' || c == 0) {
                ecError(cp, "Warning", &stream->loc, "Illegal newline in regular expression");
                return makeToken(tp, 0, T_ERR, 0);
            }
            addCharToToken(tp, '\\');
            addCharToToken(tp, c);
            break;

        case '\r':
        case '\n':
            ecError(cp, "Warning", &stream->loc, "Illegal newline in regular expression");
            return makeToken(tp, 0, T_ERR, 0);

        default:
            addCharToToken(tp, c);
        }
    }
}


/*
    Put back the current lexer token
 */
int ecPutToken(EcCompiler *cp)
{
    ecPutSpecificToken(cp, cp->token);
    cp->token = 0;
    return 0;
}


/*
    Put the given (specific) token back on the input queue. The current input token is unaffected.
 */
int ecPutSpecificToken(EcCompiler *cp, EcToken *tp)
{
    mprAssert(tp);
    mprAssert(tp->tokenId > 0);

    tp->next = cp->putback;
    cp->putback = tp;
    return 0;
}


EcToken *ecTakeToken(EcCompiler *cp)
{
    EcToken *token;

    token = cp->token;
    cp->token = 0;
    return token;
}


/*
    Hex:        0(x|X)[DIGITS]
    Octal:      0[DIGITS]
    Float:      [DIGITS].[DIGITS][(e|E)[+|-]DIGITS]
 */
static int makeNumberToken(EcCompiler *cp, EcToken *tp, int c)
{
    EcStream    *stream;

    stream = cp->stream;
    if (c == '0') {
        c = getNextChar(stream);
        if (tolower((uchar) c) == 'x') {
            /* Hex */
            addCharToToken(tp, '0');
            do {
                addCharToToken(tp, c);
                c = getNextChar(stream);
            } while (isxdigit(c));
            putBackChar(stream, c);
            setTokenID(tp, T_NUMBER, -1, 0);
            return finalizeToken(tp);

        } else if ('0' <= c && c <= '7') {
            /* Octal */
            addCharToToken(tp, '0');
            do {
                addCharToToken(tp, c);
                c = getNextChar(stream);
            } while ('0' <= c && c <= '7');
            putBackChar(stream, c);
            setTokenID(tp, T_NUMBER, -1, 0);
            return finalizeToken(tp);

        } else {
            putBackChar(stream, c);
            c = '0';
        }
    }

    /*
        Float
     */
    while (isdigit((uchar) c)) {
        addCharToToken(tp, c);
        c = getNextChar(stream);
    }
    if (c == '.') {
        addCharToToken(tp, c);
        c = getNextChar(stream);
    }
    while (isdigit((uchar) c)) {
        addCharToToken(tp, c);
        c = getNextChar(stream);
    }
    if (tolower((uchar) c) == 'e') {
        addCharToToken(tp, c);
        c = getNextChar(stream);
        if (c == '+' || c == '-') {
            addCharToToken(tp, c);
            c = getNextChar(stream);
        }
        while (isdigit((uchar) c)) {
            addCharToToken(tp, c);
            c = getNextChar(stream);
        }
    }
    putBackChar(stream, c);
    setTokenID(tp, T_NUMBER, -1, 0);
    return finalizeToken(tp);
}


static int makeAlphaToken(EcCompiler *cp, EcToken *tp, int c)
{
    ReservedWord    *rp;
    EcStream        *stream;

    /*
        We know that c is an alpha already
     */
    stream = cp->stream;

    while (isalnum((uchar) c) || c == '_' || c == '$' || c == '\\') {
        if (c == '\\') {
            c = getNextChar(stream);
            if (c == '\n' || c == '\r') {
                break;
            } else if (c == 'u') {
                c = decodeNumber(cp, 16, 4);
                //  TODO - for now, mask back to 8 bits.
                c = c & 0xff;
            }
        }
        addCharToToken(tp, c);
        c = getNextChar(stream);
    }
    if (c) {
        putBackChar(stream, c);
    }
    rp = (ReservedWord*) mprLookupKey(cp->keywords, tp->text);
    if (rp) {
        setTokenID(tp, rp->tokenId, rp->subId, rp->groupMask);
    } else {
        setTokenID(tp, T_ID, -1, 0);
    }
    return finalizeToken(tp);
}


static int makeQuotedToken(EcCompiler *cp, EcToken *tp, int c)
{
    EcStream    *stream;
    int         quoteType;

    stream = cp->stream;
    quoteType = c;

    for (c = getNextChar(stream); c && c != quoteType; c = getNextChar(stream)) {
        if (c == 0) {
            return makeToken(tp, 0, T_ERR, 0);
        }
        if (c == '\\') {
            c = getNextChar(stream);
            switch (c) {
            //  TBD -- others
            case '\\':
                break;
            case '\'':
            case '\"':
                break;
            case 'b':
                c = '\b';
                break;
            case 'f':
                c = '\f';
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            case 'u':
                c = decodeNumber(cp, 16, 4);
                break;
            case 'x':
                c = decodeNumber(cp, 16, 2);
                break;
            case 'v':
                c = '\v';
                break;
            case '0':
                c = decodeNumber(cp, 8, 3);
                break;
            default:
                break;
            }
        }
        addCharToToken(tp, c);
    }
    mprAssert(tp->text);
    setTokenID(tp, T_STRING, -1, 0);
    return finalizeToken(tp);
}


static int makeToken(EcToken *tp, int c, int tokenId, int groupMask)
{
    if (c && addCharToToken(tp, c) < 0) {
        return T_ERR;
    }
    setTokenID(tp, tokenId, -1, groupMask);
    return finalizeToken(tp);
}


static int makeSubToken(EcToken *tp, int c, int tokenId, int subId, int groupMask)
{
    if (addCharToToken(tp, c) < 0) {
        return T_ERR;
    }
    setTokenID(tp, tokenId, subId, groupMask);
    return finalizeToken(tp);
}


static int decodeNumber(EcCompiler *cp, int radix, int length)
{
    char        buf[16];
    int         i, c, lowerc;

    for (i = 0; i < length; i++) {
        c = getNextChar(cp->stream);
        if (c == 0) {
            break;
        }
        if (radix <= 10) {
            if (!isdigit((uchar) c)) {
                break;
            }
        } else if (radix == 16) {
            lowerc = tolower((uchar) c);
            if (!isdigit((uchar) lowerc) && !('a' <= lowerc && lowerc <= 'f')) {
                break;
            }
        }
        buf[i] = c;
    }
    if (i < length) {
        putBackChar(cp->stream, c);
    }
    buf[i] = '\0';
    return (int) stoiradix(buf, radix, NULL);
}


/*
    C, C++ and doc style comments. Return token or zero for no token.
 */
static int getComment(EcCompiler *cp, EcToken *tp, int c)
{
    EcStream    *stream;
    int         form, startLine;

    startLine = cp->stream->loc.lineNumber;
    stream = cp->stream;
    form = c;

    for (form = c; c > 0;) {
        c = getNextChar(stream);
        if (c <= 0) {
            /*
                Unterminated Comment
             */
            addFormattedStringToToken(tp, "Unterminated comment starting on line %d", startLine);
            makeToken(tp, 0, form == '/' ? T_EOF: T_ERR, 0);
            return 1;
        }
        if (form == '/') {
            if (c == '\n' || c == '\r') {
                break;
            }
        } else {
            if (c == '*') {
                c = getNextChar(stream);
                if (c == '/') {
                    break;
                }
                addCharToToken(tp, '*');
                putBackChar(stream, c);

            } else if (c == '/') {
                c = getNextChar(stream);
                if (c == '*') {
                    /*
                        Nested comment
                     */
                    if (cp->warnLevel > 0) {
                        ecError(cp, "Warning", &stream->loc, "Possible nested comment");
                    }
                }
                addCharToToken(tp, '/');
            }
        }
        addCharToToken(tp, c);
    }
    return 0;
}


static int initializeToken(EcToken *tp, EcStream *stream)
{
    tp->stream = stream;
    tp->loc = tp->stream->loc;
    tp->length = 0;
    tp->loc.lineNumber = 0;
    tp->tokenId = 0;
    if (tp->text == 0) {
        tp->size = EC_TOKEN_INCR;
        if ((tp->text = mprAlloc(tp->size * sizeof(wchar))) == 0) {
            return MPR_ERR_MEMORY;
        }
        tp->text[0] = '\0';
    }
    return 0;
}


static int finalizeToken(EcToken *tp)
{
    if (tp->loc.lineNumber == 0) {
        tp->loc = tp->stream->loc;
    }
    return tp->tokenId;
}


static int addCharToToken(EcToken *tp, int c)
{
    if (tp->length >= (tp->size - 1)) {
        tp->size += EC_TOKEN_INCR;
        if ((tp->text = mprRealloc(tp->text, tp->size * sizeof(wchar))) == 0) {
            return MPR_ERR_MEMORY;
        }
    }
    tp->text[tp->length++] = c;
    tp->text[tp->length] = '\0';
    if (tp->loc.lineNumber == 0) {
        tp->loc = tp->stream->loc;
    }
    return 0;
}


static int addStringToToken(EcToken *tp, char *str)
{
    char    *cp;

    for (cp = str; *cp; cp++) {
        if (addCharToToken(tp, *cp) < 0) {
            return MPR_ERR_MEMORY;
        }
    }
    return 0;
}


static int addFormattedStringToToken(EcToken *tp, char *fmt, ...)
{
    va_list     args;
    char        *buf;

    va_start(args, fmt);
    buf = sfmtv(fmt, args);
    addStringToToken(tp, buf);
    va_end(args);
    return 0;
}


static int setTokenID(EcToken *tp, int tokenId, int subId, int groupMask)
{
    mprAssert(tp);

    tp->tokenId = tokenId;
    tp->subId = subId;
    tp->groupMask = groupMask;
    return tokenId;
}


static int getNextChar(EcStream *stream)
{
    wchar       c, *next, *start;

    if (stream->nextChar >= stream->end && stream->getInput) {
        if (stream->getInput(stream) < 0) {
            return 0;
        }
    }
    if (stream->nextChar < stream->end) {
        c = *stream->nextChar++;
        if (c == '\n') {
            stream->lastLoc = stream->loc;
            stream->loc.lineNumber++;
            stream->loc.column = 0;
            stream->loc.source = 0;
        } else {
            stream->loc.column++;
        }
        if (stream->loc.source == 0) {
            for (start = stream->nextChar - 1; isspace((uchar) *start); start++) ;
            for (next = start; *next && *next != '\n'; next++) ;
            stream->loc.source = wsub(start, 0, next - start);
        }
        return c;
    }
    return 0;
}


static void putBackChar(EcStream *stream, int c)
{
    if (stream->buf < stream->nextChar && c) {
        stream->nextChar--;
        mprAssert(c == (int) *stream->nextChar);
        if (c == '\n') {
            stream->loc = stream->lastLoc;
            stream->loc.column = 0;
        } else {
            stream->loc.column--;
        }
        mprAssert(stream->loc.column >= 0);
    }
}


void ecManageStream(EcStream *sp, int flags) 
{
    if (flags & MPR_MANAGE_MARK) {
        ecMarkLocation(&sp->loc);
        ecMarkLocation(&sp->lastLoc);
        mprMark(sp->buf);
    }
}


void *ecCreateStream(EcCompiler *cp, ssize size, cchar *path, void *manager)
{
    EcLocation  *loc;
    EcStream    *sp;

    if ((sp = mprAllocBlock(size, MPR_ALLOC_ZERO | MPR_ALLOC_MANAGER)) == 0) {
        return NULL;
    }
    mprSetManager(sp, manager);
    sp->compiler = cp;
    cp->stream = sp;
    loc = &sp->loc;
    loc->column = 0;
    loc->source = 0;
    loc->lineNumber = 1;
    loc->filename = sclone(path);
    cp->putback = NULL;
    return sp;
}


void ecSetStreamBuf(EcStream *sp, cchar *contents, ssize len)
{
    wchar       *buf;

    if (contents) {
#if BIT_CHAR_LEN > 1
        buf = amtow(cp, contents, &len);
#else
        buf = (wchar*) contents;
        if (len <= 0) {
            len = strlen(buf);
        }
#endif
        sp->buf = buf;
        sp->nextChar = buf;
        sp->end = &buf[len];
        putBackChar(sp, getNextChar(sp));
    }
}


void manageFileStream(EcFileStream *fs, int flags) 
{
    if (flags & MPR_MANAGE_MARK) {
        ecManageStream((EcStream*) fs, flags);
        mprMark(fs->file);

    } else if (flags & MPR_MANAGE_FREE) {
        mprCloseFile(fs->file);
    }
}


int ecOpenFileStream(EcCompiler *cp, cchar *path)
{
    EcFileStream    *fs;
    MprPath         info;
    char            *contents;

    if ((fs = ecCreateStream(cp, sizeof(EcFileStream), path, manageFileStream)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if ((fs->file = mprOpenFile(path, O_RDONLY | O_BINARY, 0666)) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    if (mprGetPathInfo(path, &info) < 0 || info.size < 0) {
        mprCloseFile(fs->file);
        return MPR_ERR_CANT_ACCESS;
    }
    if ((contents = mprAlloc((int) info.size + 1)) == 0) {
        mprCloseFile(fs->file);
        return MPR_ERR_MEMORY;
    }
    if (mprReadFile(fs->file, contents, (int) info.size) != (int) info.size) {
        mprCloseFile(fs->file);
        return MPR_ERR_CANT_READ;
    }
    contents[info.size] = '\0';
    ecSetStreamBuf((EcStream*) fs, contents, (ssize) info.size);
    return 0;
}


int ecOpenMemoryStream(EcCompiler *cp, cchar *contents, ssize len)
{
    EcMemStream     *ms;

    if ((ms = ecCreateStream(cp, sizeof(EcMemStream), "memory", ecManageStream)) == 0) {
        return MPR_ERR_MEMORY;
    }
    ecSetStreamBuf((EcStream*) ms, contents, len);
    return 0;
}


int ecOpenConsoleStream(EcCompiler *cp, EcStreamGet getInput, cchar *contents)
{
    EcConsoleStream     *cs;

    if ((cs = ecCreateStream(cp, sizeof(EcConsoleStream), "console", ecManageStream)) == 0) {
        return MPR_ERR_MEMORY;
    }
    cs->stream.getInput = getInput;
    ecSetStreamBuf((EcStream*) cs, sclone(contents), contents ? strlen(contents) : 0);
    return 0;
}


void ecCloseStream(EcCompiler *cp)
{
    cp->stream = 0;
}


/*
    @copy   default
 
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
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

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
