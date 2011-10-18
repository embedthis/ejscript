/*
  	binary.files - Files needed for the binary installation 

 	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.

    script Task Root Bare
 */

var top = App.dir.findAbove("configure").dirname
load(top.join("package/copy.es"))
var bare: Boolean = App.args[3] == "1"
var options = copySetup({task: App.args[1], root: Path(App.args[2])})
var build = options.build

/*
    Sources
 */
var sout: Path = build.BLD_OUT_DIR
var sbin: Path = build.BLD_BIN_DIR
var slib: Path = build.BLD_LIB_DIR
var sjem: Path = build.BLD_JEM_DIR

/*
    Destinations
 */
var bin: Path = build.BLD_BIN_PREFIX
var lib: Path = build.BLD_LIB_PREFIX
var ver: Path = build.BLD_VER_PREFIX
var jem: Path = build.BLD_JEM_PREFIX

bin.makeDir()
lib.makeDir()
lib.join("www").makeDir()
ver.makeDir()
jem.makeDir()

var saveLink 
if (options.task == "Remove" && bin.join("linkup").exists) {
    saveLink = Path.temp()
    bin.join("linkup").copy(saveLink)
}

copy("ejs*", bin, {from: sbin, permissions: 0755, strip: true})

if (!bare) {
    copy("LICENSE.TXT", ver, { from: "doc/licenses", fold: true, expand: true })
    copy("*.TXT", ver, { from: "doc/product", fold: true, expand: true })
    copy("uninstall.sh", bin.join("uninstall"), {from: "package", permissions: 0755, expand: true})
    copy("linkup", bin.join("linkup"), {from: "package", permissions: 0755, expand: true})
    copy("www/*", lib, {from: slib, permissions: 0644, recurse: true})

    let cmdFilter = (Config.OS == "WIN") ? /undefined/ : /\.cmd/
    copy("*", bin, {
        from: sbin,
        include: /ejs*|mvc*|ejspage*|jem*|http*|sqlite*|angel**/
        exclude: cmdFilter,
        permissions: 0755,
    })
    if (jem.exists) {
        copy("*", jem, {from: sjem, recurse: true})
    }
}

copy("*.mod", lib, {from: slib})
copy("*" + build.BLD_SHOBJ, lib, {from: slib, permissions: 0755, strip: true})

if (build.BLD_HOST_OS == "WIN") {
    if (build.BLD_CC_CL_VERSION == 16) {
        copy("msvcrt.lib", bin, {from: build.BLD_VS})
        copy("msvcr100.dll", bin, {from: build.BLD_VS.parent.join("redist/x86/Microsoft.VC100.CRT")})
    }
    copy(sbin.join("removeFiles*"), bin)
}

if (options.task == "Install") {
    if (!bare) {
        Cmd.sh(bin.join("linkup") + " " + options.task + " " + options.root)
    }
} else if (saveLink && saveLink.exists) {
    Cmd.sh(saveLink + " " + options.task + " " + options.root)
    saveLink.remove()
}
