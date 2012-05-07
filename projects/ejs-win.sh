#
#   ejs-win.sh -- Build It Shell Script to build Embedthis Ejscript
#

export PATH="$(SDK)/Bin:$(VS)/VC/Bin:$(VS)/Common7/IDE:$(VS)/Common7/Tools:$(VS)/SDK/v3.5/bin:$(VS)/VC/VCPackages;$(PATH)"
export INCLUDE="$(INCLUDE);$(SDK)/INCLUDE:$(VS)/VC/INCLUDE"
export LIB="$(LIB);$(SDK)/lib:$(VS)/VC/lib"

ARCH="x86"
ARCH="$(shell uname -m | sed 's/i.86/x86/')"
OS="win"
PROFILE="debug"
CONFIG="${OS}-${ARCH}-${PROFILE}"
CC="cl.exe"
LD="link"
CFLAGS="-nologo -GR- -W3 -Zi -Od -MDd"
DFLAGS="-D_REENTRANT -D_MT -DBLD_DEBUG"
IFLAGS="-I${CONFIG}/inc"
LDFLAGS="-nologo -nodefaultlib -incremental:no -debug -machine:x86"
LIBPATHS="-libpath:${CONFIG}/bin"
LIBS="ws2_32.lib advapi32.lib user32.lib kernel32.lib oldnames.lib msvcrt.lib shell32.lib"

[ ! -x ${CONFIG}/inc ] && mkdir -p ${CONFIG}/inc ${CONFIG}/obj ${CONFIG}/lib ${CONFIG}/bin

[ ! -f ${CONFIG}/inc/bit.h ] && cp projects/ejs-${OS}-bit.h ${CONFIG}/inc/bit.h
if ! diff ${CONFIG}/inc/bit.h projects/ejs-${OS}-bit.h >/dev/null ; then
	cp projects/ejs-${OS}-bit.h ${CONFIG}/inc/bit.h
fi

rm -rf ${CONFIG}/inc/mpr.h
cp -r src/deps/mpr/mpr.h ${CONFIG}/inc/mpr.h

rm -rf ${CONFIG}/inc/mprSsl.h
cp -r src/deps/mpr/mprSsl.h ${CONFIG}/inc/mprSsl.h

"${CC}" -c -Fo${CONFIG}/obj/mprLib.obj -Fd${CONFIG}/obj/mprLib.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprLib.c

"${LD}" -dll -out:${CONFIG}/bin/libmpr.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libmpr.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/mprLib.obj ${LIBS}

"${CC}" -c -Fo${CONFIG}/obj/mprSsl.obj -Fd${CONFIG}/obj/mprSsl.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprSsl.c

"${LD}" -dll -out:${CONFIG}/bin/libmprssl.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libmprssl.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/mprSsl.obj ${LIBS} libmpr.lib

"${CC}" -c -Fo${CONFIG}/obj/manager.obj -Fd${CONFIG}/obj/manager.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/manager.c

"${LD}" -out:${CONFIG}/bin/ejsman.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/manager.obj ${LIBS} libmpr.lib

"${CC}" -c -Fo${CONFIG}/obj/makerom.obj -Fd${CONFIG}/obj/makerom.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/makerom.c

"${LD}" -out:${CONFIG}/bin/makerom.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/makerom.obj ${LIBS} libmpr.lib

rm -rf ${CONFIG}/inc/pcre.h
cp -r src/deps/pcre/pcre.h ${CONFIG}/inc/pcre.h

"${CC}" -c -Fo${CONFIG}/obj/pcre.obj -Fd${CONFIG}/obj/pcre.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/pcre/pcre.c

"${LD}" -dll -out:${CONFIG}/bin/libpcre.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libpcre.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/pcre.obj ${LIBS}

rm -rf ${CONFIG}/inc/http.h
cp -r src/deps/http/http.h ${CONFIG}/inc/http.h

"${CC}" -c -Fo${CONFIG}/obj/httpLib.obj -Fd${CONFIG}/obj/httpLib.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/httpLib.c

"${LD}" -dll -out:${CONFIG}/bin/libhttp.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libhttp.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/httpLib.obj ${LIBS} libmpr.lib libpcre.lib libmprssl.lib

