#
#   ejs-solaris.sh -- Build It Shell Script to build Embedthis Ejscript
#

ARCH="x86"
ARCH="$(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/')"
OS="solaris"
PROFILE="debug"
CONFIG="${OS}-${ARCH}-${PROFILE}"
CC="/usr/bin/gcc"
LD="/usr/bin/ld"
CFLAGS="-Wall -fPIC -g -Wshorten-64-to-32 -mtune=generic"
DFLAGS="-D_REENTRANT -DPIC -DBIT_DEBUG"
IFLAGS="-I${CONFIG}/inc"
LDFLAGS="-g"
LIBPATHS="-L${CONFIG}/bin"
LIBS="-llxnet -lrt -lsocket -lpthread -lm -ldl"

[ ! -x ${CONFIG}/inc ] && mkdir -p ${CONFIG}/inc ${CONFIG}/obj ${CONFIG}/lib ${CONFIG}/bin

[ ! -f ${CONFIG}/inc/bit.h ] && cp projects/ejs-${OS}-bit.h ${CONFIG}/inc/bit.h
if ! diff ${CONFIG}/inc/bit.h projects/ejs-${OS}-bit.h >/dev/null ; then
	cp projects/ejs-${OS}-bit.h ${CONFIG}/inc/bit.h
fi

rm -rf ${CONFIG}/inc/mpr.h
cp -r src/deps/mpr/mpr.h ${CONFIG}/inc/mpr.h

${CC} -c -o ${CONFIG}/obj/mprLib.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprLib.c

${CC} -shared -o ${CONFIG}/bin/libmpr.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/mprLib.o ${LIBS}

${CC} -c -o ${CONFIG}/obj/mprSsl.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprSsl.c

${CC} -shared -o ${CONFIG}/bin/libmprssl.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/mprSsl.o ${LIBS} -lmpr

${CC} -c -o ${CONFIG}/obj/manager.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/manager.c

${CC} -o ${CONFIG}/bin/ejsman ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/manager.o ${LIBS} -lmpr ${LDFLAGS}

rm -rf ${CONFIG}/inc/pcre.h
cp -r src/deps/pcre/pcre.h ${CONFIG}/inc/pcre.h

${CC} -c -o ${CONFIG}/obj/pcre.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/deps/pcre/pcre.c

${CC} -shared -o ${CONFIG}/bin/libpcre.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/pcre.o ${LIBS}

rm -rf ${CONFIG}/inc/http.h
cp -r src/deps/http/http.h ${CONFIG}/inc/http.h

${CC} -c -o ${CONFIG}/obj/httpLib.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/deps/http/httpLib.c

${CC} -shared -o ${CONFIG}/bin/libhttp.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/httpLib.o ${LIBS} -lpam -lmpr -lpcre -lmprssl

rm -rf ${CONFIG}/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h ${CONFIG}/inc/sqlite3.h

${CC} -c -o ${CONFIG}/obj/sqlite3.o -fPIC ${LDFLAGS} -mtune=generic -w ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite3.c

${CC} -shared -o ${CONFIG}/bin/libsqlite3.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite3.o ${LIBS}

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

${CC} -c -o ${CONFIG}/obj/ecAst.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/compiler/ecAst.c

${CC} -c -o ${CONFIG}/obj/ecCodeGen.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/compiler/ecCodeGen.c

${CC} -c -o ${CONFIG}/obj/ecCompiler.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/compiler/ecCompiler.c

${CC} -c -o ${CONFIG}/obj/ecLex.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/compiler/ecLex.c

${CC} -c -o ${CONFIG}/obj/ecModuleWrite.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/compiler/ecModuleWrite.c

${CC} -c -o ${CONFIG}/obj/ecParser.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/compiler/ecParser.c

${CC} -c -o ${CONFIG}/obj/ecState.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/compiler/ecState.c

${CC} -c -o ${CONFIG}/obj/ejsApp.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsApp.c

${CC} -c -o ${CONFIG}/obj/ejsArray.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsArray.c

${CC} -c -o ${CONFIG}/obj/ejsBlock.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsBlock.c

${CC} -c -o ${CONFIG}/obj/ejsBoolean.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsBoolean.c

${CC} -c -o ${CONFIG}/obj/ejsByteArray.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsByteArray.c

${CC} -c -o ${CONFIG}/obj/ejsCache.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsCache.c

${CC} -c -o ${CONFIG}/obj/ejsCmd.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsCmd.c

${CC} -c -o ${CONFIG}/obj/ejsConfig.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsConfig.c

${CC} -c -o ${CONFIG}/obj/ejsDate.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsDate.c

${CC} -c -o ${CONFIG}/obj/ejsDebug.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsDebug.c

${CC} -c -o ${CONFIG}/obj/ejsError.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsError.c

