/*
   Mail.es -- Simple mail using sendmail
 */

module ejs.mail {
    //  TODO - include @options here
    /** 
        Send mail using sendmail
        @param options Object hash with properties for: from, to, subject, date and Content-Type.
        @param msg String message to send
        @example:
            sendmail({
            to: 'john@example.com',
            subject: 'Hello',
            from: 'judy@example.com',
            type: 'text/plain',
            date: new Date(),
            files: [],
            attach: [],
            }, 'Welcome to Example.com')
     */
    public function sendmail(options: Object, message: String): Void {
        if (!options['to']) {
            throw "Missing to"
        }
        options.from ||= 'anonymous'
        options.subject ||= 'Mail message'
        let boundary = 'ejs.mail=' + md5('BOUNDARY')
        let msg = []
        msg.push('To: ' + options['to'])
        msg.push('From: ' + options.from)
        msg.push('Date: ' + (options.date || new Date()))
        msg.push('Subject: ' + options.subject)
        msg.push('MIME-Version: 1.0')
        msg.push('Content-Type: multipart/mixed; boundary=' + boundary)
        msg.push('')
        boundary = '--' + boundary
        msg.push(boundary)
        msg.push('Content-Type: ' + (options.type || 'text/plain') + '\n\n' + message)
        msg.push('')

        for each (let file: Path in options.files) {
            msg.push(boundary)
            let mime = Uri(file).mimeType || 'application/octet-stream'
            msg.push('Content-Transfer-Encoding: base64')
            //  TODO - adding '' does nothing. Should the basename be in quotes
            msg.push('Content-Disposition: inline; filename=' + file.basename + '')
            msg.push('Content-Type: ' + mime + '; name="' + file.basename + '"')
            msg.push('')
            let encoded = base64(file.readString())
            for (i = 0; i < encoded.length; i += 76) {
                msg.push(encoded.slice(i, i + 76))
            }
        }
        for each (let data in options.attach) {
            msg.push(boundary)
            msg.push('Content-Type: application/octet-stream')
            msg.push('Content-Transfer-Encoding: base64')
            if (data is ByteArray) {
                data = data.readString()
            }
            let encoded = base64(data)
            for (i = 0; i < encoded.length; i += 76) {
                msg.push(encoded.slice(i, i + 76))
            }
        }
        msg.push(boundary + '--')
        Cmd.run(['sendmail', '-t'], {}, msg.join('\n'))
    }
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
