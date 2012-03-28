#
#   build.sh -- Build It Shell Script to build Embedthis Ejscript
#

PLATFORM="win-i686-debug"
CC="C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/cl.exe"
CFLAGS="-nologo -GR- -W3 -Zi -Od -MDd"
DFLAGS="-D_REENTRANT -D_MT"
IFLAGS="-Iwin-i686-debug\inc"
LDFLAGS="-nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86
LIBS="ws2_32.lib advapi32.lib user32.lib kernel32.lib oldnames.lib msvcrt.lib"

export PATH="C:/Program Files/Microsoft SDKs/Windows/v7.1/Bin;C:/Program Files/Microsoft Visual Studio 10.0/VC/Bin;C:/Program Files/Microsoft Visual Studio 10.0/Common7/IDE;C:/Program Files/Microsoft Visual Studio 10.0/Common7/Tools;C:/Program Files/Microsoft Visual Studio 10.0/SDK/v3.5/bin;C:/Program Files/Microsoft Visual Studio 10.0/VC/VCPackages"
export INCLUDE="C:/Program Files/Microsoft SDKs/Windows/v7.1/INCLUDE;C:/Program Files/Microsoft Visual Studio 10.0/VC/INCLUDE"
export LIB="C:/Program Files/Microsoft SDKs/Windows/v7.1/lib;C:/Program Files/Microsoft Visual Studio 10.0/VC/lib"
[ ! -x ${PLATFORM}/inc ] && mkdir -p ${PLATFORM}/inc ${PLATFORM}/obj ${PLATFORM}/lib ${PLATFORM}/bin
[ ! -f ${PLATFORM}/inc/buildConfig.h ] && cp projects/buildConfig.${PLATFORM} ${PLATFORM}/inc/buildConfig.h

rm -rf win-i686-debug\inc\mpr.h
cp -r src\deps\mpr\mpr.h win-i686-debug\inc\mpr.h

rm -rf win-i686-debug\inc\mprSsl.h
cp -r src\deps\mpr\mprSsl.h win-i686-debug\inc\mprSsl.h

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\mprLib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\deps\mpr\mprLib.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:${PLATFORM}\bin\libmpr.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}\bin\libmpr.def ${LDFLAGS} ${PLATFORM}\obj\mprLib.obj ${LIBS}

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\mprSsl.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -I..\packages-win-i686\openssl\openssl-1.0.0d\inc32 C:\cygwin\home\mob\ejs\src\deps\mpr\mprSsl.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:${PLATFORM}\bin\libmprssl.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}\bin\libmprssl.def ${LDFLAGS} -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll ${PLATFORM}\obj\mprSsl.obj ${LIBS} libmpr.lib libeay32.lib ssleay32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\manager.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\deps\mpr\manager.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:${PLATFORM}/bin/ejsman -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} C:\cygwin\home\mob\ejs\${PLATFORM}\obj\manager.obj ${LIBS} libmpr.lib shell32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\makerom.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\deps\mpr\makerom.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:${PLATFORM}\bin\makerom.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} C:\cygwin\home\mob\ejs\${PLATFORM}\obj\makerom.obj ${LIBS} libmpr.lib

rm -rf win-i686-debug\inc\pcre.h
cp -r src\deps\pcre\pcre.h win-i686-debug\inc\pcre.h

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\pcre.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\deps\pcre\pcre.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:${PLATFORM}\bin\libpcre.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}\bin\libpcre.def ${LDFLAGS} ${PLATFORM}\obj\pcre.obj ${LIBS}

rm -rf win-i686-debug\inc\http.h
cp -r src\deps\http\http.h win-i686-debug\inc\http.h

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\httpLib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\deps\http\httpLib.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:${PLATFORM}\bin\libhttp.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}\bin\libhttp.def ${LDFLAGS} -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll ${PLATFORM}\obj\httpLib.obj ${LIBS} libmpr.lib libpcre.lib libmprssl.lib libeay32.lib ssleay32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\http.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\deps\http\http.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:${PLATFORM}\bin\http.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\http.obj ${LIBS} libhttp.lib libmpr.lib libpcre.lib libmprssl.lib libeay32.lib ssleay32.lib

rm -rf win-i686-debug\inc\sqlite3.h
cp -r src\deps\sqlite\sqlite3.h win-i686-debug\inc\sqlite3.h

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\sqlite3.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\deps\sqlite\sqlite3.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:${PLATFORM}\bin\libsqlite3.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}\bin\libsqlite3.def ${LDFLAGS} ${PLATFORM}\obj\sqlite3.obj ${LIBS}

rm -rf win-i686-debug\inc\ejs.cache.local.slots.h
cp -r src\slots\ejs.cache.local.slots.h win-i686-debug\inc\ejs.cache.local.slots.h

