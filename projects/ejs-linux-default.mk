#
#   ejs-linux-default.mk -- Makefile to build Embedthis Ejscript for linux
#

PRODUCT           := ejs
VERSION           := 2.3.1
BUILD_NUMBER      := 2
PROFILE           := default
ARCH              := $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
OS                := linux
CC                := /usr/bin/gcc
LD                := /usr/bin/ld
CONFIG            := $(OS)-$(ARCH)-$(PROFILE)
LBIN              := $(CONFIG)/bin


CFLAGS            += -fPIC   -w
DFLAGS            += -D_REENTRANT -DPIC  $(patsubst %,-D%,$(filter BIT_%,$(MAKEFLAGS))) 
IFLAGS            += -I$(CONFIG)/inc
LDFLAGS           += '-Wl,--enable-new-dtags' '-Wl,-rpath,$$ORIGIN/' '-rdynamic'
LIBPATHS          += -L$(CONFIG)/bin
LIBS              += -lpthread -lm -lrt -ldl

DEBUG             := debug
CFLAGS-debug      := -g
DFLAGS-debug      := -DBIT_DEBUG
LDFLAGS-debug     := -g
DFLAGS-release    := 
CFLAGS-release    := -O2
LDFLAGS-release   := 
CFLAGS            += $(CFLAGS-$(DEBUG))
DFLAGS            += $(DFLAGS-$(DEBUG))
LDFLAGS           += $(LDFLAGS-$(DEBUG))

BIT_ROOT_PREFIX   := 
BIT_BASE_PREFIX   := $(BIT_ROOT_PREFIX)/usr/local
BIT_DATA_PREFIX   := $(BIT_ROOT_PREFIX)/
BIT_STATE_PREFIX  := $(BIT_ROOT_PREFIX)/var
BIT_APP_PREFIX    := $(BIT_BASE_PREFIX)/lib/$(PRODUCT)
BIT_VAPP_PREFIX   := $(BIT_APP_PREFIX)/$(VERSION)
BIT_BIN_PREFIX    := $(BIT_ROOT_PREFIX)/usr/local/bin
BIT_INC_PREFIX    := $(BIT_ROOT_PREFIX)/usr/local/include
BIT_LIB_PREFIX    := $(BIT_ROOT_PREFIX)/usr/local/lib
BIT_MAN_PREFIX    := $(BIT_ROOT_PREFIX)/usr/local/share/man
BIT_SBIN_PREFIX   := $(BIT_ROOT_PREFIX)/usr/local/sbin
BIT_ETC_PREFIX    := $(BIT_ROOT_PREFIX)/etc/$(PRODUCT)
BIT_WEB_PREFIX    := $(BIT_ROOT_PREFIX)/var/www/$(PRODUCT)-default
BIT_LOG_PREFIX    := $(BIT_ROOT_PREFIX)/var/log/$(PRODUCT)
BIT_SPOOL_PREFIX  := $(BIT_ROOT_PREFIX)/var/spool/$(PRODUCT)
BIT_CACHE_PREFIX  := $(BIT_ROOT_PREFIX)/var/spool/$(PRODUCT)/cache
BIT_SRC_PREFIX    := $(BIT_ROOT_PREFIX)$(PRODUCT)-$(VERSION)


TARGETS           += $(CONFIG)/bin/libmpr.so
TARGETS           += $(CONFIG)/bin/libmprssl.so
TARGETS           += $(CONFIG)/bin/ejsman
TARGETS           += $(CONFIG)/bin/makerom
TARGETS           += $(CONFIG)/bin/ca.crt
TARGETS           += $(CONFIG)/bin/libpcre.so
TARGETS           += $(CONFIG)/bin/libhttp.so
TARGETS           += $(CONFIG)/bin/http
TARGETS           += $(CONFIG)/bin/libsqlite3.so
TARGETS           += $(CONFIG)/bin/sqlite
TARGETS           += $(CONFIG)/bin/libzlib.so
TARGETS           += $(CONFIG)/bin/libejs.so
TARGETS           += $(CONFIG)/bin/ejs
TARGETS           += $(CONFIG)/bin/ejsc
TARGETS           += $(CONFIG)/bin/ejsmod
TARGETS           += $(CONFIG)/bin/ejsrun
TARGETS           += $(CONFIG)/bin/ejs.mod
TARGETS           += $(CONFIG)/bin/ejs.unix.mod
TARGETS           += $(CONFIG)/bin/jem.es
TARGETS           += $(CONFIG)/bin/jem
TARGETS           += $(CONFIG)/bin/ejs.db.mod
TARGETS           += $(CONFIG)/bin/ejs.db.mapper.mod
TARGETS           += $(CONFIG)/bin/ejs.db.sqlite.mod
TARGETS           += $(CONFIG)/bin/libejs.db.sqlite.so
TARGETS           += $(CONFIG)/bin/ejs.mail.mod
TARGETS           += $(CONFIG)/bin/ejs.web.mod
TARGETS           += $(CONFIG)/bin/libejs.web.so
TARGETS           += $(CONFIG)/bin/www
TARGETS           += $(CONFIG)/bin/ejs.template.mod
TARGETS           += $(CONFIG)/bin/ejs.zlib.mod
TARGETS           += $(CONFIG)/bin/libejs.zlib.so
TARGETS           += $(CONFIG)/bin/ejs.tar.mod
TARGETS           += $(CONFIG)/bin/mvc.es
TARGETS           += $(CONFIG)/bin/mvc
TARGETS           += $(CONFIG)/bin/ejs.mvc.mod
TARGETS           += $(CONFIG)/bin/utest.es
TARGETS           += $(CONFIG)/bin/utest.worker
TARGETS           += $(CONFIG)/bin/utest

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
	@[ ! -f $(CONFIG)/inc/bit.h ] && cp projects/ejs-linux-default-bit.h $(CONFIG)/inc/bit.h ; true
	@[ ! -f $(CONFIG)/inc/bitos.h ] && cp src/bitos.h $(CONFIG)/inc/bitos.h ; true
	@if ! diff $(CONFIG)/inc/bit.h projects/ejs-linux-default-bit.h >/dev/null ; then\
		cp projects/ejs-linux-default-bit.h $(CONFIG)/inc/bit.h  ; \
	fi; true

clean:
	rm -f "$(CONFIG)/bin/libmpr.so"
	rm -f "$(CONFIG)/bin/libmprssl.so"
	rm -f "$(CONFIG)/bin/ejsman"
	rm -f "$(CONFIG)/bin/makerom"
	rm -f "$(CONFIG)/bin/libest.so"
	rm -f "$(CONFIG)/bin/ca.crt"
	rm -f "$(CONFIG)/bin/libpcre.so"
	rm -f "$(CONFIG)/bin/libhttp.so"
	rm -f "$(CONFIG)/bin/http"
	rm -f "$(CONFIG)/bin/libsqlite3.so"
	rm -f "$(CONFIG)/bin/sqlite"
	rm -f "$(CONFIG)/bin/libzlib.so"
	rm -f "$(CONFIG)/bin/libejs.so"
	rm -f "$(CONFIG)/bin/ejs"
	rm -f "$(CONFIG)/bin/ejsc"
	rm -f "$(CONFIG)/bin/ejsmod"
	rm -f "$(CONFIG)/bin/ejsrun"
	rm -f "$(CONFIG)/bin/jem"
	rm -f "$(CONFIG)/bin/libejs.db.sqlite.so"
	rm -f "$(CONFIG)/bin/libejs.web.so"
	rm -f "$(CONFIG)/bin/libejs.zlib.so"
	rm -f "$(CONFIG)/bin/utest"
	rm -f "$(CONFIG)/obj/mprLib.o"
	rm -f "$(CONFIG)/obj/mprSsl.o"
	rm -f "$(CONFIG)/obj/manager.o"
	rm -f "$(CONFIG)/obj/makerom.o"
	rm -f "$(CONFIG)/obj/estLib.o"
	rm -f "$(CONFIG)/obj/pcre.o"
	rm -f "$(CONFIG)/obj/httpLib.o"
	rm -f "$(CONFIG)/obj/http.o"
	rm -f "$(CONFIG)/obj/sqlite3.o"
	rm -f "$(CONFIG)/obj/sqlite.o"
	rm -f "$(CONFIG)/obj/zlib.o"
	rm -f "$(CONFIG)/obj/ecAst.o"
	rm -f "$(CONFIG)/obj/ecCodeGen.o"
	rm -f "$(CONFIG)/obj/ecCompiler.o"
	rm -f "$(CONFIG)/obj/ecLex.o"
	rm -f "$(CONFIG)/obj/ecModuleWrite.o"
	rm -f "$(CONFIG)/obj/ecParser.o"
	rm -f "$(CONFIG)/obj/ecState.o"
	rm -f "$(CONFIG)/obj/dtoa.o"
	rm -f "$(CONFIG)/obj/ejsApp.o"
	rm -f "$(CONFIG)/obj/ejsArray.o"
	rm -f "$(CONFIG)/obj/ejsBlock.o"
	rm -f "$(CONFIG)/obj/ejsBoolean.o"
	rm -f "$(CONFIG)/obj/ejsByteArray.o"
	rm -f "$(CONFIG)/obj/ejsCache.o"
	rm -f "$(CONFIG)/obj/ejsCmd.o"
	rm -f "$(CONFIG)/obj/ejsConfig.o"
	rm -f "$(CONFIG)/obj/ejsDate.o"
	rm -f "$(CONFIG)/obj/ejsDebug.o"
	rm -f "$(CONFIG)/obj/ejsError.o"
	rm -f "$(CONFIG)/obj/ejsFile.o"
	rm -f "$(CONFIG)/obj/ejsFileSystem.o"
	rm -f "$(CONFIG)/obj/ejsFrame.o"
	rm -f "$(CONFIG)/obj/ejsFunction.o"
	rm -f "$(CONFIG)/obj/ejsGC.o"
	rm -f "$(CONFIG)/obj/ejsGlobal.o"
	rm -f "$(CONFIG)/obj/ejsHttp.o"
	rm -f "$(CONFIG)/obj/ejsIterator.o"
	rm -f "$(CONFIG)/obj/ejsJSON.o"
	rm -f "$(CONFIG)/obj/ejsLocalCache.o"
	rm -f "$(CONFIG)/obj/ejsMath.o"
	rm -f "$(CONFIG)/obj/ejsMemory.o"
	rm -f "$(CONFIG)/obj/ejsMprLog.o"
	rm -f "$(CONFIG)/obj/ejsNamespace.o"
	rm -f "$(CONFIG)/obj/ejsNull.o"
	rm -f "$(CONFIG)/obj/ejsNumber.o"
	rm -f "$(CONFIG)/obj/ejsObject.o"
	rm -f "$(CONFIG)/obj/ejsPath.o"
	rm -f "$(CONFIG)/obj/ejsPot.o"
	rm -f "$(CONFIG)/obj/ejsRegExp.o"
	rm -f "$(CONFIG)/obj/ejsSocket.o"
	rm -f "$(CONFIG)/obj/ejsString.o"
	rm -f "$(CONFIG)/obj/ejsSystem.o"
	rm -f "$(CONFIG)/obj/ejsTimer.o"
	rm -f "$(CONFIG)/obj/ejsType.o"
	rm -f "$(CONFIG)/obj/ejsUri.o"
	rm -f "$(CONFIG)/obj/ejsVoid.o"
	rm -f "$(CONFIG)/obj/ejsWebSocket.o"
	rm -f "$(CONFIG)/obj/ejsWorker.o"
	rm -f "$(CONFIG)/obj/ejsXML.o"
	rm -f "$(CONFIG)/obj/ejsXMLList.o"
	rm -f "$(CONFIG)/obj/ejsXMLLoader.o"
	rm -f "$(CONFIG)/obj/ejsByteCode.o"
	rm -f "$(CONFIG)/obj/ejsException.o"
	rm -f "$(CONFIG)/obj/ejsHelper.o"
	rm -f "$(CONFIG)/obj/ejsInterp.o"
	rm -f "$(CONFIG)/obj/ejsLoader.o"
	rm -f "$(CONFIG)/obj/ejsModule.o"
	rm -f "$(CONFIG)/obj/ejsScope.o"
	rm -f "$(CONFIG)/obj/ejsService.o"
	rm -f "$(CONFIG)/obj/ejs.o"
	rm -f "$(CONFIG)/obj/ejsc.o"
	rm -f "$(CONFIG)/obj/ejsmod.o"
	rm -f "$(CONFIG)/obj/doc.o"
	rm -f "$(CONFIG)/obj/docFiles.o"
	rm -f "$(CONFIG)/obj/listing.o"
	rm -f "$(CONFIG)/obj/slotGen.o"
	rm -f "$(CONFIG)/obj/ejsrun.o"
	rm -f "$(CONFIG)/obj/ejsSqlite.o"
	rm -f "$(CONFIG)/obj/ejsHttpServer.o"
	rm -f "$(CONFIG)/obj/ejsRequest.o"
	rm -f "$(CONFIG)/obj/ejsSession.o"
	rm -f "$(CONFIG)/obj/ejsWeb.o"
	rm -f "$(CONFIG)/obj/ejsZlib.o"

