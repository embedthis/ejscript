#
#   ejs-solaris-static.mk -- Makefile to build Embedthis Ejscript for solaris
#

PRODUCT         ?= ejs
VERSION         ?= 2.3.0
BUILD_NUMBER    ?= 1
PROFILE         ?= static
ARCH            ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
OS              ?= solaris
CC              ?= /usr/bin/gcc
LD              ?= /usr/bin/ld
CONFIG          ?= $(OS)-$(ARCH)-$(PROFILE)

CFLAGS          += -fPIC -Os -w
DFLAGS          += -D_REENTRANT -DPIC$(patsubst %,-D%,$(filter BIT_%,$(MAKEFLAGS)))
IFLAGS          += -I$(CONFIG)/inc
LDFLAGS         += 
LIBPATHS        += -L$(CONFIG)/bin
LIBS            += -llxnet -lrt -lsocket -lpthread -lm -ldl

DEBUG           ?= release
CFLAGS-debug    := -g
CFLAGS-release  := -O2
DFLAGS-debug    := -DBIT_DEBUG
DFLAGS-release  := 
LDFLAGS-debug   := -g
LDFLAGS-release := 
CFLAGS          += $(CFLAGS-$(PROFILE))
DFLAGS          += $(DFLAGS-$(PROFILE))
LDFLAGS         += $(LDFLAGS-$(PROFILE))

all compile: prep \
        $(CONFIG)/bin/libmpr.a \
        $(CONFIG)/bin/libmprssl.a \
        $(CONFIG)/bin/ejsman \
        $(CONFIG)/bin/libest.a \
        $(CONFIG)/bin/ca.crt \
        $(CONFIG)/bin/libpcre.a \
        $(CONFIG)/bin/libhttp.a \
        $(CONFIG)/bin/http \
        $(CONFIG)/bin/http-ca.crt \
        $(CONFIG)/bin/libsqlite3.a \
        $(CONFIG)/bin/sqlite \
        $(CONFIG)/bin/libzlib.a \
        $(CONFIG)/bin/libejs.a \
        $(CONFIG)/bin/ejs \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/bin/ejsrun \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/bin/ejs.unix.mod \
        $(CONFIG)/bin/jem.es \
        $(CONFIG)/bin/jem \
        $(CONFIG)/bin/ejs.db.mod \
        $(CONFIG)/bin/ejs.db.mapper.mod \
        $(CONFIG)/bin/ejs.db.sqlite.mod \
        $(CONFIG)/bin/libejs.db.sqlite.a \
        $(CONFIG)/bin/ejs.mail.mod \
        $(CONFIG)/bin/ejs.web.mod \
        $(CONFIG)/bin/libejs.web.a \
        $(CONFIG)/bin/www \
        $(CONFIG)/bin/ejs.template.mod \
        $(CONFIG)/bin/ejs.zlib.mod \
        $(CONFIG)/bin/libejs.zlib.a \
        $(CONFIG)/bin/ejs.tar.mod \
        $(CONFIG)/bin/mvc.es \
        $(CONFIG)/bin/mvc \
        $(CONFIG)/bin/ejs.mvc.mod \
        $(CONFIG)/bin/utest.es \
        $(CONFIG)/bin/utest.worker \
        $(CONFIG)/bin/utest

.PHONY: prep

prep:
	@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi
	@[ ! -x $(CONFIG)/inc ] && mkdir -p $(CONFIG)/inc $(CONFIG)/obj $(CONFIG)/lib $(CONFIG)/bin ; true
	@[ ! -f $(CONFIG)/inc/bit.h ] && cp projects/ejs-$(OS)-$(PROFILE)-bit.h $(CONFIG)/inc/bit.h ; true
	@[ ! -f $(CONFIG)/inc/bitos.h ] && cp src/bitos.h $(CONFIG)/inc/bitos.h ; true
	@if ! diff $(CONFIG)/inc/bit.h projects/ejs-$(OS)-$(PROFILE)-bit.h >/dev/null ; then\
		echo cp projects/ejs-$(OS)-$(PROFILE)-bit.h $(CONFIG)/inc/bit.h  ; \
		cp projects/ejs-$(OS)-$(PROFILE)-bit.h $(CONFIG)/inc/bit.h  ; \
	fi; true
	@echo $(DFLAGS) $(CFLAGS) >projects/.flags

