#
#   build.mk -- Build It Makefile to build Embedthis Ejscript for macosx on i686
#

PLATFORM  := macosx-i686-debug
CC        := cc
CFLAGS    := -fPIC -Wall -g
DFLAGS    := -DPIC -DCPU=I686
IFLAGS    := -I$(PLATFORM)/inc
LDFLAGS   := -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl
LIBS      := -lpthread -lm


all: prep \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/bin/ejsman \
        $(PLATFORM)/bin/makerom \
        $(PLATFORM)/lib/libpcre.dylib \
        $(PLATFORM)/lib/libhttp.dylib \
        $(PLATFORM)/bin/http \
        $(PLATFORM)/lib/libsqlite3.dylib \
        $(PLATFORM)/lib/libejs.dylib \
        $(PLATFORM)/bin/ejs \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/bin/ejsrun \
        $(PLATFORM)/lib/ejs.mod \
        $(PLATFORM)/bin/bit.es \
        $(PLATFORM)/bin/bit \
        $(PLATFORM)/bin/utest.es \
        $(PLATFORM)/bin/utest \
        $(PLATFORM)/lib/bits \
        $(PLATFORM)/lib/ejs.unix.mod \
        $(PLATFORM)/bin/jem.es \
        $(PLATFORM)/bin/jem \
        $(PLATFORM)/lib/ejs.db.mod \
        $(PLATFORM)/lib/ejs.db.mapper.mod \
        $(PLATFORM)/lib/ejs.db.sqlite.mod \
        $(PLATFORM)/lib/ejs.db.sqlite.dylib \
        $(PLATFORM)/lib/ejs.web.mod \
        $(PLATFORM)/lib/ejs.web.dylib \
        $(PLATFORM)/lib/www \
        $(PLATFORM)/lib/ejs.template.mod \
        $(PLATFORM)/lib/ejs.tar.mod \
        $(PLATFORM)/lib/ejs.zlib.mod \
        $(PLATFORM)/lib/ejs.zlib.dylib \
        $(PLATFORM)/bin/mvc.es \
        $(PLATFORM)/bin/mvc \
        $(PLATFORM)/lib/ejs.mvc.mod \
        $(PLATFORM)/bin/utest.worker

.PHONY: prep

prep:
	@[ ! -x $(PLATFORM)/inc ] && mkdir -p $(PLATFORM)/inc $(PLATFORM)/obj $(PLATFORM)/lib $(PLATFORM)/bin ; true
	@[ ! -f $(PLATFORM)/inc/buildConfig.h ] && cp projects/buildConfig.$(PLATFORM) $(PLATFORM)/inc/buildConfig.h ; true