clobber: clean
	rm -fr ./$(CONFIG)



#
#   version
#
version: $(DEPS_1)
	@echo NN 2.3.1-2

#
#   mpr.h
#
$(CONFIG)/inc/mpr.h: $(DEPS_2)
	@echo '      [Copy] $(CONFIG)/inc/mpr.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/mpr/mpr.h" "$(CONFIG)/inc/mpr.h"

#
#   bit.h
#
$(CONFIG)/inc/bit.h: $(DEPS_3)
	@echo '      [Copy] $(CONFIG)/inc/bit.h'

#
#   bitos.h
#
$(CONFIG)/inc/bitos.h: $(DEPS_4)
	@echo '      [Copy] $(CONFIG)/inc/bitos.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/bitos.h" "$(CONFIG)/inc/bitos.h"

#
#   mprLib.o
#
DEPS_5 += $(CONFIG)/inc/bit.h
DEPS_5 += $(CONFIG)/inc/mpr.h
DEPS_5 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/mprLib.o: \
    src/deps/mpr/mprLib.c $(DEPS_5)
	@echo '   [Compile] src/deps/mpr/mprLib.c'
	$(CC) -c -o $(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprLib.c

#
#   libmpr
#
DEPS_6 += $(CONFIG)/inc/mpr.h
DEPS_6 += $(CONFIG)/obj/mprLib.o

$(CONFIG)/bin/libmpr.so: $(DEPS_6)
	@echo '      [Link] libmpr'
	$(CC) -shared -o $(CONFIG)/bin/libmpr.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/mprLib.o $(LIBS)

#
#   est.h
#
$(CONFIG)/inc/est.h: $(DEPS_7)
	@echo '      [Copy] $(CONFIG)/inc/est.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/est/est.h" "$(CONFIG)/inc/est.h"

#
#   estLib.o
#
DEPS_8 += $(CONFIG)/inc/bit.h
DEPS_8 += $(CONFIG)/inc/est.h
DEPS_8 += $(CONFIG)/inc/bitos.h

$(CONFIG)/obj/estLib.o: \
    src/deps/est/estLib.c $(DEPS_8)
	@echo '   [Compile] src/deps/est/estLib.c'
	$(CC) -c -o $(CONFIG)/obj/estLib.o -fPIC $(DFLAGS) $(IFLAGS) src/deps/est/estLib.c

#
#   libest
#
DEPS_9 += $(CONFIG)/inc/est.h
DEPS_9 += $(CONFIG)/obj/estLib.o

$(CONFIG)/bin/libest.so: $(DEPS_9)
	@echo '      [Link] libest'
	$(CC) -shared -o $(CONFIG)/bin/libest.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/estLib.o $(LIBS)

#
#   mprSsl.o
#
DEPS_10 += $(CONFIG)/inc/bit.h
DEPS_10 += $(CONFIG)/inc/mpr.h
DEPS_10 += $(CONFIG)/inc/est.h

$(CONFIG)/obj/mprSsl.o: \
    src/deps/mpr/mprSsl.c $(DEPS_10)
	@echo '   [Compile] src/deps/mpr/mprSsl.c'
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/mprSsl.c

#
#   libmprssl
#
DEPS_11 += $(CONFIG)/bin/libmpr.so
DEPS_11 += $(CONFIG)/bin/libest.so
DEPS_11 += $(CONFIG)/obj/mprSsl.o

LIBS_11 += -lest
LIBS_11 += -lmpr

$(CONFIG)/bin/libmprssl.so: $(DEPS_11)
	@echo '      [Link] libmprssl'
	$(CC) -shared -o $(CONFIG)/bin/libmprssl.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/mprSsl.o $(LIBS_11) $(LIBS_11) $(LIBS)

#
#   manager.o
#
DEPS_12 += $(CONFIG)/inc/bit.h
DEPS_12 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/manager.o: \
    src/deps/mpr/manager.c $(DEPS_12)
	@echo '   [Compile] src/deps/mpr/manager.c'
	$(CC) -c -o $(CONFIG)/obj/manager.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/manager.c

#
#   manager
#
DEPS_13 += $(CONFIG)/bin/libmpr.so
DEPS_13 += $(CONFIG)/obj/manager.o

LIBS_13 += -lmpr

$(CONFIG)/bin/ejsman: $(DEPS_13)
	@echo '      [Link] manager'
	$(CC) -o $(CONFIG)/bin/ejsman $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/manager.o $(LIBS_13) $(LIBS_13) $(LIBS) $(LDFLAGS)

#
#   makerom.o
#
DEPS_14 += $(CONFIG)/inc/bit.h
DEPS_14 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/makerom.o: \
    src/deps/mpr/makerom.c $(DEPS_14)
	@echo '   [Compile] src/deps/mpr/makerom.c'
	$(CC) -c -o $(CONFIG)/obj/makerom.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/mpr/makerom.c

#
#   makerom
#
DEPS_15 += $(CONFIG)/bin/libmpr.so
DEPS_15 += $(CONFIG)/obj/makerom.o

LIBS_15 += -lmpr

$(CONFIG)/bin/makerom: $(DEPS_15)
	@echo '      [Link] makerom'
	$(CC) -o $(CONFIG)/bin/makerom $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o $(LIBS_15) $(LIBS_15) $(LIBS) $(LDFLAGS)

#
#   ca-crt
#
DEPS_16 += src/deps/est/ca.crt

$(CONFIG)/bin/ca.crt: $(DEPS_16)
	@echo '      [Copy] $(CONFIG)/bin/ca.crt'
	mkdir -p "$(CONFIG)/bin"
	cp "src/deps/est/ca.crt" "$(CONFIG)/bin/ca.crt"

#
#   pcre.h
#
$(CONFIG)/inc/pcre.h: $(DEPS_17)
	@echo '      [Copy] $(CONFIG)/inc/pcre.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/pcre/pcre.h" "$(CONFIG)/inc/pcre.h"

#
#   pcre.o
#
DEPS_18 += $(CONFIG)/inc/bit.h
DEPS_18 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
    src/deps/pcre/pcre.c $(DEPS_18)
	@echo '   [Compile] src/deps/pcre/pcre.c'
	$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/pcre/pcre.c

#
#   libpcre
#
DEPS_19 += $(CONFIG)/inc/pcre.h
DEPS_19 += $(CONFIG)/obj/pcre.o

$(CONFIG)/bin/libpcre.so: $(DEPS_19)
	@echo '      [Link] libpcre'
	$(CC) -shared -o $(CONFIG)/bin/libpcre.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/pcre.o $(LIBS)

#
#   http.h
#
$(CONFIG)/inc/http.h: $(DEPS_20)
	@echo '      [Copy] $(CONFIG)/inc/http.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/http/http.h" "$(CONFIG)/inc/http.h"

#
#   httpLib.o
#
DEPS_21 += $(CONFIG)/inc/bit.h
DEPS_21 += $(CONFIG)/inc/http.h
DEPS_21 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/httpLib.o: \
    src/deps/http/httpLib.c $(DEPS_21)
	@echo '   [Compile] src/deps/http/httpLib.c'
	$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/httpLib.c

#
#   libhttp
#
DEPS_22 += $(CONFIG)/bin/libmpr.so
DEPS_22 += $(CONFIG)/bin/libpcre.so
DEPS_22 += $(CONFIG)/inc/http.h
DEPS_22 += $(CONFIG)/obj/httpLib.o

LIBS_22 += -lpcre
LIBS_22 += -lmpr

$(CONFIG)/bin/libhttp.so: $(DEPS_22)
	@echo '      [Link] libhttp'
	$(CC) -shared -o $(CONFIG)/bin/libhttp.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/httpLib.o $(LIBS_22) $(LIBS_22) $(LIBS)

#
#   http.o
#
DEPS_23 += $(CONFIG)/inc/bit.h
DEPS_23 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/http.o: \
    src/deps/http/http.c $(DEPS_23)
	@echo '   [Compile] src/deps/http/http.c'
	$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/http/http.c

#
#   http
#
DEPS_24 += $(CONFIG)/bin/libhttp.so
DEPS_24 += $(CONFIG)/obj/http.o

LIBS_24 += -lhttp
LIBS_24 += -lpcre
LIBS_24 += -lmpr

$(CONFIG)/bin/http: $(DEPS_24)
	@echo '      [Link] http'
	$(CC) -o $(CONFIG)/bin/http $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o $(LIBS_24) $(LIBS_24) $(LIBS) -lmpr $(LDFLAGS)

#
#   sqlite3.h
#
$(CONFIG)/inc/sqlite3.h: $(DEPS_25)
	@echo '      [Copy] $(CONFIG)/inc/sqlite3.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/sqlite/sqlite3.h" "$(CONFIG)/inc/sqlite3.h"

#
#   sqlite3.o
#
DEPS_26 += $(CONFIG)/inc/bit.h
DEPS_26 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite3.o: \
    src/deps/sqlite/sqlite3.c $(DEPS_26)
	@echo '   [Compile] src/deps/sqlite/sqlite3.c'
	$(CC) -c -o $(CONFIG)/obj/sqlite3.o -fPIC $(DFLAGS) $(IFLAGS) src/deps/sqlite/sqlite3.c

#
#   libsqlite3
#
DEPS_27 += $(CONFIG)/inc/sqlite3.h
DEPS_27 += $(CONFIG)/obj/sqlite3.o

LIBS_27 += -lsqlite3

$(CONFIG)/bin/libsqlite3.so: $(DEPS_27)
	@echo '      [Link] libsqlite3'
	$(CC) -shared -o $(CONFIG)/bin/libsqlite3.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/sqlite3.o $(LIBS_27) $(LIBS_27) $(LIBS) -lsqlite3

#
#   sqlite.o
#
DEPS_28 += $(CONFIG)/inc/bit.h
DEPS_28 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite.o: \
    src/deps/sqlite/sqlite.c $(DEPS_28)
	@echo '   [Compile] src/deps/sqlite/sqlite.c'
	$(CC) -c -o $(CONFIG)/obj/sqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/sqlite/sqlite.c

#
#   sqlite
#
DEPS_29 += $(CONFIG)/bin/libsqlite3.so
DEPS_29 += $(CONFIG)/obj/sqlite.o

LIBS_29 += -lsqlite3

$(CONFIG)/bin/sqlite: $(DEPS_29)
	@echo '      [Link] sqlite'
	$(CC) -o $(CONFIG)/bin/sqlite $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/sqlite.o $(LIBS_29) $(LIBS_29) $(LIBS) $(LDFLAGS)

#
#   zlib.h
#
$(CONFIG)/inc/zlib.h: $(DEPS_30)
	@echo '      [Copy] $(CONFIG)/inc/zlib.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/deps/zlib/zlib.h" "$(CONFIG)/inc/zlib.h"

#
#   zlib.o
#
DEPS_31 += $(CONFIG)/inc/bit.h
DEPS_31 += $(CONFIG)/inc/zlib.h

$(CONFIG)/obj/zlib.o: \
    src/deps/zlib/zlib.c $(DEPS_31)
	@echo '   [Compile] src/deps/zlib/zlib.c'
	$(CC) -c -o $(CONFIG)/obj/zlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/deps/zlib/zlib.c

#
#   libzlib
#
DEPS_32 += $(CONFIG)/inc/zlib.h
DEPS_32 += $(CONFIG)/obj/zlib.o

$(CONFIG)/bin/libzlib.so: $(DEPS_32)
	@echo '      [Link] libzlib'
	$(CC) -shared -o $(CONFIG)/bin/libzlib.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/zlib.o $(LIBS)

#
#   ejs.cache.local.slots.h
#
$(CONFIG)/inc/ejs.cache.local.slots.h: $(DEPS_33)
	@echo '      [Copy] $(CONFIG)/inc/ejs.cache.local.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.cache.local.slots.h" "$(CONFIG)/inc/ejs.cache.local.slots.h"

#
#   ejs.db.sqlite.slots.h
#
$(CONFIG)/inc/ejs.db.sqlite.slots.h: $(DEPS_34)
	@echo '      [Copy] $(CONFIG)/inc/ejs.db.sqlite.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.db.sqlite.slots.h" "$(CONFIG)/inc/ejs.db.sqlite.slots.h"

#
#   ejs.slots.h
#
$(CONFIG)/inc/ejs.slots.h: $(DEPS_35)
	@echo '      [Copy] $(CONFIG)/inc/ejs.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.slots.h" "$(CONFIG)/inc/ejs.slots.h"

#
#   ejs.web.slots.h
#
$(CONFIG)/inc/ejs.web.slots.h: $(DEPS_36)
	@echo '      [Copy] $(CONFIG)/inc/ejs.web.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.web.slots.h" "$(CONFIG)/inc/ejs.web.slots.h"

#
#   ejs.zlib.slots.h
#
$(CONFIG)/inc/ejs.zlib.slots.h: $(DEPS_37)
	@echo '      [Copy] $(CONFIG)/inc/ejs.zlib.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/slots/ejs.zlib.slots.h" "$(CONFIG)/inc/ejs.zlib.slots.h"

#
#   ejsByteCode.h
#
$(CONFIG)/inc/ejsByteCode.h: $(DEPS_38)
	@echo '      [Copy] $(CONFIG)/inc/ejsByteCode.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsByteCode.h" "$(CONFIG)/inc/ejsByteCode.h"

#
#   ejsByteCodeTable.h
#
$(CONFIG)/inc/ejsByteCodeTable.h: $(DEPS_39)
	@echo '      [Copy] $(CONFIG)/inc/ejsByteCodeTable.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsByteCodeTable.h" "$(CONFIG)/inc/ejsByteCodeTable.h"

#
#   ejsCustomize.h
#
$(CONFIG)/inc/ejsCustomize.h: $(DEPS_40)
	@echo '      [Copy] $(CONFIG)/inc/ejsCustomize.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsCustomize.h" "$(CONFIG)/inc/ejsCustomize.h"

#
#   ejs.h
#
DEPS_41 += $(CONFIG)/inc/mpr.h
DEPS_41 += $(CONFIG)/inc/http.h
DEPS_41 += $(CONFIG)/inc/ejsByteCode.h
DEPS_41 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_41 += $(CONFIG)/inc/ejs.slots.h
DEPS_41 += $(CONFIG)/inc/ejsCustomize.h

$(CONFIG)/inc/ejs.h: $(DEPS_41)
	@echo '      [Copy] $(CONFIG)/inc/ejs.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejs.h" "$(CONFIG)/inc/ejs.h"

#
#   ejsCompiler.h
#
$(CONFIG)/inc/ejsCompiler.h: $(DEPS_42)
	@echo '      [Copy] $(CONFIG)/inc/ejsCompiler.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/ejsCompiler.h" "$(CONFIG)/inc/ejsCompiler.h"

#
#   ecAst.o
#
DEPS_43 += $(CONFIG)/inc/bit.h
DEPS_43 += $(CONFIG)/inc/ejsCompiler.h
DEPS_43 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ecAst.o: \
    src/compiler/ecAst.c $(DEPS_43)
	@echo '   [Compile] src/compiler/ecAst.c'
	$(CC) -c -o $(CONFIG)/obj/ecAst.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecAst.c

#
#   ecCodeGen.o
#
DEPS_44 += $(CONFIG)/inc/bit.h
DEPS_44 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecCodeGen.o: \
    src/compiler/ecCodeGen.c $(DEPS_44)
	@echo '   [Compile] src/compiler/ecCodeGen.c'
	$(CC) -c -o $(CONFIG)/obj/ecCodeGen.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCodeGen.c

#
#   ecCompiler.o
#
DEPS_45 += $(CONFIG)/inc/bit.h
DEPS_45 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecCompiler.o: \
    src/compiler/ecCompiler.c $(DEPS_45)
	@echo '   [Compile] src/compiler/ecCompiler.c'
	$(CC) -c -o $(CONFIG)/obj/ecCompiler.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCompiler.c

#
#   ecLex.o
#
DEPS_46 += $(CONFIG)/inc/bit.h
DEPS_46 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecLex.o: \
    src/compiler/ecLex.c $(DEPS_46)
	@echo '   [Compile] src/compiler/ecLex.c'
	$(CC) -c -o $(CONFIG)/obj/ecLex.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecLex.c

#
#   ecModuleWrite.o
#
DEPS_47 += $(CONFIG)/inc/bit.h
DEPS_47 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecModuleWrite.o: \
    src/compiler/ecModuleWrite.c $(DEPS_47)
	@echo '   [Compile] src/compiler/ecModuleWrite.c'
	$(CC) -c -o $(CONFIG)/obj/ecModuleWrite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecModuleWrite.c

#
#   ecParser.o
#
DEPS_48 += $(CONFIG)/inc/bit.h
DEPS_48 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecParser.o: \
    src/compiler/ecParser.c $(DEPS_48)
	@echo '   [Compile] src/compiler/ecParser.c'
	$(CC) -c -o $(CONFIG)/obj/ecParser.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecParser.c

#
#   ecState.o
#
DEPS_49 += $(CONFIG)/inc/bit.h
DEPS_49 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecState.o: \
    src/compiler/ecState.c $(DEPS_49)
	@echo '   [Compile] src/compiler/ecState.c'
	$(CC) -c -o $(CONFIG)/obj/ecState.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecState.c

#
#   dtoa.o
#
DEPS_50 += $(CONFIG)/inc/bit.h
DEPS_50 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/dtoa.o: \
    src/core/src/dtoa.c $(DEPS_50)
	@echo '   [Compile] src/core/src/dtoa.c'
	$(CC) -c -o $(CONFIG)/obj/dtoa.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/dtoa.c

#
#   ejsApp.o
#
DEPS_51 += $(CONFIG)/inc/bit.h
DEPS_51 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsApp.o: \
    src/core/src/ejsApp.c $(DEPS_51)
	@echo '   [Compile] src/core/src/ejsApp.c'
	$(CC) -c -o $(CONFIG)/obj/ejsApp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsApp.c

#
#   ejsArray.o
#
DEPS_52 += $(CONFIG)/inc/bit.h
DEPS_52 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsArray.o: \
    src/core/src/ejsArray.c $(DEPS_52)
	@echo '   [Compile] src/core/src/ejsArray.c'
	$(CC) -c -o $(CONFIG)/obj/ejsArray.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsArray.c

#
#   ejsBlock.o
#
DEPS_53 += $(CONFIG)/inc/bit.h
DEPS_53 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsBlock.o: \
    src/core/src/ejsBlock.c $(DEPS_53)
	@echo '   [Compile] src/core/src/ejsBlock.c'
	$(CC) -c -o $(CONFIG)/obj/ejsBlock.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBlock.c

#
#   ejsBoolean.o
#
DEPS_54 += $(CONFIG)/inc/bit.h
DEPS_54 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsBoolean.o: \
    src/core/src/ejsBoolean.c $(DEPS_54)
	@echo '   [Compile] src/core/src/ejsBoolean.c'
	$(CC) -c -o $(CONFIG)/obj/ejsBoolean.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBoolean.c

#
#   ejsByteArray.o
#
DEPS_55 += $(CONFIG)/inc/bit.h
DEPS_55 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsByteArray.o: \
    src/core/src/ejsByteArray.c $(DEPS_55)
	@echo '   [Compile] src/core/src/ejsByteArray.c'
	$(CC) -c -o $(CONFIG)/obj/ejsByteArray.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsByteArray.c

#
#   ejsCache.o
#
DEPS_56 += $(CONFIG)/inc/bit.h
DEPS_56 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsCache.o: \
    src/core/src/ejsCache.c $(DEPS_56)
	@echo '   [Compile] src/core/src/ejsCache.c'
	$(CC) -c -o $(CONFIG)/obj/ejsCache.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCache.c

#
#   ejsCmd.o
#
DEPS_57 += $(CONFIG)/inc/bit.h
DEPS_57 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsCmd.o: \
    src/core/src/ejsCmd.c $(DEPS_57)
	@echo '   [Compile] src/core/src/ejsCmd.c'
	$(CC) -c -o $(CONFIG)/obj/ejsCmd.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCmd.c

#
#   ejsConfig.o
#
DEPS_58 += $(CONFIG)/inc/bit.h
DEPS_58 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsConfig.o: \
    src/core/src/ejsConfig.c $(DEPS_58)
	@echo '   [Compile] src/core/src/ejsConfig.c'
	$(CC) -c -o $(CONFIG)/obj/ejsConfig.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsConfig.c

#
#   ejsDate.o
#
DEPS_59 += $(CONFIG)/inc/bit.h
DEPS_59 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsDate.o: \
    src/core/src/ejsDate.c $(DEPS_59)
	@echo '   [Compile] src/core/src/ejsDate.c'
	$(CC) -c -o $(CONFIG)/obj/ejsDate.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDate.c

#
#   ejsDebug.o
#
DEPS_60 += $(CONFIG)/inc/bit.h
DEPS_60 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsDebug.o: \
    src/core/src/ejsDebug.c $(DEPS_60)
	@echo '   [Compile] src/core/src/ejsDebug.c'
	$(CC) -c -o $(CONFIG)/obj/ejsDebug.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDebug.c

#
#   ejsError.o
#
DEPS_61 += $(CONFIG)/inc/bit.h
DEPS_61 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsError.o: \
    src/core/src/ejsError.c $(DEPS_61)
	@echo '   [Compile] src/core/src/ejsError.c'
	$(CC) -c -o $(CONFIG)/obj/ejsError.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsError.c

#
#   ejsFile.o
#
DEPS_62 += $(CONFIG)/inc/bit.h
DEPS_62 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFile.o: \
    src/core/src/ejsFile.c $(DEPS_62)
	@echo '   [Compile] src/core/src/ejsFile.c'
	$(CC) -c -o $(CONFIG)/obj/ejsFile.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFile.c

#
#   ejsFileSystem.o
#
DEPS_63 += $(CONFIG)/inc/bit.h
DEPS_63 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFileSystem.o: \
    src/core/src/ejsFileSystem.c $(DEPS_63)
	@echo '   [Compile] src/core/src/ejsFileSystem.c'
	$(CC) -c -o $(CONFIG)/obj/ejsFileSystem.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFileSystem.c

#
#   ejsFrame.o
#
DEPS_64 += $(CONFIG)/inc/bit.h
DEPS_64 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFrame.o: \
    src/core/src/ejsFrame.c $(DEPS_64)
	@echo '   [Compile] src/core/src/ejsFrame.c'
	$(CC) -c -o $(CONFIG)/obj/ejsFrame.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFrame.c

#
#   ejsFunction.o
#
DEPS_65 += $(CONFIG)/inc/bit.h
DEPS_65 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFunction.o: \
    src/core/src/ejsFunction.c $(DEPS_65)
	@echo '   [Compile] src/core/src/ejsFunction.c'
	$(CC) -c -o $(CONFIG)/obj/ejsFunction.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFunction.c

#
#   ejsGC.o
#
DEPS_66 += $(CONFIG)/inc/bit.h
DEPS_66 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsGC.o: \
    src/core/src/ejsGC.c $(DEPS_66)
	@echo '   [Compile] src/core/src/ejsGC.c'
	$(CC) -c -o $(CONFIG)/obj/ejsGC.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGC.c

#
#   ejsGlobal.o
#
DEPS_67 += $(CONFIG)/inc/bit.h
DEPS_67 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsGlobal.o: \
    src/core/src/ejsGlobal.c $(DEPS_67)
	@echo '   [Compile] src/core/src/ejsGlobal.c'
	$(CC) -c -o $(CONFIG)/obj/ejsGlobal.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGlobal.c

#
#   ejsHttp.o
#
DEPS_68 += $(CONFIG)/inc/bit.h
DEPS_68 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsHttp.o: \
    src/core/src/ejsHttp.c $(DEPS_68)
	@echo '   [Compile] src/core/src/ejsHttp.c'
	$(CC) -c -o $(CONFIG)/obj/ejsHttp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsHttp.c

#
#   ejsIterator.o
#
DEPS_69 += $(CONFIG)/inc/bit.h
DEPS_69 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsIterator.o: \
    src/core/src/ejsIterator.c $(DEPS_69)
	@echo '   [Compile] src/core/src/ejsIterator.c'
	$(CC) -c -o $(CONFIG)/obj/ejsIterator.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsIterator.c

#
#   ejsJSON.o
#
DEPS_70 += $(CONFIG)/inc/bit.h
DEPS_70 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsJSON.o: \
    src/core/src/ejsJSON.c $(DEPS_70)
	@echo '   [Compile] src/core/src/ejsJSON.c'
	$(CC) -c -o $(CONFIG)/obj/ejsJSON.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsJSON.c

#
#   ejsLocalCache.o
#
DEPS_71 += $(CONFIG)/inc/bit.h
DEPS_71 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsLocalCache.o: \
    src/core/src/ejsLocalCache.c $(DEPS_71)
	@echo '   [Compile] src/core/src/ejsLocalCache.c'
	$(CC) -c -o $(CONFIG)/obj/ejsLocalCache.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsLocalCache.c

#
#   ejsMath.o
#
DEPS_72 += $(CONFIG)/inc/bit.h
DEPS_72 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMath.o: \
    src/core/src/ejsMath.c $(DEPS_72)
	@echo '   [Compile] src/core/src/ejsMath.c'
	$(CC) -c -o $(CONFIG)/obj/ejsMath.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMath.c

#
#   ejsMemory.o
#
DEPS_73 += $(CONFIG)/inc/bit.h
DEPS_73 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMemory.o: \
    src/core/src/ejsMemory.c $(DEPS_73)
	@echo '   [Compile] src/core/src/ejsMemory.c'
	$(CC) -c -o $(CONFIG)/obj/ejsMemory.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMemory.c

#
#   ejsMprLog.o
#
DEPS_74 += $(CONFIG)/inc/bit.h
DEPS_74 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMprLog.o: \
    src/core/src/ejsMprLog.c $(DEPS_74)
	@echo '   [Compile] src/core/src/ejsMprLog.c'
	$(CC) -c -o $(CONFIG)/obj/ejsMprLog.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMprLog.c

#
#   ejsNamespace.o
#
DEPS_75 += $(CONFIG)/inc/bit.h
DEPS_75 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNamespace.o: \
    src/core/src/ejsNamespace.c $(DEPS_75)
	@echo '   [Compile] src/core/src/ejsNamespace.c'
	$(CC) -c -o $(CONFIG)/obj/ejsNamespace.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNamespace.c

#
#   ejsNull.o
#
DEPS_76 += $(CONFIG)/inc/bit.h
DEPS_76 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNull.o: \
    src/core/src/ejsNull.c $(DEPS_76)
	@echo '   [Compile] src/core/src/ejsNull.c'
	$(CC) -c -o $(CONFIG)/obj/ejsNull.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNull.c

#
#   ejsNumber.o
#
DEPS_77 += $(CONFIG)/inc/bit.h
DEPS_77 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNumber.o: \
    src/core/src/ejsNumber.c $(DEPS_77)
	@echo '   [Compile] src/core/src/ejsNumber.c'
	$(CC) -c -o $(CONFIG)/obj/ejsNumber.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNumber.c

#
#   ejsObject.o
#
DEPS_78 += $(CONFIG)/inc/bit.h
DEPS_78 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsObject.o: \
    src/core/src/ejsObject.c $(DEPS_78)
	@echo '   [Compile] src/core/src/ejsObject.c'
	$(CC) -c -o $(CONFIG)/obj/ejsObject.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsObject.c

#
#   ejsPath.o
#
DEPS_79 += $(CONFIG)/inc/bit.h
DEPS_79 += $(CONFIG)/inc/ejs.h
DEPS_79 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsPath.o: \
    src/core/src/ejsPath.c $(DEPS_79)
	@echo '   [Compile] src/core/src/ejsPath.c'
	$(CC) -c -o $(CONFIG)/obj/ejsPath.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPath.c

#
#   ejsPot.o
#
DEPS_80 += $(CONFIG)/inc/bit.h
DEPS_80 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsPot.o: \
    src/core/src/ejsPot.c $(DEPS_80)
	@echo '   [Compile] src/core/src/ejsPot.c'
	$(CC) -c -o $(CONFIG)/obj/ejsPot.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPot.c

#
#   ejsRegExp.o
#
DEPS_81 += $(CONFIG)/inc/bit.h
DEPS_81 += $(CONFIG)/inc/ejs.h
DEPS_81 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsRegExp.o: \
    src/core/src/ejsRegExp.c $(DEPS_81)
	@echo '   [Compile] src/core/src/ejsRegExp.c'
	$(CC) -c -o $(CONFIG)/obj/ejsRegExp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsRegExp.c

#
#   ejsSocket.o
#
DEPS_82 += $(CONFIG)/inc/bit.h
DEPS_82 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsSocket.o: \
    src/core/src/ejsSocket.c $(DEPS_82)
	@echo '   [Compile] src/core/src/ejsSocket.c'
	$(CC) -c -o $(CONFIG)/obj/ejsSocket.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSocket.c

#
#   ejsString.o
#
DEPS_83 += $(CONFIG)/inc/bit.h
DEPS_83 += $(CONFIG)/inc/ejs.h
DEPS_83 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsString.o: \
    src/core/src/ejsString.c $(DEPS_83)
	@echo '   [Compile] src/core/src/ejsString.c'
	$(CC) -c -o $(CONFIG)/obj/ejsString.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsString.c

#
#   ejsSystem.o
#
DEPS_84 += $(CONFIG)/inc/bit.h
DEPS_84 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsSystem.o: \
    src/core/src/ejsSystem.c $(DEPS_84)
	@echo '   [Compile] src/core/src/ejsSystem.c'
	$(CC) -c -o $(CONFIG)/obj/ejsSystem.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSystem.c

#
#   ejsTimer.o
#
DEPS_85 += $(CONFIG)/inc/bit.h
DEPS_85 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsTimer.o: \
    src/core/src/ejsTimer.c $(DEPS_85)
	@echo '   [Compile] src/core/src/ejsTimer.c'
	$(CC) -c -o $(CONFIG)/obj/ejsTimer.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsTimer.c

#
#   ejsType.o
#
DEPS_86 += $(CONFIG)/inc/bit.h
DEPS_86 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsType.o: \
    src/core/src/ejsType.c $(DEPS_86)
	@echo '   [Compile] src/core/src/ejsType.c'
	$(CC) -c -o $(CONFIG)/obj/ejsType.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsType.c

#
#   ejsUri.o
#
DEPS_87 += $(CONFIG)/inc/bit.h
DEPS_87 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsUri.o: \
    src/core/src/ejsUri.c $(DEPS_87)
	@echo '   [Compile] src/core/src/ejsUri.c'
	$(CC) -c -o $(CONFIG)/obj/ejsUri.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsUri.c

#
#   ejsVoid.o
#
DEPS_88 += $(CONFIG)/inc/bit.h
DEPS_88 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsVoid.o: \
    src/core/src/ejsVoid.c $(DEPS_88)
	@echo '   [Compile] src/core/src/ejsVoid.c'
	$(CC) -c -o $(CONFIG)/obj/ejsVoid.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsVoid.c

#
#   ejsWebSocket.o
#
DEPS_89 += $(CONFIG)/inc/bit.h
DEPS_89 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsWebSocket.o: \
    src/core/src/ejsWebSocket.c $(DEPS_89)
	@echo '   [Compile] src/core/src/ejsWebSocket.c'
	$(CC) -c -o $(CONFIG)/obj/ejsWebSocket.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWebSocket.c

#
#   ejsWorker.o
#
DEPS_90 += $(CONFIG)/inc/bit.h
DEPS_90 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsWorker.o: \
    src/core/src/ejsWorker.c $(DEPS_90)
	@echo '   [Compile] src/core/src/ejsWorker.c'
	$(CC) -c -o $(CONFIG)/obj/ejsWorker.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWorker.c

#
#   ejsXML.o
#
DEPS_91 += $(CONFIG)/inc/bit.h
DEPS_91 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXML.o: \
    src/core/src/ejsXML.c $(DEPS_91)
	@echo '   [Compile] src/core/src/ejsXML.c'
	$(CC) -c -o $(CONFIG)/obj/ejsXML.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXML.c

#
#   ejsXMLList.o
#
DEPS_92 += $(CONFIG)/inc/bit.h
DEPS_92 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXMLList.o: \
    src/core/src/ejsXMLList.c $(DEPS_92)
	@echo '   [Compile] src/core/src/ejsXMLList.c'
	$(CC) -c -o $(CONFIG)/obj/ejsXMLList.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLList.c

#
#   ejsXMLLoader.o
#
DEPS_93 += $(CONFIG)/inc/bit.h
DEPS_93 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXMLLoader.o: \
    src/core/src/ejsXMLLoader.c $(DEPS_93)
	@echo '   [Compile] src/core/src/ejsXMLLoader.c'
	$(CC) -c -o $(CONFIG)/obj/ejsXMLLoader.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLLoader.c

#
#   ejsByteCode.o
#
DEPS_94 += $(CONFIG)/inc/bit.h
DEPS_94 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsByteCode.o: \
    src/vm/ejsByteCode.c $(DEPS_94)
	@echo '   [Compile] src/vm/ejsByteCode.c'
	$(CC) -c -o $(CONFIG)/obj/ejsByteCode.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsByteCode.c

#
#   ejsException.o
#
DEPS_95 += $(CONFIG)/inc/bit.h
DEPS_95 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsException.o: \
    src/vm/ejsException.c $(DEPS_95)
	@echo '   [Compile] src/vm/ejsException.c'
	$(CC) -c -o $(CONFIG)/obj/ejsException.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsException.c

#
#   ejsHelper.o
#
DEPS_96 += $(CONFIG)/inc/bit.h
DEPS_96 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsHelper.o: \
    src/vm/ejsHelper.c $(DEPS_96)
	@echo '   [Compile] src/vm/ejsHelper.c'
	$(CC) -c -o $(CONFIG)/obj/ejsHelper.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsHelper.c

#
#   ejsInterp.o
#
DEPS_97 += $(CONFIG)/inc/bit.h
DEPS_97 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsInterp.o: \
    src/vm/ejsInterp.c $(DEPS_97)
	@echo '   [Compile] src/vm/ejsInterp.c'
	$(CC) -c -o $(CONFIG)/obj/ejsInterp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsInterp.c

#
#   ejsLoader.o
#
DEPS_98 += $(CONFIG)/inc/bit.h
DEPS_98 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsLoader.o: \
    src/vm/ejsLoader.c $(DEPS_98)
	@echo '   [Compile] src/vm/ejsLoader.c'
	$(CC) -c -o $(CONFIG)/obj/ejsLoader.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsLoader.c

#
#   ejsModule.o
#
DEPS_99 += $(CONFIG)/inc/bit.h
DEPS_99 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsModule.o: \
    src/vm/ejsModule.c $(DEPS_99)
	@echo '   [Compile] src/vm/ejsModule.c'
	$(CC) -c -o $(CONFIG)/obj/ejsModule.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsModule.c

#
#   ejsScope.o
#
DEPS_100 += $(CONFIG)/inc/bit.h
DEPS_100 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsScope.o: \
    src/vm/ejsScope.c $(DEPS_100)
	@echo '   [Compile] src/vm/ejsScope.c'
	$(CC) -c -o $(CONFIG)/obj/ejsScope.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsScope.c

#
#   ejsService.o
#
DEPS_101 += $(CONFIG)/inc/bit.h
DEPS_101 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsService.o: \
    src/vm/ejsService.c $(DEPS_101)
	@echo '   [Compile] src/vm/ejsService.c'
	$(CC) -c -o $(CONFIG)/obj/ejsService.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsService.c

#
#   libejs
#
DEPS_102 += $(CONFIG)/bin/libhttp.so
DEPS_102 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_102 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_102 += $(CONFIG)/inc/ejs.slots.h
DEPS_102 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_102 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_102 += $(CONFIG)/inc/bitos.h
DEPS_102 += $(CONFIG)/inc/ejs.h
DEPS_102 += $(CONFIG)/inc/ejsByteCode.h
DEPS_102 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_102 += $(CONFIG)/inc/ejsCompiler.h
DEPS_102 += $(CONFIG)/inc/ejsCustomize.h
DEPS_102 += $(CONFIG)/obj/ecAst.o
DEPS_102 += $(CONFIG)/obj/ecCodeGen.o
DEPS_102 += $(CONFIG)/obj/ecCompiler.o
DEPS_102 += $(CONFIG)/obj/ecLex.o
DEPS_102 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_102 += $(CONFIG)/obj/ecParser.o
DEPS_102 += $(CONFIG)/obj/ecState.o
DEPS_102 += $(CONFIG)/obj/dtoa.o
DEPS_102 += $(CONFIG)/obj/ejsApp.o
DEPS_102 += $(CONFIG)/obj/ejsArray.o
DEPS_102 += $(CONFIG)/obj/ejsBlock.o
DEPS_102 += $(CONFIG)/obj/ejsBoolean.o
DEPS_102 += $(CONFIG)/obj/ejsByteArray.o
DEPS_102 += $(CONFIG)/obj/ejsCache.o
DEPS_102 += $(CONFIG)/obj/ejsCmd.o
DEPS_102 += $(CONFIG)/obj/ejsConfig.o
DEPS_102 += $(CONFIG)/obj/ejsDate.o
DEPS_102 += $(CONFIG)/obj/ejsDebug.o
DEPS_102 += $(CONFIG)/obj/ejsError.o
DEPS_102 += $(CONFIG)/obj/ejsFile.o
DEPS_102 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_102 += $(CONFIG)/obj/ejsFrame.o
DEPS_102 += $(CONFIG)/obj/ejsFunction.o
DEPS_102 += $(CONFIG)/obj/ejsGC.o
DEPS_102 += $(CONFIG)/obj/ejsGlobal.o
DEPS_102 += $(CONFIG)/obj/ejsHttp.o
DEPS_102 += $(CONFIG)/obj/ejsIterator.o
DEPS_102 += $(CONFIG)/obj/ejsJSON.o
DEPS_102 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_102 += $(CONFIG)/obj/ejsMath.o
DEPS_102 += $(CONFIG)/obj/ejsMemory.o
DEPS_102 += $(CONFIG)/obj/ejsMprLog.o
DEPS_102 += $(CONFIG)/obj/ejsNamespace.o
DEPS_102 += $(CONFIG)/obj/ejsNull.o
DEPS_102 += $(CONFIG)/obj/ejsNumber.o
DEPS_102 += $(CONFIG)/obj/ejsObject.o
DEPS_102 += $(CONFIG)/obj/ejsPath.o
DEPS_102 += $(CONFIG)/obj/ejsPot.o
DEPS_102 += $(CONFIG)/obj/ejsRegExp.o
DEPS_102 += $(CONFIG)/obj/ejsSocket.o
DEPS_102 += $(CONFIG)/obj/ejsString.o
DEPS_102 += $(CONFIG)/obj/ejsSystem.o
DEPS_102 += $(CONFIG)/obj/ejsTimer.o
DEPS_102 += $(CONFIG)/obj/ejsType.o
DEPS_102 += $(CONFIG)/obj/ejsUri.o
DEPS_102 += $(CONFIG)/obj/ejsVoid.o
DEPS_102 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_102 += $(CONFIG)/obj/ejsWorker.o
DEPS_102 += $(CONFIG)/obj/ejsXML.o
DEPS_102 += $(CONFIG)/obj/ejsXMLList.o
DEPS_102 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_102 += $(CONFIG)/obj/ejsByteCode.o
DEPS_102 += $(CONFIG)/obj/ejsException.o
DEPS_102 += $(CONFIG)/obj/ejsHelper.o
DEPS_102 += $(CONFIG)/obj/ejsInterp.o
DEPS_102 += $(CONFIG)/obj/ejsLoader.o
DEPS_102 += $(CONFIG)/obj/ejsModule.o
DEPS_102 += $(CONFIG)/obj/ejsScope.o
DEPS_102 += $(CONFIG)/obj/ejsService.o

LIBS_102 += -lhttp
LIBS_102 += -lpcre
LIBS_102 += -lmpr

$(CONFIG)/bin/libejs.so: $(DEPS_102)
	@echo '      [Link] libejs'
	$(CC) -shared -o $(CONFIG)/bin/libejs.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ecAst.o $(CONFIG)/obj/ecCodeGen.o $(CONFIG)/obj/ecCompiler.o $(CONFIG)/obj/ecLex.o $(CONFIG)/obj/ecModuleWrite.o $(CONFIG)/obj/ecParser.o $(CONFIG)/obj/ecState.o $(CONFIG)/obj/dtoa.o $(CONFIG)/obj/ejsApp.o $(CONFIG)/obj/ejsArray.o $(CONFIG)/obj/ejsBlock.o $(CONFIG)/obj/ejsBoolean.o $(CONFIG)/obj/ejsByteArray.o $(CONFIG)/obj/ejsCache.o $(CONFIG)/obj/ejsCmd.o $(CONFIG)/obj/ejsConfig.o $(CONFIG)/obj/ejsDate.o $(CONFIG)/obj/ejsDebug.o $(CONFIG)/obj/ejsError.o $(CONFIG)/obj/ejsFile.o $(CONFIG)/obj/ejsFileSystem.o $(CONFIG)/obj/ejsFrame.o $(CONFIG)/obj/ejsFunction.o $(CONFIG)/obj/ejsGC.o $(CONFIG)/obj/ejsGlobal.o $(CONFIG)/obj/ejsHttp.o $(CONFIG)/obj/ejsIterator.o $(CONFIG)/obj/ejsJSON.o $(CONFIG)/obj/ejsLocalCache.o $(CONFIG)/obj/ejsMath.o $(CONFIG)/obj/ejsMemory.o $(CONFIG)/obj/ejsMprLog.o $(CONFIG)/obj/ejsNamespace.o $(CONFIG)/obj/ejsNull.o $(CONFIG)/obj/ejsNumber.o $(CONFIG)/obj/ejsObject.o $(CONFIG)/obj/ejsPath.o $(CONFIG)/obj/ejsPot.o $(CONFIG)/obj/ejsRegExp.o $(CONFIG)/obj/ejsSocket.o $(CONFIG)/obj/ejsString.o $(CONFIG)/obj/ejsSystem.o $(CONFIG)/obj/ejsTimer.o $(CONFIG)/obj/ejsType.o $(CONFIG)/obj/ejsUri.o $(CONFIG)/obj/ejsVoid.o $(CONFIG)/obj/ejsWebSocket.o $(CONFIG)/obj/ejsWorker.o $(CONFIG)/obj/ejsXML.o $(CONFIG)/obj/ejsXMLList.o $(CONFIG)/obj/ejsXMLLoader.o $(CONFIG)/obj/ejsByteCode.o $(CONFIG)/obj/ejsException.o $(CONFIG)/obj/ejsHelper.o $(CONFIG)/obj/ejsInterp.o $(CONFIG)/obj/ejsLoader.o $(CONFIG)/obj/ejsModule.o $(CONFIG)/obj/ejsScope.o $(CONFIG)/obj/ejsService.o $(LIBS_102) $(LIBS_102) $(LIBS) -lmpr

#
#   ejs.o
#
DEPS_103 += $(CONFIG)/inc/bit.h
DEPS_103 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejs.o: \
    src/cmd/ejs.c $(DEPS_103)
	@echo '   [Compile] src/cmd/ejs.c'
	$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejs.c

#
#   ejs
#
DEPS_104 += $(CONFIG)/bin/libejs.so
DEPS_104 += $(CONFIG)/obj/ejs.o

LIBS_104 += -lejs
LIBS_104 += -lhttp
LIBS_104 += -lpcre
LIBS_104 += -lmpr

$(CONFIG)/bin/ejs: $(DEPS_104)
	@echo '      [Link] ejs'
	$(CC) -o $(CONFIG)/bin/ejs $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejs.o $(LIBS_104) $(LIBS_104) $(LIBS) -lmpr $(LDFLAGS)

#
#   ejsc.o
#
DEPS_105 += $(CONFIG)/inc/bit.h
DEPS_105 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejsc.o: \
    src/cmd/ejsc.c $(DEPS_105)
	@echo '   [Compile] src/cmd/ejsc.c'
	$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsc.c

#
#   ejsc
#
DEPS_106 += $(CONFIG)/bin/libejs.so
DEPS_106 += $(CONFIG)/obj/ejsc.o

LIBS_106 += -lejs
LIBS_106 += -lhttp
LIBS_106 += -lpcre
LIBS_106 += -lmpr

$(CONFIG)/bin/ejsc: $(DEPS_106)
	@echo '      [Link] ejsc'
	$(CC) -o $(CONFIG)/bin/ejsc $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsc.o $(LIBS_106) $(LIBS_106) $(LIBS) -lmpr $(LDFLAGS)

#
#   ejsmod.h
#
src/cmd/ejsmod.h: $(DEPS_107)
	@echo '      [Copy] src/cmd/ejsmod.h'

#
#   ejsmod.o
#
DEPS_108 += $(CONFIG)/inc/bit.h
DEPS_108 += src/cmd/ejsmod.h
DEPS_108 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsmod.o: \
    src/cmd/ejsmod.c $(DEPS_108)
	@echo '   [Compile] src/cmd/ejsmod.c'
	$(CC) -c -o $(CONFIG)/obj/ejsmod.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/ejsmod.c

#
#   doc.o
#
DEPS_109 += $(CONFIG)/inc/bit.h
DEPS_109 += src/cmd/ejsmod.h

$(CONFIG)/obj/doc.o: \
    src/cmd/doc.c $(DEPS_109)
	@echo '   [Compile] src/cmd/doc.c'
	$(CC) -c -o $(CONFIG)/obj/doc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/doc.c

#
#   docFiles.o
#
DEPS_110 += $(CONFIG)/inc/bit.h
DEPS_110 += src/cmd/ejsmod.h

$(CONFIG)/obj/docFiles.o: \
    src/cmd/docFiles.c $(DEPS_110)
	@echo '   [Compile] src/cmd/docFiles.c'
	$(CC) -c -o $(CONFIG)/obj/docFiles.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/docFiles.c

#
#   listing.o
#
DEPS_111 += $(CONFIG)/inc/bit.h
DEPS_111 += src/cmd/ejsmod.h
DEPS_111 += $(CONFIG)/inc/ejsByteCodeTable.h

$(CONFIG)/obj/listing.o: \
    src/cmd/listing.c $(DEPS_111)
	@echo '   [Compile] src/cmd/listing.c'
	$(CC) -c -o $(CONFIG)/obj/listing.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/listing.c

#
#   slotGen.o
#
DEPS_112 += $(CONFIG)/inc/bit.h
DEPS_112 += src/cmd/ejsmod.h

$(CONFIG)/obj/slotGen.o: \
    src/cmd/slotGen.c $(DEPS_112)
	@echo '   [Compile] src/cmd/slotGen.c'
	$(CC) -c -o $(CONFIG)/obj/slotGen.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/cmd/slotGen.c

#
#   ejsmod
#
DEPS_113 += $(CONFIG)/bin/libejs.so
DEPS_113 += $(CONFIG)/obj/ejsmod.o
DEPS_113 += $(CONFIG)/obj/doc.o
DEPS_113 += $(CONFIG)/obj/docFiles.o
DEPS_113 += $(CONFIG)/obj/listing.o
DEPS_113 += $(CONFIG)/obj/slotGen.o

LIBS_113 += -lejs
LIBS_113 += -lhttp
LIBS_113 += -lpcre
LIBS_113 += -lmpr

$(CONFIG)/bin/ejsmod: $(DEPS_113)
	@echo '      [Link] ejsmod'
	$(CC) -o $(CONFIG)/bin/ejsmod $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsmod.o $(CONFIG)/obj/doc.o $(CONFIG)/obj/docFiles.o $(CONFIG)/obj/listing.o $(CONFIG)/obj/slotGen.o $(LIBS_113) $(LIBS_113) $(LIBS) -lmpr $(LDFLAGS)

#
#   ejsrun.o
#
DEPS_114 += $(CONFIG)/inc/bit.h
DEPS_114 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejsrun.o: \
    src/cmd/ejsrun.c $(DEPS_114)
	@echo '   [Compile] src/cmd/ejsrun.c'
	$(CC) -c -o $(CONFIG)/obj/ejsrun.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsrun.c

#
#   ejsrun
#
DEPS_115 += $(CONFIG)/bin/libejs.so
DEPS_115 += $(CONFIG)/obj/ejsrun.o

LIBS_115 += -lejs
LIBS_115 += -lhttp
LIBS_115 += -lpcre
LIBS_115 += -lmpr

$(CONFIG)/bin/ejsrun: $(DEPS_115)
	@echo '      [Link] ejsrun'
	$(CC) -o $(CONFIG)/bin/ejsrun $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LIBS_115) $(LIBS_115) $(LIBS) -lmpr $(LDFLAGS)

