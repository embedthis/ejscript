/*
    Support functions for Embedthis products
    Exporting: install(), package()

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

require ejs.tar
require ejs.unix

public function getWebUser(): String {
    let passwdFile: Path = Path("/etc/passwd")
    if (passwdFile.exists) {
        let passwords = passwdFile.readString()
        for each (u in ["www-data", "_www", "nobody", "Administrator"]) {
            if (passwords.contains(u + ":")) {
                return u
            }
        }
    }
    return '0'
}

public function getWebGroup(): String {
    let groupFile: Path = Path("/etc/group")
    if (groupFile.exists) {
        let groups = groupFile.readString()
        for each (g in ["www-data", "_www", "nobody", "nogroup", "Administrator"]) {
            if (groups.contains(g + ":")) {
                return g
            }
        }
    }
    return '0'
}


/*
    Fold long lines at column 80. On windows, will also convert line terminatations to <CR><LF>.
 */
function foldLines(path: Path, options) {
    let lines = path.readLines()
    let out = new TextStream(new File(path, 'wt'))
    for (l = 0; l < lines.length; l++) {
        let line = lines[l]
        if (options.fold && line.length > 80) {
            for (i = 79; i >= 0; i--) {
                if (line[i] == ' ') {
                    lines[l] = line.slice(0, i)
                    lines.insert(l + 1, line.slice(i + 1))
                    break
                }
            }
            if (i == 0) {
                lines[l] = line.slice(0, 80)
                lines.insert(l + 1, line.slice(80))
            }
        }
        out.writeLine(lines[l])
    }
    out.close()
}

function installCallback(src: Path, dest: Path, options = {}): Boolean {
    options.task ||= 'install'

    src = src.relative.portable
    if (options.exclude && src.match(options.exclude)) {
        return false
    }
    if (options.copytemp && src.match(TempFilter)) {
        return false
    }
    if (options.include && !src.match(options.include)) {
        return false
    }
    if (options.task == 'uninstall') {
        if (options.compress) {
            dest = Path(dest.name + '.gz')
        }
        vtrace(options.task.toPascal(), dest.relative)
        dest.remove()
        return true
    }
    let attributes = { 
        uid: options.uid
        gid: options.gid
        user: options.user
        group: options.group
        permissions: options.permissions || 
            ((src.isDir || src.extension.match(/exe|lib|so|dylib|sh|es/)) ? 0755 : 0644)
    }
    if (options.cat) {
        vtrace('Combine', dest.relative + ' += ' + src.relative)
        if (!dest.exists) {
            if (options.title) {
                dest.write('/*\n' +
                           '    ' + dest.basename + ' -- ' + options.title + '\n\n' +
                           '    This file is a catenation of all the source code. Amalgamating into a\n' +
                           '    single file makes embedding simpler and the resulting application faster.\n */\n\n')
            }
            if (options.header) {
                dest.append(options.header + '\n')
            }
        }
        dest.append('\n' +
           '/************************************************************************/\n' +
           '/*\n    Start of file \"' + src.relative + '\"\n */\n' +
           '/************************************************************************/\n\n')
        let data = src.readString()
        if (options.filter) {
            data = data.replace(options.filter, '')
        }
        dest.append(data)
        dest.setAttributes(attributes)
    } else {
        vtrace(options.task.toPascal(), dest.relative)
        dest.parent.makeDir()
        if (src.isDir) {
            dest.makeDir()
            attributes.permissions = 0755
            dest.setAttributes(attributes)
        } else {
            src.copy(dest, attributes)
        }
    }
    if (options.expand) {
        vtrace('Patch', dest)
        let o = bit
        if (options.expand != true) {
            o = options.expand
        }
        dest.write(dest.readString().expand(o, {fill: '${}'}))
        dest.setAttributes(attributes)
    }
    if (options.fold && bit.platform.like == 'windows') {
        vtrace('Fold', dest)
        foldLines(dest, options)
        dest.setAttributes(attributes)
    }
    if (options.strip && bit.packs.strip) {
        vtrace('Strip', dest)
        Cmd.run(bit.packs.strip.path + ' ' + dest)
    }
    if (options.compress) {
        vtrace('Compress', dest.relative)
        let zname = Path(dest.name + '.gz')
        zname.remove()
        global.load('ejs.zlib.mod')
        use namespace 'ejs.zlib'
        global.Zlib.compress(dest.name, zname)
        dest.remove()
    }
    if (App.uid == 0 && dest.extension == 'so' && Config.OS == 'LINUX' && options.task == 'install') {
        trace('Ldconfig', dest)
        Cmd.run('ldconfig ' + dest)
    }
    return true
}