rm -rf win-i686-debug\inc\ejs.db.sqlite.slots.h
cp -r src\slots\ejs.db.sqlite.slots.h win-i686-debug\inc\ejs.db.sqlite.slots.h

rm -rf win-i686-debug\inc\ejs.slots.h
cp -r src\slots\ejs.slots.h win-i686-debug\inc\ejs.slots.h

rm -rf win-i686-debug\inc\ejs.web.slots.h
cp -r src\slots\ejs.web.slots.h win-i686-debug\inc\ejs.web.slots.h

rm -rf win-i686-debug\inc\ejs.zlib.slots.h
cp -r src\slots\ejs.zlib.slots.h win-i686-debug\inc\ejs.zlib.slots.h

rm -rf win-i686-debug\inc\ejs.h
cp -r src\ejs.h win-i686-debug\inc\ejs.h

rm -rf win-i686-debug\inc\ejsByteCode.h
cp -r src\ejsByteCode.h win-i686-debug\inc\ejsByteCode.h

rm -rf win-i686-debug\inc\ejsByteCodeTable.h
cp -r src\ejsByteCodeTable.h win-i686-debug\inc\ejsByteCodeTable.h

rm -rf win-i686-debug\inc\ejsCompiler.h
cp -r src\ejsCompiler.h win-i686-debug\inc\ejsCompiler.h