clean:
	rm -rf $(CONFIG)/bin/libmpr.a
	rm -rf $(CONFIG)/bin/libmprssl.a
	rm -rf $(CONFIG)/bin/ejsman
	rm -rf $(CONFIG)/bin/libest.a
	rm -rf $(CONFIG)/bin/ca.crt
	rm -rf $(CONFIG)/bin/libpcre.a
	rm -rf $(CONFIG)/bin/libhttp.a
	rm -rf $(CONFIG)/bin/http
	rm -rf $(CONFIG)/bin/http-ca.crt
	rm -rf $(CONFIG)/bin/libsqlite3.a
	rm -rf $(CONFIG)/bin/sqlite
	rm -rf $(CONFIG)/bin/libzlib.a
	rm -rf $(CONFIG)/bin/libejs.a
	rm -rf $(CONFIG)/bin/ejs
	rm -rf $(CONFIG)/bin/ejsc
	rm -rf $(CONFIG)/bin/ejsmod
	rm -rf $(CONFIG)/bin/ejsrun
	rm -rf $(CONFIG)/bin/jem.es
	rm -rf $(CONFIG)/bin/jem
	rm -rf $(CONFIG)/bin/ejs.db.mod
	rm -rf $(CONFIG)/bin/ejs.db.mapper.mod
	rm -rf $(CONFIG)/bin/ejs.db.sqlite.mod
	rm -rf $(CONFIG)/bin/libejs.db.sqlite.a
	rm -rf $(CONFIG)/bin/ejs.mail.mod
	rm -rf $(CONFIG)/bin/ejs.web.mod
	rm -rf $(CONFIG)/bin/libejs.web.a
	rm -rf $(CONFIG)/bin/www
	rm -rf $(CONFIG)/bin/ejs.template.mod
	rm -rf $(CONFIG)/bin/libejs.zlib.a
	rm -rf $(CONFIG)/bin/mvc.es
	rm -rf $(CONFIG)/bin/ejs.mvc.mod
	rm -rf $(CONFIG)/bin/utest.es
	rm -rf $(CONFIG)/bin/utest.worker
	rm -rf $(CONFIG)/bin/utest
	rm -rf $(CONFIG)/obj/mprLib.o
	rm -rf $(CONFIG)/obj/mprSsl.o
	rm -rf $(CONFIG)/obj/manager.o
	rm -rf $(CONFIG)/obj/makerom.o
	rm -rf $(CONFIG)/obj/estLib.o
	rm -rf $(CONFIG)/obj/pcre.o
	rm -rf $(CONFIG)/obj/httpLib.o
	rm -rf $(CONFIG)/obj/http.o
	rm -rf $(CONFIG)/obj/sqlite3.o
	rm -rf $(CONFIG)/obj/sqlite.o
	rm -rf $(CONFIG)/obj/zlib.o
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
	rm -rf $(CONFIG)/obj/ejsWebSocket.o
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
	rm -rf $(CONFIG)/obj/ejsSqlite.o
	rm -rf $(CONFIG)/obj/ejsHttpServer.o
	rm -rf $(CONFIG)/obj/ejsRequest.o
	rm -rf $(CONFIG)/obj/ejsSession.o
	rm -rf $(CONFIG)/obj/ejsWeb.o
	rm -rf $(CONFIG)/obj/ejsZlib.o
	rm -rf $(CONFIG)/obj/removeFiles.o

clobber: clean
	rm -fr ./$(CONFIG)

$(CONFIG)/inc/bitos.h: 
	rm -fr $(CONFIG)/inc/bitos.h
	cp -r src/bitos.h $(CONFIG)/inc/bitos.h

$(CONFIG)/inc/mpr.h:  \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/bitos.h
	rm -fr $(CONFIG)/inc/mpr.h
	cp -r src/deps/mpr/mpr.h $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/mprLib.o: \
        src/deps/mpr/mprLib.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/mpr.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/mprLib.c

$(CONFIG)/bin/libmpr.a:  \
        $(CONFIG)/inc/mpr.h \
        $(CONFIG)/obj/mprLib.o
	$(LDFLAGS)$(LDFLAGS)/usr/bin/ar -cr $(CONFIG)/bin/libmpr.a $(CONFIG)/obj/mprLib.o

$(CONFIG)/inc/est.h:  \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/bitos.h
	rm -fr $(CONFIG)/inc/est.h
	cp -r src/deps/est/est.h $(CONFIG)/inc/est.h

$(CONFIG)/obj/estLib.o: \
        src/deps/est/estLib.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/est.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/estLib.o -fPIC -Os $(DFLAGS) -I$(CONFIG)/inc src/deps/est/estLib.c

$(CONFIG)/bin/libest.a:  \
        $(CONFIG)/inc/est.h \
        $(CONFIG)/obj/estLib.o
	$(LDFLAGS)$(LDFLAGS)/usr/bin/ar -cr $(CONFIG)/bin/libest.a $(CONFIG)/obj/estLib.o

$(CONFIG)/obj/mprSsl.o: \
        src/deps/mpr/mprSsl.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/mpr.h \
        $(CONFIG)/inc/est.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/mprSsl.c

$(CONFIG)/bin/libmprssl.a:  \
        $(CONFIG)/bin/libmpr.a \
        $(CONFIG)/bin/libest.a \
        $(CONFIG)/obj/mprSsl.o
	$(LDFLAGS)$(LDFLAGS)/usr/bin/ar -cr $(CONFIG)/bin/libmprssl.a $(CONFIG)/obj/mprSsl.o

$(CONFIG)/obj/manager.o: \
        src/deps/mpr/manager.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/mpr.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/manager.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/manager.c

