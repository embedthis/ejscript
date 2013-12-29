#!/usr/bin/env ejs
/*
    ejspage.es -- Ejscript web templating engine command 
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

require ejs.template

class EjsPage {
    private var verbose: Number = 1

    private var argsTemplate = {
        options: {
            layout: { range: String },
            overwrite: {},
            quiet: { alias: 'q' },
            verbose: { alias: 'v', value: true },
        },
        usage: usage,
    }

    function usage(e = null): Void {
        if (e) {
            App.log.error(e)
        }
        App.log.error("\nUsage: ejspage [options] [commands] ...\n" +
            "  Options:\n" + 
            "    --layout layoutPage\n" + 
            "    --overwrite\n" + 
            "    --quiet\n" + 
            "    --verbose\n")
        App.exit(1)
    }

    function main() {
        let args
        try {
            args = Args(argsTemplate)
            processOptions(args)
            if (args.rest.length == 0) {
                usage()
            }
        } catch (e) {
            usage(e)
        }
        try {
            let options = args.options
            let parser = new TemplateParser
            for each (f in args.rest) {
                f = Path(f)
                options.dir = f.dirname
                result = parser.build(f.readString(), options)
                let script = f.replaceExt(".es");
                if (script.exists && !options.overwrite) {
                    throw "Script " + script + " already exists. Use --overwrite."
                }
                script.write(result)
            }
        } catch (e) {
            App.log.error("ejspage: Error: " + ((e is String) ? e : e.message) + "\n")
            App.exit(2)
        }
    }

    function processOptions(args) {
        let options = args.options
        if (options.version) {
            print(Config.version)
            App.exit(0)
        }
        if (options.v) {
            options.verbose = true
        }
        if (options.quiet) {
            options.verbose = false
        }
    }
}

var page: EjsPage = new EjsPage
page.main()

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