"${CC}" -c -Fo${CONFIG}/obj/http.obj -Fd${CONFIG}/obj/http.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/http.c

"${LD}" -out:${CONFIG}/bin/http.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/http.obj ${LIBS} libhttp.lib libmpr.lib libpcre.lib libmprssl.lib

rm -rf ${CONFIG}/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h ${CONFIG}/inc/sqlite3.h

"${CC}" -c -Fo${CONFIG}/obj/sqlite3.obj -Fd${CONFIG}/obj/sqlite3.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite3.c

"${LD}" -dll -out:${CONFIG}/bin/libsqlite3.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libsqlite3.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite3.obj ${LIBS}

"${CC}" -c -Fo${CONFIG}/obj/sqlite.obj -Fd${CONFIG}/obj/sqlite.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite.c

"${LD}" -out:${CONFIG}/bin/sqlite.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite.obj ${LIBS} libsqlite3.lib

rm -rf ${CONFIG}/inc/ejs.cache.local.slots.h
cp -r src/slots/ejs.cache.local.slots.h ${CONFIG}/inc/ejs.cache.local.slots.h

rm -rf ${CONFIG}/inc/ejs.db.sqlite.slots.h
cp -r src/slots/ejs.db.sqlite.slots.h ${CONFIG}/inc/ejs.db.sqlite.slots.h

rm -rf ${CONFIG}/inc/ejs.slots.h
cp -r src/slots/ejs.slots.h ${CONFIG}/inc/ejs.slots.h

rm -rf ${CONFIG}/inc/ejs.web.slots.h
cp -r src/slots/ejs.web.slots.h ${CONFIG}/inc/ejs.web.slots.h

rm -rf ${CONFIG}/inc/ejs.zlib.slots.h
cp -r src/slots/ejs.zlib.slots.h ${CONFIG}/inc/ejs.zlib.slots.h

rm -rf ${CONFIG}/inc/ejs.h
cp -r src/ejs.h ${CONFIG}/inc/ejs.h

rm -rf ${CONFIG}/inc/ejsByteCode.h
cp -r src/ejsByteCode.h ${CONFIG}/inc/ejsByteCode.h

rm -rf ${CONFIG}/inc/ejsByteCodeTable.h
cp -r src/ejsByteCodeTable.h ${CONFIG}/inc/ejsByteCodeTable.h

rm -rf ${CONFIG}/inc/ejsCompiler.h
cp -r src/ejsCompiler.h ${CONFIG}/inc/ejsCompiler.h

rm -rf ${CONFIG}/inc/ejsCustomize.h
cp -r src/ejsCustomize.h ${CONFIG}/inc/ejsCustomize.h

"${CC}" -c -Fo${CONFIG}/obj/ecAst.obj -Fd${CONFIG}/obj/ecAst.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecAst.c

"${CC}" -c -Fo${CONFIG}/obj/ecCodeGen.obj -Fd${CONFIG}/obj/ecCodeGen.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecCodeGen.c

"${CC}" -c -Fo${CONFIG}/obj/ecCompiler.obj -Fd${CONFIG}/obj/ecCompiler.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecCompiler.c

"${CC}" -c -Fo${CONFIG}/obj/ecLex.obj -Fd${CONFIG}/obj/ecLex.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecLex.c

"${CC}" -c -Fo${CONFIG}/obj/ecModuleWrite.obj -Fd${CONFIG}/obj/ecModuleWrite.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecModuleWrite.c

"${CC}" -c -Fo${CONFIG}/obj/ecParser.obj -Fd${CONFIG}/obj/ecParser.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecParser.c

"${CC}" -c -Fo${CONFIG}/obj/ecState.obj -Fd${CONFIG}/obj/ecState.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecState.c

"${CC}" -c -Fo${CONFIG}/obj/ejsApp.obj -Fd${CONFIG}/obj/ejsApp.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsApp.c

"${CC}" -c -Fo${CONFIG}/obj/ejsArray.obj -Fd${CONFIG}/obj/ejsArray.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsArray.c

"${CC}" -c -Fo${CONFIG}/obj/ejsBlock.obj -Fd${CONFIG}/obj/ejsBlock.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsBlock.c

