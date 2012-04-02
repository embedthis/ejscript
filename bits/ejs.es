/*
    Support functions for Embedthis Ejscript
    Exporting: packageBinaryFiles, createLinks, startAppweb

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

require ejs.tar
require ejs.unix

/*
    Copy binary files to package staging area
 */
public function packageBinaryFiles() {
    let settings = bit.settings
    let bin = bit.dir.pkg.join('bin')
    safeRemove(bit.dir.pkg)
    let vname = settings.product + '-' + settings.version + '-' + settings.buildNumber
    let pkg = bin.join(vname)
    pkg.makeDir()

    /* These three files are replicated outside the data directory */
    install('doc/product/README.TXT', pkg, {fold: true, expand: true})
    install('package/install.sh', pkg.join('install'), {permissions: 0755, expand: true})
    install('package/uninstall.sh', pkg.join('uninstall'), {permissions: 0755, expand: true})
    let contents = pkg.join('contents')

    let prefixes = bit.prefixes;
    let p = {}
    for (prefix in bit.prefixes) {
        p[prefix] = Path(contents.portable.name + bit.prefixes[prefix].removeDrive().portable)
        p[prefix].makeDir()
    }
    let strip = settings.profile == 'debug'

    install('LICENSE.md', p.product, {fold: true, expand: true})
    install('doc/product/README.TXT', p.product, {fold: true, expand: true})
    install('package/uninstall.sh', p.bin.join('uninstall'), {permissions: 0755, expand: true})
    install('package/linkup', p.bin, {permissions: 0755})

    install(bit.dir.bin + '/*', p.bin, {
        include: /bit|ejs|ejsc|ejsman|ejspage|http|jem|mvc|sqlite/,
        permissions: 0755, 
    })
    install(bit.dir.lib.join('*'), p.lib, {
        permissions: 0755, 
        exclude: /bits|file-save|www|simple|sample/,
    })
    install(bit.dir.lib.join('bits/**'), p.lib.join('bits'))
    install(bit.dir.lib.join('www/**'), p.lib.join('www'))

    if (bit.targets.libmprssl.enable && bit.platform.os == 'linux') {
        install(bit.dir.lib.join('*.' + bit.ext.shobj + '*'), p.lib, {strip: strip, permissions: 0755})
        for each (f in p.lib.glob('*.so.*')) {
            let withver = f.basename
            let nover = withver.name.replace(/\.[0-9]*.*/, '.so')
            let link = p.lib.join(nover)
            f.remove()
            f.symlink(link.basename)
        }
    }
    if (OS == 'win') {
        install(bit.packs.compiler.path.join('../../redist/x86/Microsoft.VC100.CRT/msvcr100.dll'), p.bin)
        /*
            install(bit.packs.compiler.path.join('../../lib/msvcrt.lib'), p.bin)
         */
        install(bit.dir.bin.join('removeFiles*'), bin)
    }
    if (bit.platform.like == 'posix') {
        install('doc/man/*.1', p.productver.join('doc/man/man1'), {compress: true})
    }
}

/*
    Create symlinks for binaries and man pages
 */
public function createLinks() {
    let programs = ['bit', 'ejs', 'ejsc', 'ejsmod', 'ejsman', 'ejspage', 'http', 'jem', 'mvc', 'sqlite' ]
    let localbin = Path('/usr/local/bin')
    let bin = bit.prefixes.bin
    let target: Path
    let log = []

    for each (program in programs) {
        let link = Path(localbin.join(program))
        link.symlink(bin.join(program))
        log.push(link)
    }
    for each (page in bit.prefixes.productver.join('doc/man').glob('**/*.1.gz')) {
        let link = Path('/usr/share/man/man1/' + page.basename)
        link.symlink(page)
        log.push(link)
    }
    bit.prefixes.productver.join('files.log').append(log.join('\n') + '\n')
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
