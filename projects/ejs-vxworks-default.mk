#
#   ejs-vxworks-default.mk -- Makefile to build Embedthis Ejscript for vxworks
#

export WIND_BASE := $(WIND_BASE)
export WIND_HOME := $(WIND_BASE)/..
export WIND_PLATFORM := $(WIND_PLATFORM)

PRODUCT         := ejs
VERSION         := 2.3.0
BUILD_NUMBER    := 1
PROFILE         := default
ARCH            := $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
OS              := vxworks
CC              := ccpentium
LD              := /usr/bin/ld
CONFIG          := $(OS)-$(ARCH)-$(PROFILE)
LBIN            := $(CONFIG)/bin

CFLAGS          += -fno-builtin -fno-defer-pop -fvolatile -w
DFLAGS          += -D_REENTRANT -DVXWORKS -DRW_MULTI_THREAD -D_GNU_TOOL -DCPU=PENTIUM $(patsubst %,-D%,$(filter BIT_%,$(MAKEFLAGS)))
IFLAGS          += -I$(CONFIG)/inc -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/h/wrn/coreip
LDFLAGS         += '-Wl,-r'
LIBPATHS        += -L$(CONFIG)/bin
LIBS            += 

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

BIT_PACK_SQLITE       := 1

BIT_ROOT_PREFIX       := deploy
BIT_BASE_PREFIX       := $(BIT_ROOT_PREFIX)
BIT_DATA_PREFIX       := $(BIT_VAPP_PREFIX)
BIT_STATE_PREFIX      := $(BIT_VAPP_PREFIX)
BIT_BIN_PREFIX        := $(BIT_VAPP_PREFIX)
BIT_INC_PREFIX        := $(BIT_VAPP_PREFIX)/inc
BIT_LIB_PREFIX        := $(BIT_VAPP_PREFIX)
BIT_MAN_PREFIX        := $(BIT_VAPP_PREFIX)
BIT_SBIN_PREFIX       := $(BIT_VAPP_PREFIX)
BIT_ETC_PREFIX        := $(BIT_VAPP_PREFIX)
BIT_WEB_PREFIX        := $(BIT_VAPP_PREFIX)/web
BIT_LOG_PREFIX        := $(BIT_VAPP_PREFIX)
BIT_SPOOL_PREFIX      := $(BIT_VAPP_PREFIX)
BIT_CACHE_PREFIX      := $(BIT_VAPP_PREFIX)
BIT_APP_PREFIX        := $(BIT_BASE_PREFIX)
BIT_VAPP_PREFIX       := $(BIT_APP_PREFIX)
BIT_SRC_PREFIX        := $(BIT_ROOT_PREFIX)/usr/src/$(PRODUCT)-$(VERSION)


TARGETS     += $(CONFIG)/bin/libmpr.out
TARGETS     += $(CONFIG)/bin/libmprssl.out
TARGETS     += $(CONFIG)/bin/ejsman.out
TARGETS     += $(CONFIG)/bin/makerom.out
TARGETS     += $(CONFIG)/bin/ca.crt
TARGETS     += $(CONFIG)/bin/libpcre.out
TARGETS     += $(CONFIG)/bin/libhttp.out
TARGETS     += $(CONFIG)/bin/http.out
ifeq ($(BIT_PACK_SQLITE),1)
TARGETS += $(CONFIG)/bin/libsqlite3.out
endif
ifeq ($(BIT_PACK_SQLITE),1)
TARGETS += $(CONFIG)/bin/sqlite.out
endif
TARGETS     += $(CONFIG)/bin/libzlib.out
TARGETS     += $(CONFIG)/bin/libejs.out
TARGETS     += $(CONFIG)/bin/ejs.out
TARGETS     += $(CONFIG)/bin/ejsc.out
TARGETS     += $(CONFIG)/bin/ejsmod.out
TARGETS     += $(CONFIG)/bin/ejsrun.out
TARGETS     += $(CONFIG)/bin/ejs.mod
TARGETS     += $(CONFIG)/bin/ejs.unix.mod
TARGETS     += $(CONFIG)/bin/jem.es
TARGETS     += $(CONFIG)/bin/jem.out
TARGETS     += $(CONFIG)/bin/ejs.db.mod
TARGETS     += $(CONFIG)/bin/ejs.db.mapper.mod
TARGETS     += $(CONFIG)/bin/ejs.db.sqlite.mod
TARGETS     += $(CONFIG)/bin/libejs.db.sqlite.out
TARGETS     += $(CONFIG)/bin/ejs.web.mod
TARGETS     += $(CONFIG)/bin/libejs.web.out
TARGETS     += $(CONFIG)/bin/www
TARGETS     += $(CONFIG)/bin/ejs.template.mod
TARGETS     += $(CONFIG)/bin/ejs.zlib.mod
TARGETS     += $(CONFIG)/bin/libejs.zlib.out
TARGETS     += $(CONFIG)/bin/ejs.tar.mod
TARGETS     += $(CONFIG)/bin/mvc.es
TARGETS     += $(CONFIG)/bin/mvc.out
TARGETS     += $(CONFIG)/bin/ejs.mvc.mod
TARGETS     += $(CONFIG)/bin/utest.es
TARGETS     += $(CONFIG)/bin/utest.worker
TARGETS     += $(CONFIG)/bin/utest.out

unexport CDPATH

ifndef SHOW
.SILENT:
endif

all compile: prep $(TARGETS)

.PHONY: prep

prep:
	@echo "      [Info] Use "make SHOW=1" to trace executed commands."
	@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi
	@if [ "$(BIT_APP_PREFIX)" = "" ] ; then echo WARNING: BIT_APP_PREFIX not set ; exit 255 ; fi
	@[ ! -x $(CONFIG)/bin ] && mkdir -p $(CONFIG)/bin; true
	@[ ! -x $(CONFIG)/inc ] && mkdir -p $(CONFIG)/inc; true
	@[ ! -x $(CONFIG)/obj ] && mkdir -p $(CONFIG)/obj; true
	@[ ! -f $(CONFIG)/inc/bit.h ] && cp projects/ejs-vxworks-default-bit.h $(CONFIG)/inc/bit.h ; true
	@[ ! -f $(CONFIG)/inc/bitos.h ] && cp src/bitos.h $(CONFIG)/inc/bitos.h ; true
	@if ! diff $(CONFIG)/inc/bit.h projects/ejs-vxworks-default-bit.h >/dev/null ; then\
		echo cp projects/ejs-vxworks-default-bit.h $(CONFIG)/inc/bit.h  ; \
		cp projects/ejs-vxworks-default-bit.h $(CONFIG)/inc/bit.h  ; \
	fi; true

clean:
	rm -rf $(CONFIG)/bin/libmpr.out
	rm -rf $(CONFIG)/bin/libmprssl.out
	rm -rf $(CONFIG)/bin/ejsman.out
	rm -rf $(CONFIG)/bin/makerom.out
	rm -rf $(CONFIG)/bin/ca.crt
	rm -rf $(CONFIG)/bin/libpcre.out
	rm -rf $(CONFIG)/bin/libhttp.out
	rm -rf $(CONFIG)/bin/http.out
	rm -rf $(CONFIG)/bin/libsqlite3.out
	rm -rf $(CONFIG)/bin/sqlite.out
	rm -rf $(CONFIG)/bin/libzlib.out
	rm -rf $(CONFIG)/bin/libejs.out
	rm -rf $(CONFIG)/bin/ejs.out
	rm -rf $(CONFIG)/bin/ejsc.out
	rm -rf $(CONFIG)/bin/ejsmod.out
	rm -rf $(CONFIG)/bin/ejsrun.out
	rm -rf $(CONFIG)/bin/jem.es
	rm -rf $(CONFIG)/bin/jem.out
	rm -rf $(CONFIG)/bin/ejs.db.mod
	rm -rf $(CONFIG)/bin/ejs.db.mapper.mod
	rm -rf $(CONFIG)/bin/ejs.db.sqlite.mod
	rm -rf $(CONFIG)/bin/libejs.db.sqlite.out
	rm -rf $(CONFIG)/bin/ejs.web.mod
	rm -rf $(CONFIG)/bin/libejs.web.out
	rm -rf $(CONFIG)/bin/www
	rm -rf $(CONFIG)/bin/ejs.template.mod
	rm -rf $(CONFIG)/bin/libejs.zlib.out
	rm -rf $(CONFIG)/bin/mvc.es
	rm -rf $(CONFIG)/bin/ejs.mvc.mod
	rm -rf $(CONFIG)/bin/utest.es
	rm -rf $(CONFIG)/bin/utest.worker
	rm -rf $(CONFIG)/bin/utest.out
	rm -rf $(CONFIG)/obj/mprLib.o
	rm -rf $(CONFIG)/obj/mprSsl.o
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

