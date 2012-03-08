#
#   build.sh -- Build It Shell Script to build Embedthis Ejscript
#

PLATFORM="linux-i686-debug"
CC="cc"
CFLAGS="-fPIC -g -mcpu=i686"
DFLAGS="-DPIC"
IFLAGS="-Ilinux-i686-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots"
LDFLAGS="-L/Users/mob/git/ejs/${PLATFORM}/lib -g"
LIBS="-lpthread -lm"

${CC} -c -o ${PLATFORM}/obj/mprLib.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/mprLib.c

${CC} -shared -o ${PLATFORM}/lib/libmpr.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/mprLib.o ${LIBS}

${CC} -c -o ${PLATFORM}/obj/manager.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/manager.c

${CC} -o ${PLATFORM}/bin/manager -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/manager.o ${LIBS} -lmpr -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/makerom.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/makerom.c

${CC} -o ${PLATFORM}/bin/makerom -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/makerom.o ${LIBS} -lmpr -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/pcre.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/pcre/pcre.c

${CC} -shared -o ${PLATFORM}/lib/libpcre.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/pcre.o ${LIBS}

${CC} -c -o ${PLATFORM}/obj/httpLib.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/http/httpLib.c

${CC} -shared -o ${PLATFORM}/lib/libhttp.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/httpLib.o ${LIBS} -lmpr -lpcre

${CC} -c -o ${PLATFORM}/obj/http.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/http/http.c

${CC} -o ${PLATFORM}/bin/http -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/http.o ${LIBS} -lhttp -lmpr -lpcre -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/sqlite3.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/sqlite/sqlite3.c

${CC} -shared -o ${PLATFORM}/lib/libsqlite3.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/sqlite3.o ${LIBS}

