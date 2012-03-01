#
#   build.mk -- Build It Makefile to build Embedthis Ejscript for macosx on x86_64
#

CC        := /usr/bin/cc
CFLAGS    := -fPIC -Wall -g -Wshorten-64-to-32
DFLAGS    := -DPIC -DCPU=X86_64
IFLAGS    := -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc
LDFLAGS   := -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L/Users/mob/git/ejs/macosx-x86_64-debug/lib -g -ldl
LIBS      := -lpthread -lm

all: \
        macosx-x86_64-debug/lib/libmpr.dylib \
        macosx-x86_64-debug/lib/libmprssl.dylib \
        macosx-x86_64-debug/bin/manager \
        macosx-x86_64-debug/bin/makerom \
        macosx-x86_64-debug/lib/libpcre.dylib \
        macosx-x86_64-debug/lib/libhttp.dylib \
        macosx-x86_64-debug/bin/http \
        macosx-x86_64-debug/lib/libsqlite3.dylib \
        macosx-x86_64-debug/inc/ejs.slots.h \
        macosx-x86_64-debug/lib/libejs.dylib \
        macosx-x86_64-debug/bin/ejs \
        macosx-x86_64-debug/bin/ejsc \
        macosx-x86_64-debug/bin/ejsmod \
        macosx-x86_64-debug/bin/ejsrun \
        macosx-x86_64-debug/lib/ejs.mod \
        macosx-x86_64-debug/bin/bit.es \
        macosx-x86_64-debug/bin/bit \
        macosx-x86_64-debug/lib/bits \
        macosx-x86_64-debug/lib/ejs.unix.mod \
        macosx-x86_64-debug/bin/jem.es \
        macosx-x86_64-debug/bin/jem \
        macosx-x86_64-debug/lib/ejs.db.mod \
        macosx-x86_64-debug/lib/ejs.db.mapper.mod \
        macosx-x86_64-debug/lib/ejs.db.sqlite.mod \
        macosx-x86_64-debug/lib/ejs.db.sqlite.dylib \
        macosx-x86_64-debug/lib/ejs.web.mod \
        macosx-x86_64-debug/lib/ejs.web.dylib \
        macosx-x86_64-debug/lib/ejs.template.mod \
        macosx-x86_64-debug/lib/ejs.tar.mod \
        macosx-x86_64-debug/lib/ejs.zlib.mod \
        macosx-x86_64-debug/lib/ejs.zlib.dylib \
        macosx-x86_64-debug/bin/mvc.es \
        macosx-x86_64-debug/bin/mvc \
        macosx-x86_64-debug/lib/ejs.mvc.mod

