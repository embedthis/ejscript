/**
    Debugger.es -- Debugger class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs.debugger {

    /**
        Debugger
        @spec ejs
        @stabilitiy prototype
     */
    class Debugger {

        var prompt: String = "edb> "
        var emitter: Emitter

        function Debugger() {
            emitter = new Emitter
            emitter.on("breakpoint, process)
        }

        native function breakpoint(spec): String
        /*
            source: String = "Current source code line"
            lineNumber: Number
            fileName: File name
         */
        native function getLocation(): Object
        native function jump(reset): Object
        native function readline(): String
        native function restart(): String
        native function run(): String
        native function set(rest): String
        native function shell(): String
        native function stack(): String

        function process(): Void {

            while (true) {
                status()
                let line = readline()
                out.write("\n")
                args = line.split(/ \t/)
                cmd = args[0]
                rest = args.slice(1)
                switch (cmd) {
                case "br":          //  breakpoint [spec]
                    breakpoint(rest)
                    break
                case "bt":          //  backtrace
                    out.write(stack())
                    break
                case "c":           //  call
                    run()
                    return
                case "f":           //  finish [n]
                    break
                case "h":           //  help
                    break
                case "j":           //  jump
                    jump(rest)
                    break
                case "n":           //  next [n]
                    next()
                    break
                case "r":           //  run 
                    restart()
                    return
                case "sh":           //  shell
                    shell()
                    break
                case "set args":    //  step [n]
                    break
                case "st":          //  step [n]
                    step()
                    break
                case "se":          //  set
                    set(args)
                    break
                case "p":           //  print
                    print(rest)
                }
            }
        }

        function next() {
            breakpoint("next")
            run()
        }

        function status() {
            where = getLocation()
            out.write("Breakpoint " + num + " at " + where.filename + ":" + where.lineNumber)
            out.write("%6d %s".format(where.lineNumber, where.source))
            out.write(prompt)
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
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
    
    @end
 */