$(CONFIG)/bin/ejsman:  \
        $(CONFIG)/bin/libmpr.a \
        $(CONFIG)/obj/manager.o
	$(LDFLAGS)$(LDFLAGS)$(CC) -o $(CONFIG)/bin/ejsman $(LIBPATHS) $(CONFIG)/obj/manager.o -lmpr $(LIBS) -lmpr -llxnet -lrt -lsocket -lpthread -lm -ldl 

$(CONFIG)/bin/ca.crt: 
	rm -fr $(CONFIG)/bin/ca.crt
	cp -r src/deps/est/ca.crt $(CONFIG)/bin/ca.crt

$(CONFIG)/inc/pcre.h:  \
        $(CONFIG)/inc/bit.h
	rm -fr $(CONFIG)/inc/pcre.h
	cp -r src/deps/pcre/pcre.h $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
        src/deps/pcre/pcre.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/pcre.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/pcre/pcre.c

$(CONFIG)/bin/libpcre.a:  \
        $(CONFIG)/inc/pcre.h \
        $(CONFIG)/obj/pcre.o
	$(LDFLAGS)$(LDFLAGS)/usr/bin/ar -cr $(CONFIG)/bin/libpcre.a $(CONFIG)/obj/pcre.o

$(CONFIG)/inc/http.h:  \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/mpr.h
	rm -fr $(CONFIG)/inc/http.h
	cp -r src/deps/http/http.h $(CONFIG)/inc/http.h

$(CONFIG)/obj/httpLib.o: \
        src/deps/http/httpLib.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/http.h \
        $(CONFIG)/inc/pcre.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/http/httpLib.c

$(CONFIG)/bin/libhttp.a:  \
        $(CONFIG)/bin/libmpr.a \
        $(CONFIG)/bin/libpcre.a \
        $(CONFIG)/inc/http.h \
        $(CONFIG)/obj/httpLib.o
	$(LDFLAGS)$(LDFLAGS)/usr/bin/ar -cr $(CONFIG)/bin/libhttp.a $(CONFIG)/obj/httpLib.o

$(CONFIG)/obj/http.o: \
        src/deps/http/http.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/http.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/http/http.c

$(CONFIG)/bin/http:  \
        $(CONFIG)/bin/libhttp.a \
        $(CONFIG)/obj/http.o
	$(LDFLAGS)$(LDFLAGS)$(CC) -o $(CONFIG)/bin/http $(LIBPATHS) $(CONFIG)/obj/http.o -lhttp $(LIBS) -lpcre -lmpr -lhttp -llxnet -lrt -lsocket -lpthread -lm -ldl -lpcre -lmpr 

$(CONFIG)/bin/http-ca.crt: 
	rm -fr $(CONFIG)/bin/http-ca.crt
	cp -r src/deps/http/http-ca.crt $(CONFIG)/bin/http-ca.crt

$(CONFIG)/inc/sqlite3.h:  \
        $(CONFIG)/inc/bit.h
	rm -fr $(CONFIG)/inc/sqlite3.h
	cp -r src/deps/sqlite/sqlite3.h $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite3.o: \
        src/deps/sqlite/sqlite3.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/sqlite3.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/sqlite3.o -fPIC -Os -w $(DFLAGS) -I$(CONFIG)/inc src/deps/sqlite/sqlite3.c

$(CONFIG)/bin/libsqlite3.a:  \
        $(CONFIG)/inc/sqlite3.h \
        $(CONFIG)/obj/sqlite3.o
	$(LDFLAGS)$(LDFLAGS)/usr/bin/ar -cr $(CONFIG)/bin/libsqlite3.a $(CONFIG)/obj/sqlite3.o

$(CONFIG)/obj/sqlite.o: \
        src/deps/sqlite/sqlite.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/sqlite3.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/sqlite.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/sqlite/sqlite.c

$(CONFIG)/bin/sqlite:  \
        $(CONFIG)/bin/libsqlite3.a \
        $(CONFIG)/obj/sqlite.o
	$(LDFLAGS)$(LDFLAGS)$(CC) -o $(CONFIG)/bin/sqlite $(LIBPATHS) $(CONFIG)/obj/sqlite.o -lsqlite3 $(LIBS) -lsqlite3 -llxnet -lrt -lsocket -lpthread -lm -ldl 

$(CONFIG)/inc/zlib.h:  \
        $(CONFIG)/inc/bit.h
	rm -fr $(CONFIG)/inc/zlib.h
	cp -r src/deps/zlib/zlib.h $(CONFIG)/inc/zlib.h

$(CONFIG)/obj/zlib.o: \
        src/deps/zlib/zlib.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/zlib.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/zlib.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/zlib/zlib.c

$(CONFIG)/bin/libzlib.a:  \
        $(CONFIG)/inc/zlib.h \
        $(CONFIG)/obj/zlib.o
	$(LDFLAGS)$(LDFLAGS)/usr/bin/ar -cr $(CONFIG)/bin/libzlib.a $(CONFIG)/obj/zlib.o

$(CONFIG)/inc/ejs.cache.local.slots.h: 
	rm -fr $(CONFIG)/inc/ejs.cache.local.slots.h
	cp -r src/slots/ejs.cache.local.slots.h $(CONFIG)/inc/ejs.cache.local.slots.h