"${CC}" -c -Fo${CONFIG}/obj/ejsBoolean.obj -Fd${CONFIG}/obj/ejsBoolean.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsBoolean.c

"${CC}" -c -Fo${CONFIG}/obj/ejsByteArray.obj -Fd${CONFIG}/obj/ejsByteArray.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsByteArray.c

"${CC}" -c -Fo${CONFIG}/obj/ejsCache.obj -Fd${CONFIG}/obj/ejsCache.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsCache.c

"${CC}" -c -Fo${CONFIG}/obj/ejsCmd.obj -Fd${CONFIG}/obj/ejsCmd.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsCmd.c

"${CC}" -c -Fo${CONFIG}/obj/ejsConfig.obj -Fd${CONFIG}/obj/ejsConfig.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsConfig.c

"${CC}" -c -Fo${CONFIG}/obj/ejsDate.obj -Fd${CONFIG}/obj/ejsDate.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsDate.c

"${CC}" -c -Fo${CONFIG}/obj/ejsDebug.obj -Fd${CONFIG}/obj/ejsDebug.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsDebug.c

"${CC}" -c -Fo${CONFIG}/obj/ejsError.obj -Fd${CONFIG}/obj/ejsError.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsError.c

"${CC}" -c -Fo${CONFIG}/obj/ejsFile.obj -Fd${CONFIG}/obj/ejsFile.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsFile.c

"${CC}" -c -Fo${CONFIG}/obj/ejsFileSystem.obj -Fd${CONFIG}/obj/ejsFileSystem.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsFileSystem.c

"${CC}" -c -Fo${CONFIG}/obj/ejsFrame.obj -Fd${CONFIG}/obj/ejsFrame.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsFrame.c

"${CC}" -c -Fo${CONFIG}/obj/ejsFunction.obj -Fd${CONFIG}/obj/ejsFunction.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsFunction.c

"${CC}" -c -Fo${CONFIG}/obj/ejsGC.obj -Fd${CONFIG}/obj/ejsGC.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsGC.c

"${CC}" -c -Fo${CONFIG}/obj/ejsGlobal.obj -Fd${CONFIG}/obj/ejsGlobal.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsGlobal.c

"${CC}" -c -Fo${CONFIG}/obj/ejsHttp.obj -Fd${CONFIG}/obj/ejsHttp.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsHttp.c

"${CC}" -c -Fo${CONFIG}/obj/ejsIterator.obj -Fd${CONFIG}/obj/ejsIterator.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsIterator.c

"${CC}" -c -Fo${CONFIG}/obj/ejsJSON.obj -Fd${CONFIG}/obj/ejsJSON.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsJSON.c

"${CC}" -c -Fo${CONFIG}/obj/ejsLocalCache.obj -Fd${CONFIG}/obj/ejsLocalCache.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsLocalCache.c

"${CC}" -c -Fo${CONFIG}/obj/ejsMath.obj -Fd${CONFIG}/obj/ejsMath.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsMath.c

"${CC}" -c -Fo${CONFIG}/obj/ejsMemory.obj -Fd${CONFIG}/obj/ejsMemory.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsMemory.c

"${CC}" -c -Fo${CONFIG}/obj/ejsMprLog.obj -Fd${CONFIG}/obj/ejsMprLog.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsMprLog.c

"${CC}" -c -Fo${CONFIG}/obj/ejsNamespace.obj -Fd${CONFIG}/obj/ejsNamespace.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsNamespace.c

"${CC}" -c -Fo${CONFIG}/obj/ejsNull.obj -Fd${CONFIG}/obj/ejsNull.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsNull.c

"${CC}" -c -Fo${CONFIG}/obj/ejsNumber.obj -Fd${CONFIG}/obj/ejsNumber.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsNumber.c

"${CC}" -c -Fo${CONFIG}/obj/ejsObject.obj -Fd${CONFIG}/obj/ejsObject.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsObject.c

"${CC}" -c -Fo${CONFIG}/obj/ejsPath.obj -Fd${CONFIG}/obj/ejsPath.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsPath.c

"${CC}" -c -Fo${CONFIG}/obj/ejsPot.obj -Fd${CONFIG}/obj/ejsPot.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsPot.c

