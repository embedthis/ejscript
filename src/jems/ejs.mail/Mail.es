/*
   Mail.es -- Simple mail using sendmail
 */

module ejs.mail {
    /** 
        Send mail using sendmail
        @param options Object hash with properties for: from, to, subject, date and Content-Type.
        @param msg String message to send
        @example:
            mail({
            to: 'john@example.com',
            subject: 'Hello',
            from: 'judy@example.com',
            'Content-type': 'text/html',
            date: new Date(),                                                                      
            }, 'Welcome to Example.com')
     */
    public function mail(options: Object, msg: String): Void {
        let hdr = serialize(options).replace(/"/g, '').trim('{').trim('}').replace(/,/g, '\n')
        Cmd.run(['sendmail', '-t'], {}, hdr + '\n' + msg)
    }
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