$(CONFIG)/inc/ejs.db.sqlite.slots.h:  \
        $(CONFIG)/inc/bit.h
	rm -fr $(CONFIG)/inc/ejs.db.sqlite.slots.h
	cp -r src/slots/ejs.db.sqlite.slots.h $(CONFIG)/inc/ejs.db.sqlite.slots.h

$(CONFIG)/inc/ejs.slots.h:  \
        $(CONFIG)/inc/bit.h
	rm -fr $(CONFIG)/inc/ejs.slots.h
	cp -r src/slots/ejs.slots.h $(CONFIG)/inc/ejs.slots.h

$(CONFIG)/inc/ejs.web.slots.h:  \
        $(CONFIG)/inc/bit.h
	rm -fr $(CONFIG)/inc/ejs.web.slots.h
	cp -r src/slots/ejs.web.slots.h $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/inc/ejs.zlib.slots.h:  \
        $(CONFIG)/inc/bit.h
	rm -fr $(CONFIG)/inc/ejs.zlib.slots.h
	cp -r src/slots/ejs.zlib.slots.h $(CONFIG)/inc/ejs.zlib.slots.h

$(CONFIG)/inc/ejsByteCode.h:  \
        $(CONFIG)/inc/bit.h
	rm -fr $(CONFIG)/inc/ejsByteCode.h
	cp -r src/ejsByteCode.h $(CONFIG)/inc/ejsByteCode.h

$(CONFIG)/inc/ejsByteCodeTable.h:  \
        $(CONFIG)/inc/bit.h
	rm -fr $(CONFIG)/inc/ejsByteCodeTable.h
	cp -r src/ejsByteCodeTable.h $(CONFIG)/inc/ejsByteCodeTable.h

$(CONFIG)/inc/ejsCustomize.h:  \
        $(CONFIG)/inc/bit.h
	rm -fr $(CONFIG)/inc/ejsCustomize.h
	cp -r src/ejsCustomize.h $(CONFIG)/inc/ejsCustomize.h

$(CONFIG)/inc/ejs.h:  \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/mpr.h \
        $(CONFIG)/inc/http.h \
        $(CONFIG)/inc/ejsByteCode.h \
        $(CONFIG)/inc/ejsByteCodeTable.h \
        $(CONFIG)/inc/ejs.slots.h \
        $(CONFIG)/inc/ejsCustomize.h
	rm -fr $(CONFIG)/inc/ejs.h
	cp -r src/ejs.h $(CONFIG)/inc/ejs.h

$(CONFIG)/inc/ejsCompiler.h:  \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	rm -fr $(CONFIG)/inc/ejsCompiler.h
	cp -r src/ejsCompiler.h $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecAst.o: \
        src/compiler/ecAst.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ecAst.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecAst.c

$(CONFIG)/obj/ecCodeGen.o: \
        src/compiler/ecCodeGen.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ecCodeGen.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecCodeGen.c

$(CONFIG)/obj/ecCompiler.o: \
        src/compiler/ecCompiler.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ecCompiler.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecCompiler.c

$(CONFIG)/obj/ecLex.o: \
        src/compiler/ecLex.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ecLex.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecLex.c

$(CONFIG)/obj/ecModuleWrite.o: \
        src/compiler/ecModuleWrite.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ecModuleWrite.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecModuleWrite.c

$(CONFIG)/obj/ecParser.o: \
        src/compiler/ecParser.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ecParser.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecParser.c

$(CONFIG)/obj/ecState.o: \
        src/compiler/ecState.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ecState.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/compiler/ecState.c

$(CONFIG)/obj/dtoa.o: \
        src/core/src/dtoa.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/mpr.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/dtoa.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/dtoa.c

$(CONFIG)/obj/ejsApp.o: \
        src/core/src/ejsApp.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsApp.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsApp.c

$(CONFIG)/obj/ejsArray.o: \
        src/core/src/ejsArray.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsArray.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsArray.c

$(CONFIG)/obj/ejsBlock.o: \
        src/core/src/ejsBlock.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsBlock.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsBlock.c

$(CONFIG)/obj/ejsBoolean.o: \
        src/core/src/ejsBoolean.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsBoolean.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsBoolean.c

$(CONFIG)/obj/ejsByteArray.o: \
        src/core/src/ejsByteArray.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsByteArray.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsByteArray.c

$(CONFIG)/obj/ejsCache.o: \
        src/core/src/ejsCache.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsCache.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsCache.c

$(CONFIG)/obj/ejsCmd.o: \
        src/core/src/ejsCmd.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsCmd.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsCmd.c

$(CONFIG)/obj/ejsConfig.o: \
        src/core/src/ejsConfig.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsConfig.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsConfig.c

$(CONFIG)/obj/ejsDate.o: \
        src/core/src/ejsDate.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsDate.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsDate.c

$(CONFIG)/obj/ejsDebug.o: \
        src/core/src/ejsDebug.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsDebug.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsDebug.c

$(CONFIG)/obj/ejsError.o: \
        src/core/src/ejsError.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsError.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsError.c