"${CC}" -c -Fo${CONFIG}/obj/ejsRegExp.obj -Fd${CONFIG}/obj/ejsRegExp.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsRegExp.c

"${CC}" -c -Fo${CONFIG}/obj/ejsSocket.obj -Fd${CONFIG}/obj/ejsSocket.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsSocket.c

"${CC}" -c -Fo${CONFIG}/obj/ejsString.obj -Fd${CONFIG}/obj/ejsString.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsString.c

"${CC}" -c -Fo${CONFIG}/obj/ejsSystem.obj -Fd${CONFIG}/obj/ejsSystem.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsSystem.c

"${CC}" -c -Fo${CONFIG}/obj/ejsTimer.obj -Fd${CONFIG}/obj/ejsTimer.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsTimer.c

"${CC}" -c -Fo${CONFIG}/obj/ejsType.obj -Fd${CONFIG}/obj/ejsType.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsType.c

"${CC}" -c -Fo${CONFIG}/obj/ejsUri.obj -Fd${CONFIG}/obj/ejsUri.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsUri.c

"${CC}" -c -Fo${CONFIG}/obj/ejsVoid.obj -Fd${CONFIG}/obj/ejsVoid.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsVoid.c

"${CC}" -c -Fo${CONFIG}/obj/ejsWorker.obj -Fd${CONFIG}/obj/ejsWorker.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsWorker.c

"${CC}" -c -Fo${CONFIG}/obj/ejsXML.obj -Fd${CONFIG}/obj/ejsXML.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsXML.c

"${CC}" -c -Fo${CONFIG}/obj/ejsXMLList.obj -Fd${CONFIG}/obj/ejsXMLList.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsXMLList.c

"${CC}" -c -Fo${CONFIG}/obj/ejsXMLLoader.obj -Fd${CONFIG}/obj/ejsXMLLoader.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsXMLLoader.c

"${CC}" -c -Fo${CONFIG}/obj/ejsByteCode.obj -Fd${CONFIG}/obj/ejsByteCode.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsByteCode.c

"${CC}" -c -Fo${CONFIG}/obj/ejsException.obj -Fd${CONFIG}/obj/ejsException.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsException.c

"${CC}" -c -Fo${CONFIG}/obj/ejsHelper.obj -Fd${CONFIG}/obj/ejsHelper.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsHelper.c

"${CC}" -c -Fo${CONFIG}/obj/ejsInterp.obj -Fd${CONFIG}/obj/ejsInterp.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsInterp.c

"${CC}" -c -Fo${CONFIG}/obj/ejsLoader.obj -Fd${CONFIG}/obj/ejsLoader.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsLoader.c

"${CC}" -c -Fo${CONFIG}/obj/ejsModule.obj -Fd${CONFIG}/obj/ejsModule.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsModule.c

"${CC}" -c -Fo${CONFIG}/obj/ejsScope.obj -Fd${CONFIG}/obj/ejsScope.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsScope.c

"${CC}" -c -Fo${CONFIG}/obj/ejsService.obj -Fd${CONFIG}/obj/ejsService.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsService.c

