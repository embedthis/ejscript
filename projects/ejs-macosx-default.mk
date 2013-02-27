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

BIT_PACK_EST          := 0
BIT_PACK_SQLITE       := 1

BIT_ROOT_PREFIX       := 
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


TARGETS     += $(CONFIG)/bin/libmpr.dylib
TARGETS     += $(CONFIG)/bin/ejsman
TARGETS     += $(CONFIG)/bin/makerom
TARGETS     += $(CONFIG)/bin/ca.crt
TARGETS     += $(CONFIG)/bin/libpcre.dylib
TARGETS     += $(CONFIG)/bin/libhttp.dylib
TARGETS     += $(CONFIG)/bin/http
ifeq ($(BIT_PACK_SQLITE),1)
TARGETS += $(CONFIG)/bin/libsqlite3.dylib
endif
ifeq ($(BIT_PACK_SQLITE),1)
TARGETS += $(CONFIG)/bin/sqlite
endif
TARGETS     += $(CONFIG)/bin/libzlib.dylib
TARGETS     += $(CONFIG)/bin/libejs.dylib
TARGETS     += $(CONFIG)/bin/ejs
TARGETS     += $(CONFIG)/bin/ejsc
TARGETS     += $(CONFIG)/bin/ejsmod
TARGETS     += $(CONFIG)/bin/ejsrun
TARGETS     += $(CONFIG)/bin/ejs.mod
TARGETS     += $(CONFIG)/bin/ejs.unix.mod
TARGETS     += $(CONFIG)/bin/jem.es
TARGETS     += $(CONFIG)/bin/jem
TARGETS     += $(CONFIG)/bin/ejs.db.mod
TARGETS     += $(CONFIG)/bin/ejs.db.mapper.mod
TARGETS     += $(CONFIG)/bin/ejs.db.sqlite.mod
TARGETS     += $(CONFIG)/bin/libejs.db.sqlite.dylib
TARGETS     += $(CONFIG)/bin/ejs.web.mod
TARGETS     += $(CONFIG)/bin/libejs.web.dylib
TARGETS     += $(CONFIG)/bin/www
TARGETS     += $(CONFIG)/bin/ejs.template.mod
TARGETS     += $(CONFIG)/bin/ejs.zlib.mod
TARGETS     += $(CONFIG)/bin/libejs.zlib.dylib
TARGETS     += $(CONFIG)/bin/ejs.tar.mod
TARGETS     += $(CONFIG)/bin/mvc.es
TARGETS     += $(CONFIG)/bin/mvc
TARGETS     += $(CONFIG)/bin/ejs.mvc.mod
TARGETS     += $(CONFIG)/bin/utest.es
TARGETS     += $(CONFIG)/bin/utest.worker
TARGETS     += $(CONFIG)/bin/utest

unexport CDPATH

ifndef SHOW
.SILENT:
endif

all build compile: prep $(TARGETS)

.PHONY: prep

prep:
	@echo "      [Info] Use "make SHOW=1" to trace executed commands."
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
	rm -rf $(CONFIG)/bin/ejsman
	rm -rf $(CONFIG)/bin/makerom
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
	rm -rf $(CONFIG)/obj/mprLib.o
	rm -rf $(CONFIG)/obj/manager.o
	rm -rf $(CONFIG)/obj/makerom.o
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

#
#   mpr.h
#
$(CONFIG)/inc/mpr.h: $(DEPS_1)
	@echo '      [File] $(CONFIG)/inc/mpr.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/mpr/mpr.h" "$(CONFIG)/inc/mpr.h"

#
#   bit.h
#
$(CONFIG)/inc/bit.h: $(DEPS_2)

#
#   bitos.h
#
$(CONFIG)/inc/bitos.h: $(DEPS_3)
	@echo '      [File] $(CONFIG)/inc/bitos.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/bitos.h" "$(CONFIG)/inc/bitos.h"

