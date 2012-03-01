#
#   build.sh -- Build It Shell Script to build Embedthis Ejscript
#

CC="/usr/bin/cc"
CFLAGS="-fPIC -Wall -g -Wshorten-64-to-32"
DFLAGS="-DPIC -DCPU=X86_64"
IFLAGS="-Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots"
LDFLAGS="-Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L/Users/mob/git/ejs/macosx-x86_64-debug/lib -g -ldl"
LIBS="-lpthread -lm"

${CC} -c -o macosx-x86_64-debug/obj/mprLib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/mprLib.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/libmpr.dylib -arch x86_64 ${LDFLAGS} -install_name @rpath/libmpr.dylib macosx-x86_64-debug/obj/mprLib.o ${LIBS}

${CC} -c -o macosx-x86_64-debug/obj/mprSsl.o -arch x86_64 ${CFLAGS} ${DFLAGS} -DPOSIX -DMATRIX_USE_FILE_SYSTEM -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -I../packages-macosx-x86_64/openssl/openssl-1.0.0d/include -I../packages-macosx-x86_64/matrixssl/matrixssl-3-3-open/matrixssl -I../packages-macosx-x86_64/matrixssl/matrixssl-3-3-open src/deps/mpr/mprSsl.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/libmprssl.dylib -arch x86_64 ${LDFLAGS} -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -install_name @rpath/libmprssl.dylib macosx-x86_64-debug/obj/mprSsl.o ${LIBS} -lmpr -lssl -lcrypto -lmatrixssl

${CC} -c -o macosx-x86_64-debug/obj/manager.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/manager.c

${CC} -o macosx-x86_64-debug/bin/manager -arch x86_64 ${LDFLAGS} -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/manager.o ${LIBS} -lmpr

${CC} -c -o macosx-x86_64-debug/obj/makerom.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/makerom.c

${CC} -o macosx-x86_64-debug/bin/makerom -arch x86_64 ${LDFLAGS} -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/makerom.o ${LIBS} -lmpr

${CC} -c -o macosx-x86_64-debug/obj/pcre.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/pcre/pcre.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/libpcre.dylib -arch x86_64 ${LDFLAGS} -install_name @rpath/libpcre.dylib macosx-x86_64-debug/obj/pcre.o ${LIBS}

${CC} -c -o macosx-x86_64-debug/obj/httpLib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/http/httpLib.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/libhttp.dylib -arch x86_64 ${LDFLAGS} -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -install_name @rpath/libhttp.dylib macosx-x86_64-debug/obj/httpLib.o ${LIBS} -lmpr -lpcre -lmprssl -lssl -lcrypto -lmatrixssl

${CC} -c -o macosx-x86_64-debug/obj/http.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/http/http.c

${CC} -o macosx-x86_64-debug/bin/http -arch x86_64 ${LDFLAGS} -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/http.o ${LIBS} -lhttp -lmpr -lpcre -lmprssl -lssl -lcrypto -lmatrixssl

${CC} -c -o macosx-x86_64-debug/obj/sqlite3.o -arch x86_64 -fPIC -g ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/sqlite/sqlite3.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/libsqlite3.dylib -arch x86_64 ${LDFLAGS} -install_name @rpath/libsqlite3.dylib macosx-x86_64-debug/obj/sqlite3.o ${LIBS}

#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/inc/ejs.slots.h undefined
${CC} -c -o macosx-x86_64-debug/obj/ecAst.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecAst.c

${CC} -c -o macosx-x86_64-debug/obj/ecCodeGen.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecCodeGen.c

${CC} -c -o macosx-x86_64-debug/obj/ecCompiler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecCompiler.c

${CC} -c -o macosx-x86_64-debug/obj/ecLex.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecLex.c

${CC} -c -o macosx-x86_64-debug/obj/ecModuleWrite.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecModuleWrite.c

${CC} -c -o macosx-x86_64-debug/obj/ecParser.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecParser.c

${CC} -c -o macosx-x86_64-debug/obj/ecState.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecState.c