"${LD}" -dll -out:${CONFIG}/bin/libejs.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libejs.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ecAst.obj ${CONFIG}/obj/ecCodeGen.obj ${CONFIG}/obj/ecCompiler.obj ${CONFIG}/obj/ecLex.obj ${CONFIG}/obj/ecModuleWrite.obj ${CONFIG}/obj/ecParser.obj ${CONFIG}/obj/ecState.obj ${CONFIG}/obj/ejsApp.obj ${CONFIG}/obj/ejsArray.obj ${CONFIG}/obj/ejsBlock.obj ${CONFIG}/obj/ejsBoolean.obj ${CONFIG}/obj/ejsByteArray.obj ${CONFIG}/obj/ejsCache.obj ${CONFIG}/obj/ejsCmd.obj ${CONFIG}/obj/ejsConfig.obj ${CONFIG}/obj/ejsDate.obj ${CONFIG}/obj/ejsDebug.obj ${CONFIG}/obj/ejsError.obj ${CONFIG}/obj/ejsFile.obj ${CONFIG}/obj/ejsFileSystem.obj ${CONFIG}/obj/ejsFrame.obj ${CONFIG}/obj/ejsFunction.obj ${CONFIG}/obj/ejsGC.obj ${CONFIG}/obj/ejsGlobal.obj ${CONFIG}/obj/ejsHttp.obj ${CONFIG}/obj/ejsIterator.obj ${CONFIG}/obj/ejsJSON.obj ${CONFIG}/obj/ejsLocalCache.obj ${CONFIG}/obj/ejsMath.obj ${CONFIG}/obj/ejsMemory.obj ${CONFIG}/obj/ejsMprLog.obj ${CONFIG}/obj/ejsNamespace.obj ${CONFIG}/obj/ejsNull.obj ${CONFIG}/obj/ejsNumber.obj ${CONFIG}/obj/ejsObject.obj ${CONFIG}/obj/ejsPath.obj ${CONFIG}/obj/ejsPot.obj ${CONFIG}/obj/ejsRegExp.obj ${CONFIG}/obj/ejsSocket.obj ${CONFIG}/obj/ejsString.obj ${CONFIG}/obj/ejsSystem.obj ${CONFIG}/obj/ejsTimer.obj ${CONFIG}/obj/ejsType.obj ${CONFIG}/obj/ejsUri.obj ${CONFIG}/obj/ejsVoid.obj ${CONFIG}/obj/ejsWorker.obj ${CONFIG}/obj/ejsXML.obj ${CONFIG}/obj/ejsXMLList.obj ${CONFIG}/obj/ejsXMLLoader.obj ${CONFIG}/obj/ejsByteCode.obj ${CONFIG}/obj/ejsException.obj ${CONFIG}/obj/ejsHelper.obj ${CONFIG}/obj/ejsInterp.obj ${CONFIG}/obj/ejsLoader.obj ${CONFIG}/obj/ejsModule.obj ${CONFIG}/obj/ejsScope.obj ${CONFIG}/obj/ejsService.obj ${LIBS} libhttp.lib libmpr.lib libpcre.lib libmprssl.lib

"${CC}" -c -Fo${CONFIG}/obj/ejs.obj -Fd${CONFIG}/obj/ejs.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/cmd/ejs.c

"${LD}" -out:${CONFIG}/bin/ejs.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejs.obj ${LIBS} libejs.lib libhttp.lib libmpr.lib libpcre.lib libmprssl.lib

"${CC}" -c -Fo${CONFIG}/obj/ejsc.obj -Fd${CONFIG}/obj/ejsc.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/cmd/ejsc.c

"${LD}" -out:${CONFIG}/bin/ejsc.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsc.obj ${LIBS} libejs.lib libhttp.lib libmpr.lib libpcre.lib libmprssl.lib

"${CC}" -c -Fo${CONFIG}/obj/ejsmod.obj -Fd${CONFIG}/obj/ejsmod.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/cmd src/cmd/ejsmod.c

"${CC}" -c -Fo${CONFIG}/obj/doc.obj -Fd${CONFIG}/obj/doc.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/cmd src/cmd/doc.c

"${CC}" -c -Fo${CONFIG}/obj/docFiles.obj -Fd${CONFIG}/obj/docFiles.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/cmd src/cmd/docFiles.c

"${CC}" -c -Fo${CONFIG}/obj/listing.obj -Fd${CONFIG}/obj/listing.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/cmd src/cmd/listing.c

"${CC}" -c -Fo${CONFIG}/obj/slotGen.obj -Fd${CONFIG}/obj/slotGen.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/cmd src/cmd/slotGen.c

"${LD}" -out:${CONFIG}/bin/ejsmod.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsmod.obj ${CONFIG}/obj/doc.obj ${CONFIG}/obj/docFiles.obj ${CONFIG}/obj/listing.obj ${CONFIG}/obj/slotGen.obj ${LIBS} libejs.lib libhttp.lib libmpr.lib libpcre.lib libmprssl.lib

"${CC}" -c -Fo${CONFIG}/obj/ejsrun.obj -Fd${CONFIG}/obj/ejsrun.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/cmd/ejsrun.c

"${LD}" -out:${CONFIG}/bin/ejsrun.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsrun.obj ${LIBS} libejs.lib libhttp.lib libmpr.lib libpcre.lib libmprssl.lib

