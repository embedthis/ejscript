#
#   win-i686-debug.sh -- Build It Shell Script to build Embedthis Ejscript
#

VS="${VSINSTALLDIR}"
: ${VS:="$(PROGRAMFILES)\Microsoft Visual Studio 10.0"}
SDK="${WindowsSDKDir}"
: ${SDK:="$(PROGRAMFILES)\Microsoft SDKs\Windows\v7.1"}

export SDK VS
export PATH="$(SDK)/Bin;$(VS)/VC/Bin;$(VS)/Common7/IDE;$(VS)/Common7/Tools;$(VS)/SDK/v3.5/bin;$(VS)/VC/VCPackages;$(PATH)"
export INCLUDE="$(INCLUDE);$(SDK)/INCLUDE;$(VS)/VC/INCLUDE"
export LIB="$(LIB);$(SDK)/lib;$(VS)/VC/lib"

PLATFORM="win-i686-debug"
CC="C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/cl.exe"
LD="C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe"
CFLAGS="-nologo -GR- -W3 -Zi -Od -MDd"
DFLAGS="-D_REENTRANT -D_MT"
IFLAGS="-Iwin-i686-debug/inc"
LDFLAGS="-nologo -nodefaultlib -incremental:no -debug -machine:x86"
LIBPATHS="-libpath:${PLATFORM}/bin"
LIBS="ws2_32.lib advapi32.lib user32.lib kernel32.lib oldnames.lib msvcrt.lib shell32.lib"

[ ! -x ${PLATFORM}/inc ] && mkdir -p ${PLATFORM}/inc ${PLATFORM}/obj ${PLATFORM}/lib ${PLATFORM}/bin
[ ! -f ${PLATFORM}/inc/buildConfig.h ] && cp projects/buildConfig.${PLATFORM} ${PLATFORM}/inc/buildConfig.h

rm -rf win-i686-debug/inc/mpr.h
cp -r src/deps/mpr/mpr.h win-i686-debug/inc/mpr.h

rm -rf win-i686-debug/inc/mprSsl.h
cp -r src/deps/mpr/mprSsl.h win-i686-debug/inc/mprSsl.h

"${CC}" -c -Fo${PLATFORM}/obj/mprLib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/mpr/mprLib.c

"${LD}" -dll -out:${PLATFORM}/bin/libmpr.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libmpr.def ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/mprLib.obj ${LIBS}

"${CC}" -c -Fo${PLATFORM}/obj/manager.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/mpr/manager.c

"${LD}" -out:${PLATFORM}/bin/ejsman -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/manager.obj ${LIBS} libmpr.lib

"${CC}" -c -Fo${PLATFORM}/obj/makerom.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/mpr/makerom.c

"${LD}" -out:${PLATFORM}/bin/makerom.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/makerom.obj ${LIBS} libmpr.lib

rm -rf win-i686-debug/inc/pcre.h
cp -r src/deps/pcre/pcre.h win-i686-debug/inc/pcre.h

"${CC}" -c -Fo${PLATFORM}/obj/pcre.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/pcre/pcre.c

"${LD}" -dll -out:${PLATFORM}/bin/libpcre.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libpcre.def ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/pcre.obj ${LIBS}

rm -rf win-i686-debug/inc/http.h
cp -r src/deps/http/http.h win-i686-debug/inc/http.h

"${CC}" -c -Fo${PLATFORM}/obj/httpLib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/http/httpLib.c

"${LD}" -dll -out:${PLATFORM}/bin/libhttp.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libhttp.def ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/httpLib.obj ${LIBS} libmpr.lib libpcre.lib

"${CC}" -c -Fo${PLATFORM}/obj/http.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/http/http.c

"${LD}" -out:${PLATFORM}/bin/http.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/http.obj ${LIBS} libhttp.lib libmpr.lib libpcre.lib

rm -rf win-i686-debug/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h win-i686-debug/inc/sqlite3.h

"${CC}" -c -Fo${PLATFORM}/obj/sqlite3.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/sqlite/sqlite3.c