$(CONFIG)/obj/ejsFile.o: \
        src/core/src/ejsFile.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsFile.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsFile.c

$(CONFIG)/obj/ejsFileSystem.o: \
        src/core/src/ejsFileSystem.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsFileSystem.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsFileSystem.c

$(CONFIG)/obj/ejsFrame.o: \
        src/core/src/ejsFrame.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsFrame.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsFrame.c

$(CONFIG)/obj/ejsFunction.o: \
        src/core/src/ejsFunction.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsFunction.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsFunction.c

$(CONFIG)/obj/ejsGC.o: \
        src/core/src/ejsGC.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsGC.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsGC.c

$(CONFIG)/obj/ejsGlobal.o: \
        src/core/src/ejsGlobal.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsGlobal.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsGlobal.c

$(CONFIG)/obj/ejsHttp.o: \
        src/core/src/ejsHttp.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsHttp.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsHttp.c

$(CONFIG)/obj/ejsIterator.o: \
        src/core/src/ejsIterator.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsIterator.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsIterator.c

$(CONFIG)/obj/ejsJSON.o: \
        src/core/src/ejsJSON.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsJSON.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsJSON.c

$(CONFIG)/obj/ejsLocalCache.o: \
        src/core/src/ejsLocalCache.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsLocalCache.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsLocalCache.c

$(CONFIG)/obj/ejsMath.o: \
        src/core/src/ejsMath.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsMath.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsMath.c

$(CONFIG)/obj/ejsMemory.o: \
        src/core/src/ejsMemory.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsMemory.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsMemory.c

$(CONFIG)/obj/ejsMprLog.o: \
        src/core/src/ejsMprLog.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsMprLog.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsMprLog.c

$(CONFIG)/obj/ejsNamespace.o: \
        src/core/src/ejsNamespace.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsNamespace.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsNamespace.c

$(CONFIG)/obj/ejsNull.o: \
        src/core/src/ejsNull.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsNull.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsNull.c

$(CONFIG)/obj/ejsNumber.o: \
        src/core/src/ejsNumber.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsNumber.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsNumber.c

$(CONFIG)/obj/ejsObject.o: \
        src/core/src/ejsObject.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsObject.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsObject.c

$(CONFIG)/obj/ejsPath.o: \
        src/core/src/ejsPath.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/pcre.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsPath.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsPath.c

$(CONFIG)/obj/ejsPot.o: \
        src/core/src/ejsPot.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsPot.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsPot.c

$(CONFIG)/obj/ejsRegExp.o: \
        src/core/src/ejsRegExp.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/pcre.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsRegExp.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsRegExp.c

$(CONFIG)/obj/ejsSocket.o: \
        src/core/src/ejsSocket.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsSocket.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsSocket.c

$(CONFIG)/obj/ejsString.o: \
        src/core/src/ejsString.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/pcre.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsString.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsString.c

$(CONFIG)/obj/ejsSystem.o: \
        src/core/src/ejsSystem.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsSystem.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsSystem.c

$(CONFIG)/obj/ejsTimer.o: \
        src/core/src/ejsTimer.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsTimer.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsTimer.c

$(CONFIG)/obj/ejsType.o: \
        src/core/src/ejsType.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsType.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsType.c

$(CONFIG)/obj/ejsUri.o: \
        src/core/src/ejsUri.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsUri.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsUri.c

$(CONFIG)/obj/ejsVoid.o: \
        src/core/src/ejsVoid.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsVoid.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsVoid.c

$(CONFIG)/obj/ejsWebSocket.o: \
        src/core/src/ejsWebSocket.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsWebSocket.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsWebSocket.c

$(CONFIG)/obj/ejsWorker.o: \
        src/core/src/ejsWorker.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsWorker.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsWorker.c

$(CONFIG)/obj/ejsXML.o: \
        src/core/src/ejsXML.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsXML.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsXML.c

$(CONFIG)/obj/ejsXMLList.o: \
        src/core/src/ejsXMLList.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsXMLList.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsXMLList.c

$(CONFIG)/obj/ejsXMLLoader.o: \
        src/core/src/ejsXMLLoader.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsXMLLoader.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/core/src/ejsXMLLoader.c

$(CONFIG)/obj/ejsByteCode.o: \
        src/vm/ejsByteCode.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsByteCode.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsByteCode.c

$(CONFIG)/obj/ejsException.o: \
        src/vm/ejsException.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsException.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsException.c

$(CONFIG)/obj/ejsHelper.o: \
        src/vm/ejsHelper.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsHelper.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsHelper.c

$(CONFIG)/obj/ejsInterp.o: \
        src/vm/ejsInterp.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsInterp.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsInterp.c

$(CONFIG)/obj/ejsLoader.o: \
        src/vm/ejsLoader.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsLoader.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsLoader.c

$(CONFIG)/obj/ejsModule.o: \
        src/vm/ejsModule.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsModule.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsModule.c

$(CONFIG)/obj/ejsScope.o: \
        src/vm/ejsScope.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsScope.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsScope.c