cd src/core >/dev/null ;\
../../${CONFIG}/bin/ejsc --out ../../${CONFIG}/bin/ejs.mod --debug --optimize 9 --bind --require null *.es  ;\
../../${CONFIG}/bin/ejsmod --require null --cslots ../../${CONFIG}/bin/ejs.mod ;\
if ! diff ejs.slots.h ../../${CONFIG}/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ../../${CONFIG}/inc; fi ;\
rm -f ejs.slots.h ;\
cd - >/dev/null 

rm -rf ${CONFIG}/bin/bit.es
cp -r src/jems/ejs.bit/bit.es ${CONFIG}/bin/bit.es

cd src/jems/ejs.bit >/dev/null ;\
rm -fr ../../../${CONFIG}/bin/bits ;\
cp -r bits ../../../${CONFIG}/bin ;\
cd - >/dev/null 

"${CC}" -c -Fo${CONFIG}/obj/ejsZlib.obj -Fd${CONFIG}/obj/ejsZlib.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/jems/ejs.zlib/ejsZlib.c

"${LD}" -out:${CONFIG}/bin/bit.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsrun.obj ${CONFIG}/obj/ejsZlib.obj ${CONFIG}/obj/mprLib.obj ${CONFIG}/obj/pcre.obj ${CONFIG}/obj/mprSsl.obj ${CONFIG}/obj/httpLib.obj ${CONFIG}/obj/ecAst.obj ${CONFIG}/obj/ecCodeGen.obj ${CONFIG}/obj/ecCompiler.obj ${CONFIG}/obj/ecLex.obj ${CONFIG}/obj/ecModuleWrite.obj ${CONFIG}/obj/ecParser.obj ${CONFIG}/obj/ecState.obj ${CONFIG}/obj/ejsApp.obj ${CONFIG}/obj/ejsArray.obj ${CONFIG}/obj/ejsBlock.obj ${CONFIG}/obj/ejsBoolean.obj ${CONFIG}/obj/ejsByteArray.obj ${CONFIG}/obj/ejsCache.obj ${CONFIG}/obj/ejsCmd.obj ${CONFIG}/obj/ejsConfig.obj ${CONFIG}/obj/ejsDate.obj ${CONFIG}/obj/ejsDebug.obj ${CONFIG}/obj/ejsError.obj ${CONFIG}/obj/ejsFile.obj ${CONFIG}/obj/ejsFileSystem.obj ${CONFIG}/obj/ejsFrame.obj ${CONFIG}/obj/ejsFunction.obj ${CONFIG}/obj/ejsGC.obj ${CONFIG}/obj/ejsGlobal.obj ${CONFIG}/obj/ejsHttp.obj ${CONFIG}/obj/ejsIterator.obj ${CONFIG}/obj/ejsJSON.obj ${CONFIG}/obj/ejsLocalCache.obj ${CONFIG}/obj/ejsMath.obj ${CONFIG}/obj/ejsMemory.obj ${CONFIG}/obj/ejsMprLog.obj ${CONFIG}/obj/ejsNamespace.obj ${CONFIG}/obj/ejsNull.obj ${CONFIG}/obj/ejsNumber.obj ${CONFIG}/obj/ejsObject.obj ${CONFIG}/obj/ejsPath.obj ${CONFIG}/obj/ejsPot.obj ${CONFIG}/obj/ejsRegExp.obj ${CONFIG}/obj/ejsSocket.obj ${CONFIG}/obj/ejsString.obj ${CONFIG}/obj/ejsSystem.obj ${CONFIG}/obj/ejsTimer.obj ${CONFIG}/obj/ejsType.obj ${CONFIG}/obj/ejsUri.obj ${CONFIG}/obj/ejsVoid.obj ${CONFIG}/obj/ejsWorker.obj ${CONFIG}/obj/ejsXML.obj ${CONFIG}/obj/ejsXMLList.obj ${CONFIG}/obj/ejsXMLLoader.obj ${CONFIG}/obj/ejsByteCode.obj ${CONFIG}/obj/ejsException.obj ${CONFIG}/obj/ejsHelper.obj ${CONFIG}/obj/ejsInterp.obj ${CONFIG}/obj/ejsLoader.obj ${CONFIG}/obj/ejsModule.obj ${CONFIG}/obj/ejsScope.obj ${CONFIG}/obj/ejsService.obj ${LIBS}