${CC} -c -o macosx-x86_64-debug/obj/ejsApp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsApp.c

${CC} -c -o macosx-x86_64-debug/obj/ejsArray.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsArray.c

${CC} -c -o macosx-x86_64-debug/obj/ejsBlock.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsBlock.c

${CC} -c -o macosx-x86_64-debug/obj/ejsBoolean.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsBoolean.c

${CC} -c -o macosx-x86_64-debug/obj/ejsByteArray.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsByteArray.c

${CC} -c -o macosx-x86_64-debug/obj/ejsCache.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsCache.c

${CC} -c -o macosx-x86_64-debug/obj/ejsCmd.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsCmd.c

${CC} -c -o macosx-x86_64-debug/obj/ejsConfig.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsConfig.c

${CC} -c -o macosx-x86_64-debug/obj/ejsDate.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsDate.c

${CC} -c -o macosx-x86_64-debug/obj/ejsDebug.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsDebug.c

${CC} -c -o macosx-x86_64-debug/obj/ejsError.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsError.c

${CC} -c -o macosx-x86_64-debug/obj/ejsFile.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFile.c

${CC} -c -o macosx-x86_64-debug/obj/ejsFileSystem.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFileSystem.c

${CC} -c -o macosx-x86_64-debug/obj/ejsFrame.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFrame.c

${CC} -c -o macosx-x86_64-debug/obj/ejsFunction.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFunction.c

${CC} -c -o macosx-x86_64-debug/obj/ejsGC.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsGC.c

${CC} -c -o macosx-x86_64-debug/obj/ejsGlobal.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsGlobal.c

${CC} -c -o macosx-x86_64-debug/obj/ejsHttp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsHttp.c

${CC} -c -o macosx-x86_64-debug/obj/ejsIterator.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsIterator.c

${CC} -c -o macosx-x86_64-debug/obj/ejsJSON.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsJSON.c

${CC} -c -o macosx-x86_64-debug/obj/ejsLocalCache.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsLocalCache.c

${CC} -c -o macosx-x86_64-debug/obj/ejsMath.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMath.c

${CC} -c -o macosx-x86_64-debug/obj/ejsMemory.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMemory.c

${CC} -c -o macosx-x86_64-debug/obj/ejsMprLog.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMprLog.c

${CC} -c -o macosx-x86_64-debug/obj/ejsNamespace.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNamespace.c

${CC} -c -o macosx-x86_64-debug/obj/ejsNull.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNull.c

${CC} -c -o macosx-x86_64-debug/obj/ejsNumber.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNumber.c

${CC} -c -o macosx-x86_64-debug/obj/ejsObject.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsObject.c

${CC} -c -o macosx-x86_64-debug/obj/ejsPath.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsPath.c

${CC} -c -o macosx-x86_64-debug/obj/ejsPot.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsPot.c

${CC} -c -o macosx-x86_64-debug/obj/ejsRegExp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsRegExp.c

${CC} -c -o macosx-x86_64-debug/obj/ejsSocket.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsSocket.c

${CC} -c -o macosx-x86_64-debug/obj/ejsString.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsString.c

${CC} -c -o macosx-x86_64-debug/obj/ejsSystem.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsSystem.c

${CC} -c -o macosx-x86_64-debug/obj/ejsTimer.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsTimer.c

${CC} -c -o macosx-x86_64-debug/obj/ejsType.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsType.c

${CC} -c -o macosx-x86_64-debug/obj/ejsUri.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsUri.c

${CC} -c -o macosx-x86_64-debug/obj/ejsVoid.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsVoid.c

${CC} -c -o macosx-x86_64-debug/obj/ejsWorker.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsWorker.c

${CC} -c -o macosx-x86_64-debug/obj/ejsXML.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXML.c

${CC} -c -o macosx-x86_64-debug/obj/ejsXMLList.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXMLList.c

${CC} -c -o macosx-x86_64-debug/obj/ejsXMLLoader.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXMLLoader.c

${CC} -c -o macosx-x86_64-debug/obj/ejsByteCode.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsByteCode.c