#
#   mprLib.o
#
DEPS_4 += $(CONFIG)/inc/bit.h
DEPS_4 += $(CONFIG)/inc/mpr.h
DEPS_4 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/mprLib.o: \
    src/deps/mpr/mprLib.c $(DEPS_4)
	@echo '   [Compile] src/deps/mpr/mprLib.c'
	$(CC) -c -o $(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprLib.c

#
#   libmpr
#
DEPS_5 += $(CONFIG)/inc/mpr.h
DEPS_5 += $(CONFIG)/obj/mprLib.o

$(CONFIG)/bin/libmpr.dylib: $(DEPS_5)
	@echo '      [Link] libmpr'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libmpr.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libmpr.dylib $(CONFIG)/obj/mprLib.o $(LIBS)

#
#   manager.o
#
DEPS_6 += $(CONFIG)/inc/bit.h
DEPS_6 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/manager.o: \
    src/deps/mpr/manager.c $(DEPS_6)
	@echo '   [Compile] src/deps/mpr/manager.c'
	$(CC) -c -o $(CONFIG)/obj/manager.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/manager.c

#
#   manager
#
DEPS_7 += $(CONFIG)/bin/libmpr.dylib
DEPS_7 += $(CONFIG)/obj/manager.o

LIBS_7 += -lmpr

$(CONFIG)/bin/ejsman: $(DEPS_7)
	@echo '      [Link] manager'
	$(CC) -o $(CONFIG)/bin/ejsman -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/manager.o $(LIBS_7) $(LIBS_7) $(LIBS)

#
#   makerom.o
#
DEPS_8 += $(CONFIG)/inc/bit.h
DEPS_8 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/makerom.o: \
    src/deps/mpr/makerom.c $(DEPS_8)
	@echo '   [Compile] src/deps/mpr/makerom.c'
	$(CC) -c -o $(CONFIG)/obj/makerom.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/makerom.c

#
#   makerom
#
DEPS_9 += $(CONFIG)/bin/libmpr.dylib
DEPS_9 += $(CONFIG)/obj/makerom.o

LIBS_9 += -lmpr

$(CONFIG)/bin/makerom: $(DEPS_9)
	@echo '      [Link] makerom'
	$(CC) -o $(CONFIG)/bin/makerom -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o $(LIBS_9) $(LIBS_9) $(LIBS)

#
#   ca-crt
#
DEPS_10 += src/deps/est/ca.crt

$(CONFIG)/bin/ca.crt: $(DEPS_10)
	@echo '      [File] $(CONFIG)/bin/ca.crt'
	mkdir -p "$(CONFIG)/bin"
	cp "src/deps/est/ca.crt" "$(CONFIG)/bin/ca.crt"

#
#   pcre.h
#
$(CONFIG)/inc/pcre.h: $(DEPS_11)
	@echo '      [File] $(CONFIG)/inc/pcre.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/pcre/pcre.h" "$(CONFIG)/inc/pcre.h"

#
#   pcre.o
#
DEPS_12 += $(CONFIG)/inc/bit.h
DEPS_12 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
    src/deps/pcre/pcre.c $(DEPS_12)
	@echo '   [Compile] src/deps/pcre/pcre.c'
	$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/pcre/pcre.c

#
#   libpcre
#
DEPS_13 += $(CONFIG)/inc/pcre.h
DEPS_13 += $(CONFIG)/obj/pcre.o

$(CONFIG)/bin/libpcre.dylib: $(DEPS_13)
	@echo '      [Link] libpcre'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libpcre.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libpcre.dylib $(CONFIG)/obj/pcre.o $(LIBS)

#
#   http.h
#
$(CONFIG)/inc/http.h: $(DEPS_14)
	@echo '      [File] $(CONFIG)/inc/http.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/http/http.h" "$(CONFIG)/inc/http.h"

#
#   httpLib.o
#
DEPS_15 += $(CONFIG)/inc/bit.h
DEPS_15 += $(CONFIG)/inc/http.h
DEPS_15 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/httpLib.o: \
    src/deps/http/httpLib.c $(DEPS_15)
	@echo '   [Compile] src/deps/http/httpLib.c'
	$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/httpLib.c

#
#   libhttp
#
DEPS_16 += $(CONFIG)/bin/libmpr.dylib
DEPS_16 += $(CONFIG)/bin/libpcre.dylib
DEPS_16 += $(CONFIG)/inc/http.h
DEPS_16 += $(CONFIG)/obj/httpLib.o

LIBS_16 += -lpcre
LIBS_16 += -lmpr

$(CONFIG)/bin/libhttp.dylib: $(DEPS_16)
	@echo '      [Link] libhttp'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libhttp.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libhttp.dylib $(CONFIG)/obj/httpLib.o $(LIBS_16) $(LIBS_16) $(LIBS) -lpam

#
#   http.o
#
DEPS_17 += $(CONFIG)/inc/bit.h
DEPS_17 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/http.o: \
    src/deps/http/http.c $(DEPS_17)
	@echo '   [Compile] src/deps/http/http.c'
	$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/http.c

#
#   http
#
DEPS_18 += $(CONFIG)/bin/libhttp.dylib
DEPS_18 += $(CONFIG)/obj/http.o

LIBS_18 += -lhttp
LIBS_18 += -lpcre
LIBS_18 += -lmpr

$(CONFIG)/bin/http: $(DEPS_18)
	@echo '      [Link] http'
	$(CC) -o $(CONFIG)/bin/http -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o $(LIBS_18) $(LIBS_18) $(LIBS) -lpam

#
#   sqlite3.h
#
$(CONFIG)/inc/sqlite3.h: $(DEPS_19)
	@echo '      [File] $(CONFIG)/inc/sqlite3.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/sqlite/sqlite3.h" "$(CONFIG)/inc/sqlite3.h"

#
#   sqlite3.o
#
DEPS_20 += $(CONFIG)/inc/bit.h
DEPS_20 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite3.o: \
    src/deps/sqlite/sqlite3.c $(DEPS_20)
	@echo '   [Compile] src/deps/sqlite/sqlite3.c'
	$(CC) -c -o $(CONFIG)/obj/sqlite3.o $(DFLAGS) $(IFLAGS) src/deps/sqlite/sqlite3.c

ifeq ($(BIT_PACK_SQLITE),1)
#
#   libsqlite3
#
DEPS_21 += $(CONFIG)/inc/sqlite3.h
DEPS_21 += $(CONFIG)/obj/sqlite3.o

$(CONFIG)/bin/libsqlite3.dylib: $(DEPS_21)
	@echo '      [Link] libsqlite3'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libsqlite3.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libsqlite3.dylib $(CONFIG)/obj/sqlite3.o $(LIBS)
endif

#
#   sqlite.o
#
DEPS_22 += $(CONFIG)/inc/bit.h
DEPS_22 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite.o: \
    src/deps/sqlite/sqlite.c $(DEPS_22)
	@echo '   [Compile] src/deps/sqlite/sqlite.c'
	$(CC) -c -o $(CONFIG)/obj/sqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/sqlite/sqlite.c

ifeq ($(BIT_PACK_SQLITE),1)
#
#   sqlite
#
ifeq ($(BIT_PACK_SQLITE),1)
    DEPS_23 += $(CONFIG)/bin/libsqlite3.dylib
endif
DEPS_23 += $(CONFIG)/obj/sqlite.o

ifeq ($(BIT_PACK_SQLITE),1)
    LIBS_23 += -lsqlite3
endif

$(CONFIG)/bin/sqlite: $(DEPS_23)
	@echo '      [Link] sqlite'
	$(CC) -o $(CONFIG)/bin/sqlite -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/sqlite.o $(LIBS_23) $(LIBS_23) $(LIBS)
endif

#
#   zlib.h
#
$(CONFIG)/inc/zlib.h: $(DEPS_24)
	@echo '      [File] $(CONFIG)/inc/zlib.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/zlib/zlib.h" "$(CONFIG)/inc/zlib.h"

#
#   zlib.o
#
DEPS_25 += $(CONFIG)/inc/bit.h
DEPS_25 += $(CONFIG)/inc/zlib.h

$(CONFIG)/obj/zlib.o: \
    src/deps/zlib/zlib.c $(DEPS_25)
	@echo '   [Compile] src/deps/zlib/zlib.c'
	$(CC) -c -o $(CONFIG)/obj/zlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/zlib/zlib.c

#
#   libzlib
#
DEPS_26 += $(CONFIG)/inc/zlib.h
DEPS_26 += $(CONFIG)/obj/zlib.o

$(CONFIG)/bin/libzlib.dylib: $(DEPS_26)
	@echo '      [Link] libzlib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libzlib.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libzlib.dylib $(CONFIG)/obj/zlib.o $(LIBS)

#
#   ejs.cache.local.slots.h
#
$(CONFIG)/inc/ejs.cache.local.slots.h: $(DEPS_27)
	@echo '      [File] $(CONFIG)/inc/ejs.cache.local.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.cache.local.slots.h" "$(CONFIG)/inc/ejs.cache.local.slots.h"

#
#   ejs.db.sqlite.slots.h
#
$(CONFIG)/inc/ejs.db.sqlite.slots.h: $(DEPS_28)
	@echo '      [File] $(CONFIG)/inc/ejs.db.sqlite.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.db.sqlite.slots.h" "$(CONFIG)/inc/ejs.db.sqlite.slots.h"

#
#   ejs.slots.h
#
$(CONFIG)/inc/ejs.slots.h: $(DEPS_29)
	@echo '      [File] $(CONFIG)/inc/ejs.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.slots.h" "$(CONFIG)/inc/ejs.slots.h"

#
#   ejs.web.slots.h
#
$(CONFIG)/inc/ejs.web.slots.h: $(DEPS_30)
	@echo '      [File] $(CONFIG)/inc/ejs.web.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.web.slots.h" "$(CONFIG)/inc/ejs.web.slots.h"

#
#   ejs.zlib.slots.h
#
$(CONFIG)/inc/ejs.zlib.slots.h: $(DEPS_31)
	@echo '      [File] $(CONFIG)/inc/ejs.zlib.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.zlib.slots.h" "$(CONFIG)/inc/ejs.zlib.slots.h"

#
#   ejsByteCode.h
#
$(CONFIG)/inc/ejsByteCode.h: $(DEPS_32)
	@echo '      [File] $(CONFIG)/inc/ejsByteCode.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsByteCode.h" "$(CONFIG)/inc/ejsByteCode.h"

#
#   ejsByteCodeTable.h
#
$(CONFIG)/inc/ejsByteCodeTable.h: $(DEPS_33)
	@echo '      [File] $(CONFIG)/inc/ejsByteCodeTable.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsByteCodeTable.h" "$(CONFIG)/inc/ejsByteCodeTable.h"

#
#   ejsCustomize.h
#
$(CONFIG)/inc/ejsCustomize.h: $(DEPS_34)
	@echo '      [File] $(CONFIG)/inc/ejsCustomize.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsCustomize.h" "$(CONFIG)/inc/ejsCustomize.h"

#
#   ejs.h
#
DEPS_35 += $(CONFIG)/inc/mpr.h
DEPS_35 += $(CONFIG)/inc/http.h
DEPS_35 += $(CONFIG)/inc/ejsByteCode.h
DEPS_35 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_35 += $(CONFIG)/inc/ejs.slots.h
DEPS_35 += $(CONFIG)/inc/ejsCustomize.h

$(CONFIG)/inc/ejs.h: $(DEPS_35)
	@echo '      [File] $(CONFIG)/inc/ejs.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejs.h" "$(CONFIG)/inc/ejs.h"

#
#   ejsCompiler.h
#
$(CONFIG)/inc/ejsCompiler.h: $(DEPS_36)
	@echo '      [File] $(CONFIG)/inc/ejsCompiler.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsCompiler.h" "$(CONFIG)/inc/ejsCompiler.h"

#
#   ecAst.o
#
DEPS_37 += $(CONFIG)/inc/bit.h
DEPS_37 += $(CONFIG)/inc/ejsCompiler.h
DEPS_37 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ecAst.o: \
    src/compiler/ecAst.c $(DEPS_37)
	@echo '   [Compile] src/compiler/ecAst.c'
	$(CC) -c -o $(CONFIG)/obj/ecAst.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecAst.c

#
#   ecCodeGen.o
#
DEPS_38 += $(CONFIG)/inc/bit.h
DEPS_38 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecCodeGen.o: \
    src/compiler/ecCodeGen.c $(DEPS_38)
	@echo '   [Compile] src/compiler/ecCodeGen.c'
	$(CC) -c -o $(CONFIG)/obj/ecCodeGen.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCodeGen.c

#
#   ecCompiler.o
#
DEPS_39 += $(CONFIG)/inc/bit.h
DEPS_39 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecCompiler.o: \
    src/compiler/ecCompiler.c $(DEPS_39)
	@echo '   [Compile] src/compiler/ecCompiler.c'
	$(CC) -c -o $(CONFIG)/obj/ecCompiler.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCompiler.c

#
#   ecLex.o
#
DEPS_40 += $(CONFIG)/inc/bit.h
DEPS_40 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecLex.o: \
    src/compiler/ecLex.c $(DEPS_40)
	@echo '   [Compile] src/compiler/ecLex.c'
	$(CC) -c -o $(CONFIG)/obj/ecLex.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecLex.c

#
#   ecModuleWrite.o
#
DEPS_41 += $(CONFIG)/inc/bit.h
DEPS_41 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecModuleWrite.o: \
    src/compiler/ecModuleWrite.c $(DEPS_41)
	@echo '   [Compile] src/compiler/ecModuleWrite.c'
	$(CC) -c -o $(CONFIG)/obj/ecModuleWrite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecModuleWrite.c

#
#   ecParser.o
#
DEPS_42 += $(CONFIG)/inc/bit.h
DEPS_42 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecParser.o: \
    src/compiler/ecParser.c $(DEPS_42)
	@echo '   [Compile] src/compiler/ecParser.c'
	$(CC) -c -o $(CONFIG)/obj/ecParser.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecParser.c

#
#   ecState.o
#
DEPS_43 += $(CONFIG)/inc/bit.h
DEPS_43 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecState.o: \
    src/compiler/ecState.c $(DEPS_43)
	@echo '   [Compile] src/compiler/ecState.c'
	$(CC) -c -o $(CONFIG)/obj/ecState.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecState.c

#
#   dtoa.o
#
DEPS_44 += $(CONFIG)/inc/bit.h
DEPS_44 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/dtoa.o: \
    src/core/src/dtoa.c $(DEPS_44)
	@echo '   [Compile] src/core/src/dtoa.c'
	$(CC) -c -o $(CONFIG)/obj/dtoa.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/dtoa.c

#
#   ejsApp.o
#
DEPS_45 += $(CONFIG)/inc/bit.h
DEPS_45 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsApp.o: \
    src/core/src/ejsApp.c $(DEPS_45)
	@echo '   [Compile] src/core/src/ejsApp.c'
	$(CC) -c -o $(CONFIG)/obj/ejsApp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsApp.c

#
#   ejsArray.o
#
DEPS_46 += $(CONFIG)/inc/bit.h
DEPS_46 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsArray.o: \
    src/core/src/ejsArray.c $(DEPS_46)
	@echo '   [Compile] src/core/src/ejsArray.c'
	$(CC) -c -o $(CONFIG)/obj/ejsArray.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsArray.c

#
#   ejsBlock.o
#
DEPS_47 += $(CONFIG)/inc/bit.h
DEPS_47 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsBlock.o: \
    src/core/src/ejsBlock.c $(DEPS_47)
	@echo '   [Compile] src/core/src/ejsBlock.c'
	$(CC) -c -o $(CONFIG)/obj/ejsBlock.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBlock.c

#
#   ejsBoolean.o
#
DEPS_48 += $(CONFIG)/inc/bit.h
DEPS_48 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsBoolean.o: \
    src/core/src/ejsBoolean.c $(DEPS_48)
	@echo '   [Compile] src/core/src/ejsBoolean.c'
	$(CC) -c -o $(CONFIG)/obj/ejsBoolean.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBoolean.c

#
#   ejsByteArray.o
#
DEPS_49 += $(CONFIG)/inc/bit.h
DEPS_49 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsByteArray.o: \
    src/core/src/ejsByteArray.c $(DEPS_49)
	@echo '   [Compile] src/core/src/ejsByteArray.c'
	$(CC) -c -o $(CONFIG)/obj/ejsByteArray.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsByteArray.c

#
#   ejsCache.o
#
DEPS_50 += $(CONFIG)/inc/bit.h
DEPS_50 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsCache.o: \
    src/core/src/ejsCache.c $(DEPS_50)
	@echo '   [Compile] src/core/src/ejsCache.c'
	$(CC) -c -o $(CONFIG)/obj/ejsCache.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCache.c

#
#   ejsCmd.o
#
DEPS_51 += $(CONFIG)/inc/bit.h
DEPS_51 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsCmd.o: \
    src/core/src/ejsCmd.c $(DEPS_51)
	@echo '   [Compile] src/core/src/ejsCmd.c'
	$(CC) -c -o $(CONFIG)/obj/ejsCmd.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCmd.c

#
#   ejsConfig.o
#
DEPS_52 += $(CONFIG)/inc/bit.h
DEPS_52 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsConfig.o: \
    src/core/src/ejsConfig.c $(DEPS_52)
	@echo '   [Compile] src/core/src/ejsConfig.c'
	$(CC) -c -o $(CONFIG)/obj/ejsConfig.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsConfig.c

#
#   ejsDate.o
#
DEPS_53 += $(CONFIG)/inc/bit.h
DEPS_53 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsDate.o: \
    src/core/src/ejsDate.c $(DEPS_53)
	@echo '   [Compile] src/core/src/ejsDate.c'
	$(CC) -c -o $(CONFIG)/obj/ejsDate.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDate.c

#
#   ejsDebug.o
#
DEPS_54 += $(CONFIG)/inc/bit.h
DEPS_54 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsDebug.o: \
    src/core/src/ejsDebug.c $(DEPS_54)
	@echo '   [Compile] src/core/src/ejsDebug.c'
	$(CC) -c -o $(CONFIG)/obj/ejsDebug.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDebug.c

#
#   ejsError.o
#
DEPS_55 += $(CONFIG)/inc/bit.h
DEPS_55 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsError.o: \
    src/core/src/ejsError.c $(DEPS_55)
	@echo '   [Compile] src/core/src/ejsError.c'
	$(CC) -c -o $(CONFIG)/obj/ejsError.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsError.c

#
#   ejsFile.o
#
DEPS_56 += $(CONFIG)/inc/bit.h
DEPS_56 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFile.o: \
    src/core/src/ejsFile.c $(DEPS_56)
	@echo '   [Compile] src/core/src/ejsFile.c'
	$(CC) -c -o $(CONFIG)/obj/ejsFile.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFile.c

#
#   ejsFileSystem.o
#
DEPS_57 += $(CONFIG)/inc/bit.h
DEPS_57 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFileSystem.o: \
    src/core/src/ejsFileSystem.c $(DEPS_57)
	@echo '   [Compile] src/core/src/ejsFileSystem.c'
	$(CC) -c -o $(CONFIG)/obj/ejsFileSystem.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFileSystem.c

#
#   ejsFrame.o
#
DEPS_58 += $(CONFIG)/inc/bit.h
DEPS_58 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFrame.o: \
    src/core/src/ejsFrame.c $(DEPS_58)
	@echo '   [Compile] src/core/src/ejsFrame.c'
	$(CC) -c -o $(CONFIG)/obj/ejsFrame.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFrame.c

#
#   ejsFunction.o
#
DEPS_59 += $(CONFIG)/inc/bit.h
DEPS_59 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFunction.o: \
    src/core/src/ejsFunction.c $(DEPS_59)
	@echo '   [Compile] src/core/src/ejsFunction.c'
	$(CC) -c -o $(CONFIG)/obj/ejsFunction.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFunction.c

#
#   ejsGC.o
#
DEPS_60 += $(CONFIG)/inc/bit.h
DEPS_60 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsGC.o: \
    src/core/src/ejsGC.c $(DEPS_60)
	@echo '   [Compile] src/core/src/ejsGC.c'
	$(CC) -c -o $(CONFIG)/obj/ejsGC.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGC.c

#
#   ejsGlobal.o
#
DEPS_61 += $(CONFIG)/inc/bit.h
DEPS_61 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsGlobal.o: \
    src/core/src/ejsGlobal.c $(DEPS_61)
	@echo '   [Compile] src/core/src/ejsGlobal.c'
	$(CC) -c -o $(CONFIG)/obj/ejsGlobal.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGlobal.c

#
#   ejsHttp.o
#
DEPS_62 += $(CONFIG)/inc/bit.h
DEPS_62 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsHttp.o: \
    src/core/src/ejsHttp.c $(DEPS_62)
	@echo '   [Compile] src/core/src/ejsHttp.c'
	$(CC) -c -o $(CONFIG)/obj/ejsHttp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsHttp.c

#
#   ejsIterator.o
#
DEPS_63 += $(CONFIG)/inc/bit.h
DEPS_63 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsIterator.o: \
    src/core/src/ejsIterator.c $(DEPS_63)
	@echo '   [Compile] src/core/src/ejsIterator.c'
	$(CC) -c -o $(CONFIG)/obj/ejsIterator.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsIterator.c

#
#   ejsJSON.o
#
DEPS_64 += $(CONFIG)/inc/bit.h
DEPS_64 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsJSON.o: \
    src/core/src/ejsJSON.c $(DEPS_64)
	@echo '   [Compile] src/core/src/ejsJSON.c'
	$(CC) -c -o $(CONFIG)/obj/ejsJSON.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsJSON.c

#
#   ejsLocalCache.o
#
DEPS_65 += $(CONFIG)/inc/bit.h
DEPS_65 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsLocalCache.o: \
    src/core/src/ejsLocalCache.c $(DEPS_65)
	@echo '   [Compile] src/core/src/ejsLocalCache.c'
	$(CC) -c -o $(CONFIG)/obj/ejsLocalCache.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsLocalCache.c

#
#   ejsMath.o
#
DEPS_66 += $(CONFIG)/inc/bit.h
DEPS_66 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMath.o: \
    src/core/src/ejsMath.c $(DEPS_66)
	@echo '   [Compile] src/core/src/ejsMath.c'
	$(CC) -c -o $(CONFIG)/obj/ejsMath.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMath.c

#
#   ejsMemory.o
#
DEPS_67 += $(CONFIG)/inc/bit.h
DEPS_67 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMemory.o: \
    src/core/src/ejsMemory.c $(DEPS_67)
	@echo '   [Compile] src/core/src/ejsMemory.c'
	$(CC) -c -o $(CONFIG)/obj/ejsMemory.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMemory.c

#
#   ejsMprLog.o
#
DEPS_68 += $(CONFIG)/inc/bit.h
DEPS_68 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMprLog.o: \
    src/core/src/ejsMprLog.c $(DEPS_68)
	@echo '   [Compile] src/core/src/ejsMprLog.c'
	$(CC) -c -o $(CONFIG)/obj/ejsMprLog.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMprLog.c

#
#   ejsNamespace.o
#
DEPS_69 += $(CONFIG)/inc/bit.h
DEPS_69 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNamespace.o: \
    src/core/src/ejsNamespace.c $(DEPS_69)
	@echo '   [Compile] src/core/src/ejsNamespace.c'
	$(CC) -c -o $(CONFIG)/obj/ejsNamespace.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNamespace.c

#
#   ejsNull.o
#
DEPS_70 += $(CONFIG)/inc/bit.h
DEPS_70 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNull.o: \
    src/core/src/ejsNull.c $(DEPS_70)
	@echo '   [Compile] src/core/src/ejsNull.c'
	$(CC) -c -o $(CONFIG)/obj/ejsNull.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNull.c

#
#   ejsNumber.o
#
DEPS_71 += $(CONFIG)/inc/bit.h
DEPS_71 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNumber.o: \
    src/core/src/ejsNumber.c $(DEPS_71)
	@echo '   [Compile] src/core/src/ejsNumber.c'
	$(CC) -c -o $(CONFIG)/obj/ejsNumber.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNumber.c

#
#   ejsObject.o
#
DEPS_72 += $(CONFIG)/inc/bit.h
DEPS_72 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsObject.o: \
    src/core/src/ejsObject.c $(DEPS_72)
	@echo '   [Compile] src/core/src/ejsObject.c'
	$(CC) -c -o $(CONFIG)/obj/ejsObject.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsObject.c

#
#   ejsPath.o
#
DEPS_73 += $(CONFIG)/inc/bit.h
DEPS_73 += $(CONFIG)/inc/ejs.h
DEPS_73 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsPath.o: \
    src/core/src/ejsPath.c $(DEPS_73)
	@echo '   [Compile] src/core/src/ejsPath.c'
	$(CC) -c -o $(CONFIG)/obj/ejsPath.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPath.c

#
#   ejsPot.o
#
DEPS_74 += $(CONFIG)/inc/bit.h
DEPS_74 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsPot.o: \
    src/core/src/ejsPot.c $(DEPS_74)
	@echo '   [Compile] src/core/src/ejsPot.c'
	$(CC) -c -o $(CONFIG)/obj/ejsPot.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPot.c

#
#   ejsRegExp.o
#
DEPS_75 += $(CONFIG)/inc/bit.h
DEPS_75 += $(CONFIG)/inc/ejs.h
DEPS_75 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsRegExp.o: \
    src/core/src/ejsRegExp.c $(DEPS_75)
	@echo '   [Compile] src/core/src/ejsRegExp.c'
	$(CC) -c -o $(CONFIG)/obj/ejsRegExp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsRegExp.c

#
#   ejsSocket.o
#
DEPS_76 += $(CONFIG)/inc/bit.h
DEPS_76 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsSocket.o: \
    src/core/src/ejsSocket.c $(DEPS_76)
	@echo '   [Compile] src/core/src/ejsSocket.c'
	$(CC) -c -o $(CONFIG)/obj/ejsSocket.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSocket.c

#
#   ejsString.o
#
DEPS_77 += $(CONFIG)/inc/bit.h
DEPS_77 += $(CONFIG)/inc/ejs.h
DEPS_77 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsString.o: \
    src/core/src/ejsString.c $(DEPS_77)
	@echo '   [Compile] src/core/src/ejsString.c'
	$(CC) -c -o $(CONFIG)/obj/ejsString.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsString.c

#
#   ejsSystem.o
#
DEPS_78 += $(CONFIG)/inc/bit.h
DEPS_78 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsSystem.o: \
    src/core/src/ejsSystem.c $(DEPS_78)
	@echo '   [Compile] src/core/src/ejsSystem.c'
	$(CC) -c -o $(CONFIG)/obj/ejsSystem.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSystem.c

#
#   ejsTimer.o
#
DEPS_79 += $(CONFIG)/inc/bit.h
DEPS_79 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsTimer.o: \
    src/core/src/ejsTimer.c $(DEPS_79)
	@echo '   [Compile] src/core/src/ejsTimer.c'
	$(CC) -c -o $(CONFIG)/obj/ejsTimer.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsTimer.c

#
#   ejsType.o
#
DEPS_80 += $(CONFIG)/inc/bit.h
DEPS_80 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsType.o: \
    src/core/src/ejsType.c $(DEPS_80)
	@echo '   [Compile] src/core/src/ejsType.c'
	$(CC) -c -o $(CONFIG)/obj/ejsType.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsType.c

#
#   ejsUri.o
#
DEPS_81 += $(CONFIG)/inc/bit.h
DEPS_81 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsUri.o: \
    src/core/src/ejsUri.c $(DEPS_81)
	@echo '   [Compile] src/core/src/ejsUri.c'
	$(CC) -c -o $(CONFIG)/obj/ejsUri.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsUri.c

#
#   ejsVoid.o
#
DEPS_82 += $(CONFIG)/inc/bit.h
DEPS_82 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsVoid.o: \
    src/core/src/ejsVoid.c $(DEPS_82)
	@echo '   [Compile] src/core/src/ejsVoid.c'
	$(CC) -c -o $(CONFIG)/obj/ejsVoid.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsVoid.c

#
#   ejsWebSocket.o
#
DEPS_83 += $(CONFIG)/inc/bit.h
DEPS_83 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsWebSocket.o: \
    src/core/src/ejsWebSocket.c $(DEPS_83)
	@echo '   [Compile] src/core/src/ejsWebSocket.c'
	$(CC) -c -o $(CONFIG)/obj/ejsWebSocket.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWebSocket.c

#
#   ejsWorker.o
#
DEPS_84 += $(CONFIG)/inc/bit.h
DEPS_84 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsWorker.o: \
    src/core/src/ejsWorker.c $(DEPS_84)
	@echo '   [Compile] src/core/src/ejsWorker.c'
	$(CC) -c -o $(CONFIG)/obj/ejsWorker.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWorker.c

#
#   ejsXML.o
#
DEPS_85 += $(CONFIG)/inc/bit.h
DEPS_85 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXML.o: \
    src/core/src/ejsXML.c $(DEPS_85)
	@echo '   [Compile] src/core/src/ejsXML.c'
	$(CC) -c -o $(CONFIG)/obj/ejsXML.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXML.c

#
#   ejsXMLList.o
#
DEPS_86 += $(CONFIG)/inc/bit.h
DEPS_86 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXMLList.o: \
    src/core/src/ejsXMLList.c $(DEPS_86)
	@echo '   [Compile] src/core/src/ejsXMLList.c'
	$(CC) -c -o $(CONFIG)/obj/ejsXMLList.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLList.c

#
#   ejsXMLLoader.o
#
DEPS_87 += $(CONFIG)/inc/bit.h
DEPS_87 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXMLLoader.o: \
    src/core/src/ejsXMLLoader.c $(DEPS_87)
	@echo '   [Compile] src/core/src/ejsXMLLoader.c'
	$(CC) -c -o $(CONFIG)/obj/ejsXMLLoader.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLLoader.c

#
#   ejsByteCode.o
#
DEPS_88 += $(CONFIG)/inc/bit.h
DEPS_88 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsByteCode.o: \
    src/vm/ejsByteCode.c $(DEPS_88)
	@echo '   [Compile] src/vm/ejsByteCode.c'
	$(CC) -c -o $(CONFIG)/obj/ejsByteCode.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsByteCode.c

#
#   ejsException.o
#
DEPS_89 += $(CONFIG)/inc/bit.h
DEPS_89 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsException.o: \
    src/vm/ejsException.c $(DEPS_89)
	@echo '   [Compile] src/vm/ejsException.c'
	$(CC) -c -o $(CONFIG)/obj/ejsException.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsException.c

#
#   ejsHelper.o
#
DEPS_90 += $(CONFIG)/inc/bit.h
DEPS_90 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsHelper.o: \
    src/vm/ejsHelper.c $(DEPS_90)
	@echo '   [Compile] src/vm/ejsHelper.c'
	$(CC) -c -o $(CONFIG)/obj/ejsHelper.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsHelper.c

#
#   ejsInterp.o
#
DEPS_91 += $(CONFIG)/inc/bit.h
DEPS_91 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsInterp.o: \
    src/vm/ejsInterp.c $(DEPS_91)
	@echo '   [Compile] src/vm/ejsInterp.c'
	$(CC) -c -o $(CONFIG)/obj/ejsInterp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsInterp.c

#
#   ejsLoader.o
#
DEPS_92 += $(CONFIG)/inc/bit.h
DEPS_92 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsLoader.o: \
    src/vm/ejsLoader.c $(DEPS_92)
	@echo '   [Compile] src/vm/ejsLoader.c'
	$(CC) -c -o $(CONFIG)/obj/ejsLoader.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsLoader.c

#
#   ejsModule.o
#
DEPS_93 += $(CONFIG)/inc/bit.h
DEPS_93 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsModule.o: \
    src/vm/ejsModule.c $(DEPS_93)
	@echo '   [Compile] src/vm/ejsModule.c'
	$(CC) -c -o $(CONFIG)/obj/ejsModule.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsModule.c

#
#   ejsScope.o
#
DEPS_94 += $(CONFIG)/inc/bit.h
DEPS_94 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsScope.o: \
    src/vm/ejsScope.c $(DEPS_94)
	@echo '   [Compile] src/vm/ejsScope.c'
	$(CC) -c -o $(CONFIG)/obj/ejsScope.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsScope.c

#
#   ejsService.o
#
DEPS_95 += $(CONFIG)/inc/bit.h
DEPS_95 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsService.o: \
    src/vm/ejsService.c $(DEPS_95)
	@echo '   [Compile] src/vm/ejsService.c'
	$(CC) -c -o $(CONFIG)/obj/ejsService.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsService.c

#
#   libejs
#
DEPS_96 += $(CONFIG)/bin/libhttp.dylib
DEPS_96 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_96 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_96 += $(CONFIG)/inc/ejs.slots.h
DEPS_96 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_96 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_96 += $(CONFIG)/inc/bitos.h
DEPS_96 += $(CONFIG)/inc/ejs.h
DEPS_96 += $(CONFIG)/inc/ejsByteCode.h
DEPS_96 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_96 += $(CONFIG)/inc/ejsCompiler.h
DEPS_96 += $(CONFIG)/inc/ejsCustomize.h
DEPS_96 += $(CONFIG)/obj/ecAst.o
DEPS_96 += $(CONFIG)/obj/ecCodeGen.o
DEPS_96 += $(CONFIG)/obj/ecCompiler.o
DEPS_96 += $(CONFIG)/obj/ecLex.o
DEPS_96 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_96 += $(CONFIG)/obj/ecParser.o
DEPS_96 += $(CONFIG)/obj/ecState.o
DEPS_96 += $(CONFIG)/obj/dtoa.o
DEPS_96 += $(CONFIG)/obj/ejsApp.o
DEPS_96 += $(CONFIG)/obj/ejsArray.o
DEPS_96 += $(CONFIG)/obj/ejsBlock.o
DEPS_96 += $(CONFIG)/obj/ejsBoolean.o
DEPS_96 += $(CONFIG)/obj/ejsByteArray.o
DEPS_96 += $(CONFIG)/obj/ejsCache.o
DEPS_96 += $(CONFIG)/obj/ejsCmd.o
DEPS_96 += $(CONFIG)/obj/ejsConfig.o
DEPS_96 += $(CONFIG)/obj/ejsDate.o
DEPS_96 += $(CONFIG)/obj/ejsDebug.o
DEPS_96 += $(CONFIG)/obj/ejsError.o
DEPS_96 += $(CONFIG)/obj/ejsFile.o
DEPS_96 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_96 += $(CONFIG)/obj/ejsFrame.o
DEPS_96 += $(CONFIG)/obj/ejsFunction.o
DEPS_96 += $(CONFIG)/obj/ejsGC.o
DEPS_96 += $(CONFIG)/obj/ejsGlobal.o
DEPS_96 += $(CONFIG)/obj/ejsHttp.o
DEPS_96 += $(CONFIG)/obj/ejsIterator.o
DEPS_96 += $(CONFIG)/obj/ejsJSON.o
DEPS_96 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_96 += $(CONFIG)/obj/ejsMath.o
DEPS_96 += $(CONFIG)/obj/ejsMemory.o
DEPS_96 += $(CONFIG)/obj/ejsMprLog.o
DEPS_96 += $(CONFIG)/obj/ejsNamespace.o
DEPS_96 += $(CONFIG)/obj/ejsNull.o
DEPS_96 += $(CONFIG)/obj/ejsNumber.o
DEPS_96 += $(CONFIG)/obj/ejsObject.o
DEPS_96 += $(CONFIG)/obj/ejsPath.o
DEPS_96 += $(CONFIG)/obj/ejsPot.o
DEPS_96 += $(CONFIG)/obj/ejsRegExp.o
DEPS_96 += $(CONFIG)/obj/ejsSocket.o
DEPS_96 += $(CONFIG)/obj/ejsString.o
DEPS_96 += $(CONFIG)/obj/ejsSystem.o
DEPS_96 += $(CONFIG)/obj/ejsTimer.o
DEPS_96 += $(CONFIG)/obj/ejsType.o
DEPS_96 += $(CONFIG)/obj/ejsUri.o
DEPS_96 += $(CONFIG)/obj/ejsVoid.o
DEPS_96 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_96 += $(CONFIG)/obj/ejsWorker.o
DEPS_96 += $(CONFIG)/obj/ejsXML.o
DEPS_96 += $(CONFIG)/obj/ejsXMLList.o
DEPS_96 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_96 += $(CONFIG)/obj/ejsByteCode.o
DEPS_96 += $(CONFIG)/obj/ejsException.o
DEPS_96 += $(CONFIG)/obj/ejsHelper.o
DEPS_96 += $(CONFIG)/obj/ejsInterp.o
DEPS_96 += $(CONFIG)/obj/ejsLoader.o
DEPS_96 += $(CONFIG)/obj/ejsModule.o
DEPS_96 += $(CONFIG)/obj/ejsScope.o
DEPS_96 += $(CONFIG)/obj/ejsService.o

LIBS_96 += -lhttp
LIBS_96 += -lpcre
LIBS_96 += -lmpr

$(CONFIG)/bin/libejs.dylib: $(DEPS_96)
	@echo '      [Link] libejs'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libejs.dylib $(CONFIG)/obj/ecAst.o $(CONFIG)/obj/ecCodeGen.o $(CONFIG)/obj/ecCompiler.o $(CONFIG)/obj/ecLex.o $(CONFIG)/obj/ecModuleWrite.o $(CONFIG)/obj/ecParser.o $(CONFIG)/obj/ecState.o $(CONFIG)/obj/dtoa.o $(CONFIG)/obj/ejsApp.o $(CONFIG)/obj/ejsArray.o $(CONFIG)/obj/ejsBlock.o $(CONFIG)/obj/ejsBoolean.o $(CONFIG)/obj/ejsByteArray.o $(CONFIG)/obj/ejsCache.o $(CONFIG)/obj/ejsCmd.o $(CONFIG)/obj/ejsConfig.o $(CONFIG)/obj/ejsDate.o $(CONFIG)/obj/ejsDebug.o $(CONFIG)/obj/ejsError.o $(CONFIG)/obj/ejsFile.o $(CONFIG)/obj/ejsFileSystem.o $(CONFIG)/obj/ejsFrame.o $(CONFIG)/obj/ejsFunction.o $(CONFIG)/obj/ejsGC.o $(CONFIG)/obj/ejsGlobal.o $(CONFIG)/obj/ejsHttp.o $(CONFIG)/obj/ejsIterator.o $(CONFIG)/obj/ejsJSON.o $(CONFIG)/obj/ejsLocalCache.o $(CONFIG)/obj/ejsMath.o $(CONFIG)/obj/ejsMemory.o $(CONFIG)/obj/ejsMprLog.o $(CONFIG)/obj/ejsNamespace.o $(CONFIG)/obj/ejsNull.o $(CONFIG)/obj/ejsNumber.o $(CONFIG)/obj/ejsObject.o $(CONFIG)/obj/ejsPath.o $(CONFIG)/obj/ejsPot.o $(CONFIG)/obj/ejsRegExp.o $(CONFIG)/obj/ejsSocket.o $(CONFIG)/obj/ejsString.o $(CONFIG)/obj/ejsSystem.o $(CONFIG)/obj/ejsTimer.o $(CONFIG)/obj/ejsType.o $(CONFIG)/obj/ejsUri.o $(CONFIG)/obj/ejsVoid.o $(CONFIG)/obj/ejsWebSocket.o $(CONFIG)/obj/ejsWorker.o $(CONFIG)/obj/ejsXML.o $(CONFIG)/obj/ejsXMLList.o $(CONFIG)/obj/ejsXMLLoader.o $(CONFIG)/obj/ejsByteCode.o $(CONFIG)/obj/ejsException.o $(CONFIG)/obj/ejsHelper.o $(CONFIG)/obj/ejsInterp.o $(CONFIG)/obj/ejsLoader.o $(CONFIG)/obj/ejsModule.o $(CONFIG)/obj/ejsScope.o $(CONFIG)/obj/ejsService.o $(LIBS_96) $(LIBS_96) $(LIBS) -lpam

#
#   ejs.o
#
DEPS_97 += $(CONFIG)/inc/bit.h
DEPS_97 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejs.o: \
    src/cmd/ejs.c $(DEPS_97)
	@echo '   [Compile] src/cmd/ejs.c'
	$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejs.c

#
#   ejs
#
DEPS_98 += $(CONFIG)/bin/libejs.dylib
DEPS_98 += $(CONFIG)/obj/ejs.o

LIBS_98 += -lejs
LIBS_98 += -lhttp
LIBS_98 += -lpcre
LIBS_98 += -lmpr

$(CONFIG)/bin/ejs: $(DEPS_98)
	@echo '      [Link] ejs'
	$(CC) -o $(CONFIG)/bin/ejs -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejs.o $(LIBS_98) $(LIBS_98) $(LIBS) -ledit

#
#   ejsc.o
#
DEPS_99 += $(CONFIG)/inc/bit.h
DEPS_99 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejsc.o: \
    src/cmd/ejsc.c $(DEPS_99)
	@echo '   [Compile] src/cmd/ejsc.c'
	$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsc.c

#
#   ejsc
#
DEPS_100 += $(CONFIG)/bin/libejs.dylib
DEPS_100 += $(CONFIG)/obj/ejsc.o

LIBS_100 += -lejs
LIBS_100 += -lhttp
LIBS_100 += -lpcre
LIBS_100 += -lmpr

$(CONFIG)/bin/ejsc: $(DEPS_100)
	@echo '      [Link] ejsc'
	$(CC) -o $(CONFIG)/bin/ejsc -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsc.o $(LIBS_100) $(LIBS_100) $(LIBS) -lpam

#
#   ejsmod.h
#
src/cmd/ejsmod.h: $(DEPS_101)

#
#   ejsmod.o
#
DEPS_102 += $(CONFIG)/inc/bit.h
DEPS_102 += src/cmd/ejsmod.h
DEPS_102 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsmod.o: \
    src/cmd/ejsmod.c $(DEPS_102)
	@echo '   [Compile] src/cmd/ejsmod.c'
	$(CC) -c -o $(CONFIG)/obj/ejsmod.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/ejsmod.c

#
#   doc.o
#
DEPS_103 += $(CONFIG)/inc/bit.h
DEPS_103 += src/cmd/ejsmod.h

$(CONFIG)/obj/doc.o: \
    src/cmd/doc.c $(DEPS_103)
	@echo '   [Compile] src/cmd/doc.c'
	$(CC) -c -o $(CONFIG)/obj/doc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/doc.c

#
#   docFiles.o
#
DEPS_104 += $(CONFIG)/inc/bit.h
DEPS_104 += src/cmd/ejsmod.h

$(CONFIG)/obj/docFiles.o: \
    src/cmd/docFiles.c $(DEPS_104)
	@echo '   [Compile] src/cmd/docFiles.c'
	$(CC) -c -o $(CONFIG)/obj/docFiles.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/docFiles.c

#
#   listing.o
#
DEPS_105 += $(CONFIG)/inc/bit.h
DEPS_105 += src/cmd/ejsmod.h
DEPS_105 += $(CONFIG)/inc/ejsByteCodeTable.h

$(CONFIG)/obj/listing.o: \
    src/cmd/listing.c $(DEPS_105)
	@echo '   [Compile] src/cmd/listing.c'
	$(CC) -c -o $(CONFIG)/obj/listing.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/listing.c

#
#   slotGen.o
#
DEPS_106 += $(CONFIG)/inc/bit.h
DEPS_106 += src/cmd/ejsmod.h

$(CONFIG)/obj/slotGen.o: \
    src/cmd/slotGen.c $(DEPS_106)
	@echo '   [Compile] src/cmd/slotGen.c'
	$(CC) -c -o $(CONFIG)/obj/slotGen.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/slotGen.c

#
#   ejsmod
#
DEPS_107 += $(CONFIG)/bin/libejs.dylib
DEPS_107 += $(CONFIG)/obj/ejsmod.o
DEPS_107 += $(CONFIG)/obj/doc.o
DEPS_107 += $(CONFIG)/obj/docFiles.o
DEPS_107 += $(CONFIG)/obj/listing.o
DEPS_107 += $(CONFIG)/obj/slotGen.o

LIBS_107 += -lejs
LIBS_107 += -lhttp
LIBS_107 += -lpcre
LIBS_107 += -lmpr

$(CONFIG)/bin/ejsmod: $(DEPS_107)
	@echo '      [Link] ejsmod'
	$(CC) -o $(CONFIG)/bin/ejsmod -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsmod.o $(CONFIG)/obj/doc.o $(CONFIG)/obj/docFiles.o $(CONFIG)/obj/listing.o $(CONFIG)/obj/slotGen.o $(LIBS_107) $(LIBS_107) $(LIBS) -lpam

#
#   ejsrun.o
#
DEPS_108 += $(CONFIG)/inc/bit.h
DEPS_108 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejsrun.o: \
    src/cmd/ejsrun.c $(DEPS_108)
	@echo '   [Compile] src/cmd/ejsrun.c'
	$(CC) -c -o $(CONFIG)/obj/ejsrun.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsrun.c

#
#   ejsrun
#
DEPS_109 += $(CONFIG)/bin/libejs.dylib
DEPS_109 += $(CONFIG)/obj/ejsrun.o

LIBS_109 += -lejs
LIBS_109 += -lhttp
LIBS_109 += -lpcre
LIBS_109 += -lmpr

$(CONFIG)/bin/ejsrun: $(DEPS_109)
	@echo '      [Link] ejsrun'
	$(CC) -o $(CONFIG)/bin/ejsrun -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LIBS_109) $(LIBS_109) $(LIBS) -lpam