clean:
	rm -rf $(PLATFORM)/lib/libmpr.dylib
	rm -rf $(PLATFORM)/lib/libmprssl.dylib
	rm -rf $(PLATFORM)/bin/ejsman
	rm -rf $(PLATFORM)/bin/makerom
	rm -rf $(PLATFORM)/lib/libpcre.dylib
	rm -rf $(PLATFORM)/lib/libhttp.dylib
	rm -rf $(PLATFORM)/bin/http
	rm -rf $(PLATFORM)/lib/libsqlite3.dylib
	rm -rf $(PLATFORM)/lib/libejs.dylib
	rm -rf $(PLATFORM)/bin/ejs
	rm -rf $(PLATFORM)/bin/ejsc
	rm -rf $(PLATFORM)/bin/ejsmod
	rm -rf $(PLATFORM)/bin/ejsrun
	rm -rf $(PLATFORM)/lib/ejs.mod
	rm -rf $(PLATFORM)/bin/bit.es
	rm -rf $(PLATFORM)/bin/bit
	rm -rf $(PLATFORM)/bin/utest.es
	rm -rf $(PLATFORM)/bin/utest
	rm -rf $(PLATFORM)/lib/bits
	rm -rf $(PLATFORM)/lib/ejs.unix.mod
	rm -rf $(PLATFORM)/bin/jem.es
	rm -rf $(PLATFORM)/bin/jem
	rm -rf $(PLATFORM)/lib/ejs.db.mod
	rm -rf $(PLATFORM)/lib/ejs.db.mapper.mod
	rm -rf $(PLATFORM)/lib/ejs.db.sqlite.mod
	rm -rf $(PLATFORM)/lib/ejs.db.sqlite.dylib
	rm -rf $(PLATFORM)/lib/ejs.web.mod
	rm -rf $(PLATFORM)/lib/ejs.web.dylib
	rm -rf $(PLATFORM)/lib/www
	rm -rf $(PLATFORM)/lib/ejs.template.mod
	rm -rf $(PLATFORM)/lib/ejs.tar.mod
	rm -rf $(PLATFORM)/lib/ejs.zlib.mod
	rm -rf $(PLATFORM)/lib/ejs.zlib.dylib
	rm -rf $(PLATFORM)/bin/mvc.es
	rm -rf $(PLATFORM)/bin/mvc
	rm -rf $(PLATFORM)/lib/ejs.mvc.mod
	rm -rf $(PLATFORM)/bin/utest.worker
	rm -rf $(PLATFORM)/obj/mprLib.o
	rm -rf $(PLATFORM)/obj/mprSsl.o
	rm -rf $(PLATFORM)/obj/manager.o
	rm -rf $(PLATFORM)/obj/makerom.o
	rm -rf $(PLATFORM)/obj/pcre.o
	rm -rf $(PLATFORM)/obj/httpLib.o
	rm -rf $(PLATFORM)/obj/http.o
	rm -rf $(PLATFORM)/obj/sqlite3.o
	rm -rf $(PLATFORM)/obj/ecAst.o
	rm -rf $(PLATFORM)/obj/ecCodeGen.o
	rm -rf $(PLATFORM)/obj/ecCompiler.o
	rm -rf $(PLATFORM)/obj/ecLex.o
	rm -rf $(PLATFORM)/obj/ecModuleWrite.o
	rm -rf $(PLATFORM)/obj/ecParser.o
	rm -rf $(PLATFORM)/obj/ecState.o
	rm -rf $(PLATFORM)/obj/ejsApp.o
	rm -rf $(PLATFORM)/obj/ejsArray.o
	rm -rf $(PLATFORM)/obj/ejsBlock.o
	rm -rf $(PLATFORM)/obj/ejsBoolean.o
	rm -rf $(PLATFORM)/obj/ejsByteArray.o
	rm -rf $(PLATFORM)/obj/ejsCache.o
	rm -rf $(PLATFORM)/obj/ejsCmd.o
	rm -rf $(PLATFORM)/obj/ejsConfig.o
	rm -rf $(PLATFORM)/obj/ejsDate.o
	rm -rf $(PLATFORM)/obj/ejsDebug.o
	rm -rf $(PLATFORM)/obj/ejsError.o
	rm -rf $(PLATFORM)/obj/ejsFile.o
	rm -rf $(PLATFORM)/obj/ejsFileSystem.o
	rm -rf $(PLATFORM)/obj/ejsFrame.o
	rm -rf $(PLATFORM)/obj/ejsFunction.o
	rm -rf $(PLATFORM)/obj/ejsGC.o
	rm -rf $(PLATFORM)/obj/ejsGlobal.o
	rm -rf $(PLATFORM)/obj/ejsHttp.o
	rm -rf $(PLATFORM)/obj/ejsIterator.o
	rm -rf $(PLATFORM)/obj/ejsJSON.o
	rm -rf $(PLATFORM)/obj/ejsLocalCache.o
	rm -rf $(PLATFORM)/obj/ejsMath.o
	rm -rf $(PLATFORM)/obj/ejsMemory.o
	rm -rf $(PLATFORM)/obj/ejsMprLog.o
	rm -rf $(PLATFORM)/obj/ejsNamespace.o
	rm -rf $(PLATFORM)/obj/ejsNull.o
	rm -rf $(PLATFORM)/obj/ejsNumber.o
	rm -rf $(PLATFORM)/obj/ejsObject.o
	rm -rf $(PLATFORM)/obj/ejsPath.o
	rm -rf $(PLATFORM)/obj/ejsPot.o
	rm -rf $(PLATFORM)/obj/ejsRegExp.o
	rm -rf $(PLATFORM)/obj/ejsSocket.o
	rm -rf $(PLATFORM)/obj/ejsString.o
	rm -rf $(PLATFORM)/obj/ejsSystem.o
	rm -rf $(PLATFORM)/obj/ejsTimer.o
	rm -rf $(PLATFORM)/obj/ejsType.o
	rm -rf $(PLATFORM)/obj/ejsUri.o
	rm -rf $(PLATFORM)/obj/ejsVoid.o
	rm -rf $(PLATFORM)/obj/ejsWorker.o
	rm -rf $(PLATFORM)/obj/ejsXML.o
	rm -rf $(PLATFORM)/obj/ejsXMLList.o
	rm -rf $(PLATFORM)/obj/ejsXMLLoader.o
	rm -rf $(PLATFORM)/obj/ejsByteCode.o
	rm -rf $(PLATFORM)/obj/ejsException.o
	rm -rf $(PLATFORM)/obj/ejsHelper.o
	rm -rf $(PLATFORM)/obj/ejsInterp.o
	rm -rf $(PLATFORM)/obj/ejsLoader.o
	rm -rf $(PLATFORM)/obj/ejsModule.o
	rm -rf $(PLATFORM)/obj/ejsScope.o
	rm -rf $(PLATFORM)/obj/ejsService.o
	rm -rf $(PLATFORM)/obj/ejs.o
	rm -rf $(PLATFORM)/obj/ejsc.o
	rm -rf $(PLATFORM)/obj/ejsmod.o
	rm -rf $(PLATFORM)/obj/doc.o
	rm -rf $(PLATFORM)/obj/docFiles.o
	rm -rf $(PLATFORM)/obj/listing.o
	rm -rf $(PLATFORM)/obj/slotGen.o
	rm -rf $(PLATFORM)/obj/ejsrun.o
	rm -rf $(PLATFORM)/obj/ejsSqlite.o
	rm -rf $(PLATFORM)/obj/ejsHttpServer.o
	rm -rf $(PLATFORM)/obj/ejsRequest.o
	rm -rf $(PLATFORM)/obj/ejsSession.o
	rm -rf $(PLATFORM)/obj/ejsWeb.o
	rm -rf $(PLATFORM)/obj/ejsZlib.o

clobber: clean
	rm -fr ./$(PLATFORM)

$(PLATFORM)/inc/mpr.h: 
	rm -fr macosx-i686-debug/inc/mpr.h
	cp -r src/deps/mpr/mpr.h macosx-i686-debug/inc/mpr.h

$(PLATFORM)/inc/mprSsl.h: 
	rm -fr macosx-i686-debug/inc/mprSsl.h
	cp -r src/deps/mpr/mprSsl.h macosx-i686-debug/inc/mprSsl.h