$(CONFIG)/obj/ejsService.o: \
        src/vm/ejsService.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsService.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/vm/ejsService.c

$(CONFIG)/bin/libejs.a:  \
        $(CONFIG)/bin/libhttp.a \
        $(CONFIG)/inc/ejs.cache.local.slots.h \
        $(CONFIG)/inc/ejs.db.sqlite.slots.h \
        $(CONFIG)/inc/ejs.slots.h \
        $(CONFIG)/inc/ejs.web.slots.h \
        $(CONFIG)/inc/ejs.zlib.slots.h \
        $(CONFIG)/inc/bitos.h \
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
        $(CONFIG)/obj/ejsWebSocket.o \
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
	$(LDFLAGS)$(LDFLAGS)/usr/bin/ar -cr $(CONFIG)/bin/libejs.a $(CONFIG)/obj/ecAst.o $(CONFIG)/obj/ecCodeGen.o $(CONFIG)/obj/ecCompiler.o $(CONFIG)/obj/ecLex.o $(CONFIG)/obj/ecModuleWrite.o $(CONFIG)/obj/ecParser.o $(CONFIG)/obj/ecState.o $(CONFIG)/obj/dtoa.o $(CONFIG)/obj/ejsApp.o $(CONFIG)/obj/ejsArray.o $(CONFIG)/obj/ejsBlock.o $(CONFIG)/obj/ejsBoolean.o $(CONFIG)/obj/ejsByteArray.o $(CONFIG)/obj/ejsCache.o $(CONFIG)/obj/ejsCmd.o $(CONFIG)/obj/ejsConfig.o $(CONFIG)/obj/ejsDate.o $(CONFIG)/obj/ejsDebug.o $(CONFIG)/obj/ejsError.o $(CONFIG)/obj/ejsFile.o $(CONFIG)/obj/ejsFileSystem.o $(CONFIG)/obj/ejsFrame.o $(CONFIG)/obj/ejsFunction.o $(CONFIG)/obj/ejsGC.o $(CONFIG)/obj/ejsGlobal.o $(CONFIG)/obj/ejsHttp.o $(CONFIG)/obj/ejsIterator.o $(CONFIG)/obj/ejsJSON.o $(CONFIG)/obj/ejsLocalCache.o $(CONFIG)/obj/ejsMath.o $(CONFIG)/obj/ejsMemory.o $(CONFIG)/obj/ejsMprLog.o $(CONFIG)/obj/ejsNamespace.o $(CONFIG)/obj/ejsNull.o $(CONFIG)/obj/ejsNumber.o $(CONFIG)/obj/ejsObject.o $(CONFIG)/obj/ejsPath.o $(CONFIG)/obj/ejsPot.o $(CONFIG)/obj/ejsRegExp.o $(CONFIG)/obj/ejsSocket.o $(CONFIG)/obj/ejsString.o $(CONFIG)/obj/ejsSystem.o $(CONFIG)/obj/ejsTimer.o $(CONFIG)/obj/ejsType.o $(CONFIG)/obj/ejsUri.o $(CONFIG)/obj/ejsVoid.o $(CONFIG)/obj/ejsWebSocket.o $(CONFIG)/obj/ejsWorker.o $(CONFIG)/obj/ejsXML.o $(CONFIG)/obj/ejsXMLList.o $(CONFIG)/obj/ejsXMLLoader.o $(CONFIG)/obj/ejsByteCode.o $(CONFIG)/obj/ejsException.o $(CONFIG)/obj/ejsHelper.o $(CONFIG)/obj/ejsInterp.o $(CONFIG)/obj/ejsLoader.o $(CONFIG)/obj/ejsModule.o $(CONFIG)/obj/ejsScope.o $(CONFIG)/obj/ejsService.o

$(CONFIG)/obj/ejs.o: \
        src/cmd/ejs.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/cmd/ejs.c

$(CONFIG)/bin/ejs:  \
        $(CONFIG)/bin/libejs.a \
        $(CONFIG)/obj/ejs.o
	$(LDFLAGS)$(LDFLAGS)$(CC) -o $(CONFIG)/bin/ejs $(LIBPATHS) $(CONFIG)/obj/ejs.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

$(CONFIG)/obj/ejsc.o: \
        src/cmd/ejsc.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/cmd/ejsc.c

$(CONFIG)/bin/ejsc:  \
        $(CONFIG)/bin/libejs.a \
        $(CONFIG)/obj/ejsc.o
	$(LDFLAGS)$(LDFLAGS)$(CC) -o $(CONFIG)/bin/ejsc $(LIBPATHS) $(CONFIG)/obj/ejsc.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

$(CONFIG)/obj/ejsmod.o: \
        src/cmd/ejsmod.c \
        $(CONFIG)/inc/bit.h \
        src/cmd/ejsmod.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsmod.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/cmd src/cmd/ejsmod.c

$(CONFIG)/obj/doc.o: \
        src/cmd/doc.c \
        $(CONFIG)/inc/bit.h \
        src/cmd/ejsmod.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/doc.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/cmd src/cmd/doc.c

