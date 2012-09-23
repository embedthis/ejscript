#
#   ejs-macosx.mk -- Makefile to build Embedthis Ejscript for macosx
#

ARCH     ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/')
OS       ?= macosx
CC       ?= /usr/bin/clang
LD       ?= /usr/bin/ld
PROFILE  ?= debug
CONFIG   ?= $(OS)-$(ARCH)-$(PROFILE)

CFLAGS   += -Wno-deprecated-declarations -w
DFLAGS   += 
IFLAGS   += -I$(CONFIG)/inc
LDFLAGS  += '-Wl,-rpath,@executable_path/' '-Wl,-rpath,@loader_path/'
LIBPATHS += -L$(CONFIG)/bin
LIBS     += -lpthread -lm -ldl

CFLAGS-debug    := -DBIT_DEBUG -g
CFLAGS-release  := -O2
LDFLAGS-debug   := -g
LDFLAGS-release := 
CFLAGS          += $(CFLAGS-$(PROFILE))
LDFLAGS         += $(LDFLAGS-$(PROFILE))

all: prep \
        $(CONFIG)/bin/libmpr.dylib \
        $(CONFIG)/bin/libmprssl.dylib \
        $(CONFIG)/bin/ejsman \
        $(CONFIG)/bin/makerom \
        $(CONFIG)/bin/libpcre.dylib \
        $(CONFIG)/bin/libhttp.dylib \
        $(CONFIG)/bin/http \
        $(CONFIG)/bin/libsqlite3.dylib \
        $(CONFIG)/bin/sqlite \
        $(CONFIG)/bin/libzlib.dylib \
        $(CONFIG)/bin/libejs.dylib \
        $(CONFIG)/bin/ejs \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/bin/ejsrun \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/bin/bit.es \
        $(CONFIG)/bin/bit \
        $(CONFIG)/bin/utest.es \
        $(CONFIG)/bin/utest \
        $(CONFIG)/bin/bits \
        $(CONFIG)/bin/ejs.unix.mod \
        $(CONFIG)/bin/jem.es \
        $(CONFIG)/bin/jem \
        $(CONFIG)/bin/ejs.db.mod \
        $(CONFIG)/bin/ejs.db.mapper.mod \
        $(CONFIG)/bin/ejs.db.sqlite.mod \
        $(CONFIG)/bin/ejs.db.sqlite.dylib \
        $(CONFIG)/bin/ejs.web.mod \
        $(CONFIG)/bin/ejs.web.dylib \
        $(CONFIG)/bin/www \
        $(CONFIG)/bin/ejs.template.mod \
        $(CONFIG)/bin/ejs.tar.mod \
        $(CONFIG)/bin/ejs.zlib.mod \
        $(CONFIG)/bin/ejs.zlib.dylib \
        $(CONFIG)/bin/mvc.es \
        $(CONFIG)/bin/mvc \
        $(CONFIG)/bin/ejs.mvc.mod \
        $(CONFIG)/bin/utest.worker

.PHONY: prep

prep:
	@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi
	@[ ! -x $(CONFIG)/inc ] && mkdir -p $(CONFIG)/inc $(CONFIG)/obj $(CONFIG)/lib $(CONFIG)/bin ; true
	@[ ! -f $(CONFIG)/inc/bit.h ] && cp projects/ejs-$(OS)-bit.h $(CONFIG)/inc/bit.h ; true
	@if ! diff $(CONFIG)/inc/bit.h projects/ejs-$(OS)-bit.h >/dev/null ; then\
		echo cp projects/ejs-$(OS)-bit.h $(CONFIG)/inc/bit.h  ; \
		cp projects/ejs-$(OS)-bit.h $(CONFIG)/inc/bit.h  ; \
	fi; true