#
#   ejs.mod
#
DEPS_110 += $(CONFIG)/bin/ejsc
DEPS_110 += $(CONFIG)/bin/ejsmod

$(CONFIG)/bin/ejs.mod: $(DEPS_110)
	cd src/core; ../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.mod  --optimize 9 --bind --require null *.es  ; cd ../..
	cd src/core; ../../$(CONFIG)/bin/ejsmod --require null --cslots ../../$(CONFIG)/bin/ejs.mod ; cd ../..
	cd src/core; if ! diff ejs.slots.h ../../$(CONFIG)/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ../../$(CONFIG)/inc; fi ; cd ../..
	cd src/core; rm -f ejs.slots.h ; cd ../..

#
#   ejs.unix.mod
#
DEPS_111 += $(CONFIG)/bin/ejsc
DEPS_111 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.unix.mod: $(DEPS_111)
	cd src/jems/ejs.unix; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.unix.mod  --optimize 9 Unix.es ; cd ../../..

#
#   jem.es
#
$(CONFIG)/bin/jem.es: $(DEPS_112)
	cd src/jems/ejs.jem; cp jem.es ../../../$(CONFIG)/bin ; cd ../../..

#
#   jem
#
DEPS_113 += $(CONFIG)/bin/libejs.dylib
DEPS_113 += $(CONFIG)/bin/jem.es
DEPS_113 += $(CONFIG)/obj/ejsrun.o