$(CONFIG)/obj/docFiles.o: \
        src/cmd/docFiles.c \
        $(CONFIG)/inc/bit.h \
        src/cmd/ejsmod.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/docFiles.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/cmd src/cmd/docFiles.c

$(CONFIG)/obj/listing.o: \
        src/cmd/listing.c \
        $(CONFIG)/inc/bit.h \
        src/cmd/ejsmod.h \
        $(CONFIG)/inc/ejsByteCodeTable.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/listing.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/cmd src/cmd/listing.c

$(CONFIG)/obj/slotGen.o: \
        src/cmd/slotGen.c \
        $(CONFIG)/inc/bit.h \
        src/cmd/ejsmod.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/slotGen.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/cmd src/cmd/slotGen.c

$(CONFIG)/bin/ejsmod:  \
        $(CONFIG)/bin/libejs.a \
        $(CONFIG)/obj/ejsmod.o \
        $(CONFIG)/obj/doc.o \
        $(CONFIG)/obj/docFiles.o \
        $(CONFIG)/obj/listing.o \
        $(CONFIG)/obj/slotGen.o
	$(LDFLAGS)$(LDFLAGS)$(CC) -o $(CONFIG)/bin/ejsmod $(LIBPATHS) $(CONFIG)/obj/ejsmod.o $(CONFIG)/obj/doc.o $(CONFIG)/obj/docFiles.o $(CONFIG)/obj/listing.o $(CONFIG)/obj/slotGen.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

$(CONFIG)/obj/ejsrun.o: \
        src/cmd/ejsrun.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejsCompiler.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsrun.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/cmd/ejsrun.c

$(CONFIG)/bin/ejsrun:  \
        $(CONFIG)/bin/libejs.a \
        $(CONFIG)/obj/ejsrun.o
	$(LDFLAGS)$(LDFLAGS)$(CC) -o $(CONFIG)/bin/ejsrun $(LIBPATHS) $(CONFIG)/obj/ejsrun.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

$(CONFIG)/bin/ejs.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod
	cd src/core >/dev/null ;\
		../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.mod  --optimize 9 --bind --require null *.es  ;\
	../../$(CONFIG)/bin/ejsmod --require null --cslots ../../$(CONFIG)/bin/ejs.mod ;\
	if ! diff ejs.slots.h ../../$(CONFIG)/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ../../$(CONFIG)/inc; fi ;\
	rm -f ejs.slots.h ;\
		cd - >/dev/null 

$(CONFIG)/bin/ejs.unix.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.unix >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.unix.mod  --optimize 9 Unix.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/jem.es: 
	cd src/jems/ejs.jem >/dev/null ;\
		cp jem.es ../../../$(CONFIG)/bin ;\
		cd - >/dev/null 

$(CONFIG)/bin/jem:  \
        $(CONFIG)/bin/libejs.a \
        $(CONFIG)/bin/jem.es \
        $(CONFIG)/obj/ejsrun.o
	$(LDFLAGS)$(LDFLAGS)$(CC) -o $(CONFIG)/bin/jem $(LIBPATHS) $(CONFIG)/obj/ejsrun.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

$(CONFIG)/bin/ejs.db.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.db >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.mod  --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/ejs.db.mapper.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/bin/ejs.db.mod
	cd src/jems/ejs.db.mapper >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.mapper.mod  --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/ejs.db.sqlite.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.db.sqlite >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.sqlite.mod  --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/obj/ejsSqlite.o: \
        src/jems/ejs.db.sqlite/ejsSqlite.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/sqlite3.h \
        $(CONFIG)/inc/ejs.db.sqlite.slots.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsSqlite.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/jems/ejs.db.sqlite/ejsSqlite.c

$(CONFIG)/bin/libejs.db.sqlite.a:  \
        $(CONFIG)/bin/libmpr.a \
        $(CONFIG)/bin/libejs.a \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/bin/ejs.db.sqlite.mod \
        $(CONFIG)/bin/libsqlite3.a \
        $(CONFIG)/obj/ejsSqlite.o
	$(LDFLAGS)$(LDFLAGS)/usr/bin/ar -cr $(CONFIG)/bin/libejs.db.sqlite.a $(CONFIG)/obj/ejsSqlite.o

$(CONFIG)/bin/ejs.mail.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.mail >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.mail.mod  --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/ejs.web.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejsmod \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.web >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.web.mod  --optimize 9 *.es ;\
	../../../$(CONFIG)/bin/ejsmod --cslots ../../../$(CONFIG)/bin/ejs.web.mod ;\
	if ! diff ejs.web.slots.h ../../../$(CONFIG)/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../../$(CONFIG)/inc; fi ;\
	rm -f ejs.web.slots.h ;\
		cd - >/dev/null 

$(CONFIG)/inc/ejsWeb.h:  \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/http.h \
        $(CONFIG)/inc/ejs.web.slots.h
	rm -fr $(CONFIG)/inc/ejsWeb.h
	cp -r src/jems/ejs.web/ejsWeb.h $(CONFIG)/inc/ejsWeb.h

