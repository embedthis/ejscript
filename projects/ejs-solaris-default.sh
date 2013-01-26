#
#   ejs-solaris-default.sh -- Build It Shell Script to build Embedthis Ejscript
#

PRODUCT="ejs"
VERSION="2.3.0"
BUILD_NUMBER="1"
PROFILE="default"
ARCH="x86"
ARCH="`uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/'`"
OS="solaris"
CONFIG="${OS}-${ARCH}-${PROFILE}"
CC="/usr/bin/gcc"
LD="/usr/bin/ld"
CFLAGS="-fPIC -Os -w"
DFLAGS="-D_REENTRANT -DPIC"
IFLAGS="-I${CONFIG}/inc"
LDFLAGS=""
LIBPATHS="-L${CONFIG}/bin"
LIBS="-llxnet -lrt -lsocket -lpthread -lm -ldl"

[ ! -x ${CONFIG}/inc ] && mkdir -p ${CONFIG}/inc ${CONFIG}/obj ${CONFIG}/lib ${CONFIG}/bin

[ ! -f ${CONFIG}/inc/bit.h ] && cp projects/ejs-${OS}-${PROFILE}-bit.h ${CONFIG}/inc/bit.h
[ ! -f ${CONFIG}/inc/bitos.h ] && cp ${SRC}/src/bitos.h ${CONFIG}/inc/bitos.h
if ! diff ${CONFIG}/inc/bit.h projects/ejs-${OS}-${PROFILE}-bit.h >/dev/null ; then
	cp projects/ejs-${OS}-${PROFILE}-bit.h ${CONFIG}/inc/bit.h
fi

rm -rf ${CONFIG}/inc/bitos.h
cp -r src/bitos.h ${CONFIG}/inc/bitos.h

rm -rf ${CONFIG}/inc/mpr.h
cp -r src/deps/mpr/mpr.h ${CONFIG}/inc/mpr.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/mprLib.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprLib.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libmpr.so ${LIBPATHS} ${CONFIG}/obj/mprLib.o ${LIBS}

rm -rf ${CONFIG}/inc/est.h
cp -r src/deps/est/est.h ${CONFIG}/inc/est.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/estLib.o -fPIC -Os ${DFLAGS} -I${CONFIG}/inc src/deps/est/estLib.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libest.so ${LIBPATHS} ${CONFIG}/obj/estLib.o ${LIBS}

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/mprSsl.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprSsl.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libmprssl.so ${LIBPATHS} ${CONFIG}/obj/mprSsl.o -lest -lmpr ${LIBS}

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/manager.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/manager.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/ejsman ${LIBPATHS} ${CONFIG}/obj/manager.o -lmpr ${LIBS} -lmpr -llxnet -lrt -lsocket -lpthread -lm -ldl 

rm -rf ${CONFIG}/bin/ca.crt
cp -r src/deps/est/ca.crt ${CONFIG}/bin/ca.crt

rm -rf ${CONFIG}/inc/pcre.h
cp -r src/deps/pcre/pcre.h ${CONFIG}/inc/pcre.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/pcre.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/pcre/pcre.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libpcre.so ${LIBPATHS} ${CONFIG}/obj/pcre.o ${LIBS}

rm -rf ${CONFIG}/inc/http.h
cp -r src/deps/http/http.h ${CONFIG}/inc/http.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/httpLib.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/httpLib.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libhttp.so ${LIBPATHS} ${CONFIG}/obj/httpLib.o -lpcre -lmpr ${LIBS}

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/http.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/http.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/http ${LIBPATHS} ${CONFIG}/obj/http.o -lhttp ${LIBS} -lpcre -lmpr -lhttp -llxnet -lrt -lsocket -lpthread -lm -ldl -lpcre -lmpr 

rm -rf ${CONFIG}/bin/http-ca.crt
cp -r src/deps/http/http-ca.crt ${CONFIG}/bin/http-ca.crt

