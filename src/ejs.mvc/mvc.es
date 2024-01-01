#!/usr/bin/env ejs
/*
    mvc -- Ejscript web framework generator. This generates, compiles, cleans and packages Ejscript web applications. 
 */

/*
    Key directories, names should come from ejsrc
        view
        layouts
        layouts/default.ejs
 */

require ejs.web
require ejs.template
require ejs.db
require ejs.unix

use default namespace public

class EjsMvc {

    private const DIR_PERMS: Number = 0775
    private const FILE_PERMS: Number = 0666
    private const RC: String = ".ejsrc"

    private var appName: String
    private var buildAll: Boolean
    private var args: Args
    private var database: String = "sqlite"
    private var dirs: Object
    private var config: Object = {}
    private var db: Database
    private var debug: Boolean = false
    private var ext: Object
    private var endpoint: String = ":4000"
    private var layoutPage: String
    private var mode: String = "debug"
    private var mvc: String
    private var nextMigration: Number = 0
    private var options: Object
    private var overwrite: Boolean = false
    private var verbose: Number = 1

    /* This layers over App.defaultConfig */
    private var defaultConfig = {
        dirs: {
            bin: Path("bin"),
            db: Path("db"),
            cache: Path("cache"),
            controllers: Path("controllers"),
            layouts: Path("layouts"),
            migrations: Path("db/migrations"),
            models: Path("models"),
            views: Path("views"),
            src: Path("src"),
            static: Path("static"),
        },
        extensions: {
            es: "es",
            ejs: "ejs",
            mod: "mod",
        },
        mvc: {
            appmod: "App.mod",
            compiler: "ejsc --debug --web",
            start: "start.es",
        },
    }

    function EjsMvc() {
        blend(config, defaultConfig, {overwrite: false})
        loadConfig("ejsrc", true)
        ext = config.extensions

        dirs = config.dirs
        dirs.home = App.dir
        //  TODO -- should these come from ejsrc
        dirs.bin = Config.Bin
        dirs.mod = Config.Bin
        for (d in dirs) {
            dirs[d] = Path(dirs[d])
        }
        //  TODO -- should this come from ejsrc
        layoutPage = dirs.layouts.join("default.ejs")
        mvc = Path(App.args[0]).basename
    }

    private var argsTemplate = {
        options: {
            database: { range: String },
            debug: {},
            diagnose: { alias: 'd'},
            full: {},
            keep: { alias: 'k' },
            layout: { range: String },
            log: { alias: 'l', range: String },
            listen: { range: String },
            min: {},
            overwrite: {},
            quiet: { alias: 'q' },
            reverse: {}
            search: { range: String },
            verbose: { alias: 'v' },
        },
        usage: usage,
    }

    function usage(): Void {
        stderr.writeLine("\nUsage: mvc [options] [commands] ...\n" +
            "  Options:\n" + 
            "    --database [sqlite | mysql]  # Sqlite only currently supported adapter\n" + 
            "    --full                       # Generate all directories\n" + 
            "    --keep                       # Keep intermediate source in cache\n" + 
            "    --layout layoutPage          # Specify a default layout page\n" + 
            '    --log logSpec                # Save errors to a log file\n' +
            "    --listen port                # Port on which to listen for Http\n" + 
            "    --min                        # Generate mininally\n" + 
            "    --reverse                    # Reverse generated migrations\n" + 
            "    --overwrite                  # Overwrite existing files\n" + 
            "    --quiet                      # Don't trace activity to console\n" + 
            "    --search searchPath          # Specify a module search path\n" + 
            "    --verbose                    # Increase trace verbosity\n")

        let pre = "    mvc "
        stderr.writeLine("  Commands:\n" +
            pre + "clean\n" +
            pre + "compile [flat | app | controller_names | view_names]\n" +
            pre + "compile path/name.ejs ...\n" +
            pre + "generate app name\n" + 
            pre + "generate controller name [action [, action] ...]\n" + 
            pre + "generate migration description model [field:type [, field:type]...]\n" +
            pre + "generate model name [field:type [, field:type]...]\n" +
            pre + "generate scaffold model [field:type [, field:type]...\n" +
            pre + "import\n" +
            pre + "migrate [forward|backward|NNN]\n" +
            pre + "run" +
            "")

        /*
            TODO
            pre + "dbconsole \n" +             # sqlite
            pre + "console \n" +               # with ejs.db, ejs.web and app loaded (all models, all controllers)
            pre + "generate package\n" +
            pre + "install (not implemented yet)\n" +
            pre + "uninstall (not implemented yet)\n" +
            pre + "mode [development | test | production] (not implemented yet)\n" +
            pre + "deploy path.zip (not implemented yet)\n" +
        */
        App.exit(1)
    }
    function main() {
        args = Args(argsTemplate)
        try {
            processOptions(args)
            if (args.rest.length == 0) {
                usage()
            }
            process()
        } catch (e) {
            if (options.diagnose) {
                error(e)
            } else if (e is String) {
                msg = e
                error("mvc: Error: " + msg + "\n")
            } else {
                msg = e.message
                error("mvc: Error: " + msg + "\n")
            }
            App.exit(2)
        }
    }

    function error(...args): Void
        App.errorStream.write(args.join(" ") + "\n")

    function processOptions(args: Args) {
        options = args.options
        if (options.search) {
            App.search = options.search.split(App.SearchSeparator)
        }
        if (options.log) {
            App.log.redirect(options.log)
            App.mprLog.redirect(options.log)
        }
        if (options.quiet) {
            options.verbose = 0
            verbose = 0;
        }
        if (options.verbose) {
            verbose += (options.verbose cast Number)
        }
        if (options.listen) {
            endpoint = (options.listen)
        }
        App.search = [dirs.cache] + App.search
    }

    function process() {
        let task = args.rest.shift()
        let rest = args.rest

        switch (task) {
        case "browse":
            checkApp(task, rest)
            browse(rest)
            break

        case "clean":
            clean(rest)
            break

        case "compile":
            checkApp(task, rest)
            compile(rest)
            break

        case "console":
            checkApp(task, rest)
            console(rest)
            break

        case "deploy":
            checkApp(task, rest)
            deploy(rest)
            break

        case "import":
            checkApp(task, rest)
            import(rest)
            break

        case "install":
            checkApp(task, rest)
            install(rest)
            break

        case "generate":
            checkApp(task, rest)
            generate(rest)
            break

        case "migrate":
            checkApp(task, rest)
            migrate(rest)
            break

        case "run":
            checkApp(task, rest)
            run(rest)
            break

        default:
            throw "Unknown command: " + task
            break
        }
    }