#
#   ejs.mod
#
DEPS_116 += src/core/App.es
DEPS_116 += src/core/Args.es
DEPS_116 += src/core/Array.es
DEPS_116 += src/core/BinaryStream.es
DEPS_116 += src/core/Block.es
DEPS_116 += src/core/Boolean.es
DEPS_116 += src/core/ByteArray.es
DEPS_116 += src/core/Cache.es
DEPS_116 += src/core/Cmd.es
DEPS_116 += src/core/Compat.es
DEPS_116 += src/core/Config.es
DEPS_116 += src/core/Date.es
DEPS_116 += src/core/Debug.es
DEPS_116 += src/core/Emitter.es
DEPS_116 += src/core/Error.es
DEPS_116 += src/core/File.es
DEPS_116 += src/core/FileSystem.es
DEPS_116 += src/core/Frame.es
DEPS_116 += src/core/Function.es
DEPS_116 += src/core/GC.es
DEPS_116 += src/core/Global.es
DEPS_116 += src/core/Http.es
DEPS_116 += src/core/Inflector.es
DEPS_116 += src/core/Iterator.es
DEPS_116 += src/core/JSON.es
DEPS_116 += src/core/Loader.es
DEPS_116 += src/core/LocalCache.es
DEPS_116 += src/core/Locale.es
DEPS_116 += src/core/Logger.es
DEPS_116 += src/core/Math.es
DEPS_116 += src/core/Memory.es
DEPS_116 += src/core/MprLog.es
DEPS_116 += src/core/Name.es
DEPS_116 += src/core/Namespace.es
DEPS_116 += src/core/Null.es
DEPS_116 += src/core/Number.es
DEPS_116 += src/core/Object.es
DEPS_116 += src/core/Path.es
DEPS_116 += src/core/Promise.es
DEPS_116 += src/core/RegExp.es
DEPS_116 += src/core/Socket.es
DEPS_116 += src/core/Stream.es
DEPS_116 += src/core/String.es
DEPS_116 += src/core/System.es
DEPS_116 += src/core/TextStream.es
DEPS_116 += src/core/Timer.es
DEPS_116 += src/core/Type.es
DEPS_116 += src/core/Uri.es
DEPS_116 += src/core/Void.es
DEPS_116 += src/core/WebSocket.es
DEPS_116 += src/core/Worker.es
DEPS_116 += src/core/XML.es
DEPS_116 += src/core/XMLHttp.es
DEPS_116 += src/core/XMLList.es
DEPS_116 += $(CONFIG)/bin/ejsc
DEPS_116 += $(CONFIG)/bin/ejsmod

