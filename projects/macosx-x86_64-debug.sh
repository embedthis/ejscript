#
#   macosx-x86_64-debug.sh -- Build It Shell Script to build Embedthis Ejscript
#

CONFIG="macosx-x86_64-debug"
CC="/usr/bin/cc"
LD="/usr/bin/ld"
CFLAGS="-fPIC -Wall -fast -Wshorten-64-to-32"
DFLAGS="-DPIC -DCPU=X86_64"
IFLAGS="-Imacosx-x86_64-debug/inc -Imacosx-x86_64-debug/inc"
LDFLAGS="-Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/"
LIBPATHS="-L${CONFIG}/lib"
LIBS="-lpthread -lm -ldl"

[ ! -x ${CONFIG}/inc ] && mkdir -p ${CONFIG}/inc ${CONFIG}/obj ${CONFIG}/lib ${CONFIG}/bin
cp projects/buildConfig.${CONFIG} ${CONFIG}/inc/buildConfig.h

rm -rf macosx-x86_64-debug/inc/mpr.h
cp -r src/deps/mpr/mpr.h macosx-x86_64-debug/inc/mpr.h

rm -rf macosx-x86_64-debug/inc/mprSsl.h
cp -r src/deps/mpr/mprSsl.h macosx-x86_64-debug/inc/mprSsl.h

${CC} -c -o ${CONFIG}/obj/mprLib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/mpr/mprLib.c

${CC} -dynamiclib -o ${CONFIG}/lib/libmpr.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/libmpr.dylib ${CONFIG}/obj/mprLib.o ${LIBS}

${CC} -c -o ${CONFIG}/obj/manager.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/mpr/manager.c

${CC} -o ${CONFIG}/bin/ejsman -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/manager.o ${LIBS} -lmpr

${CC} -c -o ${CONFIG}/obj/makerom.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/mpr/makerom.c

${CC} -o ${CONFIG}/bin/makerom -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/makerom.o ${LIBS} -lmpr

rm -rf macosx-x86_64-debug/inc/pcre.h
cp -r src/deps/pcre/pcre.h macosx-x86_64-debug/inc/pcre.h

${CC} -c -o ${CONFIG}/obj/pcre.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/pcre/pcre.c

${CC} -dynamiclib -o ${CONFIG}/lib/libpcre.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/libpcre.dylib ${CONFIG}/obj/pcre.o ${LIBS}

rm -rf macosx-x86_64-debug/inc/http.h
cp -r src/deps/http/http.h macosx-x86_64-debug/inc/http.h

${CC} -c -o ${CONFIG}/obj/httpLib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/http/httpLib.c

${CC} -dynamiclib -o ${CONFIG}/lib/libhttp.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/libhttp.dylib ${CONFIG}/obj/httpLib.o ${LIBS} -lmpr -lpcre

${CC} -c -o ${CONFIG}/obj/http.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/http/http.c

${CC} -o ${CONFIG}/bin/http -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/http.o ${LIBS} -lhttp -lmpr -lpcre

rm -rf macosx-x86_64-debug/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h macosx-x86_64-debug/inc/sqlite3.h

${CC} -c -o ${CONFIG}/obj/sqlite3.o -arch x86_64 -fPIC -fast ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/sqlite/sqlite3.c

${CC} -dynamiclib -o ${CONFIG}/lib/libsqlite3.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/libsqlite3.dylib ${CONFIG}/obj/sqlite3.o ${LIBS}

${CC} -c -o ${CONFIG}/obj/sqlite.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/sqlite/sqlite.c

${CC} -o ${CONFIG}/bin/sqlite -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite.o ${LIBS} -lsqlite3

rm -rf macosx-x86_64-debug/inc/ejs.cache.local.slots.h
cp -r src/slots/ejs.cache.local.slots.h macosx-x86_64-debug/inc/ejs.cache.local.slots.h

rm -rf macosx-x86_64-debug/inc/ejs.db.sqlite.slots.h
cp -r src/slots/ejs.db.sqlite.slots.h macosx-x86_64-debug/inc/ejs.db.sqlite.slots.h

