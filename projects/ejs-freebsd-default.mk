#
#   ejs-freebsd-default.mk -- Makefile to build Embedthis Ejscript for freebsd
#

PRODUCT         := ejs
VERSION         := 2.3.0
BUILD_NUMBER    := 1
PROFILE         := default
ARCH            := $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
OS              := freebsd
CC              := /usr/bin/gcc
LD              := /usr/bin/ld
CONFIG          := $(OS)-$(ARCH)-$(PROFILE)
LBIN            := $(CONFIG)/bin

BIT_ROOT_PREFIX       := /
BIT_BASE_PREFIX       := $(BIT_ROOT_PREFIX)/usr/local
BIT_DATA_PREFIX       := $(BIT_ROOT_PREFIX)/
BIT_STATE_PREFIX      := $(BIT_ROOT_PREFIX)/var
BIT_APP_PREFIX        := $(BIT_BASE_PREFIX)/lib/$(PRODUCT)
BIT_VAPP_PREFIX       := $(BIT_APP_PREFIX)/$(VERSION)
BIT_BIN_PREFIX        := $(BIT_ROOT_PREFIX)/usr/local/bin
BIT_INC_PREFIX        := $(BIT_ROOT_PREFIX)/usr/local/include
BIT_LIB_PREFIX        := $(BIT_ROOT_PREFIX)/usr/local/lib
BIT_MAN_PREFIX        := $(BIT_ROOT_PREFIX)/usr/local/share/man
BIT_SBIN_PREFIX       := $(BIT_ROOT_PREFIX)/usr/local/sbin
BIT_ETC_PREFIX        := $(BIT_ROOT_PREFIX)/etc/$(PRODUCT)
BIT_WEB_PREFIX        := $(BIT_ROOT_PREFIX)/var/www/$(PRODUCT)-default
BIT_LOG_PREFIX        := $(BIT_ROOT_PREFIX)/var/log/$(PRODUCT)
BIT_SPOOL_PREFIX      := $(BIT_ROOT_PREFIX)/var/spool/$(PRODUCT)
BIT_CACHE_PREFIX      := $(BIT_ROOT_PREFIX)/var/spool/$(PRODUCT)/cache
BIT_SRC_PREFIX        := $(BIT_ROOT_PREFIX)$(PRODUCT)-$(VERSION)

CFLAGS          += -fPIC  -w
DFLAGS          += -D_REENTRANT -DPIC  $(patsubst %,-D%,$(filter BIT_%,$(MAKEFLAGS)))
IFLAGS          += -I$(CONFIG)/inc
LDFLAGS         += '-g'
LIBPATHS        += -L$(CONFIG)/bin
LIBS            += -lpthread -lm -ldl

DEBUG           := debug
CFLAGS-debug    := -g
DFLAGS-debug    := -DBIT_DEBUG
LDFLAGS-debug   := -g
DFLAGS-release  := 
CFLAGS-release  := -O2
LDFLAGS-release := 
CFLAGS          += $(CFLAGS-$(DEBUG))
DFLAGS          += $(DFLAGS-$(DEBUG))
LDFLAGS         += $(LDFLAGS-$(DEBUG))

unexport CDPATH

all compile: prep \
        $(CONFIG)/bin/libmpr.so \
        $(CONFIG)/bin/libmprssl.so \
        $(CONFIG)/bin/ejsman \
        $(CONFIG)/bin/makerom \
        $(CONFIG)/bin/libest.so \
        $(CONFIG)/bin/ca.crt \
        $(CONFIG)/bin/libpcre.so \
        $(CONFIG)/bin/libhttp.so \
        $(CONFIG)/bin/http \
        $(CONFIG)/bin/libsqlite3.so \
        $(CONFIG)/bin/sqlite \
        $(CONFIG)/bin/libzlib.so \
        $(CONFIG)/bin/libejs.so \
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
        $(CONFIG)/bin/libejs.db.sqlite.so \
        $(CONFIG)/bin/ejs.web.mod \
        $(CONFIG)/bin/libejs.web.so \
        $(CONFIG)/bin/www \
        $(CONFIG)/bin/ejs.template.mod \
        $(CONFIG)/bin/ejs.zlib.mod \
        $(CONFIG)/bin/libejs.zlib.so \
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
	@if [ "$(BIT_APP_PREFIX)" = "" ] ; then echo WARNING: BIT_APP_PREFIX not set ; exit 255 ; fi
	@[ ! -x $(CONFIG)/bin ] && mkdir -p $(CONFIG)/bin; true
	@[ ! -x $(CONFIG)/inc ] && mkdir -p $(CONFIG)/inc; true
	@[ ! -x $(CONFIG)/obj ] && mkdir -p $(CONFIG)/obj; true
	@[ ! -f $(CONFIG)/inc/bit.h ] && cp projects/ejs-freebsd-default-bit.h $(CONFIG)/inc/bit.h ; true
	@[ ! -f $(CONFIG)/inc/bitos.h ] && cp src/bitos.h $(CONFIG)/inc/bitos.h ; true
	@if ! diff $(CONFIG)/inc/bit.h projects/ejs-freebsd-default-bit.h >/dev/null ; then\
		echo cp projects/ejs-freebsd-default-bit.h $(CONFIG)/inc/bit.h  ; \
		cp projects/ejs-freebsd-default-bit.h $(CONFIG)/inc/bit.h  ; \
	fi; true