$(PLATFORM)/obj/mprLib.o: \
        src/deps/mpr/mprLib.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/mprLib.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/mpr/mprLib.c

$(PLATFORM)/lib/libmpr.dylib:  \
        $(PLATFORM)/inc/mpr.h \
        $(PLATFORM)/inc/mprSsl.h \
        $(PLATFORM)/obj/mprLib.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libmpr.dylib -arch i686 $(LDFLAGS) -install_name @rpath/libmpr.dylib $(PLATFORM)/obj/mprLib.o $(LIBS)

$(PLATFORM)/obj/manager.o: \
        src/deps/mpr/manager.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/manager.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/mpr/manager.c

$(PLATFORM)/bin/ejsman:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/manager.o
	$(CC) -o $(PLATFORM)/bin/ejsman -arch i686 $(LDFLAGS) -L$(PLATFORM)/lib $(PLATFORM)/obj/manager.o $(LIBS) -lmpr

$(PLATFORM)/obj/makerom.o: \
        src/deps/mpr/makerom.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/makerom.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/mpr/makerom.c

$(PLATFORM)/bin/makerom:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/makerom.o
	$(CC) -o $(PLATFORM)/bin/makerom -arch i686 $(LDFLAGS) -L$(PLATFORM)/lib $(PLATFORM)/obj/makerom.o $(LIBS) -lmpr

$(PLATFORM)/inc/pcre.h: 
	rm -fr macosx-i686-debug/inc/pcre.h
	cp -r src/deps/pcre/pcre.h macosx-i686-debug/inc/pcre.h

$(PLATFORM)/obj/pcre.o: \
        src/deps/pcre/pcre.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/pcre.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/pcre/pcre.c

$(PLATFORM)/lib/libpcre.dylib:  \
        $(PLATFORM)/inc/pcre.h \
        $(PLATFORM)/obj/pcre.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libpcre.dylib -arch i686 $(LDFLAGS) -install_name @rpath/libpcre.dylib $(PLATFORM)/obj/pcre.o $(LIBS)

$(PLATFORM)/inc/http.h: 
	rm -fr macosx-i686-debug/inc/http.h
	cp -r src/deps/http/http.h macosx-i686-debug/inc/http.h

$(PLATFORM)/obj/httpLib.o: \
        src/deps/http/httpLib.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/httpLib.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/http/httpLib.c

$(PLATFORM)/lib/libhttp.dylib:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/lib/libpcre.dylib \
        $(PLATFORM)/inc/http.h \
        $(PLATFORM)/obj/httpLib.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libhttp.dylib -arch i686 $(LDFLAGS) -install_name @rpath/libhttp.dylib $(PLATFORM)/obj/httpLib.o $(LIBS) -lmpr -lpcre

$(PLATFORM)/obj/http.o: \
        src/deps/http/http.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/http.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/http/http.c

$(PLATFORM)/bin/http:  \
        $(PLATFORM)/lib/libhttp.dylib \
        $(PLATFORM)/obj/http.o
	$(CC) -o $(PLATFORM)/bin/http -arch i686 $(LDFLAGS) -L$(PLATFORM)/lib $(PLATFORM)/obj/http.o $(LIBS) -lhttp -lmpr -lpcre

$(PLATFORM)/inc/sqlite3.h: 
	rm -fr macosx-i686-debug/inc/sqlite3.h
	cp -r src/deps/sqlite/sqlite3.h macosx-i686-debug/inc/sqlite3.h

$(PLATFORM)/obj/sqlite3.o: \
        src/deps/sqlite/sqlite3.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/sqlite3.o -arch i686 -fPIC -g $(DFLAGS) -I$(PLATFORM)/inc src/deps/sqlite/sqlite3.c

$(PLATFORM)/lib/libsqlite3.dylib:  \
        $(PLATFORM)/inc/sqlite3.h \
        $(PLATFORM)/obj/sqlite3.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libsqlite3.dylib -arch i686 $(LDFLAGS) -install_name @rpath/libsqlite3.dylib $(PLATFORM)/obj/sqlite3.o $(LIBS)

$(PLATFORM)/inc/ejs.cache.local.slots.h: 
	rm -fr macosx-i686-debug/inc/ejs.cache.local.slots.h
	cp -r src/slots/ejs.cache.local.slots.h macosx-i686-debug/inc/ejs.cache.local.slots.h

$(PLATFORM)/inc/ejs.db.sqlite.slots.h: 
	rm -fr macosx-i686-debug/inc/ejs.db.sqlite.slots.h
	cp -r src/slots/ejs.db.sqlite.slots.h macosx-i686-debug/inc/ejs.db.sqlite.slots.h

$(PLATFORM)/inc/ejs.slots.h: 
	rm -fr macosx-i686-debug/inc/ejs.slots.h
	cp -r src/slots/ejs.slots.h macosx-i686-debug/inc/ejs.slots.h

$(PLATFORM)/inc/ejs.web.slots.h: 
	rm -fr macosx-i686-debug/inc/ejs.web.slots.h
	cp -r src/slots/ejs.web.slots.h macosx-i686-debug/inc/ejs.web.slots.h

$(PLATFORM)/inc/ejs.zlib.slots.h: 
	rm -fr macosx-i686-debug/inc/ejs.zlib.slots.h
	cp -r src/slots/ejs.zlib.slots.h macosx-i686-debug/inc/ejs.zlib.slots.h

