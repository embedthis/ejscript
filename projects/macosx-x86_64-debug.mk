#
#   macosx-x86_64-debug.mk -- Build It Makefile to build Embedthis Ejscript for macosx on x86_64
#

CONFIG   := macosx-x86_64-debug
CC       := /usr/bin/cc
LD       := /usr/bin/ld
CFLAGS   := -fPIC -Wall -fast -Wshorten-64-to-32
DFLAGS   := -DPIC -DCPU=X86_64
IFLAGS   := -I$(CONFIG)/inc -I$(CONFIG)/inc
LDFLAGS  := '-Wl,-rpath,@executable_path/../lib' '-Wl,-rpath,@executable_path/' '-Wl,-rpath,@loader_path/'
LIBPATHS := -L$(CONFIG)/lib
LIBS     := -lpthread -lm -ldl

all: prep \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/bin/ejsman \
        $(CONFIG)/bin/makerom \
        $(CONFIG)/lib/libpcre.dylib \
        $(CONFIG)/lib/libhttp.dylib \
        $(CONFIG)/bin/http \
        $(CONFIG)/lib/libsqlite3.dylib \
        $(CONFIG)/bin/sqlite \
        $(CONFIG)/lib/libejs.dylib \
        $(CONFIG)/bin/ejs \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/bin/ejsrun \
        $(CONFIG)/lib/ejs.mod \
        $(CONFIG)/bin/bit.es \
        $(CONFIG)/bin/bit \
        $(CONFIG)/bin/utest.es \
        $(CONFIG)/bin/utest \
        $(CONFIG)/lib/bits \
        $(CONFIG)/lib/ejs.unix.mod \
        $(CONFIG)/bin/jem.es \
        $(CONFIG)/bin/jem \
        $(CONFIG)/lib/ejs.db.mod \
        $(CONFIG)/lib/ejs.db.mapper.mod \
        $(CONFIG)/lib/ejs.db.sqlite.mod \
        $(CONFIG)/lib/ejs.db.sqlite.dylib \
        $(CONFIG)/lib/ejs.web.mod \
        $(CONFIG)/lib/ejs.web.dylib \
        $(CONFIG)/lib/www \
        $(CONFIG)/lib/ejs.template.mod \
        $(CONFIG)/lib/ejs.tar.mod \
        $(CONFIG)/bin/mvc.es \
        $(CONFIG)/bin/mvc \
        $(CONFIG)/lib/ejs.mvc.mod \
        $(CONFIG)/bin/utest.worker \
        src/samples/c/composite/composite.mod \
        src/samples/c/composite/composite.dylib \
        src/samples/c/evalFile/main \
        src/samples/c/evalModule/main \
        src/samples/c/evalModule/evalModule.mod \
        src/samples/c/evalScript/main \
        src/samples/c/nclass/nclass.mod \
        src/samples/c/nclass/native.dylib

.PHONY: prep

prep:
	@[ ! -x $(CONFIG)/inc ] && mkdir -p $(CONFIG)/inc $(CONFIG)/obj $(CONFIG)/lib $(CONFIG)/bin ; true
	@[ ! -f $(CONFIG)/inc/buildConfig.h ] && cp projects/buildConfig.$(CONFIG) $(CONFIG)/inc/buildConfig.h ; true
	@if ! diff $(CONFIG)/inc/buildConfig.h projects/buildConfig.$(CONFIG) >/dev/null ; then\
		echo cp projects/buildConfig.$(CONFIG) $(CONFIG)/inc/buildConfig.h  ; \
		cp projects/buildConfig.$(CONFIG) $(CONFIG)/inc/buildConfig.h  ; \
	fi; true