rm -rf win-i686-debug\inc\ejsCustomize.h
cp -r src\ejsCustomize.h win-i686-debug\inc\ejsCustomize.h

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecAst.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\compiler\ecAst.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecCodeGen.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\compiler\ecCodeGen.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecCompiler.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\compiler\ecCompiler.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecLex.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\compiler\ecLex.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecModuleWrite.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\compiler\ecModuleWrite.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecParser.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\compiler\ecParser.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecState.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\compiler\ecState.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsApp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsApp.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsArray.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsArray.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsBlock.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsBlock.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsBoolean.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsBoolean.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsByteArray.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsByteArray.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsCache.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsCache.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsCmd.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsCmd.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsConfig.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsConfig.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsDate.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsDate.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsDebug.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsDebug.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsError.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsError.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsFile.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsFile.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsFileSystem.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsFileSystem.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsFrame.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsFrame.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsFunction.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsFunction.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsGC.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsGC.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsGlobal.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsGlobal.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsHttp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsHttp.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsIterator.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsIterator.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsJSON.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsJSON.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsLocalCache.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsLocalCache.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsMath.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsMath.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsMemory.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsMemory.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsMprLog.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsMprLog.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsNamespace.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsNamespace.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsNull.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsNull.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsNumber.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsNumber.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsObject.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsObject.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsPath.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsPath.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsPot.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsPot.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsRegExp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsRegExp.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsSocket.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsSocket.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsString.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsString.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsSystem.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsSystem.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsTimer.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsTimer.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsType.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsType.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsUri.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsUri.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsVoid.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsVoid.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsWorker.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsWorker.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsXML.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsXML.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsXMLList.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsXMLList.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsXMLLoader.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\core\src\ejsXMLLoader.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsByteCode.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\vm\ejsByteCode.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsException.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\vm\ejsException.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsHelper.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\vm\ejsHelper.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsInterp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\vm\ejsInterp.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsLoader.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\vm\ejsLoader.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsModule.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\vm\ejsModule.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsScope.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\vm\ejsScope.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsService.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\vm\ejsService.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:${PLATFORM}\bin\libejs.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}\bin\libejs.def ${LDFLAGS} -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll ${PLATFORM}\obj\ecAst.obj ${PLATFORM}\obj\ecCodeGen.obj ${PLATFORM}\obj\ecCompiler.obj ${PLATFORM}\obj\ecLex.obj ${PLATFORM}\obj\ecModuleWrite.obj ${PLATFORM}\obj\ecParser.obj ${PLATFORM}\obj\ecState.obj ${PLATFORM}\obj\ejsApp.obj ${PLATFORM}\obj\ejsArray.obj ${PLATFORM}\obj\ejsBlock.obj ${PLATFORM}\obj\ejsBoolean.obj ${PLATFORM}\obj\ejsByteArray.obj ${PLATFORM}\obj\ejsCache.obj ${PLATFORM}\obj\ejsCmd.obj ${PLATFORM}\obj\ejsConfig.obj ${PLATFORM}\obj\ejsDate.obj ${PLATFORM}\obj\ejsDebug.obj ${PLATFORM}\obj\ejsError.obj ${PLATFORM}\obj\ejsFile.obj ${PLATFORM}\obj\ejsFileSystem.obj ${PLATFORM}\obj\ejsFrame.obj ${PLATFORM}\obj\ejsFunction.obj ${PLATFORM}\obj\ejsGC.obj ${PLATFORM}\obj\ejsGlobal.obj ${PLATFORM}\obj\ejsHttp.obj ${PLATFORM}\obj\ejsIterator.obj ${PLATFORM}\obj\ejsJSON.obj ${PLATFORM}\obj\ejsLocalCache.obj ${PLATFORM}\obj\ejsMath.obj ${PLATFORM}\obj\ejsMemory.obj ${PLATFORM}\obj\ejsMprLog.obj ${PLATFORM}\obj\ejsNamespace.obj ${PLATFORM}\obj\ejsNull.obj ${PLATFORM}\obj\ejsNumber.obj ${PLATFORM}\obj\ejsObject.obj ${PLATFORM}\obj\ejsPath.obj ${PLATFORM}\obj\ejsPot.obj ${PLATFORM}\obj\ejsRegExp.obj ${PLATFORM}\obj\ejsSocket.obj ${PLATFORM}\obj\ejsString.obj ${PLATFORM}\obj\ejsSystem.obj ${PLATFORM}\obj\ejsTimer.obj ${PLATFORM}\obj\ejsType.obj ${PLATFORM}\obj\ejsUri.obj ${PLATFORM}\obj\ejsVoid.obj ${PLATFORM}\obj\ejsWorker.obj ${PLATFORM}\obj\ejsXML.obj ${PLATFORM}\obj\ejsXMLList.obj ${PLATFORM}\obj\ejsXMLLoader.obj ${PLATFORM}\obj\ejsByteCode.obj ${PLATFORM}\obj\ejsException.obj ${PLATFORM}\obj\ejsHelper.obj ${PLATFORM}\obj\ejsInterp.obj ${PLATFORM}\obj\ejsLoader.obj ${PLATFORM}\obj\ejsModule.obj ${PLATFORM}\obj\ejsScope.obj ${PLATFORM}\obj\ejsService.obj ${LIBS} libmpr.lib libpcre.lib libhttp.lib libmprssl.lib libeay32.lib ssleay32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejs.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\cmd\ejs.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:${PLATFORM}\bin\ejs.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejs.obj ${LIBS} libejs.lib libmpr.lib libpcre.lib libhttp.lib libmprssl.lib libeay32.lib ssleay32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsc.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\cmd\ejsc.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:${PLATFORM}\bin\ejsc.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsc.obj ${LIBS} libejs.lib libmpr.lib libpcre.lib libhttp.lib libmprssl.lib libeay32.lib ssleay32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsmod.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\cmd C:\cygwin\home\mob\ejs\src\cmd\ejsmod.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\doc.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\cmd C:\cygwin\home\mob\ejs\src\cmd\doc.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\docFiles.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\cmd C:\cygwin\home\mob\ejs\src\cmd\docFiles.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\listing.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\cmd C:\cygwin\home\mob\ejs\src\cmd\listing.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\slotGen.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\cmd C:\cygwin\home\mob\ejs\src\cmd\slotGen.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:${PLATFORM}\bin\ejsmod.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsmod.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\doc.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\docFiles.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\listing.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\slotGen.obj ${LIBS} libejs.lib libmpr.lib libpcre.lib libhttp.lib libmprssl.lib libeay32.lib ssleay32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsrun.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\cmd\ejsrun.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:${PLATFORM}\bin\ejsrun.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsrun.obj ${LIBS} libejs.lib libmpr.lib libpcre.lib libhttp.lib libmprssl.lib libeay32.lib ssleay32.lib