$(CONFIG)/bin/libmpr.out: $(DEPS_5)
	@echo '      [Link] libmpr'
	$(CC) -r -o $(CONFIG)/bin/libmpr.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/mprLib.o 

#
#   est.h
#
$(CONFIG)/inc/est.h: $(DEPS_6)

#
#   mprSsl.o
#
DEPS_7 += $(CONFIG)/inc/bit.h
DEPS_7 += $(CONFIG)/inc/mpr.h
DEPS_7 += $(CONFIG)/inc/est.h

$(CONFIG)/obj/mprSsl.o: \
    src/deps/mpr/mprSsl.c $(DEPS_7)
	@echo '   [Compile] src/deps/mpr/mprSsl.c'
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprSsl.c

#
#   libmprssl
#
DEPS_8 += $(CONFIG)/bin/libmpr.out
DEPS_8 += $(CONFIG)/obj/mprSsl.o

LIBS_8 += -lmpr

$(CONFIG)/bin/libmprssl.out: $(DEPS_8)
	@echo '      [Link] libmprssl'
	$(CC) -r -o $(CONFIG)/bin/libmprssl.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/mprSsl.o 

#
#   manager.o
#
DEPS_9 += $(CONFIG)/inc/bit.h
DEPS_9 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/manager.o: \
    src/deps/mpr/manager.c $(DEPS_9)
	@echo '   [Compile] src/deps/mpr/manager.c'
	$(CC) -c -o $(CONFIG)/obj/manager.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/manager.c

#
#   manager
#
DEPS_10 += $(CONFIG)/bin/libmpr.out
DEPS_10 += $(CONFIG)/obj/manager.o

LIBS_10 += -lmpr

$(CONFIG)/bin/ejsman.out: $(DEPS_10)
	@echo '      [Link] manager'
	$(CC) -o $(CONFIG)/bin/ejsman.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/manager.o $(LDFLAGS)

#
#   makerom.o
#
DEPS_11 += $(CONFIG)/inc/bit.h
DEPS_11 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/makerom.o: \
    src/deps/mpr/makerom.c $(DEPS_11)
	@echo '   [Compile] src/deps/mpr/makerom.c'
	$(CC) -c -o $(CONFIG)/obj/makerom.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/makerom.c

#
#   makerom
#
DEPS_12 += $(CONFIG)/bin/libmpr.out
DEPS_12 += $(CONFIG)/obj/makerom.o

LIBS_12 += -lmpr

$(CONFIG)/bin/makerom.out: $(DEPS_12)
	@echo '      [Link] makerom'
	$(CC) -o $(CONFIG)/bin/makerom.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o $(LDFLAGS)

#
#   ca-crt
#
DEPS_13 += src/deps/est/ca.crt

$(CONFIG)/bin/ca.crt: $(DEPS_13)
	@echo '      [File] $(CONFIG)/bin/ca.crt'
	mkdir -p "$(CONFIG)/bin"
	cp "src/deps/est/ca.crt" "$(CONFIG)/bin/ca.crt"

#
#   pcre.h
#
$(CONFIG)/inc/pcre.h: $(DEPS_14)
	@echo '      [File] $(CONFIG)/inc/pcre.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/pcre/pcre.h" "$(CONFIG)/inc/pcre.h"

#
#   pcre.o
#
DEPS_15 += $(CONFIG)/inc/bit.h
DEPS_15 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
    src/deps/pcre/pcre.c $(DEPS_15)
	@echo '   [Compile] src/deps/pcre/pcre.c'
	$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/pcre/pcre.c

#
#   libpcre
#
DEPS_16 += $(CONFIG)/inc/pcre.h
DEPS_16 += $(CONFIG)/obj/pcre.o

$(CONFIG)/bin/libpcre.out: $(DEPS_16)
	@echo '      [Link] libpcre'
	$(CC) -r -o $(CONFIG)/bin/libpcre.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/pcre.o 

#
#   http.h
#
$(CONFIG)/inc/http.h: $(DEPS_17)
	@echo '      [File] $(CONFIG)/inc/http.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/http/http.h" "$(CONFIG)/inc/http.h"

#
#   httpLib.o
#
DEPS_18 += $(CONFIG)/inc/bit.h
DEPS_18 += $(CONFIG)/inc/http.h
DEPS_18 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/httpLib.o: \
    src/deps/http/httpLib.c $(DEPS_18)
	@echo '   [Compile] src/deps/http/httpLib.c'
	$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/httpLib.c

#
#   libhttp
#
DEPS_19 += $(CONFIG)/bin/libmpr.out
DEPS_19 += $(CONFIG)/bin/libpcre.out
DEPS_19 += $(CONFIG)/inc/http.h
DEPS_19 += $(CONFIG)/obj/httpLib.o

LIBS_19 += -lpcre
LIBS_19 += -lmpr

$(CONFIG)/bin/libhttp.out: $(DEPS_19)
	@echo '      [Link] libhttp'
	$(CC) -r -o $(CONFIG)/bin/libhttp.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/httpLib.o 

#
#   http.o
#
DEPS_20 += $(CONFIG)/inc/bit.h
DEPS_20 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/http.o: \
    src/deps/http/http.c $(DEPS_20)
	@echo '   [Compile] src/deps/http/http.c'
	$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/http.c

#
#   http
#
DEPS_21 += $(CONFIG)/bin/libhttp.out
DEPS_21 += $(CONFIG)/obj/http.o

LIBS_21 += -lhttp
LIBS_21 += -lpcre
LIBS_21 += -lmpr

$(CONFIG)/bin/http.out: $(DEPS_21)
	@echo '      [Link] http'
	$(CC) -o $(CONFIG)/bin/http.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o $(LDFLAGS)

#
#   sqlite3.h
#
$(CONFIG)/inc/sqlite3.h: $(DEPS_22)
	@echo '      [File] $(CONFIG)/inc/sqlite3.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/sqlite/sqlite3.h" "$(CONFIG)/inc/sqlite3.h"

#
#   sqlite3.o
#
DEPS_23 += $(CONFIG)/inc/bit.h
DEPS_23 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite3.o: \
    src/deps/sqlite/sqlite3.c $(DEPS_23)
	@echo '   [Compile] src/deps/sqlite/sqlite3.c'
	$(CC) -c -o $(CONFIG)/obj/sqlite3.o -fno-builtin -fno-defer-pop -fvolatile $(DFLAGS) $(IFLAGS) src/deps/sqlite/sqlite3.c

ifeq ($(BIT_PACK_SQLITE),1)
#
#   libsqlite3
#
DEPS_24 += $(CONFIG)/inc/sqlite3.h
DEPS_24 += $(CONFIG)/obj/sqlite3.o

$(CONFIG)/bin/libsqlite3.out: $(DEPS_24)
	@echo '      [Link] libsqlite3'
	$(CC) -r -o $(CONFIG)/bin/libsqlite3.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/sqlite3.o 
endif

#
#   sqlite.o
#
DEPS_25 += $(CONFIG)/inc/bit.h
DEPS_25 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite.o: \
    src/deps/sqlite/sqlite.c $(DEPS_25)
	@echo '   [Compile] src/deps/sqlite/sqlite.c'
	$(CC) -c -o $(CONFIG)/obj/sqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/sqlite/sqlite.c

ifeq ($(BIT_PACK_SQLITE),1)
#
#   sqlite
#
ifeq ($(BIT_PACK_SQLITE),1)
    DEPS_26 += $(CONFIG)/bin/libsqlite3.out
endif
DEPS_26 += $(CONFIG)/obj/sqlite.o

ifeq ($(BIT_PACK_SQLITE),1)
    LIBS_26 += -lsqlite3
endif

$(CONFIG)/bin/sqlite.out: $(DEPS_26)
	@echo '      [Link] sqlite'
	$(CC) -o $(CONFIG)/bin/sqlite.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/sqlite.o $(LDFLAGS)
endif

#
#   zlib.h
#
$(CONFIG)/inc/zlib.h: $(DEPS_27)
	@echo '      [File] $(CONFIG)/inc/zlib.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/zlib/zlib.h" "$(CONFIG)/inc/zlib.h"

#
#   zlib.o
#
DEPS_28 += $(CONFIG)/inc/bit.h
DEPS_28 += $(CONFIG)/inc/zlib.h

$(CONFIG)/obj/zlib.o: \
    src/deps/zlib/zlib.c $(DEPS_28)
	@echo '   [Compile] src/deps/zlib/zlib.c'
	$(CC) -c -o $(CONFIG)/obj/zlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/zlib/zlib.c

#
#   libzlib
#
DEPS_29 += $(CONFIG)/inc/zlib.h
DEPS_29 += $(CONFIG)/obj/zlib.o