clean:
	rm -rf $(CONFIG)/lib/libmpr.dylib
	rm -rf $(CONFIG)/lib/libmprssl.dylib
	rm -rf $(CONFIG)/bin/ejsman
	rm -rf $(CONFIG)/bin/makerom
	rm -rf $(CONFIG)/lib/libpcre.dylib
	rm -rf $(CONFIG)/lib/libhttp.dylib
	rm -rf $(CONFIG)/bin/http
	rm -rf $(CONFIG)/lib/libsqlite3.dylib
	rm -rf $(CONFIG)/bin/sqlite
	rm -rf $(CONFIG)/lib/libejs.dylib
	rm -rf $(CONFIG)/bin/ejs
	rm -rf $(CONFIG)/bin/ejsc
	rm -rf $(CONFIG)/bin/ejsmod
	rm -rf $(CONFIG)/bin/ejsrun
	rm -rf $(CONFIG)/bin/utest.es
	rm -rf $(CONFIG)/bin/utest
	rm -rf $(CONFIG)/bin/jem.es
	rm -rf $(CONFIG)/bin/jem
	rm -rf $(CONFIG)/lib/ejs.db.mod
	rm -rf $(CONFIG)/lib/ejs.db.mapper.mod
	rm -rf $(CONFIG)/lib/ejs.db.sqlite.mod
	rm -rf $(CONFIG)/lib/ejs.db.sqlite.dylib
	rm -rf $(CONFIG)/lib/ejs.web.mod
	rm -rf $(CONFIG)/lib/ejs.web.dylib
	rm -rf $(CONFIG)/lib/www
	rm -rf $(CONFIG)/lib/ejs.template.mod
	rm -rf $(CONFIG)/lib/ejs.zlib.dylib
	rm -rf $(CONFIG)/bin/mvc.es
	rm -rf $(CONFIG)/bin/mvc
	rm -rf $(CONFIG)/lib/ejs.mvc.mod
	rm -rf $(CONFIG)/bin/utest.worker
	rm -rf src/samples/c/composite/composite.mod
	rm -rf src/samples/c/composite/composite.dylib
	rm -rf src/samples/c/evalFile/main
	rm -rf src/samples/c/evalModule/main
	rm -rf src/samples/c/evalModule/evalModule.mod
	rm -rf src/samples/c/evalScript/main
	rm -rf src/samples/c/nclass/nclass.mod
	rm -rf src/samples/c/nclass/native.dylib
	rm -rf $(CONFIG)/obj/mprLib.o
	rm -rf $(CONFIG)/obj/mprSsl.o
	rm -rf $(CONFIG)/obj/manager.o
	rm -rf $(CONFIG)/obj/makerom.o
	rm -rf $(CONFIG)/obj/pcre.o
	rm -rf $(CONFIG)/obj/httpLib.o
	rm -rf $(CONFIG)/obj/http.o
	rm -rf $(CONFIG)/obj/sqlite3.o
	rm -rf $(CONFIG)/obj/sqlite.o
	rm -rf $(CONFIG)/obj/ecAst.o
	rm -rf $(CONFIG)/obj/ecCodeGen.o
	rm -rf $(CONFIG)/obj/ecCompiler.o
	rm -rf $(CONFIG)/obj/ecLex.o
	rm -rf $(CONFIG)/obj/ecModuleWrite.o
	rm -rf $(CONFIG)/obj/ecParser.o
	rm -rf $(CONFIG)/obj/ecState.o
	rm -rf $(CONFIG)/obj/ejsApp.o
	rm -rf $(CONFIG)/obj/ejsArray.o
	rm -rf $(CONFIG)/obj/ejsBlock.o
	rm -rf $(CONFIG)/obj/ejsBoolean.o
	rm -rf $(CONFIG)/obj/ejsByteArray.o
	rm -rf $(CONFIG)/obj/ejsCache.o
	rm -rf $(CONFIG)/obj/ejsCmd.o
	rm -rf $(CONFIG)/obj/ejsConfig.o
	rm -rf $(CONFIG)/obj/ejsDate.o
	rm -rf $(CONFIG)/obj/ejsDebug.o
	rm -rf $(CONFIG)/obj/ejsError.o
	rm -rf $(CONFIG)/obj/ejsFile.o
	rm -rf $(CONFIG)/obj/ejsFileSystem.o
	rm -rf $(CONFIG)/obj/ejsFrame.o
	rm -rf $(CONFIG)/obj/ejsFunction.o
	rm -rf $(CONFIG)/obj/ejsGC.o
	rm -rf $(CONFIG)/obj/ejsGlobal.o
	rm -rf $(CONFIG)/obj/ejsHttp.o
	rm -rf $(CONFIG)/obj/ejsIterator.o
	rm -rf $(CONFIG)/obj/ejsJSON.o
	rm -rf $(CONFIG)/obj/ejsLocalCache.o
	rm -rf $(CONFIG)/obj/ejsMath.o
	rm -rf $(CONFIG)/obj/ejsMemory.o
	rm -rf $(CONFIG)/obj/ejsMprLog.o
	rm -rf $(CONFIG)/obj/ejsNamespace.o
	rm -rf $(CONFIG)/obj/ejsNull.o
	rm -rf $(CONFIG)/obj/ejsNumber.o
	rm -rf $(CONFIG)/obj/ejsObject.o
	rm -rf $(CONFIG)/obj/ejsPath.o
	rm -rf $(CONFIG)/obj/ejsPot.o
	rm -rf $(CONFIG)/obj/ejsRegExp.o
	rm -rf $(CONFIG)/obj/ejsSocket.o
	rm -rf $(CONFIG)/obj/ejsString.o
	rm -rf $(CONFIG)/obj/ejsSystem.o
	rm -rf $(CONFIG)/obj/ejsTimer.o
	rm -rf $(CONFIG)/obj/ejsType.o
	rm -rf $(CONFIG)/obj/ejsUri.o
	rm -rf $(CONFIG)/obj/ejsVoid.o
	rm -rf $(CONFIG)/obj/ejsWorker.o
	rm -rf $(CONFIG)/obj/ejsXML.o
	rm -rf $(CONFIG)/obj/ejsXMLList.o
	rm -rf $(CONFIG)/obj/ejsXMLLoader.o
	rm -rf $(CONFIG)/obj/ejsByteCode.o
	rm -rf $(CONFIG)/obj/ejsException.o
	rm -rf $(CONFIG)/obj/ejsHelper.o
	rm -rf $(CONFIG)/obj/ejsInterp.o
	rm -rf $(CONFIG)/obj/ejsLoader.o
	rm -rf $(CONFIG)/obj/ejsModule.o
	rm -rf $(CONFIG)/obj/ejsScope.o
	rm -rf $(CONFIG)/obj/ejsService.o
	rm -rf $(CONFIG)/obj/ejs.o
	rm -rf $(CONFIG)/obj/ejsc.o
	rm -rf $(CONFIG)/obj/ejsmod.o
	rm -rf $(CONFIG)/obj/doc.o
	rm -rf $(CONFIG)/obj/docFiles.o
	rm -rf $(CONFIG)/obj/listing.o
	rm -rf $(CONFIG)/obj/slotGen.o
	rm -rf $(CONFIG)/obj/ejsrun.o
	rm -rf $(CONFIG)/obj/ejsZlib.o
	rm -rf $(CONFIG)/obj/ejsSqlite.o
	rm -rf $(CONFIG)/obj/ejsHttpServer.o
	rm -rf $(CONFIG)/obj/ejsRequest.o
	rm -rf $(CONFIG)/obj/ejsSession.o
	rm -rf $(CONFIG)/obj/ejsWeb.o
	rm -rf $(CONFIG)/obj/shape.o
	rm -rf $(CONFIG)/obj/main.o

clobber: clean
	rm -fr ./$(CONFIG)

$(CONFIG)/inc/mpr.h: 
	rm -fr macosx-x86_64-debug/inc/mpr.h
	cp -r src/deps/mpr/mpr.h macosx-x86_64-debug/inc/mpr.h

$(CONFIG)/inc/mprSsl.h: 
	rm -fr macosx-x86_64-debug/inc/mprSsl.h
	cp -r src/deps/mpr/mprSsl.h macosx-x86_64-debug/inc/mprSsl.h

$(CONFIG)/obj/mprLib.o: \
        src/deps/mpr/mprLib.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/mprLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/deps/mpr/mprLib.c