rm -rf macosx-x86_64-debug/inc/ejs.slots.h
cp -r src/slots/ejs.slots.h macosx-x86_64-debug/inc/ejs.slots.h

rm -rf macosx-x86_64-debug/inc/ejs.web.slots.h
cp -r src/slots/ejs.web.slots.h macosx-x86_64-debug/inc/ejs.web.slots.h

rm -rf macosx-x86_64-debug/inc/ejs.zlib.slots.h
cp -r src/slots/ejs.zlib.slots.h macosx-x86_64-debug/inc/ejs.zlib.slots.h

rm -rf macosx-x86_64-debug/inc/ejs.h
cp -r src/ejs.h macosx-x86_64-debug/inc/ejs.h

rm -rf macosx-x86_64-debug/inc/ejsByteCode.h
cp -r src/ejsByteCode.h macosx-x86_64-debug/inc/ejsByteCode.h

rm -rf macosx-x86_64-debug/inc/ejsByteCodeTable.h
cp -r src/ejsByteCodeTable.h macosx-x86_64-debug/inc/ejsByteCodeTable.h

rm -rf macosx-x86_64-debug/inc/ejsCompiler.h
cp -r src/ejsCompiler.h macosx-x86_64-debug/inc/ejsCompiler.h

rm -rf macosx-x86_64-debug/inc/ejsCustomize.h
cp -r src/ejsCustomize.h macosx-x86_64-debug/inc/ejsCustomize.h

${CC} -c -o ${CONFIG}/obj/ecAst.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/compiler/ecAst.c

${CC} -c -o ${CONFIG}/obj/ecCodeGen.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/compiler/ecCodeGen.c

${CC} -c -o ${CONFIG}/obj/ecCompiler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/compiler/ecCompiler.c

${CC} -c -o ${CONFIG}/obj/ecLex.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/compiler/ecLex.c

${CC} -c -o ${CONFIG}/obj/ecModuleWrite.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/compiler/ecModuleWrite.c

${CC} -c -o ${CONFIG}/obj/ecParser.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/compiler/ecParser.c

${CC} -c -o ${CONFIG}/obj/ecState.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/compiler/ecState.c

${CC} -c -o ${CONFIG}/obj/ejsApp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsApp.c

${CC} -c -o ${CONFIG}/obj/ejsArray.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsArray.c

${CC} -c -o ${CONFIG}/obj/ejsBlock.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsBlock.c

${CC} -c -o ${CONFIG}/obj/ejsBoolean.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsBoolean.c

${CC} -c -o ${CONFIG}/obj/ejsByteArray.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsByteArray.c

${CC} -c -o ${CONFIG}/obj/ejsCache.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsCache.c

${CC} -c -o ${CONFIG}/obj/ejsCmd.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsCmd.c

${CC} -c -o ${CONFIG}/obj/ejsConfig.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsConfig.c

${CC} -c -o ${CONFIG}/obj/ejsDate.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsDate.c

${CC} -c -o ${CONFIG}/obj/ejsDebug.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsDebug.c

${CC} -c -o ${CONFIG}/obj/ejsError.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsError.c

${CC} -c -o ${CONFIG}/obj/ejsFile.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsFile.c

${CC} -c -o ${CONFIG}/obj/ejsFileSystem.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsFileSystem.c

${CC} -c -o ${CONFIG}/obj/ejsFrame.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsFrame.c

${CC} -c -o ${CONFIG}/obj/ejsFunction.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsFunction.c

${CC} -c -o ${CONFIG}/obj/ejsGC.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsGC.c

${CC} -c -o ${CONFIG}/obj/ejsGlobal.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsGlobal.c

${CC} -c -o ${CONFIG}/obj/ejsHttp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsHttp.c

${CC} -c -o ${CONFIG}/obj/ejsIterator.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsIterator.c

${CC} -c -o ${CONFIG}/obj/ejsJSON.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsJSON.c

