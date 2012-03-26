#
#   build.sh -- Build It Shell Script to build Embedthis Ejscript
#

PLATFORM="linux-i686-debug"
CC="cc"
CFLAGS="-DLINUX=1 -DLINUX -Wall -fPIC -g -Wno-unused-result -mtune=i686"
DFLAGS="-D_REENTRANT -DCPU=i686 -DPIC"
IFLAGS="-Ilinux-i686-debug/inc"
LDFLAGS="-Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl"
LIBS="-lpthread -lm"

[ ! -x ${PLATFORM}/inc ] && mkdir -p ${PLATFORM}/inc ${PLATFORM}/obj ${PLATFORM}/lib ${PLATFORM}/bin
[ ! -f ${PLATFORM}/inc/buildConfig.h ] && cp src/buildConfig.default ${PLATFORM}/inc/buildConfig.h

rm -rf linux-i686-debug/inc/mpr.h
cp -r src/deps/mpr/mpr.h linux-i686-debug/inc/mpr.h

rm -rf linux-i686-debug/inc/mprSsl.h
cp -r src/deps/mpr/mprSsl.h linux-i686-debug/inc/mprSsl.h

${CC} -c -o ${PLATFORM}/obj/mprLib.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/mpr/mprLib.c

${CC} -shared -o ${PLATFORM}/lib/libmpr.so -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl ${PLATFORM}/obj/mprLib.o ${LIBS}

${CC} -c -o ${PLATFORM}/obj/makerom.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/mpr/makerom.c

${CC} -o ${PLATFORM}/bin/makerom -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/makerom.o ${LIBS} -lmpr -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl

rm -rf linux-i686-debug/inc/pcre.h
cp -r src/deps/pcre/pcre.h linux-i686-debug/inc/pcre.h

${CC} -c -o ${PLATFORM}/obj/pcre.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/pcre/pcre.c

${CC} -shared -o ${PLATFORM}/lib/libpcre.so -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl ${PLATFORM}/obj/pcre.o ${LIBS}

rm -rf linux-i686-debug/inc/http.h
cp -r src/deps/http/http.h linux-i686-debug/inc/http.h

${CC} -c -o ${PLATFORM}/obj/httpLib.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/http/httpLib.c

${CC} -shared -o ${PLATFORM}/lib/libhttp.so -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl ${PLATFORM}/obj/httpLib.o ${LIBS} -lmpr -lpcre

${CC} -c -o ${PLATFORM}/obj/http.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/http/http.c

${CC} -o ${PLATFORM}/bin/http -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/http.o ${LIBS} -lhttp -lmpr -lpcre -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl

rm -rf linux-i686-debug/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h linux-i686-debug/inc/sqlite3.h

${CC} -c -o ${PLATFORM}/obj/sqlite3.o -fPIC -g -Wno-unused-result -mtune=i686 ${DFLAGS} -I${PLATFORM}/inc src/deps/sqlite/sqlite3.c

${CC} -shared -o ${PLATFORM}/lib/libsqlite3.so -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl ${PLATFORM}/obj/sqlite3.o ${LIBS}

rm -rf linux-i686-debug/inc/ejs.cache.local.slots.h
cp -r src/slots/ejs.cache.local.slots.h linux-i686-debug/inc/ejs.cache.local.slots.h

rm -rf linux-i686-debug/inc/ejs.db.sqlite.slots.h
cp -r src/slots/ejs.db.sqlite.slots.h linux-i686-debug/inc/ejs.db.sqlite.slots.h

rm -rf linux-i686-debug/inc/ejs.slots.h
cp -r src/slots/ejs.slots.h linux-i686-debug/inc/ejs.slots.h

rm -rf linux-i686-debug/inc/ejs.web.slots.h
cp -r src/slots/ejs.web.slots.h linux-i686-debug/inc/ejs.web.slots.h

rm -rf linux-i686-debug/inc/ejs.zlib.slots.h
cp -r src/slots/ejs.zlib.slots.h linux-i686-debug/inc/ejs.zlib.slots.h

rm -rf linux-i686-debug/inc/ejs.h
cp -r src/ejs.h linux-i686-debug/inc/ejs.h

rm -rf linux-i686-debug/inc/ejsByteCode.h
cp -r src/ejsByteCode.h linux-i686-debug/inc/ejsByteCode.h

rm -rf linux-i686-debug/inc/ejsByteCodeTable.h
cp -r src/ejsByteCodeTable.h linux-i686-debug/inc/ejsByteCodeTable.h