$(CONFIG)/lib/libmpr.dylib:  \
        $(CONFIG)/inc/mpr.h \
        $(CONFIG)/inc/mprSsl.h \
        $(CONFIG)/obj/mprLib.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/libmpr.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libmpr.dylib $(CONFIG)/obj/mprLib.o $(LIBS)

$(CONFIG)/obj/manager.o: \
        src/deps/mpr/manager.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/manager.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/deps/mpr/manager.c

$(CONFIG)/bin/ejsman:  \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/obj/manager.o
	$(CC) -o $(CONFIG)/bin/ejsman -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/manager.o $(LIBS) -lmpr

$(CONFIG)/obj/makerom.o: \
        src/deps/mpr/makerom.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/makerom.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/deps/mpr/makerom.c

$(CONFIG)/bin/makerom:  \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/obj/makerom.o
	$(CC) -o $(CONFIG)/bin/makerom -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o $(LIBS) -lmpr

$(CONFIG)/inc/pcre.h: 
	rm -fr macosx-x86_64-debug/inc/pcre.h
	cp -r src/deps/pcre/pcre.h macosx-x86_64-debug/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
        src/deps/pcre/pcre.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/pcre.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/deps/pcre/pcre.c

$(CONFIG)/lib/libpcre.dylib:  \
        $(CONFIG)/inc/pcre.h \
        $(CONFIG)/obj/pcre.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/libpcre.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libpcre.dylib $(CONFIG)/obj/pcre.o $(LIBS)

$(CONFIG)/inc/http.h: 
	rm -fr macosx-x86_64-debug/inc/http.h
	cp -r src/deps/http/http.h macosx-x86_64-debug/inc/http.h

$(CONFIG)/obj/httpLib.o: \
        src/deps/http/httpLib.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/http.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/httpLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/deps/http/httpLib.c

$(CONFIG)/lib/libhttp.dylib:  \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/lib/libpcre.dylib \
        $(CONFIG)/inc/http.h \
        $(CONFIG)/obj/httpLib.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/libhttp.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libhttp.dylib $(CONFIG)/obj/httpLib.o $(LIBS) -lmpr -lpcre

$(CONFIG)/obj/http.o: \
        src/deps/http/http.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/http.h
	$(CC) -c -o $(CONFIG)/obj/http.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/deps/http/http.c

$(CONFIG)/bin/http:  \
        $(CONFIG)/lib/libhttp.dylib \
        $(CONFIG)/obj/http.o
	$(CC) -o $(CONFIG)/bin/http -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o $(LIBS) -lhttp -lmpr -lpcre

$(CONFIG)/inc/sqlite3.h: 
	rm -fr macosx-x86_64-debug/inc/sqlite3.h
	cp -r src/deps/sqlite/sqlite3.h macosx-x86_64-debug/inc/sqlite3.h

$(CONFIG)/obj/sqlite3.o: \
        src/deps/sqlite/sqlite3.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/sqlite3.h
	$(CC) -c -o $(CONFIG)/obj/sqlite3.o -arch x86_64 -fPIC -fast $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/deps/sqlite/sqlite3.c

$(CONFIG)/lib/libsqlite3.dylib:  \
        $(CONFIG)/inc/sqlite3.h \
        $(CONFIG)/obj/sqlite3.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/libsqlite3.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libsqlite3.dylib $(CONFIG)/obj/sqlite3.o $(LIBS)

$(CONFIG)/obj/sqlite.o: \
        src/deps/sqlite/sqlite.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/sqlite3.h
	$(CC) -c -o $(CONFIG)/obj/sqlite.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/deps/sqlite/sqlite.c

$(CONFIG)/bin/sqlite:  \
        $(CONFIG)/lib/libsqlite3.dylib \
        $(CONFIG)/obj/sqlite.o
	$(CC) -o $(CONFIG)/bin/sqlite -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/sqlite.o $(LIBS) -lsqlite3

$(CONFIG)/inc/ejs.cache.local.slots.h: 
	rm -fr macosx-x86_64-debug/inc/ejs.cache.local.slots.h
	cp -r src/slots/ejs.cache.local.slots.h macosx-x86_64-debug/inc/ejs.cache.local.slots.h

$(CONFIG)/inc/ejs.db.sqlite.slots.h: 
	rm -fr macosx-x86_64-debug/inc/ejs.db.sqlite.slots.h
	cp -r src/slots/ejs.db.sqlite.slots.h macosx-x86_64-debug/inc/ejs.db.sqlite.slots.h

$(CONFIG)/inc/ejs.slots.h: 
	rm -fr macosx-x86_64-debug/inc/ejs.slots.h
	cp -r src/slots/ejs.slots.h macosx-x86_64-debug/inc/ejs.slots.h

$(CONFIG)/inc/ejs.web.slots.h: 
	rm -fr macosx-x86_64-debug/inc/ejs.web.slots.h
	cp -r src/slots/ejs.web.slots.h macosx-x86_64-debug/inc/ejs.web.slots.h

$(CONFIG)/inc/ejs.zlib.slots.h: 
	rm -fr macosx-x86_64-debug/inc/ejs.zlib.slots.h
	cp -r src/slots/ejs.zlib.slots.h macosx-x86_64-debug/inc/ejs.zlib.slots.h

$(CONFIG)/inc/ejs.h: 
	rm -fr macosx-x86_64-debug/inc/ejs.h
	cp -r src/ejs.h macosx-x86_64-debug/inc/ejs.h

$(CONFIG)/inc/ejsByteCode.h: 
	rm -fr macosx-x86_64-debug/inc/ejsByteCode.h
	cp -r src/ejsByteCode.h macosx-x86_64-debug/inc/ejsByteCode.h

$(CONFIG)/inc/ejsByteCodeTable.h: 
	rm -fr macosx-x86_64-debug/inc/ejsByteCodeTable.h
	cp -r src/ejsByteCodeTable.h macosx-x86_64-debug/inc/ejsByteCodeTable.h

$(CONFIG)/inc/ejsCompiler.h: 
	rm -fr macosx-x86_64-debug/inc/ejsCompiler.h
	cp -r src/ejsCompiler.h macosx-x86_64-debug/inc/ejsCompiler.h

