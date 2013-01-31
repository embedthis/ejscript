/**
    Zip.es -- Zip class
    @hide
 */

module ejs.tar {

    //  MOB -- need some ability to trace commands  (tar tvf)

    /** @hide */
    class Zip {
        private var path: Path
        private var files = []

        use default namespace public

        function Zip(path: Path) {
            this.path = path.portable
        }
        function add(path: Path): Void {
            path = path.portable
            if (!files.contains(path)) {
                files.append(path)
            }
        }
        function create(): Void {
            let cmd = "zip -q " + path + " " + files.join(" ")
            cmd = cmd.replace(/\\/g, "\\\\")
            Cmd.sh(cmd)
        }
        function extract(...files): Void {
            let cmd = "unzip " + path + " " + files.join(" ")
            cmd = cmd.replace(/\\/g, "\\\\")
            Cmd.sh(cmd)
        }

        //  MOB -- incomplete
        function cat(...files): String {
            let cmd = "tar xOzf " + path + " " + files.join(" ")
            cmd = cmd.replace(/\\/g, "\\\\")
            return Cmd.sh(cmd)
        }
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