ejsc --out C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.mod --debug --optimize 9 --bind --require null C:/cygwin/home/mob/ejs/src/core/*.es 
ejsmod --require null --cslots C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.mod
if ! diff ejs.slots.h C:/cygwin/home/mob/ejs/${PLATFORM}/inc/ejs.slots.h >/dev/null; then mv ejs.slots.h C:/cygwin/home/mob/ejs/${PLATFORM}/inc; fi
cp src\jems\ejs.bit/bit.es C:/cygwin/home/mob/ejs/${PLATFORM}/bin
rm -rf win-i686-debug/bin/bit.exe
cp -r win-i686-debug/bin/ejsrun.exe win-i686-debug/bin/bit.exe

cp src\jems\ejs.utest/utest.es C:/cygwin/home/mob/ejs/${PLATFORM}/bin
rm -rf win-i686-debug/bin/utest.exe
cp -r win-i686-debug/bin/ejsrun.exe win-i686-debug/bin/utest.exe

rm -fr C:/cygwin/home/mob/ejs/${PLATFORM}/bin/bits
cp -r src\jems\ejs.bit/bits C:/cygwin/home/mob/ejs/${PLATFORM}/bin
ejsc --out ${PLATFORM}/bin/ejs.unix.mod --debug --optimize 9 src\jems\ejs.unix/Unix.es
cp src\jems\ejs.jem/jem.es C:/cygwin/home/mob/ejs/${PLATFORM}/bin
rm -rf win-i686-debug/bin/jem.exe
cp -r win-i686-debug/bin/ejsrun.exe win-i686-debug/bin/jem.exe

ejsc --out ${PLATFORM}/bin/ejs.db.mod --debug --optimize 9 src\jems\ejs.db/*.es
ejsc --out ${PLATFORM}/bin/ejs.db.mapper.mod --debug --optimize 9 src\jems\ejs.db.mapper/*.es
ejsc --out ${PLATFORM}/bin/ejs.db.sqlite.mod --debug --optimize 9 src\jems\ejs.db.sqlite/*.es
"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsSqlite.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\jems\ejs.db.sqlite\src\ejsSqlite.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:${PLATFORM}\bin\ejs.db.sqlite.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}\bin\ejs.db.sqlite.def ${LDFLAGS} -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll ${PLATFORM}\obj\ejsSqlite.obj ${LIBS} libmpr.lib libejs.lib libpcre.lib libhttp.lib libmprssl.lib libeay32.lib ssleay32.lib libsqlite3.lib

ejsc --out ${PLATFORM}/bin/ejs.web.mod --debug --optimize 9 src\jems\ejs.web/*.es
ejsmod --cslots ${PLATFORM}/bin/ejs.web.mod
if ! diff ejs.web.slots.h C:/cygwin/home/mob/ejs/${PLATFORM}/inc/ejs.web.slots.h >/dev/null; then mv ejs.web.slots.h C:/cygwin/home/mob/ejs/${PLATFORM}/inc; fi
"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsHttpServer.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\jems\ejs.web\src C:\cygwin\home\mob\ejs\src\jems\ejs.web\src\ejsHttpServer.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsRequest.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\jems\ejs.web\src C:\cygwin\home\mob\ejs\src\jems\ejs.web\src\ejsRequest.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsSession.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\jems\ejs.web\src C:\cygwin\home\mob\ejs\src\jems\ejs.web\src\ejsSession.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsWeb.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\jems\ejs.web\src C:\cygwin\home\mob\ejs\src\jems\ejs.web\src\ejsWeb.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:${PLATFORM}\bin\ejs.web.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}\bin\ejs.web.def ${LDFLAGS} -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll ${PLATFORM}\obj\ejsHttpServer.obj ${PLATFORM}\obj\ejsRequest.obj ${PLATFORM}\obj\ejsSession.obj ${PLATFORM}\obj\ejsWeb.obj ${LIBS} libmpr.lib libhttp.lib libpcre.lib libmprssl.lib libeay32.lib ssleay32.lib libpcre.lib libejs.lib

rm -fr C:/cygwin/home/mob/ejs/${PLATFORM}/bin/www
cp -r src\jems\ejs.web/www C:/cygwin/home/mob/ejs/${PLATFORM}/bin
ejsc --out ${PLATFORM}/bin/ejs.template.mod --debug --optimize 9 src\jems\ejs.template/TemplateParser.es
ejsc --out ${PLATFORM}/bin/ejs.tar.mod/ --debug --optimize 9 src\jems\ejs.tar/*.es
ejsc --out ${PLATFORM}/bin/ejs.zlib.mod/ --debug --optimize 9 src\jems\ejs.zlib/*.es
"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsZlib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -I..\packages-win-i686\zlib\zlib-1.2.6 C:\cygwin\home\mob\ejs\src\jems\ejs.zlib\src\ejsZlib.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:${PLATFORM}\bin\ejs.zlib.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}\bin\ejs.zlib.def ${LDFLAGS} -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll -libpath:C:\cygwin\home\mob\packages-win-i686\zlib\zlib-1.2.6 ${PLATFORM}\obj\ejsZlib.obj ${LIBS} libmpr.lib libejs.lib libpcre.lib libhttp.lib libmprssl.lib libeay32.lib ssleay32.lib libz.lib

cp src\jems\ejs.mvc/mvc.es C:/cygwin/home/mob/ejs/${PLATFORM}/bin
rm -rf win-i686-debug/bin/mvc.exe
cp -r win-i686-debug/bin/ejsrun.exe win-i686-debug/bin/mvc.exe

ejsc --out ${PLATFORM}/bin/ejs.mvc.mod/ --debug --optimize 9 src\jems\ejs.mvc/*.es
cp src\jems\ejs.utest/utest.worker C:/cygwin/home/mob/ejs/${PLATFORM}/bin
