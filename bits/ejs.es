/*
    Support functions for Embedthis Ejscript

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

require ejs.tar
require ejs.unix

/*
    Copy binary files to package staging area
    This is run for local and cross platforms. The last platform does the packaging
 */
public function packageBinaryFiles(formats = ['tar', 'native']) {
    let settings = bit.settings
    let bin = bit.dir.pkg.join('bin')
    safeRemove(bit.dir.pkg)
    let vname = settings.product + '-' + settings.version + '-' + settings.buildNumber
    let pkg = bin.join(vname)
    pkg.makeDir()

    let contents = pkg.join('contents')

    let prefixes = bit.prefixes;
    let p = {}
    for (prefix in bit.prefixes) {
        if (prefix == 'config' || prefix == 'log' || prefix == 'spool' || prefix == 'src' || prefix == 'web') {
            continue
        }
        p[prefix] = Path(contents.portable.name + bit.prefixes[prefix].removeDrive().portable)
        p[prefix].makeDir()
    }
    let strip = bit.platform.profile == 'debug'

    if (!bit.cross) {
        /* These three files are replicated outside the data directory */
        install('doc/product/README.TXT', pkg, {fold: true, expand: true})
        install('package/install.sh', pkg.join('install'), {permissions: 0755, expand: true})
        install('package/uninstall.sh', pkg.join('uninstall'), {permissions: 0755, expand: true})
        if (bit.platform.os == 'windows') {
            install('package/windows/LICENSE.TXT', bin, {fold: true, expand: true})
        }
        /* Move ejscript-license to the front */
        let files = Path('doc/licenses').files('*.txt').reject(function(p) p.contains('ejscript-license.txt'))
        install(['doc/licenses/ejscript-license.txt'] + files, p.product.join('LICENSE.TXT'), {
            cat: true,
            textfile: true,
            fold: true,
            title: bit.settings.title + ' Licenses',
        })
        install('doc/product/README.TXT', p.product, {fold: true, expand: true})
        install('package/uninstall.sh', p.bin.join('uninstall'), {permissions: 0755, expand: true})
        install('package/linkup', p.bin, {permissions: 0755})
    }
    install(bit.dir.bin + '/*', p.bin, {
        include: /bit|ejs|ejsc|ejsman|ejspage|http|jem|mvc|sqlite|utest|\.dll/,
        exclude: /\.pdb|\.exp|\.lib|\.def|\.suo|\.old/,
        permissions: 0755, 
    })
    if (bit.platform.os != 'windows') {
        install(bit.dir.bin.join('*'), p.bin, {
            permissions: 0755, 
            exclude: /bits|file-save|www|simple|sample/,
        })
    }
    install(bit.dir.bin.join('bits'), p.bin)
    install(bit.dir.bin.join('www'), p.bin.join('www'), {exclude: /tree-images/})
    install(bit.dir.inc.join('*.h'), p.inc)

    if (bit.targets.libmprssl.enable && bit.platform.os == 'linux') {
        install(bit.dir.bin.join('*.' + bit.ext.shobj + '*'), p.bin, {strip: strip, permissions: 0755})
        for each (f in p.bin.files('*.so.*')) {
            let withver = f.basename
            let nover = withver.name.replace(/\.[0-9]*.*/, '.so')
            let link = p.bin.join(nover)
            f.remove()
            f.symlink(link.basename)
        }
    }
    if (!bit.cross) {
        if (bit.platform.os == 'windows') {
            let version = bit.packs.compiler.version.replace('.', '')
            if (bit.platform.arch == 'x64') {
                install(bit.packs.compiler.dir.join('VC/redist/x64/Microsoft.VC' +
                    version + '.CRT/msvcr' + version + '.dll'), p.bin)                                     
            } else {
                install(bit.packs.compiler.dir.join('VC/redist/x86/Microsoft.VC100.CRT/msvcr' + version + '.dll'), p.bin)
            }
            /*
                install(bit.packs.compiler.path.join('../../lib/msvcrt.lib'), p.bin)
             */
            install(bit.dir.bin.join('removeFiles' + bit.globals.EXE), p.bin)
        }
        if (bit.platform.like == 'posix') {
            install('doc/man/*.1', p.productver.join('doc/man/man1'), {compress: true})
        }
    }
    let files = contents.files('**', {exclude: /\/$/, relative: true})
    files = files.map(function(f) Path("/" + f))
    p.productver.join('files.log').append(files.join('\n') + '\n')

    if (formats && bit.platform.last) {
        package(bit.dir.pkg.join('bin'), formats)
    }
}

