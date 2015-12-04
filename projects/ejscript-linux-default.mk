#
#   ejscript-linux-default.mk -- Makefile to build Embedthis Ejscript for linux
#

NAME                  := ejscript
VERSION               := 2.7.2
PROFILE               ?= default
ARCH                  ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
CC_ARCH               ?= $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                    ?= linux
CC                    ?= gcc
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
BUILD                 ?= build/$(CONFIG)
LBIN                  ?= $(BUILD)/bin
PATH                  := $(LBIN):$(PATH)

ME_COM_COMPILER       ?= 1
ME_COM_HTTP           ?= 1
ME_COM_LIB            ?= 1
ME_COM_MATRIXSSL      ?= 0
ME_COM_MBEDTLS        ?= 1
ME_COM_MPR            ?= 1
ME_COM_NANOSSL        ?= 0
ME_COM_OPENSSL        ?= 0
ME_COM_OSDEP          ?= 1
ME_COM_PCRE           ?= 1
ME_COM_SQLITE         ?= 1
ME_COM_SSL            ?= 1
ME_COM_VXWORKS        ?= 0
ME_COM_ZLIB           ?= 1

ME_COM_OPENSSL_PATH   ?= "/usr/lib"

ifeq ($(ME_COM_LIB),1)
    ME_COM_COMPILER := 1
endif
ifeq ($(ME_COM_MBEDTLS),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_OPENSSL),1)
    ME_COM_SSL := 1
endif

CFLAGS                += -fPIC -w
DFLAGS                += -DME_DEBUG=1 -D_REENTRANT -DPIC $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_COMPILER=$(ME_COM_COMPILER) -DME_COM_HTTP=$(ME_COM_HTTP) -DME_COM_LIB=$(ME_COM_LIB) -DME_COM_MATRIXSSL=$(ME_COM_MATRIXSSL) -DME_COM_MBEDTLS=$(ME_COM_MBEDTLS) -DME_COM_MPR=$(ME_COM_MPR) -DME_COM_NANOSSL=$(ME_COM_NANOSSL) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_OSDEP=$(ME_COM_OSDEP) -DME_COM_PCRE=$(ME_COM_PCRE) -DME_COM_SQLITE=$(ME_COM_SQLITE) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) -DME_COM_ZLIB=$(ME_COM_ZLIB) 
IFLAGS                += "-I$(BUILD)/inc"
LDFLAGS               += '-rdynamic' '-Wl,--enable-new-dtags' '-Wl,-rpath,$$ORIGIN/'
LIBPATHS              += -L$(BUILD)/bin
LIBS                  += -lrt -ldl -lpthread -lm

DEBUG                 ?= debug
CFLAGS-debug          ?= -g
DFLAGS-debug          ?= -DME_DEBUG
LDFLAGS-debug         ?= -g
DFLAGS-release        ?= 
CFLAGS-release        ?= -O2
LDFLAGS-release       ?= 
CFLAGS                += $(CFLAGS-$(DEBUG))
DFLAGS                += $(DFLAGS-$(DEBUG))
LDFLAGS               += $(LDFLAGS-$(DEBUG))

ME_ROOT_PREFIX        ?= 
ME_BASE_PREFIX        ?= $(ME_ROOT_PREFIX)/usr/local
ME_DATA_PREFIX        ?= $(ME_ROOT_PREFIX)/
ME_STATE_PREFIX       ?= $(ME_ROOT_PREFIX)/var
ME_APP_PREFIX         ?= $(ME_BASE_PREFIX)/lib/$(NAME)
ME_VAPP_PREFIX        ?= $(ME_APP_PREFIX)/$(VERSION)
ME_BIN_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/bin
ME_INC_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/include
ME_LIB_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/lib
ME_MAN_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/share/man
ME_SBIN_PREFIX        ?= $(ME_ROOT_PREFIX)/usr/local/sbin
ME_ETC_PREFIX         ?= $(ME_ROOT_PREFIX)/etc/$(NAME)
ME_WEB_PREFIX         ?= $(ME_ROOT_PREFIX)/var/www/$(NAME)
ME_LOG_PREFIX         ?= $(ME_ROOT_PREFIX)/var/log/$(NAME)
ME_SPOOL_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)
ME_CACHE_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)/cache
ME_SRC_PREFIX         ?= $(ME_ROOT_PREFIX)$(NAME)-$(VERSION)


TARGETS               += init
TARGETS               += $(BUILD)/bin/ejs
TARGETS               += $(BUILD)/bin/ejs.db.mapper.mod
TARGETS               += $(BUILD)/bin/ejs.db.sqlite.mod
TARGETS               += $(BUILD)/bin/ejs.mail.mod
TARGETS               += $(BUILD)/bin/ejs.mvc.mod
TARGETS               += $(BUILD)/bin/ejs.tar.mod
TARGETS               += $(BUILD)/bin/ejsrun
TARGETS               += $(BUILD)/.install-certs-modified
TARGETS               += $(BUILD)/bin/libejs.db.sqlite.so
TARGETS               += $(BUILD)/bin/libejs.web.so
TARGETS               += $(BUILD)/bin/mvc
TARGETS               += $(BUILD)/bin/utest
TARGETS               += $(BUILD)/bin/ejsman
TARGETS               += $(BUILD)/bin/www

unexport CDPATH

ifndef SHOW
.SILENT:
endif

all build compile: prep $(TARGETS)

.PHONY: prep