#  Omit build script /Users/mob/git/ejs/win-x86-debug/bin/utest.es
rm -rf ${CONFIG}/bin/utest.exe
cp -r ${CONFIG}/bin/ejsrun.exe ${CONFIG}/bin/utest.exe

#  Omit build script /Users/mob/git/ejs/win-x86-debug/bin/ejs.unix.mod
#  Omit build script /Users/mob/git/ejs/win-x86-debug/bin/jem.es
rm -rf ${CONFIG}/bin/jem.exe
cp -r ${CONFIG}/bin/ejsrun.exe ${CONFIG}/bin/jem.exe

#  Omit build script /Users/mob/git/ejs/win-x86-debug/bin/ejs.db.mod
#  Omit build script /Users/mob/git/ejs/win-x86-debug/bin/ejs.db.mapper.mod
#  Omit build script /Users/mob/git/ejs/win-x86-debug/bin/ejs.db.sqlite.mod
"${CC}" -c -Fo${CONFIG}/obj/ejsSqlite.obj -Fd${CONFIG}/obj/ejsSqlite.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/jems/ejs.db.sqlite/ejsSqlite.c

"${LD}" -dll -out:${CONFIG}/bin/ejs.db.sqlite.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/ejs.db.sqlite.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsSqlite.obj ${LIBS} libmpr.lib libejs.lib libhttp.lib libpcre.lib libmprssl.lib libsqlite3.lib

cd src/jems/ejs.web >/dev/null ;\
../../../${CONFIG}/bin/ejsc --out ../../../${CONFIG}/bin/ejs.web.mod --debug --optimize 9 *.es ;\
../../../${CONFIG}/bin/ejsmod --cslots ../../../${CONFIG}/bin/ejs.web.mod ;\
if ! diff ejs.web.slots.h ../../../${CONFIG}/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../../${CONFIG}/inc; fi ;\
rm -f ejs.web.slots.h ;\
cd - >/dev/null 

"${CC}" -c -Fo${CONFIG}/obj/ejsHttpServer.obj -Fd${CONFIG}/obj/ejsHttpServer.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsHttpServer.c

"${CC}" -c -Fo${CONFIG}/obj/ejsRequest.obj -Fd${CONFIG}/obj/ejsRequest.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsRequest.c

"${CC}" -c -Fo${CONFIG}/obj/ejsSession.obj -Fd${CONFIG}/obj/ejsSession.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsSession.c

"${CC}" -c -Fo${CONFIG}/obj/ejsWeb.obj -Fd${CONFIG}/obj/ejsWeb.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsWeb.c

"${LD}" -dll -out:${CONFIG}/bin/ejs.web.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/ejs.web.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsHttpServer.obj ${CONFIG}/obj/ejsRequest.obj ${CONFIG}/obj/ejsSession.obj ${CONFIG}/obj/ejsWeb.obj ${LIBS} libejs.lib libhttp.lib libmpr.lib libpcre.lib libmprssl.lib

cd src/jems/ejs.web >/dev/null ;\
rm -fr ../../../${CONFIG}/bin/www ;\
cp -r www ../../../${CONFIG}/bin ;\
cd - >/dev/null 

#  Omit build script /Users/mob/git/ejs/win-x86-debug/bin/ejs.template.mod
#  Omit build script /Users/mob/git/ejs/win-x86-debug/bin/ejs.tar.mod
#  Omit build script /Users/mob/git/ejs/win-x86-debug/bin/mvc.es
rm -rf ${CONFIG}/bin/mvc.exe
cp -r ${CONFIG}/bin/ejsrun.exe ${CONFIG}/bin/mvc.exe

#  Omit build script /Users/mob/git/ejs/win-x86-debug/bin/ejs.mvc.mod
#  Omit build script /Users/mob/git/ejs/win-x86-debug/bin/utest.worker
"${CC}" -c -Fo${CONFIG}/obj/removeFiles.obj -Fd${CONFIG}/obj/removeFiles.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc package/WIN/removeFiles.c

"${LD}" -out:${CONFIG}/bin/removeFiles.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/removeFiles.obj ${LIBS} libmpr.lib