$(CONFIG)/bin/ejs.mod: $(DEPS_116)
	cd src/core; ../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.mod  --optimize 9 --bind --require null *.es  ; cd ../..
	cd src/core; ../../$(CONFIG)/bin/ejsmod --require null --cslots ../../$(CONFIG)/bin/ejs.mod ; cd ../..
	cd src/core; if ! diff ejs.slots.h ../../$(CONFIG)/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ../../$(CONFIG)/inc; fi ; cd ../..
	cd src/core; rm -f ejs.slots.h ; cd ../..

#
#   ejs.unix.mod
#
DEPS_117 += src/jems/ejs.unix/Unix.es
DEPS_117 += $(CONFIG)/bin/ejsc
DEPS_117 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.unix.mod: $(DEPS_117)
	cd src/jems/ejs.unix; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.unix.mod  --optimize 9 Unix.es ; cd ../../..

#
#   jem.es
#
DEPS_118 += src/jems/ejs.jem/jem.es

$(CONFIG)/bin/jem.es: $(DEPS_118)
	cd src/jems/ejs.jem; cp jem.es ../../../$(CONFIG)/bin ; cd ../../..

#
#   jem
#
DEPS_119 += $(CONFIG)/bin/libejs.so
DEPS_119 += $(CONFIG)/bin/jem.es
DEPS_119 += $(CONFIG)/obj/ejsrun.o