clean:
	rm -rf $(CONFIG)/bin/libmpr.dylib
	rm -rf $(CONFIG)/bin/libmprssl.dylib
	rm -rf $(CONFIG)/bin/ejsman
	rm -rf $(CONFIG)/bin/makerom
	rm -rf $(CONFIG)/bin/libpcre.dylib
	rm -rf $(CONFIG)/bin/libhttp.dylib
	rm -rf $(CONFIG)/bin/http
	rm -rf $(CONFIG)/bin/libsqlite3.dylib
	rm -rf $(CONFIG)/bin/sqlite
	rm -rf $(CONFIG)/bin/libzlib.dylib
	rm -rf $(CONFIG)/bin/libejs.dylib
	rm -rf $(CONFIG)/bin/ejs
	rm -rf $(CONFIG)/bin/ejsc
	rm -rf $(CONFIG)/bin/ejsmod
	rm -rf $(CONFIG)/bin/ejsrun
	rm -rf $(CONFIG)/bin/utest.es
	rm -rf $(CONFIG)/bin/utest
	rm -rf $(CONFIG)/bin/jem.es
	rm -rf $(CONFIG)/bin/jem
	rm -rf $(CONFIG)/bin/ejs.db.mod
	rm -rf $(CONFIG)/bin/ejs.db.mapper.mod
	rm -rf $(CONFIG)/bin/ejs.db.sqlite.mod
	rm -rf $(CONFIG)/bin/ejs.db.sqlite.dylib
	rm -rf $(CONFIG)/bin/ejs.web.mod
	rm -rf $(CONFIG)/bin/ejs.web.dylib
	rm -rf $(CONFIG)/bin/www
	rm -rf $(CONFIG)/bin/ejs.template.mod
	rm -rf $(CONFIG)/bin/ejs.zlib.dylib
	rm -rf $(CONFIG)/bin/mvc.es
	rm -rf $(CONFIG)/bin/mvc
	rm -rf $(CONFIG)/bin/ejs.mvc.mod
	rm -rf $(CONFIG)/bin/utest.worker
	rm -rf $(CONFIG)/obj/mprLib.o
	rm -rf $(CONFIG)/obj/mprSsl.o
	rm -rf $(CONFIG)/obj/manager.o
	rm -rf $(CONFIG)/obj/makerom.o
	rm -rf $(CONFIG)/obj/pcre.o
	rm -rf $(CONFIG)/obj/httpLib.o
	rm -rf $(CONFIG)/obj/http.o
	rm -rf $(CONFIG)/obj/sqlite3.o
	rm -rf $(CONFIG)/obj/sqlite.o
	rm -rf $(CONFIG)/obj/adler32.o
	rm -rf $(CONFIG)/obj/compress.o
	rm -rf $(CONFIG)/obj/crc32.o
	rm -rf $(CONFIG)/obj/deflate.o
	rm -rf $(CONFIG)/obj/gzclose.o
	rm -rf $(CONFIG)/obj/gzlib.o
	rm -rf $(CONFIG)/obj/gzread.o
	rm -rf $(CONFIG)/obj/gzwrite.o
	rm -rf $(CONFIG)/obj/infback.o
	rm -rf $(CONFIG)/obj/inffast.o
	rm -rf $(CONFIG)/obj/inflate.o
	rm -rf $(CONFIG)/obj/inftrees.o
	rm -rf $(CONFIG)/obj/trees.o
	rm -rf $(CONFIG)/obj/uncompr.o
	rm -rf $(CONFIG)/obj/zutil.o
	rm -rf $(CONFIG)/obj/ecAst.o
	rm -rf $(CONFIG)/obj/ecCodeGen.o
	rm -rf $(CONFIG)/obj/ecCompiler.o
	rm -rf $(CONFIG)/obj/ecLex.o
	rm -rf $(CONFIG)/obj/ecModuleWrite.o
	rm -rf $(CONFIG)/obj/ecParser.o
	rm -rf $(CONFIG)/obj/ecState.o
	rm -rf $(CONFIG)/obj/dtoa.o
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
	rm -rf $(CONFIG)/obj/removeFiles.o

clobber: clean
	rm -fr ./$(CONFIG)

$(CONFIG)/inc/mpr.h: 
	rm -fr $(CONFIG)/inc/mpr.h
	cp -r src/deps/mpr/mpr.h $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/mprLib.o: \
        src/deps/mpr/mprLib.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/mprLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/mprLib.c

$(CONFIG)/bin/libmpr.dylib:  \
        $(CONFIG)/inc/mpr.h \
        $(CONFIG)/obj/mprLib.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmpr.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 2.0.1 -current_version 2.0.1 -compatibility_version 2.0.1 -current_version 2.0.1 $(LIBPATHS) -install_name @rpath/libmpr.dylib $(CONFIG)/obj/mprLib.o $(LIBS)

$(CONFIG)/obj/mprSsl.o: \
        src/deps/mpr/mprSsl.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/mprSsl.c

$(CONFIG)/bin/libmprssl.dylib:  \
        $(CONFIG)/bin/libmpr.dylib \
        $(CONFIG)/obj/mprSsl.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmprssl.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 2.0.1 -current_version 2.0.1 -compatibility_version 2.0.1 -current_version 2.0.1 $(LIBPATHS) -install_name @rpath/libmprssl.dylib $(CONFIG)/obj/mprSsl.o $(LIBS) -lmpr

$(CONFIG)/obj/manager.o: \
        src/deps/mpr/manager.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/manager.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/manager.c

$(CONFIG)/bin/ejsman:  \
        $(CONFIG)/bin/libmpr.dylib \
        $(CONFIG)/obj/manager.o
	$(CC) -o $(CONFIG)/bin/ejsman -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/manager.o $(LIBS) -lmpr

$(CONFIG)/obj/makerom.o: \
        src/deps/mpr/makerom.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/makerom.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/makerom.c

$(CONFIG)/bin/makerom:  \
        $(CONFIG)/bin/libmpr.dylib \
        $(CONFIG)/obj/makerom.o
	$(CC) -o $(CONFIG)/bin/makerom -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o $(LIBS) -lmpr

$(CONFIG)/inc/pcre.h: 
	rm -fr $(CONFIG)/inc/pcre.h
	cp -r src/deps/pcre/pcre.h $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
        src/deps/pcre/pcre.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/pcre.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/pcre/pcre.c

$(CONFIG)/bin/libpcre.dylib:  \
        $(CONFIG)/inc/pcre.h \
        $(CONFIG)/obj/pcre.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libpcre.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libpcre.dylib $(CONFIG)/obj/pcre.o $(LIBS)

$(CONFIG)/inc/http.h: 
	rm -fr $(CONFIG)/inc/http.h
	cp -r src/deps/http/http.h $(CONFIG)/inc/http.h

$(CONFIG)/obj/httpLib.o: \
        src/deps/http/httpLib.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/http.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/httpLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/http/httpLib.c

$(CONFIG)/bin/libhttp.dylib:  \
        $(CONFIG)/bin/libmpr.dylib \
        $(CONFIG)/bin/libpcre.dylib \
        $(CONFIG)/inc/http.h \
        $(CONFIG)/obj/httpLib.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libhttp.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 2.0.1 -current_version 2.0.1 -compatibility_version 2.0.1 -current_version 2.0.1 $(LIBPATHS) -install_name @rpath/libhttp.dylib $(CONFIG)/obj/httpLib.o $(LIBS) -lpam -lmpr -lpcre

