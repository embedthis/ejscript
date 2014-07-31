/*
    Args.es - Command line argument parsing
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {
    /**
        The Args class parses the Application's command line. A template of permissible command line options
        is passed to the Args constructor. The Args class supports command options that begin with "-" or "--" and parses
        option arguments of the forms: "-flag x" and "-flag=x". An option may have aliased forms (i.e. -v for --verbose).

        The command template contains properties for options, usage message, and error handling policy should 
        invalid options or arguments be submitted. The template can provide a range of valid values for a command option.
        The option range may be an Ejscript type, regular expression or set of values that constrains the permissible 
        option argument values. A default value may be provided for each option argument. 
        
        Once parsed, the arguments are accessible via args.options. The remaining command line arguments are available 
        in args.rest.

        @spec ejs
        @stability prototype
     */
    class Args {
        /* Program name from App.args[0] */
        public var program: Path

        /* Rest of the User supplied arguments that follow the command options */
        public var rest: Array = []

        /** User supplied command line options */
        public var options: Object = {}

        /* Copy of the argument template specification */
        public var template: Object

        /**
            The Args constructor creates a new instance of the Args. It parses the command line 
            and stores the parsed options in the $options and $args properties. 
            Args supports command options that begin with "-" or "--" and parses option arguments of the forms: 
            "-flag x" and "-flag=x".
            @param template Command argument template. The template is an object with option properties for:
                options, usage, onerror ad silent.
            @param argv Array of command arguments to parse. Defaults to App.args.
            @option options This is an object with properties for each command line option. The value of each property
                is an object with properties: 'alias', 'range', 'value' and 'separator'. The 'alias' property defines
                a String alias for the option. This is typically used to define a single character alias for the full
                command line option name. The 'range' property defines what permissible values an option parameter may take.
                It may be set to either an Ejscript type, a Regular Expression or it may be set to an array of values.

                The 'value' property may define a default value for the option if it is not defined.
                This is the value that args.options[OPTION] will be set to when the command line option is absent.  If
                an option without an argument is specified by the user, its value in options[NAME] will be set to true.  

                If the 'separator' property is defined, multiple command line options of this name are permitted.
                If 'separator' is set to a string, then the multiple command line option values are catenated
                using the 'separator'. If set to array, the option values are stored in an array.
            @option usage Function to invoke for argument parse errors 
            @option onerror
                If set to 'throw', an exception will be thrown for argument parse errors. The usage function will not be 
                called and no message will be written to the application log.
                If set to 'exit', a message will be written to the console for argument parse errors and any usage
                function will be invoked before exiting the application with a non-zero exit status.
            @option silent Don't emit any message on argument parse errors
            @option unknown Callback to invoke for unknown arguments
            @example
let args = Args({
    options: {
        depth: { range: Number },
        quiet: { },
        verbose: { alias: 'v', value: true, },
        log: { range: /\w+(:\d)/, value: 'stderr:4' },
        mode: { range: ['low', 'medium', 'high'], value: 'high' },
        with: { range: String, separator: ',' },
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
                if (item.alias) template.options[item.alias] = item
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
                            if (template.unknown is Function) {
                                i = template.unknown.call(this, argv, i)
                                continue
                            } else if (key == '?') {
                                if (template.usage) {
                                    template.usage()
                                    break
                                }
                            } else {
                                throw "Undefined option '" + key + "'"
                            }
                        } else {
                            if (!value) {
                                if (!item.range) {
                                    value = true
                                } else {
                                    if (++i >= argv.length) {
                                        throw "Missing option for " + key
                                    }
                                    value = argv[i]
                                }
                            }
                            if (item.separator) {
                                item.value ||= []
                                item.value += (item.commas && value.contains(',')) ? value.split(',') : [value]
                            } else {
                                item.value = value
                            }
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
            for (let [key,item]  in template.options) {
                if (item.value == undefined) {
                    continue
                }
                if (item.separator == Array) {
                    options[key] = []
                    for each (value in item.value) {
                        let v = validateItem(item, value)
                        options[key].push(v)
                    }
                } else if (item.separator is String) {
                    options[key] = ''
                    for each (value in item.value) {
                        if (options[key]) {
                            options[key] += item.separator + validateItem(item, value)
                        } else {
                            options[key] += validateItem(item, value)
                        }
                    }
                } else {
                    options[key] = validateItem(item, item.value)
                }
            }
        }

        private function validateItem(item, value): Object {
            if (item.range === Number) {
                if (value) {
                    if (!(value is Number)) {
                        if (value.toString().match(/^\d+$/)) {
                            value = value cast Number
                        } else { 
                           throw "Option \"" + key + "\" must be a number"
                        }
                    }
                } else {
                    value = 0
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
                value = value cast Boolean

            } else if (item.range === String) {
                value = value.toString();

            } else if (item.range === Path) {
                value = Path(value)

            } else if (item.range is RegExp) {
                value = value.toString()
                if (!value.match(item.range)) {
                    throw 'Option "' + key + '" has bad value: "' + value + '"'
                }

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
            return value
        }
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
