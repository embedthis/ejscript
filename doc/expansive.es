Expansive.load({
    meta: {
        title:       'Embedthis Ejscript Documentation',
        url:         'https://embedthis.com/ejs/doc/',
        description: 'Javacript Language Environment',
    },
    control: {
        copy: [ 'images' ],
        dependencies: { 'css/all.css.less': '**.less' },
        documents: [ '**', '!**.less', '**.css.less' ],
        plugins: [ 'less' ],
    }
})