rm -rf linux-i686-debug/inc/ejsCompiler.h
cp -r src/ejsCompiler.h linux-i686-debug/inc/ejsCompiler.h

rm -rf linux-i686-debug/inc/ejsCustomize.h
cp -r src/ejsCustomize.h linux-i686-debug/inc/ejsCustomize.h

${CC} -c -o ${PLATFORM}/obj/ecAst.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecAst.c

${CC} -c -o ${PLATFORM}/obj/ecCodeGen.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecCodeGen.c

${CC} -c -o ${PLATFORM}/obj/ecCompiler.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecCompiler.c

${CC} -c -o ${PLATFORM}/obj/ecLex.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecLex.c

${CC} -c -o ${PLATFORM}/obj/ecModuleWrite.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecModuleWrite.c

${CC} -c -o ${PLATFORM}/obj/ecParser.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecParser.c

${CC} -c -o ${PLATFORM}/obj/ecState.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/compiler/ecState.c

${CC} -c -o ${PLATFORM}/obj/ejsApp.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsApp.c

${CC} -c -o ${PLATFORM}/obj/ejsArray.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsArray.c

${CC} -c -o ${PLATFORM}/obj/ejsBlock.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsBlock.c

${CC} -c -o ${PLATFORM}/obj/ejsBoolean.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsBoolean.c

${CC} -c -o ${PLATFORM}/obj/ejsByteArray.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsByteArray.c

${CC} -c -o ${PLATFORM}/obj/ejsCache.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsCache.c

${CC} -c -o ${PLATFORM}/obj/ejsCmd.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsCmd.c

${CC} -c -o ${PLATFORM}/obj/ejsConfig.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsConfig.c

${CC} -c -o ${PLATFORM}/obj/ejsDate.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsDate.c

${CC} -c -o ${PLATFORM}/obj/ejsDebug.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsDebug.c

${CC} -c -o ${PLATFORM}/obj/ejsError.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsError.c

${CC} -c -o ${PLATFORM}/obj/ejsFile.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsFile.c

${CC} -c -o ${PLATFORM}/obj/ejsFileSystem.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsFileSystem.c

${CC} -c -o ${PLATFORM}/obj/ejsFrame.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsFrame.c

${CC} -c -o ${PLATFORM}/obj/ejsFunction.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsFunction.c

${CC} -c -o ${PLATFORM}/obj/ejsGC.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsGC.c

${CC} -c -o ${PLATFORM}/obj/ejsGlobal.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsGlobal.c

${CC} -c -o ${PLATFORM}/obj/ejsHttp.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsHttp.c

${CC} -c -o ${PLATFORM}/obj/ejsIterator.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsIterator.c

${CC} -c -o ${PLATFORM}/obj/ejsJSON.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsJSON.c

${CC} -c -o ${PLATFORM}/obj/ejsLocalCache.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsLocalCache.c

${CC} -c -o ${PLATFORM}/obj/ejsMath.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsMath.c

${CC} -c -o ${PLATFORM}/obj/ejsMemory.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsMemory.c

${CC} -c -o ${PLATFORM}/obj/ejsMprLog.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsMprLog.c

${CC} -c -o ${PLATFORM}/obj/ejsNamespace.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsNamespace.c

${CC} -c -o ${PLATFORM}/obj/ejsNull.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsNull.c

${CC} -c -o ${PLATFORM}/obj/ejsNumber.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsNumber.c

${CC} -c -o ${PLATFORM}/obj/ejsObject.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsObject.c

${CC} -c -o ${PLATFORM}/obj/ejsPath.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsPath.c

${CC} -c -o ${PLATFORM}/obj/ejsPot.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsPot.c

${CC} -c -o ${PLATFORM}/obj/ejsRegExp.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsRegExp.c

${CC} -c -o ${PLATFORM}/obj/ejsSocket.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsSocket.c

${CC} -c -o ${PLATFORM}/obj/ejsString.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsString.c

${CC} -c -o ${PLATFORM}/obj/ejsSystem.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsSystem.c

${CC} -c -o ${PLATFORM}/obj/ejsTimer.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsTimer.c

${CC} -c -o ${PLATFORM}/obj/ejsType.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsType.c

${CC} -c -o ${PLATFORM}/obj/ejsUri.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsUri.c

${CC} -c -o ${PLATFORM}/obj/ejsVoid.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsVoid.c

${CC} -c -o ${PLATFORM}/obj/ejsWorker.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsWorker.c

${CC} -c -o ${PLATFORM}/obj/ejsXML.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsXML.c

${CC} -c -o ${PLATFORM}/obj/ejsXMLList.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsXMLList.c