    function clean(args: Array): Void {
        let files: Array = find(".", "**." + ext.mod)
        if (files.length > 0) {
            trace("[CLEAN]", verbose > 1 ? files : "module files")
            for each (f in files) {
                rm(f)
            }
        }
    }

    //  TODO - refactor and reorder this. Perhaps move compile, generate, migrate etc into separate files

    function compile(args: Array): Void {
        var files: Array
        if (args.length == 0) {
            args.append("everything")
        }
        let kind: String = args[0].toLowerCase()
        let rest: Array = args.slice(1)

        switch (kind) {
        case "everything":
            /*
                Build all items but NOT as one module
             */
            buildApp()
            for each (name in find(dirs.controllers, "*." + ext.es)) {
                buildController(name)
            }
            files = find(dirs.views, "**." + ext.ejs)
            for each (name in files) {
                buildView(name, true)
            }
            files = find(dirs.static, "**." + ext.ejs)
            for each (name in files) {
                buildWebPage(name, true)
            }
            break

        case "flat":
            /*
                Build entire app flat as one module
             */
            buildAll = true
            let saveVerbose = verbose
            let saveKeep = options.keep
            let pat = "**." + ext.es
            let controllers = find(dirs.controllers, pat)
            for each (c in controllers) {
                rm(dirs.cache.join(c.basename.replaceExt(ext.mod)))
            }
            files = find("config", pat) + find("src", pat) + controllers + find(dirs.models, pat)
            let viewFiles = find(dirs.views, "**." + ext.ejs)
            let esPages = []
            for each (name in viewFiles) {
                if (!name.toString().contains(dirs.layouts.toString() + "/")) {
                    intermediate = buildView(name, false)
                    rm(intermediate.replaceExt(ext.mod))
                    esPages.append(intermediate)
                }
            }
            let webFiles = find(dirs.static, "**." + ext.ejs)
            for each (name in webFiles) {
                intermediate = buildWebPage(name, false)
                rm(intermediate.replaceExt(ext.mod))
                esPages.append(intermediate)
            }

            /*
                Make flat configuration file
             */
            flat = dirs.cache.join("_flat_.es")
            makeFile(flat, "App.config.cache.flat = true", "Flat Configuration")
            esPages += [flat]

            files += esPages
            options.keep = saveKeep
            verbose = saveVerbose
            buildFiles(dirs.cache.join("App").joinExt(ext.mod), files)
            if (!options.keep) {
                for each (name in esPages) {
                    rm(name)
                }
            }
            break

        case "app":
            /*
                Build app portions. This includes src, models and Base Controller
             */
            buildApp()
            break

        case "controller":
        case "controllers":
            /*
             *  Build controllers
             */
            if (rest.length == 0) {
                for each (name in find(dirs.controllers, "**." + ext.es)) {
                    buildController(name)
                }
            } else {
                for each (name in rest) {
                    buildController(dirs.controllers.join(name.toPascal()).joinExt(ext.es))
                }
            }
            break

        case "model":
        case "models":
            throw "WARNING: models are built with the app. Use \"" + mvc + " compile app\""
            break

        case "view":
        case "views":
            if (rest.length == 0) {
                for each (view in find(dirs.views, "**." + ext.ejs)) {
                    buildView(view, true)
                }
            } else {
                for each (view in rest) {
                    buildView(view, true)
                }
            }
            break

        default:
            for each (f in args) {
                compileItem(Path(f).relative.normalize)
            }
        }
    }

    function compileItem(file: Path) {
        if (file.isDir) {
            for each (f in file.normalize.files(true)) {
                if (f.extension == ext.ejs || f.startsWith(dirs.views) || f.startsWith(dirs.controllers)) {
                    compileItem(f)
                }
            }
        } else {
            if (file.startsWith(dirs.views)) {
                buildView(file, true)
            } else if (file.startsWith(dirs.controllers)) {
                buildController(file)
            } else if (file.extension == ext.ejs) {
                buildWebPage(file, true)
            } else {
                throw "Can't compile " + file
            }
        }
    }

    function console(args: Array): Void {
        // cmd = 'ejs --use "' + appName + '"'
        let cmd = "ejs"
        //  TODO - this won't work without stdin
        Cmd.run(cmd)
    }

    function buildController(file: Path) {
        if (!file.exists) {
            throw "Controller : \"" + file + "\" does not exist"   
        }
        if (!file.startsWith(dirs.controllers.toString())) {
            throw "File \"" + file + "\" is not a controller"
        }
        if (file.toString() == dirs.controllers.join("Base.es")) {
            // This is built into App.mod
            return
        }
        let out = dirs.cache.join(file.basename).replaceExt(ext.mod)
        let cmd: String
        let search = App.search.join(App.SearchSeparator)
        cmd = getCompilerPath() + " --out " + out + " --search \"" + search + "\" " + dirs.cache.join(config.mvc.appmod) + 
            " " + file
        trace("[BUILD]", (verbose > 1) ? cmd : file)
        let results = runCommand(cmd)
        if (!out) {
            throw "Compilation failed for " + out + "\n" + results
        }
    }