rm -rf ${CONFIG}/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h ${CONFIG}/inc/sqlite3.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/sqlite3.o -fPIC -Os ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite3.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libsqlite3.so ${LIBPATHS} ${CONFIG}/obj/sqlite3.o ${LIBS}

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/sqlite.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/sqlite ${LIBPATHS} ${CONFIG}/obj/sqlite.o -lsqlite3 ${LIBS} -lsqlite3 -llxnet -lrt -lsocket -lpthread -lm -ldl 

rm -rf ${CONFIG}/inc/zlib.h
cp -r src/deps/zlib/zlib.h ${CONFIG}/inc/zlib.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/zlib.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/zlib/zlib.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libzlib.so ${LIBPATHS} ${CONFIG}/obj/zlib.o ${LIBS}

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

rm -rf ${CONFIG}/inc/ejsByteCode.h
cp -r src/ejsByteCode.h ${CONFIG}/inc/ejsByteCode.h

rm -rf ${CONFIG}/inc/ejsByteCodeTable.h
cp -r src/ejsByteCodeTable.h ${CONFIG}/inc/ejsByteCodeTable.h

rm -rf ${CONFIG}/inc/ejsCustomize.h
cp -r src/ejsCustomize.h ${CONFIG}/inc/ejsCustomize.h

rm -rf ${CONFIG}/inc/ejs.h
cp -r src/ejs.h ${CONFIG}/inc/ejs.h

