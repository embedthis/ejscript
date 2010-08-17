/**
   Html.es -- MVC HTML view support
 */

module ejs.web {

//  MOB -- should not be public
	/**
	    The Html Connector provides bare HTML encoding of Ejscript controls
        @stability prototype
        @spec ejs
        @hide
	 */
	public class HtmlConnector {

        use default namespace module

        private var request: Request
        private var view: View

//  MOB -- all elements must have a DOM-ID
        /* Sequential DOM ID generator */
        private var nextDomID: Number = 0

        /*
            Mapping of helper options to HTML attributes ("" value means don't map the name)
         */
        private const htmlOptions: Object = { 
            background: "",
            color: "",
            domid: "id",
            height: "",
            method: "",
            size: "",
            style: "class",
            visible: "",
            width: "",
        }
        function HtmlConnector(view) {
            this.view = view
            this.request = view.request
        }

//  MOB - what about data-remote?
		function button(field: String, label: String, options: Object): Void {
            write('<input name="' + field + '" type="submit" value="' + label + '"' + getAttributes(options) + ' />')
        }

		function buttonLink(text: String, options: Object): Void {
            let options.uri ||= request.makeUri(options)
            if (options["data-remote"]) {
                let attributes = getDataAttributes(options)
                write('<button ' + attributes + '>' + text + '</button></a>')
            } else {
                write('<button onclick="window.location=\'' + options.uri + '\';">' + text + '</button></a>')
            }
        }

		function chart(initialData: Array, options: Object): Void {
            //  TODO
            throw 'HtmlConnector control "chart" not implemented.'
		}

		function checkbox(field: String, choice: String, submitValue: String, options: Object): Void {
            let checked = (choice == submitValue) ? ' checked="yes" ' : ''
            //  MOB -- should these have \r\n at the end of each line?
            write('<input name="' + field + '" type="checkbox" "' + getAttributes(options) + checked + 
                '" value="' + submitValue + '" />\n')
            write('    <input name="' + field + '" type="hidden" "' + getAttributes(options) + '" value="" />')
        }

		function endform(): Void {
            write('</form>')
        }

		function form(record: Object, options: Object): Void {
            options.uri ||= request.makeUri(options)
            write('<form method="' + options.method + '" action="' + options.uri + '"' + getAttributes(options) + 
                ' xonsubmit="ejs.fixCheckboxes();">')
            if (options.id) {
                //  MOB -- should this be some more unique field?
                write('<input name="id" type="hidden" value="' + options.id + '" />')
            }
        }

        function image(src: String, options: Object): Void {
			write('<img src="' + src + '"' + getAttributes(options) + '/>')
        }

        function imageLink(src: String, options: Object): Void {
            options.uri ||= request.makeUri(options)
			//  MOB - TODO
        }

        function label(text: String, options: Object): Void {
            write('<span ' + getAttributes(options) + ' type="' + getTextKind(options) + '">' +  text + '</span>')
        }

		function link(text: String, options: Object): Void {
            options.uri ||= request.makeUri(options)
			write('<a href="' + options.uri + '"' + getAttributes(options) + ' rel="nofollow">' + text + '</a>')
		}

		function list(name: String, choices: Object, defaultValue: String, options: Object): Void {
            write('<select name="' + name + '" ' + getAttributes(options) + '>')
            let isSelected: Boolean
            let i = 0
            for each (choice in choices) {
                if (choice is Array) {
                    isSelected = (choice[0] == defaultValue) ? 'selected="yes"' : ''
                    write('  <option value="' + choice[0] + '"' + isSelected + '>' + choice[1] + '</option>')
                } else {
                    if (choice && choice.id) {
                        for (field in choice) {
                            isSelected = (choice.id == defaultValue) ? 'selected="yes"' : ''
                            if (field != "id") {
                                write('  <option value="' + choice.id + '"' + isSelected + '>' + choice[field] + '</option>')
                                done = true
                                break
                            }
                        }
                    } else {
                        isSelected = (choice == defaultValue) ? 'selected="yes"' : ''
                        write('  <option value="' + choice + '"' + isSelected + '>' + choice + '</option>')
                    }
                }
                i++
            }
            write('</select>')
        }