"${LD}" -dll -out:${PLATFORM}/bin/libsqlite3.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libsqlite3.def ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/sqlite3.obj ${LIBS}

rm -rf win-i686-debug/inc/ejs.cache.local.slots.h
cp -r src/slots/ejs.cache.local.slots.h win-i686-debug/inc/ejs.cache.local.slots.h

rm -rf win-i686-debug/inc/ejs.db.sqlite.slots.h
cp -r src/slots/ejs.db.sqlite.slots.h win-i686-debug/inc/ejs.db.sqlite.slots.h

rm -rf win-i686-debug/inc/ejs.slots.h
cp -r src/slots/ejs.slots.h win-i686-debug/inc/ejs.slots.h

rm -rf win-i686-debug/inc/ejs.web.slots.h
cp -r src/slots/ejs.web.slots.h win-i686-debug/inc/ejs.web.slots.h

rm -rf win-i686-debug/inc/ejs.zlib.slots.h
cp -r src/slots/ejs.zlib.slots.h win-i686-debug/inc/ejs.zlib.slots.h

rm -rf win-i686-debug/inc/ejs.h
cp -r src/ejs.h win-i686-debug/inc/ejs.h

rm -rf win-i686-debug/inc/ejsByteCode.h
cp -r src/ejsByteCode.h win-i686-debug/inc/ejsByteCode.h

rm -rf win-i686-debug/inc/ejsByteCodeTable.h
cp -r src/ejsByteCodeTable.h win-i686-debug/inc/ejsByteCodeTable.h

rm -rf win-i686-debug/inc/ejsCompiler.h
cp -r src/ejsCompiler.h win-i686-debug/inc/ejsCompiler.h

rm -rf win-i686-debug/inc/ejsCustomize.h
cp -r src/ejsCustomize.h win-i686-debug/inc/ejsCustomize.h

"${CC}" -c -Fo${PLATFORM}/obj/ecAst.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecAst.c

"${CC}" -c -Fo${PLATFORM}/obj/ecCodeGen.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecCodeGen.c

"${CC}" -c -Fo${PLATFORM}/obj/ecCompiler.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecCompiler.c

"${CC}" -c -Fo${PLATFORM}/obj/ecLex.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecLex.c

"${CC}" -c -Fo${PLATFORM}/obj/ecModuleWrite.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecModuleWrite.c

"${CC}" -c -Fo${PLATFORM}/obj/ecParser.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecParser.c

"${CC}" -c -Fo${PLATFORM}/obj/ecState.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecState.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsApp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsApp.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsArray.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsArray.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsBlock.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsBlock.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsBoolean.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsBoolean.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsByteArray.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsByteArray.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsCache.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsCache.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsCmd.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsCmd.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsConfig.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsConfig.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsDate.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsDate.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsDebug.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsDebug.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsError.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsError.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsFile.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsFile.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsFileSystem.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsFileSystem.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsFrame.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsFrame.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsFunction.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsFunction.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsGC.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsGC.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsGlobal.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsGlobal.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsHttp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsHttp.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsIterator.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsIterator.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsJSON.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsJSON.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsLocalCache.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsLocalCache.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsMath.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsMath.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsMemory.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsMemory.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsMprLog.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsMprLog.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsNamespace.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsNamespace.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsNull.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsNull.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsNumber.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsNumber.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsObject.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsObject.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsPath.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsPath.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsPot.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsPot.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsRegExp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsRegExp.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsSocket.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsSocket.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsString.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsString.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsSystem.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsSystem.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsTimer.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsTimer.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsType.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsType.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsUri.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsUri.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsVoid.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsVoid.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsWorker.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsWorker.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsXML.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsXML.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsXMLList.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsXMLList.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsXMLLoader.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsXMLLoader.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsByteCode.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsByteCode.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsException.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsException.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsHelper.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsHelper.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsInterp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsInterp.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsLoader.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsLoader.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsModule.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsModule.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsScope.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsScope.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsService.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsService.c