LIBS_119 += -lejs
LIBS_119 += -lhttp
LIBS_119 += -lpcre
LIBS_119 += -lmpr

$(CONFIG)/bin/jem: $(DEPS_119)
	@echo '      [Link] jem'
	$(CC) -o $(CONFIG)/bin/jem $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LIBS_119) $(LIBS_119) $(LIBS) -lmpr $(LDFLAGS)

#
#   ejs.db.mod
#
DEPS_120 += src/jems/ejs.db/Database.es
DEPS_120 += src/jems/ejs.db/DatabaseConnector.es
DEPS_120 += $(CONFIG)/bin/ejsc
DEPS_120 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.db.mod: $(DEPS_120)
	cd src/jems/ejs.db; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.mod  --optimize 9 *.es ; cd ../../..

#
#   ejs.db.mapper.mod
#
DEPS_121 += src/jems/ejs.db.mapper/Record.es
DEPS_121 += $(CONFIG)/bin/ejsc
DEPS_121 += $(CONFIG)/bin/ejs.mod
DEPS_121 += $(CONFIG)/bin/ejs.db.mod

$(CONFIG)/bin/ejs.db.mapper.mod: $(DEPS_121)
	cd src/jems/ejs.db.mapper; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.mapper.mod  --optimize 9 *.es ; cd ../../..