$(CONFIG)/obj/http.o: \
        src/deps/http/http.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/http.h
	$(CC) -c -o $(CONFIG)/obj/http.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/http/http.c

$(CONFIG)/bin/http:  \
        $(CONFIG)/bin/libhttp.dylib \
        $(CONFIG)/obj/http.o
	$(CC) -o $(CONFIG)/bin/http -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o $(LIBS) -lhttp -lpam -lmpr -lpcre

$(CONFIG)/inc/sqlite3.h: 
	rm -fr $(CONFIG)/inc/sqlite3.h
	cp -r src/deps/sqlite/sqlite3.h $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite3.o: \
        src/deps/sqlite/sqlite3.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/sqlite3.h
	$(CC) -c -o $(CONFIG)/obj/sqlite3.o -arch x86_64 -Wno-deprecated-declarations -g -Wno-unused-result -w -w $(DFLAGS) -I$(CONFIG)/inc src/deps/sqlite/sqlite3.c

$(CONFIG)/bin/libsqlite3.dylib:  \
        $(CONFIG)/inc/sqlite3.h \
        $(CONFIG)/obj/sqlite3.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libsqlite3.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 2.0.1 -current_version 2.0.1 -compatibility_version 2.0.1 -current_version 2.0.1 $(LIBPATHS) -install_name @rpath/libsqlite3.dylib $(CONFIG)/obj/sqlite3.o $(LIBS)

$(CONFIG)/obj/sqlite.o: \
        src/deps/sqlite/sqlite.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/sqlite3.h
	$(CC) -c -o $(CONFIG)/obj/sqlite.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/sqlite/sqlite.c

$(CONFIG)/bin/sqlite:  \
        $(CONFIG)/bin/libsqlite3.dylib \
        $(CONFIG)/obj/sqlite.o
	$(CC) -o $(CONFIG)/bin/sqlite -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/sqlite.o $(LIBS) -lsqlite3

$(CONFIG)/inc/zlib.h: 
	rm -fr $(CONFIG)/inc/zlib.h
	cp -r src/deps/zlib/zlib.h $(CONFIG)/inc/zlib.h

$(CONFIG)/inc/zconf.h: 
	rm -fr $(CONFIG)/inc/zconf.h
	cp -r src/deps/zlib/zconf.h $(CONFIG)/inc/zconf.h

$(CONFIG)/obj/adler32.o: \
        src/deps/zlib/adler32.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/adler32.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/adler32.c

$(CONFIG)/obj/compress.o: \
        src/deps/zlib/compress.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/zlib.h
	$(CC) -c -o $(CONFIG)/obj/compress.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/compress.c

$(CONFIG)/obj/crc32.o: \
        src/deps/zlib/crc32.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/crc32.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/crc32.c

$(CONFIG)/obj/deflate.o: \
        src/deps/zlib/deflate.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/deflate.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/deflate.c

$(CONFIG)/obj/gzclose.o: \
        src/deps/zlib/gzclose.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/gzclose.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/gzclose.c

$(CONFIG)/obj/gzlib.o: \
        src/deps/zlib/gzlib.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/gzlib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/gzlib.c

$(CONFIG)/obj/gzread.o: \
        src/deps/zlib/gzread.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/gzread.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/gzread.c

$(CONFIG)/obj/gzwrite.o: \
        src/deps/zlib/gzwrite.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/gzwrite.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/gzwrite.c

$(CONFIG)/obj/infback.o: \
        src/deps/zlib/infback.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/infback.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/infback.c

$(CONFIG)/obj/inffast.o: \
        src/deps/zlib/inffast.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/inffast.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/inffast.c

$(CONFIG)/obj/inflate.o: \
        src/deps/zlib/inflate.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/inflate.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/inflate.c

$(CONFIG)/obj/inftrees.o: \
        src/deps/zlib/inftrees.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/inftrees.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/inftrees.c

$(CONFIG)/obj/trees.o: \
        src/deps/zlib/trees.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/trees.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/trees.c

$(CONFIG)/obj/uncompr.o: \
        src/deps/zlib/uncompr.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/zlib.h
	$(CC) -c -o $(CONFIG)/obj/uncompr.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/uncompr.c

$(CONFIG)/obj/zutil.o: \
        src/deps/zlib/zutil.c \
        $(CONFIG)/inc/bit.h
	$(CC) -c -o $(CONFIG)/obj/zutil.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/zutil.c

$(CONFIG)/bin/libzlib.dylib:  \
        $(CONFIG)/inc/zlib.h \
        $(CONFIG)/inc/zconf.h \
        $(CONFIG)/obj/adler32.o \
        $(CONFIG)/obj/compress.o \
        $(CONFIG)/obj/crc32.o \
        $(CONFIG)/obj/deflate.o \
        $(CONFIG)/obj/gzclose.o \
        $(CONFIG)/obj/gzlib.o \
        $(CONFIG)/obj/gzread.o \
        $(CONFIG)/obj/gzwrite.o \
        $(CONFIG)/obj/infback.o \
        $(CONFIG)/obj/inffast.o \
        $(CONFIG)/obj/inflate.o \
        $(CONFIG)/obj/inftrees.o \
        $(CONFIG)/obj/trees.o \
        $(CONFIG)/obj/uncompr.o \
        $(CONFIG)/obj/zutil.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libzlib.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 2.0.1 -current_version 2.0.1 -compatibility_version 2.0.1 -current_version 2.0.1 $(LIBPATHS) -install_name @rpath/libzlib.dylib $(CONFIG)/obj/adler32.o $(CONFIG)/obj/compress.o $(CONFIG)/obj/crc32.o $(CONFIG)/obj/deflate.o $(CONFIG)/obj/gzclose.o $(CONFIG)/obj/gzlib.o $(CONFIG)/obj/gzread.o $(CONFIG)/obj/gzwrite.o $(CONFIG)/obj/infback.o $(CONFIG)/obj/inffast.o $(CONFIG)/obj/inflate.o $(CONFIG)/obj/inftrees.o $(CONFIG)/obj/trees.o $(CONFIG)/obj/uncompr.o $(CONFIG)/obj/zutil.o $(LIBS)