"${LD}" -dll -out:${PLATFORM}/bin/libejs.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libejs.def ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/ecAst.obj ${PLATFORM}/obj/ecCodeGen.obj ${PLATFORM}/obj/ecCompiler.obj ${PLATFORM}/obj/ecLex.obj ${PLATFORM}/obj/ecModuleWrite.obj ${PLATFORM}/obj/ecParser.obj ${PLATFORM}/obj/ecState.obj ${PLATFORM}/obj/ejsApp.obj ${PLATFORM}/obj/ejsArray.obj ${PLATFORM}/obj/ejsBlock.obj ${PLATFORM}/obj/ejsBoolean.obj ${PLATFORM}/obj/ejsByteArray.obj ${PLATFORM}/obj/ejsCache.obj ${PLATFORM}/obj/ejsCmd.obj ${PLATFORM}/obj/ejsConfig.obj ${PLATFORM}/obj/ejsDate.obj ${PLATFORM}/obj/ejsDebug.obj ${PLATFORM}/obj/ejsError.obj ${PLATFORM}/obj/ejsFile.obj ${PLATFORM}/obj/ejsFileSystem.obj ${PLATFORM}/obj/ejsFrame.obj ${PLATFORM}/obj/ejsFunction.obj ${PLATFORM}/obj/ejsGC.obj ${PLATFORM}/obj/ejsGlobal.obj ${PLATFORM}/obj/ejsHttp.obj ${PLATFORM}/obj/ejsIterator.obj ${PLATFORM}/obj/ejsJSON.obj ${PLATFORM}/obj/ejsLocalCache.obj ${PLATFORM}/obj/ejsMath.obj ${PLATFORM}/obj/ejsMemory.obj ${PLATFORM}/obj/ejsMprLog.obj ${PLATFORM}/obj/ejsNamespace.obj ${PLATFORM}/obj/ejsNull.obj ${PLATFORM}/obj/ejsNumber.obj ${PLATFORM}/obj/ejsObject.obj ${PLATFORM}/obj/ejsPath.obj ${PLATFORM}/obj/ejsPot.obj ${PLATFORM}/obj/ejsRegExp.obj ${PLATFORM}/obj/ejsSocket.obj ${PLATFORM}/obj/ejsString.obj ${PLATFORM}/obj/ejsSystem.obj ${PLATFORM}/obj/ejsTimer.obj ${PLATFORM}/obj/ejsType.obj ${PLATFORM}/obj/ejsUri.obj ${PLATFORM}/obj/ejsVoid.obj ${PLATFORM}/obj/ejsWorker.obj ${PLATFORM}/obj/ejsXML.obj ${PLATFORM}/obj/ejsXMLList.obj ${PLATFORM}/obj/ejsXMLLoader.obj ${PLATFORM}/obj/ejsByteCode.obj ${PLATFORM}/obj/ejsException.obj ${PLATFORM}/obj/ejsHelper.obj ${PLATFORM}/obj/ejsInterp.obj ${PLATFORM}/obj/ejsLoader.obj ${PLATFORM}/obj/ejsModule.obj ${PLATFORM}/obj/ejsScope.obj ${PLATFORM}/obj/ejsService.obj ${LIBS} libmpr.lib libpcre.lib libhttp.lib

"${CC}" -c -Fo${PLATFORM}/obj/ejs.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/cmd/ejs.c

"${LD}" -out:${PLATFORM}/bin/ejs.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/ejs.obj ${LIBS} libejs.lib libmpr.lib libpcre.lib libhttp.lib

"${CC}" -c -Fo${PLATFORM}/obj/ejsc.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/cmd/ejsc.c

"${LD}" -out:${PLATFORM}/bin/ejsc.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/ejsc.obj ${LIBS} libejs.lib libmpr.lib libpcre.lib libhttp.lib

"${CC}" -c -Fo${PLATFORM}/obj/ejsmod.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/cmd src/cmd/ejsmod.c

"${CC}" -c -Fo${PLATFORM}/obj/doc.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/cmd src/cmd/doc.c

"${CC}" -c -Fo${PLATFORM}/obj/docFiles.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/cmd src/cmd/docFiles.c