#
#   ejs.db.sqlite.mod
#
DEPS_122 += src/jems/ejs.db.sqlite/Sqlite.es
DEPS_122 += $(CONFIG)/bin/ejsc
DEPS_122 += $(CONFIG)/bin/ejsmod
DEPS_122 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.db.sqlite.mod: $(DEPS_122)
	cd src/jems/ejs.db.sqlite; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.db.sqlite.mod  --optimize 9 *.es ; cd ../../..

#
#   ejsSqlite.o
#
DEPS_123 += $(CONFIG)/inc/bit.h
DEPS_123 += $(CONFIG)/inc/ejs.h
DEPS_123 += $(CONFIG)/inc/ejs.db.sqlite.slots.h

$(CONFIG)/obj/ejsSqlite.o: \
    src/jems/ejs.db.sqlite/ejsSqlite.c $(DEPS_123)
	@echo '   [Compile] src/jems/ejs.db.sqlite/ejsSqlite.c'
	$(CC) -c -o $(CONFIG)/obj/ejsSqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/jems/ejs.db.sqlite/ejsSqlite.c

#
#   libejs.db.sqlite
#
DEPS_124 += $(CONFIG)/bin/libmpr.so
DEPS_124 += $(CONFIG)/bin/libejs.so
DEPS_124 += $(CONFIG)/bin/ejs.mod
DEPS_124 += $(CONFIG)/bin/ejs.db.sqlite.mod
DEPS_124 += $(CONFIG)/bin/libsqlite3.so
DEPS_124 += $(CONFIG)/obj/ejsSqlite.o