$(CONFIG)/inc/ejs.cache.local.slots.h: 
	rm -fr $(CONFIG)/inc/ejs.cache.local.slots.h
	cp -r src/slots/ejs.cache.local.slots.h $(CONFIG)/inc/ejs.cache.local.slots.h

$(CONFIG)/inc/ejs.db.sqlite.slots.h: 
	rm -fr $(CONFIG)/inc/ejs.db.sqlite.slots.h
	cp -r src/slots/ejs.db.sqlite.slots.h $(CONFIG)/inc/ejs.db.sqlite.slots.h

$(CONFIG)/inc/ejs.slots.h: 
	rm -fr $(CONFIG)/inc/ejs.slots.h
	cp -r src/slots/ejs.slots.h $(CONFIG)/inc/ejs.slots.h

$(CONFIG)/inc/ejs.web.slots.h: 
	rm -fr $(CONFIG)/inc/ejs.web.slots.h
	cp -r src/slots/ejs.web.slots.h $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/inc/ejs.zlib.slots.h: 
	rm -fr $(CONFIG)/inc/ejs.zlib.slots.h
	cp -r src/slots/ejs.zlib.slots.h $(CONFIG)/inc/ejs.zlib.slots.h

$(CONFIG)/inc/ejs.h: 
	rm -fr $(CONFIG)/inc/ejs.h
	cp -r src/ejs.h $(CONFIG)/inc/ejs.h

$(CONFIG)/inc/ejsByteCode.h: 
	rm -fr $(CONFIG)/inc/ejsByteCode.h
	cp -r src/ejsByteCode.h $(CONFIG)/inc/ejsByteCode.h

$(CONFIG)/inc/ejsByteCodeTable.h: 
	rm -fr $(CONFIG)/inc/ejsByteCodeTable.h
	cp -r src/ejsByteCodeTable.h $(CONFIG)/inc/ejsByteCodeTable.h

$(CONFIG)/inc/ejsCompiler.h: 
	rm -fr $(CONFIG)/inc/ejsCompiler.h
	cp -r src/ejsCompiler.h $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/inc/ejsCustomize.h: 
	rm -fr $(CONFIG)/inc/ejsCustomize.h
	cp -r src/ejsCustomize.h $(CONFIG)/inc/ejsCustomize.h

$(CONFIG)/obj/ecAst.o: \
        src/compiler/ecAst.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecAst.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecAst.c

$(CONFIG)/obj/ecCodeGen.o: \
        src/compiler/ecCodeGen.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecCodeGen.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecCodeGen.c

$(CONFIG)/obj/ecCompiler.o: \
        src/compiler/ecCompiler.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecCompiler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecCompiler.c

$(CONFIG)/obj/ecLex.o: \
        src/compiler/ecLex.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecLex.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecLex.c

$(CONFIG)/obj/ecModuleWrite.o: \
        src/compiler/ecModuleWrite.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecModuleWrite.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecModuleWrite.c

$(CONFIG)/obj/ecParser.o: \
        src/compiler/ecParser.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecParser.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecParser.c

$(CONFIG)/obj/ecState.o: \
        src/compiler/ecState.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecState.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecState.c

$(CONFIG)/obj/dtoa.o: \
        src/core/src/dtoa.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/dtoa.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/dtoa.c

$(CONFIG)/obj/ejsApp.o: \
        src/core/src/ejsApp.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsApp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsApp.c

$(CONFIG)/obj/ejsArray.o: \
        src/core/src/ejsArray.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsArray.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsArray.c

$(CONFIG)/obj/ejsBlock.o: \
        src/core/src/ejsBlock.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsBlock.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsBlock.c

$(CONFIG)/obj/ejsBoolean.o: \
        src/core/src/ejsBoolean.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsBoolean.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsBoolean.c

$(CONFIG)/obj/ejsByteArray.o: \
        src/core/src/ejsByteArray.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsByteArray.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsByteArray.c

$(CONFIG)/obj/ejsCache.o: \
        src/core/src/ejsCache.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsCache.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsCache.c

$(CONFIG)/obj/ejsCmd.o: \
        src/core/src/ejsCmd.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsCmd.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsCmd.c

$(CONFIG)/obj/ejsConfig.o: \
        src/core/src/ejsConfig.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsConfig.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsConfig.c

$(CONFIG)/obj/ejsDate.o: \
        src/core/src/ejsDate.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsDate.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsDate.c

$(CONFIG)/obj/ejsDebug.o: \
        src/core/src/ejsDebug.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsDebug.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsDebug.c

$(CONFIG)/obj/ejsError.o: \
        src/core/src/ejsError.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsError.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsError.c

$(CONFIG)/obj/ejsFile.o: \
        src/core/src/ejsFile.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFile.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsFile.c

$(CONFIG)/obj/ejsFileSystem.o: \
        src/core/src/ejsFileSystem.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFileSystem.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsFileSystem.c

$(CONFIG)/obj/ejsFrame.o: \
        src/core/src/ejsFrame.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFrame.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsFrame.c

$(CONFIG)/obj/ejsFunction.o: \
        src/core/src/ejsFunction.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFunction.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsFunction.c