clean:
	rm -f macosx-x86_64-debug/lib/libmpr.dylib
	rm -f macosx-x86_64-debug/lib/libmprssl.dylib
	rm -f macosx-x86_64-debug/bin/manager
	rm -f macosx-x86_64-debug/bin/makerom
	rm -f macosx-x86_64-debug/lib/libpcre.dylib
	rm -f macosx-x86_64-debug/lib/libhttp.dylib
	rm -f macosx-x86_64-debug/bin/http
	rm -f macosx-x86_64-debug/lib/libsqlite3.dylib
	rm -f macosx-x86_64-debug/inc/ejs.slots.h
	rm -f macosx-x86_64-debug/lib/libejs.dylib
	rm -f macosx-x86_64-debug/bin/ejs
	rm -f macosx-x86_64-debug/bin/ejsc
	rm -f macosx-x86_64-debug/bin/ejsmod
	rm -f macosx-x86_64-debug/bin/ejsrun
	rm -f macosx-x86_64-debug/lib/ejs.mod
	rm -f macosx-x86_64-debug/bin/bit.es
	rm -f macosx-x86_64-debug/bin/bit
	rm -f macosx-x86_64-debug/lib/bits
	rm -f macosx-x86_64-debug/lib/ejs.unix.mod
	rm -f macosx-x86_64-debug/bin/jem.es
	rm -f macosx-x86_64-debug/bin/jem
	rm -f macosx-x86_64-debug/lib/ejs.db.mod
	rm -f macosx-x86_64-debug/lib/ejs.db.mapper.mod
	rm -f macosx-x86_64-debug/lib/ejs.db.sqlite.mod
	rm -f macosx-x86_64-debug/lib/ejs.db.sqlite.dylib
	rm -f macosx-x86_64-debug/lib/ejs.web.mod
	rm -f macosx-x86_64-debug/lib/ejs.web.dylib
	rm -f macosx-x86_64-debug/lib/ejs.template.mod
	rm -f macosx-x86_64-debug/lib/ejs.tar.mod
	rm -f macosx-x86_64-debug/lib/ejs.zlib.mod
	rm -f macosx-x86_64-debug/lib/ejs.zlib.dylib
	rm -f macosx-x86_64-debug/bin/mvc.es
	rm -f macosx-x86_64-debug/bin/mvc
	rm -f macosx-x86_64-debug/lib/ejs.mvc.mod
	rm -f macosx-x86_64-debug/obj/mprLib.o
	rm -f macosx-x86_64-debug/obj/mprSsl.o
	rm -f macosx-x86_64-debug/obj/manager.o
	rm -f macosx-x86_64-debug/obj/makerom.o
	rm -f macosx-x86_64-debug/obj/pcre.o
	rm -f macosx-x86_64-debug/obj/httpLib.o
	rm -f macosx-x86_64-debug/obj/http.o
	rm -f macosx-x86_64-debug/obj/sqlite3.o
	rm -f macosx-x86_64-debug/obj/ecAst.o
	rm -f macosx-x86_64-debug/obj/ecCodeGen.o
	rm -f macosx-x86_64-debug/obj/ecCompiler.o
	rm -f macosx-x86_64-debug/obj/ecLex.o
	rm -f macosx-x86_64-debug/obj/ecModuleWrite.o
	rm -f macosx-x86_64-debug/obj/ecParser.o
	rm -f macosx-x86_64-debug/obj/ecState.o
	rm -f macosx-x86_64-debug/obj/ejsApp.o
	rm -f macosx-x86_64-debug/obj/ejsArray.o
	rm -f macosx-x86_64-debug/obj/ejsBlock.o
	rm -f macosx-x86_64-debug/obj/ejsBoolean.o
	rm -f macosx-x86_64-debug/obj/ejsByteArray.o
	rm -f macosx-x86_64-debug/obj/ejsCache.o
	rm -f macosx-x86_64-debug/obj/ejsCmd.o
	rm -f macosx-x86_64-debug/obj/ejsConfig.o
	rm -f macosx-x86_64-debug/obj/ejsDate.o
	rm -f macosx-x86_64-debug/obj/ejsDebug.o
	rm -f macosx-x86_64-debug/obj/ejsError.o
	rm -f macosx-x86_64-debug/obj/ejsFile.o
	rm -f macosx-x86_64-debug/obj/ejsFileSystem.o
	rm -f macosx-x86_64-debug/obj/ejsFrame.o
	rm -f macosx-x86_64-debug/obj/ejsFunction.o
	rm -f macosx-x86_64-debug/obj/ejsGC.o
	rm -f macosx-x86_64-debug/obj/ejsGlobal.o
	rm -f macosx-x86_64-debug/obj/ejsHttp.o
	rm -f macosx-x86_64-debug/obj/ejsIterator.o
	rm -f macosx-x86_64-debug/obj/ejsJSON.o
	rm -f macosx-x86_64-debug/obj/ejsLocalCache.o
	rm -f macosx-x86_64-debug/obj/ejsMath.o
	rm -f macosx-x86_64-debug/obj/ejsMemory.o
	rm -f macosx-x86_64-debug/obj/ejsMprLog.o
	rm -f macosx-x86_64-debug/obj/ejsNamespace.o
	rm -f macosx-x86_64-debug/obj/ejsNull.o
	rm -f macosx-x86_64-debug/obj/ejsNumber.o
	rm -f macosx-x86_64-debug/obj/ejsObject.o
	rm -f macosx-x86_64-debug/obj/ejsPath.o
	rm -f macosx-x86_64-debug/obj/ejsPot.o
	rm -f macosx-x86_64-debug/obj/ejsRegExp.o
	rm -f macosx-x86_64-debug/obj/ejsSocket.o
	rm -f macosx-x86_64-debug/obj/ejsString.o
	rm -f macosx-x86_64-debug/obj/ejsSystem.o
	rm -f macosx-x86_64-debug/obj/ejsTimer.o
	rm -f macosx-x86_64-debug/obj/ejsType.o
	rm -f macosx-x86_64-debug/obj/ejsUri.o
	rm -f macosx-x86_64-debug/obj/ejsVoid.o
	rm -f macosx-x86_64-debug/obj/ejsWorker.o
	rm -f macosx-x86_64-debug/obj/ejsXML.o
	rm -f macosx-x86_64-debug/obj/ejsXMLList.o
	rm -f macosx-x86_64-debug/obj/ejsXMLLoader.o
	rm -f macosx-x86_64-debug/obj/ejsByteCode.o
	rm -f macosx-x86_64-debug/obj/ejsException.o
	rm -f macosx-x86_64-debug/obj/ejsHelper.o
	rm -f macosx-x86_64-debug/obj/ejsInterp.o
	rm -f macosx-x86_64-debug/obj/ejsLoader.o
	rm -f macosx-x86_64-debug/obj/ejsModule.o
	rm -f macosx-x86_64-debug/obj/ejsScope.o
	rm -f macosx-x86_64-debug/obj/ejsService.o
	rm -f macosx-x86_64-debug/obj/ejs.o
	rm -f macosx-x86_64-debug/obj/ejsc.o
	rm -f macosx-x86_64-debug/obj/ejsmod.o
	rm -f macosx-x86_64-debug/obj/doc.o
	rm -f macosx-x86_64-debug/obj/docFiles.o
	rm -f macosx-x86_64-debug/obj/listing.o
	rm -f macosx-x86_64-debug/obj/slotGen.o
	rm -f macosx-x86_64-debug/obj/ejsrun.o
	rm -f macosx-x86_64-debug/obj/ejsSqlite.o
	rm -f macosx-x86_64-debug/obj/ejsFilter.o
	rm -f macosx-x86_64-debug/obj/ejsHttpServer.o
	rm -f macosx-x86_64-debug/obj/ejsRequest.o
	rm -f macosx-x86_64-debug/obj/ejsSession.o
	rm -f macosx-x86_64-debug/obj/ejsWeb.o
	rm -f macosx-x86_64-debug/obj/ejsZlib.o