LIBS_124 += -lsqlite3
LIBS_124 += -lejs
LIBS_124 += -lmpr
LIBS_124 += -lhttp
LIBS_124 += -lpcre

$(CONFIG)/bin/libejs.db.sqlite.so: $(DEPS_124)
	@echo '      [Link] libejs.db.sqlite'
	$(CC) -shared -o $(CONFIG)/bin/libejs.db.sqlite.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsSqlite.o $(LIBS_124) $(LIBS_124) $(LIBS) -lpcre

#
#   ejs.mail.mod
#
DEPS_125 += src/jems/ejs.mail/Mail.es
DEPS_125 += $(CONFIG)/bin/ejsc
DEPS_125 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.mail.mod: $(DEPS_125)
	cd src/jems/ejs.mail; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.mail.mod  --optimize 9 *.es ; cd ../../..

#
#   ejs.web.mod
#
DEPS_126 += src/jems/ejs.web/Cascade.es
DEPS_126 += src/jems/ejs.web/CommonLog.es
DEPS_126 += src/jems/ejs.web/ContentType.es
DEPS_126 += src/jems/ejs.web/Controller.es
DEPS_126 += src/jems/ejs.web/Dir.es
DEPS_126 += src/jems/ejs.web/Google.es
DEPS_126 += src/jems/ejs.web/Head.es
DEPS_126 += src/jems/ejs.web/Html.es
DEPS_126 += src/jems/ejs.web/HttpServer.es
DEPS_126 += src/jems/ejs.web/MethodOverride.es
DEPS_126 += src/jems/ejs.web/Middleware.es
DEPS_126 += src/jems/ejs.web/Mvc.es
DEPS_126 += src/jems/ejs.web/Request.es
DEPS_126 += src/jems/ejs.web/Router.es
DEPS_126 += src/jems/ejs.web/Script.es
DEPS_126 += src/jems/ejs.web/Session.es
DEPS_126 += src/jems/ejs.web/ShowExceptions.es
DEPS_126 += src/jems/ejs.web/Static.es
DEPS_126 += src/jems/ejs.web/Template.es
DEPS_126 += src/jems/ejs.web/UploadFile.es
DEPS_126 += src/jems/ejs.web/UrlMap.es
DEPS_126 += src/jems/ejs.web/Utils.es
DEPS_126 += src/jems/ejs.web/View.es
DEPS_126 += $(CONFIG)/bin/ejsc
DEPS_126 += $(CONFIG)/bin/ejsmod
DEPS_126 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.web.mod: $(DEPS_126)
	cd src/jems/ejs.web; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.web.mod  --optimize 9 *.es ; cd ../../..
	cd src/jems/ejs.web; ../../../$(CONFIG)/bin/ejsmod --cslots ../../../$(CONFIG)/bin/ejs.web.mod ; cd ../../..
	cd src/jems/ejs.web; if ! diff ejs.web.slots.h ../../../$(CONFIG)/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../../$(CONFIG)/inc; fi ; cd ../../..
	cd src/jems/ejs.web; rm -f ejs.web.slots.h ; cd ../../..