clean:
	rm -rf $(CONFIG)/bin/libmpr.so
	rm -rf $(CONFIG)/bin/libmprssl.so
	rm -rf $(CONFIG)/bin/ejsman
	rm -rf $(CONFIG)/bin/makerom
	rm -rf $(CONFIG)/bin/libest.so
	rm -rf $(CONFIG)/bin/ca.crt
	rm -rf $(CONFIG)/bin/libpcre.so
	rm -rf $(CONFIG)/bin/libhttp.so
	rm -rf $(CONFIG)/bin/http
	rm -rf $(CONFIG)/bin/libsqlite3.so
	rm -rf $(CONFIG)/bin/sqlite
	rm -rf $(CONFIG)/bin/libzlib.so
	rm -rf $(CONFIG)/bin/libejs.so
	rm -rf $(CONFIG)/bin/ejs
	rm -rf $(CONFIG)/bin/ejsc
	rm -rf $(CONFIG)/bin/ejsmod
	rm -rf $(CONFIG)/bin/ejsrun
	rm -rf $(CONFIG)/bin/jem.es
	rm -rf $(CONFIG)/bin/jem
	rm -rf $(CONFIG)/bin/ejs.db.mod
	rm -rf $(CONFIG)/bin/ejs.db.mapper.mod
	rm -rf $(CONFIG)/bin/ejs.db.sqlite.mod
	rm -rf $(CONFIG)/bin/libejs.db.sqlite.so
	rm -rf $(CONFIG)/bin/ejs.web.mod
	rm -rf $(CONFIG)/bin/libejs.web.so
	rm -rf $(CONFIG)/bin/www
	rm -rf $(CONFIG)/bin/ejs.template.mod
	rm -rf $(CONFIG)/bin/libejs.zlib.so
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

clobber: clean
	rm -fr ./$(CONFIG)

$(CONFIG)/inc/mpr.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/mpr/mpr.h" "$(CONFIG)/inc/mpr.h"

$(CONFIG)/inc/bit.h: 

$(CONFIG)/inc/bitos.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/bitos.h" "$(CONFIG)/inc/bitos.h"

$(CONFIG)/obj/mprLib.o: \
    src/deps/mpr/mprLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/inc/bitos.h
	$(CC) -c -o $(CONFIG)/obj/mprLib.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprLib.c

$(CONFIG)/bin/libmpr.so: \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/obj/mprLib.o
	$(CC) -shared -o $(CONFIG)/bin/libmpr.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/mprLib.o $(LIBS)

$(CONFIG)/inc/est.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/est/est.h" "$(CONFIG)/inc/est.h"

$(CONFIG)/obj/estLib.o: \
    src/deps/est/estLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/est.h \
    $(CONFIG)/inc/bitos.h
	$(CC) -c -o $(CONFIG)/obj/estLib.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/est/estLib.c

$(CONFIG)/bin/libest.so: \
    $(CONFIG)/inc/est.h \
    $(CONFIG)/obj/estLib.o
	$(CC) -shared -o $(CONFIG)/bin/libest.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/estLib.o $(LIBS)

$(CONFIG)/obj/mprSsl.o: \
    src/deps/mpr/mprSsl.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/inc/est.h
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprSsl.c

$(CONFIG)/bin/libmprssl.so: \
    $(CONFIG)/bin/libmpr.so \
    $(CONFIG)/bin/libest.so \
    $(CONFIG)/obj/mprSsl.o
	$(CC) -shared -o $(CONFIG)/bin/libmprssl.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/mprSsl.o -lest -lmpr $(LIBS)

$(CONFIG)/obj/manager.o: \
    src/deps/mpr/manager.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/manager.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/manager.c

$(CONFIG)/bin/ejsman: \
    $(CONFIG)/bin/libmpr.so \
    $(CONFIG)/obj/manager.o
	$(CC) -o $(CONFIG)/bin/ejsman $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/manager.o -lmpr $(LIBS) -lmpr -lpthread -lm -ldl $(LDFLAGS)

$(CONFIG)/obj/makerom.o: \
    src/deps/mpr/makerom.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/makerom.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/makerom.c

$(CONFIG)/bin/makerom: \
    $(CONFIG)/bin/libmpr.so \
    $(CONFIG)/obj/makerom.o
	$(CC) -o $(CONFIG)/bin/makerom $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o -lmpr $(LIBS) -lmpr -lpthread -lm -ldl $(LDFLAGS)

$(CONFIG)/bin/ca.crt: \
    src/deps/est/ca.crt
	mkdir -p "$(CONFIG)/bin"
	cp "src/deps/est/ca.crt" "$(CONFIG)/bin/ca.crt"

$(CONFIG)/inc/pcre.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/pcre/pcre.h" "$(CONFIG)/inc/pcre.h"

$(CONFIG)/obj/pcre.o: \
    src/deps/pcre/pcre.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/pcre.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/pcre/pcre.c

$(CONFIG)/bin/libpcre.so: \
    $(CONFIG)/inc/pcre.h \
    $(CONFIG)/obj/pcre.o
	$(CC) -shared -o $(CONFIG)/bin/libpcre.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/pcre.o $(LIBS)

$(CONFIG)/inc/http.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/http/http.h" "$(CONFIG)/inc/http.h"

$(CONFIG)/obj/httpLib.o: \
    src/deps/http/httpLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/http.h \
    $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/httpLib.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/httpLib.c

$(CONFIG)/bin/libhttp.so: \
    $(CONFIG)/bin/libmpr.so \
    $(CONFIG)/bin/libpcre.so \
    $(CONFIG)/inc/http.h \
    $(CONFIG)/obj/httpLib.o
	$(CC) -shared -o $(CONFIG)/bin/libhttp.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/httpLib.o -lpcre -lmpr $(LIBS)

$(CONFIG)/obj/http.o: \
    src/deps/http/http.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/http.h
	$(CC) -c -o $(CONFIG)/obj/http.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/http.c

$(CONFIG)/bin/http: \
    $(CONFIG)/bin/libhttp.so \
    $(CONFIG)/obj/http.o
	$(CC) -o $(CONFIG)/bin/http $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o -lhttp $(LIBS) -lpcre -lmpr -lhttp -lpthread -lm -ldl -lpcre -lmpr $(LDFLAGS)

$(CONFIG)/inc/sqlite3.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/sqlite/sqlite3.h" "$(CONFIG)/inc/sqlite3.h"

$(CONFIG)/obj/sqlite3.o: \
    src/deps/sqlite/sqlite3.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/sqlite3.h
	$(CC) -c -o $(CONFIG)/obj/sqlite3.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/sqlite/sqlite3.c

$(CONFIG)/bin/libsqlite3.so: \
    $(CONFIG)/inc/sqlite3.h \
    $(CONFIG)/obj/sqlite3.o
	$(CC) -shared -o $(CONFIG)/bin/libsqlite3.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/sqlite3.o $(LIBS)

$(CONFIG)/obj/sqlite.o: \
    src/deps/sqlite/sqlite.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/sqlite3.h
	$(CC) -c -o $(CONFIG)/obj/sqlite.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/sqlite/sqlite.c