$(PLATFORM)/inc/ejs.h: 
	rm -fr macosx-i686-debug/inc/ejs.h
	cp -r src/ejs.h macosx-i686-debug/inc/ejs.h

$(PLATFORM)/inc/ejsByteCode.h: 
	rm -fr macosx-i686-debug/inc/ejsByteCode.h
	cp -r src/ejsByteCode.h macosx-i686-debug/inc/ejsByteCode.h

$(PLATFORM)/inc/ejsByteCodeTable.h: 
	rm -fr macosx-i686-debug/inc/ejsByteCodeTable.h
	cp -r src/ejsByteCodeTable.h macosx-i686-debug/inc/ejsByteCodeTable.h

$(PLATFORM)/inc/ejsCompiler.h: 
	rm -fr macosx-i686-debug/inc/ejsCompiler.h
	cp -r src/ejsCompiler.h macosx-i686-debug/inc/ejsCompiler.h

$(PLATFORM)/inc/ejsCustomize.h: 
	rm -fr macosx-i686-debug/inc/ejsCustomize.h
	cp -r src/ejsCustomize.h macosx-i686-debug/inc/ejsCustomize.h

$(PLATFORM)/obj/ecAst.o: \
        src/compiler/ecAst.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ecAst.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecAst.c

$(PLATFORM)/obj/ecCodeGen.o: \
        src/compiler/ecCodeGen.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ecCodeGen.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecCodeGen.c

$(PLATFORM)/obj/ecCompiler.o: \
        src/compiler/ecCompiler.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ecCompiler.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecCompiler.c

$(PLATFORM)/obj/ecLex.o: \
        src/compiler/ecLex.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ecLex.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecLex.c

$(PLATFORM)/obj/ecModuleWrite.o: \
        src/compiler/ecModuleWrite.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ecModuleWrite.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecModuleWrite.c

$(PLATFORM)/obj/ecParser.o: \
        src/compiler/ecParser.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ecParser.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecParser.c

$(PLATFORM)/obj/ecState.o: \
        src/compiler/ecState.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ecState.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecState.c

$(PLATFORM)/obj/ejsApp.o: \
        src/core/src/ejsApp.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsApp.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsApp.c

$(PLATFORM)/obj/ejsArray.o: \
        src/core/src/ejsArray.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsArray.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsArray.c

$(PLATFORM)/obj/ejsBlock.o: \
        src/core/src/ejsBlock.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsBlock.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsBlock.c

$(PLATFORM)/obj/ejsBoolean.o: \
        src/core/src/ejsBoolean.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsBoolean.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsBoolean.c

$(PLATFORM)/obj/ejsByteArray.o: \
        src/core/src/ejsByteArray.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsByteArray.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsByteArray.c

$(PLATFORM)/obj/ejsCache.o: \
        src/core/src/ejsCache.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsCache.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsCache.c

$(PLATFORM)/obj/ejsCmd.o: \
        src/core/src/ejsCmd.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsCmd.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsCmd.c

$(PLATFORM)/obj/ejsConfig.o: \
        src/core/src/ejsConfig.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsConfig.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsConfig.c

$(PLATFORM)/obj/ejsDate.o: \
        src/core/src/ejsDate.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsDate.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsDate.c

$(PLATFORM)/obj/ejsDebug.o: \
        src/core/src/ejsDebug.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsDebug.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsDebug.c

$(PLATFORM)/obj/ejsError.o: \
        src/core/src/ejsError.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsError.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsError.c

$(PLATFORM)/obj/ejsFile.o: \
        src/core/src/ejsFile.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsFile.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsFile.c

$(PLATFORM)/obj/ejsFileSystem.o: \
        src/core/src/ejsFileSystem.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsFileSystem.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsFileSystem.c

$(PLATFORM)/obj/ejsFrame.o: \
        src/core/src/ejsFrame.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsFrame.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsFrame.c

$(PLATFORM)/obj/ejsFunction.o: \
        src/core/src/ejsFunction.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsFunction.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsFunction.c

$(PLATFORM)/obj/ejsGC.o: \
        src/core/src/ejsGC.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsGC.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsGC.c

$(PLATFORM)/obj/ejsGlobal.o: \
        src/core/src/ejsGlobal.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsGlobal.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsGlobal.c

$(PLATFORM)/obj/ejsHttp.o: \
        src/core/src/ejsHttp.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsHttp.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsHttp.c

$(PLATFORM)/obj/ejsIterator.o: \
        src/core/src/ejsIterator.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsIterator.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsIterator.c

$(PLATFORM)/obj/ejsJSON.o: \
        src/core/src/ejsJSON.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsJSON.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsJSON.c

$(PLATFORM)/obj/ejsLocalCache.o: \
        src/core/src/ejsLocalCache.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsLocalCache.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsLocalCache.c

$(PLATFORM)/obj/ejsMath.o: \
        src/core/src/ejsMath.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsMath.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsMath.c

$(PLATFORM)/obj/ejsMemory.o: \
        src/core/src/ejsMemory.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsMemory.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsMemory.c

$(PLATFORM)/obj/ejsMprLog.o: \
        src/core/src/ejsMprLog.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsMprLog.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsMprLog.c

$(PLATFORM)/obj/ejsNamespace.o: \
        src/core/src/ejsNamespace.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsNamespace.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsNamespace.c

$(PLATFORM)/obj/ejsNull.o: \
        src/core/src/ejsNull.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsNull.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsNull.c

