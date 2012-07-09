/**
   Html.es -- HtmlViewConnector. This provides HTML view support.
 */

module ejs.web {

    /**
        The Html Connector provides bare HTML encoding of View controls.

        TODO Style conventions???
            -ejs- prefixes all internal styles
            -ejs-alert
            -ejs-flash
            -ejs-flash-inform
            -ejs-flash-warn
            -ejs-flash-error
            -ejs-hidden
            -ejs-tabs
            -ejs-field-error
            -ejs-progress
            -ejs-progress-inner
            -ejs-table
            -ejs-table-download
            -ejs-even-row
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
        private static var lastDomID: Number = 0

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
            "effects":              "data-effects",
            "height":               "height",
            "method":               "data-method",
            "modal":                "data-modal",
            "period":               "data-refresh-period",
            "pivot":                "data-pivot",
            "rel":                  "rel",
            "size":                 "size",
            "sort":                 "data-sort",
            "sortOrder":            "data-sort-order",
            "style":                "class",
            "width":                "width",
        }

        //  MOB -- need to be able to control treeview or not
        private static const defaultStylesheets = [
            "/layout.css", 
            "/themes/default.css", 
            "/js/treeview.css", 
        ]

        //  MOB -- Should offer all-in-one script
        private static const defaultScripts = [
            "/js/jquery", 
            "/js/jquery.tablesorter",
            "/js/jquery.simplemodal",
            "/js/jquery.treeview",
            "/js/jquery.ejs",
        ]

        function HtmlViewConnector(view) {
            this.view = view
            this.request = view.request
        }

        function alert(text: String, options: Object): Void {
            options.style = append(options.style, "-ejs-alert")
            write('<div' + getAttributes(options) + '>' +  text + '</div>\r\n')
        }

        function anchor(text: String, options: Object): Void {
            let att = getAttributes(options, {"data-click": true})
            write('<a href="' + options.data-click + '"' + att + '>' + text + '</a>\r\n')
        }

        function button(name: String, value: String, options: Object): Void {
            write('    <input name="' + name + '" type="submit" value="' + value + '"' + getAttributes(options) + ' />\r\n')
        }

        function buttonLink(text: String, options: Object): Void {
            let attributes = getAttributes(options)
            write('<button' + attributes + '>' + text + '</button>\r\n')
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

        function div(body: String, options: Object): Void {
            write('<div' + getAttributes(options) + '>' +  body + '</div>\r\n')
        }

        function endform(): Void {
            write('</form>\r\n')
        }

        function flash(kind: String, msg: String, options: Object): Void {
            options.style = append(options.style, "-ejs-flash -ejs-flash-" + kind)
            write('<div' + getAttributes(options) + '>' + msg + '</div>\r\n')
        }

        function form(record: Object, options: Object): Void {
            let method ||= options.method || ((record && options.id) ? "PUT" : "POST")
            options.action ||= options.click
            options.action ||= ((record && options.id) ? "@update" : "@create")
            if (method != "GET" && method != "POST") {
                options.method = method
                method = "POST"
            }
            let uri = request.link(options)
            emitFormErrors(record, options)
            let attributes = getAttributes(options, {action: true, "data-click": true})
            write('<form method="' + method + '" action="' + uri + '"' + attributes + '>\r\n')
            if (options.id != undefined) {
                write('    <input name="id" type="hidden" value="' + options.id + '" />\r\n')
            }
            if (!options.insecure) {
                let token = options.securityToken || request.securityToken
                write('    <input name="' + Request.SecurityTokenName + '" type="hidden" value="' + token + '" />\r\n')
            }
        }

        function icon(uri: String, options: Object): Void {
            //  MOB - what other places should support use request.link
            uri = request.link(uri)
            write('    <link href="' + uri + '" rel="shortcut icon" />\r\n')
        }

        function image(src: String, options: Object): Void {
            src = request.link(src)
            write('<img src="' + src + '"' + getAttributes(options) + '/>\r\n')
        }

        function label(text: String, options: Object): Void {
            write('<span' + getAttributes(options) + '>' +  text + '</span>\r\n')
        }

        function list(name: String, choices: Object, defaultValue: String?, options: Object): Void {
            let selected
            write('    <select name="' + name + '" ' + getAttributes(options) + '>\r\n')
            if (choices is Array) {
                let i = 0
                for each (choice in choices) {
                    if (choice is Array) {
                        /* list("priority", [["low", "3"], ["med", "5"], ["high", "9"]]) */
                        /* Note: value is first */
                        let [value, key] = choice
                        selected = (key == defaultValue) ? ' selected="yes"' : ''
                        write('      <option value="' + key + '"' + selected + '>' + value + '</option>\r\n')

                    } else if (choice && choice.id) {
                        /* list("priority", [{id: 77, field: "value", ...}, ...]) */
                        selected = (choice.id == defaultValue) ? ' selected="yes"' : ''
                        for (let [key, value] in choice) {
                            if (key != "id" && !(value is Function)) {
                                write('      <option value="' + choice.id + '"' + selected + '>' + value + '</option>\r\n')
                                break
                            }
                        }
                    } else if (Object.getOwnPropertyCount(choice) > 0) {
                        /* list("priority", [{low: 3}, {med: 5}, {high: 9}]) */
                        for (let [value, key] in choice) {
                            selected = (key == defaultValue) ? ' selected="yes"' : ''
                            write('      <option value="' + key + '"' + selected + '>' + value + '</option>\r\n')
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
                for (let [value, key]  in choices) {
                    selected = (key == defaultValue) ? ' selected="yes"' : ''
                    write('      <option value="' + key + '"' + selected + '>' + value + '</option>\r\n')
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

        function radio(name: String, actual: String, choices: Object, options: Object): Void {
            let checked: String
            let attributes = getAttributes(options)
            if (choices is Array) {
                for each (choice in choices) {
                    if (choice is Array) {
                        /* radio("priority", [["3", "low"], ["5", "med"], ["9", "high"]]) */
                        let [key, value] = choice
                        checked = (value == actual) ? " checked" : ""
                        write('    ' + key.toPascal() + ' <input type="radio" name="' + name + '"' + attributes + 
                            ' value="' + value + '"' + checked + ' />\r\n')

                    } else if (Object.getOwnPropertyCount(choice) > 0) {
                        /* radio("priority", [{low: 3}, {med: 5}, {high: 9}]) */
                        for (let [key, value] in choice) {
                            checked = (value == actual) ? " checked" : ""
                            write('  ' + key.toPascal() + ' <input type="radio" name="' + name + '"' + attributes + 
                                ' value="' + value + '"' + checked + ' />\r\n')
                        }

                    } else {
                        /* radio("priority", ["low", "med", "high"]) */
                        checked = (choice == actual) ? " checked" : ""
                        write("    " + choice.toPascal() + 
                            ' <input type="radio" name="' + name + '"' + attributes + ' value="' + 
                            choice + '"' + checked + ' />\r\n')
                    }
                }
            } else {
                /* radio("priority", {low: 0, med: 1, high: 2}) */
                for (let [key, value] in choices) {
                    checked = (value == actual) ? " checked" : ""
                    write("    " + key.toPascal() + ' <input type="radio" name="' + name + '"' + attributes + ' value="' + 
                        value + '"' + checked + ' />\r\n')
                }
            }
        }

        function refresh(on: String, off, options: Object): Void {
            write('    <img src="' + on + '" data-on="' + on + '" data-off="' + off + '" class="-ejs-refresh" />')
        }

        function script(uri: String?, options: Object): Void {
            if (uri == null) {
                if (options.minified == undefined) {
                    options.minified = true
                }
                /* MVC directory */
                let dirs = request.config.dirs
                let sdir = (dirs && dirs.static) ? dirs.static.basename : "static"
                for each (uri in defaultScripts) {
                    uri = request.link("/" + sdir + uri)
                    uri += (options.minified) ? ".min.js" : ".js"
                    write('    <script src="' + uri + '" type="text/javascript"></script>\r\n')
                }
            } else {
                //MOB uri = request.link(uri)
                write('    <script src="' + uri + '" type="text/javascript"></script>\r\n')
            }
        }

        function securityToken(options: Object): Void {
            write('    <meta name="SecurityTokenName" content="' + Request.SecurityTokenName + '" />\r\n')
            write('    <meta name="' + Request.SecurityTokenName + '" content="' + request.securityToken + '" />\r\n')
        }

        function stylesheet(uri: String?, options: Object): Void {
            if (uri == null) {
                /* MVC directory */
                let dirs = request.config.dirs
                let sdir = (dirs && dirs.static) ? dirs.static.basename : "static"
                for each (uri in defaultStylesheets) {
                    uri = request.link("/" + sdir + uri)
                    write('    <link rel="stylesheet" type="text/css" href="' + uri + '" />\r\n')
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
    /*
            let attributes = getAttributes({
                apply: options.apply,
                period: options.period,
                refresh: options.refresh,
                sort: options.sort,
                sortOrder: options.sortOrder || "ascending",
                style: options.style,
            })
    */
            let attributes = getAttributes(options, {"data-click": true})
            let columns = getColumns(data, options)

            write('  <table' + attributes + '>\r\n')

            /*
                Table title header and column headings
             */
            if (options.showHeader != false) {
                write('    <thead>\r\n')
                if (options.title) {
                    let length = Object.getOwnPropertyCount(columns)
                    write('        <tr class="-ejs-table-title"><td colspan="' + length + '">' + 
                        options.title + '</td></tr>\r\n')
                }
                write('        <tr class="-ejs-table-header">\r\n')
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
            let row = 0
            for each (let r: Object in data) {
                let values = {}
                for (name in columns) {
                    values[name] = view.getValue(r, name, options)
                }
                let rowOptions = {
                    click: options.click,
                    edit: options.edit,
                    id: r.id,
                    key: options.key,
                    params: options.params,
                    remote: options.remote,
                }
                let styleRow = ""
                if (options.styleRows && options.styleRows[row]) {
                    styleRow = ' class="' + options.styleRows[row] + '"'
                }
                //  MOB - why not set these above when defining rowOptions
                rowOptions.record = r
                rowOptions.field = null
                rowOptions.values = values
                let att = getAttributes(rowOptions)
                write('        <tr' + att + styleRow + '>\r\n')

                let col = 0
                for (let [name, column] in columns) {
                    let value = values[name]
                    let styleCell: String = ""
                    if (options.styleColumns) {
                        styleCell = append(styleCell, options.styleColumns[col])
                    }
                    if (column.style) {
                        if (column.style is Array) {
                            styleCell = append(styleCell, column.style[r])
                        } else {
                            styleCell = append(styleCell, column.style)
                        }
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
                    if (column.click || column.edit /* MOB options.cell */) {
                        /* really cell options */
                        rowOptions.record = r
                        rowOptions.row = row
                        rowOptions.field = name
                        rowOptions.values = values
                        rowOptions.key = column.key
                        rowOptions.click = column.click
                        rowOptions.edit = column.edit
                        attr = append(attr, getAttributes(rowOptions))
                    }
                    value = view.formatValue(value, { formatter: column.formatter} )
                    write('            <td' + attr + '>' + value + '</td>\r\n')
                    col++
                }
                row++
                write('        </tr>\r\n')
            }
            write('    </tbody>\r\n</table>\r\n')
        }

        function tabs(data: Object, options: Object): Void {
            let attributes = getAttributes(options, {"data-remote": true})
            let att
            if (options.toggle) {
                att = "data-toggle"
            } else if (options.remote) {
                att = "data-remote"
            } else {
                att = "data-click"
            }
            write('<div class="-ejs-tabs"' + attributes + '>\r\n    <ul>\r\n')
            if (data is Array) {
                for each (tuple in data) {
                    for (let [name, target] in tuple) {
                        let uri = (att == "data-toggle") ? target : request.link(target)
                        write('      <li ' + att + '="' + uri + '">' + name + '</li>\r\n')
                    }
                }
            } else {
                for (let [name, target] in data) {
                    let uri = (att == "data-toggle") ? target : request.link(target)
                    write('      <li ' + att + '="' + uri + '">' + name + '</li>\r\n')
                }
            }
            write('    </ul>\r\n</div>\r\n')
        }

        function text(field: String, value: String, options: Object): Void {
            if (options.rows) {
                let cols = options.size
                if (cols == undefined) {
                    cols = 60
                }
                let rows = options.rows
                if (rows == undefined) {
                    rows = 10
                }
                att = getAttributes(options, {size: true})
                write('    <textarea name="' + field + '" type="' + getTextKind(options) + '"' + att + 
                    ' cols="' + cols + '" rows="' + rows + '">' + value + '</textarea>\r\n')
            } else {
                write('    <input name="' + field + '"' + getAttributes(options) + ' type="' + getTextKind(options) + 
                    '" value="' + value + '" />\r\n')
            }
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
                let count = Object.getOwnPropertyCount(errors)
                write('<div class="-ejs-form-error"><h2>The ' + typeOf(record).toLowerCase() + ' has ' + 
                    count + (count > 1 ? ' errors' : ' error') + ' that ' +
                    ((count > 1) ? 'prevent' : 'prevents') + '  it being saved.</h2>\r\n')
                write('    <p>There were problems with the following fields:</p>\r\n')
                write('    <ul>\r\n')
                for (e in errors) {
                    write('        <li>' + e.toPascal() + ' ' + errors[e] + '</li>\r\n')
                }
                write('    </ul>\r\n')
                write('</div>\r\n')
            }
        }

/*
        //  FUTURE -- never called. -- better to push to client via data-filter
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
*/

        /*
            Set the template key fields based on the record data
            This defines target[property] = record[property]. The property name is defined in "keyFields"
            Key fields can be an array of property names e.g. ["name", "id"], or they can provide mapping:
            e.g. ["name", {buildId: "id"}]
        */
        private function setKeyFields(target: Object, keyFields: Array, options: Object): Void {
            let record = options.record
            let row = options.row
            let values = options.values
            for (name in record) {
                // Add missing values incase columns are not being displayed 
                values[name] ||= record[name]
            }
            for each (key in keyFields) {
                if (key is String) {
                    // Array of key names corresponding to the columns 
                    let value = (values[key] != null) ? Uri.encodeComponent(values[key]) : row
                    target[key] = value
                } else {
                    // Hash of key:mapped names corresponding to the columns
                    for (field in key) {
                        let value = (values[field] != null) ? Uri.encodeComponent(values[field]) : row
                        target[key[field]] = value
                    }
                }
            }
        }

        /**
            Map options to a HTML attribute string. See htmlOptions and $View for a discussion on standard options.
            @param options Control options
            @returns a string containing the HTML attributes to emit. Will return an empty string or a string with a 
                leading space (and not trailing space)
         */
        function getAttributes(options: Object, exclude: Object? = null): String {
            if (options.hasError) {
                options.style = append(options.style, "-ejs-field-error")
            }
            if (options.click) {
                setLinkOptions(options.click, options, "data-click")

            } else if (options.remote) {
                //  MOB cleanup - data-remote only ever allow a boolean value.
                if (options.remote == true && options.action) {
                    options.remote = options.action
                }
                setLinkOptions(options.remote, options, "data-remote")

            } else if (options.edit) {
                setLinkOptions(options.edit, options, "data-edit")

            }
            if (options.refresh) {
                options.domid ||= getNextID()
                setLinkOptions(options.refresh, options, "data-refresh")
            }
            return mapAttributes(options, exclude)
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

        /*
            Get the next usable DOM ID for view controls
         */
        private function getNextID(): String
            "id_" + lastDomID++

        /*
            Map options to an HTML attribute string
         */
        private function mapAttributes(options: Object, exclude: Object? = null): String {
            let result: String = ""
            if (options.method) {
                options.method = options.method.toUpperCase();
            }
            for (let [key, value] in options) {
                if (exclude && exclude[key]) continue
                if (value != undefined) {
                    if (htmlOptions[key] || key.startsWith("data-")) {
                        let mapped = htmlOptions[key] ? htmlOptions[key] : key
                        result += mapped + '="' + value + '" '
                    } else if (key == "pass") {
                        result += value + ' '
                    }
                }
            }
            return (result == "") ? result : (" " + result.trimEnd())
        }

//  MOB
        /*
            Set option fields for a target URI:
                options[prefix] == uri
                options[prefix-method]
                options[prefix-params]
            @param target The URI target
            @param options The options object hash
            @param prefix The property prefix to use in options
         */
        private function setLinkOptions(target: Object, options: Object, prefix: String) {
            if (typeOf(target) != "Object") {
                target = target.toString()
                if (target[0] == '@') {
                    target = {action: target}
                } else {
                    /* Non-mvc URI string */
                    target = {uri: (target[0] == '/') ? (request.scriptName + target) : target}
                }
            } else {
                target = target.clone()
            }
            blend(target, options, {overwrite: false})
            if (options.key && options.record) {
                /* Set template key fields */
                setKeyFields(target, options.key, options)
            }
            target.uri ||= request.link(target)
            if (target.params) {
//  MOB - should be a utility routine
                /* Process params and convert to an encoded query string */
                let list = []
                for (let [k,v] in target.params) {
                    list.push(Uri.encodeComponent(k) + "=" + Uri.encodeComponent(v))
                }
                target.params = list.join("&")
            }
            options[prefix] = target.uri
            if (target.method) {
                options[prefix + "-" + "method"] = target.method
                delete options.method
            }
            if (target.params) {
                options[prefix + "-" + "params"] = target.params
            }
        }

        private function write(str: String): Void
            request.write(str)

        //  TODO OPT
        private function append(str: String?, suffix: String?): String {
            if (suffix) {
                return (str) ? (str + " " + suffix) : suffix
            }
            return str
        }
    }
}


/*
   @copy    default
   
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
