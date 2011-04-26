/*
    TemplateParser.es -- Ejscript web templating parser. 
 */

module ejs.template  {

    /*
        TODO implement these directives
          -%>  Omit newline after tag
          <%s   For safe output Html escape the output
     */

    /** 
        Web Page Template Parser - Parse an ejs web page and emit an Ejscript source file (.es).
      
        This parser handles embedded Ejscript using <% %> directives. It supports:
        <ul>
          <li>&lt;%       Begin an ejs directive section containing statements</li>
          <li>&lt;%=      Begin an ejs directive section that contains an expression to evaluate and substitute</li>
          <li>%&gt;          End an ejs directive</li>
          <li>&lt;%&#64; include "file" %> Include an ejs file</li>
          <li>&lt;%&#64 layout "file" %>  Specify a layout page to use. Use layout "" to disable layout management.</li>
          <li>&lt;%&#64 view "[module::]class" %>  Specify a view class to use with optional module.</li>
        </ul>
        Directives for use outside of &lt;% %&gt; 
        <ul>
          <li>&#64;&#64var &mdash; To expand the value of "var". Var can also be simple expressions (without spaces).</li>
        </ul>

        @spec ejs
        @stability prototype
     */
    public class TemplateParser {
        private const ContentMarker: String  = "__ejs:CONTENT:ejs__"
        private const ContentPattern: RegExp = new RegExp(ContentMarker)

        private var script: String
        private var pos: Number = 0
        private var lineNumber: Number = 0
        private var viewClass: String = "View"
        private var viewModule: String

        /**
            Build a templated page
            @param script String containing the script to parse
            @param options Object hash with options to control parsing
            @options layout Path Layout file
            @options dir Path Base directory to use for including files and for resolving layout directives
         */
        public function build(script: String, options: Object = {}): String {
            let code = parse(script, options)
            return "require ejs.web\n" + 
                ((viewModule) ? ("require " + viewModule + "\n") : "") + 
                "\nexports.app = function (request: Request) {\n" + 
                "    " + viewClass + "(request).render(function(request: Request) {\n" + code + "\n    })\n}\n"
        }

        /**
            Template parser. Parse the given script and return the compiled (Ejscript) result
            @param script String containing the script to parse
            @param options Object hash with options to control parsing
            @options layout Path Layout file
            @options dir Path Base directory to use for including files and for resolving layout directives
            @return The parsed and expanded template 
         */
        public function parse(script: String, options: Object = {}): String {
            var token: ByteArray = new ByteArray
            var out: ByteArray = new ByteArray
            var dir: Path = options.dir || Path(".")
            var tid: Number
            var layoutPage: Path

            if (options.layout) {
                layoutPage = Path(options.layout)
            }
            this.script = script
            while ((tid = getToken(token)) != Token.Eof) {
                // print("getToken => " + Token.tokens[tid + 1] + " TOKEN => \"" + token + "\"")

                switch (tid) {
                case Token.Literal:
                    //  OPT -- should amalgamate writes
                    out.write("\n        write(\"" + token + "\");")
                    break

                case Token.Var:
                    /*
                        Trick to get undefined variables to evaluate to "".
                        Catenate with "" to cause toString to run.
                        Write safely by HTML escaping the expression
                     */
                    out.write("\n        writeSafe(\"\" + ", token, ");\n")
                    break

                case Token.Equals:
                    /* Write safely by HTML escaping the expression */
                    out.write("\n        writeSafe(\"\" + (", token, "));\n")
                    break

                case Token.EjsTag:
                    /*
                        Just copy the Ejscript code straight through
                     */
                    out.write(token.toString())
                    break

                case Token.Control:
                    let args: Array = token.toString().split(/\s/g)
                    let cmd: String = args[0]

                    switch (cmd) {
                    case "include":
                        let path = args[1].trim("'").trim('"')
                        let incPath = dir.join(path)
                        /*
                            Recurse and process the include script
                         */
                        let inc: TemplateParser = new TemplateParser
                        out.write(inc.parse(incPath.readString(), options))
                        break

                    case "layout":
                        let layouts = options.layouts || App.config.directories.layouts || "layouts"
                        let path = args[1]
                        if (path == "" || path == '""') {
                            layoutPage = undefined
                        } else {
                            layoutPage = Path(args[1].trim("'").trim('"').trim('.ejs') + ".ejs")
                            if (!layoutPage.exists) {
                                layoutPage = Path(layouts).join(layoutPage)
                                if (!layoutPage.exists) {
                                    throw "Can't find layout page " + layoutPage
                                }
                            }
                        }
                        break

                    case "view":
                        viewClass = args[1].trim("'").trim('"')
                        if (viewClass.contains("::")) {
                            [viewModule, viewClass] = viewClass.split("::")
                        }
                        viewClass ||= "View"
                        break

                    case "content":
                        out.write(ContentMarker)
                        break

                    default:
                        throw "Bad control directive: " + cmd
                    }
                    break

                default:
                case Token.Err:
                    //  TODO - should report line numbers
                    throw "Bad input token: " + token

                }
            }
            if (layoutPage && layoutPage != options.currentLayout) {
                let layoutOptions = blend(options.clone(), { currentLayout: layoutPage })
                let layoutText: String = new TemplateParser().parse(layoutPage.readString(), layoutOptions)
                return layoutText.replace(ContentPattern, out.toString().replace(/\$/g, "$$$$"))
            }
            return out.toString()
        }

