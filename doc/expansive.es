Expansive.load({
    meta: {
        title:       'Embedthis Ejscript Documentation',
        url:         'https://embedthis.com/ejs/doc/',
        description: 'Javacript Language Environment',
        keywords:    'Ejscript, Javascript, Ecmascript, devices, http server, internet of things',
    },
    expansive: {
        copy: [ 'images' ],
        dependencies: { 'css/all.css.less': 'css/*.inc.less' },
        documents: [ '**', '!css/*.inc.less' ],
        plugins: [ 'less' ],
    }
})
