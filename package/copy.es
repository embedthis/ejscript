/*
    Install files support routines
 */

var masterOptions

const StandardFilter = /\.makedep$|\.o$|\.pdb$|\.tmp$|\.save$|\.sav$|OLD|\/Archive\/|\/sav\/|\/save\/|oldFiles|\.libs\/|\.nc|\.orig|\.svn|\.git|^\.|\.swp$|\.new$|\.nc$|.DS_Store/

/*
    copy files
    @param from Source path
    @param target Destination path
    @param options Copy options
    @option exclude Exclude files that match the pattern
    option include Include files that match the pattern
    option fold Fold long lines on windows and convert new line endings
    option zip Compress target file
    option owner Set file owner
    option group Set file group
    option perms Set file perms
    option strip Strip object or executable
    option expand Expand variables using %%TOKENS%%
 */
public function copy(src: Path, target: Path = Dir, options = {}) 
{
    blend(options, masterOptions)
    let copied = 0
    let group = options.group || -1
    let owner = options.owner || -1
    let permissions = options.permissions
    let task = options.task 
    let verbose = options.trace
    let zip = options.zip || false
    let build = options.build
    let log = App.log

    if (verbose > 0) log.activity("Process", "cpy: " + src + " " + target)

    let pat = src.basename
    let dir, from
    if (options.from) {
        from = Path(options.from).relative
        dir = from.join(src.dirname)
    } else {
        dir = src.dirname
        from = ""
    }
    let files = options.top.join(dir).find(pat, options.recurse)

    for each (let file: Path in files) {
        file = file.relative
        if (file.isDir) {
            continue
        }
        if (options.exclude && file.match(options.exclude)) {
            continue
        }
        if (file.match(StandardFilter)) {
            continue
        }
        if (options.include && !file.match(options.include)) {
            continue
        }
        if (file.startsWith(".")) {
            continue
        }
        let dest: Path
        if (target.isDir) {
/*
print("FROM " + from)
print("DIR  " + dir)
print("FILE " + file)
print("TARG " + target)
*/
            dest = Path("" + target + "/" + file.trimStart(from)).normalize
        } else {
            dest = target
        }
        copied++
        if (task == "Remove") {
            if (zip) {
                dest = dest.joinExt("gz")
            }
            if (verbose) log.activity("Remove", dest)
            dest.remove()
        } else {
            let parent = dest.parent
            if (!dest.parent.isDir) {
                if (verbose) log.activity("MakeDir", parent)
                parent.makeDir({ group: group, owner: owner, permissions: 0755})
            }
            if (verbose) log.activity("Copy", dest.relative)
            permissions ||= file.extension.match(/exe|lib|so|dylib|sh|ksh/) ? 0755 : 0644
            file.copy(dest, { permissions: permissions, owner: owner, group: group})

            if (options.expand) {
                if (verbose) log.activity("Patch", dest)
                expand(dest, options)
            }
            if (options.fold && Config.OS == "WIN") {
                fold(dest, options)
            }
            if (options.strip && build.BLD_STRIP != "" && build.BLD_UNIX_LIKE == 1 && build.BLD_BUILD_OS != "MACOSX") {
                Cmd.sh(build.BLD_STRIP + " " + dest)
                Cmd.sh(". " + App.dir.parent.join("inc/buildConfig.sh") + " ; " + build.BLD_STRIP + " " + dest)
            }
            if (zip) {
                dest.joinExt(".gz").remove
                log.activity("Compress", dest)
                Cmd.sh("gzip --best " + dest)
            }
            if (App.uid == 0 && dest.extension == "so" && Config.OS == "LINUX" && options.task == "Install") {
                log.activity("Ldconfig", dest)
                Cmd.sh("ldconfig " + dest)
            }
        }
    }
    if (options.task != "Remove" && copied == 0) {
        log.error("No files copied for " + src)
    }
}


/*
    Read out/inc/buildConfig.h and set in options.build
 */
public function readBuildConfig(options)
{
    options.build = {}
    data = options.top.join("out/inc/buildConfig.h").readLines()
    for each (line in data) {
        if (line.startsWith("#") || line.trim() == "") continue
        if (line.contains("EXPORT_OBJECTS")) {
            break
        }
        let [key, value] = line.split("=")
        options.build[key] = value
    }
}


/*
    Prepare installation component prefixes
 */
public function preparePrefixes(options)
{
    let build = options.build
    build.BLD_TOP = options.top

    build.BUILD_BIN_DIR = Path(build.BLD_TOP).join("bin")
    build.BUILD_LIB_DIR = Path(build.BLD_TOP).join("lib")

    for each (prefix in ["BLD_PREFIX", "BLD_ROOT_PREFIX", "BLD_BIN_PREFIX", "BLD_CFG_PREFIX", "BLD_DOC_PREFIX", "BLD_JEM_PREFIX", 
            "BLD_INC_PREFIX", "BLD_LIB_PREFIX", "BLD_LOG_PREFIX", "BLD_MAN_PREFIX", "BLD_PRD_PREFIX", "BLD_SAM_PREFIX", 
            "BLD_SRC_PREFIX", "BLD_VER_PREFIX", "BLD_WEB_PREFIX"]) {
        build["ORIG_" + prefix] = build[prefix] = Path(build[prefix]).absolute
        let tree: Path
        if (options.task == "Package") {
            if (prefix == "SRC_PREFIX") {
                tree = "/SRC"
            } else if (prefix == "INC_PREFIX" || prefix == "DOC_PREFIX" || prefix == "SAM_PREFIX" || prefix == "MAN_PREFIX") {
                tree = "/DEV"
                throw "Dev packaging not supported"
            } else {
                tree = "/BIN"
            }
        }
        build[prefix] = Path("" + options.root + tree + build[prefix])
    }
    let out = options.top.join("out")
    for (key in build) {
        let value = build[key].toString()
        if (value.contains("$")) {
            value = value.replace(/\$\{BLD_TOP}/g, options.top)
            value = value.replace(/\$\{BLD_OUT_DIR}/g, out)
            build[key] = value
        }
        if (key.contains("DIR")) {
            build[key] = Path(build[key])
        }
    }
    build.ABS_BLD_TOP = Path(build.BLD_TOP).absolute
    build.ABS_BLD_OUT_DIR = Path(build.BLD_OUT_DIR).absolute
    build.ABS_BLD_BIN_DIR = Path(build.BLD_BIN_DIR).absolute
    build.ABS_BLD_TOOLS_DIR = Path(build.BLD_TOOLS_DIR).absolute
}

/*
    Fold long lines. On windows, will also convert line terminatations to <CR><LF>
 */
public function fold(path: Path, options)
{
    let data = path.readLines()
    for each (line in data) {
        if (options.fold) {
            if (line.length > 80) {
                for (i = 79; i >= 0; i--) {
                    if (line[i] == ' ') {
                        line[i] = '\n'
                        break
                    }
                }
            }
        }
    }
    path.write(data)
}


/*
 */
public function expand(path: Path, options)
{
    let data = path.readString()
    for each (token in options.build) {
        data.replace(RegExp("!!" + token + "!!", "g"), options.build[token])
    }
    path.write(data)
}


public function copySetup(options = null) 
{
    options ||= {}
    options.root ||= Path("/")
    options.task ||= "Install"
    options.top ||=  App.dir.findAbove("configure").dirname
    options.trace ||= App.getenv("TRACE")
    masterOptions = options

    options.root.makeDir()
    readBuildConfig(options)
    preparePrefixes(options)
    return options
}

