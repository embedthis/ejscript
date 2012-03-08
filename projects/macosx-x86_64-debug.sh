#
#   build.sh -- Build It Shell Script to build Embedthis Ejscript
#

PLATFORM="macosx-x86_64-debug"
CC="/usr/bin/cc"
CFLAGS="-fPIC -Wall -g -Wshorten-64-to-32"
DFLAGS="-DPIC -DCPU=X86_64"
IFLAGS="-Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots"
LDFLAGS="-Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L/Users/mob/git/ejs/${PLATFORM}/lib -g -ldl"
LIBS="-lpthread -lm"

${CC} -c -o ${PLATFORM}/obj/mprLib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/mprLib.c

${CC} -dynamiclib -o ${PLATFORM}/lib/libmpr.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -install_name @rpath/libmpr.dylib ${PLATFORM}/obj/mprLib.o ${LIBS}

${CC} -c -o ${PLATFORM}/obj/manager.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/manager.c

${CC} -o ${PLATFORM}/bin/manager -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/manager.o ${LIBS} -lmpr

${CC} -c -o ${PLATFORM}/obj/makerom.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/makerom.c

${CC} -o ${PLATFORM}/bin/makerom -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/makerom.o ${LIBS} -lmpr

${CC} -c -o ${PLATFORM}/obj/pcre.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/pcre/pcre.c

${CC} -dynamiclib -o ${PLATFORM}/lib/libpcre.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -install_name @rpath/libpcre.dylib ${PLATFORM}/obj/pcre.o ${LIBS}

${CC} -c -o ${PLATFORM}/obj/httpLib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/http/httpLib.c

${CC} -dynamiclib -o ${PLATFORM}/lib/libhttp.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -install_name @rpath/libhttp.dylib ${PLATFORM}/obj/httpLib.o ${LIBS} -lmpr -lpcre

${CC} -c -o ${PLATFORM}/obj/http.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/http/http.c

${CC} -o ${PLATFORM}/bin/http -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/http.o ${LIBS} -lhttp -lmpr -lpcre

${CC} -c -o ${PLATFORM}/obj/sqlite3.o -arch x86_64 -fPIC -g ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/sqlite/sqlite3.c

${CC} -dynamiclib -o ${PLATFORM}/lib/libsqlite3.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -install_name @rpath/libsqlite3.dylib ${PLATFORM}/obj/sqlite3.o ${LIBS}