macosx-x86_64-debug/obj/mprLib.o: \
        src/deps/mpr/mprLib.c \
        macosx-x86_64-debug/inc/bit.h \
        src/deps/mpr/mpr.h
	$(CC) -c -o macosx-x86_64-debug/obj/mprLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/deps/mpr/mprLib.c

macosx-x86_64-debug/lib/libmpr.dylib:  \
        macosx-x86_64-debug/obj/mprLib.o
	$(CC) -dynamiclib -o macosx-x86_64-debug/lib/libmpr.dylib -arch x86_64 $(LDFLAGS) -install_name @rpath/libmpr.dylib macosx-x86_64-debug/obj/mprLib.o $(LIBS)

macosx-x86_64-debug/obj/mprSsl.o: \
        src/deps/mpr/mprSsl.c \
        macosx-x86_64-debug/inc/bit.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        src/deps/mpr/mpr.h \
        src/deps/mpr/mpr.h \
        src/deps/mpr/mpr.h
	$(CC) -c -o macosx-x86_64-debug/obj/mprSsl.o -arch x86_64 $(CFLAGS) $(DFLAGS) -DPOSIX -DMATRIX_USE_FILE_SYSTEM -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc -I../packages-macosx-x86_64/openssl/openssl-1.0.0d/include -I../packages-macosx-x86_64/matrixssl/matrixssl-3-3-open/matrixssl -I../packages-macosx-x86_64/matrixssl/matrixssl-3-3-open src/deps/mpr/mprSsl.c

macosx-x86_64-debug/lib/libmprssl.dylib:  \
        macosx-x86_64-debug/lib/libmpr.dylib \
        macosx-x86_64-debug/obj/mprSsl.o
	$(CC) -dynamiclib -o macosx-x86_64-debug/lib/libmprssl.dylib -arch x86_64 $(LDFLAGS) -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -install_name @rpath/libmprssl.dylib macosx-x86_64-debug/obj/mprSsl.o $(LIBS) -lmpr -lssl -lcrypto -lmatrixssl

macosx-x86_64-debug/obj/manager.o: \
        src/deps/mpr/manager.c \
        macosx-x86_64-debug/inc/bit.h \
        src/deps/mpr/mpr.h
	$(CC) -c -o macosx-x86_64-debug/obj/manager.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/deps/mpr/manager.c

macosx-x86_64-debug/bin/manager:  \
        macosx-x86_64-debug/lib/libmpr.dylib \
        macosx-x86_64-debug/obj/manager.o
	$(CC) -o macosx-x86_64-debug/bin/manager -arch x86_64 $(LDFLAGS) -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/manager.o $(LIBS) -lmpr

macosx-x86_64-debug/obj/makerom.o: \
        src/deps/mpr/makerom.c \
        macosx-x86_64-debug/inc/bit.h \
        src/deps/mpr/mpr.h
	$(CC) -c -o macosx-x86_64-debug/obj/makerom.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/deps/mpr/makerom.c

macosx-x86_64-debug/bin/makerom:  \
        macosx-x86_64-debug/lib/libmpr.dylib \
        macosx-x86_64-debug/obj/makerom.o
	$(CC) -o macosx-x86_64-debug/bin/makerom -arch x86_64 $(LDFLAGS) -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/makerom.o $(LIBS) -lmpr

macosx-x86_64-debug/obj/pcre.o: \
        src/deps/pcre/pcre.c \
        macosx-x86_64-debug/inc/bit.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        src/deps/pcre/pcre.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        macosx-x86_64-debug/inc/buildConfig.h
	$(CC) -c -o macosx-x86_64-debug/obj/pcre.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/deps/pcre/pcre.c

macosx-x86_64-debug/lib/libpcre.dylib:  \
        macosx-x86_64-debug/obj/pcre.o
	$(CC) -dynamiclib -o macosx-x86_64-debug/lib/libpcre.dylib -arch x86_64 $(LDFLAGS) -install_name @rpath/libpcre.dylib macosx-x86_64-debug/obj/pcre.o $(LIBS)

macosx-x86_64-debug/obj/httpLib.o: \
        src/deps/http/httpLib.c \
        macosx-x86_64-debug/inc/bit.h \
        src/deps/http/http.h \
        src/deps/pcre/pcre.h
	$(CC) -c -o macosx-x86_64-debug/obj/httpLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/deps/http/httpLib.c

macosx-x86_64-debug/lib/libhttp.dylib:  \
        macosx-x86_64-debug/lib/libmpr.dylib \
        macosx-x86_64-debug/lib/libpcre.dylib \
        macosx-x86_64-debug/lib/libmprssl.dylib \
        macosx-x86_64-debug/obj/httpLib.o
	$(CC) -dynamiclib -o macosx-x86_64-debug/lib/libhttp.dylib -arch x86_64 $(LDFLAGS) -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -install_name @rpath/libhttp.dylib macosx-x86_64-debug/obj/httpLib.o $(LIBS) -lmpr -lpcre -lmprssl -lssl -lcrypto -lmatrixssl

macosx-x86_64-debug/obj/http.o: \
        src/deps/http/http.c \
        macosx-x86_64-debug/inc/bit.h \
        src/deps/http/http.h
	$(CC) -c -o macosx-x86_64-debug/obj/http.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/deps/http/http.c

