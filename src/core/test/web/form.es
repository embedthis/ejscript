require ejs.web

exports.app = function (request: Request) {
    View(request).render(function(request: Request) {

        write("<html>
<head><title>Form Test</title></head>
<body>
");dump(request); 
        write("
</body>
</html>
");
})
}