    function buildView(file: Path, compile: Boolean = false): Path? {
        if (file.toString().startsWith(dirs.layouts + "/")) {
             //  Skip layouts
            return null
        }
        if (!file.startsWith(dirs.views.toString()) || file.extension != ext.ejs) {
            throw "File \"" + file + " \" is not a view. Path should be \"" + dirs.views.join("CONTROLLER/VIEW.ejs") + "\""
        }
        if (!file.exists) {
            throw "Can't find view file: " + file
        }
        let sansExt: Path = file.trimExt()
        let controller: String
        let controllerPrefix: String

        let viewName = sansExt.basename
        controller = getNthSegment(sansExt, 1).toPascal()

        // Ensure the corresponding controller (if there is one) is built first
        let controllerSource: Path = dirs.controllers.join(controller).joinExt(ext.es)
        let controllerMod: Path = dirs.cache.join(controller).joinExt(ext.mod)

        if (controllerSource.exists && !buildAll) {
            if (!controllerMod.exists) {
                buildController(controllerSource)
            }
            controllerPrefix = controller + "_"
        }

        if (verbose > 1) {
            trace("[PARSE]", file)
        }
        let parser: TemplateParser = new TemplateParser
        // results = parser.buildView(controllerPrefix + viewName, file.readString(), { layout: layoutPage })
        // results = parser.build(file.readString(), { layout: layoutPage })
        results = Loader.wrap(file, parser.build(file.readString(), { layout: layoutPage }))
        let intermediate: Path = dirs.cache.join(md5(file)).joinExt(ext.es)
        let out = intermediate.replaceExt(ext.mod)
        try {
            intermediate.write(results)
        } catch (e) {
            throw "Can't write module file: " + intermediate + ". Ensure directory is writable."
        }
        if (compile) {
            let cmd: String = getCompilerPath()
            let search = App.search.join(App.SearchSeparator)
            if (controllerMod.exists) {
                cmd += " --out " + out + " --search \"" + search + "\" " + dirs.cache.join(config.mvc.appmod) + " " + 
                    controllerMod + " " + intermediate
            } else if (appName) {
                cmd += " --out " + out + " --search \"" + search + "\" " +  dirs.cache.join(config.mvc.appmod) + " " + 
                    intermediate
            } else {
                cmd += " --out " + out + " --search \"" + search + "\" " + intermediate
            }
            if (verbose > 1) {
                trace("[BUILD]", cmd)
            } else {
                trace("[BUILD]", file)
            }
            runCommand(cmd)

            if (!exists(out)) {
                throw "Compilation failed for " + out + "\n" + results
            }
            if (!options.keep) {
                rm(intermediate)
            }
            return null
        }
        return intermediate
    }

    function buildWebPage(file: Path, compile: Boolean = true): Path? {
        let sansExt: Path = file.trimExt()
        file = file.joinExt(ext.ejs)
        if (file.extension != ext.ejs) {
            throw "File is not an Ejscript web page: " + file
        }
        if (!file.exists) {
            throw "Can't find ejs page: " + file
        }
        if (verbose > 1) {
            trace("[PARSE]", file)
        }
        let parser: TemplateParser = new TemplateParser
        // results = Loader.wrap(file, parser.build(file.readString(), { layout: layoutPage }))
        results = Loader.wrap(file, parser.build(file.readString()))
        let intermediate: Path = dirs.cache.join(md5(file)).joinExt(ext.es)
        try {
            intermediate.write(results)
        } catch (e) {
            throw "Can't write module file: " + intermediate + ". Ensure directory is writable."
        }
        if (compile) {
            let out = intermediate.replaceExt(ext.mod)
            let cmd: String = getCompilerPath()
            let search = App.search.join(App.SearchSeparator)
            cmd += " --out " + out + " --search \"" + search + "\" " + intermediate
            trace("[BUILD]", (verbose > 1) ? cmd : file)
            runCommand(cmd)
            if (!out.exists) {
                throw "Compilation failed for " + out + "\n" + results
            }
            if (!options.keep) {
                rm(intermediate)
                return null
            }
        }
        return intermediate
    }

    function getCompilerPath(): String {
        let cmd = config.mvc.compiler
        let parts = cmd.split(" ")
        let path = parts[0].trim('"')
        if (path.isAbsolute) {
            /* Absolute compiler path */
            return cmd
        }
        path = App.exeDir.join(path)
        return '"' + path + '" ' + parts.slice(1).join(" ")
    }

    /*
        Build the entire app into a single mod file.
     */
    function buildFiles(out: Path, files: Array) {
        let search = App.search.join(App.SearchSeparator)
        let cmd = getCompilerPath() + " --out " + out + " --search \"" + search + "\" " + files.join(" ")
        if (verbose > 1) {
            trace("[BUILD]", cmd)
        } else {
            trace("[BUILD]", files.join(" "))
        }
        let results = runCommand(cmd)
        if (!exists(out)) {
            throw "Compilation failed for " + out + "\n" + results
        }
    }

    function buildApp(): Void {
        let pat = "**." + ext.es
        let files = find("src", pat) + find(dirs.models, pat) + find(dirs.controllers, "Base.es")
        buildFiles(dirs.cache.join(config.mvc.appmod), files)
    }