cp /Users/mob/git/ejs/src/slots/*.h /Users/mob/git/ejs/linux-i686-debug/inc
${CC} -c -o ${PLATFORM}/obj/ecAst.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecAst.c

${CC} -c -o ${PLATFORM}/obj/ecCodeGen.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecCodeGen.c

${CC} -c -o ${PLATFORM}/obj/ecCompiler.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecCompiler.c

${CC} -c -o ${PLATFORM}/obj/ecLex.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecLex.c

${CC} -c -o ${PLATFORM}/obj/ecModuleWrite.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecModuleWrite.c

${CC} -c -o ${PLATFORM}/obj/ecParser.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecParser.c

${CC} -c -o ${PLATFORM}/obj/ecState.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecState.c

${CC} -c -o ${PLATFORM}/obj/ejsApp.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsApp.c

${CC} -c -o ${PLATFORM}/obj/ejsArray.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsArray.c

${CC} -c -o ${PLATFORM}/obj/ejsBlock.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsBlock.c

${CC} -c -o ${PLATFORM}/obj/ejsBoolean.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsBoolean.c

${CC} -c -o ${PLATFORM}/obj/ejsByteArray.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsByteArray.c

${CC} -c -o ${PLATFORM}/obj/ejsCache.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsCache.c

${CC} -c -o ${PLATFORM}/obj/ejsCmd.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsCmd.c

${CC} -c -o ${PLATFORM}/obj/ejsConfig.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsConfig.c

${CC} -c -o ${PLATFORM}/obj/ejsDate.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsDate.c

${CC} -c -o ${PLATFORM}/obj/ejsDebug.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsDebug.c

${CC} -c -o ${PLATFORM}/obj/ejsError.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsError.c

${CC} -c -o ${PLATFORM}/obj/ejsFile.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFile.c

${CC} -c -o ${PLATFORM}/obj/ejsFileSystem.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFileSystem.c

${CC} -c -o ${PLATFORM}/obj/ejsFrame.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFrame.c

${CC} -c -o ${PLATFORM}/obj/ejsFunction.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFunction.c

${CC} -c -o ${PLATFORM}/obj/ejsGC.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsGC.c

${CC} -c -o ${PLATFORM}/obj/ejsGlobal.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsGlobal.c

${CC} -c -o ${PLATFORM}/obj/ejsHttp.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsHttp.c

${CC} -c -o ${PLATFORM}/obj/ejsIterator.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsIterator.c

${CC} -c -o ${PLATFORM}/obj/ejsJSON.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsJSON.c

${CC} -c -o ${PLATFORM}/obj/ejsLocalCache.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsLocalCache.c

${CC} -c -o ${PLATFORM}/obj/ejsMath.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMath.c

${CC} -c -o ${PLATFORM}/obj/ejsMemory.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMemory.c

${CC} -c -o ${PLATFORM}/obj/ejsMprLog.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMprLog.c

${CC} -c -o ${PLATFORM}/obj/ejsNamespace.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNamespace.c

${CC} -c -o ${PLATFORM}/obj/ejsNull.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNull.c

${CC} -c -o ${PLATFORM}/obj/ejsNumber.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNumber.c

${CC} -c -o ${PLATFORM}/obj/ejsObject.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsObject.c

${CC} -c -o ${PLATFORM}/obj/ejsPath.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsPath.c

${CC} -c -o ${PLATFORM}/obj/ejsPot.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsPot.c

${CC} -c -o ${PLATFORM}/obj/ejsRegExp.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsRegExp.c

${CC} -c -o ${PLATFORM}/obj/ejsSocket.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsSocket.c

${CC} -c -o ${PLATFORM}/obj/ejsString.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsString.c

${CC} -c -o ${PLATFORM}/obj/ejsSystem.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsSystem.c

${CC} -c -o ${PLATFORM}/obj/ejsTimer.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsTimer.c

${CC} -c -o ${PLATFORM}/obj/ejsType.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsType.c

${CC} -c -o ${PLATFORM}/obj/ejsUri.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsUri.c

${CC} -c -o ${PLATFORM}/obj/ejsVoid.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsVoid.c

${CC} -c -o ${PLATFORM}/obj/ejsWorker.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsWorker.c

${CC} -c -o ${PLATFORM}/obj/ejsXML.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXML.c

${CC} -c -o ${PLATFORM}/obj/ejsXMLList.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXMLList.c

${CC} -c -o ${PLATFORM}/obj/ejsXMLLoader.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXMLLoader.c

${CC} -c -o ${PLATFORM}/obj/ejsByteCode.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsByteCode.c

${CC} -c -o ${PLATFORM}/obj/ejsException.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsException.c

${CC} -c -o ${PLATFORM}/obj/ejsHelper.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsHelper.c

${CC} -c -o ${PLATFORM}/obj/ejsInterp.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsInterp.c

${CC} -c -o ${PLATFORM}/obj/ejsLoader.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsLoader.c

${CC} -c -o ${PLATFORM}/obj/ejsModule.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsModule.c

${CC} -c -o ${PLATFORM}/obj/ejsScope.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsScope.c

${CC} -c -o ${PLATFORM}/obj/ejsService.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsService.c

${CC} -shared -o ${PLATFORM}/lib/libejs.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/ecAst.o ${PLATFORM}/obj/ecCodeGen.o ${PLATFORM}/obj/ecCompiler.o ${PLATFORM}/obj/ecLex.o ${PLATFORM}/obj/ecModuleWrite.o ${PLATFORM}/obj/ecParser.o ${PLATFORM}/obj/ecState.o ${PLATFORM}/obj/ejsApp.o ${PLATFORM}/obj/ejsArray.o ${PLATFORM}/obj/ejsBlock.o ${PLATFORM}/obj/ejsBoolean.o ${PLATFORM}/obj/ejsByteArray.o ${PLATFORM}/obj/ejsCache.o ${PLATFORM}/obj/ejsCmd.o ${PLATFORM}/obj/ejsConfig.o ${PLATFORM}/obj/ejsDate.o ${PLATFORM}/obj/ejsDebug.o ${PLATFORM}/obj/ejsError.o ${PLATFORM}/obj/ejsFile.o ${PLATFORM}/obj/ejsFileSystem.o ${PLATFORM}/obj/ejsFrame.o ${PLATFORM}/obj/ejsFunction.o ${PLATFORM}/obj/ejsGC.o ${PLATFORM}/obj/ejsGlobal.o ${PLATFORM}/obj/ejsHttp.o ${PLATFORM}/obj/ejsIterator.o ${PLATFORM}/obj/ejsJSON.o ${PLATFORM}/obj/ejsLocalCache.o ${PLATFORM}/obj/ejsMath.o ${PLATFORM}/obj/ejsMemory.o ${PLATFORM}/obj/ejsMprLog.o ${PLATFORM}/obj/ejsNamespace.o ${PLATFORM}/obj/ejsNull.o ${PLATFORM}/obj/ejsNumber.o ${PLATFORM}/obj/ejsObject.o ${PLATFORM}/obj/ejsPath.o ${PLATFORM}/obj/ejsPot.o ${PLATFORM}/obj/ejsRegExp.o ${PLATFORM}/obj/ejsSocket.o ${PLATFORM}/obj/ejsString.o ${PLATFORM}/obj/ejsSystem.o ${PLATFORM}/obj/ejsTimer.o ${PLATFORM}/obj/ejsType.o ${PLATFORM}/obj/ejsUri.o ${PLATFORM}/obj/ejsVoid.o ${PLATFORM}/obj/ejsWorker.o ${PLATFORM}/obj/ejsXML.o ${PLATFORM}/obj/ejsXMLList.o ${PLATFORM}/obj/ejsXMLLoader.o ${PLATFORM}/obj/ejsByteCode.o ${PLATFORM}/obj/ejsException.o ${PLATFORM}/obj/ejsHelper.o ${PLATFORM}/obj/ejsInterp.o ${PLATFORM}/obj/ejsLoader.o ${PLATFORM}/obj/ejsModule.o ${PLATFORM}/obj/ejsScope.o ${PLATFORM}/obj/ejsService.o ${LIBS} -lmpr -lpcre -lhttp

${CC} -c -o ${PLATFORM}/obj/ejs.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejs.c

${CC} -o ${PLATFORM}/bin/ejs -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/ejs.o ${LIBS} -lejs -lmpr -lpcre -lhttp -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/ejsc.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejsc.c

${CC} -o ${PLATFORM}/bin/ejsc -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/ejsc.o ${LIBS} -lejs -lmpr -lpcre -lhttp -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/ejsmod.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/ejsmod.c

${CC} -c -o ${PLATFORM}/obj/doc.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/doc.c

${CC} -c -o ${PLATFORM}/obj/docFiles.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/docFiles.c

${CC} -c -o ${PLATFORM}/obj/listing.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/listing.c

${CC} -c -o ${PLATFORM}/obj/slotGen.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/slotGen.c

${CC} -o ${PLATFORM}/bin/ejsmod -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/ejsmod.o ${PLATFORM}/obj/doc.o ${PLATFORM}/obj/docFiles.o ${PLATFORM}/obj/listing.o ${PLATFORM}/obj/slotGen.o ${LIBS} -lejs -lmpr -lpcre -lhttp -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/ejsrun.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejsrun.c

${CC} -o ${PLATFORM}/bin/ejsrun -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/ejsrun.o ${LIBS} -lejs -lmpr -lpcre -lhttp -L${PLATFORM}/lib -g

ejsc --out /Users/mob/git/ejs/linux-i686-debug/lib/ejs.mod --debug --optimize 9 --bind --require null /Users/mob/git/ejs/src/core/*.es 
ejsmod --require null --listing --cslots /Users/mob/git/ejs/linux-i686-debug/lib/ejs.mod
cp ejs.slots.h /Users/mob/git/ejs/linux-i686-debug/inc
cp /Users/mob/git/ejs/src/jems/ejs.bit/bit.es /Users/mob/git/ejs/linux-i686-debug/bin
rm -fr /Users/mob/git/ejs/linux-i686-debug/lib/bits
cp -r /Users/mob/git/ejs/src/jems/ejs.bit/bits /Users/mob/git/ejs/linux-i686-debug/lib
ejsc --out /Users/mob/git/ejs/linux-i686-debug/lib/ejs.unix.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.unix/Unix.es
cp /Users/mob/git/ejs/src/jems/ejs.jem/jem.es /Users/mob/git/ejs/linux-i686-debug/bin
ejsc --out /Users/mob/git/ejs/linux-i686-debug/lib/ejs.db.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.db/*.es
ejsc --out /Users/mob/git/ejs/linux-i686-debug/lib/ejs.db.mapper.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.db.mapper/*.es
ejsc --out /Users/mob/git/ejs/linux-i686-debug/lib/ejs.db.sqlite.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.db.sqlite/*.es
${CC} -c -o ${PLATFORM}/obj/ejsSqlite.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/jems/ejs.db.sqlite/src/ejsSqlite.c

${CC} -shared -o ${PLATFORM}/lib/ejs.db.sqlite.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/ejsSqlite.o ${LIBS} -lmpr -lejs -lpcre -lhttp -lsqlite3

ejsc --out /Users/mob/git/ejs/linux-i686-debug/lib/ejs.web.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.web/*.es
ejsmod --cslots /Users/mob/git/ejs/linux-i686-debug/lib/ejs.web.mod
cp ejs.web.slots.h /Users/mob/git/ejs/linux-i686-debug/inc
${CC} -c -o ${PLATFORM}/obj/ejsHttpServer.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsHttpServer.c

${CC} -c -o ${PLATFORM}/obj/ejsRequest.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsRequest.c

${CC} -c -o ${PLATFORM}/obj/ejsSession.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsSession.c

${CC} -c -o ${PLATFORM}/obj/ejsWeb.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsWeb.c

${CC} -shared -o ${PLATFORM}/lib/ejs.web.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/ejsHttpServer.o ${PLATFORM}/obj/ejsRequest.o ${PLATFORM}/obj/ejsSession.o ${PLATFORM}/obj/ejsWeb.o ${LIBS} -lmpr -lhttp -lpcre -lpcre -lejs

ejsc --out /Users/mob/git/ejs/linux-i686-debug/lib/ejs.template.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.template/*.es
ejsc --out /Users/mob/git/ejs/linux-i686-debug/lib/ejs.tar.mod/ --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.tar/*.es
cp /Users/mob/git/ejs/src/jems/ejs.mvc/mvc.es /Users/mob/git/ejs/linux-i686-debug/bin
ejsc --out /Users/mob/git/ejs/linux-i686-debug/lib/ejs.mvc.mod/ --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.mvc/*.es