LIBS_113 += -lejs
LIBS_113 += -lhttp
LIBS_113 += -lpcre
LIBS_113 += -lmpr

$(CONFIG)/bin/jem: $(DEPS_113)
	@echo '      [Link] jem'
	$(CC) -o $(CONFIG)/bin/jem -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LIBS_113) $(LIBS_113) $(LIBS) -lpam

#
#   ejs.db.mod
#
DEPS_114 += $(CONFIG)/bin/ejsc
DEPS_114 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.db.mod: $(DEPS_114)
	cd src/jems/ejs.db; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.mod  --optimize 9 *.es ; cd ../../..

#
#   ejs.db.mapper.mod
#
DEPS_115 += $(CONFIG)/bin/ejsc
DEPS_115 += $(CONFIG)/bin/ejs.mod
DEPS_115 += $(CONFIG)/bin/ejs.db.mod

$(CONFIG)/bin/ejs.db.mapper.mod: $(DEPS_115)
	cd src/jems/ejs.db.mapper; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.mapper.mod  --optimize 9 *.es ; cd ../../..

#
#   ejs.db.sqlite.mod
#
DEPS_116 += $(CONFIG)/bin/ejsc
DEPS_116 += $(CONFIG)/bin/ejsmod
DEPS_116 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.db.sqlite.mod: $(DEPS_116)
	cd src/jems/ejs.db.sqlite; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.sqlite.mod  --optimize 9 *.es ; cd ../../..