$(CONFIG)/obj/ejsGC.o: \
        src/core/src/ejsGC.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsGC.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsGC.c

$(CONFIG)/obj/ejsGlobal.o: \
        src/core/src/ejsGlobal.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsGlobal.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsGlobal.c

$(CONFIG)/obj/ejsHttp.o: \
        src/core/src/ejsHttp.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsHttp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsHttp.c

$(CONFIG)/obj/ejsIterator.o: \
        src/core/src/ejsIterator.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsIterator.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsIterator.c

$(CONFIG)/obj/ejsJSON.o: \
        src/core/src/ejsJSON.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsJSON.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsJSON.c

$(CONFIG)/obj/ejsLocalCache.o: \
        src/core/src/ejsLocalCache.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsLocalCache.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsLocalCache.c

$(CONFIG)/obj/ejsMath.o: \
        src/core/src/ejsMath.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsMath.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsMath.c

$(CONFIG)/obj/ejsMemory.o: \
        src/core/src/ejsMemory.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsMemory.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsMemory.c

$(CONFIG)/obj/ejsMprLog.o: \
        src/core/src/ejsMprLog.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsMprLog.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsMprLog.c

$(CONFIG)/obj/ejsNamespace.o: \
        src/core/src/ejsNamespace.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsNamespace.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsNamespace.c

$(CONFIG)/obj/ejsNull.o: \
        src/core/src/ejsNull.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsNull.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsNull.c

$(CONFIG)/obj/ejsNumber.o: \
        src/core/src/ejsNumber.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsNumber.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsNumber.c

$(CONFIG)/obj/ejsObject.o: \
        src/core/src/ejsObject.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsObject.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsObject.c

$(CONFIG)/obj/ejsPath.o: \
        src/core/src/ejsPath.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/ejsPath.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsPath.c

$(CONFIG)/obj/ejsPot.o: \
        src/core/src/ejsPot.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsPot.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsPot.c

$(CONFIG)/obj/ejsRegExp.o: \
        src/core/src/ejsRegExp.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/ejsRegExp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsRegExp.c

$(CONFIG)/obj/ejsSocket.o: \
        src/core/src/ejsSocket.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsSocket.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsSocket.c

$(CONFIG)/obj/ejsString.o: \
        src/core/src/ejsString.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/ejsString.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsString.c

$(CONFIG)/obj/ejsSystem.o: \
        src/core/src/ejsSystem.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsSystem.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsSystem.c

$(CONFIG)/obj/ejsTimer.o: \
        src/core/src/ejsTimer.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsTimer.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsTimer.c

$(CONFIG)/obj/ejsType.o: \
        src/core/src/ejsType.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsType.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsType.c

$(CONFIG)/obj/ejsUri.o: \
        src/core/src/ejsUri.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsUri.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsUri.c

$(CONFIG)/obj/ejsVoid.o: \
        src/core/src/ejsVoid.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsVoid.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsVoid.c

$(CONFIG)/obj/ejsWorker.o: \
        src/core/src/ejsWorker.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsWorker.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsWorker.c

$(CONFIG)/obj/ejsXML.o: \
        src/core/src/ejsXML.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsXML.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsXML.c

$(CONFIG)/obj/ejsXMLList.o: \
        src/core/src/ejsXMLList.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsXMLList.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsXMLList.c

$(CONFIG)/obj/ejsXMLLoader.o: \
        src/core/src/ejsXMLLoader.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsXMLLoader.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsXMLLoader.c

$(CONFIG)/obj/ejsByteCode.o: \
        src/vm/ejsByteCode.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsByteCode.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsByteCode.c

$(CONFIG)/obj/ejsException.o: \
        src/vm/ejsException.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsException.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsException.c

$(CONFIG)/obj/ejsHelper.o: \
        src/vm/ejsHelper.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsHelper.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsHelper.c

$(CONFIG)/obj/ejsInterp.o: \
        src/vm/ejsInterp.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsInterp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsInterp.c

$(CONFIG)/obj/ejsLoader.o: \
        src/vm/ejsLoader.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsLoader.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsLoader.c

$(CONFIG)/obj/ejsModule.o: \
        src/vm/ejsModule.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsModule.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsModule.c

$(CONFIG)/obj/ejsScope.o: \
        src/vm/ejsScope.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsScope.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsScope.c

$(CONFIG)/obj/ejsService.o: \
        src/vm/ejsService.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsService.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsService.c

