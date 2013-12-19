/*
    Compat.es -- Compatibility with other JS engines

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

//  TODO -- much more 

module ejs {

    use default namespace public

    /** @hide */
    function gc(): Void
        GC.run 

    /** @hide */
    function readFile(path: String, encoding: String? = null): String?
        Path(path).readString()

//    //  Rhino
//    /** @hide */
//    var arguments = App.args
//
//    //  Command line history
//    /** @hide */
//    var history
//
//    /** @hide */
//    function help() {}
//
//    /** @hide */
//    function get environment() {
//        //  TODO
//    }
//
//    /** @hide */
//    function runCommand(name: String, ...args): Number {
//        try {
//            Cmd.sh(name + args.join(" "))
//            return 0
//        } catch (e) {
//            return 2
//        }
//    }
//
//    /** @hide */
//    function deserialize(filename): Object
//
//    /** @hide */
//    function load([filenames, ...])
//
//    /** @hide */
//    function defineClass(name) {}
//
//    /** @hide */
//    function loadClass(name) {}
//
//    /** @hide */
//    function seal(obj) {}
//
//    /** @hide */
//    function serialize(obj, filename) {}
//
//    /** @hide */
//    //  Run in another thread
//    function spawn(scriptOrFunction) {}
//
//    /** @hide */
//    function quit() {}
//
//    /** @hide */
//    function version(version) {}
}

/*
    Ease backward compatibility

    module ejs.xml { }
    module ejs.sys { }
    module ejs.io { }
*/

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