${CC} -c -o ${CONFIG}/obj/ejsLocalCache.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsLocalCache.c

${CC} -c -o ${CONFIG}/obj/ejsMath.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsMath.c

${CC} -c -o ${CONFIG}/obj/ejsMemory.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsMemory.c

${CC} -c -o ${CONFIG}/obj/ejsMprLog.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsMprLog.c

${CC} -c -o ${CONFIG}/obj/ejsNamespace.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsNamespace.c

${CC} -c -o ${CONFIG}/obj/ejsNull.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsNull.c

${CC} -c -o ${CONFIG}/obj/ejsNumber.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsNumber.c

${CC} -c -o ${CONFIG}/obj/ejsObject.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsObject.c

${CC} -c -o ${CONFIG}/obj/ejsPath.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsPath.c

${CC} -c -o ${CONFIG}/obj/ejsPot.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsPot.c

${CC} -c -o ${CONFIG}/obj/ejsRegExp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsRegExp.c

${CC} -c -o ${CONFIG}/obj/ejsSocket.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsSocket.c

${CC} -c -o ${CONFIG}/obj/ejsString.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsString.c

${CC} -c -o ${CONFIG}/obj/ejsSystem.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsSystem.c

${CC} -c -o ${CONFIG}/obj/ejsTimer.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsTimer.c

${CC} -c -o ${CONFIG}/obj/ejsType.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsType.c

${CC} -c -o ${CONFIG}/obj/ejsUri.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsUri.c

${CC} -c -o ${CONFIG}/obj/ejsVoid.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsVoid.c

${CC} -c -o ${CONFIG}/obj/ejsWorker.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsWorker.c

${CC} -c -o ${CONFIG}/obj/ejsXML.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsXML.c

${CC} -c -o ${CONFIG}/obj/ejsXMLList.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsXMLList.c

${CC} -c -o ${CONFIG}/obj/ejsXMLLoader.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/core/src/ejsXMLLoader.c

${CC} -c -o ${CONFIG}/obj/ejsByteCode.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/vm/ejsByteCode.c

${CC} -c -o ${CONFIG}/obj/ejsException.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/vm/ejsException.c

${CC} -c -o ${CONFIG}/obj/ejsHelper.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/vm/ejsHelper.c

${CC} -c -o ${CONFIG}/obj/ejsInterp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/vm/ejsInterp.c

${CC} -c -o ${CONFIG}/obj/ejsLoader.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/vm/ejsLoader.c

${CC} -c -o ${CONFIG}/obj/ejsModule.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/vm/ejsModule.c

${CC} -c -o ${CONFIG}/obj/ejsScope.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/vm/ejsScope.c

${CC} -c -o ${CONFIG}/obj/ejsService.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/vm/ejsService.c