$(CONFIG)/inc/ejsCustomize.h: 
	rm -fr macosx-x86_64-debug/inc/ejsCustomize.h
	cp -r src/ejsCustomize.h macosx-x86_64-debug/inc/ejsCustomize.h

$(CONFIG)/obj/ecAst.o: \
        src/compiler/ecAst.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecAst.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/compiler/ecAst.c

$(CONFIG)/obj/ecCodeGen.o: \
        src/compiler/ecCodeGen.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecCodeGen.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/compiler/ecCodeGen.c

$(CONFIG)/obj/ecCompiler.o: \
        src/compiler/ecCompiler.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecCompiler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/compiler/ecCompiler.c

$(CONFIG)/obj/ecLex.o: \
        src/compiler/ecLex.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecLex.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/compiler/ecLex.c

$(CONFIG)/obj/ecModuleWrite.o: \
        src/compiler/ecModuleWrite.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecModuleWrite.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/compiler/ecModuleWrite.c

$(CONFIG)/obj/ecParser.o: \
        src/compiler/ecParser.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecParser.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/compiler/ecParser.c

$(CONFIG)/obj/ecState.o: \
        src/compiler/ecState.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecState.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/compiler/ecState.c

$(CONFIG)/obj/ejsApp.o: \
        src/core/src/ejsApp.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsApp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsApp.c

$(CONFIG)/obj/ejsArray.o: \
        src/core/src/ejsArray.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsArray.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsArray.c

$(CONFIG)/obj/ejsBlock.o: \
        src/core/src/ejsBlock.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsBlock.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsBlock.c

$(CONFIG)/obj/ejsBoolean.o: \
        src/core/src/ejsBoolean.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsBoolean.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsBoolean.c

$(CONFIG)/obj/ejsByteArray.o: \
        src/core/src/ejsByteArray.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsByteArray.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsByteArray.c

$(CONFIG)/obj/ejsCache.o: \
        src/core/src/ejsCache.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsCache.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsCache.c

$(CONFIG)/obj/ejsCmd.o: \
        src/core/src/ejsCmd.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsCmd.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsCmd.c

$(CONFIG)/obj/ejsConfig.o: \
        src/core/src/ejsConfig.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsConfig.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsConfig.c

$(CONFIG)/obj/ejsDate.o: \
        src/core/src/ejsDate.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsDate.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsDate.c

$(CONFIG)/obj/ejsDebug.o: \
        src/core/src/ejsDebug.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsDebug.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsDebug.c

$(CONFIG)/obj/ejsError.o: \
        src/core/src/ejsError.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsError.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsError.c

$(CONFIG)/obj/ejsFile.o: \
        src/core/src/ejsFile.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFile.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsFile.c

$(CONFIG)/obj/ejsFileSystem.o: \
        src/core/src/ejsFileSystem.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFileSystem.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsFileSystem.c

$(CONFIG)/obj/ejsFrame.o: \
        src/core/src/ejsFrame.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFrame.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsFrame.c

$(CONFIG)/obj/ejsFunction.o: \
        src/core/src/ejsFunction.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFunction.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsFunction.c

$(CONFIG)/obj/ejsGC.o: \
        src/core/src/ejsGC.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsGC.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsGC.c

$(CONFIG)/obj/ejsGlobal.o: \
        src/core/src/ejsGlobal.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsGlobal.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsGlobal.c

$(CONFIG)/obj/ejsHttp.o: \
        src/core/src/ejsHttp.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsHttp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsHttp.c

$(CONFIG)/obj/ejsIterator.o: \
        src/core/src/ejsIterator.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsIterator.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsIterator.c

$(CONFIG)/obj/ejsJSON.o: \
        src/core/src/ejsJSON.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsJSON.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsJSON.c

$(CONFIG)/obj/ejsLocalCache.o: \
        src/core/src/ejsLocalCache.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsLocalCache.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsLocalCache.c

$(CONFIG)/obj/ejsMath.o: \
        src/core/src/ejsMath.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsMath.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsMath.c

$(CONFIG)/obj/ejsMemory.o: \
        src/core/src/ejsMemory.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsMemory.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsMemory.c

$(CONFIG)/obj/ejsMprLog.o: \
        src/core/src/ejsMprLog.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsMprLog.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsMprLog.c

$(CONFIG)/obj/ejsNamespace.o: \
        src/core/src/ejsNamespace.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsNamespace.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsNamespace.c

$(CONFIG)/obj/ejsNull.o: \
        src/core/src/ejsNull.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsNull.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsNull.c

$(CONFIG)/obj/ejsNumber.o: \
        src/core/src/ejsNumber.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsNumber.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsNumber.c

$(CONFIG)/obj/ejsObject.o: \
        src/core/src/ejsObject.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsObject.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsObject.c

$(CONFIG)/obj/ejsPath.o: \
        src/core/src/ejsPath.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/ejsPath.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsPath.c

$(CONFIG)/obj/ejsPot.o: \
        src/core/src/ejsPot.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsPot.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsPot.c

$(CONFIG)/obj/ejsRegExp.o: \
        src/core/src/ejsRegExp.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/ejsRegExp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsRegExp.c

$(CONFIG)/obj/ejsSocket.o: \
        src/core/src/ejsSocket.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsSocket.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsSocket.c

$(CONFIG)/obj/ejsString.o: \
        src/core/src/ejsString.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/ejsString.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsString.c

$(CONFIG)/obj/ejsSystem.o: \
        src/core/src/ejsSystem.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsSystem.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsSystem.c

$(CONFIG)/obj/ejsTimer.o: \
        src/core/src/ejsTimer.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsTimer.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsTimer.c

$(CONFIG)/obj/ejsType.o: \
        src/core/src/ejsType.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsType.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsType.c

$(CONFIG)/obj/ejsUri.o: \
        src/core/src/ejsUri.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsUri.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsUri.c

$(CONFIG)/obj/ejsVoid.o: \
        src/core/src/ejsVoid.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsVoid.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsVoid.c

$(CONFIG)/obj/ejsWorker.o: \
        src/core/src/ejsWorker.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsWorker.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsWorker.c