macosx-x86_64-debug/bin/http:  \
        macosx-x86_64-debug/lib/libhttp.dylib \
        macosx-x86_64-debug/obj/http.o
	$(CC) -o macosx-x86_64-debug/bin/http -arch x86_64 $(LDFLAGS) -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/http.o $(LIBS) -lhttp -lmpr -lpcre -lmprssl -lssl -lcrypto -lmatrixssl

macosx-x86_64-debug/obj/sqlite3.o: \
        src/deps/sqlite/sqlite3.c \
        macosx-x86_64-debug/inc/bit.h \
        macosx-x86_64-debug/inc/buildConfig.h \
        src/deps/sqlite/sqlite3.h
	$(CC) -c -o macosx-x86_64-debug/obj/sqlite3.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/deps/sqlite/sqlite3.c

macosx-x86_64-debug/lib/libsqlite3.dylib:  \
        macosx-x86_64-debug/obj/sqlite3.o
	$(CC) -dynamiclib -o macosx-x86_64-debug/lib/libsqlite3.dylib -arch x86_64 $(LDFLAGS) -install_name @rpath/libsqlite3.dylib macosx-x86_64-debug/obj/sqlite3.o $(LIBS)

macosx-x86_64-debug/obj/ecAst.o: \
        src/compiler/ecAst.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejsCompiler.h
	$(CC) -c -o macosx-x86_64-debug/obj/ecAst.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/compiler/ecAst.c

macosx-x86_64-debug/obj/ecCodeGen.o: \
        src/compiler/ecCodeGen.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejsCompiler.h
	$(CC) -c -o macosx-x86_64-debug/obj/ecCodeGen.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/compiler/ecCodeGen.c

macosx-x86_64-debug/obj/ecCompiler.o: \
        src/compiler/ecCompiler.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejsCompiler.h
	$(CC) -c -o macosx-x86_64-debug/obj/ecCompiler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/compiler/ecCompiler.c

macosx-x86_64-debug/obj/ecLex.o: \
        src/compiler/ecLex.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejsCompiler.h
	$(CC) -c -o macosx-x86_64-debug/obj/ecLex.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/compiler/ecLex.c

macosx-x86_64-debug/obj/ecModuleWrite.o: \
        src/compiler/ecModuleWrite.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejsCompiler.h
	$(CC) -c -o macosx-x86_64-debug/obj/ecModuleWrite.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/compiler/ecModuleWrite.c

macosx-x86_64-debug/obj/ecParser.o: \
        src/compiler/ecParser.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejsCompiler.h
	$(CC) -c -o macosx-x86_64-debug/obj/ecParser.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/compiler/ecParser.c

macosx-x86_64-debug/obj/ecState.o: \
        src/compiler/ecState.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejsCompiler.h
	$(CC) -c -o macosx-x86_64-debug/obj/ecState.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/compiler/ecState.c

macosx-x86_64-debug/obj/ejsApp.o: \
        src/core/src/ejsApp.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsApp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsApp.c

macosx-x86_64-debug/obj/ejsArray.o: \
        src/core/src/ejsArray.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsArray.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsArray.c

macosx-x86_64-debug/obj/ejsBlock.o: \
        src/core/src/ejsBlock.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsBlock.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsBlock.c

macosx-x86_64-debug/obj/ejsBoolean.o: \
        src/core/src/ejsBoolean.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsBoolean.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsBoolean.c

macosx-x86_64-debug/obj/ejsByteArray.o: \
        src/core/src/ejsByteArray.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsByteArray.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsByteArray.c

macosx-x86_64-debug/obj/ejsCache.o: \
        src/core/src/ejsCache.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsCache.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsCache.c

macosx-x86_64-debug/obj/ejsCmd.o: \
        src/core/src/ejsCmd.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsCmd.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsCmd.c

macosx-x86_64-debug/obj/ejsConfig.o: \
        src/core/src/ejsConfig.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsConfig.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsConfig.c

macosx-x86_64-debug/obj/ejsDate.o: \
        src/core/src/ejsDate.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsDate.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsDate.c

macosx-x86_64-debug/obj/ejsDebug.o: \
        src/core/src/ejsDebug.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsDebug.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsDebug.c

macosx-x86_64-debug/obj/ejsError.o: \
        src/core/src/ejsError.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsError.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsError.c

macosx-x86_64-debug/obj/ejsFile.o: \
        src/core/src/ejsFile.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsFile.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsFile.c

macosx-x86_64-debug/obj/ejsFileSystem.o: \
        src/core/src/ejsFileSystem.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsFileSystem.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsFileSystem.c

macosx-x86_64-debug/obj/ejsFrame.o: \
        src/core/src/ejsFrame.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsFrame.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsFrame.c

macosx-x86_64-debug/obj/ejsFunction.o: \
        src/core/src/ejsFunction.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsFunction.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsFunction.c

macosx-x86_64-debug/obj/ejsGC.o: \
        src/core/src/ejsGC.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsGC.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsGC.c

macosx-x86_64-debug/obj/ejsGlobal.o: \
        src/core/src/ejsGlobal.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsGlobal.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsGlobal.c

macosx-x86_64-debug/obj/ejsHttp.o: \
        src/core/src/ejsHttp.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsHttp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsHttp.c

macosx-x86_64-debug/obj/ejsIterator.o: \
        src/core/src/ejsIterator.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsIterator.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsIterator.c

macosx-x86_64-debug/obj/ejsJSON.o: \
        src/core/src/ejsJSON.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsJSON.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsJSON.c

