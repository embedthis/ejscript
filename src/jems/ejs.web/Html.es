/**
   Html.es -- HtmlViewConnector. This provides HTML view support.
 */

module ejs.web {

	/**
	    The Html Connector provides bare HTML encoding of View controls.

        MOB Style conventions???
            -ejs- prefixes all internal styles
            -ejs-alert
            -ejs-flash
            -ejs-flash-inform
            -ejs-flash-warn
            -ejs-flash-error
            -ejs-hidden
            -ejs-tabs
            -ejs-fieldError     MOB -- should this be field-error
            -ejs-progress
            -ejs-progress-inner
            -ejs-table
            -ejs-table-download
            -ejs-even-row
            -ejs-click
            -ejs-field-error
            -ejs-form-error
        @stability prototype
        @spec ejs
        @hide
	 */
	public class HtmlViewConnector {

        use default namespace module

        private var request: Request
        private var view: View

        /* Sequential DOM ID generator */
        private var lastDomID: Number = 0

        /*
            Mapping of helper options to HTML attributes.
            NOTE: data-*, click and remote are handled specially in getAttributes.
         */
        private static const htmlOptions: Object = { 
            "apply":                "data-apply",
            "background":           "background",
            "class":                "class",
            "color":                "color",
            "colour":               "color",
            "domid":                "id",
            "height":               "height",
            "key":                  "data-key",
            "method":               "data-method",
            "modal":                "data-modal",
            "period":               "data-refresh-period",
            "pivot":                "data-pivot",
            "refresh":              "data-refresh",
            "rel":                  "rel",
            "size":                 "size",
            "sort":                 "data-sort",
            "sortOrder":            "data-sort-order",
            "style":                "class",
            "visible":              "visible",
            "width":                "width",
        }

        private static const defaultScripts = [
            "web/layout.css", 
            "web/themes/default.css", 
            "web/js/tv/jquery.treeview.css",
        ]

        //  MOB -- what about minified?

        private static const defaultStylesheets = [
            "web/js/jquery.js", 
            "web/js/jquery.tablesorter.js",
            "web/js/tv/jquery.treeview.js",
            "web/js/tv/lib/jquery.cookie.js",
            "web/js/tv/jquery.treeview.async.js",
            "web/js/jquery.address.js",
            "web/js/jquery.simplemodal.js",
            "web/js/jquery.ejs.js",
        ]

        function HtmlViewConnector(view) {
            this.view = view
            this.request = view.request
        }

		function alert(text: String, options: Object): Void {
            options.style = append(options.style, "-ejs-alert")
            write('<div' + getAttributes(options) + '>' +  text + '</div>\r\n')
        }

		function button(field: String, label: String, options: Object): Void {
            write('    <input name="' + field + '" type="submit" value="' + label + '"' + getAttributes(options) + ' />\r\n')
        }

		function buttonLink(text: String, options: Object): Void {
            options.click ||= true
            let attributes = getAttributes(options)
            write('<button ' + attributes + '>' + text + '</button></a>\r\n')
        }

		function chart(data: Array, options: Object): Void {
            //  TODO
            throw 'HtmlConnector control "chart" not implemented.'
		}

		function checkbox(field: String, value: Object, checkedValue: Object, options: Object): Void {
            let checked = (value == checkedValue) ? ' checked="yes"' : ''
            write('    <input name="' + field + '" type="checkbox"' + getAttributes(options) + checked + 
                ' value="' + checkedValue + '" />\r\n')
            write('    <input name="' + field + '" type="hidden"' + getAttributes(options) + ' value="" />\r\n')
        }

		function endform(): Void {
            write('</form>\r\n')
        }

		function flash(kind: String, msg: String, options: Object): Void {
            options.style = append(options.style, "-ejs-flash -ejs-flash-" + kind)
            write('<div' + getAttributes(options) + '>' + msg + '</div>\r\n')
            if (kind == "inform") {
                write('<script>$(document).ready(function() {
                        $("div.-ejs-flash-inform").animate({opacity: 1.0}, 2000).hide("slow");
                    });}</script>\r\n')
            }
		}

		function form(record: Object, options: Object): Void {
            let uri ||= request.makeUri(options)
            emitFormErrors(record, options)
            write('<form action="' + uri + '"' + getAttributes(options) + '>\r\n')
            if (options.id != undefined) {
                write('    <input name="id" type="hidden" value="' + options.id + '" />\r\n')
                if (!options.insecure) {
                    let token = options.securityToken || request.securityToken
                    write('    <input name="' + Request.SecurityTokenName + '" type="hidden" value="' + token + '" />\r\n')
                }
            }
        }

        function image(src: String, options: Object): Void {
			write('<img src="' + src + '"' + getAttributes(options) + '/>\r\n')
        }

