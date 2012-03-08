#
#   build.mk -- Build It Makefile to build Embedthis Ejscript for solaris on i686
#

PLATFORM  := solaris-i686-debug
CC        := cc
CFLAGS    := -fPIC -g -mcpu=i686
DFLAGS    := -DPIC
IFLAGS    := -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots
LDFLAGS   := -L/Users/mob/git/ejs/$(PLATFORM)/lib -g
LIBS      := -lpthread -lm

all: prep \
        $(PLATFORM)/lib/libmpr.so \
        $(PLATFORM)/bin/manager \
        $(PLATFORM)/bin/makerom \
        $(PLATFORM)/lib/libpcre.so \
        $(PLATFORM)/lib/libhttp.so \
        $(PLATFORM)/bin/http \
        $(PLATFORM)/lib/libsqlite3.so \
        $(PLATFORM)/inc/ejs.slots.h \
        $(PLATFORM)/lib/libejs.so \
        $(PLATFORM)/bin/ejs \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/bin/ejsrun \
        $(PLATFORM)/lib/ejs.mod \
        $(PLATFORM)/bin/bit.es \
        $(PLATFORM)/bin/bit \
        $(PLATFORM)/lib/bits \
        $(PLATFORM)/lib/ejs.unix.mod \
        $(PLATFORM)/bin/jem.es \
        $(PLATFORM)/bin/jem \
        $(PLATFORM)/lib/ejs.db.mod \
        $(PLATFORM)/lib/ejs.db.mapper.mod \
        $(PLATFORM)/lib/ejs.db.sqlite.mod \
        $(PLATFORM)/lib/ejs.db.sqlite.so \
        $(PLATFORM)/lib/ejs.web.mod \
        $(PLATFORM)/lib/ejs.web.so \
        $(PLATFORM)/lib/ejs.template.mod \
        $(PLATFORM)/lib/ejs.tar.mod \
        $(PLATFORM)/bin/mvc.es \
        $(PLATFORM)/bin/mvc \
        $(PLATFORM)/lib/ejs.mvc.mod

.PHONY: prep

prep:
	@if [ ! -x $(PLATFORM)/inc ] ; then \
		mkdir -p $(PLATFORM)/inc $(PLATFORM)/obj $(PLATFORM)/lib $(PLATFORM)/bin ; \
		cp src/buildConfig.default $(PLATFORM)/inc/buildConfig.h ; \
	fi

clean:
	rm -rf $(PLATFORM)/lib/libmpr.so
	rm -rf $(PLATFORM)/lib/libmprssl.so
	rm -rf $(PLATFORM)/bin/manager
	rm -rf $(PLATFORM)/bin/makerom
	rm -rf $(PLATFORM)/lib/libpcre.so
	rm -rf $(PLATFORM)/lib/libhttp.so
	rm -rf $(PLATFORM)/bin/http
	rm -rf $(PLATFORM)/lib/libsqlite3.so
	rm -rf $(PLATFORM)/inc/ejs.slots.h
	rm -rf $(PLATFORM)/lib/libejs.so
	rm -rf $(PLATFORM)/bin/ejs
	rm -rf $(PLATFORM)/bin/ejsc
	rm -rf $(PLATFORM)/bin/ejsmod
	rm -rf $(PLATFORM)/bin/ejsrun
	rm -rf $(PLATFORM)/lib/ejs.mod
	rm -rf $(PLATFORM)/bin/bit.es
	rm -rf $(PLATFORM)/bin/bit
	rm -rf $(PLATFORM)/lib/bits
	rm -rf $(PLATFORM)/lib/ejs.unix.mod
	rm -rf $(PLATFORM)/bin/jem.es
	rm -rf $(PLATFORM)/bin/jem
	rm -rf $(PLATFORM)/lib/ejs.db.mod
	rm -rf $(PLATFORM)/lib/ejs.db.mapper.mod
	rm -rf $(PLATFORM)/lib/ejs.db.sqlite.mod
	rm -rf $(PLATFORM)/lib/ejs.db.sqlite.so
	rm -rf $(PLATFORM)/lib/ejs.web.mod
	rm -rf $(PLATFORM)/lib/ejs.web.so
	rm -rf $(PLATFORM)/lib/ejs.template.mod
	rm -rf $(PLATFORM)/lib/ejs.tar.mod
	rm -rf $(PLATFORM)/lib/ejs.zlib.mod
	rm -rf $(PLATFORM)/lib/ejs.zlib.so
	rm -rf $(PLATFORM)/bin/mvc.es
	rm -rf $(PLATFORM)/bin/mvc
	rm -rf $(PLATFORM)/lib/ejs.mvc.mod
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