#
#   ejsSqlite.o
#
DEPS_117 += $(CONFIG)/inc/bit.h
DEPS_117 += $(CONFIG)/inc/ejs.h
DEPS_117 += $(CONFIG)/inc/ejs.db.sqlite.slots.h

$(CONFIG)/obj/ejsSqlite.o: \
    src/jems/ejs.db.sqlite/ejsSqlite.c $(DEPS_117)
	@echo '   [Compile] src/jems/ejs.db.sqlite/ejsSqlite.c'
	$(CC) -c -o $(CONFIG)/obj/ejsSqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/jems/ejs.db.sqlite/ejsSqlite.c

#
#   libejs.db.sqlite
#
DEPS_118 += $(CONFIG)/bin/libmpr.dylib
DEPS_118 += $(CONFIG)/bin/libejs.dylib
DEPS_118 += $(CONFIG)/bin/ejs.mod
DEPS_118 += $(CONFIG)/bin/ejs.db.sqlite.mod
ifeq ($(BIT_PACK_SQLITE),1)
    DEPS_118 += $(CONFIG)/bin/libsqlite3.dylib
endif
DEPS_118 += $(CONFIG)/obj/ejsSqlite.o

ifeq ($(BIT_PACK_SQLITE),1)
    LIBS_118 += -lsqlite3