$(CONFIG)/bin/sqlite: \
    $(CONFIG)/bin/libsqlite3.so \
    $(CONFIG)/obj/sqlite.o
	$(CC) -o $(CONFIG)/bin/sqlite $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/sqlite.o -lsqlite3 $(LIBS) -lsqlite3 -lpthread -lm -ldl $(LDFLAGS)

$(CONFIG)/inc/zlib.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/zlib/zlib.h" "$(CONFIG)/inc/zlib.h"

$(CONFIG)/obj/zlib.o: \
    src/deps/zlib/zlib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/zlib.h
	$(CC) -c -o $(CONFIG)/obj/zlib.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/deps/zlib/zlib.c

$(CONFIG)/bin/libzlib.so: \
    $(CONFIG)/inc/zlib.h \
    $(CONFIG)/obj/zlib.o
	$(CC) -shared -o $(CONFIG)/bin/libzlib.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/zlib.o $(LIBS)

$(CONFIG)/inc/ejs.cache.local.slots.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.cache.local.slots.h" "$(CONFIG)/inc/ejs.cache.local.slots.h"

$(CONFIG)/inc/ejs.db.sqlite.slots.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.db.sqlite.slots.h" "$(CONFIG)/inc/ejs.db.sqlite.slots.h"

$(CONFIG)/inc/ejs.slots.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.slots.h" "$(CONFIG)/inc/ejs.slots.h"

$(CONFIG)/inc/ejs.web.slots.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.web.slots.h" "$(CONFIG)/inc/ejs.web.slots.h"

$(CONFIG)/inc/ejs.zlib.slots.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.zlib.slots.h" "$(CONFIG)/inc/ejs.zlib.slots.h"

$(CONFIG)/inc/ejsByteCode.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsByteCode.h" "$(CONFIG)/inc/ejsByteCode.h"

$(CONFIG)/inc/ejsByteCodeTable.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsByteCodeTable.h" "$(CONFIG)/inc/ejsByteCodeTable.h"

$(CONFIG)/inc/ejsCustomize.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsCustomize.h" "$(CONFIG)/inc/ejsCustomize.h"

$(CONFIG)/inc/ejs.h: \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/inc/http.h \
    $(CONFIG)/inc/ejsByteCode.h \
    $(CONFIG)/inc/ejsByteCodeTable.h \
    $(CONFIG)/inc/ejs.slots.h \
    $(CONFIG)/inc/ejsCustomize.h
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejs.h" "$(CONFIG)/inc/ejs.h"

$(CONFIG)/inc/ejsCompiler.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsCompiler.h" "$(CONFIG)/inc/ejsCompiler.h"

$(CONFIG)/obj/ecAst.o: \
    src/compiler/ecAst.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ecAst.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecAst.c

$(CONFIG)/obj/ecCodeGen.o: \
    src/compiler/ecCodeGen.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecCodeGen.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCodeGen.c

$(CONFIG)/obj/ecCompiler.o: \
    src/compiler/ecCompiler.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecCompiler.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCompiler.c

$(CONFIG)/obj/ecLex.o: \
    src/compiler/ecLex.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecLex.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecLex.c

$(CONFIG)/obj/ecModuleWrite.o: \
    src/compiler/ecModuleWrite.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecModuleWrite.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecModuleWrite.c

$(CONFIG)/obj/ecParser.o: \
    src/compiler/ecParser.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecParser.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecParser.c

$(CONFIG)/obj/ecState.o: \
    src/compiler/ecState.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecState.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecState.c

$(CONFIG)/obj/dtoa.o: \
    src/core/src/dtoa.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/dtoa.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/dtoa.c

$(CONFIG)/obj/ejsApp.o: \
    src/core/src/ejsApp.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsApp.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsApp.c

$(CONFIG)/obj/ejsArray.o: \
    src/core/src/ejsArray.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsArray.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsArray.c

$(CONFIG)/obj/ejsBlock.o: \
    src/core/src/ejsBlock.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsBlock.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBlock.c

$(CONFIG)/obj/ejsBoolean.o: \
    src/core/src/ejsBoolean.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsBoolean.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBoolean.c

$(CONFIG)/obj/ejsByteArray.o: \
    src/core/src/ejsByteArray.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsByteArray.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsByteArray.c

$(CONFIG)/obj/ejsCache.o: \
    src/core/src/ejsCache.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsCache.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCache.c

$(CONFIG)/obj/ejsCmd.o: \
    src/core/src/ejsCmd.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsCmd.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCmd.c

$(CONFIG)/obj/ejsConfig.o: \
    src/core/src/ejsConfig.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsConfig.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsConfig.c

$(CONFIG)/obj/ejsDate.o: \
    src/core/src/ejsDate.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsDate.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDate.c

$(CONFIG)/obj/ejsDebug.o: \
    src/core/src/ejsDebug.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsDebug.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDebug.c

$(CONFIG)/obj/ejsError.o: \
    src/core/src/ejsError.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsError.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsError.c

$(CONFIG)/obj/ejsFile.o: \
    src/core/src/ejsFile.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFile.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFile.c

$(CONFIG)/obj/ejsFileSystem.o: \
    src/core/src/ejsFileSystem.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFileSystem.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFileSystem.c

$(CONFIG)/obj/ejsFrame.o: \
    src/core/src/ejsFrame.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFrame.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFrame.c

$(CONFIG)/obj/ejsFunction.o: \
    src/core/src/ejsFunction.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFunction.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFunction.c

$(CONFIG)/obj/ejsGC.o: \
    src/core/src/ejsGC.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsGC.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGC.c

$(CONFIG)/obj/ejsGlobal.o: \
    src/core/src/ejsGlobal.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsGlobal.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGlobal.c

$(CONFIG)/obj/ejsHttp.o: \
    src/core/src/ejsHttp.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsHttp.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsHttp.c

$(CONFIG)/obj/ejsIterator.o: \
    src/core/src/ejsIterator.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsIterator.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsIterator.c

$(CONFIG)/obj/ejsJSON.o: \
    src/core/src/ejsJSON.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsJSON.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsJSON.c

$(CONFIG)/obj/ejsLocalCache.o: \
    src/core/src/ejsLocalCache.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsLocalCache.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsLocalCache.c