    function browse(args: Array): Void {
        throw("No yet supported")
        let cmd = config.mvc.browser

        if (!cmd.match(/^\/|^[a-zA-Z]:\//)) {
            cmd = cmd.trim('"').replace(/^[^ ]+/, App.exeDir.join("$&"))
        }
        trace("[RUN]", cmd)
        Cmd.run(cmd)
    }

    function deploy(args: Array): Void {
        //  TODO
    }

    //  TODO - needs to be updated
    function import(args: Array): Void {
        overwrite = true
        let exe = ""
        let lib = ""
        switch (Config.OS) {
        case "windows":
            exe = ".exe"
            lib = ".dll"
            break
        case "macosx":
            lib = ".dylib"
            break
        default:
            lib = ".so"
            break
        }

        //  TODO - import must support ajs alternatives
        binFiles = [ "ejs", "ejsc", "mvc", ]
        extFiles = [ "libcrypto", "libssl", "libmprssl" ]

        //  TODO - not used
        confFiles = [ ]

        if (exists(dirs.mod.join("ejs.db.mod"))) {
            //  TODO - other modules
            modFiles = [ "ejs.mod", "ejs.db.mod", "ejs.db.sqlite.mod", "ejs.web.mod" ]
            modLibFiles = [ "ejs.db.sqlite", "ejs.web" ]
            libFiles = [ "libpcre", "libec", "libmpr", "libsql", "libejs" ]
        } else {
            //  Static
            modFiles = [ "ejs.mod", "ejsweb.mod" ]
            modLibFiles = [ ]
            libFiles = [ ]
            binFiles = [ "ejsc", "mvc" ]
        }

        for each (file in modFiles) {
            src = dirs.mod.join(file)
            dest = Path("bin").join(file)
            if (!exists(src)) {
                error("WARNING: Can't find: " + file + " Continuing ...")
            }
            copyFile(src, dest, "Import")
            chmod(dest, 0644)
        }
        for each (file in modLibFiles) {
            src = dirs.mod.join(file).toString() + lib
            dest = Path("bin").join(file).toString() + lib
            if (!exists(src)) {
                error("WARNING: Can't find: " + file + " Continuing ...")
            }
            copyFile(src, dest, "Import")
            chmod(dest, 0644)
        }
        for each (file in libFiles) {
            dest = Path("bin").join(file).joinExt(lib)
            src = dirs.bin.join(file).joinExt(lib)
            if (!exists(src)) {
                error("WARNING: Can't find: " + file + " Continuing ...")
            }
            copyFile(src, dest, "Import")
            chmod(dest, 0755)
        }
        for each (file in binFiles) {
            dest = Path("bin").join(file).joinExt(exe)
            src = dirs.bin.join(file).joinExt(exe)
            if (!exists(src)) {
                error("WARNING: Can't find: " + file + " Continuing ...")
            }
            copyFile(src, dest, "Import")
            chmod(dest, 0755)
        }
        for each (file in extFiles) {
            dest = Path("bin").join(file).joinExt(lib)
            src = dirs.bin.join(file).joinExt(lib)
            if (exists(file)) {
                copyFile(src, dest, "Import")
                chmod(dest, 0755)
            }
        }
        overwrite = false
        for each (file in confFiles) {
            dest = Path("bin").join(file)
            src = dirs.bin.join(file)
            if (!exists(src)) {
                error("WARNING: Can't find: " + file + " Continuing ...")
            }
            copyFile(src, dest, "Import")
            chmod(dest, 0644)
        }
    }

    //  TODO - should this be here or in jem
    function install(args: Array): Void {
    }

    //  TODO - should this be here or in jem
    function uninstall(args: Array): Void {
    }

    function generate(args: Array): Void {
        if (args.length == 0) {
            args.append("all")
        }
        let kind: String = args[0].toLowerCase()
        let rest: Array = args.slice(1)

        if (rest.length == 0) {
            usage()
            return
        }
        switch (kind) {
        case "app":
            generateApp(rest)
            break

        case "controller":
            generateController(rest)
            break

        case "migration":
            generateMigration(rest)
            break

        case "model":
            generateModel(rest, "Create Model " + rest[0].toPascal())
            break

        case "scaffold":
            generateScaffold(rest)
            break

        default:
            usage()
            return
        }
    }

    /*
        mvc migrate              # Apply all migrations
        mvc migrate NNN          # Intelliegently set to a specific migration
        mvc migrate forward      # Migrate forward one
        mvc migrate backward     # Migrate backward one
     */
    function migrate(args: Array? = null): Void {
        let files = dirs.migrations.files().sort()
        let onlyOne = false
        let backward = false
        let targetSeq = null
        let id = null

        openDatabase()

        /*  
            Load the models
         */
        let appmod = dirs.cache.join(config.mvc.appmod)
        if (!appmod.exists) {
            buildApp()
        }
        load(appmod)
        if (overwrite) {
            let name = config.database[mode].name
            rm(name)
            generateDatabase()
        }

        /*
            The database has a _EjsMigrations table which has a record for each migration applied
         */
        let MigrationsTable = getMigrationModel()
        let migrations = MigrationsTable.findAll()
        let lastMigration = migrations.slice(-1)
        let command

        if (args && args.length > 0) {
            command = args.slice(0).toString().toLowerCase()
        } else {
            command = ""
        }
        if (command == "forward" || command == "forw") {
            onlyOne = true

        } else if (command == "backward" || command == "back") {
            onlyOne = true
            backward = true

        } else if (command != "") {
            /* command may be a pure sequence number or a filename */
            targetSeq = command
            let found = false
            for each (f in files) {
                //  TODO - copy esp code
                let base = f.basename.toString().toLowerCase()
                if (Path(targetSeq).basename == base) {
                    targetSeq = base.replace(/^([0-9]*)_.*es/, "$1")
                    found = true
                } else {
                    let seq = base.replace(/^([0-9]*)_.*es/, "$1")
                    if (seq == targetSeq) {
                        found = true
                    }
                }
            }
            if (! found) {
                throw "Can't find target migration: " + targetSeq
            }
            if (lastMigration && targetSeq < lastMigration[0].version) {
                backward = true
            }
        }
        if (backward) {
            files = files.reverse()
        }

        for each (f in files) {
            let base = f.basename.toString()
            if (!base.match(/^([0-9]+).*es/)) {
                continue
            }
            let seq = base.replace(/^([0-9]*)_.*es/, "$1")
            if (seq == "") {
                continue
            }
            let found = false
            for each (appliedMigration in migrations) {
                if (appliedMigration["version"] == seq) {
                    found = true
                    id = appliedMigration["id"]
                    break
                }
            }
            if (backward) {
                /* TODO - cant happen
                if (targetSeq && targetSeq == seq) {
                    return
                }
                */
                found = !found
            }
            if (!found) {
                try { delete Migration; } catch {}
                load(f)
                if (backward) {
                    trace("[MIGRATE]", "Reverse " + base)
                    new Migration().backward(db)
                } else {
                    trace("[MIGRATE]", "Apply " + base)
                    new Migration().forward(db)
                }
                if (backward) {
//  TODO - what if id not set
                    MigrationsTable.remove(id)
                } else {
                    migration = new MigrationsTable
                    migration["version"] = seq.toString()
                    migration.save()
                }
                if (onlyOne) {
                    return
                }
            }
            //  TODO - don't need to test backward
            if (!backward && targetSeq && targetSeq == seq) {
                return
            }
        }
        //  TODO - should this not be !onlyOne
        if (onlyOne) {
            if (backward) {
//  TODO - what is OMIT?
                trace("[OMIT]", "All migrations reversed")
            } else {
                trace("[OMIT]", "All migrations applied")
            }
        }
    }

    function run(args: Array): Void {
        let start = config.mvc.start
        trace("[RUN]", start)
        load(start)
    }

    function findTop(dir: Path) {
        do {
            if (exists(dir.join("configure"))) {
                return dir
            }
            prev = dir
            dir = dir.parent
        } while (!dir.same(prev))
        throw "Can't find configure in local source tree"
    }

    /*  
        Generate an application.
        mvc generate app appName
     */
    function generateApp(args: Array): Void {
        appName = args[0].toLowerCase()
        if (appName == '.') {
            appName = App.dir.basename
        } else {
            makeDir(appName)
            App.chdir(appName)
        }
        if (options.full) {
            makeDir("bin")
            makeDir("doc")
            makeDir("logs")
            makeDir("messages")
            makeDir("test")
        }
        makeDir(dirs.cache)
        makeDir(dirs.controllers)
        makeDir(dirs.db)
        makeDir(dirs.migrations)
        makeDir(dirs.models)
        makeDir(dirs.src)
        makeDir(dirs.views)
        makeDir(dirs.views.join("Base"))
        makeDir(dirs.layouts)
        makeDir(dirs.static)
        makeDir(dirs.static.join("images"))
        makeDir(dirs.static.join("themes"))

        generateStart()
        generateAppSrc()
        generateConfig()
        generateLayouts()
        generateHomePage()
        generatePages()
        generateBaseController()
        if (!options.min) {
            generateReadme()
        }
        generateDatabase()

        let files = ["src/App.es", dirs.controllers.join("Base.es")]
        buildFiles(dirs.cache.join("App").joinExt(ext.mod), files)
        App.chdir("..")

        if (verbose) {
            print("\nChange directory into your application directory: " + appName)
            print("Then run the web server via: \"" + mvc + " run\"")
            print("and point your browser at: http://localhost:" + endpoint + "/ to view your app.")
        }
    }

    function generateConfig(): Void {
        let data = Templates.Ejsrc.replace(/\${NAME}/g, appName)
        data = data.replace(/\${PATH}/g, App.dir)
        makeConfigFile("ejsrc", data)
        /* Reload */
        loadConfig("ejsrc", true)
        assert(dirs == config.dirs)
        for (d in dirs) {
            dirs[d] = Path(dirs[d])
        }
    }

    function generateLayouts(): Void {
        let data = Templates.DefaultLayout.replace(/\${NAME}/g, appName.toPascal())
        data = data.replace(/\${STATIC}/g, dirs.static)
        makeFile(dirs.layouts.join("default.ejs"), data, "Layout")
    }

    function generateHomePage(): Void {
        let data = Templates.HomePage.replace(/\${NAME}/g, appName.toPascal())
        data = data.replace(/\${STATIC}/g, dirs.static)
        makeFile(dirs.views.join("Base/index.ejs"), data, "HomePage")
    }

    function generatePages(): Void {
        let path: Path = dirs.bin.join("www")
        if (!exists(path)) {
            throw "Can't find www at " + path
        }
        for each (f in find(path, "**", {exclude: /\/$/})) {
            let name = f.name.slice(path.length + 1)
            copyFile(f, dirs.static.join(name), "Static web page")
        }
    }

    function generateBaseController(): Void {
        let path: Path = dirs.controllers.join("Base.es")
        let data = Templates.BaseController.replace(/\${NAME}/g, appName)
        makeFile(path, data, "BaseController")
    }

    function generateStart(): Void {
        let data: String = Templates.Start
        makeFile("start.es", data, "start.es")
    }

    function generateAppSrc(): Void {
        let data: String = Templates.AppSrc
        makeFile("src/App.es", data, "App")
    }

    function generateReadme(): Void {
        let data: String = Templates.Readme.replace(/\${NAME}/g, appName.toPascal())
        makeFile("README", data, "README")
    }

    function loadDatabaseSupport() {
        loadMapper()
    }

    function loadMapper() {
        if (!global.Record) {
            load("ejs.db.mapper.mod")
        }
    }

    function getMigrationModel() {
        loadMapper()
        if (!global._EjsMigration) {
            eval("
                require ejs.db.mapper
                public dynamic class _EjsMigration implements Record {
                    function _EjsMigration(fields: Object? = null) {
                        initialize(fields)
                    }
                }
            ")
        }
        return global["_EjsMigration"]
    }

    function generateDatabase(): Void {
        loadMapper()
        let path = config.database[mode].name
        if (!exists(path)) {
            db = new Database(database, path)
            if (debug) {
                db.trace(true)
            }
            db.createTable("_EjsMigrations", ["version:string"])
            if (debug) {
                db.trace(true)
            }
        }
    }

    function openDatabase(): Void {
        loadMapper()
        let name = config.database[mode].name
        if (!exists(name)) {
            generateDatabase()
        } else {
            db = new Database(database, name)
            if (debug) {
                db.trace(true)
            }
        }
    }

    /*  generate controller name [action ...]
     */
    function generateController(args: Array): Void {
        let name: String = args[0].toPascal()
        let actions = args.slice(1)
        let path: String = dirs.controllers.join(name).joinExt(ext.es)
        let data: String = Templates.Controller.replace(/\${NAME}/g, name)
        data = data.replace(/\${APP}/g, appName)

        if (actions.length == 0) {
            actions.append("index")
        }
        for each (action in actions) {
            let actionData = Templates.Action.replace(/\${NAME}/g, action)
            data = data.replace(/NEXT_ACTION/, actionData + "NEXT_ACTION")
        }
        data = data.replace(/NEXT_ACTION/, "")
        data = data.replace(/\${MODEL}/g, name.toPascal())
        data = data.replace(/\${LOWER_MODEL}/g, name.toLowerCase())
        makeFile(path, data, "Controller")
    }

    function createMigrationCode(model: String, forward: String, backward: String, comment: String) {
        data = Templates.Migration
        data = data.replace(/\${COMMENT}/g, comment)
        data = data.replace(/\${FORWARD}/g, forward)
        data = data.replace(/\${BACKWARD}/g, backward)

        seq = (new Date()).format("%Y%m%d%H%M%S") + nextMigration++
        fileComment = comment.replace(/[    ]+/g, "_")
        path = Path("db/migrations/" + seq + "_" + fileComment).joinExt(ext.es)
        if (path.exists) {
            throw "Migration " + path + " already exists. Try again later."
        }
        makeFile(path, data, "Migration")
    }

    function validateAttributes(attributes: Array): Void {
        loadDatabaseSupport()
        for each (attribute in attributes) {
            column = attribute.split(":")[0]
            datatype = attribute.split(":")[1]
            if (db.dataTypeToSqlType(datatype) == undefined) {
                throw "Unsupported data type: \"" + datatype + "\" for column \"" + column + "\""
            }
        }
    }

    function createMigration(model: String, attributes: Array, comment: String, tableExists: Boolean): Void {
        let tableName = toPlural(model).toPascal()
        let forward = ''
        let backward = ''

        openDatabase()
        if (attributes && attributes.length > 0) {
            validateAttributes(attributes)
            if (!tableExists) {
                forward = '        db.createTable("' + tableName + '", ["' + attributes.join('", "') + '"])'
                backward = '        db.destroyTable("' + tableName + '")'

            } else {
                forward = ""
                for each (col in attributes)  {
                    spec = col.split(":")
                    forward += '        db.addColumn("' + tableName + '", "' + spec[0] + '", "' + spec[1] + '")\n'
                }
                backward = '        db.removeColumns("' + tableName + '", ['
                for each (col in attributes) {
                    backward += '"' + col.split(":")[0] + '", '
                }
                backward += '])'
            }

        } else {
            if (options.reverse) {
                forward = '        db.destroyTable("' + tableName + '")'
            }
        }
        if (options.reverse) {
            createMigrationCode(model, backward, forward, comment)
        } else {
            createMigrationCode(model, forward, backward, comment)
        }
    }

    /*  
        generate migration description model [field:type ...]
     */
    function generateMigration(args: Array): Void {
        if (args.length < 2) {
            usage()
        }
        comment = args[0]
        model = args[1]
        createMigration(model, args.slice(2), comment, true)
    }

    /*  
        generate model name [field:type ...]
     */
    function generateModel(args: Array, comment: String): Void {
        let model: String = args[0].toPascal()
        if (model.endsWith("s")) {
            error("WARNING: Models should typically be singluar not plural. Continuing ...")
        }
        let path = dirs.models.join(model).joinExt(ext.es)

        if (exists(path) && !overwrite) {
            traceFile(path, "[EXISTS] Migration (model already exists)")
        } else {
            createMigration(model, args.slice(1), comment, false)
        }
        let data = Templates.Model.replace(/\${NAME}/g, model)
        makeFile(path, data, "Model")
    }

    /*  
        generate scaffold model [field:type ...]
     */
    function generateScaffold(args: Array): Void {
        let model = args[0]
        if (model.match(/[a-zA-Z_]*/) != model) {
            throw "Bad model name " + model
        }
        let controller = model.toPascal()
        let attributes = args.slice(2)

        makeDir(dirs.views.join(controller))
        generateModel(args, "Create Scaffold " + model)
        generateScaffoldController(controller, model)
        generateScaffoldViews(controller, model)
        buildApp()
        migrate()
    }

    /*  
        Create a controller with scaffolding. Usage: controllerName [actions ...]
     */
    function generateScaffoldController(controller: String, model: String, extraActions: Array? = null): Void {
        let name = controller.toPascal()
        let path = dirs.controllers.join(name).joinExt(ext.es)

        let stndActions: Array = [ "create", "destroy", "edit", "index", "init", "show", "update" ]
        let views: Array = [ "index", "edit" ]
        let actions: Array = []

        if (extraActions) {
            for each (action in extraActions) {
                if (! stndActions.contains(action)) {
                    actions.append(action.toCamel())
                }
            }
        }

        let data: String = Templates.ScaffoldController.replace(/\${APP}/g, appName.toPascal())
        data = data.replace(/\${NAME}/g, name)
        data = data.replace(/\${MODEL}/g, model.toPascal())
        data = data.replace(/\${LOWER_MODEL}/g, model.toLowerCase())

        for each (action in actions) {
            let actionData = Templates.Action.replace(/\${NAME}/g, action)
            data = data.replace(/NEXT_ACTION/, actionData + "NEXT_ACTION")
        }
        data = data.replace(/NEXT_ACTION/, "")

        makeFile(path, data, "Controller")
    }

    /*  
        Create a scaffold views.  Usage: controllerName [actions ...]
     */
    function generateScaffoldViews(controller: String, model: String, extraActions: Array? = null): Void {
        let stndActions: Array = [ "create", "destroy", "edit", "index", "init", "show", "update" ]
        let views: Array = [ "index", "edit" ]
        let actions: Array = stndActions.clone()

        if (extraActions) {
            for each (action in extraActions) {
                if (! stndActions.contains(action)) {
                    views.append(action.toCamel())
                }
            }
        }
        let data: String
        model = model.toPascal()

        for each (view in views) {
            switch (view) {
            case "edit":
                data = Templates.ScaffoldEditView.replace(/\${MODEL}/g, model)
                data = data.replace(/\${LOWER_MODEL}/g, model.toLowerCase())
                break
            case "index":
                data = Templates.ScaffoldIndexView.replace(/\${MODEL}/g, model)
                break
            default:
                data = Templates.ScaffoldView.replace(/\${MODEL}/g, model)
                data = data.replace(/\${LOWER_MODEL}/g, model.toLowerCase())
                data = data.replace(/\${CONTROLLER}/g, controller)
                data = data.replace(/\${VIEW}/g, view)
                break
            }
            let path: String = dirs.views.join(controller).join(view).joinExt(ext.ejs)
            makeFile(path, data, "View")
        }
    }

    function checkApp(task: String, rest: Array): Void {
        if (task == "generate") {
            let what = rest[0]
            if (rest[0] == "app") {
                return
            }
            if (what != "app" && what != "controller" && what != "migration" && what != "model" && what != "scaffold") {
                usage()
                App.exit()
            }
        }
        appName = App.dir.basename.toString().toLowerCase()
        /* Removed so we can compile non-mve apps
        if (!config.database) {
            throw new IOError("Not an MVC application directory")
        }
        */
        if (!dirs.cache.exists || !isDir(dirs.cache)) {
            upgradeApp()
        }
    }

    function loadConfig(path: Path, overwrite: Boolean = true, mandatory: Boolean = false): Boolean {
        if (!path.exists) {
            if (mandatory) {
                throw new IOError("Can't open required configuration file: " + path)
            } else {
                return false
            }
        }
        blend(config, path.readJSON(), {overwrite: overwrite})
        return true
    }

    function makeConfigFile(path: String, data: String): Void {
        if (exists(path) && !overwrite) {
            return
        }
        data = data.replace(/\${NAME}/g, appName)
        data = data.replace(/\${ENDPOINT}/g, endpoint)
        makeFile(path, data, "Config File")
    }

    function makeFile(path: String, data: String, msg: String): Void {
        let p: Path = new Path(path)
        if (p.exists && !overwrite) {
            traceFile(path, "[EXISTS] " + msg)
            return
        }
        if (! p.exists) {
            traceFile(path, "[CREATED] " + msg)
        } else {
            traceFile(path, "[OVERWRITTEN] " + msg)
        }
        let f: File = new File(path)
        f.open("w")
        f.write(data)
        f.close()
    }

    function makeDir(path: String): Void {
        if (isDir(path)) {
            return
        }
        trace("[CREATED] " + "Directory", path)
        mkdir(path, DIR_PERMS)
    }

    function copyFile(from: Path, to: Path, msg: String) {
        let p: Path = new Path(to)
        if (to.exists && !overwrite) {
            if (!from.isDir) {
                traceFile(to, "[EXISTS] " + msg)
            }
            return
        }
        if (!to.exists) {
            traceFile(to, "[CREATED] " + msg)
        } else {
            traceFile(to, "[OVERWRITTEN] " + msg)
        }
        if (!from.isDir) {
            mkdir(to.dirname, DIR_PERMS)
            cp(from, to)
        }
    }

    function getNthSegment(path: String, nth: Number) {
        let segments: Array = path.split(/(\\|\/)+/g)
        for (let i: Number = segments.length - 1; i >= 0; i--) {
            if (segments[0] == ".") {
                segments.remove(i, i)
            }
        }
        return segments[nth]
    }

    function runCommand(command: String): String {
        let results
        try {
            results = Cmd.run(command)
        } 
        catch (e) {
            msg = "Compilation failure, for " + command + "\n\n" +
                (e cast String).replace(/Error Exception: Command failed: Status code [0-9]*.\n/, "")
            throw msg
        }
        return results
    }

    function traceFile(path: String, msg: String): Void {
        //  TODO - string method to add quotes would be useful
        trace(msg, '"' + path + '"')
    }

    function trace(tag: String, ...args): Void {
        if (verbose > 0) {
            print("  " + tag + ": " + args.join(" "))
        }
    }

    function upgradeApp() {
        if (!dirs.cache.exists || !isDir(dirs.cache)) {
            makeDir(dirs.cache)
        }
        if (!exists("ejsrc")) {
            generateConfig()
        }
    }
}


/*
    Templates for various files
 */
class Templates {
    
    /*
     ***************** ejsrc template ***********************
     */
    public static const Ejsrc =
'{
    mode: "debug",

    app: {
    },

    cache : {
        lifespan: 86400,
        app:     { enable: true, reload: true },
        actions: { enable: true, lifespan: 3600 },
        workers: { enable: true, limit: 10},
    },
    database: {
        adapter: "sqlite",
        class: "Sqlite",
        module: "ejs.db.sqlite",
        debug: {
            name: "db/${NAME}.sdb", username: "", password: "", trace: false,
        },
        test: {
            name: "db/${NAME}.sdb", username: "", password: "", trace: false,
        },
        production: {
            name: "db/${NAME}.sdb", username: "", password: "", trace: false,
        },
    },
    
    dirs: {
        cache: "cache",
        db: "db",
        migrations: "db/migrations",
        layouts: "layouts",
        src: "src",
        static: "static",
        views: "views",
    },

    log: {
        enable: true,
        location: "stderr",
        level: 0,
        showErrors: true,
    },

    mvc: {
        start: "start.es",
    },

    web: {
        expires: {
            html:   86400,
            png:    86400,
            gif:    86400,
            ico:    86400,
            jpg:    86400,
            jpeg:   86400,
            js:     86400,
            css:    86400,
        },
        limits: {
            inactivityTimeout: 3600, 
            requestTimeout: 3600, 
            sessionTimeout: 3600, 
        },
        listen: "${ENDPOINT}",
        trace: {
            rx: { exclude: ["jpg", "gif", "png", "ico", "css", "js"], conn: 5, first: 2, headers: 3, body: 4, size: 1024 },
            tx: { exclude: ["jpg", "gif", "png", "ico", "css",], headers: 4, body: 5, size: 10240 },
        },
        views: {
            connectors: {
                table: "html",
                chart: "google",
                rest: "html",
            },
            formats: {
                currency:   "$%10f",
                Date:       "%a %e %b %H:%M",
            },
            helpers: [],
        },
    },

}
'

    /*
     *****************  BaseController template ***********************
     */
    public static const BaseController = 
'/*
    BaseController.es - Base class for all controllers
 */
require ejs.web

public class BaseController extends Controller {

    public var title: String = "${NAME}"
    public var style: String

    function BaseController() {
    }
}
'


    /*
     *****************  Controller template ***********************
     */
    public static const Controller = 
'
require ejs.web

public class ${NAME}Controller extends BaseController {

    public var ${LOWER_MODEL}: ${MODEL}

    function ${NAME}Controller() {
    }

    NEXT_ACTION
}
'


    /*
     *****************  ScaffoldController template ******************
     */
    public static const ScaffoldController = 
'
require ejs.web

public class ${NAME}Controller extends BaseController {

    public var ${LOWER_MODEL}: ${MODEL}

    function ${NAME}Controller() {
    }

    action function create() {
        ${LOWER_MODEL} = new ${MODEL}(params.${LOWER_MODEL})
        if (${LOWER_MODEL}.save()) {
            inform("New ${LOWER_MODEL} created")
            redirect("@")
        } else {
            writeView("edit")
        }
    }

    action function destroy() {
        ${MODEL}.remove(params.id)
        inform("${MODEL} " + params.id + " removed")
        redirect("@")
    }

    action function edit() {
        ${LOWER_MODEL} = ${MODEL}.find(params.id)
    }

    action function init() {
        ${LOWER_MODEL} = new ${MODEL}
        writeView("edit")
    }

    action function show() {
        ${LOWER_MODEL} = ${MODEL}.find(params.id)
        writeContent(post)
    }

    action function update() {
        ${LOWER_MODEL} = ${MODEL}.find(params.id)
        if (${LOWER_MODEL}.saveUpdate(params.${LOWER_MODEL})) {
            inform("${MODEL} updated successfully.")
            redirect("@")
        } else {
            /* Validation failed */
            writeView("edit")
        }
    }

    NEXT_ACTION
}
'


    /*
     *****************  ScaffoldIndexView template ******************
     */
    public static const ScaffoldIndexView = 
'<h1>${MODEL} List</h1>

<% table(${MODEL}.findAll(), {click: "@edit"}) %>
<% buttonLink("New ${MODEL}", "@init") %>
'


    /*
     *****************  ScaffoldEditView template ******************
     */
    public static const ScaffoldEditView = 
'<h1><%= (${LOWER_MODEL}.id) ? "Edit" : "Create" %> ${MODEL}</h1>

<% form(${LOWER_MODEL}) %>
    <table border="0">
        <% for each (name in ${MODEL}.getColumnNames()) {
            if (name == "id") continue
            uname = name.toPascal()
        %>
        <tr><td>@@uname</td><td><% input(name) %></td></tr>
    <% } %>
    </table>
    <% button("commit", "OK") %>
    <% buttonLink("Cancel", "@") %>
    <% if (${LOWER_MODEL}.id) buttonLink("Delete", {method: "DELETE", click: "@destroy"}) %>
<% endform() %>
'


    /*
     *****************  ScaffoldView template ******************
     */
    public static const ScaffoldView = 
'<h1>View "${CONTROLLER}/${VIEW}" for Model ${MODEL}</h1>
<p>Edit in "views/${CONTROLLER}/${VIEW}.ejs"</p>
'


    /*
     ***********************  Action template ***********************
     */
    public static const Action = '
    action function ${NAME}() {
    }

'


    /*
     ***********************  Model template ***********************
     */
    public static const Model = 
'/*
    ${NAME}.es - ${NAME} Model Class
 */

require ejs.db.mapper

public dynamic class ${NAME} implements Record {

    function ${NAME}(fields: Object? = null) {
        initialize(fields)
    }
}
'


    /*
     ***********************  Migration template ***********************
     */
    public static const Migration = 
'/*
    ${COMMENT}
 */
require App
require ejs.db
require ejs.db.mapper

public class Migration {

    public function forward(db) {
${FORWARD}    
    }

    public function backward(db) {
${BACKWARD}
    }
}
'



    /*
     **************************** README template ***********************
     */
    public static const Readme = 
'
README - Overview of files and documentation generated by ejsmvc.
Some of these directories will not be present if generation did 
not use the --full switch. 

These Directories are created via command "ejsmvc generate app"

    bin                       Programs and scripts
    cache                     Cached module files
    controllers               Controller source
    db                        SQL databases and database scripts
    db/migrations             SQL database migration scripts
    doc                       Documentation for the application
    layouts                   Page template layout files
    models                    Database models
    src                       Extra application source
    views                     View source files
    static                    Public static web content directory
    static/js                 Client side Javascript files
    static/themes             Theme style sheet directory

These files are also created:

    ejsrc                     Configuration file
    layouts/default.ejs       Default template layout page
    start.es                  Startup script for web serving
    static/layout.css         Default layout style sheet
    static/themes/default.css Default theme style sheet
    static/images/banner.jpg  Default UI banner
'

/*  FUTURE
    logs                      Log files
    messages                  Internationalization messages
    test                      Test files
*/

    /*
     ***************************  start.es ****************************
     */
    public static const Start = 
'
require ejs.web
    
let server: HttpServer = new HttpServer
var router = new Router(Router.Restful)

server.on("readable", function (event, request) {
    App.log.info(request.method, request.uri, request.scheme)
    server.serve(request, router)
})

App.log.info("Listen on", App.config.web.listen)
server.listen(App.config.web.listen)
App.run()
'

    /*
     ***************************  src/App.es ****************************
     */
    public static const AppSrc = 
'
module App {
}
'

    /*
     ***************************  Default Layout templates ******************
     */
    public static const DefaultLayout = 
'<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8" />
    <% securityToken() %>
    <% icon("/${STATIC}/images/favicon.ico") %>
    <% stylesheet() %>
    <% script() %>
    <title>@@title</title>
</head>

<body>
    <div class="top">
        <h1><% label("${NAME} Application", request.home) %></h1>
    </div>
    <div id="logo">EJScript&trade;</div>

    <% flash(["inform", "error", "message", "warning"]) %>
    <div class="content">
        <%@ content %>
    </div>

    <div class="bottom">
        <p class="footnote">Powered by Ejscript&trade;</p>
    </div>
</body>
</html>
'

    public static const HomePage = '
<%@ layout "default" %>
<div class="contentLeft">
    <h1>Welcome to Ejscript</h1>

    <% image("/${STATIC}/images/splash.jpg", { style: "wrapLeft shadow" }) %>
    <p>Ejscript is a web framework that makes it dramatically easier to create dynamic web 
    applications using Server-Side JavaScript.</p>

    <p>Ejscript has an application generator, templating engine, a Model/View/Controller framework and a library 
        of Ajax view controls. Enjoy!</p>
    <br/>

    <h2>Quick Start</h2>
    <ol>
        <li><b>Review Configuration Files</b>
            <p>Review ejsrc and tailor if required. Update your database configuration and password.</p>
        </li>

        <li><b><a href="https://www.embedthis.com/ejscript/doc/web/views.html">Create Views</b></a>
            <p>Create views under the <b>views</b> directory. Modify the layout in "layouts/default.ejs" and 
               customize the style sheet in the "web" directory for static content.</p>
        </li>
        <li><b><a href="https://www.embedthis.com/ejscript/doc/web/controllers.html">Generate 
            Models and Controllers</a></b>:
            <p>Create controllers to manage your app. Run 
                <a href="https://www.embedthis.com/ejscript/doc/web/mvc.html"><b>mvc</b></a> 
                with no options to see its usage.</p>
            <pre>mvc generate controller NAME [action, ...]</pre>
        </li>
        <li><b><a href="https://www.embedthis.com/ejscript/doc/web/mvc.html#scaffolds">Generate 
            Scaffolds</a></b>:
            <p>Create entire scaffolds for large sections of your application. 
            <pre>mvc generate scaffold model [field:type, ...]</pre>
        </li>
        
        <li><b><a href="https://www.embedthis.com/ejscript/doc/index.html">Read the Documentation</b>
            <p>Go to <a href="https://www.embedthis.com/ejscript/doc/index.html">
            https://www.embedthis.com/ejscript/doc/index.html</a> for the latest Ejscript documentation. 
            Here you can read quick tours, overviews and access all the Ejscript APIs.</b>
        </li>
        <li><b>Enjoy!</b></li>
        <p>&nbsp;</p>
</div>

<div class="contentRight">
    <h2 class="section">Ejscript Links</h2>
    <ul>
        <li><a href="https://www.embedthis.com">Official Web Site</li>
        <li><a href="https://www.embedthis.com/ejscript/doc/index.html">Documentation</li>
        <li><a href="https://www.embedthis.com/ejscript/doc/ref/webArchitecture.html">Web Framework</li>
        <li><a href="https://www.embedthis.com/ejscript/doc/api/ejscript/index.html">Ejscript API</li>
        <li><a href="https://www.embedthis.com/blog/">Blog</li>
    </ul>
</div>
'

/* End of class Templates */
}


EjsMvc().main()

/*
    @copy   default

    Copyright (c) Embedthis Software. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