macosx-x86_64-debug/obj/ejsLocalCache.o: \
        src/core/src/ejsLocalCache.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsLocalCache.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsLocalCache.c

macosx-x86_64-debug/obj/ejsMath.o: \
        src/core/src/ejsMath.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsMath.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsMath.c

macosx-x86_64-debug/obj/ejsMemory.o: \
        src/core/src/ejsMemory.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsMemory.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsMemory.c

macosx-x86_64-debug/obj/ejsMprLog.o: \
        src/core/src/ejsMprLog.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsMprLog.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsMprLog.c

macosx-x86_64-debug/obj/ejsNamespace.o: \
        src/core/src/ejsNamespace.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsNamespace.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsNamespace.c

macosx-x86_64-debug/obj/ejsNull.o: \
        src/core/src/ejsNull.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsNull.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsNull.c

macosx-x86_64-debug/obj/ejsNumber.o: \
        src/core/src/ejsNumber.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsNumber.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsNumber.c

macosx-x86_64-debug/obj/ejsObject.o: \
        src/core/src/ejsObject.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsObject.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsObject.c

macosx-x86_64-debug/obj/ejsPath.o: \
        src/core/src/ejsPath.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h \
        src/deps/pcre/pcre.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsPath.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsPath.c

macosx-x86_64-debug/obj/ejsPot.o: \
        src/core/src/ejsPot.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsPot.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsPot.c

macosx-x86_64-debug/obj/ejsRegExp.o: \
        src/core/src/ejsRegExp.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h \
        src/deps/pcre/pcre.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsRegExp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsRegExp.c

macosx-x86_64-debug/obj/ejsSocket.o: \
        src/core/src/ejsSocket.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsSocket.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsSocket.c

macosx-x86_64-debug/obj/ejsString.o: \
        src/core/src/ejsString.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h \
        src/deps/pcre/pcre.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsString.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsString.c

macosx-x86_64-debug/obj/ejsSystem.o: \
        src/core/src/ejsSystem.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsSystem.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsSystem.c

macosx-x86_64-debug/obj/ejsTimer.o: \
        src/core/src/ejsTimer.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsTimer.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsTimer.c

macosx-x86_64-debug/obj/ejsType.o: \
        src/core/src/ejsType.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsType.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsType.c

macosx-x86_64-debug/obj/ejsUri.o: \
        src/core/src/ejsUri.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsUri.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsUri.c

macosx-x86_64-debug/obj/ejsVoid.o: \
        src/core/src/ejsVoid.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsVoid.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsVoid.c

macosx-x86_64-debug/obj/ejsWorker.o: \
        src/core/src/ejsWorker.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsWorker.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsWorker.c

macosx-x86_64-debug/obj/ejsXML.o: \
        src/core/src/ejsXML.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsXML.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsXML.c

macosx-x86_64-debug/obj/ejsXMLList.o: \
        src/core/src/ejsXMLList.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsXMLList.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsXMLList.c

macosx-x86_64-debug/obj/ejsXMLLoader.o: \
        src/core/src/ejsXMLLoader.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsXMLLoader.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/core/src/ejsXMLLoader.c

macosx-x86_64-debug/obj/ejsByteCode.o: \
        src/vm/ejsByteCode.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsByteCode.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/vm/ejsByteCode.c

macosx-x86_64-debug/obj/ejsException.o: \
        src/vm/ejsException.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsException.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/vm/ejsException.c

macosx-x86_64-debug/obj/ejsHelper.o: \
        src/vm/ejsHelper.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsHelper.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/vm/ejsHelper.c

macosx-x86_64-debug/obj/ejsInterp.o: \
        src/vm/ejsInterp.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsInterp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/vm/ejsInterp.c

macosx-x86_64-debug/obj/ejsLoader.o: \
        src/vm/ejsLoader.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsLoader.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/vm/ejsLoader.c

macosx-x86_64-debug/obj/ejsModule.o: \
        src/vm/ejsModule.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsModule.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/vm/ejsModule.c

macosx-x86_64-debug/obj/ejsScope.o: \
        src/vm/ejsScope.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsScope.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/vm/ejsScope.c

macosx-x86_64-debug/obj/ejsService.o: \
        src/vm/ejsService.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsService.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/vm/ejsService.c