${CC} -dynamiclib -o ${CONFIG}/lib/libejs.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/libejs.dylib ${CONFIG}/obj/ecAst.o ${CONFIG}/obj/ecCodeGen.o ${CONFIG}/obj/ecCompiler.o ${CONFIG}/obj/ecLex.o ${CONFIG}/obj/ecModuleWrite.o ${CONFIG}/obj/ecParser.o ${CONFIG}/obj/ecState.o ${CONFIG}/obj/ejsApp.o ${CONFIG}/obj/ejsArray.o ${CONFIG}/obj/ejsBlock.o ${CONFIG}/obj/ejsBoolean.o ${CONFIG}/obj/ejsByteArray.o ${CONFIG}/obj/ejsCache.o ${CONFIG}/obj/ejsCmd.o ${CONFIG}/obj/ejsConfig.o ${CONFIG}/obj/ejsDate.o ${CONFIG}/obj/ejsDebug.o ${CONFIG}/obj/ejsError.o ${CONFIG}/obj/ejsFile.o ${CONFIG}/obj/ejsFileSystem.o ${CONFIG}/obj/ejsFrame.o ${CONFIG}/obj/ejsFunction.o ${CONFIG}/obj/ejsGC.o ${CONFIG}/obj/ejsGlobal.o ${CONFIG}/obj/ejsHttp.o ${CONFIG}/obj/ejsIterator.o ${CONFIG}/obj/ejsJSON.o ${CONFIG}/obj/ejsLocalCache.o ${CONFIG}/obj/ejsMath.o ${CONFIG}/obj/ejsMemory.o ${CONFIG}/obj/ejsMprLog.o ${CONFIG}/obj/ejsNamespace.o ${CONFIG}/obj/ejsNull.o ${CONFIG}/obj/ejsNumber.o ${CONFIG}/obj/ejsObject.o ${CONFIG}/obj/ejsPath.o ${CONFIG}/obj/ejsPot.o ${CONFIG}/obj/ejsRegExp.o ${CONFIG}/obj/ejsSocket.o ${CONFIG}/obj/ejsString.o ${CONFIG}/obj/ejsSystem.o ${CONFIG}/obj/ejsTimer.o ${CONFIG}/obj/ejsType.o ${CONFIG}/obj/ejsUri.o ${CONFIG}/obj/ejsVoid.o ${CONFIG}/obj/ejsWorker.o ${CONFIG}/obj/ejsXML.o ${CONFIG}/obj/ejsXMLList.o ${CONFIG}/obj/ejsXMLLoader.o ${CONFIG}/obj/ejsByteCode.o ${CONFIG}/obj/ejsException.o ${CONFIG}/obj/ejsHelper.o ${CONFIG}/obj/ejsInterp.o ${CONFIG}/obj/ejsLoader.o ${CONFIG}/obj/ejsModule.o ${CONFIG}/obj/ejsScope.o ${CONFIG}/obj/ejsService.o ${LIBS} -lmpr -lpcre -lhttp

${CC} -c -o ${CONFIG}/obj/ejs.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/cmd/ejs.c

${CC} -o ${CONFIG}/bin/ejs -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejs.o ${LIBS} -lejs -lmpr -lpcre -lhttp -ledit

${CC} -c -o ${CONFIG}/obj/ejsc.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/cmd/ejsc.c

${CC} -o ${CONFIG}/bin/ejsc -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsc.o ${LIBS} -lejs -lmpr -lpcre -lhttp

${CC} -c -o ${CONFIG}/obj/ejsmod.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc -Isrc/cmd src/cmd/ejsmod.c

${CC} -c -o ${CONFIG}/obj/doc.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc -Isrc/cmd src/cmd/doc.c

${CC} -c -o ${CONFIG}/obj/docFiles.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc -Isrc/cmd src/cmd/docFiles.c

${CC} -c -o ${CONFIG}/obj/listing.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc -Isrc/cmd src/cmd/listing.c

${CC} -c -o ${CONFIG}/obj/slotGen.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc -Isrc/cmd src/cmd/slotGen.c

${CC} -o ${CONFIG}/bin/ejsmod -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsmod.o ${CONFIG}/obj/doc.o ${CONFIG}/obj/docFiles.o ${CONFIG}/obj/listing.o ${CONFIG}/obj/slotGen.o ${LIBS} -lejs -lmpr -lpcre -lhttp

${CC} -c -o ${CONFIG}/obj/ejsrun.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/cmd/ejsrun.c

${CC} -o ${CONFIG}/bin/ejsrun -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsrun.o ${LIBS} -lejs -lmpr -lpcre -lhttp