rm -rf ${CONFIG}/inc/ejsCompiler.h
cp -r src/ejsCompiler.h ${CONFIG}/inc/ejsCompiler.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ecAst.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecAst.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ecCodeGen.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecCodeGen.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ecCompiler.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecCompiler.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ecLex.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecLex.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ecModuleWrite.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecModuleWrite.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ecParser.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecParser.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ecState.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/compiler/ecState.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/dtoa.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/dtoa.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsApp.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsApp.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsArray.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsArray.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsBlock.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsBlock.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsBoolean.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsBoolean.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsByteArray.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsByteArray.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsCache.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsCache.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsCmd.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsCmd.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsConfig.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsConfig.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsDate.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsDate.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsDebug.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsDebug.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsError.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsError.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsFile.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsFile.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsFileSystem.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsFileSystem.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsFrame.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsFrame.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsFunction.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsFunction.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsGC.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsGC.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsGlobal.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsGlobal.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsHttp.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsHttp.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsIterator.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsIterator.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsJSON.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsJSON.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsLocalCache.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsLocalCache.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsMath.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsMath.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsMemory.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsMemory.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsMprLog.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsMprLog.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsNamespace.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsNamespace.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsNull.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsNull.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsNumber.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsNumber.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsObject.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsObject.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsPath.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsPath.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsPot.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsPot.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsRegExp.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsRegExp.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsSocket.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsSocket.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsString.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsString.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsSystem.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsSystem.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsTimer.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsTimer.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsType.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsType.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsUri.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsUri.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsVoid.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsVoid.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsWebSocket.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsWebSocket.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsWorker.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsWorker.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsXML.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsXML.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsXMLList.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsXMLList.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsXMLLoader.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/core/src/ejsXMLLoader.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsByteCode.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsByteCode.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsException.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsException.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsHelper.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsHelper.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsInterp.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsInterp.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsLoader.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsLoader.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsModule.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsModule.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsScope.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsScope.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsService.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/vm/ejsService.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libejs.so ${LIBPATHS} ${CONFIG}/obj/ecAst.o ${CONFIG}/obj/ecCodeGen.o ${CONFIG}/obj/ecCompiler.o ${CONFIG}/obj/ecLex.o ${CONFIG}/obj/ecModuleWrite.o ${CONFIG}/obj/ecParser.o ${CONFIG}/obj/ecState.o ${CONFIG}/obj/dtoa.o ${CONFIG}/obj/ejsApp.o ${CONFIG}/obj/ejsArray.o ${CONFIG}/obj/ejsBlock.o ${CONFIG}/obj/ejsBoolean.o ${CONFIG}/obj/ejsByteArray.o ${CONFIG}/obj/ejsCache.o ${CONFIG}/obj/ejsCmd.o ${CONFIG}/obj/ejsConfig.o ${CONFIG}/obj/ejsDate.o ${CONFIG}/obj/ejsDebug.o ${CONFIG}/obj/ejsError.o ${CONFIG}/obj/ejsFile.o ${CONFIG}/obj/ejsFileSystem.o ${CONFIG}/obj/ejsFrame.o ${CONFIG}/obj/ejsFunction.o ${CONFIG}/obj/ejsGC.o ${CONFIG}/obj/ejsGlobal.o ${CONFIG}/obj/ejsHttp.o ${CONFIG}/obj/ejsIterator.o ${CONFIG}/obj/ejsJSON.o ${CONFIG}/obj/ejsLocalCache.o ${CONFIG}/obj/ejsMath.o ${CONFIG}/obj/ejsMemory.o ${CONFIG}/obj/ejsMprLog.o ${CONFIG}/obj/ejsNamespace.o ${CONFIG}/obj/ejsNull.o ${CONFIG}/obj/ejsNumber.o ${CONFIG}/obj/ejsObject.o ${CONFIG}/obj/ejsPath.o ${CONFIG}/obj/ejsPot.o ${CONFIG}/obj/ejsRegExp.o ${CONFIG}/obj/ejsSocket.o ${CONFIG}/obj/ejsString.o ${CONFIG}/obj/ejsSystem.o ${CONFIG}/obj/ejsTimer.o ${CONFIG}/obj/ejsType.o ${CONFIG}/obj/ejsUri.o ${CONFIG}/obj/ejsVoid.o ${CONFIG}/obj/ejsWebSocket.o ${CONFIG}/obj/ejsWorker.o ${CONFIG}/obj/ejsXML.o ${CONFIG}/obj/ejsXMLList.o ${CONFIG}/obj/ejsXMLLoader.o ${CONFIG}/obj/ejsByteCode.o ${CONFIG}/obj/ejsException.o ${CONFIG}/obj/ejsHelper.o ${CONFIG}/obj/ejsInterp.o ${CONFIG}/obj/ejsLoader.o ${CONFIG}/obj/ejsModule.o ${CONFIG}/obj/ejsScope.o ${CONFIG}/obj/ejsService.o -lhttp ${LIBS} -lpcre -lmpr

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejs.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/cmd/ejs.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/ejs ${LIBPATHS} ${CONFIG}/obj/ejs.o -lejs ${LIBS} -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsc.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/cmd/ejsc.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/ejsc ${LIBPATHS} ${CONFIG}/obj/ejsc.o -lejs ${LIBS} -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsmod.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/cmd src/cmd/ejsmod.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/doc.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/cmd src/cmd/doc.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/docFiles.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/cmd src/cmd/docFiles.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/listing.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/cmd src/cmd/listing.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/slotGen.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/cmd src/cmd/slotGen.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/ejsmod ${LIBPATHS} ${CONFIG}/obj/ejsmod.o ${CONFIG}/obj/doc.o ${CONFIG}/obj/docFiles.o ${CONFIG}/obj/listing.o ${CONFIG}/obj/slotGen.o -lejs ${LIBS} -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsrun.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/cmd/ejsrun.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/ejsrun ${LIBPATHS} ${CONFIG}/obj/ejsrun.o -lejs ${LIBS} -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

cd src/core >/dev/null ;\
../../${CONFIG}/bin/ejsc --out ../../${CONFIG}/bin/ejs.mod  --optimize 9 --bind --require null *.es  ;\
../../${CONFIG}/bin/ejsmod --require null --cslots ../../${CONFIG}/bin/ejs.mod ;\
if ! diff ejs.slots.h ../../${CONFIG}/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ../../${CONFIG}/inc; fi ;\
rm -f ejs.slots.h ;\
cd - >/dev/null 