macosx-x86_64-debug/lib/libejs.dylib:  \
        macosx-x86_64-debug/lib/libmpr.dylib \
        macosx-x86_64-debug/lib/libpcre.dylib \
        macosx-x86_64-debug/lib/libhttp.dylib \
        macosx-x86_64-debug/inc/ejs.slots.h \
        macosx-x86_64-debug/obj/ecAst.o \
        macosx-x86_64-debug/obj/ecCodeGen.o \
        macosx-x86_64-debug/obj/ecCompiler.o \
        macosx-x86_64-debug/obj/ecLex.o \
        macosx-x86_64-debug/obj/ecModuleWrite.o \
        macosx-x86_64-debug/obj/ecParser.o \
        macosx-x86_64-debug/obj/ecState.o \
        macosx-x86_64-debug/obj/ejsApp.o \
        macosx-x86_64-debug/obj/ejsArray.o \
        macosx-x86_64-debug/obj/ejsBlock.o \
        macosx-x86_64-debug/obj/ejsBoolean.o \
        macosx-x86_64-debug/obj/ejsByteArray.o \
        macosx-x86_64-debug/obj/ejsCache.o \
        macosx-x86_64-debug/obj/ejsCmd.o \
        macosx-x86_64-debug/obj/ejsConfig.o \
        macosx-x86_64-debug/obj/ejsDate.o \
        macosx-x86_64-debug/obj/ejsDebug.o \
        macosx-x86_64-debug/obj/ejsError.o \
        macosx-x86_64-debug/obj/ejsFile.o \
        macosx-x86_64-debug/obj/ejsFileSystem.o \
        macosx-x86_64-debug/obj/ejsFrame.o \
        macosx-x86_64-debug/obj/ejsFunction.o \
        macosx-x86_64-debug/obj/ejsGC.o \
        macosx-x86_64-debug/obj/ejsGlobal.o \
        macosx-x86_64-debug/obj/ejsHttp.o \
        macosx-x86_64-debug/obj/ejsIterator.o \
        macosx-x86_64-debug/obj/ejsJSON.o \
        macosx-x86_64-debug/obj/ejsLocalCache.o \
        macosx-x86_64-debug/obj/ejsMath.o \
        macosx-x86_64-debug/obj/ejsMemory.o \
        macosx-x86_64-debug/obj/ejsMprLog.o \
        macosx-x86_64-debug/obj/ejsNamespace.o \
        macosx-x86_64-debug/obj/ejsNull.o \
        macosx-x86_64-debug/obj/ejsNumber.o \
        macosx-x86_64-debug/obj/ejsObject.o \
        macosx-x86_64-debug/obj/ejsPath.o \
        macosx-x86_64-debug/obj/ejsPot.o \
        macosx-x86_64-debug/obj/ejsRegExp.o \
        macosx-x86_64-debug/obj/ejsSocket.o \
        macosx-x86_64-debug/obj/ejsString.o \
        macosx-x86_64-debug/obj/ejsSystem.o \
        macosx-x86_64-debug/obj/ejsTimer.o \
        macosx-x86_64-debug/obj/ejsType.o \
        macosx-x86_64-debug/obj/ejsUri.o \
        macosx-x86_64-debug/obj/ejsVoid.o \
        macosx-x86_64-debug/obj/ejsWorker.o \
        macosx-x86_64-debug/obj/ejsXML.o \
        macosx-x86_64-debug/obj/ejsXMLList.o \
        macosx-x86_64-debug/obj/ejsXMLLoader.o \
        macosx-x86_64-debug/obj/ejsByteCode.o \
        macosx-x86_64-debug/obj/ejsException.o \
        macosx-x86_64-debug/obj/ejsHelper.o \
        macosx-x86_64-debug/obj/ejsInterp.o \
        macosx-x86_64-debug/obj/ejsLoader.o \
        macosx-x86_64-debug/obj/ejsModule.o \
        macosx-x86_64-debug/obj/ejsScope.o \
        macosx-x86_64-debug/obj/ejsService.o
	$(CC) -dynamiclib -o macosx-x86_64-debug/lib/libejs.dylib -arch x86_64 $(LDFLAGS) -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -install_name @rpath/libejs.dylib macosx-x86_64-debug/obj/ecAst.o macosx-x86_64-debug/obj/ecCodeGen.o macosx-x86_64-debug/obj/ecCompiler.o macosx-x86_64-debug/obj/ecLex.o macosx-x86_64-debug/obj/ecModuleWrite.o macosx-x86_64-debug/obj/ecParser.o macosx-x86_64-debug/obj/ecState.o macosx-x86_64-debug/obj/ejsApp.o macosx-x86_64-debug/obj/ejsArray.o macosx-x86_64-debug/obj/ejsBlock.o macosx-x86_64-debug/obj/ejsBoolean.o macosx-x86_64-debug/obj/ejsByteArray.o macosx-x86_64-debug/obj/ejsCache.o macosx-x86_64-debug/obj/ejsCmd.o macosx-x86_64-debug/obj/ejsConfig.o macosx-x86_64-debug/obj/ejsDate.o macosx-x86_64-debug/obj/ejsDebug.o macosx-x86_64-debug/obj/ejsError.o macosx-x86_64-debug/obj/ejsFile.o macosx-x86_64-debug/obj/ejsFileSystem.o macosx-x86_64-debug/obj/ejsFrame.o macosx-x86_64-debug/obj/ejsFunction.o macosx-x86_64-debug/obj/ejsGC.o macosx-x86_64-debug/obj/ejsGlobal.o macosx-x86_64-debug/obj/ejsHttp.o macosx-x86_64-debug/obj/ejsIterator.o macosx-x86_64-debug/obj/ejsJSON.o macosx-x86_64-debug/obj/ejsLocalCache.o macosx-x86_64-debug/obj/ejsMath.o macosx-x86_64-debug/obj/ejsMemory.o macosx-x86_64-debug/obj/ejsMprLog.o macosx-x86_64-debug/obj/ejsNamespace.o macosx-x86_64-debug/obj/ejsNull.o macosx-x86_64-debug/obj/ejsNumber.o macosx-x86_64-debug/obj/ejsObject.o macosx-x86_64-debug/obj/ejsPath.o macosx-x86_64-debug/obj/ejsPot.o macosx-x86_64-debug/obj/ejsRegExp.o macosx-x86_64-debug/obj/ejsSocket.o macosx-x86_64-debug/obj/ejsString.o macosx-x86_64-debug/obj/ejsSystem.o macosx-x86_64-debug/obj/ejsTimer.o macosx-x86_64-debug/obj/ejsType.o macosx-x86_64-debug/obj/ejsUri.o macosx-x86_64-debug/obj/ejsVoid.o macosx-x86_64-debug/obj/ejsWorker.o macosx-x86_64-debug/obj/ejsXML.o macosx-x86_64-debug/obj/ejsXMLList.o macosx-x86_64-debug/obj/ejsXMLLoader.o macosx-x86_64-debug/obj/ejsByteCode.o macosx-x86_64-debug/obj/ejsException.o macosx-x86_64-debug/obj/ejsHelper.o macosx-x86_64-debug/obj/ejsInterp.o macosx-x86_64-debug/obj/ejsLoader.o macosx-x86_64-debug/obj/ejsModule.o macosx-x86_64-debug/obj/ejsScope.o macosx-x86_64-debug/obj/ejsService.o $(LIBS) -lmpr -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl

macosx-x86_64-debug/obj/ejs.o: \
        src/cmd/ejs.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejsCompiler.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejs.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/cmd/ejs.c

macosx-x86_64-debug/bin/ejs:  \
        macosx-x86_64-debug/lib/libejs.dylib \
        macosx-x86_64-debug/obj/ejs.o
	$(CC) -o macosx-x86_64-debug/bin/ejs -arch x86_64 $(LDFLAGS) -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/ejs.o $(LIBS) -lejs -lmpr -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl -ledit -ledit

macosx-x86_64-debug/obj/ejsc.o: \
        src/cmd/ejsc.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejsCompiler.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsc.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/cmd/ejsc.c

macosx-x86_64-debug/bin/ejsc:  \
        macosx-x86_64-debug/lib/libejs.dylib \
        macosx-x86_64-debug/obj/ejsc.o
	$(CC) -o macosx-x86_64-debug/bin/ejsc -arch x86_64 $(LDFLAGS) -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/ejsc.o $(LIBS) -lejs -lmpr -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl

macosx-x86_64-debug/obj/ejsmod.o: \
        src/cmd/ejsmod.c \
        macosx-x86_64-debug/inc/bit.h \
        src/cmd/ejsmod.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsmod.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/cmd -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/cmd/ejsmod.c

macosx-x86_64-debug/obj/doc.o: \
        src/cmd/doc.c \
        macosx-x86_64-debug/inc/bit.h \
        src/cmd/ejsmod.h
	$(CC) -c -o macosx-x86_64-debug/obj/doc.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/cmd -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/cmd/doc.c

macosx-x86_64-debug/obj/docFiles.o: \
        src/cmd/docFiles.c \
        macosx-x86_64-debug/inc/bit.h \
        src/cmd/ejsmod.h
	$(CC) -c -o macosx-x86_64-debug/obj/docFiles.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/cmd -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/cmd/docFiles.c

macosx-x86_64-debug/obj/listing.o: \
        src/cmd/listing.c \
        macosx-x86_64-debug/inc/bit.h \
        src/cmd/ejsmod.h \
        src/ejsByteCodeTable.h
	$(CC) -c -o macosx-x86_64-debug/obj/listing.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/cmd -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/cmd/listing.c

macosx-x86_64-debug/obj/slotGen.o: \
        src/cmd/slotGen.c \
        macosx-x86_64-debug/inc/bit.h \
        src/cmd/ejsmod.h
	$(CC) -c -o macosx-x86_64-debug/obj/slotGen.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/cmd -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/cmd/slotGen.c

macosx-x86_64-debug/bin/ejsmod:  \
        macosx-x86_64-debug/lib/libejs.dylib \
        macosx-x86_64-debug/obj/ejsmod.o \
        macosx-x86_64-debug/obj/doc.o \
        macosx-x86_64-debug/obj/docFiles.o \
        macosx-x86_64-debug/obj/listing.o \
        macosx-x86_64-debug/obj/slotGen.o
	$(CC) -o macosx-x86_64-debug/bin/ejsmod -arch x86_64 $(LDFLAGS) -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/ejsmod.o macosx-x86_64-debug/obj/doc.o macosx-x86_64-debug/obj/docFiles.o macosx-x86_64-debug/obj/listing.o macosx-x86_64-debug/obj/slotGen.o $(LIBS) -lejs -lmpr -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl

macosx-x86_64-debug/obj/ejsrun.o: \
        src/cmd/ejsrun.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejsCompiler.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsrun.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/cmd/ejsrun.c

macosx-x86_64-debug/bin/ejsrun:  \
        macosx-x86_64-debug/lib/libejs.dylib \
        macosx-x86_64-debug/obj/ejsrun.o
	$(CC) -o macosx-x86_64-debug/bin/ejsrun -arch x86_64 $(LDFLAGS) -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/ejsrun.o $(LIBS) -lejs -lmpr -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl

macosx-x86_64-debug/obj/ejsSqlite.o: \
        src/jems/ejs.db.sqlite/src/ejsSqlite.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h \
        src/deps/sqlite/sqlite3.h \
        src/slots/ejs.db.sqlite.slots.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsSqlite.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/jems/ejs.db.sqlite/src/ejsSqlite.c