$(CONFIG)/bin/libzlib.out: $(DEPS_29)
	@echo '      [Link] libzlib'
	$(CC) -r -o $(CONFIG)/bin/libzlib.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/zlib.o 

#
#   ejs.cache.local.slots.h
#
$(CONFIG)/inc/ejs.cache.local.slots.h: $(DEPS_30)
	@echo '      [File] $(CONFIG)/inc/ejs.cache.local.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.cache.local.slots.h" "$(CONFIG)/inc/ejs.cache.local.slots.h"

#
#   ejs.db.sqlite.slots.h
#
$(CONFIG)/inc/ejs.db.sqlite.slots.h: $(DEPS_31)
	@echo '      [File] $(CONFIG)/inc/ejs.db.sqlite.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.db.sqlite.slots.h" "$(CONFIG)/inc/ejs.db.sqlite.slots.h"

#
#   ejs.slots.h
#
$(CONFIG)/inc/ejs.slots.h: $(DEPS_32)
	@echo '      [File] $(CONFIG)/inc/ejs.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.slots.h" "$(CONFIG)/inc/ejs.slots.h"

#
#   ejs.web.slots.h
#
$(CONFIG)/inc/ejs.web.slots.h: $(DEPS_33)
	@echo '      [File] $(CONFIG)/inc/ejs.web.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.web.slots.h" "$(CONFIG)/inc/ejs.web.slots.h"

#
#   ejs.zlib.slots.h
#
$(CONFIG)/inc/ejs.zlib.slots.h: $(DEPS_34)
	@echo '      [File] $(CONFIG)/inc/ejs.zlib.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.zlib.slots.h" "$(CONFIG)/inc/ejs.zlib.slots.h"

#
#   ejsByteCode.h
#
$(CONFIG)/inc/ejsByteCode.h: $(DEPS_35)
	@echo '      [File] $(CONFIG)/inc/ejsByteCode.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsByteCode.h" "$(CONFIG)/inc/ejsByteCode.h"

#
#   ejsByteCodeTable.h
#
$(CONFIG)/inc/ejsByteCodeTable.h: $(DEPS_36)
	@echo '      [File] $(CONFIG)/inc/ejsByteCodeTable.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsByteCodeTable.h" "$(CONFIG)/inc/ejsByteCodeTable.h"

#
#   ejsCustomize.h
#
$(CONFIG)/inc/ejsCustomize.h: $(DEPS_37)
	@echo '      [File] $(CONFIG)/inc/ejsCustomize.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsCustomize.h" "$(CONFIG)/inc/ejsCustomize.h"

#
#   ejs.h
#
DEPS_38 += $(CONFIG)/inc/mpr.h
DEPS_38 += $(CONFIG)/inc/http.h
DEPS_38 += $(CONFIG)/inc/ejsByteCode.h
DEPS_38 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_38 += $(CONFIG)/inc/ejs.slots.h
DEPS_38 += $(CONFIG)/inc/ejsCustomize.h

$(CONFIG)/inc/ejs.h: $(DEPS_38)
	@echo '      [File] $(CONFIG)/inc/ejs.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejs.h" "$(CONFIG)/inc/ejs.h"

#
#   ejsCompiler.h
#
$(CONFIG)/inc/ejsCompiler.h: $(DEPS_39)
	@echo '      [File] $(CONFIG)/inc/ejsCompiler.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsCompiler.h" "$(CONFIG)/inc/ejsCompiler.h"

#
#   ecAst.o
#
DEPS_40 += $(CONFIG)/inc/bit.h
DEPS_40 += $(CONFIG)/inc/ejsCompiler.h
DEPS_40 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ecAst.o: \
    src/compiler/ecAst.c $(DEPS_40)
	@echo '   [Compile] src/compiler/ecAst.c'
	$(CC) -c -o $(CONFIG)/obj/ecAst.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecAst.c

#
#   ecCodeGen.o
#
DEPS_41 += $(CONFIG)/inc/bit.h
DEPS_41 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecCodeGen.o: \
    src/compiler/ecCodeGen.c $(DEPS_41)
	@echo '   [Compile] src/compiler/ecCodeGen.c'
	$(CC) -c -o $(CONFIG)/obj/ecCodeGen.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCodeGen.c

#
#   ecCompiler.o
#
DEPS_42 += $(CONFIG)/inc/bit.h
DEPS_42 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecCompiler.o: \
    src/compiler/ecCompiler.c $(DEPS_42)
	@echo '   [Compile] src/compiler/ecCompiler.c'
	$(CC) -c -o $(CONFIG)/obj/ecCompiler.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCompiler.c

#
#   ecLex.o
#
DEPS_43 += $(CONFIG)/inc/bit.h
DEPS_43 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecLex.o: \
    src/compiler/ecLex.c $(DEPS_43)
	@echo '   [Compile] src/compiler/ecLex.c'
	$(CC) -c -o $(CONFIG)/obj/ecLex.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecLex.c

#
#   ecModuleWrite.o
#
DEPS_44 += $(CONFIG)/inc/bit.h
DEPS_44 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecModuleWrite.o: \
    src/compiler/ecModuleWrite.c $(DEPS_44)
	@echo '   [Compile] src/compiler/ecModuleWrite.c'
	$(CC) -c -o $(CONFIG)/obj/ecModuleWrite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecModuleWrite.c

#
#   ecParser.o
#
DEPS_45 += $(CONFIG)/inc/bit.h
DEPS_45 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecParser.o: \
    src/compiler/ecParser.c $(DEPS_45)
	@echo '   [Compile] src/compiler/ecParser.c'
	$(CC) -c -o $(CONFIG)/obj/ecParser.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecParser.c

#
#   ecState.o
#
DEPS_46 += $(CONFIG)/inc/bit.h
DEPS_46 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecState.o: \
    src/compiler/ecState.c $(DEPS_46)
	@echo '   [Compile] src/compiler/ecState.c'
	$(CC) -c -o $(CONFIG)/obj/ecState.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecState.c

#
#   dtoa.o
#
DEPS_47 += $(CONFIG)/inc/bit.h
DEPS_47 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/dtoa.o: \
    src/core/src/dtoa.c $(DEPS_47)
	@echo '   [Compile] src/core/src/dtoa.c'
	$(CC) -c -o $(CONFIG)/obj/dtoa.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/dtoa.c

#
#   ejsApp.o
#
DEPS_48 += $(CONFIG)/inc/bit.h
DEPS_48 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsApp.o: \
    src/core/src/ejsApp.c $(DEPS_48)
	@echo '   [Compile] src/core/src/ejsApp.c'
	$(CC) -c -o $(CONFIG)/obj/ejsApp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsApp.c

#
#   ejsArray.o
#
DEPS_49 += $(CONFIG)/inc/bit.h
DEPS_49 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsArray.o: \
    src/core/src/ejsArray.c $(DEPS_49)
	@echo '   [Compile] src/core/src/ejsArray.c'
	$(CC) -c -o $(CONFIG)/obj/ejsArray.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsArray.c

#
#   ejsBlock.o
#
DEPS_50 += $(CONFIG)/inc/bit.h
DEPS_50 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsBlock.o: \
    src/core/src/ejsBlock.c $(DEPS_50)
	@echo '   [Compile] src/core/src/ejsBlock.c'
	$(CC) -c -o $(CONFIG)/obj/ejsBlock.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBlock.c

#
#   ejsBoolean.o
#
DEPS_51 += $(CONFIG)/inc/bit.h
DEPS_51 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsBoolean.o: \
    src/core/src/ejsBoolean.c $(DEPS_51)
	@echo '   [Compile] src/core/src/ejsBoolean.c'
	$(CC) -c -o $(CONFIG)/obj/ejsBoolean.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBoolean.c

#
#   ejsByteArray.o
#
DEPS_52 += $(CONFIG)/inc/bit.h
DEPS_52 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsByteArray.o: \
    src/core/src/ejsByteArray.c $(DEPS_52)
	@echo '   [Compile] src/core/src/ejsByteArray.c'
	$(CC) -c -o $(CONFIG)/obj/ejsByteArray.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsByteArray.c

#
#   ejsCache.o
#
DEPS_53 += $(CONFIG)/inc/bit.h
DEPS_53 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsCache.o: \
    src/core/src/ejsCache.c $(DEPS_53)
	@echo '   [Compile] src/core/src/ejsCache.c'
	$(CC) -c -o $(CONFIG)/obj/ejsCache.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCache.c

#
#   ejsCmd.o
#
DEPS_54 += $(CONFIG)/inc/bit.h
DEPS_54 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsCmd.o: \
    src/core/src/ejsCmd.c $(DEPS_54)
	@echo '   [Compile] src/core/src/ejsCmd.c'
	$(CC) -c -o $(CONFIG)/obj/ejsCmd.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCmd.c

