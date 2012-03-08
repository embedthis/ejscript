#
#   build.sh -- Build It Shell Script to build Embedthis Ejscript
#

PLATFORM="win-i686-debug"
CC="cl"
CFLAGS="-nologo -GR- -W3 -Zi -Od -MDd"
DFLAGS="-D_REENTRANT -D_MT"
IFLAGS="-Iwin-i686-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots"
LDFLAGS="-nologo -nodefaultlib -incremental:no -libpath:/Users/mob/git/ejs/${PLATFORM}/bin -debug -machine:x86"
LIBS="ws2_32.lib advapi32.lib user32.lib kernel32.lib oldnames.lib msvcrt.lib"

export PATH="%VS%/Bin:%VS%/VC/Bin:%VS%/Common7/IDE:%VS%/Common7/Tools:%VS%/SDK/v3.5/bin:%VS%/VC/VCPackages"
export INCLUDE="%VS%/INCLUDE:%VS%/VC/INCLUDE"
export LIB="%VS%/lib:%VS%/VC/lib"
"${CC}" -c -Fo${PLATFORM}/obj/mprLib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/mprLib.c

"link" -dll -out:${PLATFORM}/bin/libmpr.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libmpr.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/mprLib.obj ${LIBS}

"${CC}" -c -Fo${PLATFORM}/obj/manager.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/manager.c

"link" -out:${PLATFORM}/bin/manager.exe -entry:WinMainCRTStartup -subsystem:Windows -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/manager.obj ${LIBS} mpr.lib shell32.lib

"${CC}" -c -Fo${PLATFORM}/obj/makerom.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/makerom.c

"link" -out:${PLATFORM}/bin/makerom.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/makerom.obj ${LIBS} mpr.lib

"${CC}" -c -Fo${PLATFORM}/obj/pcre.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/pcre/pcre.c

"link" -dll -out:${PLATFORM}/bin/libpcre.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libpcre.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/pcre.obj ${LIBS}

"${CC}" -c -Fo${PLATFORM}/obj/httpLib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/http/httpLib.c

"link" -dll -out:${PLATFORM}/bin/libhttp.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libhttp.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/httpLib.obj ${LIBS} mpr.lib pcre.lib

"${CC}" -c -Fo${PLATFORM}/obj/http.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/http/http.c

"link" -out:${PLATFORM}/bin/http.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/http.obj ${LIBS} http.lib mpr.lib pcre.lib

"${CC}" -c -Fo${PLATFORM}/obj/sqlite3.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/sqlite/sqlite3.c

"link" -dll -out:${PLATFORM}/bin/libsqlite3.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libsqlite3.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/sqlite3.obj ${LIBS}