		function mail(name: String, address: String, options: Object): Void  {
			write('<a href="mailto:' + address + '" ' + getAttributes(options) + ' rel="nofollow">' + name + '</a>')
		}

		function flash(kind: String, msg: String, options: Object): Void {
            write('<div' + getAttributes(options) + '>' + msg + '</div>\r\n')
            if (kind == "inform") {
                write('<script>$(document).ready(function() {
                        $("div.-ejs-flashInform").animate({opacity: 1.0}, 2000).hide("slow");});
                    </script>')
            }
		}

		function progress(initialData: Array, options: Object): Void {
            write('<p>' + initialData + '%</p>')
		}

        function radio(name: String, choices: Object, selected: String, options: Object): Void {
            let checked: String
            if (choices is Array) {
                for each (v in choices) {
                    checked = (v == selected) ? "checked" : ""
                    write(v + ' <input type="radio" name="' + name + '"' + getAttributes(options) + 
                        ' value="' + v + '" ' + checked + ' />\r\n')
                }
            } else {
                for (item in choices) {
                    checked = (choices[item] == selected) ? "checked" : ""
                    write(item + ' <input type="radio" name="' + name + '"' + getAttributes(options) + 
                        ' value="' + choices[item] + '" ' + checked + ' />\r\n')
                }
            }
        }

		function script(uri: String, options: Object): Void {
            write('<script src="' + uri + '" type="text/javascript"></script>\r\n')
		}

		function status(initialData: Array, options: Object): Void {
            write('<p>' + initialData + '</p>\r\n')
        }

		function stylesheet(uri: String, options: Object): Void {
            write('<link rel="stylesheet" type="text/css" href="' + uri + '" />\r\n')
		}