endif
LIBS_118 += -lejs
LIBS_118 += -lmpr
LIBS_118 += -lhttp
LIBS_118 += -lpcre

$(CONFIG)/bin/libejs.db.sqlite.dylib: $(DEPS_118)
	@echo '      [Link] libejs.db.sqlite'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.db.sqlite.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libejs.db.sqlite.dylib $(CONFIG)/obj/ejsSqlite.o $(LIBS_118) $(LIBS_118) $(LIBS) -lpam

#
#   ejs.web.mod
#
DEPS_119 += $(CONFIG)/bin/ejsc
DEPS_119 += $(CONFIG)/bin/ejsmod
DEPS_119 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.web.mod: $(DEPS_119)
	cd src/jems/ejs.web; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.web.mod  --optimize 9 *.es ; cd ../../..
	cd src/jems/ejs.web; ../../../$(CONFIG)/bin/ejsmod --cslots ../../../$(CONFIG)/bin/ejs.web.mod ; cd ../../..
	cd src/jems/ejs.web; if ! diff ejs.web.slots.h ../../../$(CONFIG)/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../../$(CONFIG)/inc; fi ; cd ../../..
	cd src/jems/ejs.web; rm -f ejs.web.slots.h ; cd ../../..

#
#   ejsWeb.h
#
$(CONFIG)/inc/ejsWeb.h: $(DEPS_120)
	@echo '      [File] $(CONFIG)/inc/ejsWeb.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/jems/ejs.web/ejsWeb.h" "$(CONFIG)/inc/ejsWeb.h"

