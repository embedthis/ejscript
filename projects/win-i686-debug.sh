#
#   build.sh -- Build It Shell Script to build Embedthis Ejscript
#

PLATFORM="win-i686-debug"
CC="C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/cl.exe"
CFLAGS="-nologo -GR- -W3 -Zi -Od -MDd"
DFLAGS="-D_REENTRANT -D_MT"
IFLAGS="-Iwin-i686-debug\inc"
LDFLAGS="-nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86"
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

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.dll -entry:_DllMainCRTStartup@12 -def:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.def -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 C:\cygwin\home\mob\ejs\${PLATFORM}\obj\mprLib.obj ${LIBS}

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\mprSsl.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -I..\packages-win-i686\openssl\openssl-1.0.0d\inc32 C:\cygwin\home\mob\ejs\src\deps\mpr\mprSsl.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmprssl.dll -entry:_DllMainCRTStartup@12 -def:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmprssl.def -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\mprSsl.obj ${LIBS} C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.lib libeay32.lib ssleay32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\makerom.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\deps\mpr\makerom.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\makerom.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 C:\cygwin\home\mob\ejs\${PLATFORM}\obj\makerom.obj ${LIBS} C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.lib

rm -rf win-i686-debug\inc\pcre.h
cp -r src\deps\pcre\pcre.h win-i686-debug\inc\pcre.h

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\pcre.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\deps\pcre\pcre.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.dll -entry:_DllMainCRTStartup@12 -def:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.def -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 C:\cygwin\home\mob\ejs\${PLATFORM}\obj\pcre.obj ${LIBS}

rm -rf win-i686-debug\inc\http.h
cp -r src\deps\http\http.h win-i686-debug\inc\http.h

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\httpLib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\deps\http\httpLib.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libhttp.dll -entry:_DllMainCRTStartup@12 -def:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libhttp.def -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\httpLib.obj ${LIBS} C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmprssl.lib libeay32.lib ssleay32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\http.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\deps\http\http.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\http.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\http.obj ${LIBS} C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libhttp.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmprssl.lib libeay32.lib ssleay32.lib

rm -rf win-i686-debug\inc\sqlite3.h
cp -r src\deps\sqlite\sqlite3.h win-i686-debug\inc\sqlite3.h

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\sqlite3.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\deps\sqlite\sqlite3.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libsqlite3.dll -entry:_DllMainCRTStartup@12 -def:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libsqlite3.def -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 C:\cygwin\home\mob\ejs\${PLATFORM}\obj\sqlite3.obj ${LIBS}

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

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libejs.dll -entry:_DllMainCRTStartup@12 -def:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libejs.def -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecAst.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecCodeGen.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecCompiler.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecLex.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecModuleWrite.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecParser.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ecState.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsApp.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsArray.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsBlock.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsBoolean.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsByteArray.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsCache.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsCmd.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsConfig.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsDate.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsDebug.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsError.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsFile.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsFileSystem.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsFrame.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsFunction.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsGC.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsGlobal.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsHttp.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsIterator.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsJSON.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsLocalCache.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsMath.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsMemory.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsMprLog.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsNamespace.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsNull.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsNumber.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsObject.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsPath.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsPot.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsRegExp.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsSocket.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsString.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsSystem.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsTimer.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsType.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsUri.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsVoid.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsWorker.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsXML.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsXMLList.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsXMLLoader.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsByteCode.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsException.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsHelper.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsInterp.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsLoader.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsModule.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsScope.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsService.obj ${LIBS} C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libhttp.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmprssl.lib libeay32.lib ssleay32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejs.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\cmd\ejs.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\ejs.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejs.obj ${LIBS} C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libejs.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libhttp.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmprssl.lib libeay32.lib ssleay32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsc.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\cmd\ejsc.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\ejsc.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsc.obj ${LIBS} C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libejs.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libhttp.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmprssl.lib libeay32.lib ssleay32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsmod.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\cmd C:\cygwin\home\mob\ejs\src\cmd\ejsmod.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\doc.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\cmd C:\cygwin\home\mob\ejs\src\cmd\doc.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\docFiles.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\cmd C:\cygwin\home\mob\ejs\src\cmd\docFiles.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\listing.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\cmd C:\cygwin\home\mob\ejs\src\cmd\listing.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\slotGen.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\cmd C:\cygwin\home\mob\ejs\src\cmd\slotGen.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\ejsmod.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsmod.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\doc.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\docFiles.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\listing.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\slotGen.obj ${LIBS} C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libejs.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libhttp.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmprssl.lib libeay32.lib ssleay32.lib

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsrun.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\cmd\ejsrun.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\ejsrun.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsrun.obj ${LIBS} C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libejs.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libhttp.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmprssl.lib libeay32.lib ssleay32.lib

