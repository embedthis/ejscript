/**
    View.es -- View class 
 */
module ejs.web {

    require ejs.web

    /**
        Base class for web framework Views. This class provides the core functionality for templated Ejscript view 
        web pages. Ejscript web pages are compiled to create a new View class which extends the View base class.  
        This class provides a suite of high-level control methods that generate HTML for input, output and 
        presentation needs.  In addition to the properties defined by this class, user view classes will typically 
        inherit at runtime, all public properites of any associated controller object defined in Request.controller.

        <h4>Control Methods</h4>
        Control methods are grouped into two families: input form controls and general output controls. Input controls
        are typically located inside a form/endform control pair that defines a current data record from which data
        will be presented. Output controls can be used anywhere on a page.

        Input controls are generally of the form: function(field, options) where field is the name of the property
        in the current record that contains the data to display. The options is an object hash that controls and modifies
        how the control will render. The options hash can also be a plain string, in which case it is interpreted as an 
        object hash with a single "action" property set to the value of the options string. i.e. {action: options}. Note
        that controls will modify the options object and so sharing one option set over many controls is not advisable.

        Various controls have custom options, but they share the following common set of option properties:

        @option apply String Client DOM-ID to apply the data fetched from the $remote URI.
        @option background String Background color. This is a CSS RGB color specification. For example "FF0000" for red.
        @option click (Boolean|Uri|String) The control is click by the user. If set to true, the rest of the 
            options specifies the URI to invoke. Otherwise click can be set to a URI to invoke.
        @option color String Foreground color. This is a CSS RGB color specification. For example "FF0000" for red.
        @option confirm String Message to prompt the user to requeset confirmation before submitting a form or request.
        @option data-* All other data-* names are passed through to the HTML unmodified.
        @option domid String Client-side DOM-ID to use for the control
        @option escape Boolean Escape the text before rendering. This converts HTML reserved tags and delimiters into
            an encoded form.
        @option field String Client DOM name to use for the generated HTML element.
        @option height (Number|String) Height of the table. Can be a number of pixels or a percentage string. 
            Defaults to unlimited.
        @option id Number Numeric database ID for the record that originated the data for the view element.
        @option method String HTTP method to invoke. May be: GET, POST, PUT or DELETE.
        @option modal String Make a form a modal dialog.
        @option period Number Period in milliseconds to invoke the $refresh URI to update the control data. If period
            is zero (or undefined), then refresh will be done using a perisistent connection.
        @option rel String HTML rel attribute. Can be used to generate "rel=nofollow" on links.
        @option remote (String|Boolean) Perform the request in the background without changing the browser location.
            The option may be set to the URI to invoke or it may be set to true if an implicit URI is defined by the control.
        @option refresh (String|URI|Object) URI to invoke in the background to refresh the control data every $period.
            milliseconds. If period is undefined or zero, a persistent connection will be used to refresh data.
        @option size (Number|String) Size of the element.
        @option style String CSS Style to use for the table.
        @option value Object Override value to display if used without a form control record.
        @option visible Boolean Make the control visible. Defaults to true.
        @option width (Number|String) Width of the table or column. Can be a number of pixels or a percentage string. 

        <h4>Dynamic Data</h4>
        Most controls can perform background updates of their data after the initial presentation. This is done via
        the refresh and period options.

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
            } else {
                request = {}
            }
            for (helper in config.web.view.helpers) {
                if (helper.contains("::")) {
                    [mod, klass] = helper.split("::")
                    global.load(mod + ".mod")
                    /*  MOB -- should use 
                        blend(this, global.[mod]::[klass])
                     */
                    blend(this, global[klass])
                } else {
                    blend(this, global[helper])
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

        /** 
            Emit a status alert area
            @param text Initial message text to display. Status messages may be updated by calling the 
                $Controller.status function.
            @param options Optional extra options. See $View for a list of the standard options.
            @option refresh URI to call to to get status message updates.
            @option period Polling period in milliseconds for the client to check the server for status message 
                updates. If this is not specifed, the connection to the server will be kept open. This permits the 
                server to "push" alerts to the console, but will consume a connection at the server for each client.
            @example
                <% status("Status Message", { refresh: "/getData", period: 2000" }) %>
         */
        function alert(text: String, options: Object = {}): Void {
            options = getOptions(options)
            text = formatValue(text, options)
            getConnector("alert", options).alert(text, options)
        }

        /**
            Render a form button. This creates a button suitable for use inside an input form. When the button is clicked,
            the input form will be submitted.
            @param name Name for the input button. This defines the HTML element name and provides the source of the
                initial value to display. The field should be a property of the form control record. It can be a simple 
                property of the record or it can have multiple parts, such as: field.field.field. If this call is used 
                without a form control record, the actual data value should be supplied via the options.value property.
            @param label Text label to display in the button.
            @param options Optional extra options. See $View for a list of the standard options.
            Examples:
                button("commit", "OK")
         */
        function button(name: String, label: String, options: Object = {}): Void {
            options = getOptions(options)
            name = getFieldName(name, options)
            getConnector("button", options).button(name, label, options)
        }

        /**
            Render a link button. This creates a button suitable for use outside an input form. When the button 
            is clicked, the associated URI will be invoked.
            @param text Text to display in the button. The text can contain embedded HTML.
            @param options Options specifying the target URI to invoke. See $View for a list of the standard options.
         */
        function buttonLink(text: String, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("buttonLink", options).buttonLink(text, options)
        }

        /**
            Render a chart. The chart control can display static or dynamic tabular data. The client chart control manages
                sorting by column, dynamic data refreshes, pagination and clicking on rows.
            MOB -- update
            @param data Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data.

            @param data Initial data to display. This should be a data grid (array of objects).
            @param options Object Optional extra options. See also $View for a list of the standard options.
            @option columns Object hash of column entries. Each column entry is in-turn an object hash of options. If unset, 
                all columns are displayed using defaults.
            @option kind String Type of chart. Select from: piechart, table, linechart, annotatedtimeline, guage, map, 
                motionchart, areachart, intensitymap, imageareachart, barchart, imagebarchart, bioheatmap, columnchart, 
                linechart, imagelinechart, imagepiechart, scatterchart (and more)
            @example
                <% chart(grid, { refresh: "/getData", period: 2000" }) %>
                <% chart(data, { onclick: "action" }) %>
         */
        function chart(data: Array, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("chart", options).chart(data, options)
        }

        /**
            Render an input checkbox. This creates a checkbox suitable for use within an input form. 
            @param name Name for the input checkbox. This defines the HTML element name and provides the source of the
                initial value for the checkbox. The field should be a property of the $form control record. It can be a 
                simple property of the record or it can have multiple parts, such as: field.field.field. If this call 
                is used without a form control record, the actual data value should be supplied via the options.value 
                property.
            @param checkedValue Value to use for the field when checked. Defaults to true.
            @param options Optional extra options. See $View for a list of the standard options.
            @option value Object Override value to display if used without a form control record.
         */
        function checkbox(name: String, checkedValue: Object = true, options: Object = {}): Void {
            options = getOptions(options)
            let value = getValue(currentRecord, name, options)
            name = getFieldName(name, options) 
            getConnector("checkbox", options).checkbox(name, value, checkedValue, options)
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
            options = getOptions(options)
            let cflash ||= request.flash
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
            let connector = getConnector("flash", options)
            for (kind in msgs) {
                let msg: String = msgs[kind]
                if (msg && msg != "") {
                    connector.flash(kind, msg, options.clone())
                }
            }
        }

        /**
            Render a form.
            The generated form HTML will include by default a security token definition to guard against CSRF threats.
            This token will automatically be included when the form is submitted and will be validated by the 
            receiving Controller. To disable this functionality, set options.nosecurity to true. This security token
            will be generated once for the view and the same token will be used by all forms on the view page. To use
            security tokens outside a form, you need to manually call $securityToken in the &lt;head> section of the page.


MOB -- much more doc here
    - Talk about controllers updating the record
    - Talk about HTML ids generated from field names
            @param record Record to display and optionally update
            @param options Optional extra options. See $View for a list of the standard options.
            @option hideErrors Don't display model errors. Models retain error diagnostics from a failed write. Setting
                thish option will prevent their display.
            @option method HTTP method to use when submitting the form.
            @option action Action to invoke when the form is submitted. Defaults to "create" or "update" depending on 
                whether the field has been previously saved.
            @option controller Controller to invoke when the form is submitted. Defaults to the current controller.
            @option nosecurity Don't generate a security token for the form.
            @option securityToken String Override CSRF security token to include when the form is submitted. A default 
                security token will always be generated unless options.nosecurity is defined to be true.
            @option uri String Use a complete URI rather than an action and controller option to create the target uri.
         */
        function form(record: Object, options: Object = {}): Void {
            options = getOptions(options)
            if (currentRecord = record) {
                options.id ||= record.id
            }
            options.method ||= "POST"
            options.action ||= "update"
            let connector = getConnector("form", options)
            connector.form(record, options)
        }

        /**
            Render an image
            @param src Source name for the image.
            @param options Optional extra options. See $View for a list of the standard options.
            @examples
                <% image("pic.gif") %>
                <% image("pic.gif", { refresh: "/getData", period: 2000, style: "myStyle" }) %>
                <% image("pic.gif", { click: true, uri: "/foreground/click" }) %>
                <% image("checkout.gif", { click: true, action: "checkout" }) %>
                <% image("pic.gif", { remote: "/background/click" }) %>
         */
        function image(src: String, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("image", options).image(src, options)
        }

        /**
            Render an input field as part of a form. This is a smart input control that will call the appropriate
                input control based on the model field data type.
            @param name Name for the input field. This defines the HTML element name and provides the source 
                of the initial value to display. The field should be a property of the form control record. It can 
                be a simple property of the record or it can have multiple parts, such as: field.field.field. If 
                this call is used without a form control record, the actual data value should be supplied via the 
                options.value property.
            @param options Optional extra options. See $View for a list of the standard options.
            @examples
                <% input("phone") %>
         */
        function input(name: String, options: Object = {}): Void {
            try {
                let datatype
                if (currentRecord && Object.getType(currentRecord).getColumnType) {
                    datatype = Object.getType(currentRecord).getColumnType(name)
                } else {
                    let value = getValue(currentRecord, name, options)
                    datatype = Object.getTypeName(value).toLowerCase()
                }
                //  MOB TODO - needs fleshing out for each type
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
                    text(name, options)
                    break
                case "text":
                    textarea(name, options)
                    break
                case "boolean":
                    checkbox(name, "true", options)
                    break
                default:
                    throw "input control: Unknown field type: " + datatype + " for field " + name
                }
            } catch (e) {
print("CATCH " + e)
                text(name, options)
            }
        }

        /**
            Render a text label field. This renders an output-only text field. Use text() for input fields.
            @param text Label text to display.
            @param options Optional extra options. See $View for a list of the standard options.
            @examples
                <% label("Hello World") %>
                <% label("Hello", { refresh: "/getData", period: 2000, style: "myStyle" }) %>
                <% label("Hello", { click: "/foreground/link" }) %>
                <% label("Checkout", { click: true, action: "checkout" }) %>
         */
        function label(text: String, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("label", options).label(text, options)
        }