$(CONFIG)/obj/ejsHttpServer.o: \
        src/jems/ejs.web/ejsHttpServer.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/ejsCompiler.h \
        $(CONFIG)/inc/ejsWeb.h \
        $(CONFIG)/inc/ejs.web.slots.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsHttpServer.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsHttpServer.c

$(CONFIG)/obj/ejsRequest.o: \
        src/jems/ejs.web/ejsRequest.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/ejsCompiler.h \
        $(CONFIG)/inc/ejsWeb.h \
        $(CONFIG)/inc/ejs.web.slots.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsRequest.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsRequest.c

$(CONFIG)/obj/ejsSession.o: \
        src/jems/ejs.web/ejsSession.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/ejsWeb.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsSession.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsSession.c

$(CONFIG)/obj/ejsWeb.o: \
        src/jems/ejs.web/ejsWeb.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/ejsCompiler.h \
        $(CONFIG)/inc/ejsWeb.h \
        $(CONFIG)/inc/ejs.web.slots.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsWeb.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsWeb.c

$(CONFIG)/bin/libejs.web.a:  \
        $(CONFIG)/bin/libejs.a \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/inc/ejsWeb.h \
        $(CONFIG)/obj/ejsHttpServer.o \
        $(CONFIG)/obj/ejsRequest.o \
        $(CONFIG)/obj/ejsSession.o \
        $(CONFIG)/obj/ejsWeb.o
	$(LDFLAGS)$(LDFLAGS)/usr/bin/ar -cr $(CONFIG)/bin/libejs.web.a $(CONFIG)/obj/ejsHttpServer.o $(CONFIG)/obj/ejsRequest.o $(CONFIG)/obj/ejsSession.o $(CONFIG)/obj/ejsWeb.o

$(CONFIG)/bin/www: 
	cd src/jems/ejs.web >/dev/null ;\
		rm -fr ../../../$(CONFIG)/bin/www ;\
	cp -r www ../../../$(CONFIG)/bin ;\
		cd - >/dev/null 

$(CONFIG)/bin/ejs.template.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.template >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.template.mod  --optimize 9 TemplateParser.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/ejs.zlib.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.zlib >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.zlib.mod  --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/obj/ejsZlib.o: \
        src/jems/ejs.zlib/ejsZlib.c \
        $(CONFIG)/inc/bit.h \
        $(CONFIG)/inc/ejs.h \
        $(CONFIG)/inc/zlib.h \
        $(CONFIG)/inc/ejs.zlib.slots.h
	$(LDFLAGS)$(LDFLAGS)$(CC) -c -o $(CONFIG)/obj/ejsZlib.o $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/jems/ejs.zlib/ejsZlib.c

$(CONFIG)/bin/libejs.zlib.a:  \
        $(CONFIG)/bin/libejs.a \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/bin/ejs.zlib.mod \
        $(CONFIG)/bin/libzlib.a \
        $(CONFIG)/obj/ejsZlib.o
	$(LDFLAGS)$(LDFLAGS)/usr/bin/ar -cr $(CONFIG)/bin/libejs.zlib.a $(CONFIG)/obj/ejsZlib.o

$(CONFIG)/bin/ejs.tar.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.tar >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.tar.mod  --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/mvc.es: 
	cd src/jems/ejs.mvc >/dev/null ;\
		cp mvc.es ../../../$(CONFIG)/bin ;\
		cd - >/dev/null 

$(CONFIG)/bin/mvc:  \
        $(CONFIG)/bin/libejs.a \
        $(CONFIG)/bin/mvc.es \
        $(CONFIG)/obj/ejsrun.o
	$(LDFLAGS)$(LDFLAGS)$(CC) -o $(CONFIG)/bin/mvc $(LIBPATHS) $(CONFIG)/obj/ejsrun.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

$(CONFIG)/bin/ejs.mvc.mod:  \
        $(CONFIG)/bin/ejsc \
        $(CONFIG)/bin/ejs.mod \
        $(CONFIG)/bin/ejs.web.mod \
        $(CONFIG)/bin/ejs.template.mod \
        $(CONFIG)/bin/ejs.unix.mod
	cd src/jems/ejs.mvc >/dev/null ;\
		../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.mvc.mod  --optimize 9 *.es ;\
		cd - >/dev/null 

$(CONFIG)/bin/utest.es: 
	cd src/jems/ejs.utest >/dev/null ;\
		cp utest.es ../../../$(CONFIG)/bin ;\
		cd - >/dev/null 

$(CONFIG)/bin/utest.worker: 
	cd src/jems/ejs.utest >/dev/null ;\
		cp utest.worker ../../../$(CONFIG)/bin ;\
		cd - >/dev/null 

$(CONFIG)/bin/utest:  \
        $(CONFIG)/bin/libejs.a \
        $(CONFIG)/bin/utest.es \
        $(CONFIG)/bin/utest.worker \
        $(CONFIG)/obj/ejsrun.o
	$(LDFLAGS)$(LDFLAGS)$(CC) -o $(CONFIG)/bin/utest $(LIBPATHS) $(CONFIG)/obj/ejsrun.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lhttp -lpcre -lmpr 

version: 
	@echo 2.3.0-1 

