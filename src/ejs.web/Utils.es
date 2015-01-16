/**
    Utils.es -- Utility routines for the web framework.
 */

# Config.WEB
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
        let cookie
        /*
            Input example: 
            $Version="1"; NAME="value"; $Path="PATH"; $Domain="DOMAIN"; NAME="value"; $Path="PATH"; $Domain="DOMAIN"; 
         */
        for each (c in cookieHeader.split(";")) {
            let [key,value] = c.trim().split("=")
            key = key.toLowerCase().trimStart("$")
            if (key == "domain" || key == "path" || key == "version") {
                if (cookie) {
                    cookie[key] = value
                }
            } else {
                cookie = { name: key, value: value }
                cookies[key] = cookie
            }
        }
        return cookies
    }

    /*
        Return the request form parameters. This creates the params object on demand.
        @returns Object hash of user parameters
        @spec ejs
        @stability prototype

        UNUSED && KEEP

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
            & > < " ' characters into their ampersand HTML equivalents. This is equivalent to:

        str.replace(/&/g,'&amp;').replace(/\>/g,'&gt;').replace(/</g,'&lt;').replace(/"/g,'&quot;').replace(/'/g,'&#39;')

        @param str input string
        @returns an new escaped HTML string
        @spec ejs
        @stability prototype
     */
    native function escapeHtml(str: String): String

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