$(PLATFORM)/obj/ejsNumber.o: \
        src/core/src/ejsNumber.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsNumber.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsNumber.c

$(PLATFORM)/obj/ejsObject.o: \
        src/core/src/ejsObject.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsObject.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsObject.c

$(PLATFORM)/obj/ejsPath.o: \
        src/core/src/ejsPath.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsPath.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsPath.c

$(PLATFORM)/obj/ejsPot.o: \
        src/core/src/ejsPot.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsPot.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsPot.c

$(PLATFORM)/obj/ejsRegExp.o: \
        src/core/src/ejsRegExp.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsRegExp.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsRegExp.c

$(PLATFORM)/obj/ejsSocket.o: \
        src/core/src/ejsSocket.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsSocket.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsSocket.c

$(PLATFORM)/obj/ejsString.o: \
        src/core/src/ejsString.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsString.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsString.c

$(PLATFORM)/obj/ejsSystem.o: \
        src/core/src/ejsSystem.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsSystem.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsSystem.c

$(PLATFORM)/obj/ejsTimer.o: \
        src/core/src/ejsTimer.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsTimer.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsTimer.c

$(PLATFORM)/obj/ejsType.o: \
        src/core/src/ejsType.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsType.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsType.c

$(PLATFORM)/obj/ejsUri.o: \
        src/core/src/ejsUri.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsUri.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsUri.c

$(PLATFORM)/obj/ejsVoid.o: \
        src/core/src/ejsVoid.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsVoid.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsVoid.c

$(PLATFORM)/obj/ejsWorker.o: \
        src/core/src/ejsWorker.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsWorker.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsWorker.c

$(PLATFORM)/obj/ejsXML.o: \
        src/core/src/ejsXML.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsXML.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsXML.c

$(PLATFORM)/obj/ejsXMLList.o: \
        src/core/src/ejsXMLList.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsXMLList.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsXMLList.c

$(PLATFORM)/obj/ejsXMLLoader.o: \
        src/core/src/ejsXMLLoader.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsXMLLoader.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsXMLLoader.c

$(PLATFORM)/obj/ejsByteCode.o: \
        src/vm/ejsByteCode.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsByteCode.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsByteCode.c

$(PLATFORM)/obj/ejsException.o: \
        src/vm/ejsException.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsException.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsException.c

$(PLATFORM)/obj/ejsHelper.o: \
        src/vm/ejsHelper.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsHelper.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsHelper.c

$(PLATFORM)/obj/ejsInterp.o: \
        src/vm/ejsInterp.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsInterp.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsInterp.c

$(PLATFORM)/obj/ejsLoader.o: \
        src/vm/ejsLoader.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsLoader.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsLoader.c

$(PLATFORM)/obj/ejsModule.o: \
        src/vm/ejsModule.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsModule.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsModule.c

$(PLATFORM)/obj/ejsScope.o: \
        src/vm/ejsScope.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsScope.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsScope.c

$(PLATFORM)/obj/ejsService.o: \
        src/vm/ejsService.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsService.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsService.c