#
#   ejsHttpServer.o
#
DEPS_121 += $(CONFIG)/inc/bit.h
DEPS_121 += $(CONFIG)/inc/ejs.h
DEPS_121 += $(CONFIG)/inc/ejsCompiler.h
DEPS_121 += $(CONFIG)/inc/ejsWeb.h
DEPS_121 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_121 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/ejsHttpServer.o: \
    src/jems/ejs.web/ejsHttpServer.c $(DEPS_121)
	@echo '   [Compile] src/jems/ejs.web/ejsHttpServer.c'
	$(CC) -c -o $(CONFIG)/obj/ejsHttpServer.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsHttpServer.c

#
#   ejsRequest.o
#
DEPS_122 += $(CONFIG)/inc/bit.h
DEPS_122 += $(CONFIG)/inc/ejs.h
DEPS_122 += $(CONFIG)/inc/ejsCompiler.h
DEPS_122 += $(CONFIG)/inc/ejsWeb.h
DEPS_122 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsRequest.o: \
    src/jems/ejs.web/ejsRequest.c $(DEPS_122)
	@echo '   [Compile] src/jems/ejs.web/ejsRequest.c'
	$(CC) -c -o $(CONFIG)/obj/ejsRequest.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsRequest.c

#
#   ejsSession.o
#
DEPS_123 += $(CONFIG)/inc/bit.h
DEPS_123 += $(CONFIG)/inc/ejs.h
DEPS_123 += $(CONFIG)/inc/ejsWeb.h

$(CONFIG)/obj/ejsSession.o: \
    src/jems/ejs.web/ejsSession.c $(DEPS_123)
	@echo '   [Compile] src/jems/ejs.web/ejsSession.c'
	$(CC) -c -o $(CONFIG)/obj/ejsSession.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsSession.c

#
#   ejsWeb.o
#
DEPS_124 += $(CONFIG)/inc/bit.h
DEPS_124 += $(CONFIG)/inc/ejs.h
DEPS_124 += $(CONFIG)/inc/ejsCompiler.h
DEPS_124 += $(CONFIG)/inc/ejsWeb.h
DEPS_124 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsWeb.o: \
    src/jems/ejs.web/ejsWeb.c $(DEPS_124)
	@echo '   [Compile] src/jems/ejs.web/ejsWeb.c'
	$(CC) -c -o $(CONFIG)/obj/ejsWeb.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsWeb.c

#
#   libejs.web
#
DEPS_125 += $(CONFIG)/bin/libejs.dylib
DEPS_125 += $(CONFIG)/bin/ejs.mod
DEPS_125 += $(CONFIG)/inc/ejsWeb.h
DEPS_125 += $(CONFIG)/obj/ejsHttpServer.o
DEPS_125 += $(CONFIG)/obj/ejsRequest.o
DEPS_125 += $(CONFIG)/obj/ejsSession.o
DEPS_125 += $(CONFIG)/obj/ejsWeb.o

LIBS_125 += -lejs
LIBS_125 += -lhttp
LIBS_125 += -lpcre
LIBS_125 += -lmpr

$(CONFIG)/bin/libejs.web.dylib: $(DEPS_125)
	@echo '      [Link] libejs.web'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.web.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libejs.web.dylib $(CONFIG)/obj/ejsHttpServer.o $(CONFIG)/obj/ejsRequest.o $(CONFIG)/obj/ejsSession.o $(CONFIG)/obj/ejsWeb.o $(LIBS_125) $(LIBS_125) $(LIBS) -lpam

#
#   www
#
$(CONFIG)/bin/www: $(DEPS_126)
	cd src/jems/ejs.web; rm -fr ../../../$(CONFIG)/bin/www ; cd ../../..
	cd src/jems/ejs.web; cp -r www ../../../$(CONFIG)/bin ; cd ../../..