#
#   ejsWeb.h
#
$(CONFIG)/inc/ejsWeb.h: $(DEPS_127)
	@echo '      [Copy] $(CONFIG)/inc/ejsWeb.h'
	mkdir -p "$(CONFIG)/inc"
	cp "src/jems/ejs.web/ejsWeb.h" "$(CONFIG)/inc/ejsWeb.h"

#
#   ejsHttpServer.o
#
DEPS_128 += $(CONFIG)/inc/bit.h
DEPS_128 += $(CONFIG)/inc/ejs.h
DEPS_128 += $(CONFIG)/inc/ejsCompiler.h
DEPS_128 += $(CONFIG)/inc/ejsWeb.h
DEPS_128 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_128 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/ejsHttpServer.o: \
    src/jems/ejs.web/ejsHttpServer.c $(DEPS_128)
	@echo '   [Compile] src/jems/ejs.web/ejsHttpServer.c'
	$(CC) -c -o $(CONFIG)/obj/ejsHttpServer.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/jems/ejs.web/ejsHttpServer.c

#
#   ejsRequest.o
#
DEPS_129 += $(CONFIG)/inc/bit.h
DEPS_129 += $(CONFIG)/inc/ejs.h
DEPS_129 += $(CONFIG)/inc/ejsCompiler.h
DEPS_129 += $(CONFIG)/inc/ejsWeb.h
DEPS_129 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsRequest.o: \
    src/jems/ejs.web/ejsRequest.c $(DEPS_129)
	@echo '   [Compile] src/jems/ejs.web/ejsRequest.c'
	$(CC) -c -o $(CONFIG)/obj/ejsRequest.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/jems/ejs.web/ejsRequest.c

#
#   ejsSession.o
#
DEPS_130 += $(CONFIG)/inc/bit.h
DEPS_130 += $(CONFIG)/inc/ejs.h
DEPS_130 += $(CONFIG)/inc/ejsWeb.h

$(CONFIG)/obj/ejsSession.o: \
    src/jems/ejs.web/ejsSession.c $(DEPS_130)
	@echo '   [Compile] src/jems/ejs.web/ejsSession.c'
	$(CC) -c -o $(CONFIG)/obj/ejsSession.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/jems/ejs.web/ejsSession.c

#
#   ejsWeb.o
#
DEPS_131 += $(CONFIG)/inc/bit.h
DEPS_131 += $(CONFIG)/inc/ejs.h
DEPS_131 += $(CONFIG)/inc/ejsCompiler.h
DEPS_131 += $(CONFIG)/inc/ejsWeb.h
DEPS_131 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsWeb.o: \
    src/jems/ejs.web/ejsWeb.c $(DEPS_131)
	@echo '   [Compile] src/jems/ejs.web/ejsWeb.c'
	$(CC) -c -o $(CONFIG)/obj/ejsWeb.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/jems/ejs.web/ejsWeb.c

#
#   libejs.web
#
DEPS_132 += $(CONFIG)/bin/libejs.so
DEPS_132 += $(CONFIG)/bin/ejs.mod
DEPS_132 += $(CONFIG)/inc/ejsWeb.h
DEPS_132 += $(CONFIG)/obj/ejsHttpServer.o
DEPS_132 += $(CONFIG)/obj/ejsRequest.o
DEPS_132 += $(CONFIG)/obj/ejsSession.o
DEPS_132 += $(CONFIG)/obj/ejsWeb.o

LIBS_132 += -lejs
LIBS_132 += -lhttp
LIBS_132 += -lpcre
LIBS_132 += -lmpr

$(CONFIG)/bin/libejs.web.so: $(DEPS_132)
	@echo '      [Link] libejs.web'
	$(CC) -shared -o $(CONFIG)/bin/libejs.web.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsHttpServer.o $(CONFIG)/obj/ejsRequest.o $(CONFIG)/obj/ejsSession.o $(CONFIG)/obj/ejsWeb.o $(LIBS_132) $(LIBS_132) $(LIBS) -lmpr

#
#   www
#
DEPS_133 += src/jems/ejs.web/www

$(CONFIG)/bin/www: $(DEPS_133)
	cd src/jems/ejs.web; rm -fr ../../../$(CONFIG)/bin/www ; cd ../../..
	cd src/jems/ejs.web; cp -r www ../../../$(CONFIG)/bin ; cd ../../..

#
#   ejs.template.mod
#
DEPS_134 += src/jems/ejs.template/TemplateParser.es
DEPS_134 += $(CONFIG)/bin/ejsc
DEPS_134 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.template.mod: $(DEPS_134)
	cd src/jems/ejs.template; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.template.mod  --optimize 9 TemplateParser.es ; cd ../../..

#
#   ejs.zlib.mod
#
DEPS_135 += src/jems/ejs.zlib/Zlib.es
DEPS_135 += $(CONFIG)/bin/ejsc
DEPS_135 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.zlib.mod: $(DEPS_135)
	cd src/jems/ejs.zlib; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.zlib.mod  --optimize 9 *.es ; cd ../../..

#
#   ejsZlib.o
#
DEPS_136 += $(CONFIG)/inc/bit.h
DEPS_136 += $(CONFIG)/inc/ejs.h
DEPS_136 += $(CONFIG)/inc/zlib.h
DEPS_136 += $(CONFIG)/inc/ejs.zlib.slots.h

$(CONFIG)/obj/ejsZlib.o: \
    src/jems/ejs.zlib/ejsZlib.c $(DEPS_136)
	@echo '   [Compile] src/jems/ejs.zlib/ejsZlib.c'
	$(CC) -c -o $(CONFIG)/obj/ejsZlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) -Isrc/cmd src/jems/ejs.zlib/ejsZlib.c

#
#   libejs.zlib
#
DEPS_137 += $(CONFIG)/bin/libejs.so
DEPS_137 += $(CONFIG)/bin/ejs.mod
DEPS_137 += $(CONFIG)/bin/ejs.zlib.mod
DEPS_137 += $(CONFIG)/bin/libzlib.so
DEPS_137 += $(CONFIG)/obj/ejsZlib.o

LIBS_137 += -lzlib
LIBS_137 += -lejs
LIBS_137 += -lhttp
LIBS_137 += -lpcre
LIBS_137 += -lmpr

$(CONFIG)/bin/libejs.zlib.so: $(DEPS_137)
	@echo '      [Link] libejs.zlib'
	$(CC) -shared -o $(CONFIG)/bin/libejs.zlib.so $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsZlib.o $(LIBS_137) $(LIBS_137) $(LIBS) -lmpr

#
#   ejs.tar.mod
#
DEPS_138 += src/jems/ejs.tar/Tar.es
DEPS_138 += $(CONFIG)/bin/ejsc
DEPS_138 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.tar.mod: $(DEPS_138)
	cd src/jems/ejs.tar; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.tar.mod  --optimize 9 *.es ; cd ../../..

#
#   mvc.es
#
DEPS_139 += src/jems/ejs.mvc/mvc.es

$(CONFIG)/bin/mvc.es: $(DEPS_139)
	cd src/jems/ejs.mvc; cp mvc.es ../../../$(CONFIG)/bin ; cd ../../..

#
#   mvc
#
DEPS_140 += $(CONFIG)/bin/libejs.so
DEPS_140 += $(CONFIG)/bin/mvc.es
DEPS_140 += $(CONFIG)/obj/ejsrun.o

LIBS_140 += -lejs
LIBS_140 += -lhttp
LIBS_140 += -lpcre
LIBS_140 += -lmpr

$(CONFIG)/bin/mvc: $(DEPS_140)
	@echo '      [Link] mvc'
	$(CC) -o $(CONFIG)/bin/mvc $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LIBS_140) $(LIBS_140) $(LIBS) -lmpr $(LDFLAGS)

#
#   ejs.mvc.mod
#
DEPS_141 += src/jems/ejs.mvc/mvc.es
DEPS_141 += $(CONFIG)/bin/ejsc
DEPS_141 += $(CONFIG)/bin/ejs.mod
DEPS_141 += $(CONFIG)/bin/ejs.web.mod
DEPS_141 += $(CONFIG)/bin/ejs.template.mod
DEPS_141 += $(CONFIG)/bin/ejs.unix.mod

$(CONFIG)/bin/ejs.mvc.mod: $(DEPS_141)
	cd src/jems/ejs.mvc; ../../../$(CONFIG)/bin/ejsc --out ../../../$(CONFIG)/bin/ejs.mvc.mod  --optimize 9 *.es ; cd ../../..

#
#   utest.es
#
DEPS_142 += src/jems/ejs.utest/utest.es

$(CONFIG)/bin/utest.es: $(DEPS_142)
	cd src/jems/ejs.utest; cp utest.es ../../../$(CONFIG)/bin ; cd ../../..

#
#   utest.worker
#
DEPS_143 += src/jems/ejs.utest/utest.worker

$(CONFIG)/bin/utest.worker: $(DEPS_143)
	cd src/jems/ejs.utest; cp utest.worker ../../../$(CONFIG)/bin ; cd ../../..

#
#   utest
#
DEPS_144 += $(CONFIG)/bin/libejs.so
DEPS_144 += $(CONFIG)/bin/utest.es
DEPS_144 += $(CONFIG)/bin/utest.worker
DEPS_144 += $(CONFIG)/obj/ejsrun.o

LIBS_144 += -lejs
LIBS_144 += -lhttp
LIBS_144 += -lpcre
LIBS_144 += -lmpr

$(CONFIG)/bin/utest: $(DEPS_144)
	@echo '      [Link] utest'
	$(CC) -o $(CONFIG)/bin/utest $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/ejsrun.o $(LIBS_144) $(LIBS_144) $(LIBS) -lmpr $(LDFLAGS)

#
#   stop
#
stop: $(DEPS_145)

#
#   installBinary
#
DEPS_146 += stop

installBinary: $(DEPS_146)

#
#   start
#
start: $(DEPS_147)

#
#   install
#
DEPS_148 += stop
DEPS_148 += installBinary
DEPS_148 += start

install: $(DEPS_148)
	

#
#   uninstall
#
DEPS_149 += stop

uninstall: $(DEPS_149)
	

