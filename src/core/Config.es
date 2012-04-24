/*
    Config.es - Configuration settings from ./configure
 
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
 */

module ejs {

    /**
        Config class providing settings for various "configure" program settings.
        @spec ejs
        @stability evolving
     */
    enumerable class Config {

        use default namespace public

        /**
            True if a debug build
         */
        native static const Debug: Boolean

        /**
            CPU type (eg. x86, x86_64, ppc, arm)
         */
        native static const CPU: String

        /**
            Operating system version. One of: WIN, LINUX, MACOSX, FREEBSD, SOLARIS, CYGWIN, VXWORKS
         */
        native static const OS: String

        /**
            Ejscript product name. Single word name.
         */
        native static const Product: String

        /**
            Ejscript product title. Multiword title.
         */
        native static const Title: String

        /**
            Ejscript version. Multiword title. Format is Major.Minor.Patch-Build For example: 1.1.2-1
         */
        native static const Version: String

        /**
            Installation library directory
         */
        native static const LibDir: Path

        /**
            Binaries directory
         */
        native static const BinDir: Path

        /**
            Include directory
         */
        native static const IncDir: Path

        /** @hide */
        native static const Legacy: Boolean

        /** @hide */
        native static const SSL: Boolean

        /** @hide */
        native static const SQLITE: Boolean
    }
}