//  MOB - merge label and link?
        function label(text: String, options: Object): Void {
            // write('<span ' + getAttributes(options) + ' type="' + getTextKind(options) + '">' +  text + '</span>\r\n')
            write('<span ' + getAttributes(options) + '>' +  text + '</span>\r\n')
        }

		function link(text: String, options: Object): Void {
            let uri ||= request.makeUri(options)
			write('<a href="' + uri + '"' + getAttributes(options) + '>' + text + '</a>\r\n')
		}

		function list(name: String, choices: Object, defaultValue: String, options: Object): Void {
            let selected
            write('    <select name="' + name + '" ' + getAttributes(options) + '>\r\n')
            if (choices is Array) {
                let i = 0
                for each (choice in choices) {
                    if (choice is Array) {
                        /* list("priority", [["3", "low"], ["5", "med"], ["9", "high"]]) */
                        let [key, value] = choice
                        selected = (choice[0] == defaultValue) ? ' selected="yes"' : ''
                        write('      <option value="' + choice[0] + '"' + selected + '>' + choice[1] + '</option>\r\n')

                    } else if (Object.getOwnPropertyCount(choice) > 0) {
                        /* list("priority", [{low: 3}, {med: 5}, {high: 9}]) */
                        for (let [key, value] in choice) {
                            selected = (value == defaultValue) ? ' selected="yes"' : ''
                            write('      <option value="' + value + '"' + selected + '>' + key + '</option>\r\n')
                        }
                    } else {
                        /* list("priority", ["low", "med", "high"]) */
                        selected = (choice == defaultValue) ? ' selected="yes"' : ''
                        write('      <option value="' + i + '"' + selected + '>' + choice + '</option>\r\n')
                    }
                    i++
                }
            } else {
                /* list("priority", {low: 0, med: 1, high: 2}) */
                for (let [key, value]  in choices) {
                    selected = (value == defaultValue) ? ' selected="yes"' : ''
                    write('      <option value="' + value + '"' + selected + '>' + key + '</option>\r\n')
                }
            }
            write('    </select>\r\n')
        }

		function mail(name: String, address: String, options: Object): Void  {
			write('<a href="mailto:' + address + '"' + getAttributes(options) + '>' + name + '</a>\r\n')
		}

		function progress(data: Number, options: Object): Void {
            options["data-progress"] = data
            write('<div class="-ejs-progress">\r\n    <div class="-ejs-progress-inner"' + getAttributes(options) + 
                '>' + data + '%</div>\r\n</div>>\r\n')
		}

        function radio(name: String, choices: Object, selected: String, options: Object): Void {
            let checked: String
            let attributes = getAttributes(options)
            if (choices is Array) {
                for each (choice in choices) {
                    if (choice is Array) {
                        /* radio("priority", [["3", "low"], ["5", "med"], ["9", "high"]]) */
                        let [key, value] = choice
                        checked = (value == selected) ? "checked " : ""
                        write('    ' + key.toPascal() + ' <input type="radio" name=' + name + attributes + ' value="' + 
                            value + '"' + checked + '/>\r\n')

                    } else if (Object.getOwnPropertyCount(choice) > 0) {
                        /* radio("priority", [{low: 3}, {med: 5}, {high: 9}]) */
                        for (let [key, value] in choice) {
                            checked = (value == selected) ? "checked " : ""
                            write('  ' + key.toPascal() + ' <input type="radio" name=' + name + attributes + ' value="' + 
                                value + '"' + checked + '/>\r\n')
                        }

                    } else {
                        /* radio("priority", ["low", "med", "high"]) */
                        checked = (choice == selected) ? "checked " : ""
                        write("    " + choice + ' <input type="radio" name="' + name + '"' + attributes + ' value="' + 
                            choice + '" ' + checked + '/>\r\n')
                    }
                }
            } else {
                /* radio("priority", {low: 0, med: 1, high: 2}) */
                for (let [key, value] in choices) {
                    checked = (value == selected) ? "checked " : ""
                    write("    " + key.toPascal() + ' <input type="radio" name="' + name + '"' + attributes + ' value="' + 
                        value + '" ' + checked + '/>\r\n')
                }
            }
        }

		function script(uri: String, options: Object): Void {
            if (uri == null) {
                for each (uri in defaultScripts) {
                    script(request.home.join(uri), options)
                }
            } else {
                write('    <script src="' + uri + '" type="text/javascript"></script>\r\n')
            }
		}

        function securityToken(options: Object): Void {
            write('    <meta name="SecurityTokenName"=' + Request.SecurityTokenName + '" />\r\n')
            write('    <meta name="' + Request.SecurityTokenName + '" content="' + request.securityToken + '" />\r\n')
        }

		function stylesheet(uri: String, options: Object): Void {
            if (uri == null) {
                for each (uri in defaultStylesheets) {
                    stylesheet(request.home.join(uri), options)
                }
            } else {
                write('    <link rel="stylesheet" type="text/css" href="' + uri + '" />\r\n')
            }
		}

		function table(data, options: Object): Void {
            if (!data is Array) {
                data = [data]
            }
            if (data.length == 0) {
                write("<p>No Data</p>\n")
                return
            }
            options.style = append(options.style, "-ejs-table")

            o = options.clone(); delete o.click; delete o.remote; delete o.method

            let attributes = getAttributes(o)
            let columns = getColumns(data, options)

			write('  <table' + attributes + '>\r\n')
            /*
                Table title header and column headings
             */
            if (options.showHeader != false) {
                write('    <thead>\r\n')
                if (options.title) {
                    write('        <tr><td colspan="' + columns.length + '">' + options.title + '</td></tr>\r\n')
                }
                write('        <tr>\r\n')
                for (let [name, column] in columns) {
                    if (name == null) continue
                    let header = (column.header) ? (column.header) : name.toPascal()
                    let width = (column.width) ? ' width="' + column.width + '"' : ''
                    write('            <th' + width + '>' + header + '</th>\r\n')
                }
                write("        </tr>\r\n    </thead>\r\n")
            }
            write('    <tbody>\r\n')

            /*
                Render each row
             */
            let rowAtt = getAttributes({ click: options.click, method: options.method, remote: options.remote })

            let row: Number = 0
			for each (let r: Object in data) {
                let styleRow = options.styleRows ? ('class="' + options.styleRows[row] + '"') : ""

                write('        <tr' + rowAtt + styleRow + '>\r\n')

                let col = 0
				for (let [name, column] in columns) {
                    let value = view.getValue(r, name, options)
                    let styleCell: String = ""
                    if (options.styleColumns) {
                        styleCell = append(styleCell, options.styleColumns[col])
                    }
                    if (column.style) {
                        styleCell = append(styleCell, column.style)
                    }
                    if (options.styleCells && options.styleCells[row]) {
                        styleCell = append(styleCell, options.styleCells[row][col])
                    }
                    let attr = ""
                    if (styleCell) {
                        attr = ' class="' + styleCell + '"'
                    }
                    if (column.align) {
                        attr = append(attr, ' align="' + column.align + '"')
                    } else if (value is Number) {
                        attr = append(attr, ' align="right"')
                    }
                    value = view.formatValue(value, r, name, { formatter: column.formatter} )
                    write('            <td' + attr + '>' + value + '</td>\r\n')
                    col++
				}
                row++
				write('        </tr>\r\n')
			}
			write('    </tbody>\r\n</table>\r\n')
		}

		function tabs(data: Object, options: Object): Void {
            let attributes = getAttributes(options)
            let att
            if (options["data-remote"]) {
                att = "data-remote"
            } else if (options.click) {
                att = "data-click"
            } else {
                att = "data-show"
            }
            write('<div class="-ejs-tabs">\r\n    <ul>\r\n')
            if (data is Array) {
                for each (tuple in data) {
                    for (let [name, target] in tuple) {
                        let uri = request.makeUri(target)
                        write('      <li ' + att + '="' + uri + '">' + name + '</li>\r\n')
                    }
                }
            } else {
                for (let [name, target] in data) {
                    let uri = request.makeUri(target)
                    write('      <li ' + att + '="' + uri + '">' + name + '</li>\r\n')
                }
            }
            write('    </ul>\r\n</div>\r\n')
        }

        function text(field: String, value: String, options: Object): Void {
            write('    <input name="' + field + '"' + getAttributes(options) + ' type="' + getTextKind(options) + 
                '" value="' + value + '" />\r\n')
        }

        function textarea(name: String, value: String, options: Object): Void {
            numCols = options.numCols
            if (numCols == undefined) {
                numCols = 60
            }
            numRows = options.numRows
            if (numRows == undefined) {
                numRows = 10
            }
            write('<textarea name="' + name + '" type="' + getTextKind(options) + '"' + getAttributes(options) + 
                ' cols="' + numCols + '" rows="' + numRows + '">' + value + '</textarea>\r\n')
        }

        function tree(data: Object, options: Object): Void {
            options.style = append(options.style, "-ejs-tree")
            let attributes = getAttributes(options)
            let columns = getColumns(data, options)

			write('<div' + attributes + '>\r\n')
            write(serialize(data, {pretty: true}))
			write('</div>\r\n')
        }

        /************************************************** Support ***************************************************/

        private function emitFormErrors(record, options): Void {
            if (!record || !record.getErrors || options.hideErrors) {
                return
            }
            let errors = record.getErrors()
            if (errors) {
                write('<div class="-ejs-form-error"><h2>The ' + Object.getName(record).toLowerCase() + ' has ' + 
                    errors.length + (errors.length > 1 ? ' errors' : ' error') + ' that ' +
                    ((errors.length > 1) ? 'prevent' : 'prevents') + '  it being saved.</h2>\r\n')
                write('    <p>There were problems with the following fields:</p>\r\n')
                write('    <ul>\r\n')
                for (e in errors) {
                    write('        <li>' + e.toPascal() + ' ' + errors[e] + '</li>\r\n')
                }
                write('    </ul>\r\n')
                write('</div>\r\n')
            }
        }

        //  FUTURE MOB -- never called. MOB -- better to push to client via data-filter
        //  TODO - this actually modifies the grid. Need to doc this.
        private function filter(data: Array): Array {
            data = data.clone()
            pattern = request.params.filter.toLowerCase()
            for (let i = 0; i < data.length; i++) {
                let found: Boolean = false
                for each (f in data[i]) {
                    if (f.toString().toLowerCase().indexOf(pattern) >= 0) {
                        found = true
                    }
                }
                if (!found) {
                    data.remove(i, i)
                    i--
                }
            }
            return data
        }

        private function makeUri(location: Object, options: Object): Uri {
            if (location == true) {
                return request.makeUri(options)
            } else if (location is String) {
                return request.makeUri({action: location})
            }
            return request.makeUri(location)
        }

        /**
            Map options to a HTML attribute string. See htmlOptions and $View for a discussion on standard options.
            @param options Control options
            @returns a string containing the HTML attributes to emit. Will return an empty string or a string with a 
                leading space (and not trailing space)
         */
        private function getAttributes(options: Object): String {
            if (options.hasError) {
                options.style = append(options.style, "-ejs-field-error")
            }
            if (options.remote) {
                options["data-remote"] = makeUri(options.remote, options)
            } else if (options.click) {
                options["data-click"] = makeUri(options.click, options)
            }
            let result: String = ""
            for (let [key, value] in options) {
                if (value != undefined) {
                    let mapped = htmlOptions[key] || key
                    result += mapped + '="' + value + '" '
                }
            }
            return (result == "") ? "" : (" " + result.trimEnd())
        }

        private function getColumns(data: Object, options: Object): Object {
            let columns
            if (options.columns) {
                if (options.columns is Array) {
                    columns = {}
                    for each (name in options.columns) {
                        columns[name] = name
                    }
                } else {
                    columns = options.columns
                }
            } else {
                /*
                    No supplied columns, so infer from data.
                 */
                columns = {}
                if (data is Array) {
                    for (let name in data[0]) {
                        if (name == "id" && !options.showId) continue
                        columns[name] = name
                    }
                }
            }
            return columns
        }
    
        private function getTextKind(options): String {
            var kind: String
            if (options.password) {
                kind = "password"
            } else if (options.hidden) {
                kind = "hidden"
            } else {
                kind = "text"
            }
            return kind
        }

        /** 
            Get the next usable DOM ID for view controls
         */
        private function get nextDomID(): String
            "id_" + lastDomID++

        private function write(str: String): Void
            request.write(str)

        private function append(str: String, suffix: String): String {
            if (suffix) {
                return (str) ? (str + " " + suffix) : suffix
            }
            return str
        }
	}
}


/*
   @copy	default
   
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