${CC} -c -o ${CONFIG}/obj/ejsFile.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsFile.c

${CC} -c -o ${CONFIG}/obj/ejsFileSystem.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsFileSystem.c

${CC} -c -o ${CONFIG}/obj/ejsFrame.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsFrame.c

${CC} -c -o ${CONFIG}/obj/ejsFunction.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsFunction.c

${CC} -c -o ${CONFIG}/obj/ejsGC.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsGC.c

${CC} -c -o ${CONFIG}/obj/ejsGlobal.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsGlobal.c

${CC} -c -o ${CONFIG}/obj/ejsHttp.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsHttp.c

${CC} -c -o ${CONFIG}/obj/ejsIterator.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsIterator.c

${CC} -c -o ${CONFIG}/obj/ejsJSON.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsJSON.c

${CC} -c -o ${CONFIG}/obj/ejsLocalCache.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsLocalCache.c

${CC} -c -o ${CONFIG}/obj/ejsMath.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsMath.c

${CC} -c -o ${CONFIG}/obj/ejsMemory.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsMemory.c

${CC} -c -o ${CONFIG}/obj/ejsMprLog.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsMprLog.c

${CC} -c -o ${CONFIG}/obj/ejsNamespace.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsNamespace.c

${CC} -c -o ${CONFIG}/obj/ejsNull.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsNull.c

${CC} -c -o ${CONFIG}/obj/ejsNumber.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsNumber.c

${CC} -c -o ${CONFIG}/obj/ejsObject.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsObject.c

${CC} -c -o ${CONFIG}/obj/ejsPath.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsPath.c

${CC} -c -o ${CONFIG}/obj/ejsPot.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsPot.c

${CC} -c -o ${CONFIG}/obj/ejsRegExp.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsRegExp.c

${CC} -c -o ${CONFIG}/obj/ejsSocket.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsSocket.c

${CC} -c -o ${CONFIG}/obj/ejsString.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsString.c

${CC} -c -o ${CONFIG}/obj/ejsSystem.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsSystem.c

${CC} -c -o ${CONFIG}/obj/ejsTimer.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsTimer.c

${CC} -c -o ${CONFIG}/obj/ejsType.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsType.c

${CC} -c -o ${CONFIG}/obj/ejsUri.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsUri.c

${CC} -c -o ${CONFIG}/obj/ejsVoid.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsVoid.c

${CC} -c -o ${CONFIG}/obj/ejsWorker.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsWorker.c

${CC} -c -o ${CONFIG}/obj/ejsXML.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsXML.c

${CC} -c -o ${CONFIG}/obj/ejsXMLList.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsXMLList.c

${CC} -c -o ${CONFIG}/obj/ejsXMLLoader.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsXMLLoader.c

${CC} -c -o ${CONFIG}/obj/ejsByteCode.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/vm/ejsByteCode.c

${CC} -c -o ${CONFIG}/obj/ejsException.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/vm/ejsException.c

${CC} -c -o ${CONFIG}/obj/ejsHelper.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/vm/ejsHelper.c

${CC} -c -o ${CONFIG}/obj/ejsInterp.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/vm/ejsInterp.c

${CC} -c -o ${CONFIG}/obj/ejsLoader.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/vm/ejsLoader.c

${CC} -c -o ${CONFIG}/obj/ejsModule.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/vm/ejsModule.c

${CC} -c -o ${CONFIG}/obj/ejsScope.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/vm/ejsScope.c

${CC} -c -o ${CONFIG}/obj/ejsService.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/vm/ejsService.c