cp /Users/mob/git/ejs/src/slots/*.h /Users/mob/git/ejs/macosx-x86_64-debug/inc
${CC} -c -o ${PLATFORM}/obj/ecAst.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecAst.c

${CC} -c -o ${PLATFORM}/obj/ecCodeGen.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecCodeGen.c

${CC} -c -o ${PLATFORM}/obj/ecCompiler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecCompiler.c

${CC} -c -o ${PLATFORM}/obj/ecLex.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecLex.c

${CC} -c -o ${PLATFORM}/obj/ecModuleWrite.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecModuleWrite.c

${CC} -c -o ${PLATFORM}/obj/ecParser.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecParser.c

${CC} -c -o ${PLATFORM}/obj/ecState.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecState.c

${CC} -c -o ${PLATFORM}/obj/ejsApp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsApp.c

${CC} -c -o ${PLATFORM}/obj/ejsArray.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsArray.c

${CC} -c -o ${PLATFORM}/obj/ejsBlock.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsBlock.c

${CC} -c -o ${PLATFORM}/obj/ejsBoolean.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsBoolean.c

${CC} -c -o ${PLATFORM}/obj/ejsByteArray.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsByteArray.c

${CC} -c -o ${PLATFORM}/obj/ejsCache.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsCache.c

${CC} -c -o ${PLATFORM}/obj/ejsCmd.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsCmd.c

${CC} -c -o ${PLATFORM}/obj/ejsConfig.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsConfig.c

${CC} -c -o ${PLATFORM}/obj/ejsDate.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsDate.c

${CC} -c -o ${PLATFORM}/obj/ejsDebug.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsDebug.c

${CC} -c -o ${PLATFORM}/obj/ejsError.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsError.c

${CC} -c -o ${PLATFORM}/obj/ejsFile.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFile.c

${CC} -c -o ${PLATFORM}/obj/ejsFileSystem.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFileSystem.c

${CC} -c -o ${PLATFORM}/obj/ejsFrame.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFrame.c

${CC} -c -o ${PLATFORM}/obj/ejsFunction.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFunction.c

${CC} -c -o ${PLATFORM}/obj/ejsGC.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsGC.c

${CC} -c -o ${PLATFORM}/obj/ejsGlobal.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsGlobal.c

${CC} -c -o ${PLATFORM}/obj/ejsHttp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsHttp.c

${CC} -c -o ${PLATFORM}/obj/ejsIterator.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsIterator.c

${CC} -c -o ${PLATFORM}/obj/ejsJSON.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsJSON.c

${CC} -c -o ${PLATFORM}/obj/ejsLocalCache.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsLocalCache.c

${CC} -c -o ${PLATFORM}/obj/ejsMath.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMath.c

${CC} -c -o ${PLATFORM}/obj/ejsMemory.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMemory.c

${CC} -c -o ${PLATFORM}/obj/ejsMprLog.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMprLog.c

${CC} -c -o ${PLATFORM}/obj/ejsNamespace.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNamespace.c

${CC} -c -o ${PLATFORM}/obj/ejsNull.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNull.c

${CC} -c -o ${PLATFORM}/obj/ejsNumber.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNumber.c

${CC} -c -o ${PLATFORM}/obj/ejsObject.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsObject.c

${CC} -c -o ${PLATFORM}/obj/ejsPath.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsPath.c

${CC} -c -o ${PLATFORM}/obj/ejsPot.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsPot.c

${CC} -c -o ${PLATFORM}/obj/ejsRegExp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsRegExp.c

${CC} -c -o ${PLATFORM}/obj/ejsSocket.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsSocket.c

${CC} -c -o ${PLATFORM}/obj/ejsString.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsString.c

${CC} -c -o ${PLATFORM}/obj/ejsSystem.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsSystem.c

${CC} -c -o ${PLATFORM}/obj/ejsTimer.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsTimer.c

${CC} -c -o ${PLATFORM}/obj/ejsType.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsType.c

${CC} -c -o ${PLATFORM}/obj/ejsUri.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsUri.c

${CC} -c -o ${PLATFORM}/obj/ejsVoid.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsVoid.c

${CC} -c -o ${PLATFORM}/obj/ejsWorker.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsWorker.c

${CC} -c -o ${PLATFORM}/obj/ejsXML.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXML.c

${CC} -c -o ${PLATFORM}/obj/ejsXMLList.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXMLList.c

${CC} -c -o ${PLATFORM}/obj/ejsXMLLoader.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXMLLoader.c

${CC} -c -o ${PLATFORM}/obj/ejsByteCode.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsByteCode.c

${CC} -c -o ${PLATFORM}/obj/ejsException.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsException.c

${CC} -c -o ${PLATFORM}/obj/ejsHelper.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsHelper.c

${CC} -c -o ${PLATFORM}/obj/ejsInterp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsInterp.c

${CC} -c -o ${PLATFORM}/obj/ejsLoader.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsLoader.c

${CC} -c -o ${PLATFORM}/obj/ejsModule.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsModule.c

${CC} -c -o ${PLATFORM}/obj/ejsScope.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsScope.c

${CC} -c -o ${PLATFORM}/obj/ejsService.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsService.c

${CC} -dynamiclib -o ${PLATFORM}/lib/libejs.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -install_name @rpath/libejs.dylib ${PLATFORM}/obj/ecAst.o ${PLATFORM}/obj/ecCodeGen.o ${PLATFORM}/obj/ecCompiler.o ${PLATFORM}/obj/ecLex.o ${PLATFORM}/obj/ecModuleWrite.o ${PLATFORM}/obj/ecParser.o ${PLATFORM}/obj/ecState.o ${PLATFORM}/obj/ejsApp.o ${PLATFORM}/obj/ejsArray.o ${PLATFORM}/obj/ejsBlock.o ${PLATFORM}/obj/ejsBoolean.o ${PLATFORM}/obj/ejsByteArray.o ${PLATFORM}/obj/ejsCache.o ${PLATFORM}/obj/ejsCmd.o ${PLATFORM}/obj/ejsConfig.o ${PLATFORM}/obj/ejsDate.o ${PLATFORM}/obj/ejsDebug.o ${PLATFORM}/obj/ejsError.o ${PLATFORM}/obj/ejsFile.o ${PLATFORM}/obj/ejsFileSystem.o ${PLATFORM}/obj/ejsFrame.o ${PLATFORM}/obj/ejsFunction.o ${PLATFORM}/obj/ejsGC.o ${PLATFORM}/obj/ejsGlobal.o ${PLATFORM}/obj/ejsHttp.o ${PLATFORM}/obj/ejsIterator.o ${PLATFORM}/obj/ejsJSON.o ${PLATFORM}/obj/ejsLocalCache.o ${PLATFORM}/obj/ejsMath.o ${PLATFORM}/obj/ejsMemory.o ${PLATFORM}/obj/ejsMprLog.o ${PLATFORM}/obj/ejsNamespace.o ${PLATFORM}/obj/ejsNull.o ${PLATFORM}/obj/ejsNumber.o ${PLATFORM}/obj/ejsObject.o ${PLATFORM}/obj/ejsPath.o ${PLATFORM}/obj/ejsPot.o ${PLATFORM}/obj/ejsRegExp.o ${PLATFORM}/obj/ejsSocket.o ${PLATFORM}/obj/ejsString.o ${PLATFORM}/obj/ejsSystem.o ${PLATFORM}/obj/ejsTimer.o ${PLATFORM}/obj/ejsType.o ${PLATFORM}/obj/ejsUri.o ${PLATFORM}/obj/ejsVoid.o ${PLATFORM}/obj/ejsWorker.o ${PLATFORM}/obj/ejsXML.o ${PLATFORM}/obj/ejsXMLList.o ${PLATFORM}/obj/ejsXMLLoader.o ${PLATFORM}/obj/ejsByteCode.o ${PLATFORM}/obj/ejsException.o ${PLATFORM}/obj/ejsHelper.o ${PLATFORM}/obj/ejsInterp.o ${PLATFORM}/obj/ejsLoader.o ${PLATFORM}/obj/ejsModule.o ${PLATFORM}/obj/ejsScope.o ${PLATFORM}/obj/ejsService.o ${LIBS} -lmpr -lpcre -lhttp

${CC} -c -o ${PLATFORM}/obj/ejs.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejs.c

${CC} -o ${PLATFORM}/bin/ejs -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/ejs.o ${LIBS} -lejs -lmpr -lpcre -lhttp -ledit

${CC} -c -o ${PLATFORM}/obj/ejsc.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejsc.c

${CC} -o ${PLATFORM}/bin/ejsc -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/ejsc.o ${LIBS} -lejs -lmpr -lpcre -lhttp

${CC} -c -o ${PLATFORM}/obj/ejsmod.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/ejsmod.c

${CC} -c -o ${PLATFORM}/obj/doc.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/doc.c

${CC} -c -o ${PLATFORM}/obj/docFiles.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/docFiles.c

${CC} -c -o ${PLATFORM}/obj/listing.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/listing.c

${CC} -c -o ${PLATFORM}/obj/slotGen.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/slotGen.c

${CC} -o ${PLATFORM}/bin/ejsmod -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/ejsmod.o ${PLATFORM}/obj/doc.o ${PLATFORM}/obj/docFiles.o ${PLATFORM}/obj/listing.o ${PLATFORM}/obj/slotGen.o ${LIBS} -lejs -lmpr -lpcre -lhttp

${CC} -c -o ${PLATFORM}/obj/ejsrun.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejsrun.c

${CC} -o ${PLATFORM}/bin/ejsrun -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/ejsrun.o ${LIBS} -lejs -lmpr -lpcre -lhttp

ejsc --out /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.mod --debug --optimize 9 --bind --require null /Users/mob/git/ejs/src/core/*.es 
ejsmod --require null --listing --cslots /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.mod
cp ejs.slots.h /Users/mob/git/ejs/macosx-x86_64-debug/inc
cp /Users/mob/git/ejs/src/jems/ejs.bit/bit.es /Users/mob/git/ejs/macosx-x86_64-debug/bin
rm -fr /Users/mob/git/ejs/macosx-x86_64-debug/lib/bits
cp -r /Users/mob/git/ejs/src/jems/ejs.bit/bits /Users/mob/git/ejs/macosx-x86_64-debug/lib
ejsc --out /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.unix.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.unix/Unix.es
cp /Users/mob/git/ejs/src/jems/ejs.jem/jem.es /Users/mob/git/ejs/macosx-x86_64-debug/bin
ejsc --out /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.db.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.db/*.es
ejsc --out /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.db.mapper.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.db.mapper/*.es
ejsc --out /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.db.sqlite.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.db.sqlite/*.es
${CC} -c -o ${PLATFORM}/obj/ejsSqlite.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/jems/ejs.db.sqlite/src/ejsSqlite.c

${CC} -dynamiclib -o ${PLATFORM}/lib/ejs.db.sqlite.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -install_name @rpath/ejs.db.sqlite.dylib ${PLATFORM}/obj/ejsSqlite.o ${LIBS} -lmpr -lejs -lpcre -lhttp -lsqlite3

ejsc --out /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.web.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.web/*.es
ejsmod --cslots /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.web.mod
cp ejs.web.slots.h /Users/mob/git/ejs/macosx-x86_64-debug/inc
${CC} -c -o ${PLATFORM}/obj/ejsHttpServer.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsHttpServer.c

${CC} -c -o ${PLATFORM}/obj/ejsRequest.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsRequest.c

${CC} -c -o ${PLATFORM}/obj/ejsSession.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsSession.c

${CC} -c -o ${PLATFORM}/obj/ejsWeb.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsWeb.c

${CC} -dynamiclib -o ${PLATFORM}/lib/ejs.web.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -install_name @rpath/ejs.web.dylib ${PLATFORM}/obj/ejsHttpServer.o ${PLATFORM}/obj/ejsRequest.o ${PLATFORM}/obj/ejsSession.o ${PLATFORM}/obj/ejsWeb.o ${LIBS} -lmpr -lhttp -lpcre -lpcre -lejs

ejsc --out /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.template.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.template/*.es
ejsc --out /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.tar.mod/ --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.tar/*.es
ejsc --out /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.zlib.mod/ --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.zlib/*.es
${CC} -c -o ${PLATFORM}/obj/ejsZlib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -I../packages-macosx-x86_64/zlib/zlib-1.2.6 src/jems/ejs.zlib/src/ejsZlib.c

${CC} -dynamiclib -o ${PLATFORM}/lib/ejs.zlib.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -ldl -L/usr/lib -install_name @rpath/ejs.zlib.dylib ${PLATFORM}/obj/ejsZlib.o ${LIBS} -lmpr -lejs -lpcre -lhttp -lz

cp /Users/mob/git/ejs/src/jems/ejs.mvc/mvc.es /Users/mob/git/ejs/macosx-x86_64-debug/bin
ejsc --out /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.mvc.mod/ --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.mvc/*.es