$(CONFIG)/obj/ejsMath.o: \
    src/core/src/ejsMath.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsMath.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMath.c

$(CONFIG)/obj/ejsMemory.o: \
    src/core/src/ejsMemory.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsMemory.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMemory.c

$(CONFIG)/obj/ejsMprLog.o: \
    src/core/src/ejsMprLog.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsMprLog.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMprLog.c

$(CONFIG)/obj/ejsNamespace.o: \
    src/core/src/ejsNamespace.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsNamespace.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNamespace.c

$(CONFIG)/obj/ejsNull.o: \
    src/core/src/ejsNull.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsNull.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNull.c

$(CONFIG)/obj/ejsNumber.o: \
    src/core/src/ejsNumber.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsNumber.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNumber.c

$(CONFIG)/obj/ejsObject.o: \
    src/core/src/ejsObject.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsObject.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsObject.c

$(CONFIG)/obj/ejsPath.o: \
    src/core/src/ejsPath.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/ejsPath.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPath.c

$(CONFIG)/obj/ejsPot.o: \
    src/core/src/ejsPot.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsPot.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPot.c

$(CONFIG)/obj/ejsRegExp.o: \
    src/core/src/ejsRegExp.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/ejsRegExp.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsRegExp.c

$(CONFIG)/obj/ejsSocket.o: \
    src/core/src/ejsSocket.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsSocket.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSocket.c

$(CONFIG)/obj/ejsString.o: \
    src/core/src/ejsString.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/ejsString.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsString.c

$(CONFIG)/obj/ejsSystem.o: \
    src/core/src/ejsSystem.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsSystem.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSystem.c

$(CONFIG)/obj/ejsTimer.o: \
    src/core/src/ejsTimer.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsTimer.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsTimer.c

$(CONFIG)/obj/ejsType.o: \
    src/core/src/ejsType.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsType.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsType.c

$(CONFIG)/obj/ejsUri.o: \
    src/core/src/ejsUri.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsUri.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsUri.c

$(CONFIG)/obj/ejsVoid.o: \
    src/core/src/ejsVoid.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsVoid.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsVoid.c

$(CONFIG)/obj/ejsWebSocket.o: \
    src/core/src/ejsWebSocket.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsWebSocket.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWebSocket.c

$(CONFIG)/obj/ejsWorker.o: \
    src/core/src/ejsWorker.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsWorker.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWorker.c

$(CONFIG)/obj/ejsXML.o: \
    src/core/src/ejsXML.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsXML.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXML.c

$(CONFIG)/obj/ejsXMLList.o: \
    src/core/src/ejsXMLList.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsXMLList.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLList.c

$(CONFIG)/obj/ejsXMLLoader.o: \
    src/core/src/ejsXMLLoader.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsXMLLoader.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLLoader.c

$(CONFIG)/obj/ejsByteCode.o: \
    src/vm/ejsByteCode.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsByteCode.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsByteCode.c

$(CONFIG)/obj/ejsException.o: \
    src/vm/ejsException.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsException.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsException.c

$(CONFIG)/obj/ejsHelper.o: \
    src/vm/ejsHelper.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsHelper.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsHelper.c

$(CONFIG)/obj/ejsInterp.o: \
    src/vm/ejsInterp.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsInterp.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsInterp.c

$(CONFIG)/obj/ejsLoader.o: \
    src/vm/ejsLoader.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsLoader.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsLoader.c

$(CONFIG)/obj/ejsModule.o: \
    src/vm/ejsModule.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsModule.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsModule.c

$(CONFIG)/obj/ejsScope.o: \
    src/vm/ejsScope.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsScope.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsScope.c

$(CONFIG)/obj/ejsService.o: \
    src/vm/ejsService.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsService.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsService.c

$(CONFIG)/bin/libejs.so: \
    $(CONFIG)/bin/libhttp.so \
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
	$(CC) -shared -o $(CONFIG)/bin/libejs.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ecAst.o $(CONFIG)/obj/ecCodeGen.o $(CONFIG)/obj/ecCompiler.o $(CONFIG)/obj/ecLex.o $(CONFIG)/obj/ecModuleWrite.o $(CONFIG)/obj/ecParser.o $(CONFIG)/obj/ecState.o $(CONFIG)/obj/dtoa.o $(CONFIG)/obj/ejsApp.o $(CONFIG)/obj/ejsArray.o $(CONFIG)/obj/ejsBlock.o $(CONFIG)/obj/ejsBoolean.o $(CONFIG)/obj/ejsByteArray.o $(CONFIG)/obj/ejsCache.o $(CONFIG)/obj/ejsCmd.o $(CONFIG)/obj/ejsConfig.o $(CONFIG)/obj/ejsDate.o $(CONFIG)/obj/ejsDebug.o $(CONFIG)/obj/ejsError.o $(CONFIG)/obj/ejsFile.o $(CONFIG)/obj/ejsFileSystem.o $(CONFIG)/obj/ejsFrame.o $(CONFIG)/obj/ejsFunction.o $(CONFIG)/obj/ejsGC.o $(CONFIG)/obj/ejsGlobal.o $(CONFIG)/obj/ejsHttp.o $(CONFIG)/obj/ejsIterator.o $(CONFIG)/obj/ejsJSON.o $(CONFIG)/obj/ejsLocalCache.o $(CONFIG)/obj/ejsMath.o $(CONFIG)/obj/ejsMemory.o $(CONFIG)/obj/ejsMprLog.o $(CONFIG)/obj/ejsNamespace.o $(CONFIG)/obj/ejsNull.o $(CONFIG)/obj/ejsNumber.o $(CONFIG)/obj/ejsObject.o $(CONFIG)/obj/ejsPath.o $(CONFIG)/obj/ejsPot.o $(CONFIG)/obj/ejsRegExp.o $(CONFIG)/obj/ejsSocket.o $(CONFIG)/obj/ejsString.o $(CONFIG)/obj/ejsSystem.o $(CONFIG)/obj/ejsTimer.o $(CONFIG)/obj/ejsType.o $(CONFIG)/obj/ejsUri.o $(CONFIG)/obj/ejsVoid.o $(CONFIG)/obj/ejsWebSocket.o $(CONFIG)/obj/ejsWorker.o $(CONFIG)/obj/ejsXML.o $(CONFIG)/obj/ejsXMLList.o $(CONFIG)/obj/ejsXMLLoader.o $(CONFIG)/obj/ejsByteCode.o $(CONFIG)/obj/ejsException.o $(CONFIG)/obj/ejsHelper.o $(CONFIG)/obj/ejsInterp.o $(CONFIG)/obj/ejsLoader.o $(CONFIG)/obj/ejsModule.o $(CONFIG)/obj/ejsScope.o $(CONFIG)/obj/ejsService.o -lhttp $(LIBS) -lpcre -lmpr