$(CONFIG)/bin/libejs.dylib:  \
        $(CONFIG)/bin/libhttp.dylib \
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
        $(CONFIG)/obj/dtoa.o \
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
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 2.0.1 -current_version 2.0.1 -compatibility_version 2.0.1 -current_version 2.0.1 $(LIBPATHS) -install_name @rpath/libejs.dylib $(CONFIG)/obj/ecAst.o $(CONFIG)/obj/ecCodeGen.o $(CONFIG)/obj/ecCompiler.o $(CONFIG)/obj/ecLex.o $(CONFIG)/obj/ecModuleWrite.o $(CONFIG)/obj/ecParser.o $(CONFIG)/obj/ecState.o $(CONFIG)/obj/dtoa.o $(CONFIG)/obj/ejsApp.o $(CONFIG)/obj/ejsArray.o $(CONFIG)/obj/ejsBlock.o $(CONFIG)/obj/ejsBoolean.o $(CONFIG)/obj/ejsByteArray.o $(CONFIG)/obj/ejsCache.o $(CONFIG)/obj/ejsCmd.o $(CONFIG)/obj/ejsConfig.o $(CONFIG)/obj/ejsDate.o $(CONFIG)/obj/ejsDebug.o $(CONFIG)/obj/ejsError.o $(CONFIG)/obj/ejsFile.o $(CONFIG)/obj/ejsFileSystem.o $(CONFIG)/obj/ejsFrame.o $(CONFIG)/obj/ejsFunction.o $(CONFIG)/obj/ejsGC.o $(CONFIG)/obj/ejsGlobal.o $(CONFIG)/obj/ejsHttp.o $(CONFIG)/obj/ejsIterator.o $(CONFIG)/obj/ejsJSON.o $(CONFIG)/obj/ejsLocalCache.o $(CONFIG)/obj/ejsMath.o $(CONFIG)/obj/ejsMemory.o $(CONFIG)/obj/ejsMprLog.o $(CONFIG)/obj/ejsNamespace.o $(CONFIG)/obj/ejsNull.o $(CONFIG)/obj/ejsNumber.o $(CONFIG)/obj/ejsObject.o $(CONFIG)/obj/ejsPath.o $(CONFIG)/obj/ejsPot.o $(CONFIG)/obj/ejsRegExp.o $(CONFIG)/obj/ejsSocket.o $(CONFIG)/obj/ejsString.o $(CONFIG)/obj/ejsSystem.o $(CONFIG)/obj/ejsTimer.o $(CONFIG)/obj/ejsType.o $(CONFIG)/obj/ejsUri.o $(CONFIG)/obj/ejsVoid.o $(CONFIG)/obj/ejsWorker.o $(CONFIG)/obj/ejsXML.o $(CONFIG)/obj/ejsXMLList.o $(CONFIG)/obj/ejsXMLLoader.o $(CONFIG)/obj/ejsByteCode.o $(CONFIG)/obj/ejsException.o $(CONFIG)/obj/ejsHelper.o $(CONFIG)/obj/ejsInterp.o $(CONFIG)/obj/ejsLoader.o $(CONFIG)/obj/ejsModule.o $(CONFIG)/obj/ejsScope.o $(CONFIG)/obj/ejsService.o $(LIBS) -lhttp -lpam -lmpr -lpcre

$(CONFIG)/obj/ejs.o: \
        src/cmd/ejs.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ejs.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/cmd/ejs.c

$(CONFIG)/bin/ejs:  \
        $(CONFIG)/bin/libejs.dylib \
        $(CONFIG)/obj/ejs.o
	$(CC) -o $(CONFIG)/bin/ejs -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejs.o $(LIBS) -lejs -lhttp -lpam -lmpr -lpcre -ledit -ledit

$(CONFIG)/obj/ejsc.o: \
        src/cmd/ejsc.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ejsc.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/cmd/ejsc.c

$(CONFIG)/bin/ejsc:  \
        $(CONFIG)/bin/libejs.dylib \
        $(CONFIG)/obj/ejsc.o
	$(CC) -o $(CONFIG)/bin/ejsc -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsc.o $(LIBS) -lejs -lhttp -lpam -lmpr -lpcre

$(CONFIG)/obj/ejsmod.o: \
        src/cmd/ejsmod.c \
        $(CONFIG)/inc/bit.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/ejsmod.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/cmd src/cmd/ejsmod.c

$(CONFIG)/obj/doc.o: \
        src/cmd/doc.c \
        $(CONFIG)/inc/bit.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/doc.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/cmd src/cmd/doc.c

$(CONFIG)/obj/docFiles.o: \
        src/cmd/docFiles.c \
        $(CONFIG)/inc/bit.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/docFiles.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/cmd src/cmd/docFiles.c

$(CONFIG)/obj/listing.o: \
        src/cmd/listing.c \
        $(CONFIG)/inc/bit.h \
        src/cmd/ejsmod.h \
        $(CONFIG)/inc/ejsByteCodeTable.h
	$(CC) -c -o $(CONFIG)/obj/listing.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/cmd src/cmd/listing.c

$(CONFIG)/obj/slotGen.o: \
        src/cmd/slotGen.c \
        $(CONFIG)/inc/bit.h \
        src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/slotGen.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/cmd src/cmd/slotGen.c

$(CONFIG)/bin/ejsmod:  \
        $(CONFIG)/bin/libejs.dylib \
        $(CONFIG)/obj/ejsmod.o \
        $(CONFIG)/obj/doc.o \
        $(CONFIG)/obj/docFiles.o \
        $(CONFIG)/obj/listing.o \
        $(CONFIG)/obj/slotGen.o
	$(CC) -o $(CONFIG)/bin/ejsmod -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsmod.o $(CONFIG)/obj/doc.o $(CONFIG)/obj/docFiles.o $(CONFIG)/obj/listing.o $(CONFIG)/obj/slotGen.o $(LIBS) -lejs -lhttp -lpam -lmpr -lpcre

$(CONFIG)/obj/ejsrun.o: \
        src/cmd/ejsrun.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ejsrun.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/cmd/ejsrun.c

$(CONFIG)/bin/ejsrun:  \
        $(CONFIG)/bin/libejs.dylib \
        $(CONFIG)/obj/ejsrun.o
	$(CC) -o $(CONFIG)/bin/ejsrun -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LIBS) -lejs -lhttp -lpam -lmpr -lpcre

$(CONFIG)/bin/ejs.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod
	cd src/core >/dev/null ;\
		../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.mod --debug --optimize 9 --bind --require null *.es  ;\
	../../$(CONFIG)/bin/ejsmod --require null --cslots ../../$(CONFIG)/bin/ejs.mod ;\
	if ! diff ejs.slots.h ../../$(CONFIG)/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ../../$(CONFIG)/inc; fi ;\
	rm -f ejs.slots.h ;\
		cd - >/dev/null 