/*
    Install and uninstall files.
    If options.task is 'install' or 'package', the files are installed. If the options.task is 'uninstall', the 
        files are removed.
    @param src Source path. May contain glob style wild cards including '*', '?' and '**'. May also be an array
        of source paths.
    @param dest Destination path
    @param options Process options
    @option compress Compress target file
    @option copytemp Copy files that look like temp files
    @option exclude Exclude files that match the pattern. The pattern should be in portable file format.
    @option expand Expand tokens. Set to true or an Object hash containing properties to use when replacing 
        tokens of the form ${token} in the src and dest filenames. If set to true, the 'bit' object is used.
    @option fold Fold long lines on windows at column 80 and convert new line endings.
    @option group Set file group
    @option include Include files that match the pattern. The pattern should be in portable file format.
    @option user Set file file user
    @option perms Set file perms
    @option strip Strip object or executable
 */
public function install(src, dest: Path, options = {}) {
    if (!(src is Array)) src = [src]
    if (options.cat) {
        let files = []
        for each (pat in src) {
            files += Path('.').glob(pat)
        }
        src = files.unique()
    }
    cp(src, dest, blend({process: this.installCallback, warn: true}, options))
}

public function package(pkg: Path, formats) {
    /*
        Dynamically zlib load to avoid a hard dependency
     */
    global.load('ejs.zlib.mod')
    if (!(formats is Array)) formats = [formats]

    let options = {relativeTo: pkg, user: 'root', group: 'root', uid: 0, gid: 0}
    if (bit.platform.os == 'macosx') {
        options.group = 'wheel'
    }
    options.vname = bit.settings.product + '-' + bit.settings.version + '-' + bit.settings.buildNumber

    for each (fmt in formats) {
        switch (fmt) {
        case 'combo':
            packageCombo(pkg, options)
            break
        case 'flat':
            packageFlat(pkg, options)
            break
        case 'install':
            packageInstall(pkg, options)
            break
        case 'native':
            packageNative(pkg, options)
            break
        case 'src':
            packageSrc(pkg, options)
            break
        case 'tar':
            packageTar(pkg, options)
            break
        default:
            throw 'Unknown package format: ' + pkg
        }
    }
}

function packageSimple(pkg: Path, options, fmt) {
    let s = bit.settings
    let rel = bit.dir.rel
    let name = rel.join(options.vname + '-' + fmt + '.tar')
    let zname = name.replaceExt('tgz')

    trace('Package', zname)
    let tar = new Tar(name, options)
    tar.create(pkg.glob('**', {exclude: /\/$/}))
    use namespace 'ejs.zlib'
    global.Zlib.compress(tar.name, zname)
    name.remove()
    let generic = rel.join(s.product + '-' + fmt + '.tgz')
    generic.remove()
    Path(generic).symlink(zname)
    rel.join('md5-' + options.vname + '-' + fmt + '.tar.txt').write(md5(zname.readString()))
}

function packageFlat(pkg: Path, options) {
    let s = bit.settings
    let flat: Path = bit.dir.flat
    safeRemove(flat)
    let vflat = flat.join(options.vname)
    vflat.makeDir()
    for each (f in pkg.glob('**', {exclude: /\/$/})) {
        f.copy(vflat.join(f.basename))
    }
    packageSimple(flat, options, 'flat')
}

function packageCombo(pkg: Path, options) {
    packageSimple(pkg, options, 'combo')
}

function packageSrc(pkg: Path, options) {
    packageSimple(pkg, options, 'src')
}

function packageTar(pkg: Path, options) {
    let s = bit.settings
    let rel = bit.dir.rel
    let base = [s.product, s.version, s.buildNumber, bit.platform.dist, OS.toUpper(), ARCH].join('-')
    let name = rel.join(base).joinExt('tar', true)
    let zname = name.replaceExt('tgz')
    let files = pkg.glob('**', {exclude: /\/$/})
    let tar = new Tar(name, options)

    trace('Package', zname)
    tar.create(files)

    use namespace 'ejs.zlib'
    global.Zlib.compress(name, zname)
    name.remove()
    rel.join('md5-' + base).joinExt('tar.txt', true).write(md5(zname.readString()))
    let generic = rel.join(s.product + '-tar' + '.tgz')
    generic.remove()
    Path(generic).symlink(zname)
}