"${CC}" -c -Fo${PLATFORM}/obj/listing.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/cmd src/cmd/listing.c

"${CC}" -c -Fo${PLATFORM}/obj/slotGen.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/cmd src/cmd/slotGen.c

"${LD}" -out:${PLATFORM}/bin/ejsmod.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/ejsmod.obj ${PLATFORM}/obj/doc.obj ${PLATFORM}/obj/docFiles.obj ${PLATFORM}/obj/listing.obj ${PLATFORM}/obj/slotGen.obj ${LIBS} libejs.lib libmpr.lib libpcre.lib libhttp.lib

"${CC}" -c -Fo${PLATFORM}/obj/ejsrun.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/cmd/ejsrun.c

"${LD}" -out:${PLATFORM}/bin/ejsrun.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/ejsrun.obj ${PLATFORM}/obj/mprLib.obj ${PLATFORM}/obj/pcre.obj ${PLATFORM}/obj/httpLib.obj ${PLATFORM}/obj/ecAst.obj ${PLATFORM}/obj/ecCodeGen.obj ${PLATFORM}/obj/ecCompiler.obj ${PLATFORM}/obj/ecLex.obj ${PLATFORM}/obj/ecModuleWrite.obj ${PLATFORM}/obj/ecParser.obj ${PLATFORM}/obj/ecState.obj ${PLATFORM}/obj/ejsApp.obj ${PLATFORM}/obj/ejsArray.obj ${PLATFORM}/obj/ejsBlock.obj ${PLATFORM}/obj/ejsBoolean.obj ${PLATFORM}/obj/ejsByteArray.obj ${PLATFORM}/obj/ejsCache.obj ${PLATFORM}/obj/ejsCmd.obj ${PLATFORM}/obj/ejsConfig.obj ${PLATFORM}/obj/ejsDate.obj ${PLATFORM}/obj/ejsDebug.obj ${PLATFORM}/obj/ejsError.obj ${PLATFORM}/obj/ejsFile.obj ${PLATFORM}/obj/ejsFileSystem.obj ${PLATFORM}/obj/ejsFrame.obj ${PLATFORM}/obj/ejsFunction.obj ${PLATFORM}/obj/ejsGC.obj ${PLATFORM}/obj/ejsGlobal.obj ${PLATFORM}/obj/ejsHttp.obj ${PLATFORM}/obj/ejsIterator.obj ${PLATFORM}/obj/ejsJSON.obj ${PLATFORM}/obj/ejsLocalCache.obj ${PLATFORM}/obj/ejsMath.obj ${PLATFORM}/obj/ejsMemory.obj ${PLATFORM}/obj/ejsMprLog.obj ${PLATFORM}/obj/ejsNamespace.obj ${PLATFORM}/obj/ejsNull.obj ${PLATFORM}/obj/ejsNumber.obj ${PLATFORM}/obj/ejsObject.obj ${PLATFORM}/obj/ejsPath.obj ${PLATFORM}/obj/ejsPot.obj ${PLATFORM}/obj/ejsRegExp.obj ${PLATFORM}/obj/ejsSocket.obj ${PLATFORM}/obj/ejsString.obj ${PLATFORM}/obj/ejsSystem.obj ${PLATFORM}/obj/ejsTimer.obj ${PLATFORM}/obj/ejsType.obj ${PLATFORM}/obj/ejsUri.obj ${PLATFORM}/obj/ejsVoid.obj ${PLATFORM}/obj/ejsWorker.obj ${PLATFORM}/obj/ejsXML.obj ${PLATFORM}/obj/ejsXMLList.obj ${PLATFORM}/obj/ejsXMLLoader.obj ${PLATFORM}/obj/ejsByteCode.obj ${PLATFORM}/obj/ejsException.obj ${PLATFORM}/obj/ejsHelper.obj ${PLATFORM}/obj/ejsInterp.obj ${PLATFORM}/obj/ejsLoader.obj ${PLATFORM}/obj/ejsModule.obj ${PLATFORM}/obj/ejsScope.obj ${PLATFORM}/obj/ejsService.obj ${LIBS}

