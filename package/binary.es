/*
  	binary.files - Files needed for the binary installation 

 	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.

    script Task Root Bare
 */

var top = App.dir.findAbove("configure").dirname

load(top.join("package/copy.es"))
var bare: Boolean = App.args[3] == "1"
var options = copySetup({task: App.args[1], root: Path(App.args[2])})
var build = options.build
var os = build.BLD_HOST_OS
var product = build.BLD_PRODUCT
var debug: Boolean = build.BLD_DEBUG
var strip = !debug

/*
    Sources
 */
var sout: Path = build.BLD_OUT_DIR
var sbin: Path = build.BLD_BIN_DIR
var sinc: Path = build.BLD_INC_DIR
var slib: Path = build.BLD_LIB_DIR
var sjem: Path = build.BLD_JEM_DIR

/*
    Destinations
 */
var bin: Path = build.BLD_BIN_PREFIX
var lib: Path = build.BLD_LIB_PREFIX
var inc: Path = build.BLD_INC_PREFIX
var ver: Path = build.BLD_VER_PREFIX
var jem: Path = build.BLD_JEM_PREFIX
var man: Path = build.BLD_MAN_PREFIX

dperms = {permissions: 0755, uid: 0, gid: 0}
bin.makeDir(dperms)
lib.makeDir(dperms)
ver.makeDir(dperms)
jem.makeDir(dperms)
if (!bare) {
    man.join("man1").makeDir(dperms)
    lib.join("www").makeDir(dperms)
}

var saveLink 
if (options.task == "Remove" && bin.join("linkup").exists) {
    saveLink = Path(".").temp()
    bin.join("linkup").copy(saveLink)
    saveLink.attributes = {permissions: 0755}
}

if (!bare) {
    copy("LICENSE.TXT", ver, { from: "doc/licenses", fold: true, expand: true })
    copy("*.TXT", ver, { from: "doc/product", fold: true, expand: true })
    copy("uninstall.sh", bin.join("uninstall"), {from: "package", permissions: 0755, expand: true})
    copy("linkup", bin.join("linkup"), {from: "package", permissions: 0755, expand: true})

    copy("www/**/*", lib, {
        from: slib, 
        exclude: /treeview/,
        permissions: 0644, 
    })
    copy("bits/**", lib, {
        from: slib, 
        permissions: 0644, 
        hidden: true,
    })

    let cmdFilter = (Config.OS == "WIN") ? /undefined/ : /\.cmd/
    copy("*", bin, {
        from: sbin,
        include: /ejs|mvc|ejspage|jem|http|manager|sqlite/
        exclude: cmdFilter,
        permissions: 0755,
    })
    //  if (jem.exists) {
    //      copy("**/*", jem, {from: sjem})
    //  }
}

copy("ejs*", bin, {from: sbin, permissions: 0755, strip: strip, exclude: /ejspage/})
copy("*.mod", lib, {from: slib})

/*
    Copy libraries and symlink to sonames
 */
copy("*" + build.BLD_SHOBJ, lib, {from: slib, permissions: 0755, strip: strip})
if (options.task != "Remove" && build.BLD_FEATURE_SSL == 1 && os == "LINUX") {
    copy("*" + build.BLD_SHOBJ + "*", lib, {from: slib, permissions: 0755, strip: strip})
    for each (f in slib.glob("*.so*")) {
        let withver = f.basename
        let nover = withver.name.replace(/\.[0-9]*.*/, ".so")
        Cmd.sh("rm -f " + lib.join(nover))
        Cmd.sh("ln -s " + withver + " " + lib.join(nover))
    }
}

if (build.BLD_UNIX_LIKE == 1) {
    copy("*.1", man.join("man1"), {from: "doc/man", compress: true })
}

if (build.BLD_HOST_OS == "WIN") {
    if (build.BLD_CC_CL_VERSION == 16) {
        copy("msvcrt.lib", bin, {from: build.BLD_VS})
        copy("msvcr100.dll", bin, {from: build.BLD_VS.parent.join("redist/x86/Microsoft.VC100.CRT")})
    }
    copy(sbin.join("removeFiles*"), bin)
}

if (options.task == "Install") {
    if (!bare && build.BLD_HOST_OS != "WIN") {
        Cmd.sh([bin.join("linkup"), options.task, options.root])
    }
} else if (saveLink && saveLink.exists) {
    if (build.BLD_HOST_OS != "WIN") {
        Cmd.sh([saveLink, options.task, options.root])
    }
    saveLink.remove()
}