$(PLATFORM)/obj/mprLib.o: \
        src/deps/mpr/mprLib.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/mpr/mpr.h
	$(CC) -c -o $(PLATFORM)/obj/mprLib.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/mprLib.c

$(PLATFORM)/lib/libmpr.so:  \
        $(PLATFORM)/obj/mprLib.o
	$(CC) -shared -o $(PLATFORM)/lib/libmpr.so -L$(PLATFORM)/lib -g $(PLATFORM)/obj/mprLib.o $(LIBS)

$(PLATFORM)/obj/manager.o: \
        src/deps/mpr/manager.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/mpr/mpr.h
	$(CC) -c -o $(PLATFORM)/obj/manager.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/manager.c

$(PLATFORM)/bin/manager:  \
        $(PLATFORM)/lib/libmpr.so \
        $(PLATFORM)/obj/manager.o
	$(CC) -o $(PLATFORM)/bin/manager -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/manager.o $(LIBS) -lmpr -L$(PLATFORM)/lib -g

$(PLATFORM)/obj/makerom.o: \
        src/deps/mpr/makerom.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/mpr/mpr.h
	$(CC) -c -o $(PLATFORM)/obj/makerom.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/mpr/makerom.c

$(PLATFORM)/bin/makerom:  \
        $(PLATFORM)/lib/libmpr.so \
        $(PLATFORM)/obj/makerom.o
	$(CC) -o $(PLATFORM)/bin/makerom -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/makerom.o $(LIBS) -lmpr -L$(PLATFORM)/lib -g

$(PLATFORM)/obj/pcre.o: \
        src/deps/pcre/pcre.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/pcre/pcre.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/pcre.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/pcre/pcre.c

$(PLATFORM)/lib/libpcre.so:  \
        $(PLATFORM)/obj/pcre.o
	$(CC) -shared -o $(PLATFORM)/lib/libpcre.so -L$(PLATFORM)/lib -g $(PLATFORM)/obj/pcre.o $(LIBS)

$(PLATFORM)/obj/httpLib.o: \
        src/deps/http/httpLib.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/http/http.h \
        src/deps/pcre/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/httpLib.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/http/httpLib.c

$(PLATFORM)/lib/libhttp.so:  \
        $(PLATFORM)/lib/libmpr.so \
        $(PLATFORM)/lib/libpcre.so \
        $(PLATFORM)/obj/httpLib.o
	$(CC) -shared -o $(PLATFORM)/lib/libhttp.so -L$(PLATFORM)/lib -g $(PLATFORM)/obj/httpLib.o $(LIBS) -lmpr -lpcre

$(PLATFORM)/obj/http.o: \
        src/deps/http/http.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/http/http.h
	$(CC) -c -o $(PLATFORM)/obj/http.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/http/http.c

$(PLATFORM)/bin/http:  \
        $(PLATFORM)/lib/libhttp.so \
        $(PLATFORM)/obj/http.o
	$(CC) -o $(PLATFORM)/bin/http -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/http.o $(LIBS) -lhttp -lmpr -lpcre -L$(PLATFORM)/lib -g

$(PLATFORM)/obj/sqlite3.o: \
        src/deps/sqlite/sqlite3.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/sqlite/sqlite3.h
	$(CC) -c -o $(PLATFORM)/obj/sqlite3.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/deps/sqlite/sqlite3.c

$(PLATFORM)/lib/libsqlite3.so:  \
        $(PLATFORM)/obj/sqlite3.o
	$(CC) -shared -o $(PLATFORM)/lib/libsqlite3.so -L$(PLATFORM)/lib -g $(PLATFORM)/obj/sqlite3.o $(LIBS)