ejsc --out C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.mod --debug --optimize 9 --bind --require null C:/cygwin/home/mob/ejs/src/core/*.es 
ejsmod --require null --listing --cslots C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.mod
cp ejs.slots.h C:/cygwin/home/mob/ejs/${PLATFORM}/inc
cp src\jems\ejs.bit/bit.es C:/cygwin/home/mob/ejs/${PLATFORM}/bin
rm -rf win-i686-debug/bin/bit.exe
cp -r win-i686-debug/bin/ejsrun.exe win-i686-debug/bin/bit.exe

cp src\jems\ejs.utest/utest.es C:/cygwin/home/mob/ejs/${PLATFORM}/bin
rm -rf win-i686-debug/bin/utest.exe
cp -r win-i686-debug/bin/ejsrun.exe win-i686-debug/bin/utest.exe

rm -fr C:/cygwin/home/mob/ejs/${PLATFORM}/bin/bits
cp -r src\jems\ejs.bit/bits C:/cygwin/home/mob/ejs/${PLATFORM}/bin
ejsc --out C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.unix.mod --debug --optimize 9 src\jems\ejs.unix/Unix.es
cp src\jems\ejs.jem/jem.es C:/cygwin/home/mob/ejs/${PLATFORM}/bin
rm -rf win-i686-debug/bin/jem.exe
cp -r win-i686-debug/bin/ejsrun.exe win-i686-debug/bin/jem.exe

ejsc --out C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.db.mod --debug --optimize 9 src\jems\ejs.db/*.es
ejsc --out C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.db.mapper.mod --debug --optimize 9 src\jems\ejs.db.mapper/*.es
ejsc --out C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.db.sqlite.mod --debug --optimize 9 src\jems\ejs.db.sqlite/*.es
"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsSqlite.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc C:\cygwin\home\mob\ejs\src\jems\ejs.db.sqlite\src\ejsSqlite.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\ejs.db.sqlite.dll -entry:_DllMainCRTStartup@12 -def:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\ejs.db.sqlite.def -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsSqlite.obj ${LIBS} C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libejs.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libhttp.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmprssl.lib libeay32.lib ssleay32.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libsqlite3.lib

ejsc --out C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.web.mod --debug --optimize 9 src\jems\ejs.web/*.es
ejsmod --cslots C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.web.mod
cp ejs.web.slots.h C:/cygwin/home/mob/ejs/${PLATFORM}/inc
"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsHttpServer.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\jems\ejs.web\src C:\cygwin\home\mob\ejs\src\jems\ejs.web\src\ejsHttpServer.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsRequest.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\jems\ejs.web\src C:\cygwin\home\mob\ejs\src\jems\ejs.web\src\ejsRequest.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsSession.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\jems\ejs.web\src C:\cygwin\home\mob\ejs\src\jems\ejs.web\src\ejsSession.c

"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsWeb.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -Isrc\jems\ejs.web\src C:\cygwin\home\mob\ejs\src\jems\ejs.web\src\ejsWeb.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\ejs.web.dll -entry:_DllMainCRTStartup@12 -def:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\ejs.web.def -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsHttpServer.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsRequest.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsSession.obj C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsWeb.obj ${LIBS} C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libhttp.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmprssl.lib libeay32.lib ssleay32.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libejs.lib

rm -fr C:/cygwin/home/mob/ejs/${PLATFORM}/bin/www
cp -r src\jems\ejs.web/www C:/cygwin/home/mob/ejs/${PLATFORM}/bin
ejsc --out C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.template.mod --debug --optimize 9 src\jems\ejs.template/TemplateParser.es
ejsc --out C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.tar.mod/ --debug --optimize 9 src\jems\ejs.tar/*.es
ejsc --out C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.zlib.mod/ --debug --optimize 9 src\jems\ejs.zlib/*.es
"${CC}" -c -FoC:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsZlib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}\inc -I..\packages-win-i686\zlib\zlib-1.2.6 C:\cygwin\home\mob\ejs\src\jems\ejs.zlib\src\ejsZlib.c

"C:/Program Files/Microsoft Visual Studio 10.0/VC/bin/link.exe" -dll -out:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\ejs.zlib.dll -entry:_DllMainCRTStartup@12 -def:C:\cygwin\home\mob\ejs\${PLATFORM}\bin\ejs.zlib.def -nologo -nodefaultlib -incremental:no -libpath:C:\cygwin\home\mob\ejs\${PLATFORM}\bin -debug -machine:x86 -libpath:C:\cygwin\home\mob\packages-win-i686\openssl\openssl-1.0.0d\out32dll -libpath:C:\cygwin\home\mob\packages-win-i686\zlib\zlib-1.2.6 C:\cygwin\home\mob\ejs\${PLATFORM}\obj\ejsZlib.obj ${LIBS} C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmpr.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libejs.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libpcre.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libhttp.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libmprssl.lib libeay32.lib ssleay32.lib C:\cygwin\home\mob\ejs\${PLATFORM}\bin\libz.lib

cp src\jems\ejs.mvc/mvc.es C:/cygwin/home/mob/ejs/${PLATFORM}/bin
rm -rf win-i686-debug/bin/mvc.exe
cp -r win-i686-debug/bin/ejsrun.exe win-i686-debug/bin/mvc.exe

ejsc --out C:/cygwin/home/mob/ejs/${PLATFORM}/bin/ejs.mvc.mod/ --debug --optimize 9 src\jems\ejs.mvc/*.es
cp src\jems\ejs.utest/utest.worker C:/cygwin/home/mob/ejs/${PLATFORM}/bin