#
#   ejsConfig.o
#
DEPS_55 += $(CONFIG)/inc/bit.h
DEPS_55 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsConfig.o: \
    src/core/src/ejsConfig.c $(DEPS_55)
	@echo '   [Compile] src/core/src/ejsConfig.c'
	$(CC) -c -o $(CONFIG)/obj/ejsConfig.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsConfig.c

#
#   ejsDate.o
#
DEPS_56 += $(CONFIG)/inc/bit.h
DEPS_56 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsDate.o: \
    src/core/src/ejsDate.c $(DEPS_56)
	@echo '   [Compile] src/core/src/ejsDate.c'
	$(CC) -c -o $(CONFIG)/obj/ejsDate.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDate.c

#
#   ejsDebug.o
#
DEPS_57 += $(CONFIG)/inc/bit.h
DEPS_57 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsDebug.o: \
    src/core/src/ejsDebug.c $(DEPS_57)
	@echo '   [Compile] src/core/src/ejsDebug.c'
	$(CC) -c -o $(CONFIG)/obj/ejsDebug.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDebug.c

#
#   ejsError.o
#
DEPS_58 += $(CONFIG)/inc/bit.h
DEPS_58 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsError.o: \
    src/core/src/ejsError.c $(DEPS_58)
	@echo '   [Compile] src/core/src/ejsError.c'
	$(CC) -c -o $(CONFIG)/obj/ejsError.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsError.c

#
#   ejsFile.o
#
DEPS_59 += $(CONFIG)/inc/bit.h
DEPS_59 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFile.o: \
    src/core/src/ejsFile.c $(DEPS_59)
	@echo '   [Compile] src/core/src/ejsFile.c'
	$(CC) -c -o $(CONFIG)/obj/ejsFile.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFile.c

#
#   ejsFileSystem.o
#
DEPS_60 += $(CONFIG)/inc/bit.h
DEPS_60 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFileSystem.o: \
    src/core/src/ejsFileSystem.c $(DEPS_60)
	@echo '   [Compile] src/core/src/ejsFileSystem.c'
	$(CC) -c -o $(CONFIG)/obj/ejsFileSystem.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFileSystem.c

#
#   ejsFrame.o
#
DEPS_61 += $(CONFIG)/inc/bit.h
DEPS_61 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFrame.o: \
    src/core/src/ejsFrame.c $(DEPS_61)
	@echo '   [Compile] src/core/src/ejsFrame.c'
	$(CC) -c -o $(CONFIG)/obj/ejsFrame.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFrame.c

#
#   ejsFunction.o
#
DEPS_62 += $(CONFIG)/inc/bit.h
DEPS_62 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFunction.o: \
    src/core/src/ejsFunction.c $(DEPS_62)
	@echo '   [Compile] src/core/src/ejsFunction.c'
	$(CC) -c -o $(CONFIG)/obj/ejsFunction.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFunction.c

#
#   ejsGC.o
#
DEPS_63 += $(CONFIG)/inc/bit.h
DEPS_63 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsGC.o: \
    src/core/src/ejsGC.c $(DEPS_63)
	@echo '   [Compile] src/core/src/ejsGC.c'
	$(CC) -c -o $(CONFIG)/obj/ejsGC.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGC.c

#
#   ejsGlobal.o
#
DEPS_64 += $(CONFIG)/inc/bit.h
DEPS_64 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsGlobal.o: \
    src/core/src/ejsGlobal.c $(DEPS_64)
	@echo '   [Compile] src/core/src/ejsGlobal.c'
	$(CC) -c -o $(CONFIG)/obj/ejsGlobal.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGlobal.c

#
#   ejsHttp.o
#
DEPS_65 += $(CONFIG)/inc/bit.h
DEPS_65 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsHttp.o: \
    src/core/src/ejsHttp.c $(DEPS_65)
	@echo '   [Compile] src/core/src/ejsHttp.c'
	$(CC) -c -o $(CONFIG)/obj/ejsHttp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsHttp.c

#
#   ejsIterator.o
#
DEPS_66 += $(CONFIG)/inc/bit.h
DEPS_66 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsIterator.o: \
    src/core/src/ejsIterator.c $(DEPS_66)
	@echo '   [Compile] src/core/src/ejsIterator.c'
	$(CC) -c -o $(CONFIG)/obj/ejsIterator.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsIterator.c

#
#   ejsJSON.o
#
DEPS_67 += $(CONFIG)/inc/bit.h
DEPS_67 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsJSON.o: \
    src/core/src/ejsJSON.c $(DEPS_67)
	@echo '   [Compile] src/core/src/ejsJSON.c'
	$(CC) -c -o $(CONFIG)/obj/ejsJSON.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsJSON.c

#
#   ejsLocalCache.o
#
DEPS_68 += $(CONFIG)/inc/bit.h
DEPS_68 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsLocalCache.o: \
    src/core/src/ejsLocalCache.c $(DEPS_68)
	@echo '   [Compile] src/core/src/ejsLocalCache.c'
	$(CC) -c -o $(CONFIG)/obj/ejsLocalCache.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsLocalCache.c

#
#   ejsMath.o
#
DEPS_69 += $(CONFIG)/inc/bit.h
DEPS_69 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMath.o: \
    src/core/src/ejsMath.c $(DEPS_69)
	@echo '   [Compile] src/core/src/ejsMath.c'
	$(CC) -c -o $(CONFIG)/obj/ejsMath.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMath.c

#
#   ejsMemory.o
#
DEPS_70 += $(CONFIG)/inc/bit.h
DEPS_70 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMemory.o: \
    src/core/src/ejsMemory.c $(DEPS_70)
	@echo '   [Compile] src/core/src/ejsMemory.c'
	$(CC) -c -o $(CONFIG)/obj/ejsMemory.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMemory.c

#
#   ejsMprLog.o
#
DEPS_71 += $(CONFIG)/inc/bit.h
DEPS_71 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMprLog.o: \
    src/core/src/ejsMprLog.c $(DEPS_71)
	@echo '   [Compile] src/core/src/ejsMprLog.c'
	$(CC) -c -o $(CONFIG)/obj/ejsMprLog.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMprLog.c

#
#   ejsNamespace.o
#
DEPS_72 += $(CONFIG)/inc/bit.h
DEPS_72 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNamespace.o: \
    src/core/src/ejsNamespace.c $(DEPS_72)
	@echo '   [Compile] src/core/src/ejsNamespace.c'
	$(CC) -c -o $(CONFIG)/obj/ejsNamespace.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNamespace.c

#
#   ejsNull.o
#
DEPS_73 += $(CONFIG)/inc/bit.h
DEPS_73 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNull.o: \
    src/core/src/ejsNull.c $(DEPS_73)
	@echo '   [Compile] src/core/src/ejsNull.c'
	$(CC) -c -o $(CONFIG)/obj/ejsNull.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNull.c

#
#   ejsNumber.o
#
DEPS_74 += $(CONFIG)/inc/bit.h
DEPS_74 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNumber.o: \
    src/core/src/ejsNumber.c $(DEPS_74)
	@echo '   [Compile] src/core/src/ejsNumber.c'
	$(CC) -c -o $(CONFIG)/obj/ejsNumber.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNumber.c

#
#   ejsObject.o
#
DEPS_75 += $(CONFIG)/inc/bit.h
DEPS_75 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsObject.o: \
    src/core/src/ejsObject.c $(DEPS_75)
	@echo '   [Compile] src/core/src/ejsObject.c'
	$(CC) -c -o $(CONFIG)/obj/ejsObject.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsObject.c

#
#   ejsPath.o
#
DEPS_76 += $(CONFIG)/inc/bit.h
DEPS_76 += $(CONFIG)/inc/ejs.h
DEPS_76 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsPath.o: \
    src/core/src/ejsPath.c $(DEPS_76)
	@echo '   [Compile] src/core/src/ejsPath.c'
	$(CC) -c -o $(CONFIG)/obj/ejsPath.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPath.c

#
#   ejsPot.o
#
DEPS_77 += $(CONFIG)/inc/bit.h
DEPS_77 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsPot.o: \
    src/core/src/ejsPot.c $(DEPS_77)
	@echo '   [Compile] src/core/src/ejsPot.c'
	$(CC) -c -o $(CONFIG)/obj/ejsPot.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPot.c

#
#   ejsRegExp.o
#
DEPS_78 += $(CONFIG)/inc/bit.h
DEPS_78 += $(CONFIG)/inc/ejs.h
DEPS_78 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsRegExp.o: \
    src/core/src/ejsRegExp.c $(DEPS_78)
	@echo '   [Compile] src/core/src/ejsRegExp.c'
	$(CC) -c -o $(CONFIG)/obj/ejsRegExp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsRegExp.c