/*  MOB - deprecate link vs label
 */
        /** 
MOB - doc not right
            Emit a text link to an action. The URI is constructed from the given action and the current controller. 
            The controller may be overridden by setting the controller option.
            @param text Link text to display
            @param options Optional extra options. See $View for a list of the standard options.
            @option controller String Name of the target controller for the given action
            @option uri String Use a URI rather than action and controller for the target uri.
         */
        function link(text: String, options: Object = {}): Void {
            options = getOptions(options)
            options.action ||= text.split(" ")[0].toLowerCase()
            getConnector("link", options).link(text, options)
        }

        /**
            Emit a selection list. 
            @param field Field to provide the default value for the list. The field should be a property of the form control 
                record. The field can be a simple property of the record or it can have multiple parts, 
                i.e. field.field.field. The field name is used to create the HTML input control name.
                If this call is used without a form control record, the actual data value should be supplied via the 
                options.value property.
            @param choices Choices to select from. This can be an array list where each element is displayed and the value 
                returned is an element index (origin zero). It can also be an array of key/value array tuples where the 
                first entry is the value to display and the second is the value to send to the app. Or it can be an 
                array of objects such as those returned from a table lookup. Lastly, it can be an object where the
                property key is the value to display and the property value is the value to send to the app.
            @param options control options
            @example
                list("stockId", Stock.stockList) 
                list("priority", ["low", "med", "high"])
                list("priority", [["low", 0], ["med", 0.5], ["high", 1]])
                list("priority", [{low: 3}, {med: 5}, {high: 9}])
                list("priority", {low: 0, med: 1, high: 2})
         */
        function list(name: String, choices: Object, options: Object = {}): Void {
            options = getOptions(options)
            let value = getValue(currentRecord, name, options)
            name = getFieldName(name, options) 
            getConnector("list", options).list(name, choices, value, options)
        }

        /**
            Emit a mail link
            @param name Recipient name to display
            @param address Mail recipient address link
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function mail(name: String, address: String, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("mail", options).mail(name, address, options)
        }

//  MOB -- redo progress using a commet style
        /** 
            Emit a progress bar.
            @param data Optional initial data for the control. The data option may be used with the refresh option 
                to dynamically refresh the data. Progress data is a simple Number in the range 0 to 100 and represents 
                a percentage completion value.
            @param options Optional extra options. See $View for a list of the standard options.
            @example
                <% progress(percent, { refresh: "/getData", period: 2000" }) %>
         */
        function progress(percent: Number, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("progress", options).progress(percent, options)
        }

        /** 
            Render a radio button. This creates a radio button suitable for use within an input form. 
            @param name Name for the input radio button. This defines the HTML element name and provides the source 
                of the initial value to display. The field should be a property of the form control record. It can 
                be a simple property of the record or it can have multiple parts, such as: field.field.field. If 
                this call is used without a form control record, the actual data value should be supplied via the 
                options.value property.
            @param choices Choices to select from. This can be an array list where each element is displayed and the value 
                returned is an element index (origin zero). It can also be an array of key/value array tuples where the 
                first entry is the value to display and the second is the value to send to the app. Or it can be an 
                array of objects such as those returned from a table lookup. Lastly, it can be an object where the
                property key is the value to display and the property value is the value to send to the app.
            @param options Optional extra options. See $View for a list of the standard options.
            @option value Object Override value to display if used without a form control record.
            @example
                radio("priority", ["low", "med", "high"])
                radio("priority", [["low", 0], ["med", 0.5], ["high", 1]])
                radio("priority", [{low: 3}, {med: 5}, {high: 9}])
                radio("priority", {low: 0, med: 1, high: 2})
                radio("priority", Message.priorities)
         */
        function radio(name: String, choices: Object, options: Object = {}): Void {
            options = getOptions(options)
            let value = getValue(currentRecord, name, options)
            name = getFieldName(name, options) 
            getConnector("radio", options).radio(name, choices, value, options)
        }

        /** 
            Emit a script link.
            @param uri URI for the script to load. Call with no arguments or uri set to null to get a 
                default set of scripts.
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function script(uri: Object, options: Object = {}): Void {
            let connector = getConnector("script", options)
            if (uri is Array) {
                for each (u in uri) {
                    connector.script(request.home.join(u), options)
                }
            } else if (uri == null) {
                connector.script(null, options)
            } else {
                connector.script(request.home.join(uri), options)
            }
        }

        /**
            Generate a security token for the page and emit a &lt;meta HTML element for the security token.
            Security tokens are used to help guard against CSRF threats.
            This token will automatically be included whenever forms are submitted and the token be validated by the 
            receiving Controller. Forms will normally automatically generate the security token and that explicitly
            calling this routine is not required unless a security token is required for non-form requests such as AJAX
            requests. The $securityToken control should be called inside the &lt;head section of the view page.
            @param options Optional extra options. See $View for a list of the standard options.
            @example
                &lt;head>
                    <% securityToken() %>
                &lt;/head>
        */
        function securityToken(options: Object = {}): Void {
            options = getOptions(options)
            getConnector("securityToken", options).securityToken(options)
        }

        /** 
            Emit a style sheet link.
            @param uri Stylesheet uri or array of stylesheets. Call with no arguments or uri set to null to get a 
                default set of stylesheets.
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function stylesheet(uri: Object, options: Object = {}): Void {
            let connector = getConnector("stylesheet", options)
            if (uri is Array) {
                for each (u in uri) {
                    connector.stylesheet(request.home.join(u), options)
                }
            } else if (uri == null) {
                connector.stylesheet(null, options)
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
            Render a table. The table control can display static or dynamic tabular data. The client table control 
                manages sorting by column, dynamic data refreshes, pagination and clicking on rows or cells.
            @param data Initial data to display. The data must be a grid of data, ie. an Array of 
                objects where each object represents the data for a row. The column names are the object property names 
                and the cell text is the object property values.
            @param options Optional extra options. See $View for a list of the standard options.
            @option columns (Array|Object) The columsn list can be either an array of column names or an object hash 
                of column objects where each column entry is hash of column options. 
                Column options: align, formatter, header, sort, sortOrder, style, width.
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
            @option style String CSS class to use for the table.
            @option styleCells 2D Array of styles to use for the table body cells.
            @option styleColumns Array of styles to use for the table body columns. Can also use the style option in the
                columns option.
            @option styleRows Array of styles to use for the table body rows
            @option title String Table title.

//  MOB
@option key Number Key column index. The key is added to click requests as a "key=value" POST or query field.
@option method String Http method to use for click events

//  MOB -- should auto-align currency to the right

            Column options:
            <ul>
                <li>align</li>
                <li>formatter</li>
                <li>header</li>
                <li>sort String Define the column to sort by and the sort order</li>
                    Defaults to ascending.</li>
                <li>sortOrder String Set to "ascending" or "descending".  Defaults to ascending.</li>
                <li>style</li>
                <li>width</li>
            </ul>
        
            @example
                <% table(gridData, { refresh: "/getData", period: 1000, pivot: true" }) %>
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
            options = getOptions(options)
            //  MOB - move to client side (data-pivot). No good here as it can't be refreshed!
            if (options.pivot) {
                data = pivot(data)
            }
            getConnector("table", options).table(data, options)
        }

        /**
            Render a tab control. 
            The tab control can manage a set of panes and selectively show and hide or invoke the selected panes. 
            If the click option is defined, the selected pane will be invoked via a foreground click. If the
            remote option is defined, the selected pane will be invoked via a background click. Otherwise the 
            selected pane will be made visible and other panes will be hidden.
            @param data Initial data for the control. Tab data can be an array of objects -- one per tab. It can also
                be a single object where the tab text is the property key and the property value is the target.
            @param options Optional extra options. See $View for a list of the standard options.
            @param click Invoke the target URI in the foreground when clicked.
            @param remote Invoke the target URI in the background when clicked.
            @example
                tabs({Status: "pane-1", "Edit: "pane-2"})
                tabs([{Status: "/url-1"}, {"Edit: "/url-2"}], { click: true})
         */
        function tabs(data: Object, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("tabs", options).tabs(data, options)
        }

        /**
            Render a text input field as part of a form.
            @param name Name for the input text field. This defines the HTML element name and provides the source 
                of the initial value to display. The field should be a property of the form control record. It can 
                be a simple property of the record or it can have multiple parts, such as: field.field.field. If 
                this call is used without a form control record, the actual data value should be supplied via the 
                options.value property.
            @param options Optional extra options. See $View for a list of the standard options.
            @option escape Boolean Escape the text before rendering. This converts HTML reserved tags and delimiters into
                an encoded form.
            @option password Boolean The data to display is a password and should be obfuscated.
            @option value Object Override value to display if used without a form control record.
            @examples
                <% text("name") %>
                <% text("product.name") %>
                <% text("address", { escape: true }) %>
                <% text("password", {value: params.password, password: true}) %>
         */
        function text(name: String, options: Object = {}): Void {
            options = getOptions(options)
            let value = getValue(currentRecord, name, options)
            value = formatValue(value, options)
            name = getFieldName(name, options) 
            getConnector("text", options).text(name, value, options)
        }


        /**
            Render a text area
            @param name Name for the input textarea field. This defines the HTML element name and provides the source 
                of the initial value to display. The field should be a property of the form control record. It can 
                be a simple property of the record or it can have multiple parts, such as: field.field.field. If 
                this call is used without a form control record, the actual data value should be supplied via the 
                options.value property.
            @option Boolean escape Escape the text before rendering. This converts HTML reserved tags and delimiters into
                an encoded form.
            @param options Optional extra options. See $View for a list of the standard options.
            @option numCols Number number of text columns
            @option numRows Number number of text rows
            @option value Object Override value to display if used without a form control record.
            @examples
                <% textarea("name") %>
         */
        function textarea(name: String, options: Object = {}): Void {
            options = getOptions(options)
            let value = getValue(currentRecord, name, options)
            value = formatValue(value, options)
            name = getFieldName(name, options) 
            getConnector("textarea", options).textarea(name, value, options)
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
            options = getOptions(options)
            getConnector("tree", options).tree(data, options)
        }

        // MOB TODO - need a rich text editor. Wiki style.  wiki()
        // MOB TODO - need markdown style output?

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

        /**
            @duplicate Request.writeSafe
         */
        function writeSafe(...data): Number
            request.writeSafe(...data)

        /*********************************************** Support ****************************************************/
        /**
            Get the data value for presentation.
            @param record Object containing the data to present. This could be a plain-old-object or it could be a model.
         */
        function formatValue(value: Object, options: Object): String {
            if (value == undefined) {
                value = ""
            }
            let formatter = options.formatter || formatters[typeOf(value)] || plainFormatter
            let result = formatter(this, value, options)
            if (options.escape) {
                result = escapeHtml(result)
            }
            return result
        }

        private function getConnector(kind: String, options: Object = {}) {
            let vc = config.web.view
            let connectorName = options.connector || vc.connectors[kind] || vc.connectors["rest"]
            let name = (connectorName + "ViewConnector").toPascal()
            if (connectors[name]) {
                return connectors[name]
            }
            try {
                return connectors[name] = new global[name](this)
            } catch (e: Error) {
                throw new Error("Undefined view connector: " + name)
            }
        }

        /*
            Get a field name for an input control. This will use the Record type name if not a plain-old-object.
            If options.field is defined, it is used in preference of the given field.
         */
        private function getFieldName(field: Object, options: Object): String {
            if (options.field) {
                return options.field
            }
            if (currentRecord) {
                if (Object.getType(currentRecord) != Object) {
                    return typeOf(currentRecord).toCamel() + '.' + field
                }
            }
            return field
        }

        private function getOptions(options: Object): Object {
            if (options is String) {
                options = {action: options}
            } else if (options is Uri) {
                options = {uri: options.toString() }
            }
            return options
        }

        /*
            Get the data value. Data may be:
                options.value       Overrides all
                data                if a record is not defined
                fieldname           data is a record[data] if a simple string
                { field: name }
         */
        function getValue(record: Object, data: Object, options: Object): Object {
            let value
            if (options.value !== undefined) {
                value = options.value
            } else if (record) {
                let field = data.field || data
                value = record[field]
                if (value == undefined) {
                    /* Support field.field.field ... */
                    let parts = field 
                    value = record
                    for each (field in parts.split(".")) {
                        value = value[field]
                    }
                }
                if (record.hasError && record.hasError(field)) {
                    options.hasError = true
                }
            } else {
                value = data
            }
            return value
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

        /************************************************ View Renderers **************************************************/

        var formatters = {
            Date: dateFormatter,
            String: plainFormatter,
            Number: plainFormatter,
            Boolean: plainFormatter,
            //  MOB -- put all standard types here -- faster
        }

        private static function dateFormatter(view: View, value: Object, options: Object): String
            new Date(value).format(view.formats.Date)

        private static function plainFormatter(view: View, value: Object, options: Object): String
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