$(PLATFORM)/lib/libejs.dylib:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/lib/libpcre.dylib \
        $(PLATFORM)/lib/libhttp.dylib \
        $(PLATFORM)/inc/ejs.cache.local.slots.h \
        $(PLATFORM)/inc/ejs.db.sqlite.slots.h \
        $(PLATFORM)/inc/ejs.slots.h \
        $(PLATFORM)/inc/ejs.web.slots.h \
        $(PLATFORM)/inc/ejs.zlib.slots.h \
        $(PLATFORM)/inc/ejs.h \
        $(PLATFORM)/inc/ejsByteCode.h \
        $(PLATFORM)/inc/ejsByteCodeTable.h \
        $(PLATFORM)/inc/ejsCompiler.h \
        $(PLATFORM)/inc/ejsCustomize.h \
        $(PLATFORM)/obj/ecAst.o \
        $(PLATFORM)/obj/ecCodeGen.o \
        $(PLATFORM)/obj/ecCompiler.o \
        $(PLATFORM)/obj/ecLex.o \
        $(PLATFORM)/obj/ecModuleWrite.o \
        $(PLATFORM)/obj/ecParser.o \
        $(PLATFORM)/obj/ecState.o \
        $(PLATFORM)/obj/ejsApp.o \
        $(PLATFORM)/obj/ejsArray.o \
        $(PLATFORM)/obj/ejsBlock.o \
        $(PLATFORM)/obj/ejsBoolean.o \
        $(PLATFORM)/obj/ejsByteArray.o \
        $(PLATFORM)/obj/ejsCache.o \
        $(PLATFORM)/obj/ejsCmd.o \
        $(PLATFORM)/obj/ejsConfig.o \
        $(PLATFORM)/obj/ejsDate.o \
        $(PLATFORM)/obj/ejsDebug.o \
        $(PLATFORM)/obj/ejsError.o \
        $(PLATFORM)/obj/ejsFile.o \
        $(PLATFORM)/obj/ejsFileSystem.o \
        $(PLATFORM)/obj/ejsFrame.o \
        $(PLATFORM)/obj/ejsFunction.o \
        $(PLATFORM)/obj/ejsGC.o \
        $(PLATFORM)/obj/ejsGlobal.o \
        $(PLATFORM)/obj/ejsHttp.o \
        $(PLATFORM)/obj/ejsIterator.o \
        $(PLATFORM)/obj/ejsJSON.o \
        $(PLATFORM)/obj/ejsLocalCache.o \
        $(PLATFORM)/obj/ejsMath.o \
        $(PLATFORM)/obj/ejsMemory.o \
        $(PLATFORM)/obj/ejsMprLog.o \
        $(PLATFORM)/obj/ejsNamespace.o \
        $(PLATFORM)/obj/ejsNull.o \
        $(PLATFORM)/obj/ejsNumber.o \
        $(PLATFORM)/obj/ejsObject.o \
        $(PLATFORM)/obj/ejsPath.o \
        $(PLATFORM)/obj/ejsPot.o \
        $(PLATFORM)/obj/ejsRegExp.o \
        $(PLATFORM)/obj/ejsSocket.o \
        $(PLATFORM)/obj/ejsString.o \
        $(PLATFORM)/obj/ejsSystem.o \
        $(PLATFORM)/obj/ejsTimer.o \
        $(PLATFORM)/obj/ejsType.o \
        $(PLATFORM)/obj/ejsUri.o \
        $(PLATFORM)/obj/ejsVoid.o \
        $(PLATFORM)/obj/ejsWorker.o \
        $(PLATFORM)/obj/ejsXML.o \
        $(PLATFORM)/obj/ejsXMLList.o \
        $(PLATFORM)/obj/ejsXMLLoader.o \
        $(PLATFORM)/obj/ejsByteCode.o \
        $(PLATFORM)/obj/ejsException.o \
        $(PLATFORM)/obj/ejsHelper.o \
        $(PLATFORM)/obj/ejsInterp.o \
        $(PLATFORM)/obj/ejsLoader.o \
        $(PLATFORM)/obj/ejsModule.o \
        $(PLATFORM)/obj/ejsScope.o \
        $(PLATFORM)/obj/ejsService.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libejs.dylib -arch i686 $(LDFLAGS) -install_name @rpath/libejs.dylib $(PLATFORM)/obj/ecAst.o $(PLATFORM)/obj/ecCodeGen.o $(PLATFORM)/obj/ecCompiler.o $(PLATFORM)/obj/ecLex.o $(PLATFORM)/obj/ecModuleWrite.o $(PLATFORM)/obj/ecParser.o $(PLATFORM)/obj/ecState.o $(PLATFORM)/obj/ejsApp.o $(PLATFORM)/obj/ejsArray.o $(PLATFORM)/obj/ejsBlock.o $(PLATFORM)/obj/ejsBoolean.o $(PLATFORM)/obj/ejsByteArray.o $(PLATFORM)/obj/ejsCache.o $(PLATFORM)/obj/ejsCmd.o $(PLATFORM)/obj/ejsConfig.o $(PLATFORM)/obj/ejsDate.o $(PLATFORM)/obj/ejsDebug.o $(PLATFORM)/obj/ejsError.o $(PLATFORM)/obj/ejsFile.o $(PLATFORM)/obj/ejsFileSystem.o $(PLATFORM)/obj/ejsFrame.o $(PLATFORM)/obj/ejsFunction.o $(PLATFORM)/obj/ejsGC.o $(PLATFORM)/obj/ejsGlobal.o $(PLATFORM)/obj/ejsHttp.o $(PLATFORM)/obj/ejsIterator.o $(PLATFORM)/obj/ejsJSON.o $(PLATFORM)/obj/ejsLocalCache.o $(PLATFORM)/obj/ejsMath.o $(PLATFORM)/obj/ejsMemory.o $(PLATFORM)/obj/ejsMprLog.o $(PLATFORM)/obj/ejsNamespace.o $(PLATFORM)/obj/ejsNull.o $(PLATFORM)/obj/ejsNumber.o $(PLATFORM)/obj/ejsObject.o $(PLATFORM)/obj/ejsPath.o $(PLATFORM)/obj/ejsPot.o $(PLATFORM)/obj/ejsRegExp.o $(PLATFORM)/obj/ejsSocket.o $(PLATFORM)/obj/ejsString.o $(PLATFORM)/obj/ejsSystem.o $(PLATFORM)/obj/ejsTimer.o $(PLATFORM)/obj/ejsType.o $(PLATFORM)/obj/ejsUri.o $(PLATFORM)/obj/ejsVoid.o $(PLATFORM)/obj/ejsWorker.o $(PLATFORM)/obj/ejsXML.o $(PLATFORM)/obj/ejsXMLList.o $(PLATFORM)/obj/ejsXMLLoader.o $(PLATFORM)/obj/ejsByteCode.o $(PLATFORM)/obj/ejsException.o $(PLATFORM)/obj/ejsHelper.o $(PLATFORM)/obj/ejsInterp.o $(PLATFORM)/obj/ejsLoader.o $(PLATFORM)/obj/ejsModule.o $(PLATFORM)/obj/ejsScope.o $(PLATFORM)/obj/ejsService.o $(LIBS) -lmpr -lpcre -lhttp

$(PLATFORM)/obj/ejs.o: \
        src/cmd/ejs.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejs.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/cmd/ejs.c

$(PLATFORM)/bin/ejs:  \
        $(PLATFORM)/lib/libejs.dylib \
        $(PLATFORM)/obj/ejs.o
	$(CC) -o $(PLATFORM)/bin/ejs -arch i686 $(LDFLAGS) -L$(PLATFORM)/lib $(PLATFORM)/obj/ejs.o $(LIBS) -lejs -lmpr -lpcre -lhttp

$(PLATFORM)/obj/ejsc.o: \
        src/cmd/ejsc.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsc.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/cmd/ejsc.c

