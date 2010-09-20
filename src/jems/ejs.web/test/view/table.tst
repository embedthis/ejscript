/*
    table()
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

router = new Router(Router.Top)
server = new HttpServer
server.listen(HTTP)
load("proxy.es")

public var proxyData 

server.on("readable", function (event, request: Request) {
    try {
        router.route(request)
        switch (pathInfo) {
        case "/table":
            let view = new View(this)
            view.table.apply(view, proxyData)
            close()
            break

        default:
            writeError(Http.ServerError, "Bad test URI")
        }
    } catch (e) {
        writeError(Http.ServerError, e)
    }
})

//  Alert with refresh

let data = [
    {id: 1, color: "red", temp: 99.9},
    {id: 2, color: "blue", temp: 12.3}
]


//  Basic grid
proxy("table", data, [
    '<table class="-ejs-table">',
    '<thead>',
        '<tr>',
            '<th>Color</th>',
            '<th>Temp</th>',
        '</tr>',
    '</thead>',
    '<tbody>',
        '<tr>',
            '<td>red</td>',
            '<td align="right">99.9</td>',
        '</tr>',
        '<tr>',
            '<td>blue</td>',
            '<td align="right">12.3</td>',
        '</tr>',
    '</tbody>',
    '</table>',
])


//  Without title
let http = proxy("table", data, {
        showHeader: false,
    }, []
)
assert(!http.response.contains("Color"))


//  With ID
proxy("table", data, {
        showId: true,
    }, [
        '<td align="right">1</td>',
        '<td align="right">12.3</td>',
    ]
)


//  With Title
proxy("table", data, {
        title: "Color Table",
    }, [
        '<tr><td colspan="2">Color Table</td></tr>',
    ]
)


//  Sorted - note: sorting is done client side
proxy("table", data, {
    sort: "color",
    sortOrder: "descending"
}, [
    '<table data-sort="color" data-sort-order="descending" class="-ejs-table">',
    '<thead>',
        '<tr>',
            '<th>Color</th>',
            '<th>Temp</th>',
        '</tr>',
    '</thead>',
    '<tbody>',
        '<tr>',
            '<td>red</td>',
            '<td align="right">99.9</td>',
        '</tr>',
        '<tr>',
            '<td>blue</td>',
            '<td align="right">12.3</td>',
        '</tr>',
    '</tbody>',
    '</table>',
])


//  Pivot (NOTE: this will be done client side too eventually)
proxy("table", data, {
    pivot: "true",
}, [
    '<table data-pivot="true" class="-ejs-table">',
        '<thead>',
            '<tr>',
                '<th>0</th>',
                '<th>1</th>',
                '<th>2</th>',
            '</tr>',
        '</thead>',
        '<tbody>',
            '<tr>',
                '<td>id</td>',
                '<td align="right">1</td>',
                '<td align="right">2</td>',
            '</tr>',
            '<tr>',
                '<td>color</td>',
                '<td>red</td>',
                '<td>blue</td>',
            '</tr>',
            '<tr>',
                '<td>temp</td>',
                '<td align="right">99.9</td>',
                '<td align="right">12.3</td>',
            '</tr>',
        '</tbody>',
    '</table>',
])


//  Refresh
proxy("table", data, {
    refresh: "@refresh",
    period: 1000,
}, [
    '<table data-refresh-period="1000" class="-ejs-table" id="id_0" data-refresh="/refresh">',
])


//  Params
proxy("table", data, {
    refresh: "@refresh",
    period: 1000,
    params: {password: "abracadabra", "duration": 2000 },
}, [
    'data-refresh-params="password=abracadabra&duration=2000"',
])


//  Click on row
let id = {id: "[0-9]+"}
router.add('/{id}/edit', {action: "edit", constraints: id})
proxy("table", data, {
    click: "@edit",
}, [
    '<tr data-click="/1/edit">',
    '<tr data-click="/2/edit">',
])


//  Click on cell
router.add('/{id}/{color}/celledit', {action: "celledit", constraints: id})

//  Cell edit
proxy("table", data, {
    columns: {
        "color": {
            click: "@celledit",
            key: ["color", "id"],
        },
        "temp": {},
    },
}, [
    '<td data-click="/1/red/celledit">red</td>',
    '<td data-click="/2/blue/celledit">blue</td>',
])



//  Table style
proxy("table", data, {
    style: "bold",
}, [
    '<table class="bold -ejs-table">',
])


//  Row styles
proxy("table", data, {
    styleRows: ["bold-row", "normal-row"],
}, [
    '<tr class="bold-row">',
    '<tr class="normal-row">',
])


//  Cell styling
proxy("table", data, {
    style: "info-table",
    styleRows: ["bold-row", "normal-row"],
    columns: {
        "color": {
            key: ["color", "id"],
            style: "color-style",
        },
        "temp": {},
    },
}, [
    '<tr class="bold-row">',
    '<tr class="normal-row">',
    '<td class="color-style">red</td>',
    '<td class="color-style">blue</td>',
])



//  Column properties
proxy("table", data, {
    columns: {
        "color": {
            align: "center",
            width: "50%",
            header: "Colour",
        },
        "temp": {
            header: "Temperature",
        },
    },
}, [
    '<th width="50%">Colour</th>'
    '<td align="center">red</td>'
])


//  Column formatter
function colorSmart(view, value) {
    return "Very %s".format([value])
}
proxy("table", data, {
    columns: {
        "color": {
            formatter: colorSmart,
        },
        "temp": {
            header: "Temperature",
        },
    },
}, [
    '<td>Very red</td>'
    '<td>Very blue</td>'
])


//  Edit
proxy("table", data, {
    edit: "@edit",
}, [
    '<tr data-edit="/1/edit">',
    '<tr data-edit="/2/edit">',
])


server.close()