solaris-i686-debug/inc/ejs.slots.h: 
	cp /Users/mob/git/ejs/src/slots/*.h /Users/mob/git/ejs/solaris-i686-debug/inc

$(PLATFORM)/obj/ecAst.o: \
        src/compiler/ecAst.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejsCompiler.h
	$(CC) -c -o $(PLATFORM)/obj/ecAst.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecAst.c

$(PLATFORM)/obj/ecCodeGen.o: \
        src/compiler/ecCodeGen.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejsCompiler.h
	$(CC) -c -o $(PLATFORM)/obj/ecCodeGen.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecCodeGen.c

$(PLATFORM)/obj/ecCompiler.o: \
        src/compiler/ecCompiler.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejsCompiler.h
	$(CC) -c -o $(PLATFORM)/obj/ecCompiler.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecCompiler.c

$(PLATFORM)/obj/ecLex.o: \
        src/compiler/ecLex.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejsCompiler.h
	$(CC) -c -o $(PLATFORM)/obj/ecLex.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecLex.c

$(PLATFORM)/obj/ecModuleWrite.o: \
        src/compiler/ecModuleWrite.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejsCompiler.h
	$(CC) -c -o $(PLATFORM)/obj/ecModuleWrite.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecModuleWrite.c

$(PLATFORM)/obj/ecParser.o: \
        src/compiler/ecParser.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejsCompiler.h
	$(CC) -c -o $(PLATFORM)/obj/ecParser.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecParser.c

$(PLATFORM)/obj/ecState.o: \
        src/compiler/ecState.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejsCompiler.h
	$(CC) -c -o $(PLATFORM)/obj/ecState.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/compiler/ecState.c

$(PLATFORM)/obj/ejsApp.o: \
        src/core/src/ejsApp.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsApp.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsApp.c

$(PLATFORM)/obj/ejsArray.o: \
        src/core/src/ejsArray.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsArray.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsArray.c

$(PLATFORM)/obj/ejsBlock.o: \
        src/core/src/ejsBlock.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsBlock.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsBlock.c

$(PLATFORM)/obj/ejsBoolean.o: \
        src/core/src/ejsBoolean.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsBoolean.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsBoolean.c

$(PLATFORM)/obj/ejsByteArray.o: \
        src/core/src/ejsByteArray.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsByteArray.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsByteArray.c

$(PLATFORM)/obj/ejsCache.o: \
        src/core/src/ejsCache.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsCache.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsCache.c

$(PLATFORM)/obj/ejsCmd.o: \
        src/core/src/ejsCmd.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsCmd.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsCmd.c

$(PLATFORM)/obj/ejsConfig.o: \
        src/core/src/ejsConfig.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsConfig.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsConfig.c

$(PLATFORM)/obj/ejsDate.o: \
        src/core/src/ejsDate.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsDate.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsDate.c

$(PLATFORM)/obj/ejsDebug.o: \
        src/core/src/ejsDebug.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsDebug.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsDebug.c

$(PLATFORM)/obj/ejsError.o: \
        src/core/src/ejsError.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsError.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsError.c

$(PLATFORM)/obj/ejsFile.o: \
        src/core/src/ejsFile.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsFile.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFile.c

$(PLATFORM)/obj/ejsFileSystem.o: \
        src/core/src/ejsFileSystem.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsFileSystem.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFileSystem.c

$(PLATFORM)/obj/ejsFrame.o: \
        src/core/src/ejsFrame.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsFrame.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFrame.c

$(PLATFORM)/obj/ejsFunction.o: \
        src/core/src/ejsFunction.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsFunction.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsFunction.c

$(PLATFORM)/obj/ejsGC.o: \
        src/core/src/ejsGC.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsGC.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsGC.c

$(PLATFORM)/obj/ejsGlobal.o: \
        src/core/src/ejsGlobal.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsGlobal.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsGlobal.c

$(PLATFORM)/obj/ejsHttp.o: \
        src/core/src/ejsHttp.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsHttp.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsHttp.c

$(PLATFORM)/obj/ejsIterator.o: \
        src/core/src/ejsIterator.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsIterator.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsIterator.c

$(PLATFORM)/obj/ejsJSON.o: \
        src/core/src/ejsJSON.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsJSON.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsJSON.c

$(PLATFORM)/obj/ejsLocalCache.o: \
        src/core/src/ejsLocalCache.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsLocalCache.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsLocalCache.c

$(PLATFORM)/obj/ejsMath.o: \
        src/core/src/ejsMath.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsMath.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMath.c

$(PLATFORM)/obj/ejsMemory.o: \
        src/core/src/ejsMemory.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsMemory.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMemory.c

$(PLATFORM)/obj/ejsMprLog.o: \
        src/core/src/ejsMprLog.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsMprLog.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsMprLog.c

$(PLATFORM)/obj/ejsNamespace.o: \
        src/core/src/ejsNamespace.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsNamespace.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNamespace.c

$(PLATFORM)/obj/ejsNull.o: \
        src/core/src/ejsNull.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsNull.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNull.c

$(PLATFORM)/obj/ejsNumber.o: \
        src/core/src/ejsNumber.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsNumber.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsNumber.c

$(PLATFORM)/obj/ejsObject.o: \
        src/core/src/ejsObject.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsObject.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsObject.c

$(PLATFORM)/obj/ejsPath.o: \
        src/core/src/ejsPath.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h \
        src/deps/pcre/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/ejsPath.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsPath.c

$(PLATFORM)/obj/ejsPot.o: \
        src/core/src/ejsPot.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsPot.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsPot.c

$(PLATFORM)/obj/ejsRegExp.o: \
        src/core/src/ejsRegExp.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h \
        src/deps/pcre/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/ejsRegExp.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsRegExp.c

$(PLATFORM)/obj/ejsSocket.o: \
        src/core/src/ejsSocket.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsSocket.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsSocket.c

$(PLATFORM)/obj/ejsString.o: \
        src/core/src/ejsString.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h \
        src/deps/pcre/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/ejsString.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsString.c

$(PLATFORM)/obj/ejsSystem.o: \
        src/core/src/ejsSystem.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsSystem.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsSystem.c

$(PLATFORM)/obj/ejsTimer.o: \
        src/core/src/ejsTimer.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsTimer.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsTimer.c

$(PLATFORM)/obj/ejsType.o: \
        src/core/src/ejsType.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsType.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsType.c

$(PLATFORM)/obj/ejsUri.o: \
        src/core/src/ejsUri.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsUri.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsUri.c

$(PLATFORM)/obj/ejsVoid.o: \
        src/core/src/ejsVoid.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsVoid.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsVoid.c

$(PLATFORM)/obj/ejsWorker.o: \
        src/core/src/ejsWorker.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsWorker.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsWorker.c

$(PLATFORM)/obj/ejsXML.o: \
        src/core/src/ejsXML.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsXML.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXML.c

$(PLATFORM)/obj/ejsXMLList.o: \
        src/core/src/ejsXMLList.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsXMLList.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXMLList.c

$(PLATFORM)/obj/ejsXMLLoader.o: \
        src/core/src/ejsXMLLoader.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsXMLLoader.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/core/src/ejsXMLLoader.c

$(PLATFORM)/obj/ejsByteCode.o: \
        src/vm/ejsByteCode.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsByteCode.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsByteCode.c

$(PLATFORM)/obj/ejsException.o: \
        src/vm/ejsException.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsException.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsException.c

$(PLATFORM)/obj/ejsHelper.o: \
        src/vm/ejsHelper.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsHelper.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsHelper.c

$(PLATFORM)/obj/ejsInterp.o: \
        src/vm/ejsInterp.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsInterp.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsInterp.c

$(PLATFORM)/obj/ejsLoader.o: \
        src/vm/ejsLoader.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsLoader.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsLoader.c

$(PLATFORM)/obj/ejsModule.o: \
        src/vm/ejsModule.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsModule.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsModule.c

$(PLATFORM)/obj/ejsScope.o: \
        src/vm/ejsScope.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsScope.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsScope.c

$(PLATFORM)/obj/ejsService.o: \
        src/vm/ejsService.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h
	$(CC) -c -o $(PLATFORM)/obj/ejsService.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/vm/ejsService.c

$(PLATFORM)/lib/libejs.so:  \
        $(PLATFORM)/lib/libmpr.so \
        $(PLATFORM)/lib/libpcre.so \
        $(PLATFORM)/lib/libhttp.so \
        $(PLATFORM)/inc/ejs.slots.h \
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
	$(CC) -shared -o $(PLATFORM)/lib/libejs.so -L$(PLATFORM)/lib -g $(PLATFORM)/obj/ecAst.o $(PLATFORM)/obj/ecCodeGen.o $(PLATFORM)/obj/ecCompiler.o $(PLATFORM)/obj/ecLex.o $(PLATFORM)/obj/ecModuleWrite.o $(PLATFORM)/obj/ecParser.o $(PLATFORM)/obj/ecState.o $(PLATFORM)/obj/ejsApp.o $(PLATFORM)/obj/ejsArray.o $(PLATFORM)/obj/ejsBlock.o $(PLATFORM)/obj/ejsBoolean.o $(PLATFORM)/obj/ejsByteArray.o $(PLATFORM)/obj/ejsCache.o $(PLATFORM)/obj/ejsCmd.o $(PLATFORM)/obj/ejsConfig.o $(PLATFORM)/obj/ejsDate.o $(PLATFORM)/obj/ejsDebug.o $(PLATFORM)/obj/ejsError.o $(PLATFORM)/obj/ejsFile.o $(PLATFORM)/obj/ejsFileSystem.o $(PLATFORM)/obj/ejsFrame.o $(PLATFORM)/obj/ejsFunction.o $(PLATFORM)/obj/ejsGC.o $(PLATFORM)/obj/ejsGlobal.o $(PLATFORM)/obj/ejsHttp.o $(PLATFORM)/obj/ejsIterator.o $(PLATFORM)/obj/ejsJSON.o $(PLATFORM)/obj/ejsLocalCache.o $(PLATFORM)/obj/ejsMath.o $(PLATFORM)/obj/ejsMemory.o $(PLATFORM)/obj/ejsMprLog.o $(PLATFORM)/obj/ejsNamespace.o $(PLATFORM)/obj/ejsNull.o $(PLATFORM)/obj/ejsNumber.o $(PLATFORM)/obj/ejsObject.o $(PLATFORM)/obj/ejsPath.o $(PLATFORM)/obj/ejsPot.o $(PLATFORM)/obj/ejsRegExp.o $(PLATFORM)/obj/ejsSocket.o $(PLATFORM)/obj/ejsString.o $(PLATFORM)/obj/ejsSystem.o $(PLATFORM)/obj/ejsTimer.o $(PLATFORM)/obj/ejsType.o $(PLATFORM)/obj/ejsUri.o $(PLATFORM)/obj/ejsVoid.o $(PLATFORM)/obj/ejsWorker.o $(PLATFORM)/obj/ejsXML.o $(PLATFORM)/obj/ejsXMLList.o $(PLATFORM)/obj/ejsXMLLoader.o $(PLATFORM)/obj/ejsByteCode.o $(PLATFORM)/obj/ejsException.o $(PLATFORM)/obj/ejsHelper.o $(PLATFORM)/obj/ejsInterp.o $(PLATFORM)/obj/ejsLoader.o $(PLATFORM)/obj/ejsModule.o $(PLATFORM)/obj/ejsScope.o $(PLATFORM)/obj/ejsService.o $(LIBS) -lmpr -lpcre -lhttp

$(PLATFORM)/obj/ejs.o: \
        src/cmd/ejs.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejsCompiler.h
	$(CC) -c -o $(PLATFORM)/obj/ejs.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejs.c

$(PLATFORM)/bin/ejs:  \
        $(PLATFORM)/lib/libejs.so \
        $(PLATFORM)/obj/ejs.o
	$(CC) -o $(PLATFORM)/bin/ejs -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/ejs.o $(LIBS) -lejs -lmpr -lpcre -lhttp -L$(PLATFORM)/lib -g

$(PLATFORM)/obj/ejsc.o: \
        src/cmd/ejsc.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejsCompiler.h
	$(CC) -c -o $(PLATFORM)/obj/ejsc.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejsc.c

$(PLATFORM)/bin/ejsc:  \
        $(PLATFORM)/lib/libejs.so \
        $(PLATFORM)/obj/ejsc.o
	$(CC) -o $(PLATFORM)/bin/ejsc -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/ejsc.o $(LIBS) -lejs -lmpr -lpcre -lhttp -L$(PLATFORM)/lib -g

$(PLATFORM)/obj/ejsmod.o: \
        src/cmd/ejsmod.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(PLATFORM)/obj/ejsmod.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/ejsmod.c

$(PLATFORM)/obj/doc.o: \
        src/cmd/doc.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(PLATFORM)/obj/doc.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/doc.c

$(PLATFORM)/obj/docFiles.o: \
        src/cmd/docFiles.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(PLATFORM)/obj/docFiles.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/docFiles.c

$(PLATFORM)/obj/listing.o: \
        src/cmd/listing.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h \
        src/ejsByteCodeTable.h
	$(CC) -c -o $(PLATFORM)/obj/listing.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/listing.c

$(PLATFORM)/obj/slotGen.o: \
        src/cmd/slotGen.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(PLATFORM)/obj/slotGen.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/cmd src/cmd/slotGen.c

$(PLATFORM)/bin/ejsmod:  \
        $(PLATFORM)/lib/libejs.so \
        $(PLATFORM)/obj/ejsmod.o \
        $(PLATFORM)/obj/doc.o \
        $(PLATFORM)/obj/docFiles.o \
        $(PLATFORM)/obj/listing.o \
        $(PLATFORM)/obj/slotGen.o
	$(CC) -o $(PLATFORM)/bin/ejsmod -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/ejsmod.o $(PLATFORM)/obj/doc.o $(PLATFORM)/obj/docFiles.o $(PLATFORM)/obj/listing.o $(PLATFORM)/obj/slotGen.o $(LIBS) -lejs -lmpr -lpcre -lhttp -L$(PLATFORM)/lib -g

$(PLATFORM)/obj/ejsrun.o: \
        src/cmd/ejsrun.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejsCompiler.h
	$(CC) -c -o $(PLATFORM)/obj/ejsrun.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/cmd/ejsrun.c

$(PLATFORM)/bin/ejsrun:  \
        $(PLATFORM)/lib/libejs.so \
        $(PLATFORM)/obj/ejsrun.o
	$(CC) -o $(PLATFORM)/bin/ejsrun -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/ejsrun.o $(LIBS) -lejs -lmpr -lpcre -lhttp -L$(PLATFORM)/lib -g

solaris-i686-debug/lib/ejs.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod
	ejsc --out /Users/mob/git/ejs/solaris-i686-debug/lib/ejs.mod --debug --optimize 9 --bind --require null /Users/mob/git/ejs/src/core/*.es 
	ejsmod --require null --listing --cslots /Users/mob/git/ejs/solaris-i686-debug/lib/ejs.mod
	cp ejs.slots.h /Users/mob/git/ejs/solaris-i686-debug/inc

solaris-i686-debug/bin/bit.es: 
	cp /Users/mob/git/ejs/src/jems/ejs.bit/bit.es /Users/mob/git/ejs/solaris-i686-debug/bin

solaris-i686-debug/lib/bits: 
	rm -fr /Users/mob/git/ejs/solaris-i686-debug/lib/bits
	cp -r /Users/mob/git/ejs/src/jems/ejs.bit/bits /Users/mob/git/ejs/solaris-i686-debug/lib

solaris-i686-debug/lib/ejs.unix.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out /Users/mob/git/ejs/solaris-i686-debug/lib/ejs.unix.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.unix/Unix.es

solaris-i686-debug/bin/jem.es: 
	cp /Users/mob/git/ejs/src/jems/ejs.jem/jem.es /Users/mob/git/ejs/solaris-i686-debug/bin

solaris-i686-debug/lib/ejs.db.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out /Users/mob/git/ejs/solaris-i686-debug/lib/ejs.db.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.db/*.es

solaris-i686-debug/lib/ejs.db.mapper.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod \
        $(PLATFORM)/lib/ejs.db.mod
	ejsc --out /Users/mob/git/ejs/solaris-i686-debug/lib/ejs.db.mapper.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.db.mapper/*.es

solaris-i686-debug/lib/ejs.db.sqlite.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out /Users/mob/git/ejs/solaris-i686-debug/lib/ejs.db.sqlite.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.db.sqlite/*.es

$(PLATFORM)/obj/ejsSqlite.o: \
        src/jems/ejs.db.sqlite/src/ejsSqlite.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h \
        src/deps/sqlite/sqlite3.h \
        src/slots/ejs.db.sqlite.slots.h
	$(CC) -c -o $(PLATFORM)/obj/ejsSqlite.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots src/jems/ejs.db.sqlite/src/ejsSqlite.c

$(PLATFORM)/lib/ejs.db.sqlite.so:  \
        $(PLATFORM)/lib/libmpr.so \
        $(PLATFORM)/lib/libejs.so \
        $(PLATFORM)/lib/ejs.mod \
        $(PLATFORM)/lib/ejs.db.sqlite.mod \
        $(PLATFORM)/lib/libsqlite3.so \
        $(PLATFORM)/obj/ejsSqlite.o
	$(CC) -shared -o $(PLATFORM)/lib/ejs.db.sqlite.so -L$(PLATFORM)/lib -g $(PLATFORM)/obj/ejsSqlite.o $(LIBS) -lmpr -lejs -lpcre -lhttp -lsqlite3

solaris-i686-debug/lib/ejs.web.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out /Users/mob/git/ejs/solaris-i686-debug/lib/ejs.web.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.web/*.es
	ejsmod --cslots /Users/mob/git/ejs/solaris-i686-debug/lib/ejs.web.mod
	cp ejs.web.slots.h /Users/mob/git/ejs/solaris-i686-debug/inc

$(PLATFORM)/obj/ejsHttpServer.o: \
        src/jems/ejs.web/src/ejsHttpServer.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h \
        src/ejsCompiler.h \
        src/jems/ejs.web/src/ejsWeb.h \
        src/slots/ejs.web.slots.h
	$(CC) -c -o $(PLATFORM)/obj/ejsHttpServer.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsHttpServer.c

$(PLATFORM)/obj/ejsRequest.o: \
        src/jems/ejs.web/src/ejsRequest.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h \
        src/ejsCompiler.h \
        src/jems/ejs.web/src/ejsWeb.h \
        src/slots/ejs.web.slots.h
	$(CC) -c -o $(PLATFORM)/obj/ejsRequest.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsRequest.c

$(PLATFORM)/obj/ejsSession.o: \
        src/jems/ejs.web/src/ejsSession.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h \
        src/jems/ejs.web/src/ejsWeb.h
	$(CC) -c -o $(PLATFORM)/obj/ejsSession.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsSession.c

$(PLATFORM)/obj/ejsWeb.o: \
        src/jems/ejs.web/src/ejsWeb.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/ejs.h \
        src/ejsCompiler.h \
        src/jems/ejs.web/src/ejsWeb.h \
        src/slots/ejs.web.slots.h
	$(CC) -c -o $(PLATFORM)/obj/ejsWeb.o $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/slots -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsWeb.c

$(PLATFORM)/lib/ejs.web.so:  \
        $(PLATFORM)/lib/libmpr.so \
        $(PLATFORM)/lib/libhttp.so \
        $(PLATFORM)/lib/libpcre.so \
        $(PLATFORM)/lib/libejs.so \
        $(PLATFORM)/lib/ejs.mod \
        $(PLATFORM)/obj/ejsHttpServer.o \
        $(PLATFORM)/obj/ejsRequest.o \
        $(PLATFORM)/obj/ejsSession.o \
        $(PLATFORM)/obj/ejsWeb.o
	$(CC) -shared -o $(PLATFORM)/lib/ejs.web.so -L$(PLATFORM)/lib -g $(PLATFORM)/obj/ejsHttpServer.o $(PLATFORM)/obj/ejsRequest.o $(PLATFORM)/obj/ejsSession.o $(PLATFORM)/obj/ejsWeb.o $(LIBS) -lmpr -lhttp -lpcre -lpcre -lejs

solaris-i686-debug/lib/ejs.template.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out /Users/mob/git/ejs/solaris-i686-debug/lib/ejs.template.mod --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.template/*.es

solaris-i686-debug/lib/ejs.tar.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod
	ejsc --out /Users/mob/git/ejs/solaris-i686-debug/lib/ejs.tar.mod/ --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.tar/*.es

solaris-i686-debug/bin/mvc.es: 
	cp /Users/mob/git/ejs/src/jems/ejs.mvc/mvc.es /Users/mob/git/ejs/solaris-i686-debug/bin

solaris-i686-debug/lib/ejs.mvc.mod:  \
        $(PLATFORM)/bin/ejsc \
        $(PLATFORM)/bin/ejsmod \
        $(PLATFORM)/lib/ejs.mod \
        $(PLATFORM)/lib/ejs.web.mod \
        $(PLATFORM)/lib/ejs.template.mod \
        $(PLATFORM)/lib/ejs.unix.mod
	ejsc --out /Users/mob/git/ejs/solaris-i686-debug/lib/ejs.mvc.mod/ --debug --optimize 9 /Users/mob/git/ejs/src/jems/ejs.mvc/*.es