${CC} -c -o ${PLATFORM}/obj/ejsXMLLoader.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/core/src/ejsXMLLoader.c

${CC} -c -o ${PLATFORM}/obj/ejsByteCode.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsByteCode.c

${CC} -c -o ${PLATFORM}/obj/ejsException.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsException.c

${CC} -c -o ${PLATFORM}/obj/ejsHelper.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsHelper.c

${CC} -c -o ${PLATFORM}/obj/ejsInterp.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsInterp.c

${CC} -c -o ${PLATFORM}/obj/ejsLoader.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsLoader.c

${CC} -c -o ${PLATFORM}/obj/ejsModule.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsModule.c

${CC} -c -o ${PLATFORM}/obj/ejsScope.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsScope.c

${CC} -c -o ${PLATFORM}/obj/ejsService.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/vm/ejsService.c

${CC} -shared -o ${PLATFORM}/lib/libejs.so -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl ${PLATFORM}/obj/ecAst.o ${PLATFORM}/obj/ecCodeGen.o ${PLATFORM}/obj/ecCompiler.o ${PLATFORM}/obj/ecLex.o ${PLATFORM}/obj/ecModuleWrite.o ${PLATFORM}/obj/ecParser.o ${PLATFORM}/obj/ecState.o ${PLATFORM}/obj/ejsApp.o ${PLATFORM}/obj/ejsArray.o ${PLATFORM}/obj/ejsBlock.o ${PLATFORM}/obj/ejsBoolean.o ${PLATFORM}/obj/ejsByteArray.o ${PLATFORM}/obj/ejsCache.o ${PLATFORM}/obj/ejsCmd.o ${PLATFORM}/obj/ejsConfig.o ${PLATFORM}/obj/ejsDate.o ${PLATFORM}/obj/ejsDebug.o ${PLATFORM}/obj/ejsError.o ${PLATFORM}/obj/ejsFile.o ${PLATFORM}/obj/ejsFileSystem.o ${PLATFORM}/obj/ejsFrame.o ${PLATFORM}/obj/ejsFunction.o ${PLATFORM}/obj/ejsGC.o ${PLATFORM}/obj/ejsGlobal.o ${PLATFORM}/obj/ejsHttp.o ${PLATFORM}/obj/ejsIterator.o ${PLATFORM}/obj/ejsJSON.o ${PLATFORM}/obj/ejsLocalCache.o ${PLATFORM}/obj/ejsMath.o ${PLATFORM}/obj/ejsMemory.o ${PLATFORM}/obj/ejsMprLog.o ${PLATFORM}/obj/ejsNamespace.o ${PLATFORM}/obj/ejsNull.o ${PLATFORM}/obj/ejsNumber.o ${PLATFORM}/obj/ejsObject.o ${PLATFORM}/obj/ejsPath.o ${PLATFORM}/obj/ejsPot.o ${PLATFORM}/obj/ejsRegExp.o ${PLATFORM}/obj/ejsSocket.o ${PLATFORM}/obj/ejsString.o ${PLATFORM}/obj/ejsSystem.o ${PLATFORM}/obj/ejsTimer.o ${PLATFORM}/obj/ejsType.o ${PLATFORM}/obj/ejsUri.o ${PLATFORM}/obj/ejsVoid.o ${PLATFORM}/obj/ejsWorker.o ${PLATFORM}/obj/ejsXML.o ${PLATFORM}/obj/ejsXMLList.o ${PLATFORM}/obj/ejsXMLLoader.o ${PLATFORM}/obj/ejsByteCode.o ${PLATFORM}/obj/ejsException.o ${PLATFORM}/obj/ejsHelper.o ${PLATFORM}/obj/ejsInterp.o ${PLATFORM}/obj/ejsLoader.o ${PLATFORM}/obj/ejsModule.o ${PLATFORM}/obj/ejsScope.o ${PLATFORM}/obj/ejsService.o ${LIBS} -lmpr -lpcre -lhttp

${CC} -c -o ${PLATFORM}/obj/ejs.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/cmd/ejs.c

${CC} -o ${PLATFORM}/bin/ejs -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/ejs.o ${LIBS} -lejs -lmpr -lpcre -lhttp -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl

${CC} -c -o ${PLATFORM}/obj/ejsc.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/cmd/ejsc.c

${CC} -o ${PLATFORM}/bin/ejsc -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/ejsc.o ${LIBS} -lejs -lmpr -lpcre -lhttp -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl

${CC} -c -o ${PLATFORM}/obj/ejsmod.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/cmd src/cmd/ejsmod.c