$(CONFIG)/obj/ejsXML.o: \
        src/core/src/ejsXML.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsXML.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsXML.c

$(CONFIG)/obj/ejsXMLList.o: \
        src/core/src/ejsXMLList.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsXMLList.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsXMLList.c

$(CONFIG)/obj/ejsXMLLoader.o: \
        src/core/src/ejsXMLLoader.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsXMLLoader.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/core/src/ejsXMLLoader.c

$(CONFIG)/obj/ejsByteCode.o: \
        src/vm/ejsByteCode.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsByteCode.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/vm/ejsByteCode.c

$(CONFIG)/obj/ejsException.o: \
        src/vm/ejsException.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsException.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/vm/ejsException.c

$(CONFIG)/obj/ejsHelper.o: \
        src/vm/ejsHelper.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsHelper.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/vm/ejsHelper.c

$(CONFIG)/obj/ejsInterp.o: \
        src/vm/ejsInterp.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsInterp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/vm/ejsInterp.c

$(CONFIG)/obj/ejsLoader.o: \
        src/vm/ejsLoader.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsLoader.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/vm/ejsLoader.c

$(CONFIG)/obj/ejsModule.o: \
        src/vm/ejsModule.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsModule.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/vm/ejsModule.c

$(CONFIG)/obj/ejsScope.o: \
        src/vm/ejsScope.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsScope.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/vm/ejsScope.c

$(CONFIG)/obj/ejsService.o: \
        src/vm/ejsService.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsService.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/vm/ejsService.c

$(CONFIG)/lib/libejs.dylib:  \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/lib/libpcre.dylib \
        $(CONFIG)/lib/libhttp.dylib \
        $(CONFIG)/inc/ejs.cache.local.slots.h \
        $(CONFIG)/inc/ejs.db.sqlite.slots.h \
        $(CONFIG)/inc/ejs.slots.h \
        $(CONFIG)/inc/ejs.web.slots.h \
        $(CONFIG)/inc/ejs.zlib.slots.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/ejsByteCode.h \
        $(CONFIG)/inc/ejsByteCodeTable.h \
        $(CONFIG)/inc/ejsCompiler.h \
        $(CONFIG)/inc/ejsCustomize.h \
        $(CONFIG)/obj/ecAst.o \
        $(CONFIG)/obj/ecCodeGen.o \
        $(CONFIG)/obj/ecCompiler.o \
        $(CONFIG)/obj/ecLex.o \
        $(CONFIG)/obj/ecModuleWrite.o \
        $(CONFIG)/obj/ecParser.o \
        $(CONFIG)/obj/ecState.o \
        $(CONFIG)/obj/ejsApp.o \
        $(CONFIG)/obj/ejsArray.o \
        $(CONFIG)/obj/ejsBlock.o \
        $(CONFIG)/obj/ejsBoolean.o \
        $(CONFIG)/obj/ejsByteArray.o \
        $(CONFIG)/obj/ejsCache.o \
        $(CONFIG)/obj/ejsCmd.o \
        $(CONFIG)/obj/ejsConfig.o \
        $(CONFIG)/obj/ejsDate.o \
        $(CONFIG)/obj/ejsDebug.o \
        $(CONFIG)/obj/ejsError.o \
        $(CONFIG)/obj/ejsFile.o \
        $(CONFIG)/obj/ejsFileSystem.o \
        $(CONFIG)/obj/ejsFrame.o \
        $(CONFIG)/obj/ejsFunction.o \
        $(CONFIG)/obj/ejsGC.o \
        $(CONFIG)/obj/ejsGlobal.o \
        $(CONFIG)/obj/ejsHttp.o \
        $(CONFIG)/obj/ejsIterator.o \
        $(CONFIG)/obj/ejsJSON.o \
        $(CONFIG)/obj/ejsLocalCache.o \
        $(CONFIG)/obj/ejsMath.o \
        $(CONFIG)/obj/ejsMemory.o \
        $(CONFIG)/obj/ejsMprLog.o \
        $(CONFIG)/obj/ejsNamespace.o \
        $(CONFIG)/obj/ejsNull.o \
        $(CONFIG)/obj/ejsNumber.o \
        $(CONFIG)/obj/ejsObject.o \
        $(CONFIG)/obj/ejsPath.o \
        $(CONFIG)/obj/ejsPot.o \
        $(CONFIG)/obj/ejsRegExp.o \
        $(CONFIG)/obj/ejsSocket.o \
        $(CONFIG)/obj/ejsString.o \
        $(CONFIG)/obj/ejsSystem.o \
        $(CONFIG)/obj/ejsTimer.o \
        $(CONFIG)/obj/ejsType.o \
        $(CONFIG)/obj/ejsUri.o \
        $(CONFIG)/obj/ejsVoid.o \
        $(CONFIG)/obj/ejsWorker.o \
        $(CONFIG)/obj/ejsXML.o \
        $(CONFIG)/obj/ejsXMLList.o \
        $(CONFIG)/obj/ejsXMLLoader.o \
        $(CONFIG)/obj/ejsByteCode.o \
        $(CONFIG)/obj/ejsException.o \
        $(CONFIG)/obj/ejsHelper.o \
        $(CONFIG)/obj/ejsInterp.o \
        $(CONFIG)/obj/ejsLoader.o \
        $(CONFIG)/obj/ejsModule.o \
        $(CONFIG)/obj/ejsScope.o \
        $(CONFIG)/obj/ejsService.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/libejs.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libejs.dylib $(CONFIG)/obj/ecAst.o $(CONFIG)/obj/ecCodeGen.o $(CONFIG)/obj/ecCompiler.o $(CONFIG)/obj/ecLex.o $(CONFIG)/obj/ecModuleWrite.o $(CONFIG)/obj/ecParser.o $(CONFIG)/obj/ecState.o $(CONFIG)/obj/ejsApp.o $(CONFIG)/obj/ejsArray.o $(CONFIG)/obj/ejsBlock.o $(CONFIG)/obj/ejsBoolean.o $(CONFIG)/obj/ejsByteArray.o $(CONFIG)/obj/ejsCache.o $(CONFIG)/obj/ejsCmd.o $(CONFIG)/obj/ejsConfig.o $(CONFIG)/obj/ejsDate.o $(CONFIG)/obj/ejsDebug.o $(CONFIG)/obj/ejsError.o $(CONFIG)/obj/ejsFile.o $(CONFIG)/obj/ejsFileSystem.o $(CONFIG)/obj/ejsFrame.o $(CONFIG)/obj/ejsFunction.o $(CONFIG)/obj/ejsGC.o $(CONFIG)/obj/ejsGlobal.o $(CONFIG)/obj/ejsHttp.o $(CONFIG)/obj/ejsIterator.o $(CONFIG)/obj/ejsJSON.o $(CONFIG)/obj/ejsLocalCache.o $(CONFIG)/obj/ejsMath.o $(CONFIG)/obj/ejsMemory.o $(CONFIG)/obj/ejsMprLog.o $(CONFIG)/obj/ejsNamespace.o $(CONFIG)/obj/ejsNull.o $(CONFIG)/obj/ejsNumber.o $(CONFIG)/obj/ejsObject.o $(CONFIG)/obj/ejsPath.o $(CONFIG)/obj/ejsPot.o $(CONFIG)/obj/ejsRegExp.o $(CONFIG)/obj/ejsSocket.o $(CONFIG)/obj/ejsString.o $(CONFIG)/obj/ejsSystem.o $(CONFIG)/obj/ejsTimer.o $(CONFIG)/obj/ejsType.o $(CONFIG)/obj/ejsUri.o $(CONFIG)/obj/ejsVoid.o $(CONFIG)/obj/ejsWorker.o $(CONFIG)/obj/ejsXML.o $(CONFIG)/obj/ejsXMLList.o $(CONFIG)/obj/ejsXMLLoader.o $(CONFIG)/obj/ejsByteCode.o $(CONFIG)/obj/ejsException.o $(CONFIG)/obj/ejsHelper.o $(CONFIG)/obj/ejsInterp.o $(CONFIG)/obj/ejsLoader.o $(CONFIG)/obj/ejsModule.o $(CONFIG)/obj/ejsScope.o $(CONFIG)/obj/ejsService.o $(LIBS) -lmpr -lpcre -lhttp

