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

        private var nextId: Number = 0
        private var request: Request
        private var view: View

        function HtmlConnector(request, view) {
            this.request = request
            this.view = view
        }

        /*
            Options to implement:
                method
                update
                confirm     JS confirm code
                condition   JS expression. True to continue
                success
                failure
                query
   
            Not implemented
                submit      FakeFormDiv
                complete
                before
                after
                loading
                loaded
                interactive
         */

        function aform(record: Object, options: Object = {}): Void {
            options.id ||= "form"
            onsubmit = ""
            if (options.condition) {
                onsubmit += options.condition + ' && '
            }
            if (options.confirm) {
                onsubmit += 'confirm("' + options.confirm + '"); && '
            }
            onsubmit = '$.ajax({ ' +
                'uri: "' + options.uri + '", ' + 
                'type: "' + options.method + '", '

            if (options.query) {
                onsubmit += 'data: ' + options.query + ', '
            } else {
                onsubmit += 'data: $("#' + options.id + '").serialize(), '
            }

            if (options.update) {
                if (options.success) {
                    onsubmit += 'success: function(data) { $("#' + options.update + '").html(data).hide("slow"); ' + 
                        options.success + '; }, '
                } else {
                    onsubmit += 'success: function(data) { $("#' + options.update + '").html(data).hide("slow"); }, '
                }
            } else if (options.success) {
                onsubmit += 'success: function(data) { ' + options.success + '; } '
            }
            if (options.error) {
                onsubmit += 'error: function(data) { ' + options.error + '; }, '
            }
            onsubmit += '}); return false;'
            write('<form action="' + "/User/list" + '"' + getOptions(options) + "onsubmit='" + onsubmit + "' >")
        }

        /*
            TODO Extra options:
                method
                update
                confirm     JS confirm code
                condition   JS expression. True to continue
                success
                failure
                query
         */
		function alink(text: String, options: Object): Void {
            options.id ||= "alink"
            onclick = ""
            if (options.condition) {
                onclick += options.condition + ' && '
            }
            if (options.confirm) {
                onclick += 'confirm("' + options.confirm + '"); && '
            }
            onclick = '$.ajax({ ' +
                'uri: "' + options.action + '", ' + 
                'type: "' + options.method + '", '

            if (options.query) {
                'data: ' + options.query + ', '
            }

            if (options.update) {
                if (options.success) {
                    onclick += 'success: function(data) { $("#' + options.update + '").html(data); ' + 
                        options.success + '; }, '
                } else {
                    onclick += 'success: function(data) { $("#' + options.update + '").html(data); }, '
                }
            } else if (options.success) {
                onclick += 'success: function(data) { ' + options.success + '; } '
            }
            if (options.error) {
                onclick += 'error: function(data) { ' + options.error + '; }, '
            }
            onclick += '}); return false;'
            options.uri ||= request.makeUri(options)
            write('<a href="' + options.uri + '"' + getOptions(options) + "onclick='" + onclick + "' rel='nofollow'>" + 
                text + '</a>')
		}

		function button(value: String, buttonName: String, options: Object): Void {
            write('<input name="' + buttonName + '" type="submit" value="' + value + '"' + getOptions(options) + ' />')
        }

		function buttonLink(text: String, options: Object): Void {
            options.uri ||= request.makeUri(options)
			write('<button onclick="window.location=\'' + options.uri + '\';">' + text + '</button></a>')
        }

		function chart(initialData: Array, options: Object): Void {
            //  TODO
            throw 'HtmlConnector control "chart" not implemented.'
		}

		function checkbox(field: String, choice: String, submitValue: String, options: Object): Void {
            let checked = (choice == submitValue) ? ' checked="yes" ' : ''
            write('<input name="' + field + '" type="checkbox" "' + getOptions(options) + checked + 
                '" value="' + submitValue + '" />')
            write('<input name="' + field + '" type="hidden" "' + getOptions(options) + '" value="" />')
        }

		function endform(): Void {
            write('</form>')
        }

		function flash(kind: String, msg: String, options: Object): Void {
            write('<div' + getOptions(options) + '>' + msg + '</div>\r\n')
            if (kind == "inform") {
                write('<script>$(document).ready(function() {
                        $("div.-ejs-flashInform").animate({opacity: 1.0}, 2000).hide("slow");});
                    </script>')
            }
		}

		function form(record: Object, options: Object): Void {
            options.uri ||= request.makeUri(options)
            write('<form method="post" action="' + options.uri + '"' + getOptions(options) + 
                ' xonsubmit="ejs.fixCheckboxes();">')
        }

        function image(src: String, options: Object): Void {
			write('<img src="' + src + '"' + getOptions(options) + '/>')
        }

        function imageLink(src: String, options: Object): Void {
            options.uri ||= request.makeUri(options)
			//  MOB - TODO
        }

        function label(text: String, options: Object): Void {
            write('<span ' + getOptions(options) + ' type="' + getTextKind(options) + '">' +  text + '</span>')
        }

		function link(text: String, options: Object): Void {
            options.uri ||= request.makeUri(options)
			write('<a href="' + options.uri + '"' + getOptions(options) + ' rel="nofollow">' + text + '</a>')
		}

		function extlink(text: String, options: Object): Void {
            options.uri ||= request.makeUri(options)
			write('<a href="' + uri + '"' + getOptions(options) + ' rel="nofollow">' + text + '</a>')
		}

		function list(name: String, choices: Object, defaultValue: String, options: Object): Void {
            write('<select name="' + name + '" ' + getOptions(options) + '>')
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
                        isSelected = (i == defaultValue) ? 'selected="yes"' : ''
                        write('  <option value="' + i + '"' + isSelected + '>' + choice + '</option>')
                    }
                }
                i++
            }
            write('</select>')
        }

		function mail(name: String, address: String, options: Object): Void  {
			write('<a href="mailto:' + address + '" ' + getOptions(options) + ' rel="nofollow">' + name + '</a>')
		}

		function progress(initialData: Array, options: Object): Void {
            write('<p>' + initialData + '%</p>')
		}

        function radio(name: String, selected: String, choices: Object, options: Object): Void {
            let checked: String
            if (choices is Array) {
                for each (v in choices) {
                    checked = (v == selected) ? "checked" : ""
                    write(v + ' <input type="radio" name="' + name + '"' + getOptions(options) + 
                        ' value="' + v + '" ' + checked + ' />\r\n')
                }
            } else {
                for (item in choices) {
                    checked = (choices[item] == selected) ? "checked" : ""
                    write(item + ' <input type="radio" name="' + name + '"' + getOptions(options) + 
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

		function tabs(initialData: Array, options: Object): Void {
            write('<div class="-ejs-tabs">\r\n')
            write('   <ul>\r\n')
            for each (t in initialData) {
                for (name in t) {
                    let uri = t[name]
                    write('      <li onclick="window.location=\'' + uri + '\'"><a href="' + uri + '" rel="nofollow">' + 
                        name + '</a></li>\r\n')
                }
            }
            write('    </ul>')
            write('</div>')
        }

		function table(data, options: Object? = null): Void {
            let originalOptions = options
            let tableId = view.getNextId()

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

            if (options["data-remote"]) {
                attributes += ' data-remote="' + options["data-remote"] + '"'
            }
            if (options["data-apply"]) {
                attributes += ' data-apply="' + options["data-apply"] + '"'
            }

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
                let uriOptions = { controller: options.controller, query: options.query }
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
                    data = view.getValue(r, name, { render: column.render, formatter: column.formatter } )

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

        function text(field: String, value: String, options: Object): Void {
            write('<input name="' + field + '" ' + getOptions(options) + ' type="' + getTextKind(options) + 
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
            write('<textarea name="' + name + '" type="' + getTextKind(options) + '" ' + getOptions(options) + 
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

		private function getOptions(options: Object): String
            view.getOptions(options)

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