		function table(data, options: Object? = null): Void {
            let originalOptions = options
                //  MOB -- should come via getAttributes
            let tableId = nextDomID

            if (data is Array) {
                if (data.length == 0) {
                    write("<p>No Data</p>")
                    return
                }
            } else if (!(data is Array) && data is Object) {
                data = [data]
			}
            options = (originalOptions && originalOptions.clone()) || {}
            let columns = getColumns(data, options)

            let refresh = options.refresh || 10000
            let sortOrder = options.sortOrder || ""
            let sort = options.sort
            if (sort == undefined) sort = true
            let attributes = getDataAttributes(options)

            //  TODO - would be nice to auto sense this
            if (!options.ajax) {
                let uri = (data is String) ? data : null
                uri ||= options.data
                write('  <script type="text/javascript">\r\n' +
                    '   $(function() { $("#' + tableId + '").eTable({ refresh: ' + refresh + 
                    ', sort: "' + sort + '", sortOrder: "' + sortOrder + '"' + 
                    ((uri) ? (', uri: "' + uri + '"'): "") + 
                    '})});\r\n' + 
                    '  </script>\r\n')
                if (data is String) {
                    /* Data is an action method */
                    write('<table id="' + tableId + '" class="-ejs-table"></table>\r\n')
                    return
                }
            } else {
                write('  <script type="text/javascript">$("#' + tableId + '").eTableSetOptions({ refresh: ' + refresh +
                    ', sort: "' + sort + '", sortOrder: "' + sortOrder + '"})' + ';</script>\r\n')
            }
			write('  <table id="' + tableId + '" class="-ejs-table ' + (options.styleTable || "" ) + '"' + 
                attributes + '>\r\n')

            /*
                Table title and column headings
             */
            if (options.showHeader != false) {
                write('    <thead class="' + (options.styleHeader || "") + '">\r\n')
                if (options.title) {
                    let gif = request.home.join("/web/images/green.gif")
                    if (columns.length < 2) {
                        //  TODO - this icon should be styled and not be here
                        write('  <tr><td>' + options.title + ' ' + '<img src="' + 
                            gif + '" class="-ejs-table-download -ejs-clickable" onclick="$(\'#' + 
                            tableId + '\').eTableToggleRefresh();" />\r\n  </td></tr>\r\n')
                    } else {
                        write('  <tr><td colspan="' + (columns.length - 1) + '">' + options.title + 
                            '</td><td class="right">' + '<img src="' + gif + 
                            '" class="-ejs-table-download -ejs-clickable" onclick="$(\'#' + tableId + 
                            '\').eTableToggleRefresh();" />\r\n  </td></tr>\r\n')
                    }
                }
                /*
                    Emit column headings
                 */
                if (columns) {
                    write('    <tr>\r\n')
                    for (let name in columns) {
                        if (name == null) continue
                        let header = (columns[name].header) ? (columns[name].header) : name.toPascal()
                        let width = (columns[name].width) ? ' width="' + columns[name].width + '"' : ''
                        write('    <th ' + width + '>' + header + '</th>\r\n')
                    }
                }
                write("     </tr>\r\n    </thead>\r\n")
            }

            let styleBody = options.styleBody || ""
            write('    <tbody class="' + styleBody + '">\r\n')

            let row: Number = 0

			for each (let r: Object in data) {
                let uri = null
                // let uriOptions = { controller: options.controller, query: options.query }
                let uriOptions = options.clone()
                if (options.click) {
                    uriOptions.query = (options.query is Array) ? options.query[row] : options.query
                    if (options.click is Array) {
                        if (options.click[row] is String) {
                            uri = request.makeUri(blend(uriOptions, {action: options.click[row], id: r.id}))
                        }
                    } else {
                        uri = request.makeUri(blend(uriOptions, {action: options.click, id: r.id}))
                    }
                }
                let odd = options.styleOddRow || "-ejs-oddRow"
                let even = options.styleOddRow || "-ejs-evenRow"
                styleRow = ((row % 2) ? odd : even) || ""
                if (options.styleRows) {
                    styleRow += " " + (options.styleRows[row] || "")
                }
                if (uri) {
                    write('    <tr class="' + styleRow + 
                        '" onclick="window.location=\'' + uri + '\';">\r\n')
                } else {
                    write('    <tr class="' + styleRow + '">\r\n')
                }

                let col = 0
				for (name in columns) {
                    if (name == null) {
                        continue
                    }
                    let column = columns[name]
                    let styleCell: String = ""

                    if (options.styleColumns) {
                        styleCell = options.styleColumns[col] || ""
                    }
                    if (column.style) {
                        styleCell += " " + column.style
                    }
                    if (options.styleCells && options.styleCells[row]) {
                        styleCell += " " + (options.styleCells[row][col] || "")
                    }
                    styleCell = styleCell.trim()
                    data = view.formatValue(r, name, { format: column.format} )

                    let align = ""
                    if (column.align) {
                        align = 'align="' + column.align + '"'
                    }
                    let cellUrl
                    if (options.click is Array && options.click[0] is Array) {
                        if (options.query is Array) {
                            if (options.query[0] is Array) {
                                uriOptions.query = options.query[row][col]
                            } else {
                                uriOptions.query = options.query[row]
                            }
                        } else {
                            uriOptions.query = options.query
                        }
                        cellUrl = request.makeUri(blend(uriOptions, { action: options.click[row][col], id: r.id}))
                    }
					styleCell = styleCell.trim()
                    if (cellUrl) {
                        write('    <td class="' + styleCell + '"' + align + 
                            ' xonclick="window.location=\'' + cellUrl + '\';"><a href="' + cellUrl + '" rel="nofollow">' + 
                            data + '</a></td>\r\n')
                    } else {
                        write('    <td class="' + styleCell + '"' + align + '>' + data + '</td>\r\n')
                    }
                    col++
				}
                row++
				write('    </tr>\r\n')
			}
			write('    </tbody>\r\n  </table>\r\n')
		}

