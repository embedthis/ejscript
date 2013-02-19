#
#   ejs-macosx-default.mk -- Makefile to build Embedthis Ejscript for macosx
#

PRODUCT         := ejs
VERSION         := 2.3.0
BUILD_NUMBER    := 1
PROFILE         := default
ARCH            := $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
OS              := macosx
CC              := /usr/bin/clang
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

CFLAGS          += -w
DFLAGS          +=  $(patsubst %,-D%,$(filter BIT_%,$(MAKEFLAGS)))
IFLAGS          += -I$(CONFIG)/inc
LDFLAGS         += '-Wl,-rpath,@executable_path/' '-Wl,-rpath,@loader_path/'
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
        $(CONFIG)/bin/libmpr.dylib \
        $(CONFIG)/bin/libmprssl.dylib \
        $(CONFIG)/bin/ejsman \
        $(CONFIG)/bin/makerom \
        $(CONFIG)/bin/libest.dylib \
        $(CONFIG)/bin/ca.crt \
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
        $(CONFIG)/bin/ejs.unix.mod \
        $(CONFIG)/bin/jem.es \
        $(CONFIG)/bin/jem \
        $(CONFIG)/bin/ejs.db.mod \
        $(CONFIG)/bin/ejs.db.mapper.mod \
        $(CONFIG)/bin/ejs.db.sqlite.mod \
        $(CONFIG)/bin/libejs.db.sqlite.dylib \
        $(CONFIG)/bin/ejs.web.mod \
        $(CONFIG)/bin/libejs.web.dylib \
        $(CONFIG)/bin/www \
        $(CONFIG)/bin/ejs.template.mod \
        $(CONFIG)/bin/ejs.zlib.mod \
        $(CONFIG)/bin/libejs.zlib.dylib \
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
	@[ ! -f $(CONFIG)/inc/bit.h ] && cp projects/ejs-macosx-default-bit.h $(CONFIG)/inc/bit.h ; true
	@[ ! -f $(CONFIG)/inc/bitos.h ] && cp src/bitos.h $(CONFIG)/inc/bitos.h ; true
	@if ! diff $(CONFIG)/inc/bit.h projects/ejs-macosx-default-bit.h >/dev/null ; then\
		echo cp projects/ejs-macosx-default-bit.h $(CONFIG)/inc/bit.h  ; \
		cp projects/ejs-macosx-default-bit.h $(CONFIG)/inc/bit.h  ; \
	fi; true
clean:
	rm -rf $(CONFIG)/bin/libmpr.dylib
	rm -rf $(CONFIG)/bin/libmprssl.dylib
	rm -rf $(CONFIG)/bin/ejsman
	rm -rf $(CONFIG)/bin/makerom
	rm -rf $(CONFIG)/bin/libest.dylib
	rm -rf $(CONFIG)/bin/ca.crt
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
	rm -rf $(CONFIG)/bin/jem.es
	rm -rf $(CONFIG)/bin/jem
	rm -rf $(CONFIG)/bin/ejs.db.mod
	rm -rf $(CONFIG)/bin/ejs.db.mapper.mod
	rm -rf $(CONFIG)/bin/ejs.db.sqlite.mod
	rm -rf $(CONFIG)/bin/libejs.db.sqlite.dylib
	rm -rf $(CONFIG)/bin/ejs.web.mod
	rm -rf $(CONFIG)/bin/libejs.web.dylib
	rm -rf $(CONFIG)/bin/www
	rm -rf $(CONFIG)/bin/ejs.template.mod
	rm -rf $(CONFIG)/bin/libejs.zlib.dylib
	rm -rf $(CONFIG)/bin/mvc.es
	rm -rf $(CONFIG)/bin/ejs.mvc.mod
	rm -rf $(CONFIG)/bin/utest.es
	rm -rf $(CONFIG)/bin/utest.worker
	rm -rf $(CONFIG)/bin/utest
	rm -rf $(CONFIG)/obj/removeFiles.o
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
	mkdir -p "macosx-x64-default/inc"
	cp "src/deps/mpr/mpr.h" "macosx-x64-default/inc/mpr.h"

$(CONFIG)/inc/bit.h: 

$(CONFIG)/inc/bitos.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/bitos.h" "macosx-x64-default/inc/bitos.h"