$(CONFIG)/obj/ejs.o: \
    src/cmd/ejs.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ejs.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejs.c

$(CONFIG)/bin/ejs: \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/obj/ejs.o
	$(CC) -o $(CONFIG)/bin/ejs $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejs.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -lpthread -lm -ldl -lhttp -lpcre -lmpr $(LDFLAGS)

$(CONFIG)/obj/ejsc.o: \
    src/cmd/ejsc.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ejsc.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsc.c

$(CONFIG)/bin/ejsc: \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/obj/ejsc.o
	$(CC) -o $(CONFIG)/bin/ejsc $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsc.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -lpthread -lm -ldl -lhttp -lpcre -lmpr $(LDFLAGS)

src/cmd/ejsmod.h: 

$(CONFIG)/obj/ejsmod.o: \
    src/cmd/ejsmod.c\
    $(CONFIG)/inc/bit.h \
    src/cmd/ejsmod.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsmod.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/ejsmod.c

$(CONFIG)/obj/doc.o: \
    src/cmd/doc.c\
    $(CONFIG)/inc/bit.h \
    src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/doc.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/doc.c

$(CONFIG)/obj/docFiles.o: \
    src/cmd/docFiles.c\
    $(CONFIG)/inc/bit.h \
    src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/docFiles.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/docFiles.c

$(CONFIG)/obj/listing.o: \
    src/cmd/listing.c\
    $(CONFIG)/inc/bit.h \
    src/cmd/ejsmod.h \
    $(CONFIG)/inc/ejsByteCodeTable.h
	$(CC) -c -o $(CONFIG)/obj/listing.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/listing.c

$(CONFIG)/obj/slotGen.o: \
    src/cmd/slotGen.c\
    $(CONFIG)/inc/bit.h \
    src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/slotGen.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/slotGen.c

$(CONFIG)/bin/ejsmod: \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/obj/ejsmod.o \
    $(CONFIG)/obj/doc.o \
    $(CONFIG)/obj/docFiles.o \
    $(CONFIG)/obj/listing.o \
    $(CONFIG)/obj/slotGen.o
	$(CC) -o $(CONFIG)/bin/ejsmod $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsmod.o $(CONFIG)/obj/doc.o $(CONFIG)/obj/docFiles.o $(CONFIG)/obj/listing.o $(CONFIG)/obj/slotGen.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -lpthread -lm -ldl -lhttp -lpcre -lmpr $(LDFLAGS)

$(CONFIG)/obj/ejsrun.o: \
    src/cmd/ejsrun.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ejsrun.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsrun.c

$(CONFIG)/bin/ejsrun: \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/obj/ejsrun.o
	$(CC) -o $(CONFIG)/bin/ejsrun $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -lpthread -lm -ldl -lhttp -lpcre -lmpr $(LDFLAGS)

$(CONFIG)/bin/ejs.mod: \
    $(CONFIG)/bin/ejsc \
    $(CONFIG)/bin/ejsmod
	cd src/core; ../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.mod  --optimize 9 --bind --require null *.es  ; cd ../..
	cd src/core; ../../$(CONFIG)/bin/ejsmod --require null --cslots ../../$(CONFIG)/bin/ejs.mod ; cd ../..
	cd src/core; if ! diff ejs.slots.h ../../$(CONFIG)/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ../../$(CONFIG)/inc; fi ; cd ../..
	cd src/core; rm -f ejs.slots.h ; cd ../..

$(CONFIG)/bin/ejs.unix.mod: \
    $(CONFIG)/bin/ejsc \
    $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.unix; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.unix.mod  --optimize 9 Unix.es ; cd ../../..

$(CONFIG)/bin/jem.es: 
	cd src/jems/ejs.jem; cp jem.es ../../../$(CONFIG)/bin ; cd ../../..

$(CONFIG)/bin/jem: \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/bin/jem.es \
    $(CONFIG)/obj/ejsrun.o
	$(CC) -o $(CONFIG)/bin/jem $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -lpthread -lm -ldl -lhttp -lpcre -lmpr $(LDFLAGS)

$(CONFIG)/bin/ejs.db.mod: \
    $(CONFIG)/bin/ejsc \
    $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.db; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.mod  --optimize 9 *.es ; cd ../../..

$(CONFIG)/bin/ejs.db.mapper.mod: \
    $(CONFIG)/bin/ejsc \
    $(CONFIG)/bin/ejs.mod \
    $(CONFIG)/bin/ejs.db.mod
	cd src/jems/ejs.db.mapper; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.mapper.mod  --optimize 9 *.es ; cd ../../..

$(CONFIG)/bin/ejs.db.sqlite.mod: \
    $(CONFIG)/bin/ejsc \
    $(CONFIG)/bin/ejsmod \
    $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.db.sqlite; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.sqlite.mod  --optimize 9 *.es ; cd ../../..

$(CONFIG)/obj/ejsSqlite.o: \
    src/jems/ejs.db.sqlite/ejsSqlite.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/ejs.db.sqlite.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsSqlite.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/jems/ejs.db.sqlite/ejsSqlite.c

$(CONFIG)/bin/libejs.db.sqlite.so: \
    $(CONFIG)/bin/libmpr.so \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/bin/ejs.mod \
    $(CONFIG)/bin/ejs.db.sqlite.mod \
    $(CONFIG)/bin/libsqlite3.so \
    $(CONFIG)/obj/ejsSqlite.o
	$(CC) -shared -o $(CONFIG)/bin/libejs.db.sqlite.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsSqlite.o -lsqlite3 -lejs -lmpr $(LIBS) -lhttp -lpcre