$(CONFIG)/obj/ejs.o: \
        src/cmd/ejs.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ejs.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/cmd/ejs.c

$(CONFIG)/bin/ejs:  \
        $(CONFIG)/lib/libejs.dylib \
        $(CONFIG)/obj/ejs.o
	$(CC) -o $(CONFIG)/bin/ejs -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejs.o $(LIBS) -lejs -lmpr -lpcre -lhttp -ledit -ledit

$(CONFIG)/obj/ejsc.o: \
        src/cmd/ejsc.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ejsc.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/cmd/ejsc.c

$(CONFIG)/bin/ejsc:  \
        $(CONFIG)/lib/libejs.dylib \
        $(CONFIG)/obj/ejsc.o
	$(CC) -o $(CONFIG)/bin/ejsc -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsc.o $(LIBS) -lejs -lmpr -lpcre -lhttp

$(CONFIG)/obj/ejsmod.o: \
        src/cmd/ejsmod.c \
        $(CONFIG)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/ejsmod.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc -Isrc/cmd src/cmd/ejsmod.c

$(CONFIG)/obj/doc.o: \
        src/cmd/doc.c \
        $(CONFIG)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/doc.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc -Isrc/cmd src/cmd/doc.c

$(CONFIG)/obj/docFiles.o: \
        src/cmd/docFiles.c \
        $(CONFIG)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/docFiles.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc -Isrc/cmd src/cmd/docFiles.c

$(CONFIG)/obj/listing.o: \
        src/cmd/listing.c \
        $(CONFIG)/inc/buildConfig.h \
        src/cmd/ejsmod.h \
        $(CONFIG)/inc/ejsByteCodeTable.h
	$(CC) -c -o $(CONFIG)/obj/listing.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc -Isrc/cmd src/cmd/listing.c

$(CONFIG)/obj/slotGen.o: \
        src/cmd/slotGen.c \
        $(CONFIG)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/slotGen.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc -Isrc/cmd src/cmd/slotGen.c

$(CONFIG)/bin/ejsmod:  \
        $(CONFIG)/lib/libejs.dylib \
        $(CONFIG)/obj/ejsmod.o \
        $(CONFIG)/obj/doc.o \
        $(CONFIG)/obj/docFiles.o \
        $(CONFIG)/obj/listing.o \
        $(CONFIG)/obj/slotGen.o
	$(CC) -o $(CONFIG)/bin/ejsmod -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsmod.o $(CONFIG)/obj/doc.o $(CONFIG)/obj/docFiles.o $(CONFIG)/obj/listing.o $(CONFIG)/obj/slotGen.o $(LIBS) -lejs -lmpr -lpcre -lhttp

$(CONFIG)/obj/ejsrun.o: \
        src/cmd/ejsrun.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ejsrun.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/cmd/ejsrun.c

$(CONFIG)/bin/ejsrun:  \
        $(CONFIG)/lib/libejs.dylib \
        $(CONFIG)/obj/ejsrun.o
	$(CC) -o $(CONFIG)/bin/ejsrun -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LIBS) -lejs -lmpr -lpcre -lhttp