		function tabs(data: Array, options: Object): Void {
            write('<div class="-ejs-tabs">\r\n')
            write('   <ul>\r\n')
            for each (t in data) {
                for (name in t) {
                    let uri = t[name]
                    if (options["data-remote"]) {
                        write('      <li data-remote="' + uri + '">' + name + '</a></li>\r\n')
                    } else {
//  MOB -- get rid of all window.location
                        write('      <li onclick="window.location=\'' + uri + '\'"><a href="' + uri + '" rel="nofollow">' + 
                            name + '</a></li>\r\n')
                    }
                }
            }
            write('    </ul>')
            write('</div>')
        }

        function text(field: String, value: String, options: Object): Void {
            write('<input name="' + field + '" ' + getAttributes(options) + ' type="' + getTextKind(options) + 
                '" value="' + value + '" />')
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
            write('<textarea name="' + name + '" type="' + getTextKind(options) + '" ' + getAttributes(options) + 
                ' cols="' + numCols + '" rows="' + numRows + '">' + value + '</textarea>')
        }

        function tree(initialData: Array, options: Object): Void {
            throw 'HtmlConnector control "tree" not implemented.'
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
                    No supplied columns. Infer from data
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

        //  MOB -- merge in with getAttributes
        private function getDataAttributes(options): String {
            let attributes = ""
            //  MOB -- would it be better to have data-remote == uri?
            if (options["data-remote"]) {
                attributes += ' data-remote="' + options["data-remote"] + '"'
            }
            if (options["data-apply"]) {
                attributes += ' data-apply="' + options["data-apply"] + '"'
            }
            if (options["data-id"]) {
                attributes += ' data-id="' + options["data-id"] + '"'
            }
            return attributes
        }

        //  MOB -- edit this down to just HTML attributes
        /**
            Map options to a HTML attribute string.
            @param options Optional extra options.
            @returns a string containing the HTML attributes to emit.
            @option background String Background color. This is a CSS RGB color specification. For example "FF0000" for red.
            @option color String Foreground color. This is a CSS RGB color specification. For example "FF0000" for red.
            @option data String URL or action to get live data. The refresh option specifies how often to invoke
                fetch the data.
            @option domid Number Client DOM-ID to use for the generated element
            @option height (Number|String) Height of the table. Can be a number of pixels or a percentage string. 
                Defaults to unlimited.
            @option method String HTTP method to invoke. May be: GET, POST, PUT or DELETE.
            @option size (Number|String) Size of the element.
            @option style String CSS Style to use for the table.
            @option visible Boolean Make the control visible. Defaults to true.
            @option width (Number|String) Width of the table or column. Can be a number of pixels or a percentage string. 
         */
        function getAttributes(options: Object): String {
            if (!options.domid) {
                options.domid = nextDomID
            }
            if (options.hasError) {
                //  MOB -- need consistency with styles
                options.style += " -ejs-fieldError"
            }
            let result: String = ""
            for (let option: String in options) {
                let mapped = htmlOptions[option]
                if (mapped || mapped == "") {
                    if (mapped == "") {
                        /* No mapping, keep the original option name */
                        mapped = option
                    }
                    result += ' ' +  mapped + '="' + options[option] + '"'
                } else if (option.startsWith("data-")) {
                    result += ' ' +  option + '="' + options[option] + '"'
                }
            }
            return result + " "
        }

        /** 
            Get the next usable DOM ID for view controls
         */
        function get nextDomID(): String
            "id_" + nextDomID++

/* MOB Functionality moved to getAttributes
        private function getOptions(field: String, options: Object): Object {
            let record = view.currentRecord
            if (record) {
                if (record.id) {
                    options.domid ||= field + '_' + record.id
                }
             */
            }
//  MOB - probably needs some kind of prefix
            options.style ||= field
            return options
        }
*/

//  MOB -- what other should be aliased. Check Request and Control:  flash?
        private function write(str: String): Void
            request.write(str)
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