prep:
	@echo "      [Info] Use "make SHOW=1" to trace executed commands."
	@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi
	@if [ "$(ME_APP_PREFIX)" = "" ] ; then echo WARNING: ME_APP_PREFIX not set ; exit 255 ; fi
	@[ ! -x $(BUILD)/bin ] && mkdir -p $(BUILD)/bin; true
	@[ ! -x $(BUILD)/inc ] && mkdir -p $(BUILD)/inc; true
	@[ ! -x $(BUILD)/obj ] && mkdir -p $(BUILD)/obj; true
	@[ ! -f $(BUILD)/inc/me.h ] && cp projects/ejscript-linux-default-me.h $(BUILD)/inc/me.h ; true
	@if ! diff $(BUILD)/inc/me.h projects/ejscript-linux-default-me.h >/dev/null ; then\
		cp projects/ejscript-linux-default-me.h $(BUILD)/inc/me.h  ; \
	fi; true
	@if [ -f "$(BUILD)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != "`cat $(BUILD)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build" ; \
			echo "   [Warning] Previous build command: "`cat $(BUILD)/.makeflags`"" ; \
		fi ; \
	fi
	@echo "$(MAKEFLAGS)" >$(BUILD)/.makeflags

clean:
	rm -f "$(BUILD)/obj/doc.o"
	rm -f "$(BUILD)/obj/docFiles.o"
	rm -f "$(BUILD)/obj/dtoa.o"
	rm -f "$(BUILD)/obj/ecAst.o"
	rm -f "$(BUILD)/obj/ecCodeGen.o"
	rm -f "$(BUILD)/obj/ecCompiler.o"
	rm -f "$(BUILD)/obj/ecLex.o"
	rm -f "$(BUILD)/obj/ecModuleWrite.o"
	rm -f "$(BUILD)/obj/ecParser.o"
	rm -f "$(BUILD)/obj/ecState.o"
	rm -f "$(BUILD)/obj/ejs.o"
	rm -f "$(BUILD)/obj/ejsApp.o"
	rm -f "$(BUILD)/obj/ejsArray.o"
	rm -f "$(BUILD)/obj/ejsBlock.o"
	rm -f "$(BUILD)/obj/ejsBoolean.o"
	rm -f "$(BUILD)/obj/ejsByteArray.o"
	rm -f "$(BUILD)/obj/ejsByteCode.o"
	rm -f "$(BUILD)/obj/ejsCache.o"
	rm -f "$(BUILD)/obj/ejsCmd.o"
	rm -f "$(BUILD)/obj/ejsConfig.o"
	rm -f "$(BUILD)/obj/ejsDate.o"
	rm -f "$(BUILD)/obj/ejsDebug.o"
	rm -f "$(BUILD)/obj/ejsError.o"
	rm -f "$(BUILD)/obj/ejsException.o"
	rm -f "$(BUILD)/obj/ejsFile.o"
	rm -f "$(BUILD)/obj/ejsFileSystem.o"
	rm -f "$(BUILD)/obj/ejsFrame.o"
	rm -f "$(BUILD)/obj/ejsFunction.o"
	rm -f "$(BUILD)/obj/ejsGC.o"
	rm -f "$(BUILD)/obj/ejsGlobal.o"
	rm -f "$(BUILD)/obj/ejsHelper.o"
	rm -f "$(BUILD)/obj/ejsHttp.o"
	rm -f "$(BUILD)/obj/ejsHttpServer.o"
	rm -f "$(BUILD)/obj/ejsInterp.o"
	rm -f "$(BUILD)/obj/ejsIterator.o"
	rm -f "$(BUILD)/obj/ejsJSON.o"
	rm -f "$(BUILD)/obj/ejsLoader.o"
	rm -f "$(BUILD)/obj/ejsLocalCache.o"
	rm -f "$(BUILD)/obj/ejsMath.o"
	rm -f "$(BUILD)/obj/ejsMemory.o"
	rm -f "$(BUILD)/obj/ejsModule.o"
	rm -f "$(BUILD)/obj/ejsMprLog.o"
	rm -f "$(BUILD)/obj/ejsNamespace.o"
	rm -f "$(BUILD)/obj/ejsNull.o"
	rm -f "$(BUILD)/obj/ejsNumber.o"
	rm -f "$(BUILD)/obj/ejsObject.o"
	rm -f "$(BUILD)/obj/ejsPath.o"
	rm -f "$(BUILD)/obj/ejsPot.o"
	rm -f "$(BUILD)/obj/ejsRegExp.o"
	rm -f "$(BUILD)/obj/ejsRequest.o"
	rm -f "$(BUILD)/obj/ejsScope.o"
	rm -f "$(BUILD)/obj/ejsService.o"
	rm -f "$(BUILD)/obj/ejsSession.o"
	rm -f "$(BUILD)/obj/ejsSocket.o"
	rm -f "$(BUILD)/obj/ejsSqlite.o"
	rm -f "$(BUILD)/obj/ejsString.o"
	rm -f "$(BUILD)/obj/ejsSystem.o"
	rm -f "$(BUILD)/obj/ejsTimer.o"
	rm -f "$(BUILD)/obj/ejsType.o"
	rm -f "$(BUILD)/obj/ejsUri.o"
	rm -f "$(BUILD)/obj/ejsVoid.o"
	rm -f "$(BUILD)/obj/ejsWeb.o"
	rm -f "$(BUILD)/obj/ejsWebSocket.o"
	rm -f "$(BUILD)/obj/ejsWorker.o"
	rm -f "$(BUILD)/obj/ejsXML.o"
	rm -f "$(BUILD)/obj/ejsXMLList.o"
	rm -f "$(BUILD)/obj/ejsXMLLoader.o"
	rm -f "$(BUILD)/obj/ejsZlib.o"
	rm -f "$(BUILD)/obj/ejsc.o"
	rm -f "$(BUILD)/obj/ejsmod.o"
	rm -f "$(BUILD)/obj/ejsrun.o"
	rm -f "$(BUILD)/obj/http.o"
	rm -f "$(BUILD)/obj/httpLib.o"
	rm -f "$(BUILD)/obj/listing.o"
	rm -f "$(BUILD)/obj/mbedtls.o"
	rm -f "$(BUILD)/obj/mpr-mbedtls.o"
	rm -f "$(BUILD)/obj/mpr-openssl.o"
	rm -f "$(BUILD)/obj/mprLib.o"
	rm -f "$(BUILD)/obj/pcre.o"
	rm -f "$(BUILD)/obj/slotGen.o"
	rm -f "$(BUILD)/obj/sqlite.o"
	rm -f "$(BUILD)/obj/sqlite3.o"
	rm -f "$(BUILD)/obj/watchdog.o"
	rm -f "$(BUILD)/obj/zlib.o"
	rm -f "$(BUILD)/bin/ejs"
	rm -f "$(BUILD)/bin/ejsc"
	rm -f "$(BUILD)/bin/ejsmod"
	rm -f "$(BUILD)/bin/ejsrun"
	rm -f "$(BUILD)/.install-certs-modified"
	rm -f "$(BUILD)/bin/libejs.so"
	rm -f "$(BUILD)/bin/libejs.db.sqlite.so"
	rm -f "$(BUILD)/bin/libejs.web.so"
	rm -f "$(BUILD)/bin/libejs.zlib.so"
	rm -f "$(BUILD)/bin/libhttp.so"
	rm -f "$(BUILD)/bin/libmbedtls.a"
	rm -f "$(BUILD)/bin/libmpr.so"
	rm -f "$(BUILD)/bin/libmpr-mbedtls.a"
	rm -f "$(BUILD)/bin/libpcre.so"
	rm -f "$(BUILD)/bin/libsql.so"
	rm -f "$(BUILD)/bin/libzlib.so"
	rm -f "$(BUILD)/bin/mvc.es"
	rm -f "$(BUILD)/bin/utest"
	rm -f "$(BUILD)/bin/utest.es"
	rm -f "$(BUILD)/bin/utest.worker"
	rm -f "$(BUILD)/bin/ejsman"
	rm -f "$(BUILD)/bin/www"

clobber: clean
	rm -fr ./$(BUILD)

#
#   init
#

init: $(DEPS_1)
	if [ ! -d /usr/include/openssl ] ; then echo ; \
	echo Install libssl-dev to get /usr/include/openssl ; \
	exit 255 ; \
	fi

#
#   ejs.cache.local.slots.h
#
DEPS_2 += src/slots/ejs.cache.local.slots.h

$(BUILD)/inc/ejs.cache.local.slots.h: $(DEPS_2)
	@echo '      [Copy] $(BUILD)/inc/ejs.cache.local.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/slots/ejs.cache.local.slots.h $(BUILD)/inc/ejs.cache.local.slots.h

#
#   ejs.db.sqlite.slots.h
#
DEPS_3 += src/slots/ejs.db.sqlite.slots.h

$(BUILD)/inc/ejs.db.sqlite.slots.h: $(DEPS_3)
	@echo '      [Copy] $(BUILD)/inc/ejs.db.sqlite.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/slots/ejs.db.sqlite.slots.h $(BUILD)/inc/ejs.db.sqlite.slots.h

#
#   me.h
#

$(BUILD)/inc/me.h: $(DEPS_4)

#
#   osdep.h
#
DEPS_5 += src/osdep/osdep.h
DEPS_5 += $(BUILD)/inc/me.h

$(BUILD)/inc/osdep.h: $(DEPS_5)
	@echo '      [Copy] $(BUILD)/inc/osdep.h'
	mkdir -p "$(BUILD)/inc"
	cp src/osdep/osdep.h $(BUILD)/inc/osdep.h

#
#   mpr.h
#
DEPS_6 += src/mpr/mpr.h
DEPS_6 += $(BUILD)/inc/me.h
DEPS_6 += $(BUILD)/inc/osdep.h

$(BUILD)/inc/mpr.h: $(DEPS_6)
	@echo '      [Copy] $(BUILD)/inc/mpr.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mpr/mpr.h $(BUILD)/inc/mpr.h

#
#   http.h
#
DEPS_7 += src/http/http.h
DEPS_7 += $(BUILD)/inc/mpr.h

$(BUILD)/inc/http.h: $(DEPS_7)
	@echo '      [Copy] $(BUILD)/inc/http.h'
	mkdir -p "$(BUILD)/inc"
	cp src/http/http.h $(BUILD)/inc/http.h

#
#   ejsByteCode.h
#

src/ejsByteCode.h: $(DEPS_8)

#
#   ejsByteCodeTable.h
#

src/ejsByteCodeTable.h: $(DEPS_9)

#
#   ejs.slots.h
#
DEPS_10 += src/slots/ejs.slots.h

$(BUILD)/inc/ejs.slots.h: $(DEPS_10)
	@echo '      [Copy] $(BUILD)/inc/ejs.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/slots/ejs.slots.h $(BUILD)/inc/ejs.slots.h

#
#   ejsCustomize.h
#

src/ejsCustomize.h: $(DEPS_11)

#
#   ejs.h
#
DEPS_12 += src/ejs.h
DEPS_12 += $(BUILD)/inc/mpr.h
DEPS_12 += $(BUILD)/inc/http.h
DEPS_12 += src/ejsByteCode.h
DEPS_12 += src/ejsByteCodeTable.h
DEPS_12 += $(BUILD)/inc/ejs.slots.h
DEPS_12 += src/ejsCustomize.h

$(BUILD)/inc/ejs.h: $(DEPS_12)
	@echo '      [Copy] $(BUILD)/inc/ejs.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejs.h $(BUILD)/inc/ejs.h

#
#   ejs.web.slots.h
#
DEPS_13 += src/slots/ejs.web.slots.h

$(BUILD)/inc/ejs.web.slots.h: $(DEPS_13)
	@echo '      [Copy] $(BUILD)/inc/ejs.web.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/slots/ejs.web.slots.h $(BUILD)/inc/ejs.web.slots.h

#
#   ejs.zlib.slots.h
#
DEPS_14 += src/slots/ejs.zlib.slots.h

$(BUILD)/inc/ejs.zlib.slots.h: $(DEPS_14)
	@echo '      [Copy] $(BUILD)/inc/ejs.zlib.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/slots/ejs.zlib.slots.h $(BUILD)/inc/ejs.zlib.slots.h

#
#   ejsByteCode.h
#
DEPS_15 += src/ejsByteCode.h

$(BUILD)/inc/ejsByteCode.h: $(DEPS_15)
	@echo '      [Copy] $(BUILD)/inc/ejsByteCode.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejsByteCode.h $(BUILD)/inc/ejsByteCode.h

#
#   ejsByteCodeTable.h
#
DEPS_16 += src/ejsByteCodeTable.h

$(BUILD)/inc/ejsByteCodeTable.h: $(DEPS_16)
	@echo '      [Copy] $(BUILD)/inc/ejsByteCodeTable.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejsByteCodeTable.h $(BUILD)/inc/ejsByteCodeTable.h

#
#   ejs.h
#

src/ejs.h: $(DEPS_17)

#
#   ejsCompiler.h
#
DEPS_18 += src/ejsCompiler.h
DEPS_18 += src/ejs.h

$(BUILD)/inc/ejsCompiler.h: $(DEPS_18)
	@echo '      [Copy] $(BUILD)/inc/ejsCompiler.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejsCompiler.h $(BUILD)/inc/ejsCompiler.h

#
#   ejsCustomize.h
#
DEPS_19 += src/ejsCustomize.h

$(BUILD)/inc/ejsCustomize.h: $(DEPS_19)
	@echo '      [Copy] $(BUILD)/inc/ejsCustomize.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejsCustomize.h $(BUILD)/inc/ejsCustomize.h

#
#   ejsWeb.h
#
DEPS_20 += src/ejs.web/ejsWeb.h

$(BUILD)/inc/ejsWeb.h: $(DEPS_20)
	@echo '      [Copy] $(BUILD)/inc/ejsWeb.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejs.web/ejsWeb.h $(BUILD)/inc/ejsWeb.h

#
#   ejsmod.h
#
DEPS_21 += src/cmd/ejsmod.h

$(BUILD)/inc/ejsmod.h: $(DEPS_21)
	@echo '      [Copy] $(BUILD)/inc/ejsmod.h'
	mkdir -p "$(BUILD)/inc"
	cp src/cmd/ejsmod.h $(BUILD)/inc/ejsmod.h

#
#   embedtls.h
#
DEPS_22 += src/mbedtls/embedtls.h

$(BUILD)/inc/embedtls.h: $(DEPS_22)
	@echo '      [Copy] $(BUILD)/inc/embedtls.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mbedtls/embedtls.h $(BUILD)/inc/embedtls.h

#
#   mbedtls-config.h
#
DEPS_23 += src/mbedtls/mbedtls-config.h

$(BUILD)/inc/mbedtls-config.h: $(DEPS_23)
	@echo '      [Copy] $(BUILD)/inc/mbedtls-config.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mbedtls/mbedtls-config.h $(BUILD)/inc/mbedtls-config.h

#
#   mbedtls.h
#
DEPS_24 += src/mbedtls/mbedtls.h

$(BUILD)/inc/mbedtls.h: $(DEPS_24)
	@echo '      [Copy] $(BUILD)/inc/mbedtls.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mbedtls/mbedtls.h $(BUILD)/inc/mbedtls.h

#
#   pcre.h
#
DEPS_25 += src/pcre/pcre.h

$(BUILD)/inc/pcre.h: $(DEPS_25)
	@echo '      [Copy] $(BUILD)/inc/pcre.h'
	mkdir -p "$(BUILD)/inc"
	cp src/pcre/pcre.h $(BUILD)/inc/pcre.h

#
#   sqlite3.h
#
DEPS_26 += src/sqlite/sqlite3.h

$(BUILD)/inc/sqlite3.h: $(DEPS_26)
	@echo '      [Copy] $(BUILD)/inc/sqlite3.h'
	mkdir -p "$(BUILD)/inc"
	cp src/sqlite/sqlite3.h $(BUILD)/inc/sqlite3.h

#
#   zlib.h
#
DEPS_27 += src/zlib/zlib.h
DEPS_27 += $(BUILD)/inc/me.h

$(BUILD)/inc/zlib.h: $(DEPS_27)
	@echo '      [Copy] $(BUILD)/inc/zlib.h'
	mkdir -p "$(BUILD)/inc"
	cp src/zlib/zlib.h $(BUILD)/inc/zlib.h

#
#   ejsmod.h
#

src/cmd/ejsmod.h: $(DEPS_28)

#
#   doc.o
#
DEPS_29 += src/cmd/ejsmod.h

$(BUILD)/obj/doc.o: \
    src/cmd/doc.c $(DEPS_29)
	@echo '   [Compile] $(BUILD)/obj/doc.o'
	$(CC) -c -o $(BUILD)/obj/doc.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/cmd/doc.c

#
#   docFiles.o
#
DEPS_30 += src/cmd/ejsmod.h

$(BUILD)/obj/docFiles.o: \
    src/cmd/docFiles.c $(DEPS_30)
	@echo '   [Compile] $(BUILD)/obj/docFiles.o'
	$(CC) -c -o $(BUILD)/obj/docFiles.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/cmd/docFiles.c

#
#   dtoa.o
#
DEPS_31 += $(BUILD)/inc/mpr.h

$(BUILD)/obj/dtoa.o: \
    src/core/src/dtoa.c $(DEPS_31)
	@echo '   [Compile] $(BUILD)/obj/dtoa.o'
	$(CC) -c -o $(BUILD)/obj/dtoa.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/dtoa.c

#
#   ejsCompiler.h
#

src/ejsCompiler.h: $(DEPS_32)

#
#   ecAst.o
#
DEPS_33 += src/ejsCompiler.h

$(BUILD)/obj/ecAst.o: \
    src/compiler/ecAst.c $(DEPS_33)
	@echo '   [Compile] $(BUILD)/obj/ecAst.o'
	$(CC) -c -o $(BUILD)/obj/ecAst.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/compiler/ecAst.c

#
#   ecCodeGen.o
#
DEPS_34 += src/ejsCompiler.h

$(BUILD)/obj/ecCodeGen.o: \
    src/compiler/ecCodeGen.c $(DEPS_34)
	@echo '   [Compile] $(BUILD)/obj/ecCodeGen.o'
	$(CC) -c -o $(BUILD)/obj/ecCodeGen.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/compiler/ecCodeGen.c

#
#   ecCompiler.o
#
DEPS_35 += src/ejsCompiler.h

$(BUILD)/obj/ecCompiler.o: \
    src/compiler/ecCompiler.c $(DEPS_35)
	@echo '   [Compile] $(BUILD)/obj/ecCompiler.o'
	$(CC) -c -o $(BUILD)/obj/ecCompiler.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/compiler/ecCompiler.c

#
#   ecLex.o
#
DEPS_36 += src/ejsCompiler.h

$(BUILD)/obj/ecLex.o: \
    src/compiler/ecLex.c $(DEPS_36)
	@echo '   [Compile] $(BUILD)/obj/ecLex.o'
	$(CC) -c -o $(BUILD)/obj/ecLex.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/compiler/ecLex.c

#
#   ecModuleWrite.o
#
DEPS_37 += src/ejsCompiler.h

$(BUILD)/obj/ecModuleWrite.o: \
    src/compiler/ecModuleWrite.c $(DEPS_37)
	@echo '   [Compile] $(BUILD)/obj/ecModuleWrite.o'
	$(CC) -c -o $(BUILD)/obj/ecModuleWrite.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/compiler/ecModuleWrite.c

#
#   ecParser.o
#
DEPS_38 += src/ejsCompiler.h

$(BUILD)/obj/ecParser.o: \
    src/compiler/ecParser.c $(DEPS_38)
	@echo '   [Compile] $(BUILD)/obj/ecParser.o'
	$(CC) -c -o $(BUILD)/obj/ecParser.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/compiler/ecParser.c

#
#   ecState.o
#
DEPS_39 += src/ejsCompiler.h

$(BUILD)/obj/ecState.o: \
    src/compiler/ecState.c $(DEPS_39)
	@echo '   [Compile] $(BUILD)/obj/ecState.o'
	$(CC) -c -o $(BUILD)/obj/ecState.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/compiler/ecState.c

#
#   ejs.o
#
DEPS_40 += $(BUILD)/inc/me.h
DEPS_40 += $(BUILD)/inc/ejsCompiler.h

$(BUILD)/obj/ejs.o: \
    src/cmd/ejs.c $(DEPS_40)
	@echo '   [Compile] $(BUILD)/obj/ejs.o'
	$(CC) -c -o $(BUILD)/obj/ejs.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/cmd/ejs.c

#
#   ejsApp.o
#
DEPS_41 += src/ejs.h

$(BUILD)/obj/ejsApp.o: \
    src/core/src/ejsApp.c $(DEPS_41)
	@echo '   [Compile] $(BUILD)/obj/ejsApp.o'
	$(CC) -c -o $(BUILD)/obj/ejsApp.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsApp.c

#
#   ejsArray.o
#
DEPS_42 += src/ejs.h

$(BUILD)/obj/ejsArray.o: \
    src/core/src/ejsArray.c $(DEPS_42)
	@echo '   [Compile] $(BUILD)/obj/ejsArray.o'
	$(CC) -c -o $(BUILD)/obj/ejsArray.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsArray.c

#
#   ejsBlock.o
#
DEPS_43 += src/ejs.h

$(BUILD)/obj/ejsBlock.o: \
    src/core/src/ejsBlock.c $(DEPS_43)
	@echo '   [Compile] $(BUILD)/obj/ejsBlock.o'
	$(CC) -c -o $(BUILD)/obj/ejsBlock.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsBlock.c

#
#   ejsBoolean.o
#
DEPS_44 += src/ejs.h

$(BUILD)/obj/ejsBoolean.o: \
    src/core/src/ejsBoolean.c $(DEPS_44)
	@echo '   [Compile] $(BUILD)/obj/ejsBoolean.o'
	$(CC) -c -o $(BUILD)/obj/ejsBoolean.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsBoolean.c

#
#   ejsByteArray.o
#
DEPS_45 += src/ejs.h

$(BUILD)/obj/ejsByteArray.o: \
    src/core/src/ejsByteArray.c $(DEPS_45)
	@echo '   [Compile] $(BUILD)/obj/ejsByteArray.o'
	$(CC) -c -o $(BUILD)/obj/ejsByteArray.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsByteArray.c

#
#   ejsByteCode.o
#
DEPS_46 += src/ejs.h

$(BUILD)/obj/ejsByteCode.o: \
    src/vm/ejsByteCode.c $(DEPS_46)
	@echo '   [Compile] $(BUILD)/obj/ejsByteCode.o'
	$(CC) -c -o $(BUILD)/obj/ejsByteCode.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/vm/ejsByteCode.c

#
#   ejsCache.o
#
DEPS_47 += src/ejs.h

$(BUILD)/obj/ejsCache.o: \
    src/core/src/ejsCache.c $(DEPS_47)
	@echo '   [Compile] $(BUILD)/obj/ejsCache.o'
	$(CC) -c -o $(BUILD)/obj/ejsCache.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsCache.c

#
#   ejsCmd.o
#
DEPS_48 += src/ejs.h

$(BUILD)/obj/ejsCmd.o: \
    src/core/src/ejsCmd.c $(DEPS_48)
	@echo '   [Compile] $(BUILD)/obj/ejsCmd.o'
	$(CC) -c -o $(BUILD)/obj/ejsCmd.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsCmd.c

#
#   ejsConfig.o
#
DEPS_49 += src/ejs.h

$(BUILD)/obj/ejsConfig.o: \
    src/core/src/ejsConfig.c $(DEPS_49)
	@echo '   [Compile] $(BUILD)/obj/ejsConfig.o'
	$(CC) -c -o $(BUILD)/obj/ejsConfig.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsConfig.c

#
#   ejsDate.o
#
DEPS_50 += src/ejs.h

$(BUILD)/obj/ejsDate.o: \
    src/core/src/ejsDate.c $(DEPS_50)
	@echo '   [Compile] $(BUILD)/obj/ejsDate.o'
	$(CC) -c -o $(BUILD)/obj/ejsDate.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsDate.c

#
#   ejsDebug.o
#
DEPS_51 += src/ejs.h

$(BUILD)/obj/ejsDebug.o: \
    src/core/src/ejsDebug.c $(DEPS_51)
	@echo '   [Compile] $(BUILD)/obj/ejsDebug.o'
	$(CC) -c -o $(BUILD)/obj/ejsDebug.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsDebug.c

#
#   ejsError.o
#
DEPS_52 += src/ejs.h

$(BUILD)/obj/ejsError.o: \
    src/core/src/ejsError.c $(DEPS_52)
	@echo '   [Compile] $(BUILD)/obj/ejsError.o'
	$(CC) -c -o $(BUILD)/obj/ejsError.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsError.c

#
#   ejsException.o
#
DEPS_53 += src/ejs.h

$(BUILD)/obj/ejsException.o: \
    src/vm/ejsException.c $(DEPS_53)
	@echo '   [Compile] $(BUILD)/obj/ejsException.o'
	$(CC) -c -o $(BUILD)/obj/ejsException.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/vm/ejsException.c

#
#   ejsFile.o
#
DEPS_54 += src/ejs.h

$(BUILD)/obj/ejsFile.o: \
    src/core/src/ejsFile.c $(DEPS_54)
	@echo '   [Compile] $(BUILD)/obj/ejsFile.o'
	$(CC) -c -o $(BUILD)/obj/ejsFile.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsFile.c

#
#   ejsFileSystem.o
#
DEPS_55 += src/ejs.h

$(BUILD)/obj/ejsFileSystem.o: \
    src/core/src/ejsFileSystem.c $(DEPS_55)
	@echo '   [Compile] $(BUILD)/obj/ejsFileSystem.o'
	$(CC) -c -o $(BUILD)/obj/ejsFileSystem.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsFileSystem.c

#
#   ejsFrame.o
#
DEPS_56 += src/ejs.h

$(BUILD)/obj/ejsFrame.o: \
    src/core/src/ejsFrame.c $(DEPS_56)
	@echo '   [Compile] $(BUILD)/obj/ejsFrame.o'
	$(CC) -c -o $(BUILD)/obj/ejsFrame.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsFrame.c

#
#   ejsFunction.o
#
DEPS_57 += src/ejs.h

$(BUILD)/obj/ejsFunction.o: \
    src/core/src/ejsFunction.c $(DEPS_57)
	@echo '   [Compile] $(BUILD)/obj/ejsFunction.o'
	$(CC) -c -o $(BUILD)/obj/ejsFunction.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsFunction.c

#
#   ejsGC.o
#
DEPS_58 += src/ejs.h

$(BUILD)/obj/ejsGC.o: \
    src/core/src/ejsGC.c $(DEPS_58)
	@echo '   [Compile] $(BUILD)/obj/ejsGC.o'
	$(CC) -c -o $(BUILD)/obj/ejsGC.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsGC.c

#
#   ejsGlobal.o
#
DEPS_59 += src/ejs.h

$(BUILD)/obj/ejsGlobal.o: \
    src/core/src/ejsGlobal.c $(DEPS_59)
	@echo '   [Compile] $(BUILD)/obj/ejsGlobal.o'
	$(CC) -c -o $(BUILD)/obj/ejsGlobal.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsGlobal.c

#
#   ejsHelper.o
#
DEPS_60 += src/ejs.h

$(BUILD)/obj/ejsHelper.o: \
    src/vm/ejsHelper.c $(DEPS_60)
	@echo '   [Compile] $(BUILD)/obj/ejsHelper.o'
	$(CC) -c -o $(BUILD)/obj/ejsHelper.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/vm/ejsHelper.c

#
#   ejsHttp.o
#
DEPS_61 += src/ejs.h

$(BUILD)/obj/ejsHttp.o: \
    src/core/src/ejsHttp.c $(DEPS_61)
	@echo '   [Compile] $(BUILD)/obj/ejsHttp.o'
	$(CC) -c -o $(BUILD)/obj/ejsHttp.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsHttp.c

#
#   ejsWeb.h
#

src/ejs.web/ejsWeb.h: $(DEPS_62)

#
#   ejsHttpServer.o
#
DEPS_63 += $(BUILD)/inc/me.h
DEPS_63 += $(BUILD)/inc/ejs.h
DEPS_63 += $(BUILD)/inc/ejsCompiler.h
DEPS_63 += src/ejs.web/ejsWeb.h
DEPS_63 += $(BUILD)/inc/ejs.web.slots.h

$(BUILD)/obj/ejsHttpServer.o: \
    src/ejs.web/ejsHttpServer.c $(DEPS_63)
	@echo '   [Compile] $(BUILD)/obj/ejsHttpServer.o'
	$(CC) -c -o $(BUILD)/obj/ejsHttpServer.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/ejs.web/ejsHttpServer.c

#
#   ejsInterp.o
#
DEPS_64 += src/ejs.h

$(BUILD)/obj/ejsInterp.o: \
    src/vm/ejsInterp.c $(DEPS_64)
	@echo '   [Compile] $(BUILD)/obj/ejsInterp.o'
	$(CC) -c -o $(BUILD)/obj/ejsInterp.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/vm/ejsInterp.c

#
#   ejsIterator.o
#
DEPS_65 += src/ejs.h

$(BUILD)/obj/ejsIterator.o: \
    src/core/src/ejsIterator.c $(DEPS_65)
	@echo '   [Compile] $(BUILD)/obj/ejsIterator.o'
	$(CC) -c -o $(BUILD)/obj/ejsIterator.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsIterator.c

#
#   ejsJSON.o
#
DEPS_66 += src/ejs.h

$(BUILD)/obj/ejsJSON.o: \
    src/core/src/ejsJSON.c $(DEPS_66)
	@echo '   [Compile] $(BUILD)/obj/ejsJSON.o'
	$(CC) -c -o $(BUILD)/obj/ejsJSON.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsJSON.c

#
#   ejsLoader.o
#
DEPS_67 += src/ejs.h

$(BUILD)/obj/ejsLoader.o: \
    src/vm/ejsLoader.c $(DEPS_67)
	@echo '   [Compile] $(BUILD)/obj/ejsLoader.o'
	$(CC) -c -o $(BUILD)/obj/ejsLoader.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/vm/ejsLoader.c

#
#   ejsLocalCache.o
#
DEPS_68 += src/ejs.h

$(BUILD)/obj/ejsLocalCache.o: \
    src/core/src/ejsLocalCache.c $(DEPS_68)
	@echo '   [Compile] $(BUILD)/obj/ejsLocalCache.o'
	$(CC) -c -o $(BUILD)/obj/ejsLocalCache.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsLocalCache.c

#
#   ejsMath.o
#
DEPS_69 += src/ejs.h

$(BUILD)/obj/ejsMath.o: \
    src/core/src/ejsMath.c $(DEPS_69)
	@echo '   [Compile] $(BUILD)/obj/ejsMath.o'
	$(CC) -c -o $(BUILD)/obj/ejsMath.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsMath.c

#
#   ejsMemory.o
#
DEPS_70 += src/ejs.h

$(BUILD)/obj/ejsMemory.o: \
    src/core/src/ejsMemory.c $(DEPS_70)
	@echo '   [Compile] $(BUILD)/obj/ejsMemory.o'
	$(CC) -c -o $(BUILD)/obj/ejsMemory.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsMemory.c

#
#   ejsModule.o
#
DEPS_71 += src/ejs.h

$(BUILD)/obj/ejsModule.o: \
    src/vm/ejsModule.c $(DEPS_71)
	@echo '   [Compile] $(BUILD)/obj/ejsModule.o'
	$(CC) -c -o $(BUILD)/obj/ejsModule.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/vm/ejsModule.c

#
#   ejsMprLog.o
#
DEPS_72 += src/ejs.h

$(BUILD)/obj/ejsMprLog.o: \
    src/core/src/ejsMprLog.c $(DEPS_72)
	@echo '   [Compile] $(BUILD)/obj/ejsMprLog.o'
	$(CC) -c -o $(BUILD)/obj/ejsMprLog.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsMprLog.c

#
#   ejsNamespace.o
#
DEPS_73 += src/ejs.h

$(BUILD)/obj/ejsNamespace.o: \
    src/core/src/ejsNamespace.c $(DEPS_73)
	@echo '   [Compile] $(BUILD)/obj/ejsNamespace.o'
	$(CC) -c -o $(BUILD)/obj/ejsNamespace.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsNamespace.c

#
#   ejsNull.o
#
DEPS_74 += src/ejs.h

$(BUILD)/obj/ejsNull.o: \
    src/core/src/ejsNull.c $(DEPS_74)
	@echo '   [Compile] $(BUILD)/obj/ejsNull.o'
	$(CC) -c -o $(BUILD)/obj/ejsNull.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsNull.c

#
#   ejsNumber.o
#
DEPS_75 += src/ejs.h

$(BUILD)/obj/ejsNumber.o: \
    src/core/src/ejsNumber.c $(DEPS_75)
	@echo '   [Compile] $(BUILD)/obj/ejsNumber.o'
	$(CC) -c -o $(BUILD)/obj/ejsNumber.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsNumber.c

#
#   ejsObject.o
#
DEPS_76 += src/ejs.h

$(BUILD)/obj/ejsObject.o: \
    src/core/src/ejsObject.c $(DEPS_76)
	@echo '   [Compile] $(BUILD)/obj/ejsObject.o'
	$(CC) -c -o $(BUILD)/obj/ejsObject.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsObject.c

#
#   ejsPath.o
#
DEPS_77 += src/ejs.h
DEPS_77 += $(BUILD)/inc/pcre.h

$(BUILD)/obj/ejsPath.o: \
    src/core/src/ejsPath.c $(DEPS_77)
	@echo '   [Compile] $(BUILD)/obj/ejsPath.o'
	$(CC) -c -o $(BUILD)/obj/ejsPath.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsPath.c

#
#   ejsPot.o
#
DEPS_78 += src/ejs.h

$(BUILD)/obj/ejsPot.o: \
    src/core/src/ejsPot.c $(DEPS_78)
	@echo '   [Compile] $(BUILD)/obj/ejsPot.o'
	$(CC) -c -o $(BUILD)/obj/ejsPot.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsPot.c

#
#   ejsRegExp.o
#
DEPS_79 += src/ejs.h
DEPS_79 += $(BUILD)/inc/pcre.h

$(BUILD)/obj/ejsRegExp.o: \
    src/core/src/ejsRegExp.c $(DEPS_79)
	@echo '   [Compile] $(BUILD)/obj/ejsRegExp.o'
	$(CC) -c -o $(BUILD)/obj/ejsRegExp.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsRegExp.c

#
#   ejsRequest.o
#
DEPS_80 += $(BUILD)/inc/me.h
DEPS_80 += $(BUILD)/inc/ejs.h
DEPS_80 += $(BUILD)/inc/ejsCompiler.h
DEPS_80 += src/ejs.web/ejsWeb.h
DEPS_80 += $(BUILD)/inc/ejs.web.slots.h

$(BUILD)/obj/ejsRequest.o: \
    src/ejs.web/ejsRequest.c $(DEPS_80)
	@echo '   [Compile] $(BUILD)/obj/ejsRequest.o'
	$(CC) -c -o $(BUILD)/obj/ejsRequest.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/ejs.web/ejsRequest.c

#
#   ejsScope.o
#
DEPS_81 += src/ejs.h

$(BUILD)/obj/ejsScope.o: \
    src/vm/ejsScope.c $(DEPS_81)
	@echo '   [Compile] $(BUILD)/obj/ejsScope.o'
	$(CC) -c -o $(BUILD)/obj/ejsScope.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/vm/ejsScope.c

#
#   ejsService.o
#
DEPS_82 += src/ejs.h

$(BUILD)/obj/ejsService.o: \
    src/vm/ejsService.c $(DEPS_82)
	@echo '   [Compile] $(BUILD)/obj/ejsService.o'
	$(CC) -c -o $(BUILD)/obj/ejsService.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/vm/ejsService.c

#
#   ejsSession.o
#
DEPS_83 += $(BUILD)/inc/me.h
DEPS_83 += $(BUILD)/inc/ejs.h
DEPS_83 += src/ejs.web/ejsWeb.h

$(BUILD)/obj/ejsSession.o: \
    src/ejs.web/ejsSession.c $(DEPS_83)
	@echo '   [Compile] $(BUILD)/obj/ejsSession.o'
	$(CC) -c -o $(BUILD)/obj/ejsSession.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/ejs.web/ejsSession.c

#
#   ejsSocket.o
#
DEPS_84 += src/ejs.h

$(BUILD)/obj/ejsSocket.o: \
    src/core/src/ejsSocket.c $(DEPS_84)
	@echo '   [Compile] $(BUILD)/obj/ejsSocket.o'
	$(CC) -c -o $(BUILD)/obj/ejsSocket.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsSocket.c

#
#   ejsSqlite.o
#
DEPS_85 += $(BUILD)/inc/ejs.h
DEPS_85 += $(BUILD)/inc/ejs.db.sqlite.slots.h

$(BUILD)/obj/ejsSqlite.o: \
    src/ejs.db.sqlite/ejsSqlite.c $(DEPS_85)
	@echo '   [Compile] $(BUILD)/obj/ejsSqlite.o'
	$(CC) -c -o $(BUILD)/obj/ejsSqlite.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/ejs.db.sqlite/ejsSqlite.c

#
#   ejsString.o
#
DEPS_86 += src/ejs.h
DEPS_86 += $(BUILD)/inc/pcre.h

$(BUILD)/obj/ejsString.o: \
    src/core/src/ejsString.c $(DEPS_86)
	@echo '   [Compile] $(BUILD)/obj/ejsString.o'
	$(CC) -c -o $(BUILD)/obj/ejsString.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsString.c

#
#   ejsSystem.o
#
DEPS_87 += src/ejs.h

$(BUILD)/obj/ejsSystem.o: \
    src/core/src/ejsSystem.c $(DEPS_87)
	@echo '   [Compile] $(BUILD)/obj/ejsSystem.o'
	$(CC) -c -o $(BUILD)/obj/ejsSystem.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsSystem.c

#
#   ejsTimer.o
#
DEPS_88 += src/ejs.h

$(BUILD)/obj/ejsTimer.o: \
    src/core/src/ejsTimer.c $(DEPS_88)
	@echo '   [Compile] $(BUILD)/obj/ejsTimer.o'
	$(CC) -c -o $(BUILD)/obj/ejsTimer.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsTimer.c

#
#   ejsType.o
#
DEPS_89 += src/ejs.h

$(BUILD)/obj/ejsType.o: \
    src/core/src/ejsType.c $(DEPS_89)
	@echo '   [Compile] $(BUILD)/obj/ejsType.o'
	$(CC) -c -o $(BUILD)/obj/ejsType.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsType.c

#
#   ejsUri.o
#
DEPS_90 += src/ejs.h

$(BUILD)/obj/ejsUri.o: \
    src/core/src/ejsUri.c $(DEPS_90)
	@echo '   [Compile] $(BUILD)/obj/ejsUri.o'
	$(CC) -c -o $(BUILD)/obj/ejsUri.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsUri.c

#
#   ejsVoid.o
#
DEPS_91 += src/ejs.h

$(BUILD)/obj/ejsVoid.o: \
    src/core/src/ejsVoid.c $(DEPS_91)
	@echo '   [Compile] $(BUILD)/obj/ejsVoid.o'
	$(CC) -c -o $(BUILD)/obj/ejsVoid.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsVoid.c

#
#   ejsWeb.o
#
DEPS_92 += $(BUILD)/inc/me.h
DEPS_92 += $(BUILD)/inc/ejs.h
DEPS_92 += $(BUILD)/inc/ejsCompiler.h
DEPS_92 += src/ejs.web/ejsWeb.h
DEPS_92 += $(BUILD)/inc/ejs.web.slots.h

$(BUILD)/obj/ejsWeb.o: \
    src/ejs.web/ejsWeb.c $(DEPS_92)
	@echo '   [Compile] $(BUILD)/obj/ejsWeb.o'
	$(CC) -c -o $(BUILD)/obj/ejsWeb.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/ejs.web/ejsWeb.c

#
#   ejsWebSocket.o
#
DEPS_93 += src/ejs.h

$(BUILD)/obj/ejsWebSocket.o: \
    src/core/src/ejsWebSocket.c $(DEPS_93)
	@echo '   [Compile] $(BUILD)/obj/ejsWebSocket.o'
	$(CC) -c -o $(BUILD)/obj/ejsWebSocket.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsWebSocket.c

#
#   ejsWorker.o
#
DEPS_94 += src/ejs.h

$(BUILD)/obj/ejsWorker.o: \
    src/core/src/ejsWorker.c $(DEPS_94)
	@echo '   [Compile] $(BUILD)/obj/ejsWorker.o'
	$(CC) -c -o $(BUILD)/obj/ejsWorker.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsWorker.c

#
#   ejsXML.o
#
DEPS_95 += src/ejs.h

$(BUILD)/obj/ejsXML.o: \
    src/core/src/ejsXML.c $(DEPS_95)
	@echo '   [Compile] $(BUILD)/obj/ejsXML.o'
	$(CC) -c -o $(BUILD)/obj/ejsXML.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsXML.c

#
#   ejsXMLList.o
#
DEPS_96 += src/ejs.h

$(BUILD)/obj/ejsXMLList.o: \
    src/core/src/ejsXMLList.c $(DEPS_96)
	@echo '   [Compile] $(BUILD)/obj/ejsXMLList.o'
	$(CC) -c -o $(BUILD)/obj/ejsXMLList.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsXMLList.c

#
#   ejsXMLLoader.o
#
DEPS_97 += src/ejs.h

$(BUILD)/obj/ejsXMLLoader.o: \
    src/core/src/ejsXMLLoader.c $(DEPS_97)
	@echo '   [Compile] $(BUILD)/obj/ejsXMLLoader.o'
	$(CC) -c -o $(BUILD)/obj/ejsXMLLoader.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/core/src/ejsXMLLoader.c

#
#   ejsZlib.o
#
DEPS_98 += $(BUILD)/inc/ejs.h
DEPS_98 += $(BUILD)/inc/zlib.h
DEPS_98 += $(BUILD)/inc/ejs.zlib.slots.h

$(BUILD)/obj/ejsZlib.o: \
    src/ejs.zlib/ejsZlib.c $(DEPS_98)
	@echo '   [Compile] $(BUILD)/obj/ejsZlib.o'
	$(CC) -c -o $(BUILD)/obj/ejsZlib.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/ejs.zlib/ejsZlib.c

#
#   ejsc.o
#
DEPS_99 += $(BUILD)/inc/ejsCompiler.h

$(BUILD)/obj/ejsc.o: \
    src/cmd/ejsc.c $(DEPS_99)
	@echo '   [Compile] $(BUILD)/obj/ejsc.o'
	$(CC) -c -o $(BUILD)/obj/ejsc.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/cmd/ejsc.c

#
#   ejsmod.o
#
DEPS_100 += src/cmd/ejsmod.h

$(BUILD)/obj/ejsmod.o: \
    src/cmd/ejsmod.c $(DEPS_100)
	@echo '   [Compile] $(BUILD)/obj/ejsmod.o'
	$(CC) -c -o $(BUILD)/obj/ejsmod.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/cmd/ejsmod.c

#
#   ejsrun.o
#
DEPS_101 += $(BUILD)/inc/ejsCompiler.h

$(BUILD)/obj/ejsrun.o: \
    src/cmd/ejsrun.c $(DEPS_101)
	@echo '   [Compile] $(BUILD)/obj/ejsrun.o'
	$(CC) -c -o $(BUILD)/obj/ejsrun.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/cmd/ejsrun.c

#
#   http.h
#

src/http/http.h: $(DEPS_102)

#
#   http.o
#
DEPS_103 += src/http/http.h

$(BUILD)/obj/http.o: \
    src/http/http.c $(DEPS_103)
	@echo '   [Compile] $(BUILD)/obj/http.o'
	$(CC) -c -o $(BUILD)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/http/http.c

#
#   httpLib.o
#
DEPS_104 += src/http/http.h
DEPS_104 += $(BUILD)/inc/pcre.h

$(BUILD)/obj/httpLib.o: \
    src/http/httpLib.c $(DEPS_104)
	@echo '   [Compile] $(BUILD)/obj/httpLib.o'
	$(CC) -c -o $(BUILD)/obj/httpLib.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/http/httpLib.c

#
#   listing.o
#
DEPS_105 += src/cmd/ejsmod.h
DEPS_105 += $(BUILD)/inc/ejsByteCodeTable.h

$(BUILD)/obj/listing.o: \
    src/cmd/listing.c $(DEPS_105)
	@echo '   [Compile] $(BUILD)/obj/listing.o'
	$(CC) -c -o $(BUILD)/obj/listing.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/cmd/listing.c

#
#   mbedtls.h
#

src/mbedtls/mbedtls.h: $(DEPS_106)

#
#   mbedtls.o
#
DEPS_107 += src/mbedtls/mbedtls.h

$(BUILD)/obj/mbedtls.o: \
    src/mbedtls/mbedtls.c $(DEPS_107)
	@echo '   [Compile] $(BUILD)/obj/mbedtls.o'
	$(CC) -c -o $(BUILD)/obj/mbedtls.o $(CFLAGS) $(DFLAGS) -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/mbedtls/mbedtls.c

#
#   mpr-mbedtls.o
#
DEPS_108 += $(BUILD)/inc/mpr.h

$(BUILD)/obj/mpr-mbedtls.o: \
    src/mpr-mbedtls/mpr-mbedtls.c $(DEPS_108)
	@echo '   [Compile] $(BUILD)/obj/mpr-mbedtls.o'
	$(CC) -c -o $(BUILD)/obj/mpr-mbedtls.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/mpr-mbedtls/mpr-mbedtls.c

#
#   mpr-openssl.o
#
DEPS_109 += $(BUILD)/inc/mpr.h

$(BUILD)/obj/mpr-openssl.o: \
    src/mpr-openssl/mpr-openssl.c $(DEPS_109)
	@echo '   [Compile] $(BUILD)/obj/mpr-openssl.o'
	$(CC) -c -o $(BUILD)/obj/mpr-openssl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/mpr-openssl/mpr-openssl.c

#
#   mpr.h
#

src/mpr/mpr.h: $(DEPS_110)

#
#   mprLib.o
#
DEPS_111 += src/mpr/mpr.h

$(BUILD)/obj/mprLib.o: \
    src/mpr/mprLib.c $(DEPS_111)
	@echo '   [Compile] $(BUILD)/obj/mprLib.o'
	$(CC) -c -o $(BUILD)/obj/mprLib.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/mpr/mprLib.c

#
#   pcre.h
#

src/pcre/pcre.h: $(DEPS_112)

#
#   pcre.o
#
DEPS_113 += $(BUILD)/inc/me.h
DEPS_113 += src/pcre/pcre.h

$(BUILD)/obj/pcre.o: \
    src/pcre/pcre.c $(DEPS_113)
	@echo '   [Compile] $(BUILD)/obj/pcre.o'
	$(CC) -c -o $(BUILD)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/pcre/pcre.c

#
#   slotGen.o
#
DEPS_114 += src/cmd/ejsmod.h

$(BUILD)/obj/slotGen.o: \
    src/cmd/slotGen.c $(DEPS_114)
	@echo '   [Compile] $(BUILD)/obj/slotGen.o'
	$(CC) -c -o $(BUILD)/obj/slotGen.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/cmd/slotGen.c

#
#   sqlite3.h
#

src/sqlite/sqlite3.h: $(DEPS_115)

#
#   sqlite.o
#
DEPS_116 += $(BUILD)/inc/me.h
DEPS_116 += src/sqlite/sqlite3.h

$(BUILD)/obj/sqlite.o: \
    src/sqlite/sqlite.c $(DEPS_116)
	@echo '   [Compile] $(BUILD)/obj/sqlite.o'
	$(CC) -c -o $(BUILD)/obj/sqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/sqlite/sqlite.c

#
#   sqlite3.o
#
DEPS_117 += $(BUILD)/inc/me.h
DEPS_117 += src/sqlite/sqlite3.h

$(BUILD)/obj/sqlite3.o: \
    src/sqlite/sqlite3.c $(DEPS_117)
	@echo '   [Compile] $(BUILD)/obj/sqlite3.o'
	$(CC) -c -o $(BUILD)/obj/sqlite3.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/sqlite/sqlite3.c

#
#   watchdog.o
#
DEPS_118 += $(BUILD)/inc/mpr.h

$(BUILD)/obj/watchdog.o: \
    src/watchdog/watchdog.c $(DEPS_118)
	@echo '   [Compile] $(BUILD)/obj/watchdog.o'
	$(CC) -c -o $(BUILD)/obj/watchdog.o $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/watchdog/watchdog.c

#
#   zlib.h
#

src/zlib/zlib.h: $(DEPS_119)

#
#   zlib.o
#
DEPS_120 += $(BUILD)/inc/me.h
DEPS_120 += src/zlib/zlib.h

$(BUILD)/obj/zlib.o: \
    src/zlib/zlib.c $(DEPS_120)
	@echo '   [Compile] $(BUILD)/obj/zlib.o'
	$(CC) -c -o $(BUILD)/obj/zlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/zlib/zlib.c

#
#   slots
#

slots: $(DEPS_121)

ifeq ($(ME_COM_MBEDTLS),1)
#
#   libmbedtls
#
DEPS_122 += $(BUILD)/inc/osdep.h
DEPS_122 += $(BUILD)/inc/embedtls.h
DEPS_122 += $(BUILD)/inc/mbedtls-config.h
DEPS_122 += $(BUILD)/inc/mbedtls.h
DEPS_122 += $(BUILD)/obj/mbedtls.o

$(BUILD)/bin/libmbedtls.a: $(DEPS_122)
	@echo '      [Link] $(BUILD)/bin/libmbedtls.a'
	ar -cr $(BUILD)/bin/libmbedtls.a "$(BUILD)/obj/mbedtls.o"
endif

ifeq ($(ME_COM_MBEDTLS),1)
#
#   libmpr-mbedtls
#
DEPS_123 += $(BUILD)/bin/libmbedtls.a
DEPS_123 += $(BUILD)/obj/mpr-mbedtls.o

$(BUILD)/bin/libmpr-mbedtls.a: $(DEPS_123)
	@echo '      [Link] $(BUILD)/bin/libmpr-mbedtls.a'
	ar -cr $(BUILD)/bin/libmpr-mbedtls.a "$(BUILD)/obj/mpr-mbedtls.o"
endif

ifeq ($(ME_COM_OPENSSL),1)
#
#   libmpr-openssl
#
DEPS_124 += $(BUILD)/obj/mpr-openssl.o

$(BUILD)/bin/libmpr-openssl.a: $(DEPS_124)
	@echo '      [Link] $(BUILD)/bin/libmpr-openssl.a'
	ar -cr $(BUILD)/bin/libmpr-openssl.a "$(BUILD)/obj/mpr-openssl.o"
endif

#
#   libmpr
#
DEPS_125 += $(BUILD)/inc/osdep.h
ifeq ($(ME_COM_MBEDTLS),1)
    DEPS_125 += $(BUILD)/bin/libmpr-mbedtls.a
endif
ifeq ($(ME_COM_MBEDTLS),1)
    DEPS_125 += $(BUILD)/bin/libmbedtls.a
endif
ifeq ($(ME_COM_OPENSSL),1)
    DEPS_125 += $(BUILD)/bin/libmpr-openssl.a
endif
DEPS_125 += $(BUILD)/inc/mpr.h
DEPS_125 += $(BUILD)/obj/mprLib.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_125 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_125 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_125 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_125 += -lmpr-openssl
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_125 += -lmpr-mbedtls
endif

$(BUILD)/bin/libmpr.so: $(DEPS_125)
	@echo '      [Link] $(BUILD)/bin/libmpr.so'
	$(CC) -shared -o $(BUILD)/bin/libmpr.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/mprLib.o" $(LIBPATHS_125) $(LIBS_125) $(LIBS_125) $(LIBS) 

ifeq ($(ME_COM_PCRE),1)
#
#   libpcre
#
DEPS_126 += $(BUILD)/inc/pcre.h
DEPS_126 += $(BUILD)/obj/pcre.o

$(BUILD)/bin/libpcre.so: $(DEPS_126)
	@echo '      [Link] $(BUILD)/bin/libpcre.so'
	$(CC) -shared -o $(BUILD)/bin/libpcre.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/pcre.o" $(LIBS) 
endif

ifeq ($(ME_COM_HTTP),1)
#
#   libhttp
#
DEPS_127 += $(BUILD)/bin/libmpr.so
ifeq ($(ME_COM_PCRE),1)
    DEPS_127 += $(BUILD)/bin/libpcre.so
endif
DEPS_127 += $(BUILD)/inc/http.h
DEPS_127 += $(BUILD)/obj/httpLib.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_127 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_127 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_127 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_127 += -lmpr-openssl
endif
LIBS_127 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_127 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_127 += -lpcre
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_127 += -lpcre
endif
LIBS_127 += -lmpr

$(BUILD)/bin/libhttp.so: $(DEPS_127)
	@echo '      [Link] $(BUILD)/bin/libhttp.so'
	$(CC) -shared -o $(BUILD)/bin/libhttp.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/httpLib.o" $(LIBPATHS_127) $(LIBS_127) $(LIBS_127) $(LIBS) 
endif

#
#   libejs
#
DEPS_128 += slots
ifeq ($(ME_COM_HTTP),1)
    DEPS_128 += $(BUILD)/bin/libhttp.so
endif
DEPS_128 += $(BUILD)/inc/ejs.cache.local.slots.h
DEPS_128 += $(BUILD)/inc/ejs.db.sqlite.slots.h
DEPS_128 += $(BUILD)/inc/ejs.slots.h
DEPS_128 += $(BUILD)/inc/ejs.web.slots.h
DEPS_128 += $(BUILD)/inc/ejs.zlib.slots.h
DEPS_128 += $(BUILD)/inc/ejs.h
DEPS_128 += $(BUILD)/inc/ejsByteCode.h
DEPS_128 += $(BUILD)/inc/ejsByteCodeTable.h
DEPS_128 += $(BUILD)/inc/ejsCompiler.h
DEPS_128 += $(BUILD)/inc/ejsCustomize.h
DEPS_128 += $(BUILD)/obj/ecAst.o
DEPS_128 += $(BUILD)/obj/ecCodeGen.o
DEPS_128 += $(BUILD)/obj/ecCompiler.o
DEPS_128 += $(BUILD)/obj/ecLex.o
DEPS_128 += $(BUILD)/obj/ecModuleWrite.o
DEPS_128 += $(BUILD)/obj/ecParser.o
DEPS_128 += $(BUILD)/obj/ecState.o
DEPS_128 += $(BUILD)/obj/dtoa.o
DEPS_128 += $(BUILD)/obj/ejsApp.o
DEPS_128 += $(BUILD)/obj/ejsArray.o
DEPS_128 += $(BUILD)/obj/ejsBlock.o
DEPS_128 += $(BUILD)/obj/ejsBoolean.o
DEPS_128 += $(BUILD)/obj/ejsByteArray.o
DEPS_128 += $(BUILD)/obj/ejsCache.o
DEPS_128 += $(BUILD)/obj/ejsCmd.o
DEPS_128 += $(BUILD)/obj/ejsConfig.o
DEPS_128 += $(BUILD)/obj/ejsDate.o
DEPS_128 += $(BUILD)/obj/ejsDebug.o
DEPS_128 += $(BUILD)/obj/ejsError.o
DEPS_128 += $(BUILD)/obj/ejsFile.o
DEPS_128 += $(BUILD)/obj/ejsFileSystem.o
DEPS_128 += $(BUILD)/obj/ejsFrame.o
DEPS_128 += $(BUILD)/obj/ejsFunction.o
DEPS_128 += $(BUILD)/obj/ejsGC.o
DEPS_128 += $(BUILD)/obj/ejsGlobal.o
DEPS_128 += $(BUILD)/obj/ejsHttp.o
DEPS_128 += $(BUILD)/obj/ejsIterator.o
DEPS_128 += $(BUILD)/obj/ejsJSON.o
DEPS_128 += $(BUILD)/obj/ejsLocalCache.o
DEPS_128 += $(BUILD)/obj/ejsMath.o
DEPS_128 += $(BUILD)/obj/ejsMemory.o
DEPS_128 += $(BUILD)/obj/ejsMprLog.o
DEPS_128 += $(BUILD)/obj/ejsNamespace.o
DEPS_128 += $(BUILD)/obj/ejsNull.o
DEPS_128 += $(BUILD)/obj/ejsNumber.o
DEPS_128 += $(BUILD)/obj/ejsObject.o
DEPS_128 += $(BUILD)/obj/ejsPath.o
DEPS_128 += $(BUILD)/obj/ejsPot.o
DEPS_128 += $(BUILD)/obj/ejsRegExp.o
DEPS_128 += $(BUILD)/obj/ejsSocket.o
DEPS_128 += $(BUILD)/obj/ejsString.o
DEPS_128 += $(BUILD)/obj/ejsSystem.o
DEPS_128 += $(BUILD)/obj/ejsTimer.o
DEPS_128 += $(BUILD)/obj/ejsType.o
DEPS_128 += $(BUILD)/obj/ejsUri.o
DEPS_128 += $(BUILD)/obj/ejsVoid.o
DEPS_128 += $(BUILD)/obj/ejsWebSocket.o
DEPS_128 += $(BUILD)/obj/ejsWorker.o
DEPS_128 += $(BUILD)/obj/ejsXML.o
DEPS_128 += $(BUILD)/obj/ejsXMLList.o
DEPS_128 += $(BUILD)/obj/ejsXMLLoader.o
DEPS_128 += $(BUILD)/obj/ejsByteCode.o
DEPS_128 += $(BUILD)/obj/ejsException.o
DEPS_128 += $(BUILD)/obj/ejsHelper.o
DEPS_128 += $(BUILD)/obj/ejsInterp.o
DEPS_128 += $(BUILD)/obj/ejsLoader.o
DEPS_128 += $(BUILD)/obj/ejsModule.o
DEPS_128 += $(BUILD)/obj/ejsScope.o
DEPS_128 += $(BUILD)/obj/ejsService.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_128 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_128 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_128 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_128 += -lmpr-openssl
endif
LIBS_128 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_128 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_128 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_128 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_128 += -lpcre
endif
LIBS_128 += -lmpr
ifeq ($(ME_COM_HTTP),1)
    LIBS_128 += -lhttp
endif

$(BUILD)/bin/libejs.so: $(DEPS_128)
	@echo '      [Link] $(BUILD)/bin/libejs.so'
	$(CC) -shared -o $(BUILD)/bin/libejs.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ecAst.o" "$(BUILD)/obj/ecCodeGen.o" "$(BUILD)/obj/ecCompiler.o" "$(BUILD)/obj/ecLex.o" "$(BUILD)/obj/ecModuleWrite.o" "$(BUILD)/obj/ecParser.o" "$(BUILD)/obj/ecState.o" "$(BUILD)/obj/dtoa.o" "$(BUILD)/obj/ejsApp.o" "$(BUILD)/obj/ejsArray.o" "$(BUILD)/obj/ejsBlock.o" "$(BUILD)/obj/ejsBoolean.o" "$(BUILD)/obj/ejsByteArray.o" "$(BUILD)/obj/ejsCache.o" "$(BUILD)/obj/ejsCmd.o" "$(BUILD)/obj/ejsConfig.o" "$(BUILD)/obj/ejsDate.o" "$(BUILD)/obj/ejsDebug.o" "$(BUILD)/obj/ejsError.o" "$(BUILD)/obj/ejsFile.o" "$(BUILD)/obj/ejsFileSystem.o" "$(BUILD)/obj/ejsFrame.o" "$(BUILD)/obj/ejsFunction.o" "$(BUILD)/obj/ejsGC.o" "$(BUILD)/obj/ejsGlobal.o" "$(BUILD)/obj/ejsHttp.o" "$(BUILD)/obj/ejsIterator.o" "$(BUILD)/obj/ejsJSON.o" "$(BUILD)/obj/ejsLocalCache.o" "$(BUILD)/obj/ejsMath.o" "$(BUILD)/obj/ejsMemory.o" "$(BUILD)/obj/ejsMprLog.o" "$(BUILD)/obj/ejsNamespace.o" "$(BUILD)/obj/ejsNull.o" "$(BUILD)/obj/ejsNumber.o" "$(BUILD)/obj/ejsObject.o" "$(BUILD)/obj/ejsPath.o" "$(BUILD)/obj/ejsPot.o" "$(BUILD)/obj/ejsRegExp.o" "$(BUILD)/obj/ejsSocket.o" "$(BUILD)/obj/ejsString.o" "$(BUILD)/obj/ejsSystem.o" "$(BUILD)/obj/ejsTimer.o" "$(BUILD)/obj/ejsType.o" "$(BUILD)/obj/ejsUri.o" "$(BUILD)/obj/ejsVoid.o" "$(BUILD)/obj/ejsWebSocket.o" "$(BUILD)/obj/ejsWorker.o" "$(BUILD)/obj/ejsXML.o" "$(BUILD)/obj/ejsXMLList.o" "$(BUILD)/obj/ejsXMLLoader.o" "$(BUILD)/obj/ejsByteCode.o" "$(BUILD)/obj/ejsException.o" "$(BUILD)/obj/ejsHelper.o" "$(BUILD)/obj/ejsInterp.o" "$(BUILD)/obj/ejsLoader.o" "$(BUILD)/obj/ejsModule.o" "$(BUILD)/obj/ejsScope.o" "$(BUILD)/obj/ejsService.o" $(LIBPATHS_128) $(LIBS_128) $(LIBS_128) $(LIBS) 

#
#   ejs
#
DEPS_129 += $(BUILD)/bin/libejs.so
DEPS_129 += $(BUILD)/obj/ejs.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_129 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_129 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_129 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_129 += -lmpr-openssl
endif
LIBS_129 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_129 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_129 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_129 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_129 += -lpcre
endif
LIBS_129 += -lmpr
LIBS_129 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_129 += -lhttp
endif

$(BUILD)/bin/ejs: $(DEPS_129)
	@echo '      [Link] $(BUILD)/bin/ejs'
	$(CC) -o $(BUILD)/bin/ejs $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejs.o" $(LIBPATHS_129) $(LIBS_129) $(LIBS_129) $(LIBS) $(LIBS) 

#
#   ejsc
#
DEPS_130 += $(BUILD)/bin/libejs.so
DEPS_130 += $(BUILD)/obj/ejsc.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_130 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_130 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_130 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_130 += -lmpr-openssl
endif
LIBS_130 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_130 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_130 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_130 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_130 += -lpcre
endif
LIBS_130 += -lmpr
LIBS_130 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_130 += -lhttp
endif

$(BUILD)/bin/ejsc: $(DEPS_130)
	@echo '      [Link] $(BUILD)/bin/ejsc'
	$(CC) -o $(BUILD)/bin/ejsc $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsc.o" $(LIBPATHS_130) $(LIBS_130) $(LIBS_130) $(LIBS) $(LIBS) 

#
#   ejsmod
#
DEPS_131 += $(BUILD)/bin/libejs.so
DEPS_131 += $(BUILD)/inc/ejsmod.h
DEPS_131 += $(BUILD)/obj/ejsmod.o
DEPS_131 += $(BUILD)/obj/doc.o
DEPS_131 += $(BUILD)/obj/docFiles.o
DEPS_131 += $(BUILD)/obj/listing.o
DEPS_131 += $(BUILD)/obj/slotGen.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_131 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_131 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_131 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_131 += -lmpr-openssl
endif
LIBS_131 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_131 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_131 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_131 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_131 += -lpcre
endif
LIBS_131 += -lmpr
LIBS_131 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_131 += -lhttp
endif

$(BUILD)/bin/ejsmod: $(DEPS_131)
	@echo '      [Link] $(BUILD)/bin/ejsmod'
	$(CC) -o $(BUILD)/bin/ejsmod $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsmod.o" "$(BUILD)/obj/doc.o" "$(BUILD)/obj/docFiles.o" "$(BUILD)/obj/listing.o" "$(BUILD)/obj/slotGen.o" $(LIBPATHS_131) $(LIBS_131) $(LIBS_131) $(LIBS) $(LIBS) 

#
#   ejs.mod
#
DEPS_132 += src/core/App.es
DEPS_132 += src/core/Args.es
DEPS_132 += src/core/Array.es
DEPS_132 += src/core/BinaryStream.es
DEPS_132 += src/core/Block.es
DEPS_132 += src/core/Boolean.es
DEPS_132 += src/core/ByteArray.es
DEPS_132 += src/core/Cache.es
DEPS_132 += src/core/Cmd.es
DEPS_132 += src/core/Compat.es
DEPS_132 += src/core/Config.es
DEPS_132 += src/core/Date.es
DEPS_132 += src/core/Debug.es
DEPS_132 += src/core/Emitter.es
DEPS_132 += src/core/Error.es
DEPS_132 += src/core/File.es
DEPS_132 += src/core/FileSystem.es
DEPS_132 += src/core/Frame.es
DEPS_132 += src/core/Function.es
DEPS_132 += src/core/GC.es
DEPS_132 += src/core/Global.es
DEPS_132 += src/core/Http.es
DEPS_132 += src/core/Inflector.es
DEPS_132 += src/core/Iterator.es
DEPS_132 += src/core/JSON.es
DEPS_132 += src/core/Loader.es
DEPS_132 += src/core/LocalCache.es
DEPS_132 += src/core/Locale.es
DEPS_132 += src/core/Logger.es
DEPS_132 += src/core/Math.es
DEPS_132 += src/core/Memory.es
DEPS_132 += src/core/MprLog.es
DEPS_132 += src/core/Name.es
DEPS_132 += src/core/Namespace.es
DEPS_132 += src/core/Null.es
DEPS_132 += src/core/Number.es
DEPS_132 += src/core/Object.es
DEPS_132 += src/core/Path.es
DEPS_132 += src/core/Promise.es
DEPS_132 += src/core/RegExp.es
DEPS_132 += src/core/Socket.es
DEPS_132 += src/core/Stream.es
DEPS_132 += src/core/String.es
DEPS_132 += src/core/System.es
DEPS_132 += src/core/TextStream.es
DEPS_132 += src/core/Timer.es
DEPS_132 += src/core/Type.es
DEPS_132 += src/core/Uri.es
DEPS_132 += src/core/Void.es
DEPS_132 += src/core/WebSocket.es
DEPS_132 += src/core/Worker.es
DEPS_132 += src/core/XML.es
DEPS_132 += src/core/XMLHttp.es
DEPS_132 += src/core/XMLList.es
DEPS_132 += $(BUILD)/bin/ejsc
DEPS_132 += $(BUILD)/bin/ejsmod

$(BUILD)/bin/ejs.mod: $(DEPS_132)
	( \
	cd src/core; \
	echo '   [Compile] Core EJS classes' ; \
	"../../$(BUILD)/bin/ejsc" --out "../../$(BUILD)/bin/ejs.mod"  --optimize 9 --bind --require null App.es Args.es Array.es BinaryStream.es Block.es Boolean.es ByteArray.es Cache.es Cmd.es Compat.es Config.es Date.es Debug.es Emitter.es Error.es File.es FileSystem.es Frame.es Function.es GC.es Global.es Http.es Inflector.es Iterator.es JSON.es Loader.es LocalCache.es Locale.es Logger.es Math.es Memory.es MprLog.es Name.es Namespace.es Null.es Number.es Object.es Path.es Promise.es RegExp.es Socket.es Stream.es String.es System.es TextStream.es Timer.es Type.es Uri.es Void.es WebSocket.es Worker.es XML.es XMLHttp.es XMLList.es ; \
	"../../$(BUILD)/bin/ejsmod" --cslots --dir "../../$(BUILD)/inc" --require null "../../$(BUILD)/bin/ejs.mod" ; \
	)

#
#   ejs.db.mod
#
DEPS_133 += src/ejs.db/Database.es
DEPS_133 += src/ejs.db/DatabaseConnector.es
DEPS_133 += $(BUILD)/bin/ejsc
DEPS_133 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.db.mod: $(DEPS_133)
	( \
	cd src/ejs.db; \
	echo '   [Compile] ejs.db.mod' ; \
	"../../$(BUILD)/bin/ejsc" --out "../../$(BUILD)/bin/ejs.db.mod"  --optimize 9 Database.es DatabaseConnector.es ; \
	)

#
#   ejs.db.mapper.mod
#
DEPS_134 += src/ejs.db.mapper/Record.es
DEPS_134 += $(BUILD)/bin/ejsc
DEPS_134 += $(BUILD)/bin/ejs.mod
DEPS_134 += $(BUILD)/bin/ejs.db.mod

$(BUILD)/bin/ejs.db.mapper.mod: $(DEPS_134)
	( \
	cd src/ejs.db.mapper; \
	echo '   [Compile] ejs.db.mapper.mod' ; \
	"../../$(BUILD)/bin/ejsc" --out "../../$(BUILD)/bin/ejs.db.mapper.mod"  --optimize 9 Record.es ; \
	)

#
#   ejs.db.sqlite.mod
#
DEPS_135 += src/ejs.db.sqlite/Sqlite.es
DEPS_135 += $(BUILD)/bin/ejsc
DEPS_135 += $(BUILD)/bin/ejsmod
DEPS_135 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.db.sqlite.mod: $(DEPS_135)
	( \
	cd src/ejs.db.sqlite; \
	echo '   [Compile] ejs.db.sqlite.mod' ; \
	"../../$(BUILD)/bin/ejsc" --out "../../$(BUILD)/bin/ejs.db.sqlite.mod"  --optimize 9 Sqlite.es ; \
	"../../$(BUILD)/bin/ejsmod" --cslots --dir "../../$(BUILD)/bin" "../../$(BUILD)/bin/ejs.db.sqlite.mod" ; \
	)

#
#   ejs.mail.mod
#
DEPS_136 += src/ejs.mail/Mail.es
DEPS_136 += $(BUILD)/bin/ejsc
DEPS_136 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.mail.mod: $(DEPS_136)
	( \
	cd src/ejs.mail; \
	"../../$(BUILD)/bin/ejsc" --out "../../$(BUILD)/bin/ejs.mail.mod"  --optimize 9 Mail.es ; \
	)

#
#   ejs.web.mod
#
DEPS_137 += src/ejs.web/Cascade.es
DEPS_137 += src/ejs.web/CommonLog.es
DEPS_137 += src/ejs.web/ContentType.es
DEPS_137 += src/ejs.web/Controller.es
DEPS_137 += src/ejs.web/Dir.es
DEPS_137 += src/ejs.web/Google.es
DEPS_137 += src/ejs.web/Head.es
DEPS_137 += src/ejs.web/Html.es
DEPS_137 += src/ejs.web/HttpServer.es
DEPS_137 += src/ejs.web/MethodOverride.es
DEPS_137 += src/ejs.web/Middleware.es
DEPS_137 += src/ejs.web/Mvc.es
DEPS_137 += src/ejs.web/Request.es
DEPS_137 += src/ejs.web/Router.es
DEPS_137 += src/ejs.web/Script.es
DEPS_137 += src/ejs.web/Session.es
DEPS_137 += src/ejs.web/ShowExceptions.es
DEPS_137 += src/ejs.web/Static.es
DEPS_137 += src/ejs.web/Template.es
DEPS_137 += src/ejs.web/UploadFile.es
DEPS_137 += src/ejs.web/UrlMap.es
DEPS_137 += src/ejs.web/Utils.es
DEPS_137 += src/ejs.web/View.es
DEPS_137 += $(BUILD)/bin/ejsc
DEPS_137 += $(BUILD)/bin/ejsmod
DEPS_137 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.web.mod: $(DEPS_137)
	( \
	cd src/ejs.web; \
	echo '   [Compile] ejs.web.mod' ; \
	"../../$(BUILD)/bin/ejsc" --out "../../$(BUILD)/bin/ejs.web.mod"  --optimize 9 Cascade.es CommonLog.es ContentType.es Controller.es Dir.es Google.es Head.es Html.es HttpServer.es MethodOverride.es Middleware.es Mvc.es Request.es Router.es Script.es Session.es ShowExceptions.es Static.es Template.es UploadFile.es UrlMap.es Utils.es View.es ; \
	"../../$(BUILD)/bin/ejsmod" --cslots --dir "../../$(BUILD)/bin" "../../$(BUILD)/bin/ejs.web.mod" ; \
	)

#
#   ejs.template.mod
#
DEPS_138 += src/ejs.template/TemplateParser.es
DEPS_138 += $(BUILD)/bin/ejsc
DEPS_138 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.template.mod: $(DEPS_138)
	( \
	cd src/ejs.template; \
	echo '   [Compile] ejs.template.mod' ; \
	"../../$(BUILD)/bin/ejsc" --out "../../$(BUILD)/bin/ejs.template.mod"  --optimize 9 TemplateParser.es ; \
	)

#
#   ejs.unix.mod
#
DEPS_139 += src/ejs.unix/Unix.es
DEPS_139 += $(BUILD)/bin/ejsc
DEPS_139 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.unix.mod: $(DEPS_139)
	( \
	cd src/ejs.unix; \
	echo '   [Compile] ejs.unix.mod' ; \
	"../../$(BUILD)/bin/ejsc" --out "../../$(BUILD)/bin/ejs.unix.mod"  --optimize 9 Unix.es ; \
	)

#
#   ejs.mvc.mod
#
DEPS_140 += src/ejs.mvc/mvc.es
DEPS_140 += $(BUILD)/bin/ejsc
DEPS_140 += $(BUILD)/bin/ejs.mod
DEPS_140 += $(BUILD)/bin/ejs.web.mod
DEPS_140 += $(BUILD)/bin/ejs.template.mod
DEPS_140 += $(BUILD)/bin/ejs.unix.mod

$(BUILD)/bin/ejs.mvc.mod: $(DEPS_140)
	( \
	cd src/ejs.mvc; \
	echo '   [Compile] ejs.mvc.mod' ; \
	"../../$(BUILD)/bin/ejsc" --out "../../$(BUILD)/bin/ejs.mvc.mod"  --optimize 9 mvc.es ; \
	)

ifeq ($(ME_COM_ZLIB),1)
#
#   libzlib
#
DEPS_141 += $(BUILD)/inc/zlib.h
DEPS_141 += $(BUILD)/obj/zlib.o

$(BUILD)/bin/libzlib.so: $(DEPS_141)
	@echo '      [Link] $(BUILD)/bin/libzlib.so'
	$(CC) -shared -o $(BUILD)/bin/libzlib.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/zlib.o" $(LIBS) 
endif

#
#   libejs.zlib
#
DEPS_142 += $(BUILD)/bin/libejs.so
DEPS_142 += $(BUILD)/bin/ejs.mod
ifeq ($(ME_COM_ZLIB),1)
    DEPS_142 += $(BUILD)/bin/libzlib.so
endif
DEPS_142 += $(BUILD)/obj/ejsZlib.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_142 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_142 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_142 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_142 += -lmpr-openssl
endif
LIBS_142 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_142 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_142 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_142 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_142 += -lpcre
endif
LIBS_142 += -lmpr
LIBS_142 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_142 += -lhttp
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_142 += -lzlib
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_142 += -lzlib
endif
LIBS_142 += -lejs

$(BUILD)/bin/libejs.zlib.so: $(DEPS_142)
	@echo '      [Link] $(BUILD)/bin/libejs.zlib.so'
	$(CC) -shared -o $(BUILD)/bin/libejs.zlib.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsZlib.o" $(LIBPATHS_142) $(LIBS_142) $(LIBS_142) $(LIBS) 

#
#   ejs.zlib.mod
#
DEPS_143 += src/ejs.zlib/Zlib.es
DEPS_143 += $(BUILD)/bin/ejsc
DEPS_143 += $(BUILD)/bin/ejs.mod
DEPS_143 += $(BUILD)/bin/libejs.zlib.so

$(BUILD)/bin/ejs.zlib.mod: $(DEPS_143)
	( \
	cd src/ejs.zlib; \
	echo '   [Compile] ejs.zlib.mod' ; \
	"../../$(BUILD)/bin/ejsc" --out "../../$(BUILD)/bin/ejs.zlib.mod"  --optimize 9 Zlib.es ; \
	)

#
#   ejs.tar.mod
#
DEPS_144 += src/ejs.tar/Tar.es
DEPS_144 += $(BUILD)/bin/ejsc
DEPS_144 += $(BUILD)/bin/ejs.mod
DEPS_144 += $(BUILD)/bin/ejs.zlib.mod
DEPS_144 += $(BUILD)/bin/libejs.zlib.so

$(BUILD)/bin/ejs.tar.mod: $(DEPS_144)
	( \
	cd src/ejs.tar; \
	echo '   [Compile] ejs.tar.mod' ; \
	"../../$(BUILD)/bin/ejsc" --out "../../$(BUILD)/bin/ejs.tar.mod"  --optimize 9 Tar.es ; \
	)

#
#   ejsrun
#
DEPS_145 += $(BUILD)/bin/libejs.so
DEPS_145 += $(BUILD)/obj/ejsrun.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_145 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_145 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_145 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_145 += -lmpr-openssl
endif
LIBS_145 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_145 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_145 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_145 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_145 += -lpcre
endif
LIBS_145 += -lmpr
LIBS_145 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_145 += -lhttp
endif

$(BUILD)/bin/ejsrun: $(DEPS_145)
	@echo '      [Link] $(BUILD)/bin/ejsrun'
	$(CC) -o $(BUILD)/bin/ejsrun $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsrun.o" $(LIBPATHS_145) $(LIBS_145) $(LIBS_145) $(LIBS) $(LIBS) 

ifeq ($(ME_COM_HTTP),1)
#
#   httpcmd
#
DEPS_146 += $(BUILD)/bin/libhttp.so
DEPS_146 += $(BUILD)/obj/http.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_146 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_146 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_146 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_146 += -lmpr-openssl
endif
LIBS_146 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_146 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_146 += -lpcre
endif
LIBS_146 += -lhttp
ifeq ($(ME_COM_PCRE),1)
    LIBS_146 += -lpcre
endif
LIBS_146 += -lmpr

$(BUILD)/bin/http: $(DEPS_146)
	@echo '      [Link] $(BUILD)/bin/http'
	$(CC) -o $(BUILD)/bin/http $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/http.o" $(LIBPATHS_146) $(LIBS_146) $(LIBS_146) $(LIBS) $(LIBS) 
endif

#
#   install-certs
#
DEPS_147 += src/certs/samples/ca.crt
DEPS_147 += src/certs/samples/ca.key
DEPS_147 += src/certs/samples/ec.crt
DEPS_147 += src/certs/samples/ec.key
DEPS_147 += src/certs/samples/roots.crt
DEPS_147 += src/certs/samples/self.crt
DEPS_147 += src/certs/samples/self.key
DEPS_147 += src/certs/samples/test.crt
DEPS_147 += src/certs/samples/test.key

$(BUILD)/.install-certs-modified: $(DEPS_147)
	@echo '      [Copy] $(BUILD)/bin'
	mkdir -p "$(BUILD)/bin"
	cp src/certs/samples/ca.crt $(BUILD)/bin/ca.crt
	cp src/certs/samples/ca.key $(BUILD)/bin/ca.key
	cp src/certs/samples/ec.crt $(BUILD)/bin/ec.crt
	cp src/certs/samples/ec.key $(BUILD)/bin/ec.key
	cp src/certs/samples/roots.crt $(BUILD)/bin/roots.crt
	cp src/certs/samples/self.crt $(BUILD)/bin/self.crt
	cp src/certs/samples/self.key $(BUILD)/bin/self.key
	cp src/certs/samples/test.crt $(BUILD)/bin/test.crt
	cp src/certs/samples/test.key $(BUILD)/bin/test.key
	touch "$(BUILD)/.install-certs-modified"

ifeq ($(ME_COM_SQLITE),1)
#
#   libsql
#
DEPS_148 += $(BUILD)/inc/sqlite3.h
DEPS_148 += $(BUILD)/obj/sqlite3.o

$(BUILD)/bin/libsql.so: $(DEPS_148)
	@echo '      [Link] $(BUILD)/bin/libsql.so'
	$(CC) -shared -o $(BUILD)/bin/libsql.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/sqlite3.o" $(LIBS) 
endif

#
#   libejs.db.sqlite
#
DEPS_149 += $(BUILD)/bin/libmpr.so
DEPS_149 += $(BUILD)/bin/libejs.so
DEPS_149 += $(BUILD)/bin/ejs.mod
DEPS_149 += $(BUILD)/bin/ejs.db.sqlite.mod
ifeq ($(ME_COM_SQLITE),1)
    DEPS_149 += $(BUILD)/bin/libsql.so
endif
DEPS_149 += $(BUILD)/obj/ejsSqlite.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_149 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_149 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_149 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_149 += -lmpr-openssl
endif
LIBS_149 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_149 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_149 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_149 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_149 += -lpcre
endif
LIBS_149 += -lmpr
LIBS_149 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_149 += -lhttp
endif
ifeq ($(ME_COM_SQLITE),1)
    LIBS_149 += -lsql
endif
ifeq ($(ME_COM_SQLITE),1)
    LIBS_149 += -lsql
endif
LIBS_149 += -lejs

$(BUILD)/bin/libejs.db.sqlite.so: $(DEPS_149)
	@echo '      [Link] $(BUILD)/bin/libejs.db.sqlite.so'
	$(CC) -shared -o $(BUILD)/bin/libejs.db.sqlite.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsSqlite.o" $(LIBPATHS_149) $(LIBS_149) $(LIBS_149) $(LIBS) 

#
#   libejs.web
#
DEPS_150 += $(BUILD)/bin/libejs.so
DEPS_150 += $(BUILD)/bin/ejs.mod
DEPS_150 += $(BUILD)/inc/ejsWeb.h
DEPS_150 += $(BUILD)/obj/ejsHttpServer.o
DEPS_150 += $(BUILD)/obj/ejsRequest.o
DEPS_150 += $(BUILD)/obj/ejsSession.o
DEPS_150 += $(BUILD)/obj/ejsWeb.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_150 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_150 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_150 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_150 += -lmpr-openssl
endif
LIBS_150 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_150 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_150 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_150 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_150 += -lpcre
endif
LIBS_150 += -lmpr
LIBS_150 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_150 += -lhttp
endif
LIBS_150 += -lejs

$(BUILD)/bin/libejs.web.so: $(DEPS_150)
	@echo '      [Link] $(BUILD)/bin/libejs.web.so'
	$(CC) -shared -o $(BUILD)/bin/libejs.web.so $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsHttpServer.o" "$(BUILD)/obj/ejsRequest.o" "$(BUILD)/obj/ejsSession.o" "$(BUILD)/obj/ejsWeb.o" $(LIBPATHS_150) $(LIBS_150) $(LIBS_150) $(LIBS) 

#
#   mvc.es
#
DEPS_151 += src/ejs.mvc/mvc.es

$(BUILD)/bin/mvc.es: $(DEPS_151)
	@echo '      [Copy] $(BUILD)/bin/mvc.es'
	mkdir -p "$(BUILD)/bin"
	cp src/ejs.mvc/mvc.es $(BUILD)/bin/mvc.es

#
#   mvc
#
DEPS_152 += $(BUILD)/bin/libejs.so
DEPS_152 += $(BUILD)/bin/mvc.es
DEPS_152 += $(BUILD)/obj/ejsrun.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_152 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_152 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_152 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_152 += -lmpr-openssl
endif
LIBS_152 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_152 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_152 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_152 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_152 += -lpcre
endif
LIBS_152 += -lmpr
LIBS_152 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_152 += -lhttp
endif

$(BUILD)/bin/mvc: $(DEPS_152)
	@echo '      [Link] $(BUILD)/bin/mvc'
	$(CC) -o $(BUILD)/bin/mvc $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsrun.o" $(LIBPATHS_152) $(LIBS_152) $(LIBS_152) $(LIBS) $(LIBS) 

#
#   utest.es
#
DEPS_153 += src/ejs.utest/utest.es

$(BUILD)/bin/utest.es: $(DEPS_153)
	@echo '      [Copy] $(BUILD)/bin/utest.es'
	mkdir -p "$(BUILD)/bin"
	cp src/ejs.utest/utest.es $(BUILD)/bin/utest.es

#
#   utest.worker
#
DEPS_154 += src/ejs.utest/utest.worker

$(BUILD)/bin/utest.worker: $(DEPS_154)
	@echo '      [Copy] $(BUILD)/bin/utest.worker'
	mkdir -p "$(BUILD)/bin"
	cp src/ejs.utest/utest.worker $(BUILD)/bin/utest.worker

#
#   utest
#
DEPS_155 += $(BUILD)/bin/libejs.so
DEPS_155 += $(BUILD)/bin/utest.es
DEPS_155 += $(BUILD)/bin/utest.worker
DEPS_155 += $(BUILD)/obj/ejsrun.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_155 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_155 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_155 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_155 += -lmpr-openssl
endif
LIBS_155 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_155 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_155 += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    LIBS_155 += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    LIBS_155 += -lpcre
endif
LIBS_155 += -lmpr
LIBS_155 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_155 += -lhttp
endif

$(BUILD)/bin/utest: $(DEPS_155)
	@echo '      [Link] $(BUILD)/bin/utest'
	$(CC) -o $(BUILD)/bin/utest $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsrun.o" $(LIBPATHS_155) $(LIBS_155) $(LIBS_155) $(LIBS) $(LIBS) 

#
#   watchdog
#
DEPS_156 += $(BUILD)/bin/libmpr.so
DEPS_156 += $(BUILD)/obj/watchdog.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_156 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_156 += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_156 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_156 += -lmpr-openssl
endif
LIBS_156 += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_156 += -lmpr-mbedtls
endif

$(BUILD)/bin/ejsman: $(DEPS_156)
	@echo '      [Link] $(BUILD)/bin/ejsman'
	$(CC) -o $(BUILD)/bin/ejsman $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/watchdog.o" $(LIBPATHS_156) $(LIBS_156) $(LIBS_156) $(LIBS) $(LIBS) 

#
#   www
#
DEPS_157 += src/ejs.web/www/images/banner.jpg
DEPS_157 += src/ejs.web/www/images/favicon.ico
DEPS_157 += src/ejs.web/www/images/splash.jpg
DEPS_157 += src/ejs.web/www/js/jquery.ejs.min.js
DEPS_157 += src/ejs.web/www/js/jquery.min.js
DEPS_157 += src/ejs.web/www/js/jquery.simplemodal.min.js
DEPS_157 += src/ejs.web/www/js/jquery.tablesorter.js
DEPS_157 += src/ejs.web/www/js/jquery.tablesorter.min.js
DEPS_157 += src/ejs.web/www/js/jquery.treeview.min.js
DEPS_157 += src/ejs.web/www/js/tree-images/file.gif
DEPS_157 += src/ejs.web/www/js/tree-images/folder-closed.gif
DEPS_157 += src/ejs.web/www/js/tree-images/folder.gif
DEPS_157 += src/ejs.web/www/js/tree-images/minus.gif
DEPS_157 += src/ejs.web/www/js/tree-images/plus.gif
DEPS_157 += src/ejs.web/www/js/tree-images/treeview-black-line.gif
DEPS_157 += src/ejs.web/www/js/tree-images/treeview-black.gif
DEPS_157 += src/ejs.web/www/js/tree-images/treeview-default-line.gif
DEPS_157 += src/ejs.web/www/js/tree-images/treeview-default.gif
DEPS_157 += src/ejs.web/www/js/tree-images/treeview-famfamfam-line.gif
DEPS_157 += src/ejs.web/www/js/tree-images/treeview-famfamfam.gif
DEPS_157 += src/ejs.web/www/js/tree-images/treeview-gray-line.gif
DEPS_157 += src/ejs.web/www/js/tree-images/treeview-gray.gif
DEPS_157 += src/ejs.web/www/js/tree-images/treeview-red-line.gif
DEPS_157 += src/ejs.web/www/js/tree-images/treeview-red.gif
DEPS_157 += src/ejs.web/www/js/treeview.css
DEPS_157 += src/ejs.web/www/layout.css
DEPS_157 += src/ejs.web/www/themes/default.css

$(BUILD)/bin/www: $(DEPS_157)
	@echo '      [Copy] $(BUILD)/bin/www'
	mkdir -p "$(BUILD)/bin/www/images"
	cp src/ejs.web/www/images/banner.jpg $(BUILD)/bin/www/images/banner.jpg
	cp src/ejs.web/www/images/favicon.ico $(BUILD)/bin/www/images/favicon.ico
	cp src/ejs.web/www/images/splash.jpg $(BUILD)/bin/www/images/splash.jpg
	mkdir -p "$(BUILD)/bin/www/js"
	cp src/ejs.web/www/js/jquery.ejs.min.js $(BUILD)/bin/www/js/jquery.ejs.min.js
	cp src/ejs.web/www/js/jquery.min.js $(BUILD)/bin/www/js/jquery.min.js
	cp src/ejs.web/www/js/jquery.simplemodal.min.js $(BUILD)/bin/www/js/jquery.simplemodal.min.js
	cp src/ejs.web/www/js/jquery.tablesorter.js $(BUILD)/bin/www/js/jquery.tablesorter.js
	cp src/ejs.web/www/js/jquery.tablesorter.min.js $(BUILD)/bin/www/js/jquery.tablesorter.min.js
	cp src/ejs.web/www/js/jquery.treeview.min.js $(BUILD)/bin/www/js/jquery.treeview.min.js
	mkdir -p "$(BUILD)/bin/www/js/tree-images"
	cp src/ejs.web/www/js/tree-images/file.gif $(BUILD)/bin/www/js/tree-images/file.gif
	cp src/ejs.web/www/js/tree-images/folder-closed.gif $(BUILD)/bin/www/js/tree-images/folder-closed.gif
	cp src/ejs.web/www/js/tree-images/folder.gif $(BUILD)/bin/www/js/tree-images/folder.gif
	cp src/ejs.web/www/js/tree-images/minus.gif $(BUILD)/bin/www/js/tree-images/minus.gif
	cp src/ejs.web/www/js/tree-images/plus.gif $(BUILD)/bin/www/js/tree-images/plus.gif
	cp src/ejs.web/www/js/tree-images/treeview-black-line.gif $(BUILD)/bin/www/js/tree-images/treeview-black-line.gif
	cp src/ejs.web/www/js/tree-images/treeview-black.gif $(BUILD)/bin/www/js/tree-images/treeview-black.gif
	cp src/ejs.web/www/js/tree-images/treeview-default-line.gif $(BUILD)/bin/www/js/tree-images/treeview-default-line.gif
	cp src/ejs.web/www/js/tree-images/treeview-default.gif $(BUILD)/bin/www/js/tree-images/treeview-default.gif
	cp src/ejs.web/www/js/tree-images/treeview-famfamfam-line.gif $(BUILD)/bin/www/js/tree-images/treeview-famfamfam-line.gif
	cp src/ejs.web/www/js/tree-images/treeview-famfamfam.gif $(BUILD)/bin/www/js/tree-images/treeview-famfamfam.gif
	cp src/ejs.web/www/js/tree-images/treeview-gray-line.gif $(BUILD)/bin/www/js/tree-images/treeview-gray-line.gif
	cp src/ejs.web/www/js/tree-images/treeview-gray.gif $(BUILD)/bin/www/js/tree-images/treeview-gray.gif
	cp src/ejs.web/www/js/tree-images/treeview-red-line.gif $(BUILD)/bin/www/js/tree-images/treeview-red-line.gif
	cp src/ejs.web/www/js/tree-images/treeview-red.gif $(BUILD)/bin/www/js/tree-images/treeview-red.gif
	cp src/ejs.web/www/js/treeview.css $(BUILD)/bin/www/js/treeview.css
	mkdir -p "$(BUILD)/bin/www"
	cp src/ejs.web/www/layout.css $(BUILD)/bin/www/layout.css
	mkdir -p "$(BUILD)/bin/www/themes"
	cp src/ejs.web/www/themes/default.css $(BUILD)/bin/www/themes/default.css

#
#   installPrep
#

installPrep: $(DEPS_158)
	if [ "`id -u`" != 0 ] ; \
	then echo "Must run as root. Rerun with "sudo"" ; \
	exit 255 ; \
	fi

#
#   stop
#

stop: $(DEPS_159)

#
#   installBinary
#

installBinary: $(DEPS_160)
	mkdir -p "$(ME_APP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	ln -s "$(VERSION)" "$(ME_APP_PREFIX)/latest" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/ejs $(ME_VAPP_PREFIX)/bin/ejs ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/ejs" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	chmod 755 "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/ejs" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejs" "$(ME_BIN_PREFIX)/ejs" ; \
	cp $(BUILD)/bin/ejsc $(ME_VAPP_PREFIX)/bin/ejsc ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/ejsc" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	chmod 755 "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/ejsc" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsc" "$(ME_BIN_PREFIX)/ejsc" ; \
	cp $(BUILD)/bin/ejsman $(ME_VAPP_PREFIX)/bin/ejsman ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/ejsman" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	chmod 755 "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/ejsman" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsman" "$(ME_BIN_PREFIX)/ejsman" ; \
	cp $(BUILD)/bin/ejsmod $(ME_VAPP_PREFIX)/bin/ejsmod ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/ejsmod" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	chmod 755 "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/ejsmod" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsmod" "$(ME_BIN_PREFIX)/ejsmod" ; \
	cp $(BUILD)/bin/ejsrun $(ME_VAPP_PREFIX)/bin/ejsrun ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/ejsrun" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	chmod 755 "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/ejsrun" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsrun" "$(ME_BIN_PREFIX)/ejsrun" ; \
	cp $(BUILD)/bin/mvc $(ME_VAPP_PREFIX)/bin/mvc ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/mvc" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	chmod 755 "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/mvc" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/mvc" "$(ME_BIN_PREFIX)/mvc" ; \
	cp $(BUILD)/bin/utest $(ME_VAPP_PREFIX)/bin/utest ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/utest" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	chmod 755 "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/utest" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/utest" "$(ME_BIN_PREFIX)/utest" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/libejs.db.sqlite.so $(ME_VAPP_PREFIX)/bin/libejs.db.sqlite.so ; \
	cp $(BUILD)/bin/libejs.so $(ME_VAPP_PREFIX)/bin/libejs.so ; \
	cp $(BUILD)/bin/libejs.web.so $(ME_VAPP_PREFIX)/bin/libejs.web.so ; \
	cp $(BUILD)/bin/libejs.zlib.so $(ME_VAPP_PREFIX)/bin/libejs.zlib.so ; \
	cp $(BUILD)/bin/libhttp.so $(ME_VAPP_PREFIX)/bin/libhttp.so ; \
	cp $(BUILD)/bin/libmpr.so $(ME_VAPP_PREFIX)/bin/libmpr.so ; \
	cp $(BUILD)/bin/libpcre.so $(ME_VAPP_PREFIX)/bin/libpcre.so ; \
	cp $(BUILD)/bin/libsql.so $(ME_VAPP_PREFIX)/bin/libsql.so ; \
	cp $(BUILD)/bin/libzlib.so $(ME_VAPP_PREFIX)/bin/libzlib.so ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/ejs.db.mapper.mod $(ME_VAPP_PREFIX)/bin/ejs.db.mapper.mod ; \
	cp $(BUILD)/bin/ejs.db.mod $(ME_VAPP_PREFIX)/bin/ejs.db.mod ; \
	cp $(BUILD)/bin/ejs.db.sqlite.mod $(ME_VAPP_PREFIX)/bin/ejs.db.sqlite.mod ; \
	cp $(BUILD)/bin/ejs.mod $(ME_VAPP_PREFIX)/bin/ejs.mod ; \
	cp $(BUILD)/bin/ejs.mvc.mod $(ME_VAPP_PREFIX)/bin/ejs.mvc.mod ; \
	cp $(BUILD)/bin/ejs.tar.mod $(ME_VAPP_PREFIX)/bin/ejs.tar.mod ; \
	cp $(BUILD)/bin/ejs.template.mod $(ME_VAPP_PREFIX)/bin/ejs.template.mod ; \
	cp $(BUILD)/bin/ejs.unix.mod $(ME_VAPP_PREFIX)/bin/ejs.unix.mod ; \
	cp $(BUILD)/bin/ejs.web.mod $(ME_VAPP_PREFIX)/bin/ejs.web.mod ; \
	cp $(BUILD)/bin/ejs.zlib.mod $(ME_VAPP_PREFIX)/bin/ejs.zlib.mod ; \
	cp $(BUILD)/bin/mvc.es $(ME_VAPP_PREFIX)/bin/mvc.es ; \
	cp $(BUILD)/bin/utest.es $(ME_VAPP_PREFIX)/bin/utest.es ; \
	cp $(BUILD)/bin/utest.worker $(ME_VAPP_PREFIX)/bin/utest.worker ; \
	if [ "$(ME_COM_SSL)" = 1 ]; then true ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(BUILD)/bin/roots.crt $(ME_VAPP_PREFIX)/bin/roots.crt ; \
	fi ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/www" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/www/images" ; \
	cp src/ejs.web/www/images/banner.jpg $(ME_VAPP_PREFIX)/bin/www/images/banner.jpg ; \
	cp src/ejs.web/www/images/favicon.ico $(ME_VAPP_PREFIX)/bin/www/images/favicon.ico ; \
	cp src/ejs.web/www/images/splash.jpg $(ME_VAPP_PREFIX)/bin/www/images/splash.jpg ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/www/js" ; \
	cp src/ejs.web/www/js/jquery.ejs.min.js $(ME_VAPP_PREFIX)/bin/www/js/jquery.ejs.min.js ; \
	cp src/ejs.web/www/js/jquery.min.js $(ME_VAPP_PREFIX)/bin/www/js/jquery.min.js ; \
	cp src/ejs.web/www/js/jquery.simplemodal.min.js $(ME_VAPP_PREFIX)/bin/www/js/jquery.simplemodal.min.js ; \
	cp src/ejs.web/www/js/jquery.tablesorter.js $(ME_VAPP_PREFIX)/bin/www/js/jquery.tablesorter.js ; \
	cp src/ejs.web/www/js/jquery.tablesorter.min.js $(ME_VAPP_PREFIX)/bin/www/js/jquery.tablesorter.min.js ; \
	cp src/ejs.web/www/js/jquery.treeview.min.js $(ME_VAPP_PREFIX)/bin/www/js/jquery.treeview.min.js ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/www/js/tree-images" ; \
	cp src/ejs.web/www/js/tree-images/file.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/file.gif ; \
	cp src/ejs.web/www/js/tree-images/folder-closed.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/folder-closed.gif ; \
	cp src/ejs.web/www/js/tree-images/folder.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/folder.gif ; \
	cp src/ejs.web/www/js/tree-images/minus.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/minus.gif ; \
	cp src/ejs.web/www/js/tree-images/plus.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/plus.gif ; \
	cp src/ejs.web/www/js/tree-images/treeview-black-line.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/treeview-black-line.gif ; \
	cp src/ejs.web/www/js/tree-images/treeview-black.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/treeview-black.gif ; \
	cp src/ejs.web/www/js/tree-images/treeview-default-line.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/treeview-default-line.gif ; \
	cp src/ejs.web/www/js/tree-images/treeview-default.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/treeview-default.gif ; \
	cp src/ejs.web/www/js/tree-images/treeview-famfamfam-line.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/treeview-famfamfam-line.gif ; \
	cp src/ejs.web/www/js/tree-images/treeview-famfamfam.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/treeview-famfamfam.gif ; \
	cp src/ejs.web/www/js/tree-images/treeview-gray-line.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/treeview-gray-line.gif ; \
	cp src/ejs.web/www/js/tree-images/treeview-gray.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/treeview-gray.gif ; \
	cp src/ejs.web/www/js/tree-images/treeview-red-line.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/treeview-red-line.gif ; \
	cp src/ejs.web/www/js/tree-images/treeview-red.gif $(ME_VAPP_PREFIX)/bin/www/js/tree-images/treeview-red.gif ; \
	cp src/ejs.web/www/js/treeview.css $(ME_VAPP_PREFIX)/bin/www/js/treeview.css ; \
	cp src/ejs.web/www/layout.css $(ME_VAPP_PREFIX)/bin/www/layout.css ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/www/themes" ; \
	cp src/ejs.web/www/themes/default.css $(ME_VAPP_PREFIX)/bin/www/themes/default.css ; \
	mkdir -p "$(ME_VAPP_PREFIX)/inc" ; \
	cp src/ejs.h $(ME_VAPP_PREFIX)/inc/ejs.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.h" "$(ME_INC_PREFIX)/ejs/ejs.h" ; \
	cp src/ejsByteCode.h $(ME_VAPP_PREFIX)/inc/ejsByteCode.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsByteCode.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsByteCode.h" "$(ME_INC_PREFIX)/ejs/ejsByteCode.h" ; \
	cp src/ejsByteCodeTable.h $(ME_VAPP_PREFIX)/inc/ejsByteCodeTable.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsByteCodeTable.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsByteCodeTable.h" "$(ME_INC_PREFIX)/ejs/ejsByteCodeTable.h" ; \
	cp src/ejsCompiler.h $(ME_VAPP_PREFIX)/inc/ejsCompiler.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsCompiler.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsCompiler.h" "$(ME_INC_PREFIX)/ejs/ejsCompiler.h" ; \
	cp src/ejsCustomize.h $(ME_VAPP_PREFIX)/inc/ejsCustomize.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsCustomize.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsCustomize.h" "$(ME_INC_PREFIX)/ejs/ejsCustomize.h" ; \
	cp src/vm/ejsByteGoto.h $(ME_VAPP_PREFIX)/inc/ejsByteGoto.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsByteGoto.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsByteGoto.h" "$(ME_INC_PREFIX)/ejs/ejsByteGoto.h" ; \
	cp src/slots/ejs.cache.local.slots.h $(ME_VAPP_PREFIX)/inc/ejs.cache.local.slots.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.cache.local.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.cache.local.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.cache.local.slots.h" ; \
	cp src/slots/ejs.db.sqlite.slots.h $(ME_VAPP_PREFIX)/inc/ejs.db.sqlite.slots.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.db.sqlite.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.db.sqlite.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.db.sqlite.slots.h" ; \
	cp src/slots/ejs.slots.h $(ME_VAPP_PREFIX)/inc/ejs.slots.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.slots.h" ; \
	cp src/slots/ejs.web.slots.h $(ME_VAPP_PREFIX)/inc/ejs.web.slots.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.web.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.web.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.web.slots.h" ; \
	cp src/slots/ejs.zlib.slots.h $(ME_VAPP_PREFIX)/inc/ejs.zlib.slots.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.zlib.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.zlib.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.zlib.slots.h" ; \
	cp src/cmd/ejsmod.h $(ME_VAPP_PREFIX)/inc/ejsmod.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsmod.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsmod.h" "$(ME_INC_PREFIX)/ejs/ejsmod.h" ; \
	cp src/ejs.web/ejsWeb.h $(ME_VAPP_PREFIX)/inc/ejsWeb.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsWeb.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsWeb.h" "$(ME_INC_PREFIX)/ejs/ejsWeb.h" ; \
	cp src/http/http.h $(ME_VAPP_PREFIX)/inc/http.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/http.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/http.h" "$(ME_INC_PREFIX)/ejs/http.h" ; \
	cp src/mbedtls/embedtls.h $(ME_VAPP_PREFIX)/inc/embedtls.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/embedtls.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/embedtls.h" "$(ME_INC_PREFIX)/ejs/embedtls.h" ; \
	cp src/mbedtls/mbedtls-config.h $(ME_VAPP_PREFIX)/inc/mbedtls-config.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/mbedtls-config.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/mbedtls-config.h" "$(ME_INC_PREFIX)/ejs/mbedtls-config.h" ; \
	cp src/mbedtls/mbedtls.h $(ME_VAPP_PREFIX)/inc/mbedtls.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/mbedtls.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/mbedtls.h" "$(ME_INC_PREFIX)/ejs/mbedtls.h" ; \
	cp src/mpr/mpr.h $(ME_VAPP_PREFIX)/inc/mpr.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/mpr.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/mpr.h" "$(ME_INC_PREFIX)/ejs/mpr.h" ; \
	cp src/osdep/osdep.h $(ME_VAPP_PREFIX)/inc/osdep.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/osdep.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/osdep.h" "$(ME_INC_PREFIX)/ejs/osdep.h" ; \
	cp src/pcre/pcre.h $(ME_VAPP_PREFIX)/inc/pcre.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/pcre.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/pcre.h" "$(ME_INC_PREFIX)/ejs/pcre.h" ; \
	cp src/sqlite/sqlite3.h $(ME_VAPP_PREFIX)/inc/sqlite3.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/sqlite3.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/sqlite3.h" "$(ME_INC_PREFIX)/ejs/sqlite3.h" ; \
	cp src/zlib/zlib.h $(ME_VAPP_PREFIX)/inc/zlib.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/zlib.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/zlib.h" "$(ME_INC_PREFIX)/ejs/zlib.h" ; \
	cp $(BUILD)/inc/me.h $(ME_VAPP_PREFIX)/inc/me.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/me.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/me.h" "$(ME_INC_PREFIX)/ejs/me.h" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/doc/man1" ; \
	cp doc/dist/man/ejs.1 $(ME_VAPP_PREFIX)/doc/man1/ejs.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/ejs.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/ejs.1" "$(ME_MAN_PREFIX)/man1/ejs.1" ; \
	cp doc/dist/man/ejsc.1 $(ME_VAPP_PREFIX)/doc/man1/ejsc.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/ejsc.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/ejsc.1" "$(ME_MAN_PREFIX)/man1/ejsc.1" ; \
	cp doc/dist/man/ejsmod.1 $(ME_VAPP_PREFIX)/doc/man1/ejsmod.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/ejsmod.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/ejsmod.1" "$(ME_MAN_PREFIX)/man1/ejsmod.1" ; \
	cp doc/dist/man/http.1 $(ME_VAPP_PREFIX)/doc/man1/http.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/http.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/http.1" "$(ME_MAN_PREFIX)/man1/http.1" ; \
	cp doc/dist/man/manager.1 $(ME_VAPP_PREFIX)/doc/man1/manager.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/manager.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/manager.1" "$(ME_MAN_PREFIX)/man1/manager.1" ; \
	cp doc/dist/man/mvc.1 $(ME_VAPP_PREFIX)/doc/man1/mvc.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/mvc.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/mvc.1" "$(ME_MAN_PREFIX)/man1/mvc.1"

#
#   start
#

start: $(DEPS_161)

#
#   install
#
DEPS_162 += installPrep
DEPS_162 += stop
DEPS_162 += installBinary
DEPS_162 += start

install: $(DEPS_162)

#
#   uninstall
#
DEPS_163 += stop

uninstall: $(DEPS_163)

#
#   version
#

version: $(DEPS_164)
	echo $(VERSION)

