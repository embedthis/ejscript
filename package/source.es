/*
    Source files
 */

var top = App.dir.findAbove("configure").dirname
load(top.join("package/copy.es"))
var bare: Boolean = App.args[3] == "1"
var options = copySetup({task: App.args[1], root: Path(App.args[2])})
var build = options.build
var src: Path = build.BLD_SRC_PREFIX
src.makeDir()

copy("Makefile", src)
copy("LICENSE.TXT", src, {from: "doc/licenses", fold: true, expand: true})
copy("*.TXT", src, {from: "doc/product", fold: true, expand: true})
copy("configure", src, {permissions: 0755})

copy("build/*", src, {
    recurse: true,
    include: /Makefile|Makefile.top|configure\.|\.defaults|config\.|make\.|\.config/,
})

copy("build/bin/*", src, {
    permissions: 0755,
    include: /^getlib$|^makedep$|^priv$|^fakePriv$|utest*|bld$|make.*Package.*|makeInstall|combo|makedep|cacheConfig/,
})

copy("build/src/*", src, {
    include: /^Makefile$|\.c$|\.h$/,
})

copy("build/components/*", src)

copy("package/*", src, {
    include: /Makefile|\.files/,
})

copy("package/*", src, {
    include: /\.sh|makeInstall\.|linkup/
    permissions: 0755,
})

copy("package/*", src, {
    recurse: true,
    include: /LINUX|MACOSX|WIN/
})

copy("src/*", src, {
    recurse: true,
    include: /^src\/Makefile|^src\/cmd|^src\/compiler|^src\/core|^src\/slots|^src\/deps|^src\/vm|^src\/jems|^src\/utils|^src\/samples|\.h$/,
    exclude: /\.log$|fcgi|ejs.debugger|ejs.cgi|\.lst$|ejs.zip|\.stackdump$/
})

copy("doc/*", src, {
    recurse: true,
    exclude: /api\/ejs\/xml\/|api\/ejs\/html\/|Archive|\.mod$|\.so$|\.dylib$|\.o$/
})

copy("projects/*", src, {
    exclude: /\/Debug|\/Release|\.ncb|\.mode1v3|\.pbxuser/,
})