$(CONFIG)/bin/ejs.web.mod: \
    $(CONFIG)/bin/ejsc \
    $(CONFIG)/bin/ejsmod \
    $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.web; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.web.mod  --optimize 9 *.es ; cd ../../..
	cd src/jems/ejs.web; ../../../$(CONFIG)/bin/ejsmod --cslots ../../../$(CONFIG)/bin/ejs.web.mod ; cd ../../..
	cd src/jems/ejs.web; if ! diff ejs.web.slots.h ../../../$(CONFIG)/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../../$(CONFIG)/inc; fi ; cd ../../..
	cd src/jems/ejs.web; rm -f ejs.web.slots.h ; cd ../../..

$(CONFIG)/inc/ejsWeb.h: 
	mkdir -p "$(CONFIG)/inc"
	cp "src/jems/ejs.web/ejsWeb.h" "$(CONFIG)/inc/ejsWeb.h"

$(CONFIG)/obj/ejsHttpServer.o: \
    src/jems/ejs.web/ejsHttpServer.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/ejsCompiler.h \
    $(CONFIG)/inc/ejsWeb.h \
    $(CONFIG)/inc/ejs.web.slots.h \
    $(CONFIG)/inc/http.h
	$(CC) -c -o $(CONFIG)/obj/ejsHttpServer.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsHttpServer.c

$(CONFIG)/obj/ejsRequest.o: \
    src/jems/ejs.web/ejsRequest.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/ejsCompiler.h \
    $(CONFIG)/inc/ejsWeb.h \
    $(CONFIG)/inc/ejs.web.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsRequest.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsRequest.c

$(CONFIG)/obj/ejsSession.o: \
    src/jems/ejs.web/ejsSession.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/ejsWeb.h
	$(CC) -c -o $(CONFIG)/obj/ejsSession.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsSession.c

$(CONFIG)/obj/ejsWeb.o: \
    src/jems/ejs.web/ejsWeb.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/ejsCompiler.h \
    $(CONFIG)/inc/ejsWeb.h \
    $(CONFIG)/inc/ejs.web.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsWeb.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsWeb.c

$(CONFIG)/bin/libejs.web.so: \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/bin/ejs.mod \
    $(CONFIG)/inc/ejsWeb.h \
    $(CONFIG)/obj/ejsHttpServer.o \
    $(CONFIG)/obj/ejsRequest.o \
    $(CONFIG)/obj/ejsSession.o \
    $(CONFIG)/obj/ejsWeb.o
	$(CC) -shared -o $(CONFIG)/bin/libejs.web.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsHttpServer.o $(CONFIG)/obj/ejsRequest.o $(CONFIG)/obj/ejsSession.o $(CONFIG)/obj/ejsWeb.o -lejs $(LIBS) -lhttp -lpcre -lmpr

$(CONFIG)/bin/www: 
	cd src/jems/ejs.web; rm -fr ../../../$(CONFIG)/bin/www ; cd ../../..
	cd src/jems/ejs.web; cp -r www ../../../$(CONFIG)/bin ; cd ../../..

$(CONFIG)/bin/ejs.template.mod: \
    $(CONFIG)/bin/ejsc \
    $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.template; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.template.mod  --optimize 9 TemplateParser.es ; cd ../../..

$(CONFIG)/bin/ejs.zlib.mod: \
    $(CONFIG)/bin/ejsc \
    $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.zlib; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.zlib.mod  --optimize 9 *.es ; cd ../../..

$(CONFIG)/obj/ejsZlib.o: \
    src/jems/ejs.zlib/ejsZlib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/zlib.h \
    $(CONFIG)/inc/ejs.zlib.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsZlib.o -fPIC $(LDFLAGS) $(DFLAGS) $(IFLAGS) src/jems/ejs.zlib/ejsZlib.c

$(CONFIG)/bin/libejs.zlib.so: \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/bin/ejs.mod \
    $(CONFIG)/bin/ejs.zlib.mod \
    $(CONFIG)/bin/libzlib.so \
    $(CONFIG)/obj/ejsZlib.o
	$(CC) -shared -o $(CONFIG)/bin/libejs.zlib.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsZlib.o -lzlib -lejs $(LIBS) -lhttp -lpcre -lmpr

$(CONFIG)/bin/ejs.tar.mod: \
    $(CONFIG)/bin/ejsc \
    $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.tar; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.tar.mod  --optimize 9 *.es ; cd ../../..

$(CONFIG)/bin/mvc.es: 
	cd src/jems/ejs.mvc; cp mvc.es ../../../$(CONFIG)/bin ; cd ../../..

$(CONFIG)/bin/mvc: \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/bin/mvc.es \
    $(CONFIG)/obj/ejsrun.o
	$(CC) -o $(CONFIG)/bin/mvc $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -lpthread -lm -ldl -lhttp -lpcre -lmpr $(LDFLAGS)

$(CONFIG)/bin/ejs.mvc.mod: \
    $(CONFIG)/bin/ejsc \
    $(CONFIG)/bin/ejs.mod \
    $(CONFIG)/bin/ejs.web.mod \
    $(CONFIG)/bin/ejs.template.mod \
    $(CONFIG)/bin/ejs.unix.mod
	cd src/jems/ejs.mvc; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.mvc.mod  --optimize 9 *.es ; cd ../../..

$(CONFIG)/bin/utest.es: 
	cd src/jems/ejs.utest; cp utest.es ../../../$(CONFIG)/bin ; cd ../../..

$(CONFIG)/bin/utest.worker: 
	cd src/jems/ejs.utest; cp utest.worker ../../../$(CONFIG)/bin ; cd ../../..

$(CONFIG)/bin/utest: \
    $(CONFIG)/bin/libejs.so \
    $(CONFIG)/bin/utest.es \
    $(CONFIG)/bin/utest.worker \
    $(CONFIG)/obj/ejsrun.o
	$(CC) -o $(CONFIG)/bin/utest $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lejs -lpthread -lm -ldl -lhttp -lpcre -lmpr $(LDFLAGS)

version: 
	@echo 2.3.0-1

stop: 
	