$(CONFIG)/bin/bit.es: 
	rm -fr $(CONFIG)/bin/bit.es
	cp -r src/jems/ejs.bit/bit.es $(CONFIG)/bin/bit.es

$(CONFIG)/bin/bits: 
	cd src/jems/ejs.bit >/dev/null ;\
		rm -fr ../../../$(CONFIG)/bin/bits ;\
	cp -r bits ../../../$(CONFIG)/bin ;\
		cd - >/dev/null 

$(CONFIG)/obj/ejsZlib.o: \
        src/jems/ejs.zlib/ejsZlib.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsZlib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/jems/ejs.zlib/ejsZlib.c

$(CONFIG)/bin/bit:  \
        $(CONFIG)/bin/libejs.dylib \
        $(CONFIG)/bin/libzlib.dylib \
        $(CONFIG)/bin/bits \
        $(CONFIG)/bin/bit.es \
        $(CONFIG)/obj/ejsrun.o \
        $(CONFIG)/obj/ejsZlib.o
	$(CC) -o $(CONFIG)/bin/bit -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(CONFIG)/obj/ejsZlib.o $(CONFIG)/obj/mprLib.o $(CONFIG)/obj/pcre.o $(CONFIG)/obj/httpLib.o $(CONFIG)/obj/ecAst.o $(CONFIG)/obj/ecCodeGen.o $(CONFIG)/obj/ecCompiler.o $(CONFIG)/obj/ecLex.o $(CONFIG)/obj/ecModuleWrite.o $(CONFIG)/obj/ecParser.o $(CONFIG)/obj/ecState.o $(CONFIG)/obj/dtoa.o $(CONFIG)/obj/ejsApp.o $(CONFIG)/obj/ejsArray.o $(CONFIG)/obj/ejsBlock.o $(CONFIG)/obj/ejsBoolean.o $(CONFIG)/obj/ejsByteArray.o $(CONFIG)/obj/ejsCache.o $(CONFIG)/obj/ejsCmd.o $(CONFIG)/obj/ejsConfig.o $(CONFIG)/obj/ejsDate.o $(CONFIG)/obj/ejsDebug.o $(CONFIG)/obj/ejsError.o $(CONFIG)/obj/ejsFile.o $(CONFIG)/obj/ejsFileSystem.o $(CONFIG)/obj/ejsFrame.o $(CONFIG)/obj/ejsFunction.o $(CONFIG)/obj/ejsGC.o $(CONFIG)/obj/ejsGlobal.o $(CONFIG)/obj/ejsHttp.o $(CONFIG)/obj/ejsIterator.o $(CONFIG)/obj/ejsJSON.o $(CONFIG)/obj/ejsLocalCache.o $(CONFIG)/obj/ejsMath.o $(CONFIG)/obj/ejsMemory.o $(CONFIG)/obj/ejsMprLog.o $(CONFIG)/obj/ejsNamespace.o $(CONFIG)/obj/ejsNull.o $(CONFIG)/obj/ejsNumber.o $(CONFIG)/obj/ejsObject.o $(CONFIG)/obj/ejsPath.o $(CONFIG)/obj/ejsPot.o $(CONFIG)/obj/ejsRegExp.o $(CONFIG)/obj/ejsSocket.o $(CONFIG)/obj/ejsString.o $(CONFIG)/obj/ejsSystem.o $(CONFIG)/obj/ejsTimer.o $(CONFIG)/obj/ejsType.o $(CONFIG)/obj/ejsUri.o $(CONFIG)/obj/ejsVoid.o $(CONFIG)/obj/ejsWorker.o $(CONFIG)/obj/ejsXML.o $(CONFIG)/obj/ejsXMLList.o $(CONFIG)/obj/ejsXMLLoader.o $(CONFIG)/obj/ejsByteCode.o $(CONFIG)/obj/ejsException.o $(CONFIG)/obj/ejsHelper.o $(CONFIG)/obj/ejsInterp.o $(CONFIG)/obj/ejsLoader.o $(CONFIG)/obj/ejsModule.o $(CONFIG)/obj/ejsScope.o $(CONFIG)/obj/ejsService.o $(CONFIG)/obj/adler32.o $(CONFIG)/obj/compress.o $(CONFIG)/obj/crc32.o $(CONFIG)/obj/deflate.o $(CONFIG)/obj/gzclose.o $(CONFIG)/obj/gzlib.o $(CONFIG)/obj/gzread.o $(CONFIG)/obj/gzwrite.o $(CONFIG)/obj/infback.o $(CONFIG)/obj/inffast.o $(CONFIG)/obj/inflate.o $(CONFIG)/obj/inftrees.o $(CONFIG)/obj/trees.o $(CONFIG)/obj/uncompr.o $(CONFIG)/obj/zutil.o $(LIBS) -lpam

$(CONFIG)/bin/utest.es: 
	cd src/jems/ejs.utest >/dev/null ;\
		cp utest.es ../../../$(CONFIG)/bin ;\
		cd - >/dev/null 

$(CONFIG)/bin/utest:  \
        $(CONFIG)/bin/ejsrun
	rm -fr $(CONFIG)/bin/utest
	cp -r $(CONFIG)/bin/ejsrun $(CONFIG)/bin/utest

$(CONFIG)/bin/ejs.unix.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.unix >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.unix.mod --debug --optimize 9 Unix.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/jem.es: 
	cd src/jems/ejs.jem >/dev/null ;\
		cp jem.es ../../../$(CONFIG)/bin ;\
		cd - >/dev/null 