${CC} -c -o ${PLATFORM}/obj/doc.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/cmd src/cmd/doc.c

${CC} -c -o ${PLATFORM}/obj/docFiles.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/cmd src/cmd/docFiles.c

${CC} -c -o ${PLATFORM}/obj/listing.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/cmd src/cmd/listing.c

${CC} -c -o ${PLATFORM}/obj/slotGen.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/cmd src/cmd/slotGen.c

${CC} -o ${PLATFORM}/bin/ejsmod -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/ejsmod.o ${PLATFORM}/obj/doc.o ${PLATFORM}/obj/docFiles.o ${PLATFORM}/obj/listing.o ${PLATFORM}/obj/slotGen.o ${LIBS} -lejs -lmpr -lpcre -lhttp -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl

${CC} -c -o ${PLATFORM}/obj/ejsrun.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/cmd/ejsrun.c

${CC} -o ${PLATFORM}/bin/ejsrun -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl -L${PLATFORM}/lib ${PLATFORM}/obj/ejsrun.o ${LIBS} -lejs -lmpr -lpcre -lhttp -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl

ejsc --out ${PLATFORM}/lib/ejs.mod --debug --optimize 9 --bind --require null src/core/*.es 
ejsmod --require null --listing --cslots ${PLATFORM}/lib/ejs.mod
cp ejs.slots.h ${PLATFORM}/inc
cp src/jems/ejs.bit/bit.es ${PLATFORM}/bin
cp src/jems/ejs.utest/utest.es ${PLATFORM}/bin
rm -fr ${PLATFORM}/lib/bits
cp -r src/jems/ejs.bit/bits ${PLATFORM}/lib
ejsc --out ${PLATFORM}/lib/ejs.unix.mod --debug --optimize 9 src/jems/ejs.unix/Unix.es
cp src/jems/ejs.jem/jem.es ${PLATFORM}/bin
ejsc --out ${PLATFORM}/lib/ejs.db.mod --debug --optimize 9 src/jems/ejs.db/*.es
ejsc --out ${PLATFORM}/lib/ejs.db.mapper.mod --debug --optimize 9 src/jems/ejs.db.mapper/*.es
ejsc --out ${PLATFORM}/lib/ejs.db.sqlite.mod --debug --optimize 9 src/jems/ejs.db.sqlite/*.es
${CC} -c -o ${PLATFORM}/obj/ejsSqlite.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/jems/ejs.db.sqlite/src/ejsSqlite.c

${CC} -shared -o ${PLATFORM}/lib/ejs.db.sqlite.so -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl ${PLATFORM}/obj/ejsSqlite.o ${LIBS} -lmpr -lejs -lpcre -lhttp -lsqlite3

ejsc --out ${PLATFORM}/lib/ejs.web.mod --debug --optimize 9 src/jems/ejs.web/*.es
ejsmod --cslots ${PLATFORM}/lib/ejs.web.mod
cp ejs.web.slots.h ${PLATFORM}/inc
${CC} -c -o ${PLATFORM}/obj/ejsHttpServer.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsHttpServer.c

${CC} -c -o ${PLATFORM}/obj/ejsRequest.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsRequest.c

${CC} -c -o ${PLATFORM}/obj/ejsSession.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsSession.c

${CC} -c -o ${PLATFORM}/obj/ejsWeb.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsWeb.c

${CC} -shared -o ${PLATFORM}/lib/ejs.web.so -Wl,--enable-new-dtags -Wl,-rpath,$ORIGIN/ -Wl,-rpath,$ORIGIN/../lib -L${PLATFORM}/lib -g -ldl ${PLATFORM}/obj/ejsHttpServer.o ${PLATFORM}/obj/ejsRequest.o ${PLATFORM}/obj/ejsSession.o ${PLATFORM}/obj/ejsWeb.o ${LIBS} -lmpr -lhttp -lpcre -lpcre -lejs

rm -fr ${PLATFORM}/lib/www
cp -r src/jems/ejs.web/www ${PLATFORM}/lib
ejsc --out ${PLATFORM}/lib/ejs.template.mod --debug --optimize 9 src/jems/ejs.template/TemplateParser.es
ejsc --out ${PLATFORM}/lib/ejs.tar.mod/ --debug --optimize 9 src/jems/ejs.tar/*.es
cp src/jems/ejs.mvc/mvc.es ${PLATFORM}/bin
ejsc --out ${PLATFORM}/lib/ejs.mvc.mod/ --debug --optimize 9 src/jems/ejs.mvc/*.es
cp src/jems/ejs.utest/utest.worker ${PLATFORM}/bin