#
#   ejsSocket.o
#
DEPS_79 += $(CONFIG)/inc/bit.h
DEPS_79 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsSocket.o: \
    src/core/src/ejsSocket.c $(DEPS_79)
	@echo '   [Compile] src/core/src/ejsSocket.c'
	$(CC) -c -o $(CONFIG)/obj/ejsSocket.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSocket.c

#
#   ejsString.o
#
DEPS_80 += $(CONFIG)/inc/bit.h
DEPS_80 += $(CONFIG)/inc/ejs.h
DEPS_80 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsString.o: \
    src/core/src/ejsString.c $(DEPS_80)
	@echo '   [Compile] src/core/src/ejsString.c'
	$(CC) -c -o $(CONFIG)/obj/ejsString.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsString.c

#
#   ejsSystem.o
#
DEPS_81 += $(CONFIG)/inc/bit.h
DEPS_81 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsSystem.o: \
    src/core/src/ejsSystem.c $(DEPS_81)
	@echo '   [Compile] src/core/src/ejsSystem.c'
	$(CC) -c -o $(CONFIG)/obj/ejsSystem.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSystem.c

#
#   ejsTimer.o
#
DEPS_82 += $(CONFIG)/inc/bit.h
DEPS_82 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsTimer.o: \
    src/core/src/ejsTimer.c $(DEPS_82)
	@echo '   [Compile] src/core/src/ejsTimer.c'
	$(CC) -c -o $(CONFIG)/obj/ejsTimer.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsTimer.c

#
#   ejsType.o
#
DEPS_83 += $(CONFIG)/inc/bit.h
DEPS_83 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsType.o: \
    src/core/src/ejsType.c $(DEPS_83)
	@echo '   [Compile] src/core/src/ejsType.c'
	$(CC) -c -o $(CONFIG)/obj/ejsType.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsType.c

#
#   ejsUri.o
#
DEPS_84 += $(CONFIG)/inc/bit.h
DEPS_84 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsUri.o: \
    src/core/src/ejsUri.c $(DEPS_84)
	@echo '   [Compile] src/core/src/ejsUri.c'
	$(CC) -c -o $(CONFIG)/obj/ejsUri.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsUri.c

#
#   ejsVoid.o
#
DEPS_85 += $(CONFIG)/inc/bit.h
DEPS_85 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsVoid.o: \
    src/core/src/ejsVoid.c $(DEPS_85)
	@echo '   [Compile] src/core/src/ejsVoid.c'
	$(CC) -c -o $(CONFIG)/obj/ejsVoid.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsVoid.c

#
#   ejsWebSocket.o
#
DEPS_86 += $(CONFIG)/inc/bit.h
DEPS_86 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsWebSocket.o: \
    src/core/src/ejsWebSocket.c $(DEPS_86)
	@echo '   [Compile] src/core/src/ejsWebSocket.c'
	$(CC) -c -o $(CONFIG)/obj/ejsWebSocket.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWebSocket.c

#
#   ejsWorker.o
#
DEPS_87 += $(CONFIG)/inc/bit.h
DEPS_87 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsWorker.o: \
    src/core/src/ejsWorker.c $(DEPS_87)
	@echo '   [Compile] src/core/src/ejsWorker.c'
	$(CC) -c -o $(CONFIG)/obj/ejsWorker.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWorker.c

#
#   ejsXML.o
#
DEPS_88 += $(CONFIG)/inc/bit.h
DEPS_88 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXML.o: \
    src/core/src/ejsXML.c $(DEPS_88)
	@echo '   [Compile] src/core/src/ejsXML.c'
	$(CC) -c -o $(CONFIG)/obj/ejsXML.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXML.c

#
#   ejsXMLList.o
#
DEPS_89 += $(CONFIG)/inc/bit.h
DEPS_89 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXMLList.o: \
    src/core/src/ejsXMLList.c $(DEPS_89)
	@echo '   [Compile] src/core/src/ejsXMLList.c'
	$(CC) -c -o $(CONFIG)/obj/ejsXMLList.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLList.c

#
#   ejsXMLLoader.o
#
DEPS_90 += $(CONFIG)/inc/bit.h
DEPS_90 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXMLLoader.o: \
    src/core/src/ejsXMLLoader.c $(DEPS_90)
	@echo '   [Compile] src/core/src/ejsXMLLoader.c'
	$(CC) -c -o $(CONFIG)/obj/ejsXMLLoader.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLLoader.c

#
#   ejsByteCode.o
#
DEPS_91 += $(CONFIG)/inc/bit.h
DEPS_91 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsByteCode.o: \
    src/vm/ejsByteCode.c $(DEPS_91)
	@echo '   [Compile] src/vm/ejsByteCode.c'
	$(CC) -c -o $(CONFIG)/obj/ejsByteCode.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsByteCode.c

#
#   ejsException.o
#
DEPS_92 += $(CONFIG)/inc/bit.h
DEPS_92 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsException.o: \
    src/vm/ejsException.c $(DEPS_92)
	@echo '   [Compile] src/vm/ejsException.c'
	$(CC) -c -o $(CONFIG)/obj/ejsException.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsException.c

#
#   ejsHelper.o
#
DEPS_93 += $(CONFIG)/inc/bit.h
DEPS_93 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsHelper.o: \
    src/vm/ejsHelper.c $(DEPS_93)
	@echo '   [Compile] src/vm/ejsHelper.c'
	$(CC) -c -o $(CONFIG)/obj/ejsHelper.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsHelper.c

#
#   ejsInterp.o
#
DEPS_94 += $(CONFIG)/inc/bit.h
DEPS_94 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsInterp.o: \
    src/vm/ejsInterp.c $(DEPS_94)
	@echo '   [Compile] src/vm/ejsInterp.c'
	$(CC) -c -o $(CONFIG)/obj/ejsInterp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsInterp.c

#
#   ejsLoader.o
#
DEPS_95 += $(CONFIG)/inc/bit.h
DEPS_95 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsLoader.o: \
    src/vm/ejsLoader.c $(DEPS_95)
	@echo '   [Compile] src/vm/ejsLoader.c'
	$(CC) -c -o $(CONFIG)/obj/ejsLoader.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsLoader.c

#
#   ejsModule.o
#
DEPS_96 += $(CONFIG)/inc/bit.h
DEPS_96 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsModule.o: \
    src/vm/ejsModule.c $(DEPS_96)
	@echo '   [Compile] src/vm/ejsModule.c'
	$(CC) -c -o $(CONFIG)/obj/ejsModule.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsModule.c

#
#   ejsScope.o
#
DEPS_97 += $(CONFIG)/inc/bit.h
DEPS_97 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsScope.o: \
    src/vm/ejsScope.c $(DEPS_97)
	@echo '   [Compile] src/vm/ejsScope.c'
	$(CC) -c -o $(CONFIG)/obj/ejsScope.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsScope.c

#
#   ejsService.o
#
DEPS_98 += $(CONFIG)/inc/bit.h
DEPS_98 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsService.o: \
    src/vm/ejsService.c $(DEPS_98)
	@echo '   [Compile] src/vm/ejsService.c'
	$(CC) -c -o $(CONFIG)/obj/ejsService.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsService.c

