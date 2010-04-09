/*
    Config.es - Configuration settings from ./configure
 *
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
 */

module ejs {

    /**
        Config class providing settings for various "configure" program settings.
        @spec ejs
        @stability evolving
     */
    enumerable class Config extends Object {

        use default namespace public

        /**
            True if a debug build
         */
        static const Debug: Boolean

        /**
            CPU type (eg. i386, ppc, arm)
         */
        shared static const CPU: String

        /**
            Operating system version. One of: WIN, LINUX, MACOSX, FREEBSD, SOLARIS
         */
        static const OS: String

        /**
            Ejscript product name. Single word name.
         */
        static const Product: String

        /**
            Ejscript product title. Multiword title.
         */
        static const Title: String

        /**
            Ejscript version. Multiword title. Format is Major.Minor.Patch-Build For example: 1.1.2-1
         */
        static const Version: String

        /**
            Installation library directory
         */
        static const LibDir: String

        /**
            Binaries directory
         */
        static const BinDir: String

        /**
            Modules directory
         */
        static const ModDir: String
    }
}