        /*
         *  Get the next input token. Read from script[pos]. Return the next token ID and update the token byte array
         */
        function getToken(token: ByteArray): Number {
            var tid = Token.Literal
            token.flush(Stream.BOTH)
            var c
            while (pos < script.length) {
                c = script[pos++]
                switch (c) {
                case '<':
                    if (script[pos] == '%' && (pos < 2 || script[pos - 2] != '\\')) {
                        if (token.available > 0) {
                            pos--
                            return Token.Literal
                        }
                        pos++
                        eatSpace()
                        if (script[pos] == '=') {
                            /*
                                <%=  directive
                             */
                            pos++
                            eatSpace()
                            while (pos < script.length && (c = script[pos]) != undefined && 
                                    (c != '%' || script[pos+1] != '>' || script[pos-1] == '\\')) {
                                token.write(c)
                                pos++
                            }
                            pos += 2
                            return Token.Equals

                        } else if (script[pos] == '@') {
                            /*
                                <%@  directive
                             */
                            pos++
                            eatSpace()
                            while (pos < script.length && (c = script[pos]) != undefined && 
                                    (c != '%' || script[pos+1] != '>')) {
                                token.write(c)
                                pos++
                            }
                            pos += 2
                            return Token.Control

                        } else {
                            while (pos < script.length && 
                                    (c = script[pos]) != undefined && 
                                    (c != '%' || script[pos+1] != '>' || script[pos-1] == '\\')) {
                                token.write(c)
                                pos++
                            }
                            pos += 2
                            return Token.EjsTag
                        }
                    }
                    token.write(c)
                    break

                case '@':
                    if (script[pos] == '@' && (pos < 1 || script[pos-1] != '\\')) {
                        if (token.available > 0) {
                            pos--
                            return Token.Literal
                        }
                        pos++
                        c = script[pos++]
                        while (c.isAlpha || c.isDigit || c == '[' || c == ']' || c == '.' || c == '$' || c == '_' || 
                                c == "'") {
                            token.write(c)
                            c = script[pos++]
                        }
                        pos--
                        return Token.Var
                    }
                    token.write(c)
                    break

                case "\r":
                case "\n":
                    lineNumber++
                    token.write(c)
                    tid = Token.Literal
                    break

                default:
                    //  TODO - triple quotes would eliminate the need for this
                    if (c == '\"' || c == '\\') {
                        token.write('\\')
                    }
                    token.write(c)
                    break
                }
            }
            if (token.available == 0 && pos >= script.length) {
                return Token.Eof
            }
            return tid
        }

        private function eatSpace(): Void {
            while (pos < script.length && script[pos].isSpace) {
                pos++
            }
        }
    }

    /**
        Parser tokens
        @hide
     */
    class Token {
        public static const Err         = -1        /* Any input error */
        public static const Eof         = 0         /* End of file */
        public static const EjsTag      = 1         /* <% text %> */
        public static const Var         = 2         /* @@var */
        public static const Literal     = 3         /* literal HTML */
        public static const Equals      = 4         /* <%= expression */
        public static const Control     = 6         /* <%@ control */

        public static var tokens = [ "Err", "Eof", "EjsTag", "Var", "Literal", "Equals", "Control" ]
    }
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