macosx-x86_64-debug/lib/ejs.db.sqlite.dylib:  \
        macosx-x86_64-debug/lib/libmpr.dylib \
        macosx-x86_64-debug/lib/libejs.dylib \
        macosx-x86_64-debug/lib/ejs.mod \
        macosx-x86_64-debug/lib/ejs.db.sqlite.mod \
        macosx-x86_64-debug/lib/libsqlite3.dylib \
        macosx-x86_64-debug/obj/ejsSqlite.o
	$(CC) -dynamiclib -o macosx-x86_64-debug/lib/ejs.db.sqlite.dylib -arch x86_64 $(LDFLAGS) -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -install_name @rpath/ejs.db.sqlite.dylib macosx-x86_64-debug/obj/ejsSqlite.o $(LIBS) -lmpr -lejs -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl -lsqlite3

macosx-x86_64-debug/obj/ejsFilter.o: \
        src/jems/ejs.web/src/ejsFilter.c \
        macosx-x86_64-debug/inc/bit.h \
        src/deps/http/http.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsFilter.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/jems/ejs.web/src -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/jems/ejs.web/src/ejsFilter.c

macosx-x86_64-debug/obj/ejsHttpServer.o: \
        src/jems/ejs.web/src/ejsHttpServer.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h \
        src/ejsCompiler.h \
        src/jems/ejs.web/src/ejsWeb.h \
        src/slots/ejs.web.slots.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsHttpServer.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/jems/ejs.web/src -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/jems/ejs.web/src/ejsHttpServer.c

macosx-x86_64-debug/obj/ejsRequest.o: \
        src/jems/ejs.web/src/ejsRequest.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h \
        src/ejsCompiler.h \
        src/jems/ejs.web/src/ejsWeb.h \
        src/slots/ejs.web.slots.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsRequest.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/jems/ejs.web/src -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/jems/ejs.web/src/ejsRequest.c

macosx-x86_64-debug/obj/ejsSession.o: \
        src/jems/ejs.web/src/ejsSession.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h \
        src/jems/ejs.web/src/ejsWeb.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsSession.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/jems/ejs.web/src -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/jems/ejs.web/src/ejsSession.c

macosx-x86_64-debug/obj/ejsWeb.o: \
        src/jems/ejs.web/src/ejsWeb.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h \
        src/ejsCompiler.h \
        src/jems/ejs.web/src/ejsWeb.h \
        src/slots/ejs.web.slots.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsWeb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/jems/ejs.web/src -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc src/jems/ejs.web/src/ejsWeb.c

macosx-x86_64-debug/lib/ejs.web.dylib:  \
        macosx-x86_64-debug/lib/libmpr.dylib \
        macosx-x86_64-debug/lib/libhttp.dylib \
        macosx-x86_64-debug/lib/libpcre.dylib \
        macosx-x86_64-debug/lib/libejs.dylib \
        macosx-x86_64-debug/lib/ejs.mod \
        macosx-x86_64-debug/obj/ejsFilter.o \
        macosx-x86_64-debug/obj/ejsHttpServer.o \
        macosx-x86_64-debug/obj/ejsRequest.o \
        macosx-x86_64-debug/obj/ejsSession.o \
        macosx-x86_64-debug/obj/ejsWeb.o
	$(CC) -dynamiclib -o macosx-x86_64-debug/lib/ejs.web.dylib -arch x86_64 $(LDFLAGS) -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -install_name @rpath/ejs.web.dylib macosx-x86_64-debug/obj/ejsFilter.o macosx-x86_64-debug/obj/ejsHttpServer.o macosx-x86_64-debug/obj/ejsRequest.o macosx-x86_64-debug/obj/ejsSession.o macosx-x86_64-debug/obj/ejsWeb.o $(LIBS) -lmpr -lhttp -lpcre -lmprssl -lssl -lcrypto -lmatrixssl -lpcre -lejs

macosx-x86_64-debug/obj/ejsZlib.o: \
        src/jems/ejs.zlib/src/ejsZlib.c \
        macosx-x86_64-debug/inc/bit.h \
        src/ejs.h \
        ../packages-macosx-x86_64/zlib/zlib-1.2.6/zlib.h \
        src/slots/ejs.zlib.slots.h
	$(CC) -c -o macosx-x86_64-debug/obj/ejsZlib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc/slots -Isrc -Imacosx-x86_64-debug/inc -I../packages-macosx-x86_64/zlib/zlib-1.2.6 src/jems/ejs.zlib/src/ejsZlib.c

macosx-x86_64-debug/lib/ejs.zlib.dylib:  \
        macosx-x86_64-debug/lib/libmpr.dylib \
        macosx-x86_64-debug/lib/libejs.dylib \
        macosx-x86_64-debug/lib/ejs.mod \
        macosx-x86_64-debug/lib/ejs.zlib.mod \
        macosx-x86_64-debug/obj/ejsZlib.o
	$(CC) -dynamiclib -o macosx-x86_64-debug/lib/ejs.zlib.dylib -arch x86_64 $(LDFLAGS) -L/Users/mob/git/packages-macosx-x86_64/openssl/openssl-1.0.0d -L/Users/mob/git/packages-macosx-x86_64/matrixssl/matrixssl-3-3-open -L/Users/mob/git/packages-macosx-x86_64/zlib/zlib-1.2.6 -install_name @rpath/ejs.zlib.dylib macosx-x86_64-debug/obj/ejsZlib.o $(LIBS) -lmpr -lejs -lpcre -lhttp -lmprssl -lssl -lcrypto -lmatrixssl -lz