#
#   libejs
#
DEPS_99 += $(CONFIG)/bin/libhttp.out
DEPS_99 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_99 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_99 += $(CONFIG)/inc/ejs.slots.h
DEPS_99 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_99 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_99 += $(CONFIG)/inc/bitos.h
DEPS_99 += $(CONFIG)/inc/ejs.h
DEPS_99 += $(CONFIG)/inc/ejsByteCode.h
DEPS_99 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_99 += $(CONFIG)/inc/ejsCompiler.h
DEPS_99 += $(CONFIG)/inc/ejsCustomize.h
DEPS_99 += $(CONFIG)/obj/ecAst.o
DEPS_99 += $(CONFIG)/obj/ecCodeGen.o
DEPS_99 += $(CONFIG)/obj/ecCompiler.o
DEPS_99 += $(CONFIG)/obj/ecLex.o
DEPS_99 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_99 += $(CONFIG)/obj/ecParser.o
DEPS_99 += $(CONFIG)/obj/ecState.o
DEPS_99 += $(CONFIG)/obj/dtoa.o
DEPS_99 += $(CONFIG)/obj/ejsApp.o
DEPS_99 += $(CONFIG)/obj/ejsArray.o
DEPS_99 += $(CONFIG)/obj/ejsBlock.o
DEPS_99 += $(CONFIG)/obj/ejsBoolean.o
DEPS_99 += $(CONFIG)/obj/ejsByteArray.o
DEPS_99 += $(CONFIG)/obj/ejsCache.o
DEPS_99 += $(CONFIG)/obj/ejsCmd.o
DEPS_99 += $(CONFIG)/obj/ejsConfig.o
DEPS_99 += $(CONFIG)/obj/ejsDate.o
DEPS_99 += $(CONFIG)/obj/ejsDebug.o
DEPS_99 += $(CONFIG)/obj/ejsError.o
DEPS_99 += $(CONFIG)/obj/ejsFile.o
DEPS_99 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_99 += $(CONFIG)/obj/ejsFrame.o
DEPS_99 += $(CONFIG)/obj/ejsFunction.o
DEPS_99 += $(CONFIG)/obj/ejsGC.o
DEPS_99 += $(CONFIG)/obj/ejsGlobal.o
DEPS_99 += $(CONFIG)/obj/ejsHttp.o
DEPS_99 += $(CONFIG)/obj/ejsIterator.o
DEPS_99 += $(CONFIG)/obj/ejsJSON.o
DEPS_99 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_99 += $(CONFIG)/obj/ejsMath.o
DEPS_99 += $(CONFIG)/obj/ejsMemory.o
DEPS_99 += $(CONFIG)/obj/ejsMprLog.o
DEPS_99 += $(CONFIG)/obj/ejsNamespace.o
DEPS_99 += $(CONFIG)/obj/ejsNull.o
DEPS_99 += $(CONFIG)/obj/ejsNumber.o
DEPS_99 += $(CONFIG)/obj/ejsObject.o
DEPS_99 += $(CONFIG)/obj/ejsPath.o
DEPS_99 += $(CONFIG)/obj/ejsPot.o
DEPS_99 += $(CONFIG)/obj/ejsRegExp.o
DEPS_99 += $(CONFIG)/obj/ejsSocket.o
DEPS_99 += $(CONFIG)/obj/ejsString.o
DEPS_99 += $(CONFIG)/obj/ejsSystem.o
DEPS_99 += $(CONFIG)/obj/ejsTimer.o
DEPS_99 += $(CONFIG)/obj/ejsType.o
DEPS_99 += $(CONFIG)/obj/ejsUri.o
DEPS_99 += $(CONFIG)/obj/ejsVoid.o
DEPS_99 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_99 += $(CONFIG)/obj/ejsWorker.o
DEPS_99 += $(CONFIG)/obj/ejsXML.o
DEPS_99 += $(CONFIG)/obj/ejsXMLList.o
DEPS_99 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_99 += $(CONFIG)/obj/ejsByteCode.o
DEPS_99 += $(CONFIG)/obj/ejsException.o
DEPS_99 += $(CONFIG)/obj/ejsHelper.o
DEPS_99 += $(CONFIG)/obj/ejsInterp.o
DEPS_99 += $(CONFIG)/obj/ejsLoader.o
DEPS_99 += $(CONFIG)/obj/ejsModule.o
DEPS_99 += $(CONFIG)/obj/ejsScope.o
DEPS_99 += $(CONFIG)/obj/ejsService.o

LIBS_99 += -lhttp
LIBS_99 += -lpcre
LIBS_99 += -lmpr

$(CONFIG)/bin/libejs.out: $(DEPS_99)
	@echo '      [Link] libejs'
	$(CC) -r -o $(CONFIG)/bin/libejs.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ecAst.o $(CONFIG)/obj/ecCodeGen.o $(CONFIG)/obj/ecCompiler.o $(CONFIG)/obj/ecLex.o $(CONFIG)/obj/ecModuleWrite.o $(CONFIG)/obj/ecParser.o $(CONFIG)/obj/ecState.o $(CONFIG)/obj/dtoa.o $(CONFIG)/obj/ejsApp.o $(CONFIG)/obj/ejsArray.o $(CONFIG)/obj/ejsBlock.o $(CONFIG)/obj/ejsBoolean.o $(CONFIG)/obj/ejsByteArray.o $(CONFIG)/obj/ejsCache.o $(CONFIG)/obj/ejsCmd.o $(CONFIG)/obj/ejsConfig.o $(CONFIG)/obj/ejsDate.o $(CONFIG)/obj/ejsDebug.o $(CONFIG)/obj/ejsError.o $(CONFIG)/obj/ejsFile.o $(CONFIG)/obj/ejsFileSystem.o $(CONFIG)/obj/ejsFrame.o $(CONFIG)/obj/ejsFunction.o $(CONFIG)/obj/ejsGC.o $(CONFIG)/obj/ejsGlobal.o $(CONFIG)/obj/ejsHttp.o $(CONFIG)/obj/ejsIterator.o $(CONFIG)/obj/ejsJSON.o $(CONFIG)/obj/ejsLocalCache.o $(CONFIG)/obj/ejsMath.o $(CONFIG)/obj/ejsMemory.o $(CONFIG)/obj/ejsMprLog.o $(CONFIG)/obj/ejsNamespace.o $(CONFIG)/obj/ejsNull.o $(CONFIG)/obj/ejsNumber.o $(CONFIG)/obj/ejsObject.o $(CONFIG)/obj/ejsPath.o $(CONFIG)/obj/ejsPot.o $(CONFIG)/obj/ejsRegExp.o $(CONFIG)/obj/ejsSocket.o $(CONFIG)/obj/ejsString.o $(CONFIG)/obj/ejsSystem.o $(CONFIG)/obj/ejsTimer.o $(CONFIG)/obj/ejsType.o $(CONFIG)/obj/ejsUri.o $(CONFIG)/obj/ejsVoid.o $(CONFIG)/obj/ejsWebSocket.o $(CONFIG)/obj/ejsWorker.o $(CONFIG)/obj/ejsXML.o $(CONFIG)/obj/ejsXMLList.o $(CONFIG)/obj/ejsXMLLoader.o $(CONFIG)/obj/ejsByteCode.o $(CONFIG)/obj/ejsException.o $(CONFIG)/obj/ejsHelper.o $(CONFIG)/obj/ejsInterp.o $(CONFIG)/obj/ejsLoader.o $(CONFIG)/obj/ejsModule.o $(CONFIG)/obj/ejsScope.o $(CONFIG)/obj/ejsService.o 

#
#   ejs.o
#
DEPS_100 += $(CONFIG)/inc/bit.h
DEPS_100 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejs.o: \
    src/cmd/ejs.c $(DEPS_100)
	@echo '   [Compile] src/cmd/ejs.c'
	$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejs.c

#
#   ejs
#
DEPS_101 += $(CONFIG)/bin/libejs.out
DEPS_101 += $(CONFIG)/obj/ejs.o

LIBS_101 += -lejs
LIBS_101 += -lhttp
LIBS_101 += -lpcre
LIBS_101 += -lmpr

$(CONFIG)/bin/ejs.out: $(DEPS_101)
	@echo '      [Link] ejs'
	$(CC) -o $(CONFIG)/bin/ejs.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejs.o $(LDFLAGS)

#
#   ejsc.o
#
DEPS_102 += $(CONFIG)/inc/bit.h
DEPS_102 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejsc.o: \
    src/cmd/ejsc.c $(DEPS_102)
	@echo '   [Compile] src/cmd/ejsc.c'
	$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsc.c

#
#   ejsc
#
DEPS_103 += $(CONFIG)/bin/libejs.out
DEPS_103 += $(CONFIG)/obj/ejsc.o

LIBS_103 += -lejs
LIBS_103 += -lhttp
LIBS_103 += -lpcre
LIBS_103 += -lmpr

$(CONFIG)/bin/ejsc.out: $(DEPS_103)
	@echo '      [Link] ejsc'
	$(CC) -o $(CONFIG)/bin/ejsc.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsc.o $(LDFLAGS)

#
#   ejsmod.h
#
src/cmd/ejsmod.h: $(DEPS_104)

#
#   ejsmod.o
#
DEPS_105 += $(CONFIG)/inc/bit.h
DEPS_105 += src/cmd/ejsmod.h
DEPS_105 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsmod.o: \
    src/cmd/ejsmod.c $(DEPS_105)
	@echo '   [Compile] src/cmd/ejsmod.c'
	$(CC) -c -o $(CONFIG)/obj/ejsmod.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/ejsmod.c

#
#   doc.o
#
DEPS_106 += $(CONFIG)/inc/bit.h
DEPS_106 += src/cmd/ejsmod.h

$(CONFIG)/obj/doc.o: \
    src/cmd/doc.c $(DEPS_106)
	@echo '   [Compile] src/cmd/doc.c'
	$(CC) -c -o $(CONFIG)/obj/doc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/doc.c

#
#   docFiles.o
#
DEPS_107 += $(CONFIG)/inc/bit.h
DEPS_107 += src/cmd/ejsmod.h