ejsc --out ${PLATFORM}/bin/ejs.mod --debug --optimize 9 --bind --require null src/core/*.es 
ejsmod --require null --cslots ${PLATFORM}/bin/ejs.mod
if ! diff ejs.slots.h ${PLATFORM}/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ${PLATFORM}/inc; fi
rm -f ejs.slots.h
cp src/jems/ejs.bit/bit.es ${PLATFORM}/bin
rm -rf win-i686-debug/bin/bit.exe
cp -r win-i686-debug/bin/ejsrun.exe win-i686-debug/bin/bit.exe

cp src/jems/ejs.utest/utest.es ${PLATFORM}/bin
rm -rf win-i686-debug/bin/utest.exe
cp -r win-i686-debug/bin/ejsrun.exe win-i686-debug/bin/utest.exe

rm -fr ${PLATFORM}/bin/bits
cp -r src/jems/ejs.bit/bits ${PLATFORM}/bin
ejsc --out ${PLATFORM}/bin/ejs.unix.mod --debug --optimize 9 src/jems/ejs.unix/Unix.es
cp src/jems/ejs.jem/jem.es ${PLATFORM}/bin
rm -rf win-i686-debug/bin/jem.exe
cp -r win-i686-debug/bin/ejsrun.exe win-i686-debug/bin/jem.exe

ejsc --out ${PLATFORM}/bin/ejs.db.mod --debug --optimize 9 src/jems/ejs.db/*.es
ejsc --out ${PLATFORM}/bin/ejs.db.mapper.mod --debug --optimize 9 src/jems/ejs.db.mapper/*.es
ejsc --out ${PLATFORM}/bin/ejs.db.sqlite.mod --debug --optimize 9 src/jems/ejs.db.sqlite/*.es
"${CC}" -c -Fo${PLATFORM}/obj/ejsSqlite.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/jems/ejs.db.sqlite/src/ejsSqlite.c

"${LD}" -dll -out:${PLATFORM}/bin/ejs.db.sqlite.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/ejs.db.sqlite.def ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/ejsSqlite.obj ${LIBS} libmpr.lib libejs.lib libpcre.lib libhttp.lib libsqlite3.lib

ejsc --out ${PLATFORM}/bin/ejs.web.mod --debug --optimize 9 src/jems/ejs.web/*.es
ejsmod --cslots ${PLATFORM}/bin/ejs.web.mod
if ! diff ejs.web.slots.h ${PLATFORM}/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ${PLATFORM}/inc; fi
rm -f ejs.web.slots.h
"${CC}" -c -Fo${PLATFORM}/obj/ejsHttpServer.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsHttpServer.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsRequest.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsRequest.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsSession.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsSession.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsWeb.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsWeb.c

"${LD}" -dll -out:${PLATFORM}/bin/ejs.web.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/ejs.web.def ${LDFLAGS} ${LIBPATHS} ${PLATFORM}/obj/ejsHttpServer.obj ${PLATFORM}/obj/ejsRequest.obj ${PLATFORM}/obj/ejsSession.obj ${PLATFORM}/obj/ejsWeb.obj ${LIBS} libmpr.lib libhttp.lib libpcre.lib libpcre.lib libejs.lib

rm -fr ${PLATFORM}/bin/www
cp -r src/jems/ejs.web/www ${PLATFORM}/bin
ejsc --out ${PLATFORM}/bin/ejs.template.mod --debug --optimize 9 src/jems/ejs.template/TemplateParser.es
ejsc --out ${PLATFORM}/bin/ejs.tar.mod --debug --optimize 9 src/jems/ejs.tar/*.es
cp src/jems/ejs.mvc/mvc.es ${PLATFORM}/bin
rm -rf win-i686-debug/bin/mvc.exe
cp -r win-i686-debug/bin/ejsrun.exe win-i686-debug/bin/mvc.exe

ejsc --out ${PLATFORM}/bin/ejs.mvc.mod --debug --optimize 9 src/jems/ejs.mvc/*.es
cp src/jems/ejs.utest/utest.worker ${PLATFORM}/bin