public function packageSourceFiles() {
    if (bit.cross) {
        return
    }
    let s = bit.settings
    let src = bit.dir.pkg.join('src')
    let pkg = src.join(s.product + '-' + s.version)
    safeRemove(pkg)
    pkg.makeDir()
    install(['Makefile', 'start.bit', 'main.bit'], pkg)
    install('bits', pkg)
    install('*.md', pkg, {fold: true, expand: true})
    install('configure', pkg, {permissions: 0755})
    install('src', pkg, {
        exclude: /\.log$|\.lst$|ejs.zip|\.stackdump$|\/cache|huge.txt|\.swp$|\.tmp|ejs.debugger|ejs.zip/,
    })
    install('doc', pkg, {
        exclude: /\/xml\/|\/html\/|Archive|\.mod$|\.so$|\.dylib$|\.o$/,
    })
    install('projects', pkg, {
        exclude: /\/Debug\/|\/Release\/|\.ncb|\.mode1v3|\.pbxuser/,
    })
    install('package', pkg, {})
    package(src, 'src')
}

public function packageComboFiles() {
    if (bit.cross) {
        return
    }
    let s = bit.settings
    let src = bit.dir.pkg.join('src')
    let pkg = src.join(s.product + '-' + s.version)
    safeRemove(pkg)
    pkg.makeDir()
    install('projects/ejs-' + bit.platform.os + '-debug-bit.h', pkg.join('src/deps/ejs/bit.h'))
    install('package/ejs-flat.bit', pkg.join('src/deps/ejs/ejs.bit'))
    install('package/Makefile-flat', pkg.join('src/deps/ejs/Makefile'))
    install('package/start-flat.bit', pkg.join('src/deps/ejs/start.bit'))
    let filter = /^#inc.*ejs.*$|^#inc.*ec.*$|^#inc.*customize.*$|#include.*zlib.h.*$|#inc.*ejs.zlib.slots.h.*/mg

    install([
        'src/deps/zlib/zlib.h', 
        'src/ejsByteCode.h', 
        'src/ejsByteCodeTable.h',
        'src/ejs.h', 
        'src/jems/ejs.web/ejsWeb.h', 
        'src/ejsCompiler.h', 
        'src/deps/pcre/pcre.h'
        ], pkg.join('src/deps/ejs/ejs.h'), {
        cat: true, filter: filter,
        header: '#include \"bit.h\"\n#include \"mpr.h\"\n#include \"http.h\"\n#include \"ejs.slots.h\"',
        title: bit.settings.title + ' Library Source',
    })
    install('src/deps/mpr/mpr.h', pkg.join('src/deps/ejs/mpr.h'))
    install('src/deps/http/http.h', pkg.join('src/deps/ejs/http.h'))
    install('src/vm/ejsByteGoto.h', pkg.join('src/deps/ejs/ejsByteGoto.h'))
    install(bit.dir.inc.join('ejs*.slots.h'), pkg.join('src/deps/ejs/ejs.slots.h'), {
        cat: true, filter: filter,
        title: bit.settings.title + ' Object Slot Definitions',
    })

    install(['src/deps/**.c'], pkg.join('src/deps/ejs/deps.c'), {
        cat: true,
        filter: filter,
        exclude: /pcre|makerom|http\.c|sqlite|manager|zlib/,
        header: '#include \"ejs.h\"',
        title: bit.settings.title + ' Library Source',
    })

    install(['src/deps/pcre/pcre.c', 'src/deps/pcre/pcre.h'], pkg.join('src/deps/appweb'))
    install(['src/deps/sqlite/sqlite3.c', 'src/deps/sqlite/sqlite3.h'], pkg.join('src/deps/sqlite'))

    install(['src/**.c'], pkg.join('src/deps/ejs/ejsLib.c'), {
        cat: true,
        filter: filter,
        /* Include deps/zlib */
        exclude: /doc\.c|listing\.c|ejsmod\.c|slotGen\.c|docFiles\.c|ejs\.c$|ejsc\.c$|mpr|http|pcre|\/sqlite|ejs.debugger|samples|utils/,
        header: '#define EJS_DEFINE_OPTABLE 1\n#include \"ejs.h\"',
        title: bit.settings.title + ' Library Source',
    })
    install(['src/core/**.es', 'src/jems/**.es'], pkg.join('src/deps/ejs/ejs.es'), {
        cat: true,
        filter: filter,
        exclude: /ejs.bit|ejs.debugger|test|sample|ejspage.es|ejs.jem|ejs.mvc/,
        title: bit.settings.title + ' Script Library',
    })
    install('src/cmd/ejs.c', pkg.join('src/deps/ejs/ejs.c'), {
        cat: true,
        filter: filter,
        header: '#include \"ejs.h\"',
        title: bit.settings.title + ' Shell Command',
    })
    install('src/cmd/ejsc.c', pkg.join('src/deps/ejs/ejsc.c'), {
        cat: true,
        filter: filter,
        header: '#include \"ejs.h\"',
        title: bit.settings.title + ' Compiler',
    })
    install([
        'src/cmd/ejsmod.h', 
        'src/cmd/doc.c', 
        'src/cmd/docFiles.c', 
        'src/cmd/ejsmod.c', 
        'src/cmd/listing.c', 
        'src/cmd/slotGen.c'
        ], pkg.join('src/deps/ejs/ejsmod.c'), {
        cat: true,
        filter: filter,
        header: '#include \"ejs.h\"',
        title: bit.settings.title + ' Manager',
    })
    package(pkg, ['combo', 'flat'])
}