$(CONFIG)/obj/docFiles.o: \
    src/cmd/docFiles.c $(DEPS_107)
	@echo '   [Compile] src/cmd/docFiles.c'
	$(CC) -c -o $(CONFIG)/obj/docFiles.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/docFiles.c

#
#   listing.o
#
DEPS_108 += $(CONFIG)/inc/bit.h
DEPS_108 += src/cmd/ejsmod.h
DEPS_108 += $(CONFIG)/inc/ejsByteCodeTable.h

$(CONFIG)/obj/listing.o: \
    src/cmd/listing.c $(DEPS_108)
	@echo '   [Compile] src/cmd/listing.c'
	$(CC) -c -o $(CONFIG)/obj/listing.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/listing.c

#
#   slotGen.o
#
DEPS_109 += $(CONFIG)/inc/bit.h
DEPS_109 += src/cmd/ejsmod.h

$(CONFIG)/obj/slotGen.o: \
    src/cmd/slotGen.c $(DEPS_109)
	@echo '   [Compile] src/cmd/slotGen.c'
	$(CC) -c -o $(CONFIG)/obj/slotGen.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/slotGen.c

#
#   ejsmod
#
DEPS_110 += $(CONFIG)/bin/libejs.out
DEPS_110 += $(CONFIG)/obj/ejsmod.o
DEPS_110 += $(CONFIG)/obj/doc.o
DEPS_110 += $(CONFIG)/obj/docFiles.o
DEPS_110 += $(CONFIG)/obj/listing.o
DEPS_110 += $(CONFIG)/obj/slotGen.o

LIBS_110 += -lejs
LIBS_110 += -lhttp
LIBS_110 += -lpcre
LIBS_110 += -lmpr

$(CONFIG)/bin/ejsmod.out: $(DEPS_110)
	@echo '      [Link] ejsmod'
	$(CC) -o $(CONFIG)/bin/ejsmod.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsmod.o $(CONFIG)/obj/doc.o $(CONFIG)/obj/docFiles.o $(CONFIG)/obj/listing.o $(CONFIG)/obj/slotGen.o $(LDFLAGS)

#
#   ejsrun.o
#
DEPS_111 += $(CONFIG)/inc/bit.h
DEPS_111 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejsrun.o: \
    src/cmd/ejsrun.c $(DEPS_111)
	@echo '   [Compile] src/cmd/ejsrun.c'
	$(CC) -c -o $(CONFIG)/obj/ejsrun.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsrun.c

#
#   ejsrun
#
DEPS_112 += $(CONFIG)/bin/libejs.out
DEPS_112 += $(CONFIG)/obj/ejsrun.o

LIBS_112 += -lejs
LIBS_112 += -lhttp
LIBS_112 += -lpcre
LIBS_112 += -lmpr

$(CONFIG)/bin/ejsrun.out: $(DEPS_112)
	@echo '      [Link] ejsrun'
	$(CC) -o $(CONFIG)/bin/ejsrun.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LDFLAGS)

#
#   ejs.mod
#
DEPS_113 += $(CONFIG)/bin/ejsc.out
DEPS_113 += $(CONFIG)/bin/ejsmod.out

$(CONFIG)/bin/ejs.mod: $(DEPS_113)
	cd src/core; ../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.mod  --optimize 9 --bind --require null *.es  ; cd ../..
	cd src/core; ../../$(CONFIG)/bin/ejsmod --require null --cslots ../../$(CONFIG)/bin/ejs.mod ; cd ../..
	cd src/core; if ! diff ejs.slots.h ../../$(CONFIG)/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ../../$(CONFIG)/inc; fi ; cd ../..
	cd src/core; rm -f ejs.slots.h ; cd ../..

#
#   ejs.unix.mod
#
DEPS_114 += $(CONFIG)/bin/ejsc.out
DEPS_114 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.unix.mod: $(DEPS_114)
	cd src/jems/ejs.unix; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.unix.mod  --optimize 9 Unix.es ; cd ../../..

#
#   jem.es
#
$(CONFIG)/bin/jem.es: $(DEPS_115)
	cd src/jems/ejs.jem; cp jem.es ../../../$(CONFIG)/bin ; cd ../../..

#
#   jem
#
DEPS_116 += $(CONFIG)/bin/libejs.out
DEPS_116 += $(CONFIG)/bin/jem.es
DEPS_116 += $(CONFIG)/obj/ejsrun.o

LIBS_116 += -lejs
LIBS_116 += -lhttp
LIBS_116 += -lpcre
LIBS_116 += -lmpr

$(CONFIG)/bin/jem.out: $(DEPS_116)
	@echo '      [Link] jem'
	$(CC) -o $(CONFIG)/bin/jem.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LDFLAGS)

#
#   ejs.db.mod
#
DEPS_117 += $(CONFIG)/bin/ejsc.out
DEPS_117 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.db.mod: $(DEPS_117)
	cd src/jems/ejs.db; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.mod  --optimize 9 *.es ; cd ../../..

#
#   ejs.db.mapper.mod
#
DEPS_118 += $(CONFIG)/bin/ejsc.out
DEPS_118 += $(CONFIG)/bin/ejs.mod
DEPS_118 += $(CONFIG)/bin/ejs.db.mod

$(CONFIG)/bin/ejs.db.mapper.mod: $(DEPS_118)
	cd src/jems/ejs.db.mapper; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.mapper.mod  --optimize 9 *.es ; cd ../../..

#
#   ejs.db.sqlite.mod
#
DEPS_119 += $(CONFIG)/bin/ejsc.out
DEPS_119 += $(CONFIG)/bin/ejsmod.out
DEPS_119 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.db.sqlite.mod: $(DEPS_119)
	cd src/jems/ejs.db.sqlite; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.sqlite.mod  --optimize 9 *.es ; cd ../../..

#
#   ejsSqlite.o
#
DEPS_120 += $(CONFIG)/inc/bit.h
DEPS_120 += $(CONFIG)/inc/ejs.h
DEPS_120 += $(CONFIG)/inc/ejs.db.sqlite.slots.h

$(CONFIG)/obj/ejsSqlite.o: \
    src/jems/ejs.db.sqlite/ejsSqlite.c $(DEPS_120)
	@echo '   [Compile] src/jems/ejs.db.sqlite/ejsSqlite.c'
	$(CC) -c -o $(CONFIG)/obj/ejsSqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/jems/ejs.db.sqlite/ejsSqlite.c

#
#   libejs.db.sqlite
#
DEPS_121 += $(CONFIG)/bin/libmpr.out
DEPS_121 += $(CONFIG)/bin/libejs.out
DEPS_121 += $(CONFIG)/bin/ejs.mod
DEPS_121 += $(CONFIG)/bin/ejs.db.sqlite.mod
ifeq ($(BIT_PACK_SQLITE),1)
    DEPS_121 += $(CONFIG)/bin/libsqlite3.out
endif
DEPS_121 += $(CONFIG)/obj/ejsSqlite.o

ifeq ($(BIT_PACK_SQLITE),1)
    LIBS_121 += -lsqlite3
endif
LIBS_121 += -lejs
LIBS_121 += -lmpr
LIBS_121 += -lhttp
LIBS_121 += -lpcre

$(CONFIG)/bin/libejs.db.sqlite.out: $(DEPS_121)
	@echo '      [Link] libejs.db.sqlite'
	$(CC) -r -o $(CONFIG)/bin/libejs.db.sqlite.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsSqlite.o 

#
#   ejs.web.mod
#
DEPS_122 += $(CONFIG)/bin/ejsc.out
DEPS_122 += $(CONFIG)/bin/ejsmod.out
DEPS_122 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.web.mod: $(DEPS_122)
	cd src/jems/ejs.web; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.web.mod  --optimize 9 *.es ; cd ../../..
	cd src/jems/ejs.web; ../../../$(CONFIG)/bin/ejsmod --cslots ../../../$(CONFIG)/bin/ejs.web.mod ; cd ../../..
	cd src/jems/ejs.web; if ! diff ejs.web.slots.h ../../../$(CONFIG)/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../../$(CONFIG)/inc; fi ; cd ../../..
	cd src/jems/ejs.web; rm -f ejs.web.slots.h ; cd ../../..

#
#   ejsWeb.h
#
$(CONFIG)/inc/ejsWeb.h: $(DEPS_123)
	@echo '      [File] $(CONFIG)/inc/ejsWeb.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/jems/ejs.web/ejsWeb.h" "$(CONFIG)/inc/ejsWeb.h"