$(PLATFORM)/bin/ejsc:  \
        $(PLATFORM)/lib/libejs.dylib \
        $(PLATFORM)/obj/ejsc.o
	$(CC) -o $(PLATFORM)/bin/ejsc -arch i686 $(LDFLAGS) -L$(PLATFORM)/lib $(PLATFORM)/obj/ejsc.o $(LIBS) -lejs -lmpr -lpcre -lhttp

$(PLATFORM)/obj/ejsmod.o: \
        src/cmd/ejsmod.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(PLATFORM)/obj/ejsmod.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/cmd src/cmd/ejsmod.c

$(PLATFORM)/obj/doc.o: \
        src/cmd/doc.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(PLATFORM)/obj/doc.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/cmd src/cmd/doc.c

$(PLATFORM)/obj/docFiles.o: \
        src/cmd/docFiles.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(PLATFORM)/obj/docFiles.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/cmd src/cmd/docFiles.c

$(PLATFORM)/obj/listing.o: \
        src/cmd/listing.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(PLATFORM)/obj/listing.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/cmd src/cmd/listing.c

$(PLATFORM)/obj/slotGen.o: \
        src/cmd/slotGen.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(PLATFORM)/obj/slotGen.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/cmd src/cmd/slotGen.c

$(PLATFORM)/bin/ejsmod:  \
        $(PLATFORM)/lib/libejs.dylib \
        $(PLATFORM)/obj/ejsmod.o \
        $(PLATFORM)/obj/doc.o \
        $(PLATFORM)/obj/docFiles.o \
        $(PLATFORM)/obj/listing.o \
        $(PLATFORM)/obj/slotGen.o
	$(CC) -o $(PLATFORM)/bin/ejsmod -arch i686 $(LDFLAGS) -L$(PLATFORM)/lib $(PLATFORM)/obj/ejsmod.o $(PLATFORM)/obj/doc.o $(PLATFORM)/obj/docFiles.o $(PLATFORM)/obj/listing.o $(PLATFORM)/obj/slotGen.o $(LIBS) -lejs -lmpr -lpcre -lhttp

$(PLATFORM)/obj/ejsrun.o: \
        src/cmd/ejsrun.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsrun.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/cmd/ejsrun.c

$(PLATFORM)/bin/ejsrun:  \
        $(PLATFORM)/lib/libejs.dylib \
        $(PLATFORM)/obj/ejsrun.o
	$(CC) -o $(PLATFORM)/bin/ejsrun -arch i686 $(LDFLAGS) -L$(PLATFORM)/lib $(PLATFORM)/obj/ejsrun.o $(LIBS) -lejs -lmpr -lpcre -lhttp