cp /Users/mob/git/ejs/src/slots/*.h /Users/mob/git/ejs/win-i686-debug/inc
"${CC}" -c -Fo${PLATFORM}/obj/ecAst.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecAst.c

"${CC}" -c -Fo${PLATFORM}/obj/ecCodeGen.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecCodeGen.c

"${CC}" -c -Fo${PLATFORM}/obj/ecCompiler.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecCompiler.c

"${CC}" -c -Fo${PLATFORM}/obj/ecLex.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecLex.c

"${CC}" -c -Fo${PLATFORM}/obj/ecModuleWrite.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecModuleWrite.c

"${CC}" -c -Fo${PLATFORM}/obj/ecParser.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecParser.c

"${CC}" -c -Fo${PLATFORM}/obj/ecState.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecState.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsApp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsApp.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsArray.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsArray.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsBlock.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsBlock.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsBoolean.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsBoolean.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsByteArray.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsByteArray.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsCache.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsCache.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsCmd.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsCmd.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsConfig.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsConfig.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsDate.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsDate.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsDebug.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsDebug.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsError.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsError.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsFile.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFile.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsFileSystem.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFileSystem.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsFrame.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFrame.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsFunction.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFunction.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsGC.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsGC.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsGlobal.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsGlobal.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsHttp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsHttp.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsIterator.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsIterator.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsJSON.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsJSON.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsLocalCache.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsLocalCache.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsMath.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMath.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsMemory.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMemory.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsMprLog.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMprLog.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsNamespace.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNamespace.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsNull.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNull.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsNumber.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNumber.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsObject.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsObject.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsPath.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsPath.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsPot.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsPot.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsRegExp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsRegExp.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsSocket.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsSocket.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsString.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsString.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsSystem.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsSystem.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsTimer.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsTimer.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsType.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsType.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsUri.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsUri.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsVoid.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsVoid.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsWorker.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsWorker.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsXML.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXML.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsXMLList.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXMLList.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsXMLLoader.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXMLLoader.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsByteCode.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsByteCode.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsException.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsException.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsHelper.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsHelper.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsInterp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsInterp.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsLoader.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsLoader.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsModule.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsModule.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsScope.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsScope.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsService.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsService.c

"link" -dll -out:${PLATFORM}/bin/libejs.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libejs.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/ecAst.obj ${PLATFORM}/obj/ecCodeGen.obj ${PLATFORM}/obj/ecCompiler.obj ${PLATFORM}/obj/ecLex.obj ${PLATFORM}/obj/ecModuleWrite.obj ${PLATFORM}/obj/ecParser.obj ${PLATFORM}/obj/ecState.obj ${PLATFORM}/obj/ejsApp.obj ${PLATFORM}/obj/ejsArray.obj ${PLATFORM}/obj/ejsBlock.obj ${PLATFORM}/obj/ejsBoolean.obj ${PLATFORM}/obj/ejsByteArray.obj ${PLATFORM}/obj/ejsCache.obj ${PLATFORM}/obj/ejsCmd.obj ${PLATFORM}/obj/ejsConfig.obj ${PLATFORM}/obj/ejsDate.obj ${PLATFORM}/obj/ejsDebug.obj ${PLATFORM}/obj/ejsError.obj ${PLATFORM}/obj/ejsFile.obj ${PLATFORM}/obj/ejsFileSystem.obj ${PLATFORM}/obj/ejsFrame.obj ${PLATFORM}/obj/ejsFunction.obj ${PLATFORM}/obj/ejsGC.obj ${PLATFORM}/obj/ejsGlobal.obj ${PLATFORM}/obj/ejsHttp.obj ${PLATFORM}/obj/ejsIterator.obj ${PLATFORM}/obj/ejsJSON.obj ${PLATFORM}/obj/ejsLocalCache.obj ${PLATFORM}/obj/ejsMath.obj ${PLATFORM}/obj/ejsMemory.obj ${PLATFORM}/obj/ejsMprLog.obj ${PLATFORM}/obj/ejsNamespace.obj ${PLATFORM}/obj/ejsNull.obj ${PLATFORM}/obj/ejsNumber.obj ${PLATFORM}/obj/ejsObject.obj ${PLATFORM}/obj/ejsPath.obj ${PLATFORM}/obj/ejsPot.obj ${PLATFORM}/obj/ejsRegExp.obj ${PLATFORM}/obj/ejsSocket.obj ${PLATFORM}/obj/ejsString.obj ${PLATFORM}/obj/ejsSystem.obj ${PLATFORM}/obj/ejsTimer.obj ${PLATFORM}/obj/ejsType.obj ${PLATFORM}/obj/ejsUri.obj ${PLATFORM}/obj/ejsVoid.obj ${PLATFORM}/obj/ejsWorker.obj ${PLATFORM}/obj/ejsXML.obj ${PLATFORM}/obj/ejsXMLList.obj ${PLATFORM}/obj/ejsXMLLoader.obj ${PLATFORM}/obj/ejsByteCode.obj ${PLATFORM}/obj/ejsException.obj ${PLATFORM}/obj/ejsHelper.obj ${PLATFORM}/obj/ejsInterp.obj ${PLATFORM}/obj/ejsLoader.obj ${PLATFORM}/obj/ejsModule.obj ${PLATFORM}/obj/ejsScope.obj ${PLATFORM}/obj/ejsService.obj ${LIBS} mpr.lib pcre.lib http.lib

"${CC}" -c -Fo${PLATFORM}/obj/ejs.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejs.c

"link" -out:${PLATFORM}/bin/ejs.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/ejs.obj ${LIBS} ejs.lib mpr.lib pcre.lib http.lib

"${CC}" -c -Fo${PLATFORM}/obj/ejsc.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejsc.c

"link" -out:${PLATFORM}/bin/ejsc.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/ejsc.obj ${LIBS} ejs.lib mpr.lib pcre.lib http.lib

"${CC}" -c -Fo${PLATFORM}/obj/ejsmod.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/ejsmod.c

"${CC}" -c -Fo${PLATFORM}/obj/doc.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/doc.c

"${CC}" -c -Fo${PLATFORM}/obj/docFiles.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/docFiles.c

"${CC}" -c -Fo${PLATFORM}/obj/listing.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/listing.c

"${CC}" -c -Fo${PLATFORM}/obj/slotGen.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/slotGen.c

"link" -out:${PLATFORM}/bin/ejsmod.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/ejsmod.obj ${PLATFORM}/obj/doc.obj ${PLATFORM}/obj/docFiles.obj ${PLATFORM}/obj/listing.obj ${PLATFORM}/obj/slotGen.obj ${LIBS} ejs.lib mpr.lib pcre.lib http.lib

"${CC}" -c -Fo${PLATFORM}/obj/ejsrun.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejsrun.c

"link" -out:${PLATFORM}/bin/ejsrun.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/ejsrun.obj ${LIBS} ejs.lib mpr.lib pcre.lib http.lib

ejsc --out /Users/mob/git/ejs/win-i686-debug/bin/ejs.mod --debug --optimize 9 --bind --require null /Users/mob/git/ejs/src/core/*.es 
ejsmod --require null --listing --cslots /Users/mob/git/ejs/win-i686-debug/bin/ejs.mod
cp ejs.slots.h /Users/mob/git/ejs/win-i686-debug/inc
cp /Users/mob/git/ejs/src/jems/ejs.bit/bit.es /Users/mob/git/ejs/win-i686-debug/bin
rm -fr /Users/mob/git/ejs/win-i686-debug/bin/bits
cp -r /Users/mob/git/ejs/src/jems/ejs.bit/bits /Users/mob/git/ejs/win-i686-debug/bin
ejsc --out /Users/mob/git/ejs/win-i686-debug/bin/ejs.unix.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.unix/Unix.es
cp /Users/mob/git/ejs/src/jems/ejs.jem/jem.es /Users/mob/git/ejs/win-i686-debug/bin
ejsc --out /Users/mob/git/ejs/win-i686-debug/bin/ejs.db.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.db/*.es
ejsc --out /Users/mob/git/ejs/win-i686-debug/bin/ejs.db.mapper.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.db.mapper/*.es
ejsc --out /Users/mob/git/ejs/win-i686-debug/bin/ejs.db.sqlite.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.db.sqlite/*.es
"${CC}" -c -Fo${PLATFORM}/obj/ejsSqlite.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/jems/ejs.db.sqlite/src/ejsSqlite.c

"link" -dll -out:${PLATFORM}/bin/ejs.db.sqlite.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/ejs.db.sqlite.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/ejsSqlite.obj ${LIBS} mpr.lib ejs.lib pcre.lib http.lib sqlite3.lib

ejsc --out /Users/mob/git/ejs/win-i686-debug/bin/ejs.web.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.web/*.es
ejsmod --cslots /Users/mob/git/ejs/win-i686-debug/bin/ejs.web.mod
cp ejs.web.slots.h /Users/mob/git/ejs/win-i686-debug/inc
"${CC}" -c -Fo${PLATFORM}/obj/ejsHttpServer.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsHttpServer.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsRequest.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsRequest.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsSession.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsSession.c

"${CC}" -c -Fo${PLATFORM}/obj/ejsWeb.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsWeb.c

"link" -dll -out:${PLATFORM}/bin/ejs.web.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/ejs.web.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/ejsHttpServer.obj ${PLATFORM}/obj/ejsRequest.obj ${PLATFORM}/obj/ejsSession.obj ${PLATFORM}/obj/ejsWeb.obj ${LIBS} mpr.lib http.lib pcre.lib pcre.lib ejs.lib

ejsc --out /Users/mob/git/ejs/win-i686-debug/bin/ejs.template.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.template/*.es
ejsc --out /Users/mob/git/ejs/win-i686-debug/bin/ejs.tar.mod/ --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.tar/*.es
cp /Users/mob/git/ejs/src/jems/ejs.mvc/mvc.es /Users/mob/git/ejs/win-i686-debug/bin
ejsc --out /Users/mob/git/ejs/win-i686-debug/bin/ejs.mvc.mod/ --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.mvc/*.es