${CONFIG}/bin/ejsc --out ${CONFIG}/lib/ejs.mod --debug --optimize 9 --bind --require null src/core/*.es 
${CONFIG}/bin/ejsmod --require null --cslots ${CONFIG}/lib/ejs.mod
if ! diff ejs.slots.h ${CONFIG}/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ${CONFIG}/inc; fi
rm -f ejs.slots.h
cp src/jems/ejs.bit/bit.es ${CONFIG}/bin
${CC} -c -o ${CONFIG}/obj/ejsZlib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/jems/ejs.zlib/src/ejsZlib.c

${CC} -o ${CONFIG}/bin/bit -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsrun.o ${CONFIG}/obj/ejsZlib.o ${CONFIG}/obj/mprLib.o ${CONFIG}/obj/pcre.o ${CONFIG}/obj/httpLib.o ${CONFIG}/obj/ecAst.o ${CONFIG}/obj/ecCodeGen.o ${CONFIG}/obj/ecCompiler.o ${CONFIG}/obj/ecLex.o ${CONFIG}/obj/ecModuleWrite.o ${CONFIG}/obj/ecParser.o ${CONFIG}/obj/ecState.o ${CONFIG}/obj/ejsApp.o ${CONFIG}/obj/ejsArray.o ${CONFIG}/obj/ejsBlock.o ${CONFIG}/obj/ejsBoolean.o ${CONFIG}/obj/ejsByteArray.o ${CONFIG}/obj/ejsCache.o ${CONFIG}/obj/ejsCmd.o ${CONFIG}/obj/ejsConfig.o ${CONFIG}/obj/ejsDate.o ${CONFIG}/obj/ejsDebug.o ${CONFIG}/obj/ejsError.o ${CONFIG}/obj/ejsFile.o ${CONFIG}/obj/ejsFileSystem.o ${CONFIG}/obj/ejsFrame.o ${CONFIG}/obj/ejsFunction.o ${CONFIG}/obj/ejsGC.o ${CONFIG}/obj/ejsGlobal.o ${CONFIG}/obj/ejsHttp.o ${CONFIG}/obj/ejsIterator.o ${CONFIG}/obj/ejsJSON.o ${CONFIG}/obj/ejsLocalCache.o ${CONFIG}/obj/ejsMath.o ${CONFIG}/obj/ejsMemory.o ${CONFIG}/obj/ejsMprLog.o ${CONFIG}/obj/ejsNamespace.o ${CONFIG}/obj/ejsNull.o ${CONFIG}/obj/ejsNumber.o ${CONFIG}/obj/ejsObject.o ${CONFIG}/obj/ejsPath.o ${CONFIG}/obj/ejsPot.o ${CONFIG}/obj/ejsRegExp.o ${CONFIG}/obj/ejsSocket.o ${CONFIG}/obj/ejsString.o ${CONFIG}/obj/ejsSystem.o ${CONFIG}/obj/ejsTimer.o ${CONFIG}/obj/ejsType.o ${CONFIG}/obj/ejsUri.o ${CONFIG}/obj/ejsVoid.o ${CONFIG}/obj/ejsWorker.o ${CONFIG}/obj/ejsXML.o ${CONFIG}/obj/ejsXMLList.o ${CONFIG}/obj/ejsXMLLoader.o ${CONFIG}/obj/ejsByteCode.o ${CONFIG}/obj/ejsException.o ${CONFIG}/obj/ejsHelper.o ${CONFIG}/obj/ejsInterp.o ${CONFIG}/obj/ejsLoader.o ${CONFIG}/obj/ejsModule.o ${CONFIG}/obj/ejsScope.o ${CONFIG}/obj/ejsService.o ${LIBS}

cp src/jems/ejs.utest/utest.es ${CONFIG}/bin
rm -rf macosx-x86_64-debug/bin/utest
cp -r macosx-x86_64-debug/bin/ejsrun macosx-x86_64-debug/bin/utest

rm -fr ${CONFIG}/lib/bits
cp -r src/jems/ejs.bit/bits ${CONFIG}/lib
${CONFIG}/bin/ejsc --out ${CONFIG}/lib/ejs.unix.mod --debug --optimize 9 src/jems/ejs.unix/Unix.es
cp src/jems/ejs.jem/jem.es ${CONFIG}/bin
rm -rf macosx-x86_64-debug/bin/jem
cp -r macosx-x86_64-debug/bin/ejsrun macosx-x86_64-debug/bin/jem

${CONFIG}/bin/ejsc --out ${CONFIG}/lib/ejs.db.mod --debug --optimize 9 src/jems/ejs.db/*.es
${CONFIG}/bin/ejsc --out ${CONFIG}/lib/ejs.db.mapper.mod --debug --optimize 9 src/jems/ejs.db.mapper/*.es
${CONFIG}/bin/ejsc --out ${CONFIG}/lib/ejs.db.sqlite.mod --debug --optimize 9 src/jems/ejs.db.sqlite/*.es
${CC} -c -o ${CONFIG}/obj/ejsSqlite.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/jems/ejs.db.sqlite/src/ejsSqlite.c

${CC} -dynamiclib -o ${CONFIG}/lib/ejs.db.sqlite.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/ejs.db.sqlite.dylib ${CONFIG}/obj/ejsSqlite.o ${LIBS} -lmpr -lejs -lpcre -lhttp -lsqlite3

${CONFIG}/bin/ejsc --out ${CONFIG}/lib/ejs.web.mod --debug --optimize 9 src/jems/ejs.web/*.es
${CONFIG}/bin/ejsmod --cslots ${CONFIG}/lib/ejs.web.mod
if ! diff ejs.web.slots.h ${CONFIG}/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ${CONFIG}/inc; fi
rm -f ejs.web.slots.h
${CC} -c -o ${CONFIG}/obj/ejsHttpServer.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsHttpServer.c

${CC} -c -o ${CONFIG}/obj/ejsRequest.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsRequest.c

${CC} -c -o ${CONFIG}/obj/ejsSession.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsSession.c

${CC} -c -o ${CONFIG}/obj/ejsWeb.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsWeb.c

${CC} -dynamiclib -o ${CONFIG}/lib/ejs.web.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/ejs.web.dylib ${CONFIG}/obj/ejsHttpServer.o ${CONFIG}/obj/ejsRequest.o ${CONFIG}/obj/ejsSession.o ${CONFIG}/obj/ejsWeb.o ${LIBS} -lmpr -lhttp -lpcre -lpcre -lejs

rm -fr ${CONFIG}/lib/www
cp -r src/jems/ejs.web/www ${CONFIG}/lib
${CONFIG}/bin/ejsc --out ${CONFIG}/lib/ejs.template.mod --debug --optimize 9 src/jems/ejs.template/TemplateParser.es
${CONFIG}/bin/ejsc --out ${CONFIG}/lib/ejs.tar.mod --debug --optimize 9 src/jems/ejs.tar/*.es
cp src/jems/ejs.mvc/mvc.es ${CONFIG}/bin
rm -rf macosx-x86_64-debug/bin/mvc
cp -r macosx-x86_64-debug/bin/ejsrun macosx-x86_64-debug/bin/mvc

${CONFIG}/bin/ejsc --out ${CONFIG}/lib/ejs.mvc.mod --debug --optimize 9 src/jems/ejs.mvc/*.es
cp src/jems/ejs.utest/utest.worker ${CONFIG}/bin
${CC} -c -o ${CONFIG}/obj/shape.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/samples/c/nclass/shape.c

${CC} -dynamiclib -o src/samples/c/composite/composite.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/composite.dylib ${CONFIG}/obj/shape.o ${LIBS} -lejs -lmpr -lpcre -lhttp

#  Omit build script /Users/mob/git/ejs/src/samples/c/composite/composite.mod
${CC} -c -o ${CONFIG}/obj/main.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/samples/c/evalScript/main.c

${CC} -o src/samples/c/evalFile/main -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/main.o ${LIBS} -lejs -lmpr -lpcre -lhttp

#  Omit build script /Users/mob/git/ejs/src/samples/c/evalModule/evalModule.mod
${CC} -o src/samples/c/evalModule/main -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/main.o ${LIBS} -lejs -lmpr -lpcre -lhttp

${CC} -o src/samples/c/evalScript/main -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/main.o ${LIBS} -lejs -lmpr -lpcre -lhttp

#  Omit build script /Users/mob/git/ejs/src/samples/c/nclass/nclass.mod
${CC} -dynamiclib -o src/samples/c/nclass/native.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/native.dylib ${CONFIG}/obj/shape.o ${LIBS} -lejs -lmpr -lpcre -lhttp