$(PLATFORM)/lib/ejs.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod
	ejsc --out $(PLATFORM)/lib/ejs.mod --debug --optimize 9 --bind --require null src/core/*.es 
	ejsmod --require null --cslots $(PLATFORM)/lib/ejs.mod
	if ! diff ejs.slots.h $(PLATFORM)/inc/ejs.slots.h >/dev/null; then mv ejs.slots.h $(PLATFORM)/inc; fi

$(PLATFORM)/bin/bit.es: 
	cp src/jems/ejs.bit/bit.es $(PLATFORM)/bin

$(PLATFORM)/bin/bit:  \
        $(PLATFORM)/bin/ejsrun
	rm -fr macosx-i686-debug/bin/bit
	cp -r macosx-i686-debug/bin/ejsrun macosx-i686-debug/bin/bit

$(PLATFORM)/bin/utest.es: 
	cp src/jems/ejs.utest/utest.es $(PLATFORM)/bin

$(PLATFORM)/bin/utest:  \
        $(PLATFORM)/bin/ejsrun
	rm -fr macosx-i686-debug/bin/utest
	cp -r macosx-i686-debug/bin/ejsrun macosx-i686-debug/bin/utest

$(PLATFORM)/lib/bits: 
	rm -fr $(PLATFORM)/lib/bits
	cp -r src/jems/ejs.bit/bits $(PLATFORM)/lib

$(PLATFORM)/lib/ejs.unix.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out $(PLATFORM)/lib/ejs.unix.mod --debug --optimize 9 src/jems/ejs.unix/Unix.es

$(PLATFORM)/bin/jem.es: 
	cp src/jems/ejs.jem/jem.es $(PLATFORM)/bin

$(PLATFORM)/bin/jem:  \
        $(PLATFORM)/bin/ejsrun
	rm -fr macosx-i686-debug/bin/jem
	cp -r macosx-i686-debug/bin/ejsrun macosx-i686-debug/bin/jem

$(PLATFORM)/lib/ejs.db.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out $(PLATFORM)/lib/ejs.db.mod --debug --optimize 9 src/jems/ejs.db/*.es

$(PLATFORM)/lib/ejs.db.mapper.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod \
        $(PLATFORM)/lib/ejs.db.mod
	ejsc --out $(PLATFORM)/lib/ejs.db.mapper.mod --debug --optimize 9 src/jems/ejs.db.mapper/*.es

$(PLATFORM)/lib/ejs.db.sqlite.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out $(PLATFORM)/lib/ejs.db.sqlite.mod --debug --optimize 9 src/jems/ejs.db.sqlite/*.es

$(PLATFORM)/obj/ejsSqlite.o: \
        src/jems/ejs.db.sqlite/src/ejsSqlite.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsSqlite.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/jems/ejs.db.sqlite/src/ejsSqlite.c

$(PLATFORM)/lib/ejs.db.sqlite.dylib:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/lib/libejs.dylib \
        $(PLATFORM)/lib/ejs.mod \
        $(PLATFORM)/lib/ejs.db.sqlite.mod \
        $(PLATFORM)/lib/libsqlite3.dylib \
        $(PLATFORM)/obj/ejsSqlite.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/ejs.db.sqlite.dylib -arch i686 $(LDFLAGS) -install_name @rpath/ejs.db.sqlite.dylib $(PLATFORM)/obj/ejsSqlite.o $(LIBS) -lmpr -lejs -lpcre -lhttp -lsqlite3

$(PLATFORM)/lib/ejs.web.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out $(PLATFORM)/lib/ejs.web.mod --debug --optimize 9 src/jems/ejs.web/*.es
	ejsmod --cslots $(PLATFORM)/lib/ejs.web.mod
	if ! diff ejs.web.slots.h $(PLATFORM)/inc/ejs.web.slots.h >/dev/null; then mv ejs.web.slots.h $(PLATFORM)/inc; fi

$(PLATFORM)/obj/ejsHttpServer.o: \
        src/jems/ejs.web/src/ejsHttpServer.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/jems/ejs.web/src/ejsWeb.h
	$(CC) -c -o $(PLATFORM)/obj/ejsHttpServer.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsHttpServer.c

$(PLATFORM)/obj/ejsRequest.o: \
        src/jems/ejs.web/src/ejsRequest.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/jems/ejs.web/src/ejsWeb.h
	$(CC) -c -o $(PLATFORM)/obj/ejsRequest.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsRequest.c

$(PLATFORM)/obj/ejsSession.o: \
        src/jems/ejs.web/src/ejsSession.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/jems/ejs.web/src/ejsWeb.h
	$(CC) -c -o $(PLATFORM)/obj/ejsSession.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsSession.c

$(PLATFORM)/obj/ejsWeb.o: \
        src/jems/ejs.web/src/ejsWeb.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/jems/ejs.web/src/ejsWeb.h
	$(CC) -c -o $(PLATFORM)/obj/ejsWeb.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsWeb.c

$(PLATFORM)/lib/ejs.web.dylib:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/lib/libhttp.dylib \
        $(PLATFORM)/lib/libpcre.dylib \
        $(PLATFORM)/lib/libejs.dylib \
        $(PLATFORM)/lib/ejs.mod \
        $(PLATFORM)/obj/ejsHttpServer.o \
        $(PLATFORM)/obj/ejsRequest.o \
        $(PLATFORM)/obj/ejsSession.o \
        $(PLATFORM)/obj/ejsWeb.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/ejs.web.dylib -arch i686 $(LDFLAGS) -install_name @rpath/ejs.web.dylib $(PLATFORM)/obj/ejsHttpServer.o $(PLATFORM)/obj/ejsRequest.o $(PLATFORM)/obj/ejsSession.o $(PLATFORM)/obj/ejsWeb.o $(LIBS) -lmpr -lhttp -lpcre -lpcre -lejs

$(PLATFORM)/lib/www: 
	rm -fr $(PLATFORM)/lib/www
	cp -r src/jems/ejs.web/www $(PLATFORM)/lib

$(PLATFORM)/lib/ejs.template.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out $(PLATFORM)/lib/ejs.template.mod --debug --optimize 9 src/jems/ejs.template/TemplateParser.es

$(PLATFORM)/lib/ejs.tar.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out $(PLATFORM)/lib/ejs.tar.mod/ --debug --optimize 9 src/jems/ejs.tar/*.es

$(PLATFORM)/lib/ejs.zlib.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out $(PLATFORM)/lib/ejs.zlib.mod/ --debug --optimize 9 src/jems/ejs.zlib/*.es

$(PLATFORM)/obj/ejsZlib.o: \
        src/jems/ejs.zlib/src/ejsZlib.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/ejsZlib.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -I../../../../usr/include src/jems/ejs.zlib/src/ejsZlib.c

$(PLATFORM)/lib/ejs.zlib.dylib:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/lib/libejs.dylib \
        $(PLATFORM)/lib/ejs.mod \
        $(PLATFORM)/lib/ejs.zlib.mod \
        $(PLATFORM)/obj/ejsZlib.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/ejs.zlib.dylib -arch i686 $(LDFLAGS) -L/usr/lib -install_name @rpath/ejs.zlib.dylib $(PLATFORM)/obj/ejsZlib.o $(LIBS) -lmpr -lejs -lpcre -lhttp -lz

$(PLATFORM)/bin/mvc.es: 
	cp src/jems/ejs.mvc/mvc.es $(PLATFORM)/bin

$(PLATFORM)/bin/mvc:  \
        $(PLATFORM)/bin/ejsrun
	rm -fr macosx-i686-debug/bin/mvc
	cp -r macosx-i686-debug/bin/ejsrun macosx-i686-debug/bin/mvc

$(PLATFORM)/lib/ejs.mvc.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod \
        $(PLATFORM)/lib/ejs.web.mod \
        $(PLATFORM)/lib/ejs.template.mod \
        $(PLATFORM)/lib/ejs.unix.mod
	ejsc --out $(PLATFORM)/lib/ejs.mvc.mod/ --debug --optimize 9 src/jems/ejs.mvc/*.es

$(PLATFORM)/bin/utest.worker: 
	cp src/jems/ejs.utest/utest.worker $(PLATFORM)/bin