#  Omit build script /Users/mob/git/ejs/solaris-x86-default/bin/ejs.unix.mod
#  Omit build script /Users/mob/git/ejs/solaris-x86-default/bin/jem.es
${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/jem ${LIBPATHS} ${CONFIG}/obj/ejsrun.o -lejs ${LIBS} -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

#  Omit build script /Users/mob/git/ejs/solaris-x86-default/bin/ejs.db.mod
#  Omit build script /Users/mob/git/ejs/solaris-x86-default/bin/ejs.db.mapper.mod
#  Omit build script /Users/mob/git/ejs/solaris-x86-default/bin/ejs.db.sqlite.mod
${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsSqlite.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/jems/ejs.db.sqlite/ejsSqlite.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libejs.db.sqlite.so ${LIBPATHS} ${CONFIG}/obj/ejsSqlite.o -lsqlite3 -lejs -lmpr ${LIBS} -lhttp -lpcre

cd src/jems/ejs.mail >/dev/null ;\
../../../${CONFIG}/bin/ejsc --out ../../../${CONFIG}/bin/ejs.mail.mod  --optimize 9 *.es ;\
cd - >/dev/null 

cd src/jems/ejs.web >/dev/null ;\
../../../${CONFIG}/bin/ejsc --out ../../../${CONFIG}/bin/ejs.web.mod  --optimize 9 *.es ;\
../../../${CONFIG}/bin/ejsmod --cslots ../../../${CONFIG}/bin/ejs.web.mod ;\
if ! diff ejs.web.slots.h ../../../${CONFIG}/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../../${CONFIG}/inc; fi ;\
rm -f ejs.web.slots.h ;\
cd - >/dev/null 

rm -rf ${CONFIG}/inc/ejsWeb.h
cp -r src/jems/ejs.web/ejsWeb.h ${CONFIG}/inc/ejsWeb.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsHttpServer.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsHttpServer.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsRequest.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsRequest.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsSession.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsSession.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsWeb.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsWeb.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libejs.web.so ${LIBPATHS} ${CONFIG}/obj/ejsHttpServer.o ${CONFIG}/obj/ejsRequest.o ${CONFIG}/obj/ejsSession.o ${CONFIG}/obj/ejsWeb.o -lejs ${LIBS} -lhttp -lpcre -lmpr

cd src/jems/ejs.web >/dev/null ;\
rm -fr ../../../${CONFIG}/bin/www ;\
cp -r www ../../../${CONFIG}/bin ;\
cd - >/dev/null 

#  Omit build script /Users/mob/git/ejs/solaris-x86-default/bin/ejs.template.mod
#  Omit build script /Users/mob/git/ejs/solaris-x86-default/bin/ejs.zlib.mod
${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsZlib.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/jems/ejs.zlib/ejsZlib.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libejs.zlib.so ${LIBPATHS} ${CONFIG}/obj/ejsZlib.o -lzlib -lejs ${LIBS} -lhttp -lpcre -lmpr

#  Omit build script /Users/mob/git/ejs/solaris-x86-default/bin/ejs.tar.mod
#  Omit build script /Users/mob/git/ejs/solaris-x86-default/bin/mvc.es
${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/mvc ${LIBPATHS} ${CONFIG}/obj/ejsrun.o -lejs ${LIBS} -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

#  Omit build script /Users/mob/git/ejs/solaris-x86-default/bin/ejs.mvc.mod
#  Omit build script /Users/mob/git/ejs/solaris-x86-default/bin/utest.es
#  Omit build script /Users/mob/git/ejs/solaris-x86-default/bin/utest.worker
${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/utest ${LIBPATHS} ${CONFIG}/obj/ejsrun.o -lejs ${LIBS} -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

#  Omit build script undefined