function packageInstall(pkg: Path, options) {
    if (App.uid != 0) {
        throw 'Must run as root. Use "sudo bit install"'
    }
    let s = bit.settings
    let rel = bit.dir.rel
    let base = [s.product, s.version, s.buildNumber, bit.platform.dist, OS.toUpper(), ARCH].join('-')
    let name = rel.join(base).joinExt('tar', true)
    let contents = pkg.join(options.vname, 'contents')
    let files = contents.glob('**')
    let log = []
    for each (file in files) {
        let target = Path('/' + file.relativeTo(contents))
        if (file.isDir) {
            target.makeDir()
        } else {
            file.copy(target)
            log.push(target)
        }
    }
    name.remove()
    bit.prefixes.productver.join('files.log').write(log.join('\n') + '\n')
    packageInstallConfigure()
}

function packageInstallConfigure() {
    let ldconfigSwitch = (OS == 'freebsd') ? '-m' : '-n'
    let ldconfig = Cmd.locate('ldconfig')
    if (ldconfig) {
        // Cmd.run(ldconfig + ' /usr/lib/lib${PRODUCT}.so.?.?.?
        Cmd.run(ldconfig + ' ' + ldconfigSwitch + ' /usr/lib/' + bit.settings.product)
        Cmd.run(ldconfig + ' ' + ldconfigSwitch + ' /usr/lib/' + bit.settings.product + '/modules')
    }
    if (bit.platform.dist == 'fedora') {
        Cmd.run('chcon /usr/bin/chcon -t texrel_shlib_t ' + bit.prefixes.lib + '/*.so')
    }
}

function packageNative(pkg: Path, options) {
    switch (bit.platform.os) {
    case 'linux':
        if (bit.platform.dist == 'ubuntu') {
            packageUbuntu(pkg, options)
        } else {
            throw 'Can\'t package for linux distribution ' + bit.platform.dist
        }
        break
    case 'macosx':
        packageMacosx(pkg, options)
        break
    case 'win':
        packageWin(pkg, options)
        break
    default:
        throw 'Can\'t package for ' + bit.platform.os
    }
}

var staffDir = {
    'var/www': true,
}

function createMacContents(pkg: Path, options) {
    let s = bit.settings
    let contents = pkg.join(options.vname, 'contents')
    let cp: File = pkg.join(s.product + '.pmdoc', '01contents-contents.xml').open('w')
    cp.write('<pkg-contents spec="1.12">')
    cp.write('<f n="contents" o="root" g="wheel" p="16877" pt="' + contents + '" m="false" t="file">')
    options.staff = staffDir
    for each (dir in contents.glob('*', {include: /\/$/})) {
        inner(pkg, options, cp, dir)
    }

    function inner(pkg: Path, options, cp: File, dir: Path) {
        let perms = dir.attributes.permissions cast Number
        let contents = pkg.join(options.vname, 'contents')
        cp.write('<f n="' + dir.basename + '" o="root" g="wheel" p="' + perms + '" />')
        for each (f in dir.files()) {
            if (f.isDir) {
                inner(pkg, options, cp, f)
            } else {
                perms = f.attributes.permissions cast Number
                cp.write('<f n="' + f.basename + '" o="root" g="wheel" p="' + perms + '" />')
            }
        }
        cp.write('</f>')
    }
    cp.write('</pkg-contents>\n')
    cp.close()
}

function packageMacosx(pkg: Path, options) {
    if (!bit.packs.pmaker || !bit.packs.pmaker.path) {
        throw 'Configured without PackageMaker'
    }
    let s = bit.settings
    let rel = bit.dir.rel
    let base = [s.product, s.version, s.buildNumber, bit.platform.dist, OS.toUpper(), ARCH].join('-')
    let name = rel.join(base).joinExt('tar', true)
    let files = pkg.glob('**', {exclude: /\/$/})
    let size = 20
    for each (file in pkg.glob('**', {exclude: /\/$/})) {
        size += ((file.size + 999) / 1000)
    }
    bit.PACKAGE_SIZE = size
    let pm = s.product + '.pmdoc'
    let pmdoc = pkg.join(pm)
    let opak = Path('package/' + OS.toUpper())
    install(opak.join('background.png'), pkg)
    install(opak.join('license.rtf'), pkg)
    install(opak.join('readme.rtf'), pkg)
    install(opak.join(pm + '/*'), pmdoc, {expand: true, hidden: true})
    let scripts = pkg.join('scripts')
    scripts.makeDir()
    install('package/' + OS.toUpper() + '/scripts/*', scripts, {expand: true})
    createMacContents(pkg, options)

    /* Remove extended attributes */
    Cmd.sh("cd " + pkg + "; for i in $(ls -Rl@ | grep '^    ' | awk '{print $1}' | sort -u); do \
        find . | xargs xattr -d $i 2>/dev/null ; done")

    let outfile = bit.dir.rel.join(base).joinExt('pkg', true)
    trace('Package', outfile)
    run(bit.packs.pmaker.path + ' --target 10.5 --domain system --doc ' + pmdoc + 
        ' --id com.embedthis.' + s.product + '.bin.pkg --root-volume-only --no-relocate' +
        ' --discard-forks --out ' + outfile)
    bit.dir.rel.join('md5-' + base).joinExt('pkg.txt', true).write(md5(outfile.readString()))
}

