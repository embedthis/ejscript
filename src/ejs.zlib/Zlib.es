/**
    Zlib.es -- Zlib compression class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

//  TODO - enabled while boostraping
# Config.ZLIB || 1
module ejs.zlib {

    /**
        Zlib database support
        @spec ejs
        @stabilitiy prototype
     */
    class Zlib {
        use default namespace public

        static native function compress(src: Path, dest: Path? = null)
        static native function uncompress(src: Path, dest: Path? = null)

        static native function compressBytes(data: ByteArray): ByteArray
        static native function uncompressBytes(data: ByteArray): ByteArray

        static native function compressString(data: String): String
        static native function uncompressString(data: String): String
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