#
#   ejsHttpServer.o
#
DEPS_124 += $(CONFIG)/inc/bit.h
DEPS_124 += $(CONFIG)/inc/ejs.h
DEPS_124 += $(CONFIG)/inc/ejsCompiler.h
DEPS_124 += $(CONFIG)/inc/ejsWeb.h
DEPS_124 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_124 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/ejsHttpServer.o: \
    src/jems/ejs.web/ejsHttpServer.c $(DEPS_124)
	@echo '   [Compile] src/jems/ejs.web/ejsHttpServer.c'
	$(CC) -c -o $(CONFIG)/obj/ejsHttpServer.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsHttpServer.c

#
#   ejsRequest.o
#
DEPS_125 += $(CONFIG)/inc/bit.h
DEPS_125 += $(CONFIG)/inc/ejs.h
DEPS_125 += $(CONFIG)/inc/ejsCompiler.h
DEPS_125 += $(CONFIG)/inc/ejsWeb.h
DEPS_125 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsRequest.o: \
    src/jems/ejs.web/ejsRequest.c $(DEPS_125)
	@echo '   [Compile] src/jems/ejs.web/ejsRequest.c'
	$(CC) -c -o $(CONFIG)/obj/ejsRequest.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsRequest.c

#
#   ejsSession.o
#
DEPS_126 += $(CONFIG)/inc/bit.h
DEPS_126 += $(CONFIG)/inc/ejs.h
DEPS_126 += $(CONFIG)/inc/ejsWeb.h

$(CONFIG)/obj/ejsSession.o: \
    src/jems/ejs.web/ejsSession.c $(DEPS_126)
	@echo '   [Compile] src/jems/ejs.web/ejsSession.c'
	$(CC) -c -o $(CONFIG)/obj/ejsSession.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsSession.c

#
#   ejsWeb.o
#
DEPS_127 += $(CONFIG)/inc/bit.h
DEPS_127 += $(CONFIG)/inc/ejs.h
DEPS_127 += $(CONFIG)/inc/ejsCompiler.h
DEPS_127 += $(CONFIG)/inc/ejsWeb.h
DEPS_127 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsWeb.o: \
    src/jems/ejs.web/ejsWeb.c $(DEPS_127)
	@echo '   [Compile] src/jems/ejs.web/ejsWeb.c'
	$(CC) -c -o $(CONFIG)/obj/ejsWeb.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/jems/ejs.web/src src/jems/ejs.web/ejsWeb.c

#
#   libejs.web
#
DEPS_128 += $(CONFIG)/bin/libejs.out
DEPS_128 += $(CONFIG)/bin/ejs.mod
DEPS_128 += $(CONFIG)/inc/ejsWeb.h
DEPS_128 += $(CONFIG)/obj/ejsHttpServer.o
DEPS_128 += $(CONFIG)/obj/ejsRequest.o
DEPS_128 += $(CONFIG)/obj/ejsSession.o
DEPS_128 += $(CONFIG)/obj/ejsWeb.o

LIBS_128 += -lejs
LIBS_128 += -lhttp
LIBS_128 += -lpcre
LIBS_128 += -lmpr

$(CONFIG)/bin/libejs.web.out: $(DEPS_128)
	@echo '      [Link] libejs.web'
	$(CC) -r -o $(CONFIG)/bin/libejs.web.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsHttpServer.o $(CONFIG)/obj/ejsRequest.o $(CONFIG)/obj/ejsSession.o $(CONFIG)/obj/ejsWeb.o 

#
#   www
#
$(CONFIG)/bin/www: $(DEPS_129)
	cd src/jems/ejs.web; rm -fr ../../../$(CONFIG)/bin/www ; cd ../../..
	cd src/jems/ejs.web; cp -r www ../../../$(CONFIG)/bin ; cd ../../..

#
#   ejs.template.mod
#
DEPS_130 += $(CONFIG)/bin/ejsc.out
DEPS_130 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.template.mod: $(DEPS_130)
	cd src/jems/ejs.template; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.template.mod  --optimize 9 TemplateParser.es ; cd ../../..

#
#   ejs.zlib.mod
#
DEPS_131 += $(CONFIG)/bin/ejsc.out
DEPS_131 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.zlib.mod: $(DEPS_131)
	cd src/jems/ejs.zlib; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.zlib.mod  --optimize 9 *.es ; cd ../../..

#
#   ejsZlib.o
#
DEPS_132 += $(CONFIG)/inc/bit.h
DEPS_132 += $(CONFIG)/inc/ejs.h
DEPS_132 += $(CONFIG)/inc/zlib.h
DEPS_132 += $(CONFIG)/inc/ejs.zlib.slots.h

$(CONFIG)/obj/ejsZlib.o: \
    src/jems/ejs.zlib/ejsZlib.c $(DEPS_132)
	@echo '   [Compile] src/jems/ejs.zlib/ejsZlib.c'
	$(CC) -c -o $(CONFIG)/obj/ejsZlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/jems/ejs.zlib/ejsZlib.c

#
#   libejs.zlib
#
DEPS_133 += $(CONFIG)/bin/libejs.out
DEPS_133 += $(CONFIG)/bin/ejs.mod
DEPS_133 += $(CONFIG)/bin/ejs.zlib.mod
DEPS_133 += $(CONFIG)/bin/libzlib.out
DEPS_133 += $(CONFIG)/obj/ejsZlib.o

LIBS_133 += -lzlib
LIBS_133 += -lejs
LIBS_133 += -lhttp
LIBS_133 += -lpcre
LIBS_133 += -lmpr

$(CONFIG)/bin/libejs.zlib.out: $(DEPS_133)
	@echo '      [Link] libejs.zlib'
	$(CC) -r -o $(CONFIG)/bin/libejs.zlib.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsZlib.o 

#
#   ejs.tar.mod
#
DEPS_134 += $(CONFIG)/bin/ejsc.out
DEPS_134 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.tar.mod: $(DEPS_134)
	cd src/jems/ejs.tar; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.tar.mod  --optimize 9 *.es ; cd ../../..

#
#   mvc.es
#
$(CONFIG)/bin/mvc.es: $(DEPS_135)
	cd src/jems/ejs.mvc; cp mvc.es ../../../$(CONFIG)/bin ; cd ../../..

#
#   mvc
#
DEPS_136 += $(CONFIG)/bin/libejs.out
DEPS_136 += $(CONFIG)/bin/mvc.es
DEPS_136 += $(CONFIG)/obj/ejsrun.o

LIBS_136 += -lejs
LIBS_136 += -lhttp
LIBS_136 += -lpcre
LIBS_136 += -lmpr

$(CONFIG)/bin/mvc.out: $(DEPS_136)
	@echo '      [Link] mvc'
	$(CC) -o $(CONFIG)/bin/mvc.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LDFLAGS)

#
#   ejs.mvc.mod
#
DEPS_137 += $(CONFIG)/bin/ejsc.out
DEPS_137 += $(CONFIG)/bin/ejs.mod
DEPS_137 += $(CONFIG)/bin/ejs.web.mod
DEPS_137 += $(CONFIG)/bin/ejs.template.mod
DEPS_137 += $(CONFIG)/bin/ejs.unix.mod

$(CONFIG)/bin/ejs.mvc.mod: $(DEPS_137)
	cd src/jems/ejs.mvc; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.mvc.mod  --optimize 9 *.es ; cd ../../..

#
#   utest.es
#
$(CONFIG)/bin/utest.es: $(DEPS_138)
	cd src/jems/ejs.utest; cp utest.es ../../../$(CONFIG)/bin ; cd ../../..

#
#   utest.worker
#
$(CONFIG)/bin/utest.worker: $(DEPS_139)
	cd src/jems/ejs.utest; cp utest.worker ../../../$(CONFIG)/bin ; cd ../../..

#
#   utest
#
DEPS_140 += $(CONFIG)/bin/libejs.out
DEPS_140 += $(CONFIG)/bin/utest.es
DEPS_140 += $(CONFIG)/bin/utest.worker
DEPS_140 += $(CONFIG)/obj/ejsrun.o

LIBS_140 += -lejs
LIBS_140 += -lhttp
LIBS_140 += -lpcre
LIBS_140 += -lmpr

$(CONFIG)/bin/utest.out: $(DEPS_140)
	@echo '      [Link] utest'
	$(CC) -o $(CONFIG)/bin/utest.out $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LDFLAGS)

#
#   version
#
version: $(DEPS_141)
	@echo 2.3.0-1

#
#   stop
#
stop: $(DEPS_142)
	

#
#   installBinary
#
DEPS_143 += stop

installBinary: $(DEPS_143)

#
#   start
#
start: $(DEPS_144)
	

#
#   install
#
DEPS_145 += stop
DEPS_145 += installBinary
DEPS_145 += start

install: $(DEPS_145)
	

#
#   uninstall
#
DEPS_146 += stop

uninstall: $(DEPS_146)