${CC} -shared -o ${CONFIG}/bin/libejs.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ecAst.o ${CONFIG}/obj/ecCodeGen.o ${CONFIG}/obj/ecCompiler.o ${CONFIG}/obj/ecLex.o ${CONFIG}/obj/ecModuleWrite.o ${CONFIG}/obj/ecParser.o ${CONFIG}/obj/ecState.o ${CONFIG}/obj/ejsApp.o ${CONFIG}/obj/ejsArray.o ${CONFIG}/obj/ejsBlock.o ${CONFIG}/obj/ejsBoolean.o ${CONFIG}/obj/ejsByteArray.o ${CONFIG}/obj/ejsCache.o ${CONFIG}/obj/ejsCmd.o ${CONFIG}/obj/ejsConfig.o ${CONFIG}/obj/ejsDate.o ${CONFIG}/obj/ejsDebug.o ${CONFIG}/obj/ejsError.o ${CONFIG}/obj/ejsFile.o ${CONFIG}/obj/ejsFileSystem.o ${CONFIG}/obj/ejsFrame.o ${CONFIG}/obj/ejsFunction.o ${CONFIG}/obj/ejsGC.o ${CONFIG}/obj/ejsGlobal.o ${CONFIG}/obj/ejsHttp.o ${CONFIG}/obj/ejsIterator.o ${CONFIG}/obj/ejsJSON.o ${CONFIG}/obj/ejsLocalCache.o ${CONFIG}/obj/ejsMath.o ${CONFIG}/obj/ejsMemory.o ${CONFIG}/obj/ejsMprLog.o ${CONFIG}/obj/ejsNamespace.o ${CONFIG}/obj/ejsNull.o ${CONFIG}/obj/ejsNumber.o ${CONFIG}/obj/ejsObject.o ${CONFIG}/obj/ejsPath.o ${CONFIG}/obj/ejsPot.o ${CONFIG}/obj/ejsRegExp.o ${CONFIG}/obj/ejsSocket.o ${CONFIG}/obj/ejsString.o ${CONFIG}/obj/ejsSystem.o ${CONFIG}/obj/ejsTimer.o ${CONFIG}/obj/ejsType.o ${CONFIG}/obj/ejsUri.o ${CONFIG}/obj/ejsVoid.o ${CONFIG}/obj/ejsWorker.o ${CONFIG}/obj/ejsXML.o ${CONFIG}/obj/ejsXMLList.o ${CONFIG}/obj/ejsXMLLoader.o ${CONFIG}/obj/ejsByteCode.o ${CONFIG}/obj/ejsException.o ${CONFIG}/obj/ejsHelper.o ${CONFIG}/obj/ejsInterp.o ${CONFIG}/obj/ejsLoader.o ${CONFIG}/obj/ejsModule.o ${CONFIG}/obj/ejsScope.o ${CONFIG}/obj/ejsService.o ${LIBS} -lhttp -lpam -lmpr -lpcre -lmprssl

${CC} -c -o ${CONFIG}/obj/ejs.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/cmd/ejs.c

${CC} -o ${CONFIG}/bin/ejs ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejs.o ${LIBS} -lejs -lhttp -lpam -lmpr -lpcre -lmprssl -ledit ${LDFLAGS}

cd src/core >/dev/null ;\
../../${CONFIG}/bin/ejsc --out ../../${CONFIG}/bin/ejs.mod --debug --optimize 9 --bind --require null *.es  ;\
../../${CONFIG}/bin/ejsmod --require null --cslots ../../${CONFIG}/bin/ejs.mod ;\
if ! diff ejs.slots.h ../../${CONFIG}/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ../../${CONFIG}/inc; fi ;\
rm -f ejs.slots.h ;\
cd - >/dev/null 

#  Omit build script /Users/mob/git/ejs/solaris-x86-debug/bin/ejs.unix.mod
#  Omit build script /Users/mob/git/ejs/solaris-x86-debug/bin/ejs.db.mod
#  Omit build script /Users/mob/git/ejs/solaris-x86-debug/bin/ejs.db.mapper.mod
#  Omit build script /Users/mob/git/ejs/solaris-x86-debug/bin/ejs.db.sqlite.mod
${CC} -c -o ${CONFIG}/obj/ejsSqlite.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc src/jems/ejs.db.sqlite/ejsSqlite.c

${CC} -shared -o ${CONFIG}/bin/ejs.db.sqlite.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsSqlite.o ${LIBS} -lmpr -lejs -lhttp -lpam -lpcre -lmprssl -lsqlite3

cd src/jems/ejs.web >/dev/null ;\
../../../${CONFIG}/bin/ejsc --out ${OUT} --debug --optimize 9 *.es ;\
../../../${CONFIG}/bin/ejsmod --cslots ${OUT} ;\
if ! diff ejs.web.slots.h ../../../${CONFIG}/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../../${CONFIG}/inc; fi ;\
rm -f ejs.web.slots.h ;\
cd - >/dev/null 

${CC} -c -o ${CONFIG}/obj/ejsHttpServer.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsHttpServer.c

${CC} -c -o ${CONFIG}/obj/ejsRequest.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsRequest.c

${CC} -c -o ${CONFIG}/obj/ejsSession.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsSession.c

${CC} -c -o ${CONFIG}/obj/ejsWeb.o -Wall -fPIC ${LDFLAGS} -Wshorten-64-to-32 -mtune=generic ${DFLAGS} -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsWeb.c

${CC} -shared -o ${CONFIG}/bin/ejs.web.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsHttpServer.o ${CONFIG}/obj/ejsRequest.o ${CONFIG}/obj/ejsSession.o ${CONFIG}/obj/ejsWeb.o ${LIBS} -lejs -lhttp -lpam -lmpr -lpcre -lmprssl

#  Omit build script /Users/mob/git/ejs/solaris-x86-debug/bin/ejs.template.mod
#  Omit build script /Users/mob/git/ejs/solaris-x86-debug/bin/ejs.tar.mod
#  Omit build script /Users/mob/git/ejs/solaris-x86-debug/bin/ejs.mvc.mod
