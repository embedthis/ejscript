/**
    Utils.es -- Utility routines for the web framework.
 */

module ejs.web {

    /** @hide */
    function errorBody(title: String, msg: String = ""): String {
        msg ||= ""
        return '<!DOCTYPE html>\r\n<html>\r\n<head><title>' + title + '</title></head>\r\n' + 
           '<body>\r\n<h1>' + title + '</h1>\r\n' +
           '    <p>' + msg + '</p>\r\n' +
           '</body>\r\n</html>\r\n'
    }

    /**
        Return the parsed cookie headers. Cookies are used to specify the session state. If sessions are being used, 
        a session cookie will be sent to and from the browser with each request. 
        @spec ejs
        @stability prototype
     */
    function parseCookies(cookieHeader: String): Object {
        let cookies = {}
        /*
            Input example: 
            $Version="1"; NAME="value"; $Path="PATH"; $Domain="DOMAIN"; NAME="value"; $Path="PATH"; $Domain="DOMAIN"; 
         */
        for each (c in cookieHeader.split(";")) {
            parts = c.split("=")
            key = parts[0].toLowerCase().trim("$")
            if (key == "version") {
                continue
            }
            if (key == "name") {
                cookies[name] = cookie = {}
            }
            cookies[key] = parts[1]
        }
        return cookies
    }

    /*
UNUSED && KEEP
        Return the request form parameters. This creates the params object on demand.
        @returns Object hash of user parameters
        @spec ejs
        @stability prototype

    function parseParams(form: Object): Object {
        params = {}
        for each (f in form) {
            let parts = f.split("=")
            let key = parts[0]
            let value = parts[1]
            if (!key.contains('.')) {
                params[key] = value
            } else {
                let keyParts = key.split(".")
                o = params
                let count = keyParts.length - 2;
                let i
                for (i = 0; i < count; i++) {
                    o[keyParts[i]] = {}
                }
                o[keyParts[i]] = value
            }
        }
        return params
    }
     */

    /**
        Transform a string to be safe for output into an HTML web page. It does this by changing the
            & > < " ' characters into their ampersand HTML equivalents.
        @param str input string
        @returns a transformed HTML escaped string
        @spec ejs
        @stability prototype
     */
    native function escapeHtml(str: String): String

/*
    MOB UNUSED
    function escapeHtml(s: String): String
        s.replace(/&/g,'&amp;').replace(/\>/g,'&gt;').replace(/</g,'&lt;').replace(/"/g,'&quot;').replace(/'/g,'&#39;')
*/


    /** 
        HTML encode the arguments. This escapes HTML directives to be safe for inclusion in a web page.
        @param args Variable arguments that will be converted to safe html
        @return A string containing the encoded arguments catenated together
        @spec ejs
        @stability prototype
     */
    function html(...args): String {
        result = ""
        for each (let s: String in args) {
            result += escapeHtml(s)
        }
        return result
    }

    /**
        Transform an escaped string into its original contents. This reverses the transformation done by $escapeHtml.
        It does this by changing &quot, &gt, &lt back into ", < and >.
        @param s input string
        @returns a transformed string
        @spec ejs
        @stability prototype
     */
    function unescapeHtml(s: String): String
        s.replace(/&amp;/g,'&').replace(/&gt;/g,'>').replace(/&lt;/g,'<').replace(/&quot;/g,'"')
}