${CC} -c -o macosx-x86_64-debug/obj/ejsException.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsException.c

${CC} -c -o macosx-x86_64-debug/obj/ejsHelper.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsHelper.c

${CC} -c -o macosx-x86_64-debug/obj/ejsInterp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsInterp.c

${CC} -c -o macosx-x86_64-debug/obj/ejsLoader.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsLoader.c

${CC} -c -o macosx-x86_64-debug/obj/ejsModule.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsModule.c

${CC} -c -o macosx-x86_64-debug/obj/ejsScope.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsScope.c

${CC} -c -o macosx-x86_64-debug/obj/ejsService.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsService.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/libejs.dylib -arch x86_64 ${LDFLAGS} -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -install_name @rpath/libejs.dylib macosx-x86_64-debug/obj/ecAst.o macosx-x86_64-debug/obj/ecCodeGen.o macosx-x86_64-debug/obj/ecCompiler.o macosx-x86_64-debug/obj/ecLex.o macosx-x86_64-debug/obj/ecModuleWrite.o macosx-x86_64-debug/obj/ecParser.o macosx-x86_64-debug/obj/ecState.o macosx-x86_64-debug/obj/ejsApp.o macosx-x86_64-debug/obj/ejsArray.o macosx-x86_64-debug/obj/ejsBlock.o macosx-x86_64-debug/obj/ejsBoolean.o macosx-x86_64-debug/obj/ejsByteArray.o macosx-x86_64-debug/obj/ejsCache.o macosx-x86_64-debug/obj/ejsCmd.o macosx-x86_64-debug/obj/ejsConfig.o macosx-x86_64-debug/obj/ejsDate.o macosx-x86_64-debug/obj/ejsDebug.o macosx-x86_64-debug/obj/ejsError.o macosx-x86_64-debug/obj/ejsFile.o macosx-x86_64-debug/obj/ejsFileSystem.o macosx-x86_64-debug/obj/ejsFrame.o macosx-x86_64-debug/obj/ejsFunction.o macosx-x86_64-debug/obj/ejsGC.o macosx-x86_64-debug/obj/ejsGlobal.o macosx-x86_64-debug/obj/ejsHttp.o macosx-x86_64-debug/obj/ejsIterator.o macosx-x86_64-debug/obj/ejsJSON.o macosx-x86_64-debug/obj/ejsLocalCache.o macosx-x86_64-debug/obj/ejsMath.o macosx-x86_64-debug/obj/ejsMemory.o macosx-x86_64-debug/obj/ejsMprLog.o macosx-x86_64-debug/obj/ejsNamespace.o macosx-x86_64-debug/obj/ejsNull.o macosx-x86_64-debug/obj/ejsNumber.o macosx-x86_64-debug/obj/ejsObject.o macosx-x86_64-debug/obj/ejsPath.o macosx-x86_64-debug/obj/ejsPot.o macosx-x86_64-debug/obj/ejsRegExp.o macosx-x86_64-debug/obj/ejsSocket.o macosx-x86_64-debug/obj/ejsString.o macosx-x86_64-debug/obj/ejsSystem.o macosx-x86_64-debug/obj/ejsTimer.o macosx-x86_64-debug/obj/ejsType.o macosx-x86_64-debug/obj/ejsUri.o macosx-x86_64-debug/obj/ejsVoid.o macosx-x86_64-debug/obj/ejsWorker.o macosx-x86_64-debug/obj/ejsXML.o macosx-x86_64-debug/obj/ejsXMLList.o macosx-x86_64-debug/obj/ejsXMLLoader.o macosx-x86_64-debug/obj/ejsByteCode.o macosx-x86_64-debug/obj/ejsException.o macosx-x86_64-debug/obj/ejsHelper.o macosx-x86_64-debug/obj/ejsInterp.o macosx-x86_64-debug/obj/ejsLoader.o macosx-x86_64-debug/obj/ejsModule.o macosx-x86_64-debug/obj/ejsScope.o macosx-x86_64-debug/obj/ejsService.o ${LIBS} -lmpr -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl

