/*
    Args.es - Command line argument parsing
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {
    /**
        The Args class parses the Application's command line. A template of permissible arguments is passed to the 
        Args constructor. The Args class supports command options that begin with "-" or "--" and parses
        option arguments of the forms: "-flag x" and "-flag=x". An option may have multiple forms (e.g. --verbose or -v).

        The template contains properties for options, usage message should invalid arguments be submitted and 
        an error handling policy. The options can provide a range of valid values for an option. The Range may be an
        Ejscript type or regular expression that constrains the permissible values for the argument. A default value
        may be provided for each option. The property in args.options will always be created and will be set to 
        the default value if the users does not provide a value.
        
        Once parsed, the arguments are accessible via args.options. The remaining command line arguments are available 
        in cmd.rest.

        @spec ejs
        @stability prototype
     */
    class Args {
        /* Program name from App.args[0] */
        public var program: Path

        /* Rest of the User supplied arguments that follow the switche options */
        public var rest: Array = []

        /** User supplied options (switches) */
        public var options: Object = {}

        /* Copy of the argument template specification */
        public var template: Object

        /**
             The Args constructor creates a new instance of the Args. It parses the command line 
             and stores the parsed options in the $options and $args properties. 
             Args supports command options that begin with "-" or "--" and parses option arguments of the forms: 
             "-flag x" and "-flag=x".
             @param template Array of permissible command option patterns. Each template element corresponds to a
                command option. Each template element is tuple (array) with three elements: Name, Range and Default.
                Tirst item is the option name. 
                The second item specifies the set of permissible values for the option argument. If the option does not
                take an argument, set this to null. If this argument is set to a regular expression, the option argument
                is validated against it. If set to a Type, the option must have a value that is castable to this type. 
                The third item is an optional default value. This is the value that options[NAME] will be set to when
                the option is absent.  If an option without an argument is specified by the user, its value in
                options[NAME] will be set to true.  To support options with aliases (such as --verbose and -v), the
                option text item can be an array of option text names.
             @param argv Array of command arguments to parse. Defaults to App.args.
             @option onerror
                If set to 'throw', an exception will be thrown for argument parse errors. The usage function will not be 
                called and no message will be written to the application log.
                If set to 'exit', a message will be written to the console for argument parse errors and any usage
                function will be invoked before exiting the application with a non-zero exit status.
            @option silent Don't emit any message on argument parse errors
             @example
let args = Args({
    options: {
        depth: { range: Number },
        quiet: { },
        verbose: { alias: 'v', value: true, },
        log: { range: /\w+(:\d)/, value: 'stderr:4' },
        mode: { range: ['low', 'medium', 'high'], value: 'high' },
    },
    usage: usage,
    onerror: 'exit|throw',
}, [])

let options = args.options
if (options.verbose) { }
for each (file in args.rest) {
    ...
}
        */
        function Args(template: Object, argv: Array = App.args) {
            this.template = template.clone()
            for each (item in template.options) {
                if (item.alias) {
                    template.options[item.alias] = item
                }
            }
            try {
                program = Path(argv[0])
                for (i = 1; i < argv.length; i++) {
                    let arg = argv[i]
                    if (arg.startsWith("-")) {
                        let name = arg.slice(arg.startsWith("--") ? 2 : 1)
                        let [key,value] = name.split("=")
                        let item = template.options[key]
                        if (!item) {
                            throw "Undefined option " + key
                        }
                        if (value) {
                            template.options[key].value = value
                            continue
                        }
                        if (!item.range) {
                            template.options[key].value = true
                        } else {
                            if (++i >= argv.length) {
                                throw "Missing option for " + key
                            }
                            template.options[key].value = argv[i]
                        }
                    } else {
                        rest.append(arg)
                    }
                }
                validate()
            } catch (e) {
                if (template.onerror == 'throw') {
                    throw e
                } 
                if (!template.silent) {
                    App.log.error(e)
                }
                if (template.usage) {
                    template.usage()
                }
                if (template.onerror == 'exit') {
                    App.exit(1)
                }
            }
        }

        /*
            Validate options against the range of permissible values
         */
        private function validate(): Void {
            for (key in template.options) {
                let item = template.options[key]
                let value = item.value
                if (value == undefined) {
                    continue
                }
                if (item.range === Number) {
                    if (value) {
                        if (!(value is Number)) {
                            if (value.toString().match(/^\d+$/)) {
                                options[key] = value cast Number
                            } else { 
                               throw "Option \"" + key + "\" must be a number"
                            }
                        }
                    } else {
                        item.value = 0
                    }
                } else if (item.range === Boolean) {
                    if (value is Boolean) {
                        value = value.toString()
                    } else {
                        value = value.toString().toLowerCase()
                    }
                    if (value != "true" && value != "false") {
                       throw 'Option "' + key + '" must be true or false'
                    }
                    item.value = value cast Boolean

                } else if (item.range === String) {
                    item.value = item.value.toString();

                } else if (item.range === Path) {
                    item.value = Path(item.value)

                } else if (item.range is RegExp) {
                    value = value.toString()
                    if (!value.match(item.range)) {
                        throw 'Option "' + key + '" has bad value: "' + value + '"'
                    }
                    item.value = value

                } else if (item.range is Array) {
                    let ok = false
                    for each (v in item.range) {
                        if (value == v) {
                            ok = true
                            break
                        }
                    }
                    if (! ok) {
                        throw 'Option "' + key + '" has bad value: "' + value + '"'
                    }
                } else {
                    if (item.range && value != item.range) {
                        throw 'Option "' + key + '" has bad value: "' + value + '"'
                    }
                }
                options[key] = item.value
            }
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
