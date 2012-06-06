require ejs.web

HttpServer.create(App.config.uris.http, {documents: "web"})