$(CONFIG)/obj/mprLib.o: \
    src/deps/mpr/mprLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/inc/bitos.h
	$(CC) -c -o $(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprLib.c

$(CONFIG)/bin/libmpr.dylib: \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/obj/mprLib.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmpr.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libmpr.dylib $(CONFIG)/obj/mprLib.o $(LIBS)

$(CONFIG)/inc/est.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/deps/est/est.h" "macosx-x64-default/inc/est.h"

$(CONFIG)/obj/estLib.o: \
    src/deps/est/estLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/est.h \
    $(CONFIG)/inc/bitos.h
	$(CC) -c -o $(CONFIG)/obj/estLib.o $(DFLAGS) $(IFLAGS) src/deps/est/estLib.c

$(CONFIG)/bin/libest.dylib: \
    $(CONFIG)/inc/est.h \
    $(CONFIG)/obj/estLib.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libest.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libest.dylib $(CONFIG)/obj/estLib.o $(LIBS)

$(CONFIG)/obj/mprSsl.o: \
    src/deps/mpr/mprSsl.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/inc/est.h
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprSsl.c

$(CONFIG)/bin/libmprssl.dylib: \
    $(CONFIG)/bin/libmpr.dylib \
    $(CONFIG)/bin/libest.dylib \
    $(CONFIG)/obj/mprSsl.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmprssl.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libmprssl.dylib $(CONFIG)/obj/mprSsl.o -lest -lmpr $(LIBS)

$(CONFIG)/obj/manager.o: \
    src/deps/mpr/manager.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/manager.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/manager.c

$(CONFIG)/bin/ejsman: \
    $(CONFIG)/bin/libmpr.dylib \
    $(CONFIG)/obj/manager.o
	$(CC) -o $(CONFIG)/bin/ejsman -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/manager.o -lmpr $(LIBS)

$(CONFIG)/obj/makerom.o: \
    src/deps/mpr/makerom.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/makerom.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/makerom.c

$(CONFIG)/bin/makerom: \
    $(CONFIG)/bin/libmpr.dylib \
    $(CONFIG)/obj/makerom.o
	$(CC) -o $(CONFIG)/bin/makerom -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o -lmpr $(LIBS)

$(CONFIG)/bin/ca.crt: \
    src/deps/est/ca.crt
	mkdir -p "macosx-x64-default/bin"
	cp "src/deps/est/ca.crt" "macosx-x64-default/bin/ca.crt"

$(CONFIG)/inc/pcre.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/deps/pcre/pcre.h" "macosx-x64-default/inc/pcre.h"

$(CONFIG)/obj/pcre.o: \
    src/deps/pcre/pcre.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/pcre/pcre.c

$(CONFIG)/bin/libpcre.dylib: \
    $(CONFIG)/inc/pcre.h \
    $(CONFIG)/obj/pcre.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libpcre.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libpcre.dylib $(CONFIG)/obj/pcre.o $(LIBS)

$(CONFIG)/inc/http.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/deps/http/http.h" "macosx-x64-default/inc/http.h"

$(CONFIG)/obj/httpLib.o: \
    src/deps/http/httpLib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/http.h \
    $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/httpLib.c

$(CONFIG)/bin/libhttp.dylib: \
    $(CONFIG)/bin/libmpr.dylib \
    $(CONFIG)/bin/libpcre.dylib \
    $(CONFIG)/inc/http.h \
    $(CONFIG)/obj/httpLib.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libhttp.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libhttp.dylib $(CONFIG)/obj/httpLib.o -lpcre -lmpr $(LIBS) -lpam

$(CONFIG)/obj/http.o: \
    src/deps/http/http.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/http.h
	$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/http.c

$(CONFIG)/bin/http: \
    $(CONFIG)/bin/libhttp.dylib \
    $(CONFIG)/obj/http.o
	$(CC) -o $(CONFIG)/bin/http -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o -lhttp $(LIBS) -lpcre -lmpr -lpam

$(CONFIG)/inc/sqlite3.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/deps/sqlite/sqlite3.h" "macosx-x64-default/inc/sqlite3.h"

$(CONFIG)/obj/sqlite3.o: \
    src/deps/sqlite/sqlite3.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/sqlite3.h
	$(CC) -c -o $(CONFIG)/obj/sqlite3.o $(DFLAGS) $(IFLAGS) src/deps/sqlite/sqlite3.c

$(CONFIG)/bin/libsqlite3.dylib: \
    $(CONFIG)/inc/sqlite3.h \
    $(CONFIG)/obj/sqlite3.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libsqlite3.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libsqlite3.dylib $(CONFIG)/obj/sqlite3.o $(LIBS)

$(CONFIG)/obj/sqlite.o: \
    src/deps/sqlite/sqlite.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/sqlite3.h
	$(CC) -c -o $(CONFIG)/obj/sqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/sqlite/sqlite.c

$(CONFIG)/bin/sqlite: \
    $(CONFIG)/bin/libsqlite3.dylib \
    $(CONFIG)/obj/sqlite.o
	$(CC) -o $(CONFIG)/bin/sqlite -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/sqlite.o -lsqlite3 $(LIBS)

$(CONFIG)/inc/zlib.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/deps/zlib/zlib.h" "macosx-x64-default/inc/zlib.h"

$(CONFIG)/obj/zlib.o: \
    src/deps/zlib/zlib.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/zlib.h
	$(CC) -c -o $(CONFIG)/obj/zlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/zlib/zlib.c

$(CONFIG)/bin/libzlib.dylib: \
    $(CONFIG)/inc/zlib.h \
    $(CONFIG)/obj/zlib.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libzlib.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libzlib.dylib $(CONFIG)/obj/zlib.o $(LIBS)

$(CONFIG)/inc/ejs.cache.local.slots.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/slots/ejs.cache.local.slots.h" "macosx-x64-default/inc/ejs.cache.local.slots.h"

$(CONFIG)/inc/ejs.db.sqlite.slots.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/slots/ejs.db.sqlite.slots.h" "macosx-x64-default/inc/ejs.db.sqlite.slots.h"

$(CONFIG)/inc/ejs.slots.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/slots/ejs.slots.h" "macosx-x64-default/inc/ejs.slots.h"

$(CONFIG)/inc/ejs.web.slots.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/slots/ejs.web.slots.h" "macosx-x64-default/inc/ejs.web.slots.h"

$(CONFIG)/inc/ejs.zlib.slots.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/slots/ejs.zlib.slots.h" "macosx-x64-default/inc/ejs.zlib.slots.h"

$(CONFIG)/inc/ejsByteCode.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/ejsByteCode.h" "macosx-x64-default/inc/ejsByteCode.h"

$(CONFIG)/inc/ejsByteCodeTable.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/ejsByteCodeTable.h" "macosx-x64-default/inc/ejsByteCodeTable.h"

$(CONFIG)/inc/ejsCustomize.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/ejsCustomize.h" "macosx-x64-default/inc/ejsCustomize.h"

$(CONFIG)/inc/ejs.h: \
    $(CONFIG)/inc/mpr.h \
    $(CONFIG)/inc/http.h \
    $(CONFIG)/inc/ejsByteCode.h \
    $(CONFIG)/inc/ejsByteCodeTable.h \
    $(CONFIG)/inc/ejs.slots.h \
    $(CONFIG)/inc/ejsCustomize.h
	mkdir -p "macosx-x64-default/inc"
	cp "src/ejs.h" "macosx-x64-default/inc/ejs.h"

$(CONFIG)/inc/ejsCompiler.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/ejsCompiler.h" "macosx-x64-default/inc/ejsCompiler.h"

$(CONFIG)/obj/ecAst.o: \
    src/compiler/ecAst.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ecAst.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecAst.c

$(CONFIG)/obj/ecCodeGen.o: \
    src/compiler/ecCodeGen.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecCodeGen.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCodeGen.c

$(CONFIG)/obj/ecCompiler.o: \
    src/compiler/ecCompiler.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecCompiler.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCompiler.c

$(CONFIG)/obj/ecLex.o: \
    src/compiler/ecLex.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecLex.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecLex.c

$(CONFIG)/obj/ecModuleWrite.o: \
    src/compiler/ecModuleWrite.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecModuleWrite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecModuleWrite.c

$(CONFIG)/obj/ecParser.o: \
    src/compiler/ecParser.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecParser.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecParser.c

$(CONFIG)/obj/ecState.o: \
    src/compiler/ecState.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ecState.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecState.c

$(CONFIG)/obj/dtoa.o: \
    src/core/src/dtoa.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/dtoa.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/dtoa.c

$(CONFIG)/obj/ejsApp.o: \
    src/core/src/ejsApp.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsApp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsApp.c

$(CONFIG)/obj/ejsArray.o: \
    src/core/src/ejsArray.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsArray.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsArray.c

$(CONFIG)/obj/ejsBlock.o: \
    src/core/src/ejsBlock.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsBlock.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBlock.c

$(CONFIG)/obj/ejsBoolean.o: \
    src/core/src/ejsBoolean.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsBoolean.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBoolean.c

$(CONFIG)/obj/ejsByteArray.o: \
    src/core/src/ejsByteArray.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsByteArray.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsByteArray.c

$(CONFIG)/obj/ejsCache.o: \
    src/core/src/ejsCache.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsCache.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCache.c

$(CONFIG)/obj/ejsCmd.o: \
    src/core/src/ejsCmd.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsCmd.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCmd.c

$(CONFIG)/obj/ejsConfig.o: \
    src/core/src/ejsConfig.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsConfig.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsConfig.c

$(CONFIG)/obj/ejsDate.o: \
    src/core/src/ejsDate.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsDate.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDate.c

$(CONFIG)/obj/ejsDebug.o: \
    src/core/src/ejsDebug.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsDebug.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDebug.c

$(CONFIG)/obj/ejsError.o: \
    src/core/src/ejsError.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsError.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsError.c

$(CONFIG)/obj/ejsFile.o: \
    src/core/src/ejsFile.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFile.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFile.c

$(CONFIG)/obj/ejsFileSystem.o: \
    src/core/src/ejsFileSystem.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFileSystem.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFileSystem.c

$(CONFIG)/obj/ejsFrame.o: \
    src/core/src/ejsFrame.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFrame.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFrame.c

$(CONFIG)/obj/ejsFunction.o: \
    src/core/src/ejsFunction.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsFunction.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFunction.c

$(CONFIG)/obj/ejsGC.o: \
    src/core/src/ejsGC.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsGC.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGC.c

$(CONFIG)/obj/ejsGlobal.o: \
    src/core/src/ejsGlobal.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsGlobal.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGlobal.c

$(CONFIG)/obj/ejsHttp.o: \
    src/core/src/ejsHttp.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsHttp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsHttp.c

$(CONFIG)/obj/ejsIterator.o: \
    src/core/src/ejsIterator.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsIterator.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsIterator.c

$(CONFIG)/obj/ejsJSON.o: \
    src/core/src/ejsJSON.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsJSON.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsJSON.c

$(CONFIG)/obj/ejsLocalCache.o: \
    src/core/src/ejsLocalCache.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsLocalCache.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsLocalCache.c

$(CONFIG)/obj/ejsMath.o: \
    src/core/src/ejsMath.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsMath.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMath.c

$(CONFIG)/obj/ejsMemory.o: \
    src/core/src/ejsMemory.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsMemory.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMemory.c

$(CONFIG)/obj/ejsMprLog.o: \
    src/core/src/ejsMprLog.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsMprLog.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMprLog.c

$(CONFIG)/obj/ejsNamespace.o: \
    src/core/src/ejsNamespace.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsNamespace.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNamespace.c

$(CONFIG)/obj/ejsNull.o: \
    src/core/src/ejsNull.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsNull.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNull.c

$(CONFIG)/obj/ejsNumber.o: \
    src/core/src/ejsNumber.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsNumber.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNumber.c

$(CONFIG)/obj/ejsObject.o: \
    src/core/src/ejsObject.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsObject.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsObject.c

$(CONFIG)/obj/ejsPath.o: \
    src/core/src/ejsPath.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/ejsPath.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPath.c

$(CONFIG)/obj/ejsPot.o: \
    src/core/src/ejsPot.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsPot.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPot.c

$(CONFIG)/obj/ejsRegExp.o: \
    src/core/src/ejsRegExp.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/ejsRegExp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsRegExp.c

$(CONFIG)/obj/ejsSocket.o: \
    src/core/src/ejsSocket.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsSocket.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSocket.c

$(CONFIG)/obj/ejsString.o: \
    src/core/src/ejsString.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/ejsString.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsString.c

$(CONFIG)/obj/ejsSystem.o: \
    src/core/src/ejsSystem.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsSystem.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSystem.c

$(CONFIG)/obj/ejsTimer.o: \
    src/core/src/ejsTimer.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsTimer.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsTimer.c

$(CONFIG)/obj/ejsType.o: \
    src/core/src/ejsType.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsType.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsType.c

$(CONFIG)/obj/ejsUri.o: \
    src/core/src/ejsUri.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsUri.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsUri.c

$(CONFIG)/obj/ejsVoid.o: \
    src/core/src/ejsVoid.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsVoid.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsVoid.c

$(CONFIG)/obj/ejsWebSocket.o: \
    src/core/src/ejsWebSocket.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsWebSocket.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWebSocket.c

$(CONFIG)/obj/ejsWorker.o: \
    src/core/src/ejsWorker.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsWorker.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWorker.c

$(CONFIG)/obj/ejsXML.o: \
    src/core/src/ejsXML.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsXML.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXML.c

$(CONFIG)/obj/ejsXMLList.o: \
    src/core/src/ejsXMLList.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsXMLList.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLList.c

$(CONFIG)/obj/ejsXMLLoader.o: \
    src/core/src/ejsXMLLoader.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsXMLLoader.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLLoader.c

$(CONFIG)/obj/ejsByteCode.o: \
    src/vm/ejsByteCode.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsByteCode.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsByteCode.c

$(CONFIG)/obj/ejsException.o: \
    src/vm/ejsException.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsException.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsException.c

$(CONFIG)/obj/ejsHelper.o: \
    src/vm/ejsHelper.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsHelper.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsHelper.c

$(CONFIG)/obj/ejsInterp.o: \
    src/vm/ejsInterp.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsInterp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsInterp.c

$(CONFIG)/obj/ejsLoader.o: \
    src/vm/ejsLoader.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsLoader.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsLoader.c

$(CONFIG)/obj/ejsModule.o: \
    src/vm/ejsModule.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsModule.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsModule.c

$(CONFIG)/obj/ejsScope.o: \
    src/vm/ejsScope.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsScope.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsScope.c

$(CONFIG)/obj/ejsService.o: \
    src/vm/ejsService.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsService.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsService.c

$(CONFIG)/bin/libejs.dylib: \
    $(CONFIG)/bin/libhttp.dylib \
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
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libejs.dylib $(CONFIG)/obj/ecAst.o $(CONFIG)/obj/ecCodeGen.o $(CONFIG)/obj/ecCompiler.o $(CONFIG)/obj/ecLex.o $(CONFIG)/obj/ecModuleWrite.o $(CONFIG)/obj/ecParser.o $(CONFIG)/obj/ecState.o $(CONFIG)/obj/dtoa.o $(CONFIG)/obj/ejsApp.o $(CONFIG)/obj/ejsArray.o $(CONFIG)/obj/ejsBlock.o $(CONFIG)/obj/ejsBoolean.o $(CONFIG)/obj/ejsByteArray.o $(CONFIG)/obj/ejsCache.o $(CONFIG)/obj/ejsCmd.o $(CONFIG)/obj/ejsConfig.o $(CONFIG)/obj/ejsDate.o $(CONFIG)/obj/ejsDebug.o $(CONFIG)/obj/ejsError.o $(CONFIG)/obj/ejsFile.o $(CONFIG)/obj/ejsFileSystem.o $(CONFIG)/obj/ejsFrame.o $(CONFIG)/obj/ejsFunction.o $(CONFIG)/obj/ejsGC.o $(CONFIG)/obj/ejsGlobal.o $(CONFIG)/obj/ejsHttp.o $(CONFIG)/obj/ejsIterator.o $(CONFIG)/obj/ejsJSON.o $(CONFIG)/obj/ejsLocalCache.o $(CONFIG)/obj/ejsMath.o $(CONFIG)/obj/ejsMemory.o $(CONFIG)/obj/ejsMprLog.o $(CONFIG)/obj/ejsNamespace.o $(CONFIG)/obj/ejsNull.o $(CONFIG)/obj/ejsNumber.o $(CONFIG)/obj/ejsObject.o $(CONFIG)/obj/ejsPath.o $(CONFIG)/obj/ejsPot.o $(CONFIG)/obj/ejsRegExp.o $(CONFIG)/obj/ejsSocket.o $(CONFIG)/obj/ejsString.o $(CONFIG)/obj/ejsSystem.o $(CONFIG)/obj/ejsTimer.o $(CONFIG)/obj/ejsType.o $(CONFIG)/obj/ejsUri.o $(CONFIG)/obj/ejsVoid.o $(CONFIG)/obj/ejsWebSocket.o $(CONFIG)/obj/ejsWorker.o $(CONFIG)/obj/ejsXML.o $(CONFIG)/obj/ejsXMLList.o $(CONFIG)/obj/ejsXMLLoader.o $(CONFIG)/obj/ejsByteCode.o $(CONFIG)/obj/ejsException.o $(CONFIG)/obj/ejsHelper.o $(CONFIG)/obj/ejsInterp.o $(CONFIG)/obj/ejsLoader.o $(CONFIG)/obj/ejsModule.o $(CONFIG)/obj/ejsScope.o $(CONFIG)/obj/ejsService.o -lhttp $(LIBS) -lpcre -lmpr -lpam

$(CONFIG)/obj/ejs.o: \
    src/cmd/ejs.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejs.c

$(CONFIG)/bin/ejs: \
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/obj/ejs.o
	$(CC) -o $(CONFIG)/bin/ejs -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejs.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lpam -ledit

$(CONFIG)/obj/ejsc.o: \
    src/cmd/ejsc.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsc.c

$(CONFIG)/bin/ejsc: \
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/obj/ejsc.o
	$(CC) -o $(CONFIG)/bin/ejsc -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsc.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lpam

src/cmd/ejsmod.h: 

$(CONFIG)/obj/ejsmod.o: \
    src/cmd/ejsmod.c\
    $(CONFIG)/inc/bit.h \
    src/cmd/ejsmod.h \
    $(CONFIG)/inc/ejs.h
	$(CC) -c -o $(CONFIG)/obj/ejsmod.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/ejsmod.c

$(CONFIG)/obj/doc.o: \
    src/cmd/doc.c\
    $(CONFIG)/inc/bit.h \
    src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/doc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/doc.c

$(CONFIG)/obj/docFiles.o: \
    src/cmd/docFiles.c\
    $(CONFIG)/inc/bit.h \
    src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/docFiles.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/docFiles.c

$(CONFIG)/obj/listing.o: \
    src/cmd/listing.c\
    $(CONFIG)/inc/bit.h \
    src/cmd/ejsmod.h \
    $(CONFIG)/inc/ejsByteCodeTable.h
	$(CC) -c -o $(CONFIG)/obj/listing.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/listing.c

$(CONFIG)/obj/slotGen.o: \
    src/cmd/slotGen.c\
    $(CONFIG)/inc/bit.h \
    src/cmd/ejsmod.h
	$(CC) -c -o $(CONFIG)/obj/slotGen.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/slotGen.c

$(CONFIG)/bin/ejsmod: \
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/obj/ejsmod.o \
    $(CONFIG)/obj/doc.o \
    $(CONFIG)/obj/docFiles.o \
    $(CONFIG)/obj/listing.o \
    $(CONFIG)/obj/slotGen.o
	$(CC) -o $(CONFIG)/bin/ejsmod -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsmod.o $(CONFIG)/obj/doc.o $(CONFIG)/obj/docFiles.o $(CONFIG)/obj/listing.o $(CONFIG)/obj/slotGen.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lpam

$(CONFIG)/obj/ejsrun.o: \
    src/cmd/ejsrun.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejsCompiler.h
	$(CC) -c -o $(CONFIG)/obj/ejsrun.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsrun.c

$(CONFIG)/bin/ejsrun: \
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/obj/ejsrun.o
	$(CC) -o $(CONFIG)/bin/ejsrun -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lpam

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
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/bin/jem.es \
    $(CONFIG)/obj/ejsrun.o
	$(CC) -o $(CONFIG)/bin/jem -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lpam

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
	$(CC) -c -o $(CONFIG)/obj/ejsSqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/jems/ejs.db.sqlite/ejsSqlite.c

$(CONFIG)/bin/libejs.db.sqlite.dylib: \
    $(CONFIG)/bin/libmpr.dylib \
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/bin/ejs.mod \
    $(CONFIG)/bin/ejs.db.sqlite.mod \
    $(CONFIG)/bin/libsqlite3.dylib \
    $(CONFIG)/obj/ejsSqlite.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.db.sqlite.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libejs.db.sqlite.dylib $(CONFIG)/obj/ejsSqlite.o -lsqlite3 -lejs -lmpr $(LIBS) -lhttp -lpcre -lpam

$(CONFIG)/bin/ejs.web.mod: \
    $(CONFIG)/bin/ejsc \
    $(CONFIG)/bin/ejsmod \
    $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.web; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.web.mod  --optimize 9 *.es ; cd ../../..
	cd src/jems/ejs.web; ../../../$(CONFIG)/bin/ejsmod --cslots ../../../$(CONFIG)/bin/ejs.web.mod ; cd ../../..
	cd src/jems/ejs.web; if ! diff ejs.web.slots.h ../../../$(CONFIG)/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../../$(CONFIG)/inc; fi ; cd ../../..
	cd src/jems/ejs.web; rm -f ejs.web.slots.h ; cd ../../..

$(CONFIG)/inc/ejsWeb.h: 
	mkdir -p "macosx-x64-default/inc"
	cp "src/jems/ejs.web/ejsWeb.h" "macosx-x64-default/inc/ejsWeb.h"

$(CONFIG)/obj/ejsHttpServer.o: \
    src/jems/ejs.web/ejsHttpServer.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/ejsCompiler.h \
    $(CONFIG)/inc/ejsWeb.h \
    $(CONFIG)/inc/ejs.web.slots.h \
    $(CONFIG)/inc/http.h
	$(CC) -c -o $(CONFIG)/obj/ejsHttpServer.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsHttpServer.c

$(CONFIG)/obj/ejsRequest.o: \
    src/jems/ejs.web/ejsRequest.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/ejsCompiler.h \
    $(CONFIG)/inc/ejsWeb.h \
    $(CONFIG)/inc/ejs.web.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsRequest.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsRequest.c

$(CONFIG)/obj/ejsSession.o: \
    src/jems/ejs.web/ejsSession.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/ejsWeb.h
	$(CC) -c -o $(CONFIG)/obj/ejsSession.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsSession.c

$(CONFIG)/obj/ejsWeb.o: \
    src/jems/ejs.web/ejsWeb.c\
    $(CONFIG)/inc/bit.h \
    $(CONFIG)/inc/ejs.h \
    $(CONFIG)/inc/ejsCompiler.h \
    $(CONFIG)/inc/ejsWeb.h \
    $(CONFIG)/inc/ejs.web.slots.h
	$(CC) -c -o $(CONFIG)/obj/ejsWeb.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsWeb.c

$(CONFIG)/bin/libejs.web.dylib: \
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/bin/ejs.mod \
    $(CONFIG)/inc/ejsWeb.h \
    $(CONFIG)/obj/ejsHttpServer.o \
    $(CONFIG)/obj/ejsRequest.o \
    $(CONFIG)/obj/ejsSession.o \
    $(CONFIG)/obj/ejsWeb.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.web.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libejs.web.dylib $(CONFIG)/obj/ejsHttpServer.o $(CONFIG)/obj/ejsRequest.o $(CONFIG)/obj/ejsSession.o $(CONFIG)/obj/ejsWeb.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lpam

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
	$(CC) -c -o $(CONFIG)/obj/ejsZlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/jems/ejs.zlib/ejsZlib.c

$(CONFIG)/bin/libejs.zlib.dylib: \
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/bin/ejs.mod \
    $(CONFIG)/bin/ejs.zlib.mod \
    $(CONFIG)/bin/libzlib.dylib \
    $(CONFIG)/obj/ejsZlib.o
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.zlib.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libejs.zlib.dylib $(CONFIG)/obj/ejsZlib.o -lzlib -lejs $(LIBS) -lhttp -lpcre -lmpr -lpam

$(CONFIG)/bin/ejs.tar.mod: \
    $(CONFIG)/bin/ejsc \
    $(CONFIG)/bin/ejs.mod
	cd src/jems/ejs.tar; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.tar.mod  --optimize 9 *.es ; cd ../../..

$(CONFIG)/bin/mvc.es: 
	cd src/jems/ejs.mvc; cp mvc.es ../../../$(CONFIG)/bin ; cd ../../..

$(CONFIG)/bin/mvc: \
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/bin/mvc.es \
    $(CONFIG)/obj/ejsrun.o
	$(CC) -o $(CONFIG)/bin/mvc -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lpam

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
    $(CONFIG)/bin/libejs.dylib \
    $(CONFIG)/bin/utest.es \
    $(CONFIG)/bin/utest.worker \
    $(CONFIG)/obj/ejsrun.o
	$(CC) -o $(CONFIG)/bin/utest -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o -lejs $(LIBS) -lhttp -lpcre -lmpr -lpam

version: 
	@echo 2.3.0-1

stop: 
	

installBinary: stop
	mkdir -p "/usr/local/lib/ejs/2.3.0/bin"
	mkdir -p "/usr/local/lib/ejs/2.3.0/bin/www"
	mkdir -p "/usr/local/lib/ejs/2.3.0/bin/www/images"
	cp "src/jems/ejs.web/www/images/banner.jpg" "/usr/local/lib/ejs/2.3.0/bin/www/images/banner.jpg"
	cp "src/jems/ejs.web/www/images/favicon.ico" "/usr/local/lib/ejs/2.3.0/bin/www/images/favicon.ico"
	cp "src/jems/ejs.web/www/images/splash.jpg" "/usr/local/lib/ejs/2.3.0/bin/www/images/splash.jpg"
	mkdir -p "/usr/local/lib/ejs/2.3.0/bin/www/js"
	cp "src/jems/ejs.web/www/js/jquery.ejs.min.js" "/usr/local/lib/ejs/2.3.0/bin/www/js/jquery.ejs.min.js"
	cp "src/jems/ejs.web/www/js/jquery.min.js" "/usr/local/lib/ejs/2.3.0/bin/www/js/jquery.min.js"
	cp "src/jems/ejs.web/www/js/jquery.simplemodal.min.js" "/usr/local/lib/ejs/2.3.0/bin/www/js/jquery.simplemodal.min.js"
	cp "src/jems/ejs.web/www/js/jquery.tablesorter.js" "/usr/local/lib/ejs/2.3.0/bin/www/js/jquery.tablesorter.js"
	cp "src/jems/ejs.web/www/js/jquery.tablesorter.min.js" "/usr/local/lib/ejs/2.3.0/bin/www/js/jquery.tablesorter.min.js"
	cp "src/jems/ejs.web/www/js/jquery.treeview.min.js" "/usr/local/lib/ejs/2.3.0/bin/www/js/jquery.treeview.min.js"
	mkdir -p "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images"
	cp "src/jems/ejs.web/www/js/tree-images/file.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/file.gif"
	cp "src/jems/ejs.web/www/js/tree-images/folder-closed.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/folder-closed.gif"
	cp "src/jems/ejs.web/www/js/tree-images/folder.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/folder.gif"
	cp "src/jems/ejs.web/www/js/tree-images/minus.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/minus.gif"
	cp "src/jems/ejs.web/www/js/tree-images/plus.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/plus.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-black-line.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/treeview-black-line.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-black.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/treeview-black.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-default-line.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/treeview-default-line.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-default.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/treeview-default.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-famfamfam-line.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/treeview-famfamfam-line.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-famfamfam.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/treeview-famfamfam.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-gray-line.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/treeview-gray-line.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-gray.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/treeview-gray.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-red-line.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/treeview-red-line.gif"
	cp "src/jems/ejs.web/www/js/tree-images/treeview-red.gif" "/usr/local/lib/ejs/2.3.0/bin/www/js/tree-images/treeview-red.gif"
	cp "src/jems/ejs.web/www/js/treeview.css" "/usr/local/lib/ejs/2.3.0/bin/www/js/treeview.css"
	cp "src/jems/ejs.web/www/layout.css" "/usr/local/lib/ejs/2.3.0/bin/www/layout.css"
	mkdir -p "/usr/local/lib/ejs/2.3.0/bin/www/themes"
	cp "src/jems/ejs.web/www/themes/default.css" "/usr/local/lib/ejs/2.3.0/bin/www/themes/default.css"
	rm -f "/usr/local/lib/ejs/latest"
	mkdir -p "/usr/local/lib/ejs"
	ln -s "2.3.0" "/usr/local/lib/ejs/latest"


start: 
	

install: stop installBinary start
	

uninstall: stop
	rmdir -p "/usr/local/lib/ejs"
	rmdir -p "/usr/local/lib/ejs/2.3.0"


