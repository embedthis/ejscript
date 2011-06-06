/*
    CmdArg.es - Command line argument parsing
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The CmdArgs class parses the Application's command line options and arguments. The template of permissible args is 
        passed to the CmdArgs constructor. CmdArgs supports command options that begin with "-" or "--" and parses
        option arguments of the forms: "-flag x" and "-flag=x". An option may have multiple forms (e.g. --verbose or -v).
        The primary option form must be specified first as it is the first option that will have its value defined in
        the options hash.
        @spec ejs
        @stability prototype
        @example 
cmd = CmdArgs({
    [ "depth", Number ]
    [ "quiet", null, false ]
    [ [ "verbose", "v", ], true ]
    [ "log", /\w+(:\d)/, "stderr:4" ],
    [ "mode", [ "low", "medium", "high" ], "high" ]
})
let options = cmd.options
if (options.verbose) { }
for each (file in cmd.args) {
    ...
}
     */
    class CmdArgs {
        /* User supplied arguments */
        public var args: Array = []

        /* User supplied options */
        public var options: Object = {}

        private var ranges: Object = {}

        /* Aliases for options with multiple forms (short and long) */
        private var aliases: Object = {}

        /*
            Parse the option template into ranges. Also setup the option default values into options[]
            Template elements are:
                text, permissibleValues, defaultValue
         */
        private function parseTemplate(template: Object): Void {
            for each (item in template) {
/*
            UNUSED
                let key = item[0]
                let range = item[1] || null
                let defaultValue = item[2]
*/
                let [key, range, defaultValue] = item
                if (key is Array) {
                    for each (k in key) {
                        ranges[k] = range || null
                        options[k] = defaultValue
                        if (k != key[0]) {
                            aliases[k] = key[0]
                        }
                    } 
                } else {
                    ranges[key] = range || null
                    options[key] = defaultValue
                }
            }
        }

        /*
            Validate options[] against the range of permissible values .
         */
        private function validate(): Void {
            for (key in options) {
                let range = ranges[key]
                let value = options[key]
                if (!range) {
                    continue
                }
                if (range === Number) {
                    if (value) {
                        if (! value.match(/^\d+$/)) {
                           throw new ArgError("Option \"" + key + "\" must be a number")
                        }
                    } else {
                        value = 0
                    }
                } else if (range === Boolean) {
                    if (value is Boolean) {
                        value = value.toString()
                    } else if (value is String) {
                        value = value.toLowerCase()
                    } else {
                        value = false
                    }
                    if (value != "true" && value != "false") {
                       throw new ArgError("Option \"" + key + "\" must be true or false")
                    }
                } else if (range === String) {
                    ;
                } else if (range is RegExp) {
                    if (!value.match(range)) {
                        throw new ArgError("Option \"" + key + "\" has bad value: \"" + value + "\"")
                    }
                } else if (range is Array) {
                    let ok = false
                    for each (v in range) {
                        if (value == v) {
                            ok = true
                            break
                        }
                    }
                    if (! ok) {
                        throw new ArgError("Option \"" + key + "\" has bad value: \"" + value + "\"")
                    }
                } else {
                    if (value != range) {
                        throw new ArgError("Option \"" + key + "\" has bad value: \"" + value + "\"")
                    }
                }
            }
        }

        /**
             The CmdArgs constructor creates a new instance of the CmdArgs. It parses the command line options and 
             arguments and stores the parsed options in the $options and $args properties. 
             CmdArgs supports command options that begin with "-" or "--" and parses option arguments of the forms: 
             "-flag x" and "-flag=x".
             @param template Array of permissible command option patterns. Each template element corresponds to a
                command option. Each template element is tuple (array) whose first item is the option text. 
                The second item specifies the set of permissible values for the option argument. If the option does not
                take an argument, set this to null. If this argument is set to a regular expression, the option argument
                is validated against it. If set to a Type, the option must have a value of this type. The third item is 
                an optional default value. This is the value that options[NAME] will be set to when the option is absent.
                If an option without an argument is specified by the user, its value in options[NAME] will be set to true. 
                To support options with aliases (such as --verbose and -v), the option text item can be an array of 
                option text names.
             @param argv Array of command arguments to parse. Defaults to App.args.
             @example
                cmd = CmdArgs([
                    [ "quiet", null, false ]
                    [ [ "verbose", "v", ] ]
                    [ "log", /\w+(:\d)/, "stderr:4" ],
                    [ "mode", [ "low", "medium", "high" ], "high" ]
                    [ "speed", Number, 60 ]
                ])
        */
        function CmdArgs(template: Object, argv: Array = App.args.slice(1)) {
            parseTemplate(template)
            for (i = 0; i < argv.length; i++) {
                let arg = argv[i]
                if (arg.startsWith("-")) {
                    let key = arg.slice(arg.startsWith("--") ? 2 : 1)
                    if (aliases[key]) {
                        key = aliases[key]
                    }
                    if (key.contains('=')) {
                        let parts = key.split('=')
                        key = parts[0]
                        options[key] = parts[1]
                        continue
                    }
                    if (ranges[key] === undefined) {
                        throw "Undefined option " + key
                    }
                    if (!ranges[key]) {
                        //  No argument for option
                        options[key] = true
                    } else {
                        if (++i >= argv.length) {
                            throw "Missing option for " + key
                        }
                        options[key] = argv[i]
                    } 
                } else {
                    args.append(arg)
                }
            }
            validate()
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.
    
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