$(CONFIG)/lib/ejs.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod
	ejsc --out $(CONFIG)/lib/ejs.mod --debug --optimize 9 --bind --require null src/core/*.es 
	ejsmod --require null --cslots $(CONFIG)/lib/ejs.mod
	if ! diff ejs.slots.h $(CONFIG)/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h $(CONFIG)/inc; fi
	rm -f ejs.slots.h

$(CONFIG)/bin/bit.es: 
	cp src/jems/ejs.bit/bit.es $(CONFIG)/bin

$(CONFIG)/obj/ejsZlib.o: \
        src/jems/ejs.zlib/src/ejsZlib.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsZlib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/jems/ejs.zlib/src/ejsZlib.c

$(CONFIG)/bin/bit:  \
        $(CONFIG)/lib/libejs.dylib \
        $(CONFIG)/obj/ejsrun.o \
        $(CONFIG)/obj/ejsZlib.o
	$(CC) -o $(CONFIG)/bin/bit -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(CONFIG)/obj/ejsZlib.o $(CONFIG)/obj/mprLib.o $(CONFIG)/obj/pcre.o $(CONFIG)/obj/httpLib.o $(CONFIG)/obj/ecAst.o $(CONFIG)/obj/ecCodeGen.o $(CONFIG)/obj/ecCompiler.o $(CONFIG)/obj/ecLex.o $(CONFIG)/obj/ecModuleWrite.o $(CONFIG)/obj/ecParser.o $(CONFIG)/obj/ecState.o $(CONFIG)/obj/ejsApp.o $(CONFIG)/obj/ejsArray.o $(CONFIG)/obj/ejsBlock.o $(CONFIG)/obj/ejsBoolean.o $(CONFIG)/obj/ejsByteArray.o $(CONFIG)/obj/ejsCache.o $(CONFIG)/obj/ejsCmd.o $(CONFIG)/obj/ejsConfig.o $(CONFIG)/obj/ejsDate.o $(CONFIG)/obj/ejsDebug.o $(CONFIG)/obj/ejsError.o $(CONFIG)/obj/ejsFile.o $(CONFIG)/obj/ejsFileSystem.o $(CONFIG)/obj/ejsFrame.o $(CONFIG)/obj/ejsFunction.o $(CONFIG)/obj/ejsGC.o $(CONFIG)/obj/ejsGlobal.o $(CONFIG)/obj/ejsHttp.o $(CONFIG)/obj/ejsIterator.o $(CONFIG)/obj/ejsJSON.o $(CONFIG)/obj/ejsLocalCache.o $(CONFIG)/obj/ejsMath.o $(CONFIG)/obj/ejsMemory.o $(CONFIG)/obj/ejsMprLog.o $(CONFIG)/obj/ejsNamespace.o $(CONFIG)/obj/ejsNull.o $(CONFIG)/obj/ejsNumber.o $(CONFIG)/obj/ejsObject.o $(CONFIG)/obj/ejsPath.o $(CONFIG)/obj/ejsPot.o $(CONFIG)/obj/ejsRegExp.o $(CONFIG)/obj/ejsSocket.o $(CONFIG)/obj/ejsString.o $(CONFIG)/obj/ejsSystem.o $(CONFIG)/obj/ejsTimer.o $(CONFIG)/obj/ejsType.o $(CONFIG)/obj/ejsUri.o $(CONFIG)/obj/ejsVoid.o $(CONFIG)/obj/ejsWorker.o $(CONFIG)/obj/ejsXML.o $(CONFIG)/obj/ejsXMLList.o $(CONFIG)/obj/ejsXMLLoader.o $(CONFIG)/obj/ejsByteCode.o $(CONFIG)/obj/ejsException.o $(CONFIG)/obj/ejsHelper.o $(CONFIG)/obj/ejsInterp.o $(CONFIG)/obj/ejsLoader.o $(CONFIG)/obj/ejsModule.o $(CONFIG)/obj/ejsScope.o $(CONFIG)/obj/ejsService.o $(LIBS)

$(CONFIG)/bin/utest.es: 
	cp src/jems/ejs.utest/utest.es $(CONFIG)/bin

$(CONFIG)/bin/utest:  \
        $(CONFIG)/bin/ejsrun
	rm -fr macosx-x86_64-debug/bin/utest
	cp -r macosx-x86_64-debug/bin/ejsrun macosx-x86_64-debug/bin/utest

$(CONFIG)/lib/bits: 
	rm -fr $(CONFIG)/lib/bits
	cp -r src/jems/ejs.bit/bits $(CONFIG)/lib

$(CONFIG)/lib/ejs.unix.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/lib/ejs.mod
	ejsc --out $(CONFIG)/lib/ejs.unix.mod --debug --optimize 9 src/jems/ejs.unix/Unix.es

$(CONFIG)/bin/jem.es: 
	cp src/jems/ejs.jem/jem.es $(CONFIG)/bin

$(CONFIG)/bin/jem:  \
        $(CONFIG)/bin/ejsrun
	rm -fr macosx-x86_64-debug/bin/jem
	cp -r macosx-x86_64-debug/bin/ejsrun macosx-x86_64-debug/bin/jem

$(CONFIG)/lib/ejs.db.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/lib/ejs.mod
	ejsc --out $(CONFIG)/lib/ejs.db.mod --debug --optimize 9 src/jems/ejs.db/*.es

$(CONFIG)/lib/ejs.db.mapper.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/lib/ejs.mod \
        $(CONFIG)/lib/ejs.db.mod
	ejsc --out $(CONFIG)/lib/ejs.db.mapper.mod --debug --optimize 9 src/jems/ejs.db.mapper/*.es

$(CONFIG)/lib/ejs.db.sqlite.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/lib/ejs.mod
	ejsc --out $(CONFIG)/lib/ejs.db.sqlite.mod --debug --optimize 9 src/jems/ejs.db.sqlite/*.es

$(CONFIG)/obj/ejsSqlite.o: \
        src/jems/ejs.db.sqlite/src/ejsSqlite.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/sqlite3.h \
        $(CONFIG)/inc/ejs.db.sqlite.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsSqlite.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/jems/ejs.db.sqlite/src/ejsSqlite.c

$(CONFIG)/lib/ejs.db.sqlite.dylib:  \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/lib/libejs.dylib \
        $(CONFIG)/lib/ejs.mod \
        $(CONFIG)/lib/ejs.db.sqlite.mod \
        $(CONFIG)/lib/libsqlite3.dylib \
        $(CONFIG)/obj/ejsSqlite.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/ejs.db.sqlite.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/ejs.db.sqlite.dylib $(CONFIG)/obj/ejsSqlite.o $(LIBS) -lmpr -lejs -lpcre -lhttp -lsqlite3

$(CONFIG)/lib/ejs.web.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/lib/ejs.mod
	ejsc --out $(CONFIG)/lib/ejs.web.mod --debug --optimize 9 src/jems/ejs.web/*.es
	ejsmod --cslots $(CONFIG)/lib/ejs.web.mod
	if ! diff ejs.web.slots.h $(CONFIG)/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h $(CONFIG)/inc; fi
	rm -f ejs.web.slots.h

$(CONFIG)/obj/ejsHttpServer.o: \
        src/jems/ejs.web/src/ejsHttpServer.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/ejsCompiler.h \
        src/jems/ejs.web/src/ejsWeb.h \
        $(CONFIG)/inc/ejs.web.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsHttpServer.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsHttpServer.c

$(CONFIG)/obj/ejsRequest.o: \
        src/jems/ejs.web/src/ejsRequest.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/ejsCompiler.h \
        src/jems/ejs.web/src/ejsWeb.h \
        $(CONFIG)/inc/ejs.web.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsRequest.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsRequest.c

$(CONFIG)/obj/ejsSession.o: \
        src/jems/ejs.web/src/ejsSession.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h \
        src/jems/ejs.web/src/ejsWeb.h
	$(CC) -c -o $(CONFIG)/obj/ejsSession.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsSession.c

$(CONFIG)/obj/ejsWeb.o: \
        src/jems/ejs.web/src/ejsWeb.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/ejsCompiler.h \
        src/jems/ejs.web/src/ejsWeb.h \
        $(CONFIG)/inc/ejs.web.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsWeb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsWeb.c

$(CONFIG)/lib/ejs.web.dylib:  \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/lib/libhttp.dylib \
        $(CONFIG)/lib/libpcre.dylib \
        $(CONFIG)/lib/libejs.dylib \
        $(CONFIG)/lib/ejs.mod \
        $(CONFIG)/obj/ejsHttpServer.o \
        $(CONFIG)/obj/ejsRequest.o \
        $(CONFIG)/obj/ejsSession.o \
        $(CONFIG)/obj/ejsWeb.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/ejs.web.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/ejs.web.dylib $(CONFIG)/obj/ejsHttpServer.o $(CONFIG)/obj/ejsRequest.o $(CONFIG)/obj/ejsSession.o $(CONFIG)/obj/ejsWeb.o $(LIBS) -lmpr -lhttp -lpcre -lpcre -lejs

$(CONFIG)/lib/www: 
	rm -fr $(CONFIG)/lib/www
	cp -r src/jems/ejs.web/www $(CONFIG)/lib

$(CONFIG)/lib/ejs.template.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/lib/ejs.mod
	ejsc --out $(CONFIG)/lib/ejs.template.mod --debug --optimize 9 src/jems/ejs.template/TemplateParser.es

$(CONFIG)/lib/ejs.tar.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/lib/ejs.mod
	ejsc --out $(CONFIG)/lib/ejs.tar.mod --debug --optimize 9 src/jems/ejs.tar/*.es

$(CONFIG)/bin/mvc.es: 
	cp src/jems/ejs.mvc/mvc.es $(CONFIG)/bin

$(CONFIG)/bin/mvc:  \
        $(CONFIG)/bin/ejsrun
	rm -fr macosx-x86_64-debug/bin/mvc
	cp -r macosx-x86_64-debug/bin/ejsrun macosx-x86_64-debug/bin/mvc

$(CONFIG)/lib/ejs.mvc.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/lib/ejs.mod \
        $(CONFIG)/lib/ejs.web.mod \
        $(CONFIG)/lib/ejs.template.mod \
        $(CONFIG)/lib/ejs.unix.mod
	ejsc --out $(CONFIG)/lib/ejs.mvc.mod --debug --optimize 9 src/jems/ejs.mvc/*.es

$(CONFIG)/bin/utest.worker: 
	cp src/jems/ejs.utest/utest.worker $(CONFIG)/bin

$(CONFIG)/obj/shape.o: \
        src/samples/c/nclass/shape.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/shape.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/samples/c/nclass/shape.c

src/samples/c/composite/composite.dylib:  \
        $(CONFIG)/lib/libejs.dylib \
        $(CONFIG)/obj/shape.o
	$(CC) -dynamiclib -o src/samples/c/composite/composite.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/composite.dylib $(CONFIG)/obj/shape.o $(LIBS) -lejs -lmpr -lpcre -lhttp

src/samples/c/composite/composite.mod:  \
        $(CONFIG)/bin/ejsc \
        src/samples/c/composite/composite.dylib
$(CONFIG)/obj/main.o: \
        src/samples/c/evalScript/main.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/main.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -I$(CONFIG)/inc src/samples/c/evalScript/main.c

src/samples/c/evalFile/main:  \
        $(CONFIG)/lib/libejs.dylib \
        $(CONFIG)/obj/main.o
	$(CC) -o src/samples/c/evalFile/main -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/main.o $(LIBS) -lejs -lmpr -lpcre -lhttp

src/samples/c/evalModule/evalModule.mod:  \
        $(CONFIG)/bin/ejsc
src/samples/c/evalModule/main:  \
        $(CONFIG)/lib/libejs.dylib \
        src/samples/c/evalModule/evalModule.mod \
        $(CONFIG)/obj/main.o
	$(CC) -o src/samples/c/evalModule/main -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/main.o $(LIBS) -lejs -lmpr -lpcre -lhttp

src/samples/c/evalScript/main:  \
        $(CONFIG)/lib/libejs.dylib \
        $(CONFIG)/obj/main.o
	$(CC) -o src/samples/c/evalScript/main -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/main.o $(LIBS) -lejs -lmpr -lpcre -lhttp

src/samples/c/nclass/nclass.mod:  \
        $(CONFIG)/bin/ejsc
src/samples/c/nclass/native.dylib:  \
        $(CONFIG)/lib/libejs.dylib \
        src/samples/c/nclass/nclass.mod \
        $(CONFIG)/obj/shape.o
	$(CC) -dynamiclib -o src/samples/c/nclass/native.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/native.dylib $(CONFIG)/obj/shape.o $(LIBS) -lejs -lmpr -lpcre -lhttp