#
#   ejs.template.mod
#
DEPS_127 += $(CONFIG)/bin/ejsc
DEPS_127 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.template.mod: $(DEPS_127)
	cd src/jems/ejs.template; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.template.mod  --optimize 9 TemplateParser.es ; cd ../../..

#
#   ejs.zlib.mod
#
DEPS_128 += $(CONFIG)/bin/ejsc
DEPS_128 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.zlib.mod: $(DEPS_128)
	cd src/jems/ejs.zlib; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.zlib.mod  --optimize 9 *.es ; cd ../../..

#
#   ejsZlib.o
#
DEPS_129 += $(CONFIG)/inc/bit.h
DEPS_129 += $(CONFIG)/inc/ejs.h
DEPS_129 += $(CONFIG)/inc/zlib.h
DEPS_129 += $(CONFIG)/inc/ejs.zlib.slots.h

$(CONFIG)/obj/ejsZlib.o: \
    src/jems/ejs.zlib/ejsZlib.c $(DEPS_129)
	@echo '   [Compile] src/jems/ejs.zlib/ejsZlib.c'
	$(CC) -c -o $(CONFIG)/obj/ejsZlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/jems/ejs.zlib/ejsZlib.c

#
#   libejs.zlib
#
DEPS_130 += $(CONFIG)/bin/libejs.dylib
DEPS_130 += $(CONFIG)/bin/ejs.mod
DEPS_130 += $(CONFIG)/bin/ejs.zlib.mod
DEPS_130 += $(CONFIG)/bin/libzlib.dylib
DEPS_130 += $(CONFIG)/obj/ejsZlib.o

LIBS_130 += -lzlib
LIBS_130 += -lejs
LIBS_130 += -lhttp
LIBS_130 += -lpcre
LIBS_130 += -lmpr

$(CONFIG)/bin/libejs.zlib.dylib: $(DEPS_130)
	@echo '      [Link] libejs.zlib'
	$(CC) -dynamiclib -o $(CONFIG)/bin/libejs.zlib.dylib $(LDFLAGS) -compatibility_version 2.3.0 -current_version 2.3.0 $(LIBPATHS) -install_name @rpath/libejs.zlib.dylib $(CONFIG)/obj/ejsZlib.o $(LIBS_130) $(LIBS_130) $(LIBS) -lpam

#
#   ejs.tar.mod
#
DEPS_131 += $(CONFIG)/bin/ejsc
DEPS_131 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.tar.mod: $(DEPS_131)
	cd src/jems/ejs.tar; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.tar.mod  --optimize 9 *.es ; cd ../../..

#
#   mvc.es
#
$(CONFIG)/bin/mvc.es: $(DEPS_132)
	cd src/jems/ejs.mvc; cp mvc.es ../../../$(CONFIG)/bin ; cd ../../..

#
#   mvc
#
DEPS_133 += $(CONFIG)/bin/libejs.dylib
DEPS_133 += $(CONFIG)/bin/mvc.es
DEPS_133 += $(CONFIG)/obj/ejsrun.o

LIBS_133 += -lejs
LIBS_133 += -lhttp
LIBS_133 += -lpcre
LIBS_133 += -lmpr

$(CONFIG)/bin/mvc: $(DEPS_133)
	@echo '      [Link] mvc'
	$(CC) -o $(CONFIG)/bin/mvc -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LIBS_133) $(LIBS_133) $(LIBS) -lpam

#
#   ejs.mvc.mod
#
DEPS_134 += $(CONFIG)/bin/ejsc
DEPS_134 += $(CONFIG)/bin/ejs.mod
DEPS_134 += $(CONFIG)/bin/ejs.web.mod
DEPS_134 += $(CONFIG)/bin/ejs.template.mod
DEPS_134 += $(CONFIG)/bin/ejs.unix.mod

$(CONFIG)/bin/ejs.mvc.mod: $(DEPS_134)
	cd src/jems/ejs.mvc; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.mvc.mod  --optimize 9 *.es ; cd ../../..

#
#   utest.es
#
$(CONFIG)/bin/utest.es: $(DEPS_135)
	cd src/jems/ejs.utest; cp utest.es ../../../$(CONFIG)/bin ; cd ../../..

#
#   utest.worker
#
$(CONFIG)/bin/utest.worker: $(DEPS_136)
	cd src/jems/ejs.utest; cp utest.worker ../../../$(CONFIG)/bin ; cd ../../..

#
#   utest
#
DEPS_137 += $(CONFIG)/bin/libejs.dylib
DEPS_137 += $(CONFIG)/bin/utest.es
DEPS_137 += $(CONFIG)/bin/utest.worker
DEPS_137 += $(CONFIG)/obj/ejsrun.o

LIBS_137 += -lejs
LIBS_137 += -lhttp
LIBS_137 += -lpcre
LIBS_137 += -lmpr

$(CONFIG)/bin/utest: $(DEPS_137)
	@echo '      [Link] utest'
	$(CC) -o $(CONFIG)/bin/utest -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LIBS_137) $(LIBS_137) $(LIBS) -lpam

#
#   version
#
version: $(DEPS_138)
	@echo 2.3.0-1

#
#   stop
#
stop: $(DEPS_139)
	

#
#   installBinary
#
DEPS_140 += stop

installBinary: $(DEPS_140)
	mkdir -p "$(BIT_APP_PREFIX)"
	mkdir -p "$(BIT_VAPP_PREFIX)"
	mkdir -p "$(BIT_APP_PREFIX)"
	rm -f "$(BIT_APP_PREFIX)/latest"
	ln -s "2.3.0" "$(BIT_APP_PREFIX)/latest"
	mkdir -p "$(BIT_VAPP_PREFIX)/bin"
	cp "$(CONFIG)/bin/ejs" "$(BIT_VAPP_PREFIX)/bin/ejs"
	mkdir -p "$(BIT_BIN_PREFIX)"
	rm -f "$(BIT_BIN_PREFIX)/ejs"
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
	cp "$(CONFIG)/bin/libejs.db.sqlite.dylib" "$(BIT_VAPP_PREFIX)/bin/libejs.db.sqlite.dylib"
	cp "$(CONFIG)/bin/libejs.dylib" "$(BIT_VAPP_PREFIX)/bin/libejs.dylib"
	cp "$(CONFIG)/bin/libejs.db.sqlite.dylib" "$(BIT_VAPP_PREFIX)/bin/libejs.db.sqlite.dylib"
	cp "$(CONFIG)/bin/libejs.web.dylib" "$(BIT_VAPP_PREFIX)/bin/libejs.web.dylib"
	cp "$(CONFIG)/bin/libejs.zlib.dylib" "$(BIT_VAPP_PREFIX)/bin/libejs.zlib.dylib"
	cp "$(CONFIG)/bin/libhttp.dylib" "$(BIT_VAPP_PREFIX)/bin/libhttp.dylib"
	cp "$(CONFIG)/bin/libmpr.dylib" "$(BIT_VAPP_PREFIX)/bin/libmpr.dylib"
	cp "$(CONFIG)/bin/libmprSsl.dylib" "$(BIT_VAPP_PREFIX)/bin/libmprSsl.dylib"
	cp "$(CONFIG)/bin/libpcre.dylib" "$(BIT_VAPP_PREFIX)/bin/libpcre.dylib"
	cp "$(CONFIG)/bin/libsqlite3.dylib" "$(BIT_VAPP_PREFIX)/bin/libsqlite3.dylib"
	cp "$(CONFIG)/bin/libzlib.dylib" "$(BIT_VAPP_PREFIX)/bin/libzlib.dylib"
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
	mkdir -p "$(BIT_INC_PREFIX)/ejs"
	rm -f "$(BIT_INC_PREFIX)/ejs/bit.h"
	ln -s "$(BIT_VAPP_PREFIX)/inc/bit.h" "$(BIT_INC_PREFIX)/ejs/bit.h"
	mkdir -p "$(BIT_VAPP_PREFIX)/doc/man1"
	cp "doc/man/ejs.1" "$(BIT_VAPP_PREFIX)/doc/man1/ejs.1"
	mkdir -p "$(BIT_MAN_PREFIX)/man1"
	rm -f "$(BIT_MAN_PREFIX)/man1/ejs.1"
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

#
#   start
#
start: $(DEPS_141)
	

#
#   install
#
DEPS_142 += stop
DEPS_142 += installBinary
DEPS_142 += start

install: $(DEPS_142)
	

#
#   uninstall
#
DEPS_143 += stop

uninstall: $(DEPS_143)
	rm -fr "$(BIT_VAPP_PREFIX)"
	rm -f "$(BIT_APP_PREFIX)/latest"
	rmdir -p "$(BIT_APP_PREFIX)"