function packageUbuntu(pkg: Path, options) {
    if (!bit.packs.dpkg || !bit.packs.dpkg.path) {
        throw 'Configured without dpkg'
    }
    let s = bit.settings
    let rel = bit.dir.rel
    let cpu = bit.platform.arch
    if (cpu == 'i686') {
        cpu = 'i386'
    } else if (cpu == 'x86_64') {
        cpu = 'amd64'
    }
    bit.platform.mappedCpu = cpu
    let contents = pkg.join(options.vname, 'contents')
    let DEBIAN = contents.join('DEBIAN')
    let opak = Path('package/' + OS.toUpper())

    install(opak.join('deb.bin/c*'), DEBIAN, {expand: true, permissions: 0644})
    install(opak.join('deb.bin/p*'), DEBIAN, {expand: true, permissions: 0755})

    let base = [s.product, s.version, s.buildNumber, bit.platform.dist, OS.toUpper(), ARCH].join('-')
    let outfile = bit.dir.rel.join(base).joinExt('deb', true)
    trace('Package', outfile)
    run(bit.packs.dpkg.path + ' --build ' + DEBIAN.dirname + ' ' + outfile)
    bit.dir.rel.join('md5-' + base).joinExt('deb.txt', true).write(md5(outfile.readString()))
}

function packageWin(pkg: Path, options) {
    if (!bit.packs.inno || !bit.packs.inno.path) {
        throw 'Configured without Inno Setup'
    }
    let s = bit.settings
    let rel = bit.dir.rel
    let opak = Path('package/' + OS.toUpper())

    install(opak.join('LICENSE.TXT'), pkg)
    let iss = pkg.join('install.iss')
    install(opak.join('install.iss'), iss, {expand: true})
    let cp: File = iss.open('atw')
    let files = pkg.glob('**', {exclude: /\/$/})
    for each (file in files) {
        let src = file.relativeTo(pkg)
        let dest = bit.prefixes.product.join(file)
        let dir = src.dirname
        cp.write('Source: "' + src + '"; DestDir: "{app}\\' + dir + '"; ' +
            'DestName: "' + src.basename + '"; Components: bin\n')

    }
    cp.close()
    let base = [s.product, s.version, s.buildNumber, bit.platform.dist, OS.toUpper(), ARCH].join('-')
    let outfile = bit.dir.rel.join(base).joinExt('exe', true)
    trace('Package', outfile)
    run([bit.packs.inno.path, iss])
    pkg.join('Output/setup.exe').copy(outfile)
    bit.dir.rel.join('md5-' + base).joinExt('exe.txt', true).write(md5(outfile.readString()))
}

public function syncup(from: Path, to: Path) {
    let tartemp: Path
    if (from.name.endsWith('.tgz') || from.name.endsWith('.gz')) {
        use namespace 'ejs.zlib'
        global.load('ejs.zlib.mod')
        global.Zlib.uncompress(from, from.replaceExt('tartemp'))
        from = from.replaceExt('tartemp')
        tartemp = from
    }
    let tar = new Tar(from)
    for each (item in tar.list()) {
        let path = to.join(item.components.slice(1).join('/'))
        let fromData = tar.readString(item)
        let toData = path.readString()
        if (fromData != toData) {
            let modified = tar.info(item)[0].modified
            if (modified <= path.modified) {
                if (!bit.options.force) {
                    trace('WARNING', path.relative + ' has been modified. Update skipped for this file.')
                    continue
                }
                trace('Update', 'Force update of ' + path)
            } else {
                trace('Update', path)
            }
            path.write(fromData)
        }
    }
    if (tartemp) {
        tartemp.remove()
    }
}

/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
  
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
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