${CC} -c -o macosx-x86_64-debug/obj/ejs.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejs.c

${CC} -o macosx-x86_64-debug/bin/ejs -arch x86_64 ${LDFLAGS} -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/ejs.o ${LIBS} -lejs -lmpr -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl -ledit

${CC} -c -o macosx-x86_64-debug/obj/ejsc.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejsc.c

${CC} -o macosx-x86_64-debug/bin/ejsc -arch x86_64 ${LDFLAGS} -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/ejsc.o ${LIBS} -lejs -lmpr -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl

${CC} -c -o macosx-x86_64-debug/obj/ejsmod.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/ejsmod.c

${CC} -c -o macosx-x86_64-debug/obj/doc.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/doc.c

${CC} -c -o macosx-x86_64-debug/obj/docFiles.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/docFiles.c

${CC} -c -o macosx-x86_64-debug/obj/listing.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/listing.c

${CC} -c -o macosx-x86_64-debug/obj/slotGen.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/slotGen.c

${CC} -o macosx-x86_64-debug/bin/ejsmod -arch x86_64 ${LDFLAGS} -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/ejsmod.o macosx-x86_64-debug/obj/doc.o macosx-x86_64-debug/obj/docFiles.o macosx-x86_64-debug/obj/listing.o macosx-x86_64-debug/obj/slotGen.o ${LIBS} -lejs -lmpr -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl

${CC} -c -o macosx-x86_64-debug/obj/ejsrun.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejsrun.c

${CC} -o macosx-x86_64-debug/bin/ejsrun -arch x86_64 ${LDFLAGS} -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/ejsrun.o ${LIBS} -lejs -lmpr -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl

#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.mod undefined
#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/bin/bit.es undefined
#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/lib/bits undefined
#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.unix.mod undefined
#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/bin/jem.es undefined
#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.db.mod undefined
#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.db.mapper.mod undefined
#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.db.sqlite.mod undefined
${CC} -c -o macosx-x86_64-debug/obj/ejsSqlite.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/jems/ejs.db.sqlite/src/ejsSqlite.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/ejs.db.sqlite.dylib -arch x86_64 ${LDFLAGS} -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -install_name @rpath/ejs.db.sqlite.dylib macosx-x86_64-debug/obj/ejsSqlite.o ${LIBS} -lmpr -lejs -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl -lsqlite3

#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.web.mod undefined
${CC} -c -o macosx-x86_64-debug/obj/ejsHttpServer.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsHttpServer.c

${CC} -c -o macosx-x86_64-debug/obj/ejsRequest.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsRequest.c

${CC} -c -o macosx-x86_64-debug/obj/ejsSession.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsSession.c

${CC} -c -o macosx-x86_64-debug/obj/ejsWeb.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsWeb.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/ejs.web.dylib -arch x86_64 ${LDFLAGS} -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -install_name @rpath/ejs.web.dylib macosx-x86_64-debug/obj/ejsHttpServer.o macosx-x86_64-debug/obj/ejsRequest.o macosx-x86_64-debug/obj/ejsSession.o macosx-x86_64-debug/obj/ejsWeb.o ${LIBS} -lmpr -lhttp -lpcre -lmprssl -lssl -lcrypto -lmatrixssl -lpcre -lejs

#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.template.mod undefined
#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.tar.mod undefined
#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.zlib.mod undefined
${CC} -c -o macosx-x86_64-debug/obj/ejsZlib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Imacosx-x86_64-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -I../packages-macosx-x86_64/zlib/zlib-1.2.6 src/jems/ejs.zlib/src/ejsZlib.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/ejs.zlib.dylib -arch x86_64 ${LDFLAGS} -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -L/usr/lib -install_name @rpath/ejs.zlib.dylib macosx-x86_64-debug/obj/ejsZlib.o ${LIBS} -lmpr -lejs -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl -lz

#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/bin/mvc.es undefined
#  Omit script /Users/mob/git/ejs/macosx-x86_64-debug/lib/ejs.mvc.mod undefined