installBinary: stop
	rm -f "$(BIT_APP_PREFIX)/latest"
	mkdir -p "$(BIT_APP_PREFIX)"
	ln -s "2.3.0" "$(BIT_APP_PREFIX)/latest"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin"
	cp "$(CONFIG)/bin/ejs" "$(BIT_VAPP_PREFIX)/bin/ejs"
	rm -f "$(BIT_BIN_PREFIX)/ejs"
	mkdir -p "$(BIT_BIN_PREFIX)"
	ln -s "$(BIT_VAPP_PREFIX)/bin/ejs" "$(BIT_BIN_PREFIX)/ejs"
	cp "$(CONFIG)/bin/ejsc" "$(BIT_VAPP_PREFIX)/bin/ejsc"
	rm -f "$(BIT_BIN_PREFIX)/ejsc"
	ln -s "$(BIT_VAPP_PREFIX)/bin/ejsc" "$(BIT_BIN_PREFIX)/ejsc"
	cp "$(CONFIG)/bin/ejsman" "$(BIT_VAPP_PREFIX)/bin/ejsman"
	rm -f "$(BIT_BIN_PREFIX)/ejsman"
	ln -s "$(BIT_VAPP_PREFIX)/bin/ejsman" "$(BIT_BIN_PREFIX)/ejsman"
	cp "$(CONFIG)/bin/ejsmod" "$(BIT_VAPP_PREFIX)/bin/ejsmod"
	rm -f "$(BIT_BIN_PREFIX)/ejsmod"
	ln -s "$(BIT_VAPP_PREFIX)/bin/ejsmod" "$(BIT_BIN_PREFIX)/ejsmod"
	cp "$(CONFIG)/bin/ejsrun" "$(BIT_VAPP_PREFIX)/bin/ejsrun"
	rm -f "$(BIT_BIN_PREFIX)/ejsrun"
	ln -s "$(BIT_VAPP_PREFIX)/bin/ejsrun" "$(BIT_BIN_PREFIX)/ejsrun"
	cp "$(CONFIG)/bin/jem" "$(BIT_VAPP_PREFIX)/bin/jem"
	rm -f "$(BIT_BIN_PREFIX)/jem"
	ln -s "$(BIT_VAPP_PREFIX)/bin/jem" "$(BIT_BIN_PREFIX)/jem"
	cp "$(CONFIG)/bin/mvc" "$(BIT_VAPP_PREFIX)/bin/mvc"
	rm -f "$(BIT_BIN_PREFIX)/mvc"
	ln -s "$(BIT_VAPP_PREFIX)/bin/mvc" "$(BIT_BIN_PREFIX)/mvc"
	cp "$(CONFIG)/bin/utest" "$(BIT_VAPP_PREFIX)/bin/utest"
	rm -f "$(BIT_BIN_PREFIX)/utest"
	ln -s "$(BIT_VAPP_PREFIX)/bin/utest" "$(BIT_BIN_PREFIX)/utest"
	cp "$(CONFIG)/bin/libejs.db.sqlite.so" "$(BIT_VAPP_PREFIX)/bin/libejs.db.sqlite.so"
	cp "$(CONFIG)/bin/libejs.so" "$(BIT_VAPP_PREFIX)/bin/libejs.so"
	cp "$(CONFIG)/bin/libejs.db.sqlite.so" "$(BIT_VAPP_PREFIX)/bin/libejs.db.sqlite.so"
	cp "$(CONFIG)/bin/libejs.web.so" "$(BIT_VAPP_PREFIX)/bin/libejs.web.so"
	cp "$(CONFIG)/bin/libejs.zlib.so" "$(BIT_VAPP_PREFIX)/bin/libejs.zlib.so"
	cp "$(CONFIG)/bin/libest.so" "$(BIT_VAPP_PREFIX)/bin/libest.so"
	cp "$(CONFIG)/bin/libhttp.so" "$(BIT_VAPP_PREFIX)/bin/libhttp.so"
	cp "$(CONFIG)/bin/libmpr.so" "$(BIT_VAPP_PREFIX)/bin/libmpr.so"
	cp "$(CONFIG)/bin/libmprSsl.so" "$(BIT_VAPP_PREFIX)/bin/libmprSsl.so"
	cp "$(CONFIG)/bin/libpcre.so" "$(BIT_VAPP_PREFIX)/bin/libpcre.so"
	cp "$(CONFIG)/bin/libsqlite3.so" "$(BIT_VAPP_PREFIX)/bin/libsqlite3.so"
	cp "$(CONFIG)/bin/libzlib.so" "$(BIT_VAPP_PREFIX)/bin/libzlib.so"
	cp "$(CONFIG)/bin/ejs.db.mapper.mod" "$(BIT_VAPP_PREFIX)/bin/ejs.db.mapper.mod"
	cp "$(CONFIG)/bin/ejs.db.mod" "$(BIT_VAPP_PREFIX)/bin/ejs.db.mod"
	cp "$(CONFIG)/bin/ejs.db.sqlite.mod" "$(BIT_VAPP_PREFIX)/bin/ejs.db.sqlite.mod"
	cp "$(CONFIG)/bin/ejs.mod" "$(BIT_VAPP_PREFIX)/bin/ejs.mod"
	cp "$(CONFIG)/bin/ejs.mvc.mod" "$(BIT_VAPP_PREFIX)/bin/ejs.mvc.mod"
	cp "$(CONFIG)/bin/ejs.tar.mod" "$(BIT_VAPP_PREFIX)/bin/ejs.tar.mod"
	cp "$(CONFIG)/bin/ejs.template.mod" "$(BIT_VAPP_PREFIX)/bin/ejs.template.mod"
	cp "$(CONFIG)/bin/ejs.unix.mod" "$(BIT_VAPP_PREFIX)/bin/ejs.unix.mod"
	cp "$(CONFIG)/bin/ejs.web.mod" "$(BIT_VAPP_PREFIX)/bin/ejs.web.mod"
	cp "$(CONFIG)/bin/ejs.zlib.mod" "$(BIT_VAPP_PREFIX)/bin/ejs.zlib.mod"
	cp "$(CONFIG)/bin/jem.es" "$(BIT_VAPP_PREFIX)/bin/jem.es"
	cp "$(CONFIG)/bin/mvc.es" "$(BIT_VAPP_PREFIX)/bin/mvc.es"
	cp "$(CONFIG)/bin/utest.es" "$(BIT_VAPP_PREFIX)/bin/utest.es"
	cp "$(CONFIG)/bin/ca.crt" "$(BIT_VAPP_PREFIX)/bin/ca.crt"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/www/images"
	cp "src/jems/ejs.web/www/images/banner.jpg" "$(BIT_VAPP_PREFIX)/bin/www/images/banner.jpg"
	cp "src/jems/ejs.web/www/images/favicon.ico" "$(BIT_VAPP_PREFIX)/bin/www/images/favicon.ico"
	cp "src/jems/ejs.web/www/images/splash.jpg" "$(BIT_VAPP_PREFIX)/bin/www/images/splash.jpg"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/www/js"
	cp "src/jems/ejs.web/www/js/jquery.ejs.min.js" "$(BIT_VAPP_PREFIX)/bin/www/js/jquery.ejs.min.js"
	cp "src/jems/ejs.web/www/js/jquery.min.js" "$(BIT_VAPP_PREFIX)/bin/www/js/jquery.min.js"
	cp "src/jems/ejs.web/www/js/jquery.simplemodal.min.js" "$(BIT_VAPP_PREFIX)/bin/www/js/jquery.simplemodal.min.js"
	cp "src/jems/ejs.web/www/js/jquery.tablesorter.js" "$(BIT_VAPP_PREFIX)/bin/www/js/jquery.tablesorter.js"
	cp "src/jems/ejs.web/www/js/jquery.tablesorter.min.js" "$(BIT_VAPP_PREFIX)/bin/www/js/jquery.tablesorter.min.js"
	cp "src/jems/ejs.web/www/js/jquery.treeview.min.js" "$(BIT_VAPP_PREFIX)/bin/www/js/jquery.treeview.min.js"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images"
	cp "src/jems/ejs.web/www/js/tree-images/file.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/file.gif"
	cp "src/jems/ejs.web/www/js/tree-images/folder-closed.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/folder-closed.gif"
	cp "src/jems/ejs.web/www/js/tree-images/folder.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/folder.gif"
	cp "src/jems/ejs.web/www/js/tree-images/minus.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/minus.gif"
	cp "src/jems/ejs.web/www/js/tree-images/plus.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/plus.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-black-line.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/treeview-black-line.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-black.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/treeview-black.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-default-line.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/treeview-default-line.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-default.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/treeview-default.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-famfamfam-line.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/treeview-famfamfam-line.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-famfamfam.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/treeview-famfamfam.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-gray-line.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/treeview-gray-line.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-gray.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/treeview-gray.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-red-line.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/treeview-red-line.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-red.gif" "$(BIT_VAPP_PREFIX)/bin/www/js/tree-images/treeview-red.gif"
	cp "src/jems/ejs.web/www/js/treeview.css" "$(BIT_VAPP_PREFIX)/bin/www/js/treeview.css"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/www"
	cp "src/jems/ejs.web/www/layout.css" "$(BIT_VAPP_PREFIX)/bin/www/layout.css"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin/www/themes"
	cp "src/jems/ejs.web/www/themes/default.css" "$(BIT_VAPP_PREFIX)/bin/www/themes/default.css"
	mkdir -p "$(BIT_VAPP_PREFIX)/inc"
	cp "$(CONFIG)/inc/bit.h" "$(BIT_VAPP_PREFIX)/inc/bit.h"
	rm -f "$(BIT_INC_PREFIX)/ejs/bit.h"
	mkdir -p "$(BIT_INC_PREFIX)/ejs"
	ln -s "$(BIT_VAPP_PREFIX)/inc/bit.h" "$(BIT_INC_PREFIX)/ejs/bit.h"
	mkdir -p "$(BIT_VAPP_PREFIX)/doc/man1"
	cp "doc/man/ejs.1" "$(BIT_VAPP_PREFIX)/doc/man1/ejs.1"
	rm -f "$(BIT_MAN_PREFIX)/man1/ejs.1"
	mkdir -p "$(BIT_MAN_PREFIX)/man1"
	ln -s "$(BIT_VAPP_PREFIX)/doc/man1/ejs.1" "$(BIT_MAN_PREFIX)/man1/ejs.1"
	cp "doc/man/ejsc.1" "$(BIT_VAPP_PREFIX)/doc/man1/ejsc.1"
	rm -f "$(BIT_MAN_PREFIX)/man1/ejsc.1"
	ln -s "$(BIT_VAPP_PREFIX)/doc/man1/ejsc.1" "$(BIT_MAN_PREFIX)/man1/ejsc.1"
	cp "doc/man/ejsmod.1" "$(BIT_VAPP_PREFIX)/doc/man1/ejsmod.1"
	rm -f "$(BIT_MAN_PREFIX)/man1/ejsmod.1"
	ln -s "$(BIT_VAPP_PREFIX)/doc/man1/ejsmod.1" "$(BIT_MAN_PREFIX)/man1/ejsmod.1"
	cp "doc/man/http.1" "$(BIT_VAPP_PREFIX)/doc/man1/http.1"
	rm -f "$(BIT_MAN_PREFIX)/man1/http.1"
	ln -s "$(BIT_VAPP_PREFIX)/doc/man1/http.1" "$(BIT_MAN_PREFIX)/man1/http.1"
	cp "doc/man/makerom.1" "$(BIT_VAPP_PREFIX)/doc/man1/makerom.1"
	rm -f "$(BIT_MAN_PREFIX)/man1/makerom.1"
	ln -s "$(BIT_VAPP_PREFIX)/doc/man1/makerom.1" "$(BIT_MAN_PREFIX)/man1/makerom.1"
	cp "doc/man/manager.1" "$(BIT_VAPP_PREFIX)/doc/man1/manager.1"
	rm -f "$(BIT_MAN_PREFIX)/man1/manager.1"
	ln -s "$(BIT_VAPP_PREFIX)/doc/man1/manager.1" "$(BIT_MAN_PREFIX)/man1/manager.1"
	cp "doc/man/mvc.1" "$(BIT_VAPP_PREFIX)/doc/man1/mvc.1"
	rm -f "$(BIT_MAN_PREFIX)/man1/mvc.1"
	ln -s "$(BIT_VAPP_PREFIX)/doc/man1/mvc.1" "$(BIT_MAN_PREFIX)/man1/mvc.1"


start: 
	

install: stop installBinary start
	

uninstall: stop