public function installBinary() {
    if (Config.OS != 'windows' && App.uid != 0) {
        throw 'Must run as root. Use \"sudo bit install\"'
    }
    packageBinaryFiles(null)
    /* Preserve bit */
    let path = App.exePath
    if (path.same(bit.prefixes.bin.join('bit'))) {
        active = path.replaceExt('old')
        active.remove()
        path.rename(active)
    }
    package(bit.dir.pkg.join('bin'), 'install')
    if (Config.OS != 'windows') {
        createLinks()                                                                                          
        updateLatestLink()                                                                                          
    }
    if (!bit.options.keep) {
        bit.dir.pkg.join('bin').removeAll()
    } else {
        trace('Keep', bit.dir.pkg.join('bin'))
    }
    trace('Complete', bit.settings.title + ' installed')
}

public function uninstallBinary() {
    if (Config.OS != 'windows' && App.uid != 0) {
        throw 'Must run as root. Use \"sudo bit uninstall\"'
    }
    trace('Uninstall', bit.settings.title)                                                     
    let fileslog = bit.prefixes.productver.join('files.log')
    if (fileslog.exists) {
        for each (let file: Path in fileslog.readLines()) {
            vtrace('Remove', file)
            file.remove()
        }
    }
    fileslog.remove()
    for each (file in bit.prefixes.log.files('*.log*')) {
        file.remove()
    }
    for each (prefix in bit.prefixes) {
        for each (dir in prefix.files('**', {include: /\/$/}).sort().reverse()) {
            vtrace('Remove', dir)
            dir.remove()
        }
        vtrace('Remove', prefix)
        prefix.remove()
    }
    updateLatestLink()
}

/*
    Create symlinks for binaries and man pages
 */
public function createLinks() {
    let log = []
    let localbin = Path('/usr/local/bin')
    if (localbin.exists) {
        let programs = ['bit', 'ejs', 'ejsc', 'ejsmod', 'ejsman', 'http', 'jem', 'mvc', 'sqlite', 'utest' ]
        let bin = bit.prefixes.bin
        let target: Path
        for each (program in programs) {
            let link = Path(localbin.join(program))
            link.symlink(bin.join(program + bit.globals.EXE))
            log.push(link)
        }
        for each (page in bit.prefixes.productver.join('doc/man').files('**/*.1.gz')) {
            let link = Path('/usr/share/man/man1/' + page.basename)
            link.symlink(page)
            log.push(link)
        }
    }
    let link = Path('/usr/include').join(bit.settings.product)
    if (Config.OS != 'windows') {
        link.symlink(bit.prefixes.inc)
        log.push(link)
        bit.prefixes.productver.join('files.log').append(log.join('\n') + '\n')
    }
}

function updateLatestLink() {
    let latest = bit.prefixes.product.join('latest')
    let version = bit.prefixes.product.files('*', {include: /\d+\.\d+\.\d+/}).sort().pop()
    if (version) {
        latest.symlink(version.basename)
    } else {
        latest.remove()
    }
}

/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
  
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details.
  
    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://www.embedthis.com/downloads/gplLicense.html
  
    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com
  
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