$(CONFIG)/bin/jem:  \
        $(CONFIG)/bin/ejsrun
	rm -fr $(CONFIG)/bin/jem
	cp -r $(CONFIG)/bin/ejsrun $(CONFIG)/bin/jem

$(CONFIG)/bin/ejs.db.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.db >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.mod --debug --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/ejs.db.mapper.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/bin/ejs.db.mod
	cd src/jems/ejs.db.mapper >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.mapper.mod --debug --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/ejs.db.sqlite.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.db.sqlite >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.sqlite.mod --debug --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/obj/ejsSqlite.o: \
        src/jems/ejs.db.sqlite/ejsSqlite.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/sqlite3.h \
        $(CONFIG)/inc/ejs.db.sqlite.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsSqlite.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/jems/ejs.db.sqlite/ejsSqlite.c

$(CONFIG)/bin/ejs.db.sqlite.dylib:  \
        $(CONFIG)/bin/libmpr.dylib \
        $(CONFIG)/bin/libejs.dylib \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/bin/ejs.db.sqlite.mod \
        $(CONFIG)/bin/libsqlite3.dylib \
        $(CONFIG)/obj/ejsSqlite.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/ejs.db.sqlite.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 2.0.1 -current_version 2.0.1 -compatibility_version 2.0.1 -current_version 2.0.1 $(LIBPATHS) -install_name @rpath/ejs.db.sqlite.dylib $(CONFIG)/obj/ejsSqlite.o $(LIBS) -lmpr -lejs -lhttp -lpam -lpcre -lsqlite3

$(CONFIG)/bin/ejs.web.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.web >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.web.mod --debug --optimize 9 *.es ;\
	../../../$(CONFIG)/bin/ejsmod --cslots ../../../$(CONFIG)/bin/ejs.web.mod ;\
	if ! diff ejs.web.slots.h ../../../$(CONFIG)/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../../$(CONFIG)/inc; fi ;\
	rm -f ejs.web.slots.h ;\
		cd - >/dev/null 

$(CONFIG)/obj/ejsHttpServer.o: \
        src/jems/ejs.web/ejsHttpServer.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/ejsCompiler.h \
        $(CONFIG)/inc/ejs.web.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsHttpServer.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsHttpServer.c

$(CONFIG)/obj/ejsRequest.o: \
        src/jems/ejs.web/ejsRequest.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/ejsCompiler.h \
        $(CONFIG)/inc/ejs.web.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsRequest.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsRequest.c

$(CONFIG)/obj/ejsSession.o: \
        src/jems/ejs.web/ejsSession.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsSession.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsSession.c

$(CONFIG)/obj/ejsWeb.o: \
        src/jems/ejs.web/ejsWeb.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/ejsCompiler.h \
        $(CONFIG)/inc/ejs.web.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsWeb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsWeb.c

$(CONFIG)/bin/ejs.web.dylib:  \
        $(CONFIG)/bin/libejs.dylib \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/obj/ejsHttpServer.o \
        $(CONFIG)/obj/ejsRequest.o \
        $(CONFIG)/obj/ejsSession.o \
        $(CONFIG)/obj/ejsWeb.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/ejs.web.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 2.0.1 -current_version 2.0.1 -compatibility_version 2.0.1 -current_version 2.0.1 $(LIBPATHS) -install_name @rpath/ejs.web.dylib $(CONFIG)/obj/ejsHttpServer.o $(CONFIG)/obj/ejsRequest.o $(CONFIG)/obj/ejsSession.o $(CONFIG)/obj/ejsWeb.o $(LIBS) -lejs -lhttp -lpam -lmpr -lpcre

$(CONFIG)/bin/www: 
	cd src/jems/ejs.web >/dev/null ;\
		rm -fr ../../../$(CONFIG)/bin/www ;\
	cp -r www ../../../$(CONFIG)/bin ;\
		cd - >/dev/null 

$(CONFIG)/bin/ejs.template.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.template >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.template.mod --debug --optimize 9 TemplateParser.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/ejs.tar.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.tar >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.tar.mod --debug --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/ejs.zlib.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.zlib >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.zlib.mod --debug --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/ejs.zlib.dylib:  \
        $(CONFIG)/bin/libmpr.dylib \
        $(CONFIG)/bin/libejs.dylib \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/bin/ejs.zlib.mod \
        $(CONFIG)/bin/libzlib.dylib \
        $(CONFIG)/obj/ejsZlib.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/ejs.zlib.dylib -arch x86_64 $(LDFLAGS) -compatibility_version 2.0.1 -current_version 2.0.1 -compatibility_version 2.0.1 -current_version 2.0.1 $(LIBPATHS) -install_name @rpath/ejs.zlib.dylib $(CONFIG)/obj/ejsZlib.o $(LIBS) -lmpr -lejs -lhttp -lpam -lpcre -lzlib

$(CONFIG)/bin/mvc.es: 
	cd src/jems/ejs.mvc >/dev/null ;\
		cp mvc.es ../../../$(CONFIG)/bin ;\
		cd - >/dev/null 

$(CONFIG)/bin/mvc:  \
        $(CONFIG)/bin/ejsrun
	rm -fr $(CONFIG)/bin/mvc
	cp -r $(CONFIG)/bin/ejsrun $(CONFIG)/bin/mvc

$(CONFIG)/bin/ejs.mvc.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/bin/ejs.web.mod \
        $(CONFIG)/bin/ejs.template.mod \
        $(CONFIG)/bin/ejs.unix.mod
	cd src/jems/ejs.mvc >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.mvc.mod --debug --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/utest.worker: 
	cd src/jems/ejs.utest >/dev/null ;\
		cp utest.worker ../../../$(CONFIG)/bin ;\
		cd - >/dev/null 

