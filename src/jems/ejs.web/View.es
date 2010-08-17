/**
    View.es -- View class 
 */
module ejs.web {

    require ejs.web

    /**
        Base class for web framework Views. This class provides the core functionality for all Ejscript view web pages.
        Ejscript web pages are compiled to create a new View class which extends the View base class. In addition to
        the properties defined by this class, user view classes will typically inherit at runtime, all public properites 
        of any associated controller object defined in Request.controller.

        CSRF
            form authenticity token
            <meta name="csrf-token" content="<%= form_authenticity_token %>" />
            <meta name="csrf-param" content="authenticity_token" />

        MOB Unobtrusive JS
            data-remote="URI | true"            - when is true useful?
            data-method="delete|post|get"
            data-confirm="Are you sure"
            data-disable-with=""
            data-update-success=
            data-update-failure=

        Many View functions accept an options attribute object. The options can take the following option properties:
            @option background String Background color. This is a CSS RGB color specification. For example "FF0000" for red.
            @option color String Foreground color. This is a CSS RGB color specification. For example "FF0000" for red.
            @option data String URI or action to get live data. The refresh option specifies how often to invoke
                fetch the data.
            @option data-remote URI to get live data or true if the control has an implicit URI (e.g. form). 
            @option data-apply String Client DOM-ID to apply the data fetched from data-remote.
            @option domid String Client-side DOM-ID to use for the control
MOB -- how does rails do this.
            @option escape Boolean Escape the text before rendering. This converts HTML reserved tags and delimiters into
                an encoded form.
            @option field String Client DOM name to use for the generated HTML element.
            @option height (Number|String) Height of the table. Can be a number of pixels or a percentage string. 
                Defaults to unlimited.
            @option id Number Database ID for the record that originated the data for the view element.
            @option method String HTTP method to invoke. May be: GET, POST, PUT or DELETE.
            @option refresh If set, this defines the data refresh period in milliseconds. Only valid if the data option 
                is defined.
            @option size (Number|String) Size of the element.
            @option style String CSS Style to use for the table.
            @option visible Boolean Make the control visible. Defaults to true.
            @option width (Number|String) Width of the table or column. Can be a number of pixels or a percentage string. 

        If this options is set to a string instead of an object hash, it is interpreted as an action method to invoke.
        i.e. It is converted into an object of the form: {action: value} where value was the original string.

        @spec ejs
        @stability prototype
     */
    enumerable dynamic class View {
        /*
            Define properties and functions are (by default) in the ejs.web namespace rather than internal to avoid clashes.
         */
        use default namespace module

        /* Cache of connector objects */
        private var connectors: Object = {}

        /* Current record being used inside a form */
        private var currentRecord: Object

        /* Data value presentation formats */
        public var formats: Object

        /* Configuration from the ejsrc configuration files */
        public var config: Object

        /** Optional controller object */
        public var controller

        /** Form and query parameters - reference to the Request.params object. */
        public var params: Object

        /** Current request object */
        public var request: Request

        /**
            Constructor method to initialize a new View
            @param request Request object
         */
        function View(request: Object) {
            if (request) {
                controller = request.controller
//  MOB -- replace all this with blend. Perhaps request and config come over automatically.
                this.request = request
                this.config = request.config
                formats = config.web.view.formats
                for each (let n: String in 
                        Object.getOwnPropertyNames(controller, {includeBases: true, excludeFunctions: true})){
                    if (n.startsWith("_")) continue
                    //  MOB - can we remove public::
                    this.public::[n] = controller[n]
                }
            }
        }

        /**
            Process and emit a view to the client. This invokes the given render function with "this" set to the view.
            @param renderer Rendering function. This may be any external function. The function will have its scope
                modified so that it executes as if it were a member method of the View class.
         */
        function render(renderer: Function): Void {
            renderer.call(this, request)
        }

        /************************************************ View Controls ***************************************************/

//  MOB - note this is input only. Does not take values from the record
        /**
            Render a form button. This creates a button suitable for use inside an input form. When the button is clicked,
            the input form will be submitted.
            @param fieldName Field name to use in the generated HTML element.
            @param label Text label to display in the button.
            @param options Optional extra options. See $View for a list of the standard options.
            Examples:
                button("commit", "OK")
         */
        function button(field: String, label: String, options: Object = {}): Void {
            options = getOptions("button", options)
            let fieldName = getField(field, options) 
            fieldName ||= label.toLowerCase()
            getConnector("button").button(fieldName, label, options)
        }

//  MOB -- action is really a uri
        /**
            Render a link button. This creates a button suitable for use outside an input form. When the button is clicked,
            the associated URI will be invoked. If data-remote is set to a valid URI, it will be invoked asynchronously
            when clicked.
            @param text Text to display in the button.
            @param target Target URI to invoke. Can be a string or  The Optional extra options. See $View for a list of the standard options.
            @option action Target action to invoke when the button is clicked.
         */
        function buttonLink(text: String, target: Object = {}): Void {
            options = getOptions("buttonLink", options)
            getConnector("buttonLink").buttonLink(text, options)
        }

        /**
            Render a chart. The chart control can display static or dynamic tabular data. The client chart control manages
            sorting by column, dynamic data refreshes, pagination and clicking on rows.
            @param data Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data.
            @param options Object Optional extra options. See also $View for a list of the standard options.
            @option columns Object hash of column entries. Each column entry is in-turn an object hash of options. If unset, 
                all columns are displayed using defaults.
            @option kind String Type of chart. Select from: piechart, table, linechart, annotatedtimeline, guage, map, 
                motionchart, areachart, intensitymap, imageareachart, barchart, imagebarchart, bioheatmap, columnchart, 
                linechart, imagelinechart, imagepiechart, scatterchart (and more)
            @option onClick String Action or URI to invoke when a chart element  is clicked.
            @example
                <% chart(null, { data: "getData", refresh: 2" }) %>
                <% chart(data, { onClick: "action" }) %>
         */
        function chart(data: Array, options: Object = {}): Void {
            options = getOptions("chart", options)
            getConnector("chart").chart(data, options)
        }

        /**
            Render an input checkbox. This creates a checkbox suitable for use within an input form. 
            @param field Name of the field to display. This is used to create a HTML "name" and "id" attribute for the 
                input element. If used inside a model form, it is the field name in the model containing the checkbox
                value to display. If used without a model, the value to display should be passed via options.value. 
            @param choice Value to submit if checked. Defaults to "true"
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function checkbox(field: String, choice: String = "true", options: Object = {}): Void {
            options = getOptions("field", options)
            let value = formatValue(currentRecord, field, options)
            let fieldName = getField(field, options) 
            getConnector("checkbox").checkbox(fieldName, choice, value, options)
        }

        /**
            End an input form. This closes an input form initiated by calling the $form method.
         */
        function endform(): Void {
            getConnector("endform").endform()
            currentRecord = undefined
        }

        /** 
            Emit a flash message area. 
            @param kinds Kinds of flash messages to display. May be a single string 
                ("error", "inform", "message", "warning"), an array of strings or null. If set to null (or omitted), 
                then all flash messages will be displayed.
            @param options Optional extra options. See $View for a list of the standard options.
            @option retain Number. Number of seconds to retain the message. If <= 0, the message is retained until another
                message is displayed. Default is 0.
            @example
                <% flash("status") %>
                <% flash() %>
                <% flash(["error", "warning"]) %>
         */

        function flash(kinds: Object = null, options: Object = {}): Void {
            options = getOptions("flash", options)
//  MOB - move flash to Request?
//            let cflash = request.flash
            let cflash = controller ? controller.flash : null
            if (cflash == null || cflash.length == 0) {
                return
            }
            let msgs: Object
            if (kinds is String) {
                msgs = {}
                msgs[kinds] = cflash[kinds]
            } else if (kinds is Array) {
                msgs = {}
                for each (kind in kinds) {
                    msgs[kind] = cflash[kind]
                }

            } else {
                msgs = cflash
            }
            for (kind in msgs) {
                let msg: String = msgs[kind]
                if (msg && msg != "") {
                    let connector = getConnector("flash", options)
                    options.style = "-ejs-flash -ejs-flash" + kind.toPascal()
                    connector.flash(kind, msg, options)
                }
            }
        }

        //  TODO - should have an option to not show validation errors
        /**
            Render a form.
            @param record Model record to edit
            @param options Optional extra options. See $View for a list of the standard options.
            @option action Action to invoke when the form is submitted. Defaults to "create" or "update" depending on 
                whether the field has been previously saved.
            @option uri String Use a URI rather than action and controller for the target uri.
         */
//  MOB -- COMPAT was form(action, record, options)
        function form(record: Object, options: Object = {}): Void {
            currentRecord = record
            log.debug(5, serialize(record, {pretty: true}))
            emitFormErrors(record)
            options = getOptions("form", options)
            options.method ||= "POST"
            options.action ||= "update"
            if (record) {
                options.id ||= record.id
            }
            let connector = getConnector("form", options)
            connector.form(record, options)
        }

        /**
            Render an image control
            @param src Optional initial source name for the image. The data option may be used with the refresh option to 
                dynamically refresh the data.
            @param options Optional extra options. See $View for a list of the standard options.
            @examples
                <% image("myPic.gif") %>
                <% image("myPic.gif", { data: "getData", refresh: 2, style: "myStyle" }) %>
         */
        function image(src: String, options: Object = {}): Void {
            options = getOptions("image", options)
            getConnector("image", options).image(src, options)
        }

        /**
            Render an input field as part of a form. This is a smart input control that will call the appropriate
                input control based on the model field data type.
            @param field Model field name containing the text data for the control.
            @param options Optional extra options. See $View for a list of the standard options.
            @examples
                <% input(modelFieldName) %>
                <% input(null, { options }) %>
         */
        function input(field: String, options: Object = {}): Void {
            try {
                datatype = Object.getType(currentRecord).getColumnType(field)

                //  TODO - needs fleshing out for each type
                switch (datatype) {
                case "binary":
                case "date":
                case "datetime":
                case "decimal":
                case "float":
                case "integer":
                case "number":
                case "string":
                case "time":
                case "timestamp":
                    text(field, options)
                    break
                case "text":
                    textarea(field, options)
                    break
                case "boolean":
                    checkbox(field, "true", options)
                    break
                default:
                    throw "input control: Unknown field type: " + datatype + " for field " + field
                }
            } catch {
                text(field, options)
            }
        }

        /**
            Render a text label field. This renders an output-only text field. Use text() for input fields.
            @param text Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data.
            @param options Optional extra options. See $View for a list of the standard options.
            @examples
                <% label("Hello World") %>
                <% label(null, { data: "getData", refresh: 2, style: "myStyle" }) %>
         */
        function label(text: String, options: Object = {}): Void {
            options = getOptions("label", options)
            getConnector("label", options).label(text, options)
        }

        //  TODO - how to do image links?
        /** 
            Emit a link to an action. The URI is constructed from the given action and the current controller. The controller
            may be overridden by setting the controller option.
            @param text Link text to display
            @param options Optional extra options. See $View for a list of the standard options.
            @option MOB action Action to invoke when the link is clicked
            @option controller String Name of the target controller for the given action
            @option uri String Use a URI rather than action and controller for the target uri.
         */
        function link(text: String, options: Object = {}): Void {
            options = getOptions("link", options)
                // MOB - Document
            options.action ||= text.split(" ")[0].toLowerCase()
            getConnector("link", options).link(text, options)
        }

        /**
            Emit a selection list. 
            @param field Name of the field to display. This is used to create a HTML "name" and "id" attribute for the 
                input element. If used inside a model form, it is the field name in the model containing the list item to
                select. If used without a model, the value to select should be passed via options.value. 
            @param choices Choices to select from. This can be an array list where each element is displayed and the value 
                returned is an element index (origin zero). It can also be an array of array tuples where the 
                first is the value to send to the app, and the second tuple entry is the value to display. Or it can be an 
                array of objects such as those returned from a table lookup. If choices is null, the $field value is 
                used to construct a model class name to use to return a data grid containing an array of row objects. 
                The first non-id field is used as the value to display.
            @param options control options
            @example
                list("stockId", Stock.stockList) 
                list("low", ["low", "med", "high"])
                list("low", [["3", "low"], ["5", "med"], ["9", "high"]])
                list("low", [{low: 3}, {med: 5}, {high: 9}])
                list("Stock Type")  // Will invoke StockType.findAll() to do a table lookup
         */
        function list(field: String, choices: Object = null, options: Object = {}): Void {
            options = getOptions("list", options)
            if (choices == null) {
                //TODO - is this de-pluralizing?
                modelTypeName = field.replace(/\s/, "").toPascal()
                modelTypeName = modelTypeName.replace(/Id$/, "")
                if (global[modelTypeName] == undefined) {
                    throw new Error("Can't find model to create list data: " + modelTypeName)
                }
                choices = global[modelTypeName].findAll()
            }
            let value = formatValue(currentRecord, field, options)
            let fieldName = getField(field, options) 
            getConnector("list", options).list(fieldName, choices, value, options)
        }

        /**
            Emit a mail link
            @param name Recipient name to display
            @param address Mail recipient address
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function mail(name: String, address: String, options: Object = {}): Void {
            options = getOptions("mail", options)
            getConnector("mail", options).mail(name, address, options)
        }

        /** 
            Emit a progress bar. MOB - Not implemented.
            @param data Optional initial data for the control. The data option may be used with the refresh option 
                to dynamically refresh the data. Progress data is a simple Number in the range 0 to 100 and represents 
                a percentage completion value.
            @param options Optional extra options. See $View for a list of the standard options.
            @example
                <% progress(null, { data: "getData", refresh: 2" }) %>
         */
        function progress(data: Object, options: Object = {}): Void {
            options = getOptions("progress", options)
            let connector = getConnector("progress", options)
            connector.progress(data, options)
            getConnector("progress", options).progress(data, options)
        }

        /** 
            Emit a radio autton. The URI is constructed from the given action and the current controller. The controller
                may be overridden by setting the controller option.
            @param field Name of the field to display. This is used to create a HTML "name" and "id" attribute for the 
                input element. If used inside a model form, it is the field name in the model containing the radio data to
                display. If used without a model, the value to display should be passed via options.value. 
            @param choices Array or object containing the option values. If array, each element is a radio option. If an 
                object hash, then they property name is the radio text to display and the property value is what is returned.
            @param options Optional extra options. See $View for a list of the standard options.
            @option controller String Name of the target controller for the given action
            @option value String Name of the option to select by default
            @example
                radio("priority", ["low", "med", "high"])
                radio("priority", {low: 0, med: 1, high: 2})
                radio(priority, Message.priorities)
         */
        function radio(field: String, choices: Object, options: Object = {}): Void {
            //  MOB -- inconsistent field. Not using "radio"
            options = getOptions("radio", options)
            let value = formatValue(currentRecord, field, options)
            let fieldName = getField(field, options) 
            getConnector("radio", options).radio(fieldName, choices, value, options)
        }

        /** 
            Emit a script link.
            @param uri URI for the script to load
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function script(uri: Object, options: Object = {}): Void {
            let connector = getConnector("script", options)
            if (uri is Array) {
                for each (u in uri) {
                    connector.script(request.home.join(u), options)
                }
            } else {
                connector.script(request.home.join(uri), options)
            }
        }

        /** 
            Emit a status control area. MOB - Not implemented.
            @param data Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data. Status data is a simple String. Status messages may be updated by calling the
                \a statusMessage function.
            @param options Optional extra options. See $View for a list of the standard options.
            @example
                <% status("Initial Status Message", { data: "getData", refresh: 2" }) %>
         */
        function status(data: Object, options: Object = {}): Void {
            options = getOptions("status", options)
            getConnector("status", options).status(data, options)
        }

        /** 
            Emit a style sheet link.
            @param uri Stylesheet uri or array of stylesheets
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function stylesheet(uri: Object, options: Object = {}): Void {
            let connector = getConnector("stylesheet", options)
            if (uri is Array) {
                for each (u in uri) {
                    connector.stylesheet(request.home.join(u), options)
                }
            } else {
                connector.stylesheet(request.home.join(uri), options)
            }
        }

        /*
            TODO table
            - in-cell editing
            - pagination
         */
        /**
            Render a table. The table control can display static or dynamic tabular data. The client table control manages
                sorting by column, dynamic data refreshes, pagination and clicking on rows. If the table supplies a URI
                or action for the data parameter, the table data is retrieved asynchronously using Ajax requests on that
                action/URI value. The action routine should call the table() control to render the data and must set the
                ajax option to true.  
            @param data Data for the control or URI/action to supply data. If data is a String, it is interpreted as a URI
                or action that will be invoked to supply HTML for the table. In this case, the refresh option defines 
                how frequently to refresh the table data. The data parameter can also be a grid of data, ie. an Array of 
                objects where each object represents the data for a row. The column names are the object property names 
                and the cell text is the object property values. The data parameter can also be a model instance.
            @param options Optional extra options. See $View for a list of the standard options.
            @option ajax Set to true if the table control is being invoked as part of an Ajax data refresh.
            @option click String Action or URI to invoke an element in the table is clicked. The click arg can be is 
                a String to apply to all cells, a single-dimension array of strings for per-row URIs, and a 
                two-dimension array for per cell URIs (order is row/column).
            @option columns Object hash of column entries. Each column entry is in-turn an object hash of options. If unset, 
                all columns are displayed using defaults. Column options: align, formatter, header, render, sort, sortOrder, 
                style, width.
            @option pageSize Number Number of rows to display per page. Omit or set to <= 0 for unlimited. 
                Defaults to unlimited.
            @option pivot Boolean Pivot the table by swaping rows for columns and vice-versa
            @option query URI query string to add to click URIs. Can be a single-dimension array for per-row query 
                strings or a two-dimensional array for per cell (order is row/column).
            @option showHeader Boolean Control if column headings are displayed.
            @option showId Boolean If a columns option is not provided, the id column is normally hidden. 
                To display, set showId to be true.
            @option sort String Enable row sorting and define the column to sort by.
            @option sortOrder String Default sort order. Set to "ascending" or "descending".Defaults to ascending.
            @option style String CSS style to use for the table.
            @option styleColumns Array of styles to use for the table body columns. Can also use the style option in the
                columns option.
            @option styleBody String CSS style to use for the table body cells
            @option styleCells Grid of styles to use for the table body cells
            @option styleHeader String CSS style to use for the table header.
            @option styleRows Array of styles to use for the table body rows
            @option styleOddRow String CSS style to use for odd data rows in the table
            @option styleEvenRow String CSS style to use for even data rows in the table
            @option title String Table title

            Column options:
            <ul>
            <li>align</li>
            <li>format</li>
            <li>formatter</li>
            <li>header</li>
            <li>render</li>
            <li>sort String Define the column to sort by and the sort order. Set to "ascending" or "descending". 
                Defaults to ascending.</li>
            <li>style</li>
            </ul>
        
            @example
                <% table("getData", { refresh: 2, pivot: true" }) %>
                <% table(gridData, { click: "edit" }) %>
                <% table(Table.findAll()) %>
                <% table(gridData, {
                    click: "edit",
                    sort: "Product",
                    columns: {
                        product:    { header: "Product", width: "20%" }
                        date:       { format: date('%m-%d-%y) }
                    }
                 }) %>
         */
        function table(data, options: Object = {}): Void {
            options = getOptions("table", options)
            let connector = getConnector("table", options)
            if (options.pivot) {
                data = pivot(data)
            }
            connector.table(data, options)
        }

        /**
            Render a tab control. The tab control can display static or dynamic tree data.
            @param data Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data. Tab data is an array of objects -- one per tab. For example:
                [{"Tab One Label", "action1"}, {"Tab Two Label", "action2"}]
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function tabs(data: Array, options: Object = {}): Void {
            options = getOptions("tabs", options)
            getConnector("tabs", options).tabs(data, options)
        }

        /**
            Render a text input field as part of a form.
            @param field Name of the field to display. This is used to create a HTML "name" and "id" attribute for the 
                input element. If used inside a model form, it is the field name in the model containing the text data to
                display. If used without a model, the value to display should be passed via options.value. 
            @param options Optional extra options. See $View for a list of the standard options.
            @option escape Boolean Escape the text before rendering. This converts HTML reserved tags and delimiters into
                an encoded form.
            @option style String CSS Style to use for the control
            @option visible Boolean Make the control visible. Defaults to true.
            @examples
                <% text("name") %>
         */
        function text(field: String, options: Object = {}): Void {
            options = getOptions("text", options)
            let value = formatValue(currentRecord, field, options)
            let fieldName = getField(field, options) 
            getConnector("text", options).text(fieldName, value, options)
        }

//  TODO - need a rich text editor. Wiki style
        /**
            Render a text area
            @param field Name of the field to display. This is used to create a HTML "name" and "id" attribute for the 
                input element. If used inside a model form, it is the field name in the model containing the text data to
                display. If used without a model, the value to display should be passed via options.value. 
            @option Boolean escape Escape the text before rendering. This converts HTML reserved tags and delimiters into
                an encoded form.
            @param options Optional extra options. See $View for a list of the standard options.
            @option data String URI or action to get data 
            @option numCols Number number of text columns
            @option numRows Number number of text rows
            @option style String CSS Style to use for the control
            @option visible Boolean Make the control visible. Defaults to true.
            @examples
                <% textarea("name") %>
         */
        function textarea(field: String, options: Object = {}): Void {
            options = getOptions("textarea", options)
            let value = formatValue(currentRecord, field, options)
            let fieldName = getField(field, options) 
            getConnector("textarea", options).textarea(fieldName, value, options)
        }

        /**
            Render a tree control. The tree control can display static or dynamic tree data.
            @param data Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data. The tree data is typically an XML document.
            @param options Optional extra options. See $View for a list of the standard options.
            @option data URI or action to get data 
            @option refresh If set, this defines the data refresh period in seconds. Only valid if the data option is defined
            @option style String CSS Style to use for the control
            @option visible Boolean Make the control visible. Defaults to true.
         */
        function tree(data: Object, options: Object = {}): Void {
            options = getOptions("tree", options)
            getConnector("tree").tree(data, options)
        }

        /***************************************** Wrapped Request Functions **********************************************/
        /**
            @duplicate Request.makeUri
         */
        function makeUri(parts: Object): Uri
            request.makeUri(parts)

        /** 
            @duplicate Request.redirect
         */
        function redirect(location: *, status: Number = Http.MovedTemporarily): Void
            request.redirect(location)

        /** 
            @duplicate Request.session 
         */
        function get session(): Session 
            request.session

        /** 
            @duplicate Request.setHeader
         */
        function setHeader(key: String, value: String, overwrite: Boolean = true): Void
            request.setHeader(key, value, overwrite)

        /**
            @duplicate Request.setHeaders
         */
        function setHeaders(headers: Object, overwrite: Boolean = true): Void
            request.setHeaders(headers, overwrite)

        /**
            @duplicate Request.setStatus
         */
        function setStatus(status: Number): Void
            request.setStatus(status)

        /** 
            @duplicate Request.show
            @hide
         */
        function show(...args): Void
            request.show(...args)

        /**
            @duplicate Request.write
         */
        function write(...data): Number
            request.write(...data)

        /*********************************************** Support ****************************************************/

        private function emitFormErrors(record): Void {
            if (!record || !record.getErrors) {
                return
            }
//  MOB -- how does this map to a grid
            let errors = record.getErrors()
            if (errors) {
                write('<div class="-ejs-formError"><h2>The ' + Object.getName(record).toLowerCase() + ' has ' + 
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

//  MOB -- never called
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

        private function getConnector(kind: String, options: Object = {}) {
            let vc = config.web.view
            let connectorName = options.connector || vc.connectors[kind] || vc.connectors["rest"]
            let name = (connectorName + "Connector").toPascal()
            if (connectors[name]) {
                return connectors[name]
            }
            return connectors[name] = new global[name](this)
/* UNUSED
            try {
                //  MOB - make connectors static without state
                return new global[name](this)
            } catch (e: Error) {
                throw new Error("Undefined view connector: " + name)
            }
*/
        }

        /**
            Get the data value for presentation.
            @param record Object containing the data to present. This could be a plain-old-object or it could be a model.
         */
        function formatValue(record: Object, field: String, options: Object): String {

            /*  MOB - OPT
                    options.value used by text() for non-form fields
                    field.split used by records containing sub records: field =>  product.name
             */
            let value = options.value
            if (value == undefined && record) {
                value = record[field]
                if (value == undefined) {
                    value = record
                    if (value) {
                        for each (let part in field.split(".")) {
                            value = value[part]
                        }
                    }
                }
                if (value == undefined) {
                    value = ""
                }
            }
            let formatter = options.formatter || formatters[typeOf(value)] || plainFormatter
            let result = formatter(this, value, record, field)
            return result
        }

        /*
            TODO - could move client side?
            Pivot the data grid. Returns a new grid, original not modified.
         */
        private function pivot(grid: Array, options: Object = {}) {
            if (!grid || grid.length == 0) return grid
            let headers = []
            let i = 0
            for (name in grid[0]) {
                headers[i++] = name
            }
            let table = []
            let row = 0
            table = []
            for each (name in headers) {
                let r = {}
                i = 1
                r[0] = name
                for (j = 0; j < grid.length; j++) {
                    r[i++] = grid[j][name]
                }
                table[row++] = r
            }
            return table
        }

        /*
            Update the options based on the model and field being considered
            MOB - but field is often just the name of the control
         */
        private function getOptions(field: String, options: Object): Object {
            if (options is String) {
                options = {action: options}
            } else if (options is Uri) {
//  MOB -- make URI needs to support this?
                options = {uri: options.toString() }
            }
            if (currentRecord) {
                options.id = currentRecord.id
                if (currentRecord.hasError(field)) {
                    options.hasError = true
                }
            }
/* WHO IS USING
//  MOB - probably needs some kind of prefix
            options.style ||= field
*/
            return options
        }

        /*
            Get a qualified field name. This will use the Record type name if defined
MOB -- no good for JS databases using POO
         */
        private function getField(field: String, options: Object): String
            options.field || (currentRecord ? (typeOf(currentRecord).toCamel() + '.' + field) : field)

        /************************************************ View Renderers **************************************************/

        var formatters = {
            Date: dateFormatter,
            String: plainFormatter,
            Number: plainFormatter,
            Boolean: plainFormatter,
            //  MOB -- put all standard types here -- faster
        }

        private static function dateFormatter(view, value, record, field)
            new Date(value).format(view.formats.Date)

        private static function plainFormatter(view, value, record, field)
            value.toString()
       
        /************************************************ View Helpers ****************************************************/
        /**
            Temporary helper function to format the date. MOB - should move to helpers somewhere
            @param fmt Format string
            @returns a formatted string
         */
        function date(fmt: String): String {
            return function (data: String): String {
                return new Date(data).format(fmt)
            }
        }

        /**
            Temporary helper function to format a number as currency. MOB
            @param fmt Format string
            @returns a formatted string
         */
        function currency(fmt: String): String {
            return function (data: String): String {
                return fmt.format(data)
            }
        }

        /**
            Temporary helper function to format a number. MOB
            @param fmt Format string
            @returns a formatted string
         */
        function number(fmt: String): String {
            return function (data: String): String {
                return fmt.format(data)
            }
        }

        /*************************************************** Deprecated ***************************************************/
        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function makeUrl(action: String, id: String = null, options: Object = {}, query: Object = null): String 
            request.makeUrl(action, id, options, query)

        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function get appUrl()
            request.appUrl
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
