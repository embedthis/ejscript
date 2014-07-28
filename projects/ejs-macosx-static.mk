#
#   ejs-macosx-static.mk -- Makefile to build Embedthis Ejscript for macosx
#

NAME                  := ejs
VERSION               := 2.4.1
PROFILE               ?= static
ARCH                  ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
CC_ARCH               ?= $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                    ?= macosx
CC                    ?= clang
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
BUILD                 ?= build/$(CONFIG)
LBIN                  ?= $(BUILD)/bin
PATH                  := $(LBIN):$(PATH)

ME_COM_EST            ?= 1
ME_COM_HTTP           ?= 1
ME_COM_MATRIXSSL      ?= 0
ME_COM_NANOSSL        ?= 0
ME_COM_OPENSSL        ?= 0
ME_COM_PCRE           ?= 1
ME_COM_SQLITE         ?= 1
ME_COM_SSL            ?= 1
ME_COM_VXWORKS        ?= 0
ME_COM_WINSDK         ?= 1
ME_COM_ZLIB           ?= 1

ifeq ($(ME_COM_EST),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_MATRIXSSL),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_NANOSSL),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_OPENSSL),1)
    ME_COM_SSL := 1
endif

ME_COM_COMPILER_PATH  ?= clang
ME_COM_LIB_PATH       ?= ar
ME_COM_MATRIXSSL_PATH ?= /usr/src/matrixssl
ME_COM_NANOSSL_PATH   ?= /usr/src/nanossl
ME_COM_OPENSSL_PATH   ?= /usr/src/openssl

CFLAGS                += -g -w
DFLAGS                +=  $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_EST=$(ME_COM_EST) -DME_COM_HTTP=$(ME_COM_HTTP) -DME_COM_MATRIXSSL=$(ME_COM_MATRIXSSL) -DME_COM_NANOSSL=$(ME_COM_NANOSSL) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_PCRE=$(ME_COM_PCRE) -DME_COM_SQLITE=$(ME_COM_SQLITE) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) -DME_COM_WINSDK=$(ME_COM_WINSDK) -DME_COM_ZLIB=$(ME_COM_ZLIB) 
IFLAGS                += "-Ibuild/$(CONFIG)/inc"
LDFLAGS               += '-Wl,-rpath,@executable_path/' '-Wl,-rpath,@loader_path/'
LIBPATHS              += -Lbuild/$(CONFIG)/bin
LIBS                  += -ldl -lpthread -lm

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
ME_WEB_PREFIX         ?= $(ME_ROOT_PREFIX)/var/www/$(NAME)-default
ME_LOG_PREFIX         ?= $(ME_ROOT_PREFIX)/var/log/$(NAME)
ME_SPOOL_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)
ME_CACHE_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)/cache
ME_SRC_PREFIX         ?= $(ME_ROOT_PREFIX)$(NAME)-$(VERSION)


TARGETS               += build/$(CONFIG)/bin/ejs
TARGETS               += build/$(CONFIG)/bin/ejs.db.mapper.mod
TARGETS               += build/$(CONFIG)/bin/ejs.db.sqlite.mod
TARGETS               += build/$(CONFIG)/bin/ejs.mail.mod
TARGETS               += build/$(CONFIG)/bin/ejs.mvc.mod
TARGETS               += build/$(CONFIG)/bin/ejs.tar.mod
TARGETS               += build/$(CONFIG)/bin/ejs.zlib.mod
TARGETS               += build/$(CONFIG)/bin/ejsrun
TARGETS               += build/$(CONFIG)/bin/ca.crt
TARGETS               += build/$(CONFIG)/bin/libejs.db.sqlite.a
TARGETS               += build/$(CONFIG)/bin/libejs.web.a
TARGETS               += build/$(CONFIG)/bin/libejs.zlib.a
ifeq ($(ME_COM_EST),1)
    TARGETS           += build/$(CONFIG)/bin/libest.a
endif
TARGETS               += build/$(CONFIG)/bin/libmprssl.a
TARGETS               += build/$(CONFIG)/bin/ejsman
TARGETS               += build/$(CONFIG)/bin/mvc
ifeq ($(ME_COM_SQLITE),1)
    TARGETS           += build/$(CONFIG)/bin/sqlite
endif
TARGETS               += build/$(CONFIG)/bin/utest
TARGETS               += build/$(CONFIG)/bin/www

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
	@[ ! -f $(BUILD)/inc/me.h ] && cp projects/ejs-macosx-static-me.h $(BUILD)/inc/me.h ; true
	@if ! diff $(BUILD)/inc/me.h projects/ejs-macosx-static-me.h >/dev/null ; then\
		cp projects/ejs-macosx-static-me.h $(BUILD)/inc/me.h  ; \
	fi; true
	@if [ -f "$(BUILD)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != "`cat $(BUILD)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build: "`cat $(BUILD)/.makeflags`"" ; \
		fi ; \
	fi
	@echo $(MAKEFLAGS) >$(BUILD)/.makeflags

clean:
	rm -f "build/$(CONFIG)/obj/doc.o"
	rm -f "build/$(CONFIG)/obj/docFiles.o"
	rm -f "build/$(CONFIG)/obj/dtoa.o"
	rm -f "build/$(CONFIG)/obj/ecAst.o"
	rm -f "build/$(CONFIG)/obj/ecCodeGen.o"
	rm -f "build/$(CONFIG)/obj/ecCompiler.o"
	rm -f "build/$(CONFIG)/obj/ecLex.o"
	rm -f "build/$(CONFIG)/obj/ecModuleWrite.o"
	rm -f "build/$(CONFIG)/obj/ecParser.o"
	rm -f "build/$(CONFIG)/obj/ecState.o"
	rm -f "build/$(CONFIG)/obj/ejs.o"
	rm -f "build/$(CONFIG)/obj/ejsApp.o"
	rm -f "build/$(CONFIG)/obj/ejsArray.o"
	rm -f "build/$(CONFIG)/obj/ejsBlock.o"
	rm -f "build/$(CONFIG)/obj/ejsBoolean.o"
	rm -f "build/$(CONFIG)/obj/ejsByteArray.o"
	rm -f "build/$(CONFIG)/obj/ejsByteCode.o"
	rm -f "build/$(CONFIG)/obj/ejsCache.o"
	rm -f "build/$(CONFIG)/obj/ejsCmd.o"
	rm -f "build/$(CONFIG)/obj/ejsConfig.o"
	rm -f "build/$(CONFIG)/obj/ejsDate.o"
	rm -f "build/$(CONFIG)/obj/ejsDebug.o"
	rm -f "build/$(CONFIG)/obj/ejsError.o"
	rm -f "build/$(CONFIG)/obj/ejsException.o"
	rm -f "build/$(CONFIG)/obj/ejsFile.o"
	rm -f "build/$(CONFIG)/obj/ejsFileSystem.o"
	rm -f "build/$(CONFIG)/obj/ejsFrame.o"
	rm -f "build/$(CONFIG)/obj/ejsFunction.o"
	rm -f "build/$(CONFIG)/obj/ejsGC.o"
	rm -f "build/$(CONFIG)/obj/ejsGlobal.o"
	rm -f "build/$(CONFIG)/obj/ejsHelper.o"
	rm -f "build/$(CONFIG)/obj/ejsHttp.o"
	rm -f "build/$(CONFIG)/obj/ejsHttpServer.o"
	rm -f "build/$(CONFIG)/obj/ejsInterp.o"
	rm -f "build/$(CONFIG)/obj/ejsIterator.o"
	rm -f "build/$(CONFIG)/obj/ejsJSON.o"
	rm -f "build/$(CONFIG)/obj/ejsLoader.o"
	rm -f "build/$(CONFIG)/obj/ejsLocalCache.o"
	rm -f "build/$(CONFIG)/obj/ejsMath.o"
	rm -f "build/$(CONFIG)/obj/ejsMemory.o"
	rm -f "build/$(CONFIG)/obj/ejsModule.o"
	rm -f "build/$(CONFIG)/obj/ejsMprLog.o"
	rm -f "build/$(CONFIG)/obj/ejsNamespace.o"
	rm -f "build/$(CONFIG)/obj/ejsNull.o"
	rm -f "build/$(CONFIG)/obj/ejsNumber.o"
	rm -f "build/$(CONFIG)/obj/ejsObject.o"
	rm -f "build/$(CONFIG)/obj/ejsPath.o"
	rm -f "build/$(CONFIG)/obj/ejsPot.o"
	rm -f "build/$(CONFIG)/obj/ejsRegExp.o"
	rm -f "build/$(CONFIG)/obj/ejsRequest.o"
	rm -f "build/$(CONFIG)/obj/ejsScope.o"
	rm -f "build/$(CONFIG)/obj/ejsService.o"
	rm -f "build/$(CONFIG)/obj/ejsSession.o"
	rm -f "build/$(CONFIG)/obj/ejsSocket.o"
	rm -f "build/$(CONFIG)/obj/ejsSqlite.o"
	rm -f "build/$(CONFIG)/obj/ejsString.o"
	rm -f "build/$(CONFIG)/obj/ejsSystem.o"
	rm -f "build/$(CONFIG)/obj/ejsTimer.o"
	rm -f "build/$(CONFIG)/obj/ejsType.o"
	rm -f "build/$(CONFIG)/obj/ejsUri.o"
	rm -f "build/$(CONFIG)/obj/ejsVoid.o"
	rm -f "build/$(CONFIG)/obj/ejsWeb.o"
	rm -f "build/$(CONFIG)/obj/ejsWebSocket.o"
	rm -f "build/$(CONFIG)/obj/ejsWorker.o"
	rm -f "build/$(CONFIG)/obj/ejsXML.o"
	rm -f "build/$(CONFIG)/obj/ejsXMLList.o"
	rm -f "build/$(CONFIG)/obj/ejsXMLLoader.o"
	rm -f "build/$(CONFIG)/obj/ejsZlib.o"
	rm -f "build/$(CONFIG)/obj/ejsc.o"
	rm -f "build/$(CONFIG)/obj/ejsmod.o"
	rm -f "build/$(CONFIG)/obj/ejsrun.o"
	rm -f "build/$(CONFIG)/obj/estLib.o"
	rm -f "build/$(CONFIG)/obj/httpLib.o"
	rm -f "build/$(CONFIG)/obj/listing.o"
	rm -f "build/$(CONFIG)/obj/makerom.o"
	rm -f "build/$(CONFIG)/obj/manager.o"
	rm -f "build/$(CONFIG)/obj/mprLib.o"
	rm -f "build/$(CONFIG)/obj/mprSsl.o"
	rm -f "build/$(CONFIG)/obj/pcre.o"
	rm -f "build/$(CONFIG)/obj/slotGen.o"
	rm -f "build/$(CONFIG)/obj/sqlite.o"
	rm -f "build/$(CONFIG)/obj/sqlite3.o"
	rm -f "build/$(CONFIG)/obj/zlib.o"
	rm -f "build/$(CONFIG)/bin/ejs"
	rm -f "build/$(CONFIG)/bin/ejsc"
	rm -f "build/$(CONFIG)/bin/ejsmod"
	rm -f "build/$(CONFIG)/bin/ejsrun"
	rm -f "build/$(CONFIG)/bin/ca.crt"
	rm -f "build/$(CONFIG)/bin/libejs.a"
	rm -f "build/$(CONFIG)/bin/libejs.db.sqlite.a"
	rm -f "build/$(CONFIG)/bin/libejs.web.a"
	rm -f "build/$(CONFIG)/bin/libejs.zlib.a"
	rm -f "build/$(CONFIG)/bin/libest.a"
	rm -f "build/$(CONFIG)/bin/libhttp.a"
	rm -f "build/$(CONFIG)/bin/libmpr.a"
	rm -f "build/$(CONFIG)/bin/libmprssl.a"
	rm -f "build/$(CONFIG)/bin/libpcre.a"
	rm -f "build/$(CONFIG)/bin/libsql.a"
	rm -f "build/$(CONFIG)/bin/libzlib.a"
	rm -f "build/$(CONFIG)/bin/makerom"
	rm -f "build/$(CONFIG)/bin/ejsman"
	rm -f "build/$(CONFIG)/bin/sqlite"
	rm -f "build/$(CONFIG)/bin/utest"

clobber: clean
	rm -fr ./$(BUILD)


#
#   slots
#
slots: $(DEPS_1)

#
#   mpr.h
#
build/$(CONFIG)/inc/mpr.h: $(DEPS_2)
	@echo '      [Copy] build/$(CONFIG)/inc/mpr.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/mpr/mpr.h build/$(CONFIG)/inc/mpr.h

#
#   me.h
#
build/$(CONFIG)/inc/me.h: $(DEPS_3)
	@echo '      [Copy] build/$(CONFIG)/inc/me.h'

#
#   osdep.h
#
build/$(CONFIG)/inc/osdep.h: $(DEPS_4)
	@echo '      [Copy] build/$(CONFIG)/inc/osdep.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/osdep/osdep.h build/$(CONFIG)/inc/osdep.h

#
#   mprLib.o
#
DEPS_5 += build/$(CONFIG)/inc/me.h
DEPS_5 += build/$(CONFIG)/inc/mpr.h
DEPS_5 += build/$(CONFIG)/inc/osdep.h

build/$(CONFIG)/obj/mprLib.o: \
    src/paks/mpr/mprLib.c $(DEPS_5)
	@echo '   [Compile] build/$(CONFIG)/obj/mprLib.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/mprLib.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/mpr/mprLib.c

#
#   libmpr
#
DEPS_6 += build/$(CONFIG)/inc/mpr.h
DEPS_6 += build/$(CONFIG)/inc/me.h
DEPS_6 += build/$(CONFIG)/inc/osdep.h
DEPS_6 += build/$(CONFIG)/obj/mprLib.o

build/$(CONFIG)/bin/libmpr.a: $(DEPS_6)
	@echo '      [Link] build/$(CONFIG)/bin/libmpr.a'
	ar -cr build/$(CONFIG)/bin/libmpr.a "build/$(CONFIG)/obj/mprLib.o"

#
#   pcre.h
#
build/$(CONFIG)/inc/pcre.h: $(DEPS_7)
	@echo '      [Copy] build/$(CONFIG)/inc/pcre.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/pcre/pcre.h build/$(CONFIG)/inc/pcre.h

#
#   pcre.o
#
DEPS_8 += build/$(CONFIG)/inc/me.h
DEPS_8 += build/$(CONFIG)/inc/pcre.h

build/$(CONFIG)/obj/pcre.o: \
    src/paks/pcre/pcre.c $(DEPS_8)
	@echo '   [Compile] build/$(CONFIG)/obj/pcre.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/pcre.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/pcre/pcre.c

ifeq ($(ME_COM_PCRE),1)
#
#   libpcre
#
DEPS_9 += build/$(CONFIG)/inc/pcre.h
DEPS_9 += build/$(CONFIG)/inc/me.h
DEPS_9 += build/$(CONFIG)/obj/pcre.o

build/$(CONFIG)/bin/libpcre.a: $(DEPS_9)
	@echo '      [Link] build/$(CONFIG)/bin/libpcre.a'
	ar -cr build/$(CONFIG)/bin/libpcre.a "build/$(CONFIG)/obj/pcre.o"
endif

#
#   http.h
#
build/$(CONFIG)/inc/http.h: $(DEPS_10)
	@echo '      [Copy] build/$(CONFIG)/inc/http.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/http/http.h build/$(CONFIG)/inc/http.h

#
#   httpLib.o
#
DEPS_11 += build/$(CONFIG)/inc/me.h
DEPS_11 += build/$(CONFIG)/inc/http.h
DEPS_11 += build/$(CONFIG)/inc/mpr.h

build/$(CONFIG)/obj/httpLib.o: \
    src/paks/http/httpLib.c $(DEPS_11)
	@echo '   [Compile] build/$(CONFIG)/obj/httpLib.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/httpLib.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/http/httpLib.c

ifeq ($(ME_COM_HTTP),1)
#
#   libhttp
#
DEPS_12 += build/$(CONFIG)/inc/mpr.h
DEPS_12 += build/$(CONFIG)/inc/me.h
DEPS_12 += build/$(CONFIG)/inc/osdep.h
DEPS_12 += build/$(CONFIG)/obj/mprLib.o
DEPS_12 += build/$(CONFIG)/bin/libmpr.a
DEPS_12 += build/$(CONFIG)/inc/pcre.h
DEPS_12 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_12 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_12 += build/$(CONFIG)/inc/http.h
DEPS_12 += build/$(CONFIG)/obj/httpLib.o

build/$(CONFIG)/bin/libhttp.a: $(DEPS_12)
	@echo '      [Link] build/$(CONFIG)/bin/libhttp.a'
	ar -cr build/$(CONFIG)/bin/libhttp.a "build/$(CONFIG)/obj/httpLib.o"
endif

#
#   ejs.cache.local.slots.h
#
build/$(CONFIG)/inc/ejs.cache.local.slots.h: $(DEPS_13)
	@echo '      [Copy] build/$(CONFIG)/inc/ejs.cache.local.slots.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/slots/ejs.cache.local.slots.h build/$(CONFIG)/inc/ejs.cache.local.slots.h

#
#   ejs.db.sqlite.slots.h
#
build/$(CONFIG)/inc/ejs.db.sqlite.slots.h: $(DEPS_14)
	@echo '      [Copy] build/$(CONFIG)/inc/ejs.db.sqlite.slots.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/slots/ejs.db.sqlite.slots.h build/$(CONFIG)/inc/ejs.db.sqlite.slots.h

#
#   ejs.slots.h
#
build/$(CONFIG)/inc/ejs.slots.h: $(DEPS_15)
	@echo '      [Copy] build/$(CONFIG)/inc/ejs.slots.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/slots/ejs.slots.h build/$(CONFIG)/inc/ejs.slots.h

#
#   ejs.web.slots.h
#
build/$(CONFIG)/inc/ejs.web.slots.h: $(DEPS_16)
	@echo '      [Copy] build/$(CONFIG)/inc/ejs.web.slots.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/slots/ejs.web.slots.h build/$(CONFIG)/inc/ejs.web.slots.h

#
#   ejs.zlib.slots.h
#
build/$(CONFIG)/inc/ejs.zlib.slots.h: $(DEPS_17)
	@echo '      [Copy] build/$(CONFIG)/inc/ejs.zlib.slots.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/slots/ejs.zlib.slots.h build/$(CONFIG)/inc/ejs.zlib.slots.h

#
#   ejsByteCode.h
#
build/$(CONFIG)/inc/ejsByteCode.h: $(DEPS_18)
	@echo '      [Copy] build/$(CONFIG)/inc/ejsByteCode.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/ejsByteCode.h build/$(CONFIG)/inc/ejsByteCode.h

#
#   ejsByteCodeTable.h
#
build/$(CONFIG)/inc/ejsByteCodeTable.h: $(DEPS_19)
	@echo '      [Copy] build/$(CONFIG)/inc/ejsByteCodeTable.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/ejsByteCodeTable.h build/$(CONFIG)/inc/ejsByteCodeTable.h

#
#   ejsCustomize.h
#
build/$(CONFIG)/inc/ejsCustomize.h: $(DEPS_20)
	@echo '      [Copy] build/$(CONFIG)/inc/ejsCustomize.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/ejsCustomize.h build/$(CONFIG)/inc/ejsCustomize.h

#
#   ejs.h
#
DEPS_21 += build/$(CONFIG)/inc/mpr.h
DEPS_21 += build/$(CONFIG)/inc/http.h
DEPS_21 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_21 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_21 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_21 += build/$(CONFIG)/inc/ejsCustomize.h

build/$(CONFIG)/inc/ejs.h: $(DEPS_21)
	@echo '      [Copy] build/$(CONFIG)/inc/ejs.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/ejs.h build/$(CONFIG)/inc/ejs.h

#
#   ejsCompiler.h
#
build/$(CONFIG)/inc/ejsCompiler.h: $(DEPS_22)
	@echo '      [Copy] build/$(CONFIG)/inc/ejsCompiler.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/ejsCompiler.h build/$(CONFIG)/inc/ejsCompiler.h

#
#   ecAst.o
#
DEPS_23 += build/$(CONFIG)/inc/me.h
DEPS_23 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_23 += build/$(CONFIG)/inc/mpr.h
DEPS_23 += build/$(CONFIG)/inc/http.h
DEPS_23 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_23 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_23 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_23 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_23 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ecAst.o: \
    src/compiler/ecAst.c $(DEPS_23)
	@echo '   [Compile] build/$(CONFIG)/obj/ecAst.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ecAst.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/compiler/ecAst.c

#
#   ecCodeGen.o
#
DEPS_24 += build/$(CONFIG)/inc/me.h
DEPS_24 += build/$(CONFIG)/inc/ejsCompiler.h

build/$(CONFIG)/obj/ecCodeGen.o: \
    src/compiler/ecCodeGen.c $(DEPS_24)
	@echo '   [Compile] build/$(CONFIG)/obj/ecCodeGen.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ecCodeGen.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/compiler/ecCodeGen.c

#
#   ecCompiler.o
#
DEPS_25 += build/$(CONFIG)/inc/me.h
DEPS_25 += build/$(CONFIG)/inc/ejsCompiler.h

build/$(CONFIG)/obj/ecCompiler.o: \
    src/compiler/ecCompiler.c $(DEPS_25)
	@echo '   [Compile] build/$(CONFIG)/obj/ecCompiler.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ecCompiler.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/compiler/ecCompiler.c

#
#   ecLex.o
#
DEPS_26 += build/$(CONFIG)/inc/me.h
DEPS_26 += build/$(CONFIG)/inc/ejsCompiler.h

build/$(CONFIG)/obj/ecLex.o: \
    src/compiler/ecLex.c $(DEPS_26)
	@echo '   [Compile] build/$(CONFIG)/obj/ecLex.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ecLex.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/compiler/ecLex.c

#
#   ecModuleWrite.o
#
DEPS_27 += build/$(CONFIG)/inc/me.h
DEPS_27 += build/$(CONFIG)/inc/ejsCompiler.h

build/$(CONFIG)/obj/ecModuleWrite.o: \
    src/compiler/ecModuleWrite.c $(DEPS_27)
	@echo '   [Compile] build/$(CONFIG)/obj/ecModuleWrite.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ecModuleWrite.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/compiler/ecModuleWrite.c

#
#   ecParser.o
#
DEPS_28 += build/$(CONFIG)/inc/me.h
DEPS_28 += build/$(CONFIG)/inc/ejsCompiler.h

build/$(CONFIG)/obj/ecParser.o: \
    src/compiler/ecParser.c $(DEPS_28)
	@echo '   [Compile] build/$(CONFIG)/obj/ecParser.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ecParser.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/compiler/ecParser.c

#
#   ecState.o
#
DEPS_29 += build/$(CONFIG)/inc/me.h
DEPS_29 += build/$(CONFIG)/inc/ejsCompiler.h

build/$(CONFIG)/obj/ecState.o: \
    src/compiler/ecState.c $(DEPS_29)
	@echo '   [Compile] build/$(CONFIG)/obj/ecState.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ecState.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/compiler/ecState.c

#
#   dtoa.o
#
DEPS_30 += build/$(CONFIG)/inc/me.h
DEPS_30 += build/$(CONFIG)/inc/mpr.h

build/$(CONFIG)/obj/dtoa.o: \
    src/core/src/dtoa.c $(DEPS_30)
	@echo '   [Compile] build/$(CONFIG)/obj/dtoa.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/dtoa.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/dtoa.c

#
#   ejsApp.o
#
DEPS_31 += build/$(CONFIG)/inc/me.h
DEPS_31 += build/$(CONFIG)/inc/mpr.h
DEPS_31 += build/$(CONFIG)/inc/http.h
DEPS_31 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_31 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_31 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_31 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_31 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsApp.o: \
    src/core/src/ejsApp.c $(DEPS_31)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsApp.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsApp.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsApp.c

#
#   ejsArray.o
#
DEPS_32 += build/$(CONFIG)/inc/me.h
DEPS_32 += build/$(CONFIG)/inc/mpr.h
DEPS_32 += build/$(CONFIG)/inc/http.h
DEPS_32 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_32 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_32 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_32 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_32 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsArray.o: \
    src/core/src/ejsArray.c $(DEPS_32)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsArray.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsArray.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsArray.c

#
#   ejsBlock.o
#
DEPS_33 += build/$(CONFIG)/inc/me.h
DEPS_33 += build/$(CONFIG)/inc/mpr.h
DEPS_33 += build/$(CONFIG)/inc/http.h
DEPS_33 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_33 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_33 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_33 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_33 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsBlock.o: \
    src/core/src/ejsBlock.c $(DEPS_33)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsBlock.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsBlock.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsBlock.c

#
#   ejsBoolean.o
#
DEPS_34 += build/$(CONFIG)/inc/me.h
DEPS_34 += build/$(CONFIG)/inc/mpr.h
DEPS_34 += build/$(CONFIG)/inc/http.h
DEPS_34 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_34 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_34 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_34 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_34 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsBoolean.o: \
    src/core/src/ejsBoolean.c $(DEPS_34)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsBoolean.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsBoolean.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsBoolean.c

#
#   ejsByteArray.o
#
DEPS_35 += build/$(CONFIG)/inc/me.h
DEPS_35 += build/$(CONFIG)/inc/mpr.h
DEPS_35 += build/$(CONFIG)/inc/http.h
DEPS_35 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_35 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_35 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_35 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_35 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsByteArray.o: \
    src/core/src/ejsByteArray.c $(DEPS_35)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsByteArray.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsByteArray.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsByteArray.c

#
#   ejsCache.o
#
DEPS_36 += build/$(CONFIG)/inc/me.h
DEPS_36 += build/$(CONFIG)/inc/mpr.h
DEPS_36 += build/$(CONFIG)/inc/http.h
DEPS_36 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_36 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_36 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_36 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_36 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsCache.o: \
    src/core/src/ejsCache.c $(DEPS_36)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsCache.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsCache.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsCache.c

#
#   ejsCmd.o
#
DEPS_37 += build/$(CONFIG)/inc/me.h
DEPS_37 += build/$(CONFIG)/inc/mpr.h
DEPS_37 += build/$(CONFIG)/inc/http.h
DEPS_37 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_37 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_37 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_37 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_37 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsCmd.o: \
    src/core/src/ejsCmd.c $(DEPS_37)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsCmd.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsCmd.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsCmd.c

#
#   ejsConfig.o
#
DEPS_38 += build/$(CONFIG)/inc/me.h
DEPS_38 += build/$(CONFIG)/inc/mpr.h
DEPS_38 += build/$(CONFIG)/inc/http.h
DEPS_38 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_38 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_38 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_38 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_38 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsConfig.o: \
    src/core/src/ejsConfig.c $(DEPS_38)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsConfig.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsConfig.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsConfig.c

#
#   ejsDate.o
#
DEPS_39 += build/$(CONFIG)/inc/me.h
DEPS_39 += build/$(CONFIG)/inc/mpr.h
DEPS_39 += build/$(CONFIG)/inc/http.h
DEPS_39 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_39 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_39 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_39 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_39 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsDate.o: \
    src/core/src/ejsDate.c $(DEPS_39)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsDate.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsDate.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsDate.c

#
#   ejsDebug.o
#
DEPS_40 += build/$(CONFIG)/inc/me.h
DEPS_40 += build/$(CONFIG)/inc/mpr.h
DEPS_40 += build/$(CONFIG)/inc/http.h
DEPS_40 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_40 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_40 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_40 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_40 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsDebug.o: \
    src/core/src/ejsDebug.c $(DEPS_40)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsDebug.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsDebug.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsDebug.c

#
#   ejsError.o
#
DEPS_41 += build/$(CONFIG)/inc/me.h
DEPS_41 += build/$(CONFIG)/inc/mpr.h
DEPS_41 += build/$(CONFIG)/inc/http.h
DEPS_41 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_41 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_41 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_41 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_41 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsError.o: \
    src/core/src/ejsError.c $(DEPS_41)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsError.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsError.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsError.c

#
#   ejsFile.o
#
DEPS_42 += build/$(CONFIG)/inc/me.h
DEPS_42 += build/$(CONFIG)/inc/mpr.h
DEPS_42 += build/$(CONFIG)/inc/http.h
DEPS_42 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_42 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_42 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_42 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_42 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsFile.o: \
    src/core/src/ejsFile.c $(DEPS_42)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsFile.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsFile.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsFile.c

#
#   ejsFileSystem.o
#
DEPS_43 += build/$(CONFIG)/inc/me.h
DEPS_43 += build/$(CONFIG)/inc/mpr.h
DEPS_43 += build/$(CONFIG)/inc/http.h
DEPS_43 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_43 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_43 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_43 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_43 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsFileSystem.o: \
    src/core/src/ejsFileSystem.c $(DEPS_43)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsFileSystem.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsFileSystem.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsFileSystem.c

#
#   ejsFrame.o
#
DEPS_44 += build/$(CONFIG)/inc/me.h
DEPS_44 += build/$(CONFIG)/inc/mpr.h
DEPS_44 += build/$(CONFIG)/inc/http.h
DEPS_44 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_44 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_44 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_44 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_44 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsFrame.o: \
    src/core/src/ejsFrame.c $(DEPS_44)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsFrame.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsFrame.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsFrame.c

#
#   ejsFunction.o
#
DEPS_45 += build/$(CONFIG)/inc/me.h
DEPS_45 += build/$(CONFIG)/inc/mpr.h
DEPS_45 += build/$(CONFIG)/inc/http.h
DEPS_45 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_45 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_45 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_45 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_45 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsFunction.o: \
    src/core/src/ejsFunction.c $(DEPS_45)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsFunction.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsFunction.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsFunction.c

#
#   ejsGC.o
#
DEPS_46 += build/$(CONFIG)/inc/me.h
DEPS_46 += build/$(CONFIG)/inc/mpr.h
DEPS_46 += build/$(CONFIG)/inc/http.h
DEPS_46 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_46 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_46 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_46 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_46 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsGC.o: \
    src/core/src/ejsGC.c $(DEPS_46)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsGC.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsGC.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsGC.c

#
#   ejsGlobal.o
#
DEPS_47 += build/$(CONFIG)/inc/me.h
DEPS_47 += build/$(CONFIG)/inc/mpr.h
DEPS_47 += build/$(CONFIG)/inc/http.h
DEPS_47 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_47 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_47 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_47 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_47 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsGlobal.o: \
    src/core/src/ejsGlobal.c $(DEPS_47)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsGlobal.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsGlobal.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsGlobal.c

#
#   ejsHttp.o
#
DEPS_48 += build/$(CONFIG)/inc/me.h
DEPS_48 += build/$(CONFIG)/inc/mpr.h
DEPS_48 += build/$(CONFIG)/inc/http.h
DEPS_48 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_48 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_48 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_48 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_48 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsHttp.o: \
    src/core/src/ejsHttp.c $(DEPS_48)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsHttp.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsHttp.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsHttp.c

#
#   ejsIterator.o
#
DEPS_49 += build/$(CONFIG)/inc/me.h
DEPS_49 += build/$(CONFIG)/inc/mpr.h
DEPS_49 += build/$(CONFIG)/inc/http.h
DEPS_49 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_49 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_49 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_49 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_49 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsIterator.o: \
    src/core/src/ejsIterator.c $(DEPS_49)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsIterator.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsIterator.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsIterator.c

#
#   ejsJSON.o
#
DEPS_50 += build/$(CONFIG)/inc/me.h
DEPS_50 += build/$(CONFIG)/inc/mpr.h
DEPS_50 += build/$(CONFIG)/inc/http.h
DEPS_50 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_50 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_50 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_50 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_50 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsJSON.o: \
    src/core/src/ejsJSON.c $(DEPS_50)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsJSON.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsJSON.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsJSON.c

#
#   ejsLocalCache.o
#
DEPS_51 += build/$(CONFIG)/inc/me.h
DEPS_51 += build/$(CONFIG)/inc/mpr.h
DEPS_51 += build/$(CONFIG)/inc/http.h
DEPS_51 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_51 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_51 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_51 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_51 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsLocalCache.o: \
    src/core/src/ejsLocalCache.c $(DEPS_51)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsLocalCache.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsLocalCache.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsLocalCache.c

#
#   ejsMath.o
#
DEPS_52 += build/$(CONFIG)/inc/me.h
DEPS_52 += build/$(CONFIG)/inc/mpr.h
DEPS_52 += build/$(CONFIG)/inc/http.h
DEPS_52 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_52 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_52 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_52 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_52 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsMath.o: \
    src/core/src/ejsMath.c $(DEPS_52)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsMath.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsMath.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsMath.c

#
#   ejsMemory.o
#
DEPS_53 += build/$(CONFIG)/inc/me.h
DEPS_53 += build/$(CONFIG)/inc/mpr.h
DEPS_53 += build/$(CONFIG)/inc/http.h
DEPS_53 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_53 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_53 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_53 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_53 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsMemory.o: \
    src/core/src/ejsMemory.c $(DEPS_53)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsMemory.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsMemory.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsMemory.c

#
#   ejsMprLog.o
#
DEPS_54 += build/$(CONFIG)/inc/me.h
DEPS_54 += build/$(CONFIG)/inc/mpr.h
DEPS_54 += build/$(CONFIG)/inc/http.h
DEPS_54 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_54 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_54 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_54 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_54 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsMprLog.o: \
    src/core/src/ejsMprLog.c $(DEPS_54)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsMprLog.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsMprLog.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsMprLog.c

#
#   ejsNamespace.o
#
DEPS_55 += build/$(CONFIG)/inc/me.h
DEPS_55 += build/$(CONFIG)/inc/mpr.h
DEPS_55 += build/$(CONFIG)/inc/http.h
DEPS_55 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_55 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_55 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_55 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_55 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsNamespace.o: \
    src/core/src/ejsNamespace.c $(DEPS_55)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsNamespace.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsNamespace.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsNamespace.c

#
#   ejsNull.o
#
DEPS_56 += build/$(CONFIG)/inc/me.h
DEPS_56 += build/$(CONFIG)/inc/mpr.h
DEPS_56 += build/$(CONFIG)/inc/http.h
DEPS_56 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_56 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_56 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_56 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_56 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsNull.o: \
    src/core/src/ejsNull.c $(DEPS_56)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsNull.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsNull.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsNull.c

#
#   ejsNumber.o
#
DEPS_57 += build/$(CONFIG)/inc/me.h
DEPS_57 += build/$(CONFIG)/inc/mpr.h
DEPS_57 += build/$(CONFIG)/inc/http.h
DEPS_57 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_57 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_57 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_57 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_57 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsNumber.o: \
    src/core/src/ejsNumber.c $(DEPS_57)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsNumber.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsNumber.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsNumber.c

#
#   ejsObject.o
#
DEPS_58 += build/$(CONFIG)/inc/me.h
DEPS_58 += build/$(CONFIG)/inc/mpr.h
DEPS_58 += build/$(CONFIG)/inc/http.h
DEPS_58 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_58 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_58 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_58 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_58 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsObject.o: \
    src/core/src/ejsObject.c $(DEPS_58)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsObject.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsObject.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsObject.c

#
#   ejsPath.o
#
DEPS_59 += build/$(CONFIG)/inc/me.h
DEPS_59 += build/$(CONFIG)/inc/mpr.h
DEPS_59 += build/$(CONFIG)/inc/http.h
DEPS_59 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_59 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_59 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_59 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_59 += build/$(CONFIG)/inc/ejs.h
DEPS_59 += build/$(CONFIG)/inc/pcre.h

build/$(CONFIG)/obj/ejsPath.o: \
    src/core/src/ejsPath.c $(DEPS_59)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsPath.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsPath.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsPath.c

#
#   ejsPot.o
#
DEPS_60 += build/$(CONFIG)/inc/me.h
DEPS_60 += build/$(CONFIG)/inc/mpr.h
DEPS_60 += build/$(CONFIG)/inc/http.h
DEPS_60 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_60 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_60 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_60 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_60 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsPot.o: \
    src/core/src/ejsPot.c $(DEPS_60)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsPot.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsPot.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsPot.c

#
#   ejsRegExp.o
#
DEPS_61 += build/$(CONFIG)/inc/me.h
DEPS_61 += build/$(CONFIG)/inc/mpr.h
DEPS_61 += build/$(CONFIG)/inc/http.h
DEPS_61 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_61 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_61 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_61 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_61 += build/$(CONFIG)/inc/ejs.h
DEPS_61 += build/$(CONFIG)/inc/pcre.h

build/$(CONFIG)/obj/ejsRegExp.o: \
    src/core/src/ejsRegExp.c $(DEPS_61)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsRegExp.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsRegExp.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsRegExp.c

#
#   ejsSocket.o
#
DEPS_62 += build/$(CONFIG)/inc/me.h
DEPS_62 += build/$(CONFIG)/inc/mpr.h
DEPS_62 += build/$(CONFIG)/inc/http.h
DEPS_62 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_62 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_62 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_62 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_62 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsSocket.o: \
    src/core/src/ejsSocket.c $(DEPS_62)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsSocket.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsSocket.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsSocket.c

#
#   ejsString.o
#
DEPS_63 += build/$(CONFIG)/inc/me.h
DEPS_63 += build/$(CONFIG)/inc/mpr.h
DEPS_63 += build/$(CONFIG)/inc/http.h
DEPS_63 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_63 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_63 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_63 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_63 += build/$(CONFIG)/inc/ejs.h
DEPS_63 += build/$(CONFIG)/inc/pcre.h

build/$(CONFIG)/obj/ejsString.o: \
    src/core/src/ejsString.c $(DEPS_63)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsString.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsString.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsString.c

#
#   ejsSystem.o
#
DEPS_64 += build/$(CONFIG)/inc/me.h
DEPS_64 += build/$(CONFIG)/inc/mpr.h
DEPS_64 += build/$(CONFIG)/inc/http.h
DEPS_64 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_64 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_64 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_64 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_64 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsSystem.o: \
    src/core/src/ejsSystem.c $(DEPS_64)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsSystem.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsSystem.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsSystem.c

#
#   ejsTimer.o
#
DEPS_65 += build/$(CONFIG)/inc/me.h
DEPS_65 += build/$(CONFIG)/inc/mpr.h
DEPS_65 += build/$(CONFIG)/inc/http.h
DEPS_65 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_65 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_65 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_65 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_65 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsTimer.o: \
    src/core/src/ejsTimer.c $(DEPS_65)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsTimer.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsTimer.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsTimer.c

#
#   ejsType.o
#
DEPS_66 += build/$(CONFIG)/inc/me.h
DEPS_66 += build/$(CONFIG)/inc/mpr.h
DEPS_66 += build/$(CONFIG)/inc/http.h
DEPS_66 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_66 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_66 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_66 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_66 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsType.o: \
    src/core/src/ejsType.c $(DEPS_66)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsType.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsType.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsType.c

#
#   ejsUri.o
#
DEPS_67 += build/$(CONFIG)/inc/me.h
DEPS_67 += build/$(CONFIG)/inc/mpr.h
DEPS_67 += build/$(CONFIG)/inc/http.h
DEPS_67 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_67 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_67 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_67 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_67 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsUri.o: \
    src/core/src/ejsUri.c $(DEPS_67)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsUri.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsUri.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsUri.c

#
#   ejsVoid.o
#
DEPS_68 += build/$(CONFIG)/inc/me.h
DEPS_68 += build/$(CONFIG)/inc/mpr.h
DEPS_68 += build/$(CONFIG)/inc/http.h
DEPS_68 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_68 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_68 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_68 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_68 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsVoid.o: \
    src/core/src/ejsVoid.c $(DEPS_68)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsVoid.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsVoid.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsVoid.c

#
#   ejsWebSocket.o
#
DEPS_69 += build/$(CONFIG)/inc/me.h
DEPS_69 += build/$(CONFIG)/inc/mpr.h
DEPS_69 += build/$(CONFIG)/inc/http.h
DEPS_69 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_69 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_69 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_69 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_69 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsWebSocket.o: \
    src/core/src/ejsWebSocket.c $(DEPS_69)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsWebSocket.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsWebSocket.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsWebSocket.c

#
#   ejsWorker.o
#
DEPS_70 += build/$(CONFIG)/inc/me.h
DEPS_70 += build/$(CONFIG)/inc/mpr.h
DEPS_70 += build/$(CONFIG)/inc/http.h
DEPS_70 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_70 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_70 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_70 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_70 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsWorker.o: \
    src/core/src/ejsWorker.c $(DEPS_70)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsWorker.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsWorker.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsWorker.c

#
#   ejsXML.o
#
DEPS_71 += build/$(CONFIG)/inc/me.h
DEPS_71 += build/$(CONFIG)/inc/mpr.h
DEPS_71 += build/$(CONFIG)/inc/http.h
DEPS_71 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_71 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_71 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_71 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_71 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsXML.o: \
    src/core/src/ejsXML.c $(DEPS_71)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsXML.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsXML.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsXML.c

#
#   ejsXMLList.o
#
DEPS_72 += build/$(CONFIG)/inc/me.h
DEPS_72 += build/$(CONFIG)/inc/mpr.h
DEPS_72 += build/$(CONFIG)/inc/http.h
DEPS_72 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_72 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_72 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_72 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_72 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsXMLList.o: \
    src/core/src/ejsXMLList.c $(DEPS_72)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsXMLList.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsXMLList.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsXMLList.c

#
#   ejsXMLLoader.o
#
DEPS_73 += build/$(CONFIG)/inc/me.h
DEPS_73 += build/$(CONFIG)/inc/mpr.h
DEPS_73 += build/$(CONFIG)/inc/http.h
DEPS_73 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_73 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_73 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_73 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_73 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsXMLLoader.o: \
    src/core/src/ejsXMLLoader.c $(DEPS_73)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsXMLLoader.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsXMLLoader.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/core/src/ejsXMLLoader.c

#
#   ejsByteCode.o
#
DEPS_74 += build/$(CONFIG)/inc/me.h
DEPS_74 += build/$(CONFIG)/inc/mpr.h
DEPS_74 += build/$(CONFIG)/inc/http.h
DEPS_74 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_74 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_74 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_74 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_74 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsByteCode.o: \
    src/vm/ejsByteCode.c $(DEPS_74)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsByteCode.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsByteCode.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/vm/ejsByteCode.c

#
#   ejsException.o
#
DEPS_75 += build/$(CONFIG)/inc/me.h
DEPS_75 += build/$(CONFIG)/inc/mpr.h
DEPS_75 += build/$(CONFIG)/inc/http.h
DEPS_75 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_75 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_75 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_75 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_75 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsException.o: \
    src/vm/ejsException.c $(DEPS_75)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsException.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsException.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/vm/ejsException.c

#
#   ejsHelper.o
#
DEPS_76 += build/$(CONFIG)/inc/me.h
DEPS_76 += build/$(CONFIG)/inc/mpr.h
DEPS_76 += build/$(CONFIG)/inc/http.h
DEPS_76 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_76 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_76 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_76 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_76 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsHelper.o: \
    src/vm/ejsHelper.c $(DEPS_76)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsHelper.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsHelper.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/vm/ejsHelper.c

#
#   ejsInterp.o
#
DEPS_77 += build/$(CONFIG)/inc/me.h
DEPS_77 += build/$(CONFIG)/inc/mpr.h
DEPS_77 += build/$(CONFIG)/inc/http.h
DEPS_77 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_77 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_77 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_77 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_77 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsInterp.o: \
    src/vm/ejsInterp.c $(DEPS_77)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsInterp.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsInterp.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/vm/ejsInterp.c

#
#   ejsLoader.o
#
DEPS_78 += build/$(CONFIG)/inc/me.h
DEPS_78 += build/$(CONFIG)/inc/mpr.h
DEPS_78 += build/$(CONFIG)/inc/http.h
DEPS_78 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_78 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_78 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_78 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_78 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsLoader.o: \
    src/vm/ejsLoader.c $(DEPS_78)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsLoader.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsLoader.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/vm/ejsLoader.c

#
#   ejsModule.o
#
DEPS_79 += build/$(CONFIG)/inc/me.h
DEPS_79 += build/$(CONFIG)/inc/mpr.h
DEPS_79 += build/$(CONFIG)/inc/http.h
DEPS_79 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_79 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_79 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_79 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_79 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsModule.o: \
    src/vm/ejsModule.c $(DEPS_79)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsModule.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsModule.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/vm/ejsModule.c

#
#   ejsScope.o
#
DEPS_80 += build/$(CONFIG)/inc/me.h
DEPS_80 += build/$(CONFIG)/inc/mpr.h
DEPS_80 += build/$(CONFIG)/inc/http.h
DEPS_80 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_80 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_80 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_80 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_80 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsScope.o: \
    src/vm/ejsScope.c $(DEPS_80)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsScope.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsScope.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/vm/ejsScope.c

#
#   ejsService.o
#
DEPS_81 += build/$(CONFIG)/inc/me.h
DEPS_81 += build/$(CONFIG)/inc/mpr.h
DEPS_81 += build/$(CONFIG)/inc/http.h
DEPS_81 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_81 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_81 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_81 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_81 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsService.o: \
    src/vm/ejsService.c $(DEPS_81)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsService.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsService.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/vm/ejsService.c

#
#   libejs
#
DEPS_82 += slots
DEPS_82 += build/$(CONFIG)/inc/mpr.h
DEPS_82 += build/$(CONFIG)/inc/me.h
DEPS_82 += build/$(CONFIG)/inc/osdep.h
DEPS_82 += build/$(CONFIG)/obj/mprLib.o
DEPS_82 += build/$(CONFIG)/bin/libmpr.a
DEPS_82 += build/$(CONFIG)/inc/pcre.h
DEPS_82 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_82 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_82 += build/$(CONFIG)/inc/http.h
DEPS_82 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_82 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_82 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_82 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_82 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_82 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_82 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_82 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_82 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_82 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_82 += build/$(CONFIG)/inc/ejs.h
DEPS_82 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_82 += build/$(CONFIG)/obj/ecAst.o
DEPS_82 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_82 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_82 += build/$(CONFIG)/obj/ecLex.o
DEPS_82 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_82 += build/$(CONFIG)/obj/ecParser.o
DEPS_82 += build/$(CONFIG)/obj/ecState.o
DEPS_82 += build/$(CONFIG)/obj/dtoa.o
DEPS_82 += build/$(CONFIG)/obj/ejsApp.o
DEPS_82 += build/$(CONFIG)/obj/ejsArray.o
DEPS_82 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_82 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_82 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_82 += build/$(CONFIG)/obj/ejsCache.o
DEPS_82 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_82 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_82 += build/$(CONFIG)/obj/ejsDate.o
DEPS_82 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_82 += build/$(CONFIG)/obj/ejsError.o
DEPS_82 += build/$(CONFIG)/obj/ejsFile.o
DEPS_82 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_82 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_82 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_82 += build/$(CONFIG)/obj/ejsGC.o
DEPS_82 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_82 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_82 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_82 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_82 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_82 += build/$(CONFIG)/obj/ejsMath.o
DEPS_82 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_82 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_82 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_82 += build/$(CONFIG)/obj/ejsNull.o
DEPS_82 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_82 += build/$(CONFIG)/obj/ejsObject.o
DEPS_82 += build/$(CONFIG)/obj/ejsPath.o
DEPS_82 += build/$(CONFIG)/obj/ejsPot.o
DEPS_82 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_82 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_82 += build/$(CONFIG)/obj/ejsString.o
DEPS_82 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_82 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_82 += build/$(CONFIG)/obj/ejsType.o
DEPS_82 += build/$(CONFIG)/obj/ejsUri.o
DEPS_82 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_82 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_82 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_82 += build/$(CONFIG)/obj/ejsXML.o
DEPS_82 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_82 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_82 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_82 += build/$(CONFIG)/obj/ejsException.o
DEPS_82 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_82 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_82 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_82 += build/$(CONFIG)/obj/ejsModule.o
DEPS_82 += build/$(CONFIG)/obj/ejsScope.o
DEPS_82 += build/$(CONFIG)/obj/ejsService.o

build/$(CONFIG)/bin/libejs.a: $(DEPS_82)
	@echo '      [Link] build/$(CONFIG)/bin/libejs.a'
	ar -cr build/$(CONFIG)/bin/libejs.a "build/$(CONFIG)/obj/ecAst.o" "build/$(CONFIG)/obj/ecCodeGen.o" "build/$(CONFIG)/obj/ecCompiler.o" "build/$(CONFIG)/obj/ecLex.o" "build/$(CONFIG)/obj/ecModuleWrite.o" "build/$(CONFIG)/obj/ecParser.o" "build/$(CONFIG)/obj/ecState.o" "build/$(CONFIG)/obj/dtoa.o" "build/$(CONFIG)/obj/ejsApp.o" "build/$(CONFIG)/obj/ejsArray.o" "build/$(CONFIG)/obj/ejsBlock.o" "build/$(CONFIG)/obj/ejsBoolean.o" "build/$(CONFIG)/obj/ejsByteArray.o" "build/$(CONFIG)/obj/ejsCache.o" "build/$(CONFIG)/obj/ejsCmd.o" "build/$(CONFIG)/obj/ejsConfig.o" "build/$(CONFIG)/obj/ejsDate.o" "build/$(CONFIG)/obj/ejsDebug.o" "build/$(CONFIG)/obj/ejsError.o" "build/$(CONFIG)/obj/ejsFile.o" "build/$(CONFIG)/obj/ejsFileSystem.o" "build/$(CONFIG)/obj/ejsFrame.o" "build/$(CONFIG)/obj/ejsFunction.o" "build/$(CONFIG)/obj/ejsGC.o" "build/$(CONFIG)/obj/ejsGlobal.o" "build/$(CONFIG)/obj/ejsHttp.o" "build/$(CONFIG)/obj/ejsIterator.o" "build/$(CONFIG)/obj/ejsJSON.o" "build/$(CONFIG)/obj/ejsLocalCache.o" "build/$(CONFIG)/obj/ejsMath.o" "build/$(CONFIG)/obj/ejsMemory.o" "build/$(CONFIG)/obj/ejsMprLog.o" "build/$(CONFIG)/obj/ejsNamespace.o" "build/$(CONFIG)/obj/ejsNull.o" "build/$(CONFIG)/obj/ejsNumber.o" "build/$(CONFIG)/obj/ejsObject.o" "build/$(CONFIG)/obj/ejsPath.o" "build/$(CONFIG)/obj/ejsPot.o" "build/$(CONFIG)/obj/ejsRegExp.o" "build/$(CONFIG)/obj/ejsSocket.o" "build/$(CONFIG)/obj/ejsString.o" "build/$(CONFIG)/obj/ejsSystem.o" "build/$(CONFIG)/obj/ejsTimer.o" "build/$(CONFIG)/obj/ejsType.o" "build/$(CONFIG)/obj/ejsUri.o" "build/$(CONFIG)/obj/ejsVoid.o" "build/$(CONFIG)/obj/ejsWebSocket.o" "build/$(CONFIG)/obj/ejsWorker.o" "build/$(CONFIG)/obj/ejsXML.o" "build/$(CONFIG)/obj/ejsXMLList.o" "build/$(CONFIG)/obj/ejsXMLLoader.o" "build/$(CONFIG)/obj/ejsByteCode.o" "build/$(CONFIG)/obj/ejsException.o" "build/$(CONFIG)/obj/ejsHelper.o" "build/$(CONFIG)/obj/ejsInterp.o" "build/$(CONFIG)/obj/ejsLoader.o" "build/$(CONFIG)/obj/ejsModule.o" "build/$(CONFIG)/obj/ejsScope.o" "build/$(CONFIG)/obj/ejsService.o"

#
#   ejs.o
#
DEPS_83 += build/$(CONFIG)/inc/me.h
DEPS_83 += build/$(CONFIG)/inc/ejsCompiler.h

build/$(CONFIG)/obj/ejs.o: \
    src/cmd/ejs.c $(DEPS_83)
	@echo '   [Compile] build/$(CONFIG)/obj/ejs.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejs.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/cmd/ejs.c

#
#   ejs
#
DEPS_84 += slots
DEPS_84 += build/$(CONFIG)/inc/mpr.h
DEPS_84 += build/$(CONFIG)/inc/me.h
DEPS_84 += build/$(CONFIG)/inc/osdep.h
DEPS_84 += build/$(CONFIG)/obj/mprLib.o
DEPS_84 += build/$(CONFIG)/bin/libmpr.a
DEPS_84 += build/$(CONFIG)/inc/pcre.h
DEPS_84 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_84 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_84 += build/$(CONFIG)/inc/http.h
DEPS_84 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_84 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_84 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_84 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_84 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_84 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_84 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_84 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_84 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_84 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_84 += build/$(CONFIG)/inc/ejs.h
DEPS_84 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_84 += build/$(CONFIG)/obj/ecAst.o
DEPS_84 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_84 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_84 += build/$(CONFIG)/obj/ecLex.o
DEPS_84 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_84 += build/$(CONFIG)/obj/ecParser.o
DEPS_84 += build/$(CONFIG)/obj/ecState.o
DEPS_84 += build/$(CONFIG)/obj/dtoa.o
DEPS_84 += build/$(CONFIG)/obj/ejsApp.o
DEPS_84 += build/$(CONFIG)/obj/ejsArray.o
DEPS_84 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_84 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_84 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_84 += build/$(CONFIG)/obj/ejsCache.o
DEPS_84 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_84 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_84 += build/$(CONFIG)/obj/ejsDate.o
DEPS_84 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_84 += build/$(CONFIG)/obj/ejsError.o
DEPS_84 += build/$(CONFIG)/obj/ejsFile.o
DEPS_84 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_84 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_84 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_84 += build/$(CONFIG)/obj/ejsGC.o
DEPS_84 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_84 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_84 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_84 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_84 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_84 += build/$(CONFIG)/obj/ejsMath.o
DEPS_84 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_84 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_84 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_84 += build/$(CONFIG)/obj/ejsNull.o
DEPS_84 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_84 += build/$(CONFIG)/obj/ejsObject.o
DEPS_84 += build/$(CONFIG)/obj/ejsPath.o
DEPS_84 += build/$(CONFIG)/obj/ejsPot.o
DEPS_84 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_84 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_84 += build/$(CONFIG)/obj/ejsString.o
DEPS_84 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_84 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_84 += build/$(CONFIG)/obj/ejsType.o
DEPS_84 += build/$(CONFIG)/obj/ejsUri.o
DEPS_84 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_84 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_84 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_84 += build/$(CONFIG)/obj/ejsXML.o
DEPS_84 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_84 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_84 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_84 += build/$(CONFIG)/obj/ejsException.o
DEPS_84 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_84 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_84 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_84 += build/$(CONFIG)/obj/ejsModule.o
DEPS_84 += build/$(CONFIG)/obj/ejsScope.o
DEPS_84 += build/$(CONFIG)/obj/ejsService.o
DEPS_84 += build/$(CONFIG)/bin/libejs.a
DEPS_84 += build/$(CONFIG)/obj/ejs.o

LIBS_84 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_84 += -lhttp
endif
LIBS_84 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_84 += -lpcre
endif

build/$(CONFIG)/bin/ejs: $(DEPS_84)
	@echo '      [Link] build/$(CONFIG)/bin/ejs'
	$(CC) -o build/$(CONFIG)/bin/ejs -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/ejs.o" $(LIBPATHS_84) $(LIBS_84) $(LIBS_84) $(LIBS) -lpam -ledit 

#
#   ejsc.o
#
DEPS_85 += build/$(CONFIG)/inc/me.h
DEPS_85 += build/$(CONFIG)/inc/ejsCompiler.h

build/$(CONFIG)/obj/ejsc.o: \
    src/cmd/ejsc.c $(DEPS_85)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsc.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsc.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/cmd/ejsc.c

#
#   ejsc
#
DEPS_86 += slots
DEPS_86 += build/$(CONFIG)/inc/mpr.h
DEPS_86 += build/$(CONFIG)/inc/me.h
DEPS_86 += build/$(CONFIG)/inc/osdep.h
DEPS_86 += build/$(CONFIG)/obj/mprLib.o
DEPS_86 += build/$(CONFIG)/bin/libmpr.a
DEPS_86 += build/$(CONFIG)/inc/pcre.h
DEPS_86 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_86 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_86 += build/$(CONFIG)/inc/http.h
DEPS_86 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_86 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_86 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_86 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_86 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_86 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_86 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_86 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_86 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_86 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_86 += build/$(CONFIG)/inc/ejs.h
DEPS_86 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_86 += build/$(CONFIG)/obj/ecAst.o
DEPS_86 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_86 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_86 += build/$(CONFIG)/obj/ecLex.o
DEPS_86 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_86 += build/$(CONFIG)/obj/ecParser.o
DEPS_86 += build/$(CONFIG)/obj/ecState.o
DEPS_86 += build/$(CONFIG)/obj/dtoa.o
DEPS_86 += build/$(CONFIG)/obj/ejsApp.o
DEPS_86 += build/$(CONFIG)/obj/ejsArray.o
DEPS_86 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_86 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_86 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_86 += build/$(CONFIG)/obj/ejsCache.o
DEPS_86 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_86 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_86 += build/$(CONFIG)/obj/ejsDate.o
DEPS_86 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_86 += build/$(CONFIG)/obj/ejsError.o
DEPS_86 += build/$(CONFIG)/obj/ejsFile.o
DEPS_86 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_86 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_86 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_86 += build/$(CONFIG)/obj/ejsGC.o
DEPS_86 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_86 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_86 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_86 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_86 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_86 += build/$(CONFIG)/obj/ejsMath.o
DEPS_86 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_86 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_86 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_86 += build/$(CONFIG)/obj/ejsNull.o
DEPS_86 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_86 += build/$(CONFIG)/obj/ejsObject.o
DEPS_86 += build/$(CONFIG)/obj/ejsPath.o
DEPS_86 += build/$(CONFIG)/obj/ejsPot.o
DEPS_86 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_86 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_86 += build/$(CONFIG)/obj/ejsString.o
DEPS_86 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_86 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_86 += build/$(CONFIG)/obj/ejsType.o
DEPS_86 += build/$(CONFIG)/obj/ejsUri.o
DEPS_86 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_86 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_86 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_86 += build/$(CONFIG)/obj/ejsXML.o
DEPS_86 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_86 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_86 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_86 += build/$(CONFIG)/obj/ejsException.o
DEPS_86 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_86 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_86 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_86 += build/$(CONFIG)/obj/ejsModule.o
DEPS_86 += build/$(CONFIG)/obj/ejsScope.o
DEPS_86 += build/$(CONFIG)/obj/ejsService.o
DEPS_86 += build/$(CONFIG)/bin/libejs.a
DEPS_86 += build/$(CONFIG)/obj/ejsc.o

LIBS_86 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_86 += -lhttp
endif
LIBS_86 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_86 += -lpcre
endif

build/$(CONFIG)/bin/ejsc: $(DEPS_86)
	@echo '      [Link] build/$(CONFIG)/bin/ejsc'
	$(CC) -o build/$(CONFIG)/bin/ejsc -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/ejsc.o" $(LIBPATHS_86) $(LIBS_86) $(LIBS_86) $(LIBS) -lpam 

#
#   ejsmod.h
#
src/cmd/ejsmod.h: $(DEPS_87)
	@echo '      [Copy] src/cmd/ejsmod.h'

#
#   ejsmod.o
#
DEPS_88 += build/$(CONFIG)/inc/me.h
DEPS_88 += src/cmd/ejsmod.h
DEPS_88 += build/$(CONFIG)/inc/mpr.h
DEPS_88 += build/$(CONFIG)/inc/http.h
DEPS_88 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_88 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_88 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_88 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_88 += build/$(CONFIG)/inc/ejs.h

build/$(CONFIG)/obj/ejsmod.o: \
    src/cmd/ejsmod.c $(DEPS_88)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsmod.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsmod.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/ejsmod.c

#
#   doc.o
#
DEPS_89 += build/$(CONFIG)/inc/me.h
DEPS_89 += src/cmd/ejsmod.h

build/$(CONFIG)/obj/doc.o: \
    src/cmd/doc.c $(DEPS_89)
	@echo '   [Compile] build/$(CONFIG)/obj/doc.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/doc.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/doc.c

#
#   docFiles.o
#
DEPS_90 += build/$(CONFIG)/inc/me.h
DEPS_90 += src/cmd/ejsmod.h

build/$(CONFIG)/obj/docFiles.o: \
    src/cmd/docFiles.c $(DEPS_90)
	@echo '   [Compile] build/$(CONFIG)/obj/docFiles.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/docFiles.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/docFiles.c

#
#   listing.o
#
DEPS_91 += build/$(CONFIG)/inc/me.h
DEPS_91 += src/cmd/ejsmod.h
DEPS_91 += build/$(CONFIG)/inc/ejsByteCodeTable.h

build/$(CONFIG)/obj/listing.o: \
    src/cmd/listing.c $(DEPS_91)
	@echo '   [Compile] build/$(CONFIG)/obj/listing.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/listing.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/listing.c

#
#   slotGen.o
#
DEPS_92 += build/$(CONFIG)/inc/me.h
DEPS_92 += src/cmd/ejsmod.h

build/$(CONFIG)/obj/slotGen.o: \
    src/cmd/slotGen.c $(DEPS_92)
	@echo '   [Compile] build/$(CONFIG)/obj/slotGen.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/slotGen.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/slotGen.c

#
#   ejsmod
#
DEPS_93 += slots
DEPS_93 += build/$(CONFIG)/inc/mpr.h
DEPS_93 += build/$(CONFIG)/inc/me.h
DEPS_93 += build/$(CONFIG)/inc/osdep.h
DEPS_93 += build/$(CONFIG)/obj/mprLib.o
DEPS_93 += build/$(CONFIG)/bin/libmpr.a
DEPS_93 += build/$(CONFIG)/inc/pcre.h
DEPS_93 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_93 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_93 += build/$(CONFIG)/inc/http.h
DEPS_93 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_93 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_93 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_93 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_93 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_93 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_93 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_93 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_93 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_93 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_93 += build/$(CONFIG)/inc/ejs.h
DEPS_93 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_93 += build/$(CONFIG)/obj/ecAst.o
DEPS_93 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_93 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_93 += build/$(CONFIG)/obj/ecLex.o
DEPS_93 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_93 += build/$(CONFIG)/obj/ecParser.o
DEPS_93 += build/$(CONFIG)/obj/ecState.o
DEPS_93 += build/$(CONFIG)/obj/dtoa.o
DEPS_93 += build/$(CONFIG)/obj/ejsApp.o
DEPS_93 += build/$(CONFIG)/obj/ejsArray.o
DEPS_93 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_93 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_93 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_93 += build/$(CONFIG)/obj/ejsCache.o
DEPS_93 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_93 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_93 += build/$(CONFIG)/obj/ejsDate.o
DEPS_93 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_93 += build/$(CONFIG)/obj/ejsError.o
DEPS_93 += build/$(CONFIG)/obj/ejsFile.o
DEPS_93 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_93 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_93 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_93 += build/$(CONFIG)/obj/ejsGC.o
DEPS_93 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_93 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_93 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_93 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_93 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_93 += build/$(CONFIG)/obj/ejsMath.o
DEPS_93 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_93 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_93 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_93 += build/$(CONFIG)/obj/ejsNull.o
DEPS_93 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_93 += build/$(CONFIG)/obj/ejsObject.o
DEPS_93 += build/$(CONFIG)/obj/ejsPath.o
DEPS_93 += build/$(CONFIG)/obj/ejsPot.o
DEPS_93 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_93 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_93 += build/$(CONFIG)/obj/ejsString.o
DEPS_93 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_93 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_93 += build/$(CONFIG)/obj/ejsType.o
DEPS_93 += build/$(CONFIG)/obj/ejsUri.o
DEPS_93 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_93 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_93 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_93 += build/$(CONFIG)/obj/ejsXML.o
DEPS_93 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_93 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_93 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_93 += build/$(CONFIG)/obj/ejsException.o
DEPS_93 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_93 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_93 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_93 += build/$(CONFIG)/obj/ejsModule.o
DEPS_93 += build/$(CONFIG)/obj/ejsScope.o
DEPS_93 += build/$(CONFIG)/obj/ejsService.o
DEPS_93 += build/$(CONFIG)/bin/libejs.a
DEPS_93 += src/cmd/ejsmod.h
DEPS_93 += build/$(CONFIG)/obj/ejsmod.o
DEPS_93 += build/$(CONFIG)/obj/doc.o
DEPS_93 += build/$(CONFIG)/obj/docFiles.o
DEPS_93 += build/$(CONFIG)/obj/listing.o
DEPS_93 += build/$(CONFIG)/obj/slotGen.o

LIBS_93 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_93 += -lhttp
endif
LIBS_93 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_93 += -lpcre
endif

build/$(CONFIG)/bin/ejsmod: $(DEPS_93)
	@echo '      [Link] build/$(CONFIG)/bin/ejsmod'
	$(CC) -o build/$(CONFIG)/bin/ejsmod -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/ejsmod.o" "build/$(CONFIG)/obj/doc.o" "build/$(CONFIG)/obj/docFiles.o" "build/$(CONFIG)/obj/listing.o" "build/$(CONFIG)/obj/slotGen.o" $(LIBPATHS_93) $(LIBS_93) $(LIBS_93) $(LIBS) -lpam 

#
#   ejs.mod
#
DEPS_94 += src/core/App.es
DEPS_94 += src/core/Args.es
DEPS_94 += src/core/Array.es
DEPS_94 += src/core/BinaryStream.es
DEPS_94 += src/core/Block.es
DEPS_94 += src/core/Boolean.es
DEPS_94 += src/core/ByteArray.es
DEPS_94 += src/core/Cache.es
DEPS_94 += src/core/Cmd.es
DEPS_94 += src/core/Compat.es
DEPS_94 += src/core/Config.es
DEPS_94 += src/core/Date.es
DEPS_94 += src/core/Debug.es
DEPS_94 += src/core/Emitter.es
DEPS_94 += src/core/Error.es
DEPS_94 += src/core/File.es
DEPS_94 += src/core/FileSystem.es
DEPS_94 += src/core/Frame.es
DEPS_94 += src/core/Function.es
DEPS_94 += src/core/GC.es
DEPS_94 += src/core/Global.es
DEPS_94 += src/core/Http.es
DEPS_94 += src/core/Inflector.es
DEPS_94 += src/core/Iterator.es
DEPS_94 += src/core/JSON.es
DEPS_94 += src/core/Loader.es
DEPS_94 += src/core/LocalCache.es
DEPS_94 += src/core/Locale.es
DEPS_94 += src/core/Logger.es
DEPS_94 += src/core/Math.es
DEPS_94 += src/core/Memory.es
DEPS_94 += src/core/MprLog.es
DEPS_94 += src/core/Name.es
DEPS_94 += src/core/Namespace.es
DEPS_94 += src/core/Null.es
DEPS_94 += src/core/Number.es
DEPS_94 += src/core/Object.es
DEPS_94 += src/core/Path.es
DEPS_94 += src/core/Promise.es
DEPS_94 += src/core/RegExp.es
DEPS_94 += src/core/Socket.es
DEPS_94 += src/core/Stream.es
DEPS_94 += src/core/String.es
DEPS_94 += src/core/System.es
DEPS_94 += src/core/TextStream.es
DEPS_94 += src/core/Timer.es
DEPS_94 += src/core/Type.es
DEPS_94 += src/core/Uri.es
DEPS_94 += src/core/Void.es
DEPS_94 += src/core/WebSocket.es
DEPS_94 += src/core/Worker.es
DEPS_94 += src/core/XML.es
DEPS_94 += src/core/XMLHttp.es
DEPS_94 += src/core/XMLList.es
DEPS_94 += slots
DEPS_94 += build/$(CONFIG)/inc/mpr.h
DEPS_94 += build/$(CONFIG)/inc/me.h
DEPS_94 += build/$(CONFIG)/inc/osdep.h
DEPS_94 += build/$(CONFIG)/obj/mprLib.o
DEPS_94 += build/$(CONFIG)/bin/libmpr.a
DEPS_94 += build/$(CONFIG)/inc/pcre.h
DEPS_94 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_94 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_94 += build/$(CONFIG)/inc/http.h
DEPS_94 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_94 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_94 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_94 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_94 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_94 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_94 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_94 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_94 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_94 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_94 += build/$(CONFIG)/inc/ejs.h
DEPS_94 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_94 += build/$(CONFIG)/obj/ecAst.o
DEPS_94 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_94 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_94 += build/$(CONFIG)/obj/ecLex.o
DEPS_94 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_94 += build/$(CONFIG)/obj/ecParser.o
DEPS_94 += build/$(CONFIG)/obj/ecState.o
DEPS_94 += build/$(CONFIG)/obj/dtoa.o
DEPS_94 += build/$(CONFIG)/obj/ejsApp.o
DEPS_94 += build/$(CONFIG)/obj/ejsArray.o
DEPS_94 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_94 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_94 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_94 += build/$(CONFIG)/obj/ejsCache.o
DEPS_94 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_94 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_94 += build/$(CONFIG)/obj/ejsDate.o
DEPS_94 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_94 += build/$(CONFIG)/obj/ejsError.o
DEPS_94 += build/$(CONFIG)/obj/ejsFile.o
DEPS_94 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_94 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_94 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_94 += build/$(CONFIG)/obj/ejsGC.o
DEPS_94 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_94 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_94 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_94 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_94 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_94 += build/$(CONFIG)/obj/ejsMath.o
DEPS_94 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_94 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_94 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_94 += build/$(CONFIG)/obj/ejsNull.o
DEPS_94 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_94 += build/$(CONFIG)/obj/ejsObject.o
DEPS_94 += build/$(CONFIG)/obj/ejsPath.o
DEPS_94 += build/$(CONFIG)/obj/ejsPot.o
DEPS_94 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_94 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_94 += build/$(CONFIG)/obj/ejsString.o
DEPS_94 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_94 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_94 += build/$(CONFIG)/obj/ejsType.o
DEPS_94 += build/$(CONFIG)/obj/ejsUri.o
DEPS_94 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_94 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_94 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_94 += build/$(CONFIG)/obj/ejsXML.o
DEPS_94 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_94 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_94 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_94 += build/$(CONFIG)/obj/ejsException.o
DEPS_94 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_94 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_94 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_94 += build/$(CONFIG)/obj/ejsModule.o
DEPS_94 += build/$(CONFIG)/obj/ejsScope.o
DEPS_94 += build/$(CONFIG)/obj/ejsService.o
DEPS_94 += build/$(CONFIG)/bin/libejs.a
DEPS_94 += build/$(CONFIG)/obj/ejsc.o
DEPS_94 += build/$(CONFIG)/bin/ejsc
DEPS_94 += src/cmd/ejsmod.h
DEPS_94 += build/$(CONFIG)/obj/ejsmod.o
DEPS_94 += build/$(CONFIG)/obj/doc.o
DEPS_94 += build/$(CONFIG)/obj/docFiles.o
DEPS_94 += build/$(CONFIG)/obj/listing.o
DEPS_94 += build/$(CONFIG)/obj/slotGen.o
DEPS_94 += build/$(CONFIG)/bin/ejsmod

build/$(CONFIG)/bin/ejs.mod: $(DEPS_94)
	( \
	cd src/core; \
	../../$(LBIN)/ejsc --out ../../build/$(CONFIG)/bin/ejs.mod  --optimize 9 --bind --require null *.es  ; \
	../../$(LBIN)/ejsmod --require null --cslots ../../build/$(CONFIG)/bin/ejs.mod ; \
	if ! diff ejs.slots.h ../../build/$(CONFIG)/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ../../build/$(CONFIG)/inc; fi ; \
	rm -f ejs.slots.h ; \
	)

#
#   ejs.db.mod
#
DEPS_95 += src/ejs.db/Database.es
DEPS_95 += src/ejs.db/DatabaseConnector.es
DEPS_95 += slots
DEPS_95 += build/$(CONFIG)/inc/mpr.h
DEPS_95 += build/$(CONFIG)/inc/me.h
DEPS_95 += build/$(CONFIG)/inc/osdep.h
DEPS_95 += build/$(CONFIG)/obj/mprLib.o
DEPS_95 += build/$(CONFIG)/bin/libmpr.a
DEPS_95 += build/$(CONFIG)/inc/pcre.h
DEPS_95 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_95 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_95 += build/$(CONFIG)/inc/http.h
DEPS_95 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_95 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_95 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_95 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_95 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_95 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_95 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_95 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_95 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_95 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_95 += build/$(CONFIG)/inc/ejs.h
DEPS_95 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_95 += build/$(CONFIG)/obj/ecAst.o
DEPS_95 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_95 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_95 += build/$(CONFIG)/obj/ecLex.o
DEPS_95 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_95 += build/$(CONFIG)/obj/ecParser.o
DEPS_95 += build/$(CONFIG)/obj/ecState.o
DEPS_95 += build/$(CONFIG)/obj/dtoa.o
DEPS_95 += build/$(CONFIG)/obj/ejsApp.o
DEPS_95 += build/$(CONFIG)/obj/ejsArray.o
DEPS_95 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_95 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_95 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_95 += build/$(CONFIG)/obj/ejsCache.o
DEPS_95 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_95 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_95 += build/$(CONFIG)/obj/ejsDate.o
DEPS_95 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_95 += build/$(CONFIG)/obj/ejsError.o
DEPS_95 += build/$(CONFIG)/obj/ejsFile.o
DEPS_95 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_95 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_95 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_95 += build/$(CONFIG)/obj/ejsGC.o
DEPS_95 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_95 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_95 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_95 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_95 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_95 += build/$(CONFIG)/obj/ejsMath.o
DEPS_95 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_95 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_95 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_95 += build/$(CONFIG)/obj/ejsNull.o
DEPS_95 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_95 += build/$(CONFIG)/obj/ejsObject.o
DEPS_95 += build/$(CONFIG)/obj/ejsPath.o
DEPS_95 += build/$(CONFIG)/obj/ejsPot.o
DEPS_95 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_95 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_95 += build/$(CONFIG)/obj/ejsString.o
DEPS_95 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_95 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_95 += build/$(CONFIG)/obj/ejsType.o
DEPS_95 += build/$(CONFIG)/obj/ejsUri.o
DEPS_95 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_95 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_95 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_95 += build/$(CONFIG)/obj/ejsXML.o
DEPS_95 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_95 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_95 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_95 += build/$(CONFIG)/obj/ejsException.o
DEPS_95 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_95 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_95 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_95 += build/$(CONFIG)/obj/ejsModule.o
DEPS_95 += build/$(CONFIG)/obj/ejsScope.o
DEPS_95 += build/$(CONFIG)/obj/ejsService.o
DEPS_95 += build/$(CONFIG)/bin/libejs.a
DEPS_95 += build/$(CONFIG)/obj/ejsc.o
DEPS_95 += build/$(CONFIG)/bin/ejsc
DEPS_95 += src/cmd/ejsmod.h
DEPS_95 += build/$(CONFIG)/obj/ejsmod.o
DEPS_95 += build/$(CONFIG)/obj/doc.o
DEPS_95 += build/$(CONFIG)/obj/docFiles.o
DEPS_95 += build/$(CONFIG)/obj/listing.o
DEPS_95 += build/$(CONFIG)/obj/slotGen.o
DEPS_95 += build/$(CONFIG)/bin/ejsmod
DEPS_95 += build/$(CONFIG)/bin/ejs.mod

build/$(CONFIG)/bin/ejs.db.mod: $(DEPS_95)
	( \
	cd src/ejs.db; \
	../../build/$(CONFIG)/bin/ejsc --out ../../build/$(CONFIG)/bin/ejs.db.mod  --optimize 9 *.es ; \
	)

#
#   ejs.db.mapper.mod
#
DEPS_96 += src/ejs.db.mapper/Record.es
DEPS_96 += slots
DEPS_96 += build/$(CONFIG)/inc/mpr.h
DEPS_96 += build/$(CONFIG)/inc/me.h
DEPS_96 += build/$(CONFIG)/inc/osdep.h
DEPS_96 += build/$(CONFIG)/obj/mprLib.o
DEPS_96 += build/$(CONFIG)/bin/libmpr.a
DEPS_96 += build/$(CONFIG)/inc/pcre.h
DEPS_96 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_96 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_96 += build/$(CONFIG)/inc/http.h
DEPS_96 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_96 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_96 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_96 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_96 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_96 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_96 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_96 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_96 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_96 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_96 += build/$(CONFIG)/inc/ejs.h
DEPS_96 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_96 += build/$(CONFIG)/obj/ecAst.o
DEPS_96 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_96 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_96 += build/$(CONFIG)/obj/ecLex.o
DEPS_96 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_96 += build/$(CONFIG)/obj/ecParser.o
DEPS_96 += build/$(CONFIG)/obj/ecState.o
DEPS_96 += build/$(CONFIG)/obj/dtoa.o
DEPS_96 += build/$(CONFIG)/obj/ejsApp.o
DEPS_96 += build/$(CONFIG)/obj/ejsArray.o
DEPS_96 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_96 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_96 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_96 += build/$(CONFIG)/obj/ejsCache.o
DEPS_96 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_96 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_96 += build/$(CONFIG)/obj/ejsDate.o
DEPS_96 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_96 += build/$(CONFIG)/obj/ejsError.o
DEPS_96 += build/$(CONFIG)/obj/ejsFile.o
DEPS_96 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_96 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_96 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_96 += build/$(CONFIG)/obj/ejsGC.o
DEPS_96 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_96 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_96 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_96 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_96 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_96 += build/$(CONFIG)/obj/ejsMath.o
DEPS_96 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_96 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_96 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_96 += build/$(CONFIG)/obj/ejsNull.o
DEPS_96 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_96 += build/$(CONFIG)/obj/ejsObject.o
DEPS_96 += build/$(CONFIG)/obj/ejsPath.o
DEPS_96 += build/$(CONFIG)/obj/ejsPot.o
DEPS_96 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_96 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_96 += build/$(CONFIG)/obj/ejsString.o
DEPS_96 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_96 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_96 += build/$(CONFIG)/obj/ejsType.o
DEPS_96 += build/$(CONFIG)/obj/ejsUri.o
DEPS_96 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_96 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_96 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_96 += build/$(CONFIG)/obj/ejsXML.o
DEPS_96 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_96 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_96 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_96 += build/$(CONFIG)/obj/ejsException.o
DEPS_96 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_96 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_96 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_96 += build/$(CONFIG)/obj/ejsModule.o
DEPS_96 += build/$(CONFIG)/obj/ejsScope.o
DEPS_96 += build/$(CONFIG)/obj/ejsService.o
DEPS_96 += build/$(CONFIG)/bin/libejs.a
DEPS_96 += build/$(CONFIG)/obj/ejsc.o
DEPS_96 += build/$(CONFIG)/bin/ejsc
DEPS_96 += src/cmd/ejsmod.h
DEPS_96 += build/$(CONFIG)/obj/ejsmod.o
DEPS_96 += build/$(CONFIG)/obj/doc.o
DEPS_96 += build/$(CONFIG)/obj/docFiles.o
DEPS_96 += build/$(CONFIG)/obj/listing.o
DEPS_96 += build/$(CONFIG)/obj/slotGen.o
DEPS_96 += build/$(CONFIG)/bin/ejsmod
DEPS_96 += build/$(CONFIG)/bin/ejs.mod
DEPS_96 += build/$(CONFIG)/bin/ejs.db.mod

build/$(CONFIG)/bin/ejs.db.mapper.mod: $(DEPS_96)
	( \
	cd src/ejs.db.mapper; \
	../../build/$(CONFIG)/bin/ejsc --out ../../build/$(CONFIG)/bin/ejs.db.mapper.mod  --optimize 9 *.es ; \
	)

#
#   ejs.db.sqlite.mod
#
DEPS_97 += src/ejs.db.sqlite/Sqlite.es
DEPS_97 += slots
DEPS_97 += build/$(CONFIG)/inc/mpr.h
DEPS_97 += build/$(CONFIG)/inc/me.h
DEPS_97 += build/$(CONFIG)/inc/osdep.h
DEPS_97 += build/$(CONFIG)/obj/mprLib.o
DEPS_97 += build/$(CONFIG)/bin/libmpr.a
DEPS_97 += build/$(CONFIG)/inc/pcre.h
DEPS_97 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_97 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_97 += build/$(CONFIG)/inc/http.h
DEPS_97 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_97 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_97 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_97 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_97 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_97 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_97 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_97 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_97 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_97 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_97 += build/$(CONFIG)/inc/ejs.h
DEPS_97 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_97 += build/$(CONFIG)/obj/ecAst.o
DEPS_97 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_97 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_97 += build/$(CONFIG)/obj/ecLex.o
DEPS_97 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_97 += build/$(CONFIG)/obj/ecParser.o
DEPS_97 += build/$(CONFIG)/obj/ecState.o
DEPS_97 += build/$(CONFIG)/obj/dtoa.o
DEPS_97 += build/$(CONFIG)/obj/ejsApp.o
DEPS_97 += build/$(CONFIG)/obj/ejsArray.o
DEPS_97 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_97 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_97 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_97 += build/$(CONFIG)/obj/ejsCache.o
DEPS_97 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_97 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_97 += build/$(CONFIG)/obj/ejsDate.o
DEPS_97 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_97 += build/$(CONFIG)/obj/ejsError.o
DEPS_97 += build/$(CONFIG)/obj/ejsFile.o
DEPS_97 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_97 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_97 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_97 += build/$(CONFIG)/obj/ejsGC.o
DEPS_97 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_97 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_97 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_97 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_97 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_97 += build/$(CONFIG)/obj/ejsMath.o
DEPS_97 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_97 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_97 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_97 += build/$(CONFIG)/obj/ejsNull.o
DEPS_97 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_97 += build/$(CONFIG)/obj/ejsObject.o
DEPS_97 += build/$(CONFIG)/obj/ejsPath.o
DEPS_97 += build/$(CONFIG)/obj/ejsPot.o
DEPS_97 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_97 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_97 += build/$(CONFIG)/obj/ejsString.o
DEPS_97 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_97 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_97 += build/$(CONFIG)/obj/ejsType.o
DEPS_97 += build/$(CONFIG)/obj/ejsUri.o
DEPS_97 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_97 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_97 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_97 += build/$(CONFIG)/obj/ejsXML.o
DEPS_97 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_97 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_97 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_97 += build/$(CONFIG)/obj/ejsException.o
DEPS_97 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_97 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_97 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_97 += build/$(CONFIG)/obj/ejsModule.o
DEPS_97 += build/$(CONFIG)/obj/ejsScope.o
DEPS_97 += build/$(CONFIG)/obj/ejsService.o
DEPS_97 += build/$(CONFIG)/bin/libejs.a
DEPS_97 += build/$(CONFIG)/obj/ejsc.o
DEPS_97 += build/$(CONFIG)/bin/ejsc
DEPS_97 += src/cmd/ejsmod.h
DEPS_97 += build/$(CONFIG)/obj/ejsmod.o
DEPS_97 += build/$(CONFIG)/obj/doc.o
DEPS_97 += build/$(CONFIG)/obj/docFiles.o
DEPS_97 += build/$(CONFIG)/obj/listing.o
DEPS_97 += build/$(CONFIG)/obj/slotGen.o
DEPS_97 += build/$(CONFIG)/bin/ejsmod
DEPS_97 += build/$(CONFIG)/bin/ejs.mod

build/$(CONFIG)/bin/ejs.db.sqlite.mod: $(DEPS_97)
	( \
	cd src/ejs.db.sqlite; \
	../../build/$(CONFIG)/bin/ejsc --out ../../build/$(CONFIG)/bin/ejs.db.sqlite.mod  --optimize 9 *.es ; \
	)

#
#   ejs.mail.mod
#
DEPS_98 += src/ejs.mail/Mail.es
DEPS_98 += slots
DEPS_98 += build/$(CONFIG)/inc/mpr.h
DEPS_98 += build/$(CONFIG)/inc/me.h
DEPS_98 += build/$(CONFIG)/inc/osdep.h
DEPS_98 += build/$(CONFIG)/obj/mprLib.o
DEPS_98 += build/$(CONFIG)/bin/libmpr.a
DEPS_98 += build/$(CONFIG)/inc/pcre.h
DEPS_98 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_98 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_98 += build/$(CONFIG)/inc/http.h
DEPS_98 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_98 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_98 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_98 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_98 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_98 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_98 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_98 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_98 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_98 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_98 += build/$(CONFIG)/inc/ejs.h
DEPS_98 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_98 += build/$(CONFIG)/obj/ecAst.o
DEPS_98 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_98 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_98 += build/$(CONFIG)/obj/ecLex.o
DEPS_98 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_98 += build/$(CONFIG)/obj/ecParser.o
DEPS_98 += build/$(CONFIG)/obj/ecState.o
DEPS_98 += build/$(CONFIG)/obj/dtoa.o
DEPS_98 += build/$(CONFIG)/obj/ejsApp.o
DEPS_98 += build/$(CONFIG)/obj/ejsArray.o
DEPS_98 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_98 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_98 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_98 += build/$(CONFIG)/obj/ejsCache.o
DEPS_98 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_98 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_98 += build/$(CONFIG)/obj/ejsDate.o
DEPS_98 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_98 += build/$(CONFIG)/obj/ejsError.o
DEPS_98 += build/$(CONFIG)/obj/ejsFile.o
DEPS_98 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_98 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_98 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_98 += build/$(CONFIG)/obj/ejsGC.o
DEPS_98 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_98 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_98 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_98 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_98 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_98 += build/$(CONFIG)/obj/ejsMath.o
DEPS_98 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_98 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_98 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_98 += build/$(CONFIG)/obj/ejsNull.o
DEPS_98 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_98 += build/$(CONFIG)/obj/ejsObject.o
DEPS_98 += build/$(CONFIG)/obj/ejsPath.o
DEPS_98 += build/$(CONFIG)/obj/ejsPot.o
DEPS_98 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_98 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_98 += build/$(CONFIG)/obj/ejsString.o
DEPS_98 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_98 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_98 += build/$(CONFIG)/obj/ejsType.o
DEPS_98 += build/$(CONFIG)/obj/ejsUri.o
DEPS_98 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_98 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_98 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_98 += build/$(CONFIG)/obj/ejsXML.o
DEPS_98 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_98 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_98 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_98 += build/$(CONFIG)/obj/ejsException.o
DEPS_98 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_98 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_98 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_98 += build/$(CONFIG)/obj/ejsModule.o
DEPS_98 += build/$(CONFIG)/obj/ejsScope.o
DEPS_98 += build/$(CONFIG)/obj/ejsService.o
DEPS_98 += build/$(CONFIG)/bin/libejs.a
DEPS_98 += build/$(CONFIG)/obj/ejsc.o
DEPS_98 += build/$(CONFIG)/bin/ejsc
DEPS_98 += src/cmd/ejsmod.h
DEPS_98 += build/$(CONFIG)/obj/ejsmod.o
DEPS_98 += build/$(CONFIG)/obj/doc.o
DEPS_98 += build/$(CONFIG)/obj/docFiles.o
DEPS_98 += build/$(CONFIG)/obj/listing.o
DEPS_98 += build/$(CONFIG)/obj/slotGen.o
DEPS_98 += build/$(CONFIG)/bin/ejsmod
DEPS_98 += build/$(CONFIG)/bin/ejs.mod

build/$(CONFIG)/bin/ejs.mail.mod: $(DEPS_98)
	( \
	cd src/ejs.mail; \
	../../build/$(CONFIG)/bin/ejsc --out ../../build/$(CONFIG)/bin/ejs.mail.mod  --optimize 9 *.es ; \
	)

#
#   ejs.web.mod
#
DEPS_99 += src/ejs.web/Cascade.es
DEPS_99 += src/ejs.web/CommonLog.es
DEPS_99 += src/ejs.web/ContentType.es
DEPS_99 += src/ejs.web/Controller.es
DEPS_99 += src/ejs.web/Dir.es
DEPS_99 += src/ejs.web/Google.es
DEPS_99 += src/ejs.web/Head.es
DEPS_99 += src/ejs.web/Html.es
DEPS_99 += src/ejs.web/HttpServer.es
DEPS_99 += src/ejs.web/MethodOverride.es
DEPS_99 += src/ejs.web/Middleware.es
DEPS_99 += src/ejs.web/Mvc.es
DEPS_99 += src/ejs.web/Request.es
DEPS_99 += src/ejs.web/Router.es
DEPS_99 += src/ejs.web/Script.es
DEPS_99 += src/ejs.web/Session.es
DEPS_99 += src/ejs.web/ShowExceptions.es
DEPS_99 += src/ejs.web/Static.es
DEPS_99 += src/ejs.web/Template.es
DEPS_99 += src/ejs.web/UploadFile.es
DEPS_99 += src/ejs.web/UrlMap.es
DEPS_99 += src/ejs.web/Utils.es
DEPS_99 += src/ejs.web/View.es
DEPS_99 += slots
DEPS_99 += build/$(CONFIG)/inc/mpr.h
DEPS_99 += build/$(CONFIG)/inc/me.h
DEPS_99 += build/$(CONFIG)/inc/osdep.h
DEPS_99 += build/$(CONFIG)/obj/mprLib.o
DEPS_99 += build/$(CONFIG)/bin/libmpr.a
DEPS_99 += build/$(CONFIG)/inc/pcre.h
DEPS_99 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_99 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_99 += build/$(CONFIG)/inc/http.h
DEPS_99 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_99 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_99 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_99 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_99 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_99 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_99 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_99 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_99 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_99 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_99 += build/$(CONFIG)/inc/ejs.h
DEPS_99 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_99 += build/$(CONFIG)/obj/ecAst.o
DEPS_99 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_99 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_99 += build/$(CONFIG)/obj/ecLex.o
DEPS_99 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_99 += build/$(CONFIG)/obj/ecParser.o
DEPS_99 += build/$(CONFIG)/obj/ecState.o
DEPS_99 += build/$(CONFIG)/obj/dtoa.o
DEPS_99 += build/$(CONFIG)/obj/ejsApp.o
DEPS_99 += build/$(CONFIG)/obj/ejsArray.o
DEPS_99 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_99 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_99 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_99 += build/$(CONFIG)/obj/ejsCache.o
DEPS_99 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_99 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_99 += build/$(CONFIG)/obj/ejsDate.o
DEPS_99 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_99 += build/$(CONFIG)/obj/ejsError.o
DEPS_99 += build/$(CONFIG)/obj/ejsFile.o
DEPS_99 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_99 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_99 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_99 += build/$(CONFIG)/obj/ejsGC.o
DEPS_99 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_99 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_99 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_99 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_99 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_99 += build/$(CONFIG)/obj/ejsMath.o
DEPS_99 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_99 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_99 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_99 += build/$(CONFIG)/obj/ejsNull.o
DEPS_99 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_99 += build/$(CONFIG)/obj/ejsObject.o
DEPS_99 += build/$(CONFIG)/obj/ejsPath.o
DEPS_99 += build/$(CONFIG)/obj/ejsPot.o
DEPS_99 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_99 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_99 += build/$(CONFIG)/obj/ejsString.o
DEPS_99 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_99 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_99 += build/$(CONFIG)/obj/ejsType.o
DEPS_99 += build/$(CONFIG)/obj/ejsUri.o
DEPS_99 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_99 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_99 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_99 += build/$(CONFIG)/obj/ejsXML.o
DEPS_99 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_99 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_99 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_99 += build/$(CONFIG)/obj/ejsException.o
DEPS_99 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_99 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_99 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_99 += build/$(CONFIG)/obj/ejsModule.o
DEPS_99 += build/$(CONFIG)/obj/ejsScope.o
DEPS_99 += build/$(CONFIG)/obj/ejsService.o
DEPS_99 += build/$(CONFIG)/bin/libejs.a
DEPS_99 += build/$(CONFIG)/obj/ejsc.o
DEPS_99 += build/$(CONFIG)/bin/ejsc
DEPS_99 += src/cmd/ejsmod.h
DEPS_99 += build/$(CONFIG)/obj/ejsmod.o
DEPS_99 += build/$(CONFIG)/obj/doc.o
DEPS_99 += build/$(CONFIG)/obj/docFiles.o
DEPS_99 += build/$(CONFIG)/obj/listing.o
DEPS_99 += build/$(CONFIG)/obj/slotGen.o
DEPS_99 += build/$(CONFIG)/bin/ejsmod
DEPS_99 += build/$(CONFIG)/bin/ejs.mod

build/$(CONFIG)/bin/ejs.web.mod: $(DEPS_99)
	( \
	cd src/ejs.web; \
	../../build/$(CONFIG)/bin/ejsc --out ../../build/$(CONFIG)/bin/ejs.web.mod  --optimize 9 *.es ; \
	../../build/$(CONFIG)/bin/ejsmod --cslots ../../build/$(CONFIG)/bin/ejs.web.mod ; \
	if ! diff ejs.web.slots.h ../../build/$(CONFIG)/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../build/$(CONFIG)/inc; fi ; \
	rm -f ejs.web.slots.h ; \
	)

#
#   ejs.template.mod
#
DEPS_100 += src/ejs.template/TemplateParser.es
DEPS_100 += slots
DEPS_100 += build/$(CONFIG)/inc/mpr.h
DEPS_100 += build/$(CONFIG)/inc/me.h
DEPS_100 += build/$(CONFIG)/inc/osdep.h
DEPS_100 += build/$(CONFIG)/obj/mprLib.o
DEPS_100 += build/$(CONFIG)/bin/libmpr.a
DEPS_100 += build/$(CONFIG)/inc/pcre.h
DEPS_100 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_100 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_100 += build/$(CONFIG)/inc/http.h
DEPS_100 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_100 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_100 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_100 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_100 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_100 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_100 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_100 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_100 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_100 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_100 += build/$(CONFIG)/inc/ejs.h
DEPS_100 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_100 += build/$(CONFIG)/obj/ecAst.o
DEPS_100 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_100 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_100 += build/$(CONFIG)/obj/ecLex.o
DEPS_100 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_100 += build/$(CONFIG)/obj/ecParser.o
DEPS_100 += build/$(CONFIG)/obj/ecState.o
DEPS_100 += build/$(CONFIG)/obj/dtoa.o
DEPS_100 += build/$(CONFIG)/obj/ejsApp.o
DEPS_100 += build/$(CONFIG)/obj/ejsArray.o
DEPS_100 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_100 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_100 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_100 += build/$(CONFIG)/obj/ejsCache.o
DEPS_100 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_100 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_100 += build/$(CONFIG)/obj/ejsDate.o
DEPS_100 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_100 += build/$(CONFIG)/obj/ejsError.o
DEPS_100 += build/$(CONFIG)/obj/ejsFile.o
DEPS_100 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_100 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_100 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_100 += build/$(CONFIG)/obj/ejsGC.o
DEPS_100 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_100 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_100 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_100 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_100 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_100 += build/$(CONFIG)/obj/ejsMath.o
DEPS_100 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_100 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_100 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_100 += build/$(CONFIG)/obj/ejsNull.o
DEPS_100 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_100 += build/$(CONFIG)/obj/ejsObject.o
DEPS_100 += build/$(CONFIG)/obj/ejsPath.o
DEPS_100 += build/$(CONFIG)/obj/ejsPot.o
DEPS_100 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_100 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_100 += build/$(CONFIG)/obj/ejsString.o
DEPS_100 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_100 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_100 += build/$(CONFIG)/obj/ejsType.o
DEPS_100 += build/$(CONFIG)/obj/ejsUri.o
DEPS_100 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_100 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_100 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_100 += build/$(CONFIG)/obj/ejsXML.o
DEPS_100 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_100 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_100 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_100 += build/$(CONFIG)/obj/ejsException.o
DEPS_100 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_100 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_100 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_100 += build/$(CONFIG)/obj/ejsModule.o
DEPS_100 += build/$(CONFIG)/obj/ejsScope.o
DEPS_100 += build/$(CONFIG)/obj/ejsService.o
DEPS_100 += build/$(CONFIG)/bin/libejs.a
DEPS_100 += build/$(CONFIG)/obj/ejsc.o
DEPS_100 += build/$(CONFIG)/bin/ejsc
DEPS_100 += src/cmd/ejsmod.h
DEPS_100 += build/$(CONFIG)/obj/ejsmod.o
DEPS_100 += build/$(CONFIG)/obj/doc.o
DEPS_100 += build/$(CONFIG)/obj/docFiles.o
DEPS_100 += build/$(CONFIG)/obj/listing.o
DEPS_100 += build/$(CONFIG)/obj/slotGen.o
DEPS_100 += build/$(CONFIG)/bin/ejsmod
DEPS_100 += build/$(CONFIG)/bin/ejs.mod

build/$(CONFIG)/bin/ejs.template.mod: $(DEPS_100)
	( \
	cd src/ejs.template; \
	../../build/$(CONFIG)/bin/ejsc --out ../../build/$(CONFIG)/bin/ejs.template.mod  --optimize 9 TemplateParser.es ; \
	)

#
#   ejs.unix.mod
#
DEPS_101 += src/ejs.unix/Unix.es
DEPS_101 += slots
DEPS_101 += build/$(CONFIG)/inc/mpr.h
DEPS_101 += build/$(CONFIG)/inc/me.h
DEPS_101 += build/$(CONFIG)/inc/osdep.h
DEPS_101 += build/$(CONFIG)/obj/mprLib.o
DEPS_101 += build/$(CONFIG)/bin/libmpr.a
DEPS_101 += build/$(CONFIG)/inc/pcre.h
DEPS_101 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_101 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_101 += build/$(CONFIG)/inc/http.h
DEPS_101 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_101 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_101 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_101 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_101 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_101 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_101 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_101 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_101 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_101 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_101 += build/$(CONFIG)/inc/ejs.h
DEPS_101 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_101 += build/$(CONFIG)/obj/ecAst.o
DEPS_101 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_101 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_101 += build/$(CONFIG)/obj/ecLex.o
DEPS_101 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_101 += build/$(CONFIG)/obj/ecParser.o
DEPS_101 += build/$(CONFIG)/obj/ecState.o
DEPS_101 += build/$(CONFIG)/obj/dtoa.o
DEPS_101 += build/$(CONFIG)/obj/ejsApp.o
DEPS_101 += build/$(CONFIG)/obj/ejsArray.o
DEPS_101 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_101 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_101 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_101 += build/$(CONFIG)/obj/ejsCache.o
DEPS_101 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_101 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_101 += build/$(CONFIG)/obj/ejsDate.o
DEPS_101 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_101 += build/$(CONFIG)/obj/ejsError.o
DEPS_101 += build/$(CONFIG)/obj/ejsFile.o
DEPS_101 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_101 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_101 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_101 += build/$(CONFIG)/obj/ejsGC.o
DEPS_101 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_101 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_101 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_101 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_101 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_101 += build/$(CONFIG)/obj/ejsMath.o
DEPS_101 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_101 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_101 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_101 += build/$(CONFIG)/obj/ejsNull.o
DEPS_101 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_101 += build/$(CONFIG)/obj/ejsObject.o
DEPS_101 += build/$(CONFIG)/obj/ejsPath.o
DEPS_101 += build/$(CONFIG)/obj/ejsPot.o
DEPS_101 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_101 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_101 += build/$(CONFIG)/obj/ejsString.o
DEPS_101 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_101 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_101 += build/$(CONFIG)/obj/ejsType.o
DEPS_101 += build/$(CONFIG)/obj/ejsUri.o
DEPS_101 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_101 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_101 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_101 += build/$(CONFIG)/obj/ejsXML.o
DEPS_101 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_101 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_101 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_101 += build/$(CONFIG)/obj/ejsException.o
DEPS_101 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_101 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_101 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_101 += build/$(CONFIG)/obj/ejsModule.o
DEPS_101 += build/$(CONFIG)/obj/ejsScope.o
DEPS_101 += build/$(CONFIG)/obj/ejsService.o
DEPS_101 += build/$(CONFIG)/bin/libejs.a
DEPS_101 += build/$(CONFIG)/obj/ejsc.o
DEPS_101 += build/$(CONFIG)/bin/ejsc
DEPS_101 += src/cmd/ejsmod.h
DEPS_101 += build/$(CONFIG)/obj/ejsmod.o
DEPS_101 += build/$(CONFIG)/obj/doc.o
DEPS_101 += build/$(CONFIG)/obj/docFiles.o
DEPS_101 += build/$(CONFIG)/obj/listing.o
DEPS_101 += build/$(CONFIG)/obj/slotGen.o
DEPS_101 += build/$(CONFIG)/bin/ejsmod
DEPS_101 += build/$(CONFIG)/bin/ejs.mod

build/$(CONFIG)/bin/ejs.unix.mod: $(DEPS_101)
	( \
	cd src/ejs.unix; \
	../../build/$(CONFIG)/bin/ejsc --out ../../build/$(CONFIG)/bin/ejs.unix.mod  --optimize 9 Unix.es ; \
	)

#
#   ejs.mvc.mod
#
DEPS_102 += src/ejs.mvc/mvc.es
DEPS_102 += slots
DEPS_102 += build/$(CONFIG)/inc/mpr.h
DEPS_102 += build/$(CONFIG)/inc/me.h
DEPS_102 += build/$(CONFIG)/inc/osdep.h
DEPS_102 += build/$(CONFIG)/obj/mprLib.o
DEPS_102 += build/$(CONFIG)/bin/libmpr.a
DEPS_102 += build/$(CONFIG)/inc/pcre.h
DEPS_102 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_102 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_102 += build/$(CONFIG)/inc/http.h
DEPS_102 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_102 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_102 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_102 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_102 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_102 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_102 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_102 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_102 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_102 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_102 += build/$(CONFIG)/inc/ejs.h
DEPS_102 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_102 += build/$(CONFIG)/obj/ecAst.o
DEPS_102 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_102 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_102 += build/$(CONFIG)/obj/ecLex.o
DEPS_102 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_102 += build/$(CONFIG)/obj/ecParser.o
DEPS_102 += build/$(CONFIG)/obj/ecState.o
DEPS_102 += build/$(CONFIG)/obj/dtoa.o
DEPS_102 += build/$(CONFIG)/obj/ejsApp.o
DEPS_102 += build/$(CONFIG)/obj/ejsArray.o
DEPS_102 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_102 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_102 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_102 += build/$(CONFIG)/obj/ejsCache.o
DEPS_102 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_102 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_102 += build/$(CONFIG)/obj/ejsDate.o
DEPS_102 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_102 += build/$(CONFIG)/obj/ejsError.o
DEPS_102 += build/$(CONFIG)/obj/ejsFile.o
DEPS_102 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_102 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_102 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_102 += build/$(CONFIG)/obj/ejsGC.o
DEPS_102 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_102 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_102 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_102 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_102 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_102 += build/$(CONFIG)/obj/ejsMath.o
DEPS_102 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_102 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_102 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_102 += build/$(CONFIG)/obj/ejsNull.o
DEPS_102 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_102 += build/$(CONFIG)/obj/ejsObject.o
DEPS_102 += build/$(CONFIG)/obj/ejsPath.o
DEPS_102 += build/$(CONFIG)/obj/ejsPot.o
DEPS_102 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_102 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_102 += build/$(CONFIG)/obj/ejsString.o
DEPS_102 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_102 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_102 += build/$(CONFIG)/obj/ejsType.o
DEPS_102 += build/$(CONFIG)/obj/ejsUri.o
DEPS_102 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_102 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_102 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_102 += build/$(CONFIG)/obj/ejsXML.o
DEPS_102 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_102 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_102 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_102 += build/$(CONFIG)/obj/ejsException.o
DEPS_102 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_102 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_102 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_102 += build/$(CONFIG)/obj/ejsModule.o
DEPS_102 += build/$(CONFIG)/obj/ejsScope.o
DEPS_102 += build/$(CONFIG)/obj/ejsService.o
DEPS_102 += build/$(CONFIG)/bin/libejs.a
DEPS_102 += build/$(CONFIG)/obj/ejsc.o
DEPS_102 += build/$(CONFIG)/bin/ejsc
DEPS_102 += src/cmd/ejsmod.h
DEPS_102 += build/$(CONFIG)/obj/ejsmod.o
DEPS_102 += build/$(CONFIG)/obj/doc.o
DEPS_102 += build/$(CONFIG)/obj/docFiles.o
DEPS_102 += build/$(CONFIG)/obj/listing.o
DEPS_102 += build/$(CONFIG)/obj/slotGen.o
DEPS_102 += build/$(CONFIG)/bin/ejsmod
DEPS_102 += build/$(CONFIG)/bin/ejs.mod
DEPS_102 += build/$(CONFIG)/bin/ejs.web.mod
DEPS_102 += build/$(CONFIG)/bin/ejs.template.mod
DEPS_102 += build/$(CONFIG)/bin/ejs.unix.mod

build/$(CONFIG)/bin/ejs.mvc.mod: $(DEPS_102)
	( \
	cd src/ejs.mvc; \
	../../build/$(CONFIG)/bin/ejsc --out ../../build/$(CONFIG)/bin/ejs.mvc.mod  --optimize 9 *.es ; \
	)

#
#   ejs.tar.mod
#
DEPS_103 += src/ejs.tar/Tar.es
DEPS_103 += slots
DEPS_103 += build/$(CONFIG)/inc/mpr.h
DEPS_103 += build/$(CONFIG)/inc/me.h
DEPS_103 += build/$(CONFIG)/inc/osdep.h
DEPS_103 += build/$(CONFIG)/obj/mprLib.o
DEPS_103 += build/$(CONFIG)/bin/libmpr.a
DEPS_103 += build/$(CONFIG)/inc/pcre.h
DEPS_103 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_103 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_103 += build/$(CONFIG)/inc/http.h
DEPS_103 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_103 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_103 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_103 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_103 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_103 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_103 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_103 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_103 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_103 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_103 += build/$(CONFIG)/inc/ejs.h
DEPS_103 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_103 += build/$(CONFIG)/obj/ecAst.o
DEPS_103 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_103 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_103 += build/$(CONFIG)/obj/ecLex.o
DEPS_103 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_103 += build/$(CONFIG)/obj/ecParser.o
DEPS_103 += build/$(CONFIG)/obj/ecState.o
DEPS_103 += build/$(CONFIG)/obj/dtoa.o
DEPS_103 += build/$(CONFIG)/obj/ejsApp.o
DEPS_103 += build/$(CONFIG)/obj/ejsArray.o
DEPS_103 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_103 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_103 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_103 += build/$(CONFIG)/obj/ejsCache.o
DEPS_103 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_103 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_103 += build/$(CONFIG)/obj/ejsDate.o
DEPS_103 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_103 += build/$(CONFIG)/obj/ejsError.o
DEPS_103 += build/$(CONFIG)/obj/ejsFile.o
DEPS_103 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_103 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_103 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_103 += build/$(CONFIG)/obj/ejsGC.o
DEPS_103 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_103 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_103 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_103 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_103 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_103 += build/$(CONFIG)/obj/ejsMath.o
DEPS_103 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_103 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_103 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_103 += build/$(CONFIG)/obj/ejsNull.o
DEPS_103 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_103 += build/$(CONFIG)/obj/ejsObject.o
DEPS_103 += build/$(CONFIG)/obj/ejsPath.o
DEPS_103 += build/$(CONFIG)/obj/ejsPot.o
DEPS_103 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_103 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_103 += build/$(CONFIG)/obj/ejsString.o
DEPS_103 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_103 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_103 += build/$(CONFIG)/obj/ejsType.o
DEPS_103 += build/$(CONFIG)/obj/ejsUri.o
DEPS_103 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_103 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_103 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_103 += build/$(CONFIG)/obj/ejsXML.o
DEPS_103 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_103 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_103 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_103 += build/$(CONFIG)/obj/ejsException.o
DEPS_103 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_103 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_103 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_103 += build/$(CONFIG)/obj/ejsModule.o
DEPS_103 += build/$(CONFIG)/obj/ejsScope.o
DEPS_103 += build/$(CONFIG)/obj/ejsService.o
DEPS_103 += build/$(CONFIG)/bin/libejs.a
DEPS_103 += build/$(CONFIG)/obj/ejsc.o
DEPS_103 += build/$(CONFIG)/bin/ejsc
DEPS_103 += src/cmd/ejsmod.h
DEPS_103 += build/$(CONFIG)/obj/ejsmod.o
DEPS_103 += build/$(CONFIG)/obj/doc.o
DEPS_103 += build/$(CONFIG)/obj/docFiles.o
DEPS_103 += build/$(CONFIG)/obj/listing.o
DEPS_103 += build/$(CONFIG)/obj/slotGen.o
DEPS_103 += build/$(CONFIG)/bin/ejsmod
DEPS_103 += build/$(CONFIG)/bin/ejs.mod

build/$(CONFIG)/bin/ejs.tar.mod: $(DEPS_103)
	( \
	cd src/ejs.tar; \
	../../build/$(CONFIG)/bin/ejsc --out ../../build/$(CONFIG)/bin/ejs.tar.mod  --optimize 9 *.es ; \
	)

#
#   ejs.zlib.mod
#
DEPS_104 += src/ejs.zlib/Zlib.es
DEPS_104 += slots
DEPS_104 += build/$(CONFIG)/inc/mpr.h
DEPS_104 += build/$(CONFIG)/inc/me.h
DEPS_104 += build/$(CONFIG)/inc/osdep.h
DEPS_104 += build/$(CONFIG)/obj/mprLib.o
DEPS_104 += build/$(CONFIG)/bin/libmpr.a
DEPS_104 += build/$(CONFIG)/inc/pcre.h
DEPS_104 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_104 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_104 += build/$(CONFIG)/inc/http.h
DEPS_104 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_104 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_104 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_104 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_104 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_104 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_104 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_104 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_104 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_104 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_104 += build/$(CONFIG)/inc/ejs.h
DEPS_104 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_104 += build/$(CONFIG)/obj/ecAst.o
DEPS_104 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_104 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_104 += build/$(CONFIG)/obj/ecLex.o
DEPS_104 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_104 += build/$(CONFIG)/obj/ecParser.o
DEPS_104 += build/$(CONFIG)/obj/ecState.o
DEPS_104 += build/$(CONFIG)/obj/dtoa.o
DEPS_104 += build/$(CONFIG)/obj/ejsApp.o
DEPS_104 += build/$(CONFIG)/obj/ejsArray.o
DEPS_104 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_104 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_104 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_104 += build/$(CONFIG)/obj/ejsCache.o
DEPS_104 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_104 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_104 += build/$(CONFIG)/obj/ejsDate.o
DEPS_104 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_104 += build/$(CONFIG)/obj/ejsError.o
DEPS_104 += build/$(CONFIG)/obj/ejsFile.o
DEPS_104 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_104 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_104 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_104 += build/$(CONFIG)/obj/ejsGC.o
DEPS_104 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_104 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_104 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_104 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_104 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_104 += build/$(CONFIG)/obj/ejsMath.o
DEPS_104 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_104 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_104 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_104 += build/$(CONFIG)/obj/ejsNull.o
DEPS_104 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_104 += build/$(CONFIG)/obj/ejsObject.o
DEPS_104 += build/$(CONFIG)/obj/ejsPath.o
DEPS_104 += build/$(CONFIG)/obj/ejsPot.o
DEPS_104 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_104 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_104 += build/$(CONFIG)/obj/ejsString.o
DEPS_104 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_104 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_104 += build/$(CONFIG)/obj/ejsType.o
DEPS_104 += build/$(CONFIG)/obj/ejsUri.o
DEPS_104 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_104 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_104 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_104 += build/$(CONFIG)/obj/ejsXML.o
DEPS_104 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_104 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_104 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_104 += build/$(CONFIG)/obj/ejsException.o
DEPS_104 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_104 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_104 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_104 += build/$(CONFIG)/obj/ejsModule.o
DEPS_104 += build/$(CONFIG)/obj/ejsScope.o
DEPS_104 += build/$(CONFIG)/obj/ejsService.o
DEPS_104 += build/$(CONFIG)/bin/libejs.a
DEPS_104 += build/$(CONFIG)/obj/ejsc.o
DEPS_104 += build/$(CONFIG)/bin/ejsc
DEPS_104 += src/cmd/ejsmod.h
DEPS_104 += build/$(CONFIG)/obj/ejsmod.o
DEPS_104 += build/$(CONFIG)/obj/doc.o
DEPS_104 += build/$(CONFIG)/obj/docFiles.o
DEPS_104 += build/$(CONFIG)/obj/listing.o
DEPS_104 += build/$(CONFIG)/obj/slotGen.o
DEPS_104 += build/$(CONFIG)/bin/ejsmod
DEPS_104 += build/$(CONFIG)/bin/ejs.mod

build/$(CONFIG)/bin/ejs.zlib.mod: $(DEPS_104)
	( \
	cd src/ejs.zlib; \
	../../build/$(CONFIG)/bin/ejsc --out ../../build/$(CONFIG)/bin/ejs.zlib.mod  --optimize 9 *.es ; \
	)

#
#   ejsrun.o
#
DEPS_105 += build/$(CONFIG)/inc/me.h
DEPS_105 += build/$(CONFIG)/inc/ejsCompiler.h

build/$(CONFIG)/obj/ejsrun.o: \
    src/cmd/ejsrun.c $(DEPS_105)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsrun.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsrun.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/cmd/ejsrun.c

#
#   ejsrun
#
DEPS_106 += slots
DEPS_106 += build/$(CONFIG)/inc/mpr.h
DEPS_106 += build/$(CONFIG)/inc/me.h
DEPS_106 += build/$(CONFIG)/inc/osdep.h
DEPS_106 += build/$(CONFIG)/obj/mprLib.o
DEPS_106 += build/$(CONFIG)/bin/libmpr.a
DEPS_106 += build/$(CONFIG)/inc/pcre.h
DEPS_106 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_106 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_106 += build/$(CONFIG)/inc/http.h
DEPS_106 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_106 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_106 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_106 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_106 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_106 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_106 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_106 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_106 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_106 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_106 += build/$(CONFIG)/inc/ejs.h
DEPS_106 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_106 += build/$(CONFIG)/obj/ecAst.o
DEPS_106 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_106 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_106 += build/$(CONFIG)/obj/ecLex.o
DEPS_106 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_106 += build/$(CONFIG)/obj/ecParser.o
DEPS_106 += build/$(CONFIG)/obj/ecState.o
DEPS_106 += build/$(CONFIG)/obj/dtoa.o
DEPS_106 += build/$(CONFIG)/obj/ejsApp.o
DEPS_106 += build/$(CONFIG)/obj/ejsArray.o
DEPS_106 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_106 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_106 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_106 += build/$(CONFIG)/obj/ejsCache.o
DEPS_106 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_106 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_106 += build/$(CONFIG)/obj/ejsDate.o
DEPS_106 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_106 += build/$(CONFIG)/obj/ejsError.o
DEPS_106 += build/$(CONFIG)/obj/ejsFile.o
DEPS_106 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_106 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_106 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_106 += build/$(CONFIG)/obj/ejsGC.o
DEPS_106 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_106 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_106 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_106 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_106 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_106 += build/$(CONFIG)/obj/ejsMath.o
DEPS_106 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_106 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_106 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_106 += build/$(CONFIG)/obj/ejsNull.o
DEPS_106 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_106 += build/$(CONFIG)/obj/ejsObject.o
DEPS_106 += build/$(CONFIG)/obj/ejsPath.o
DEPS_106 += build/$(CONFIG)/obj/ejsPot.o
DEPS_106 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_106 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_106 += build/$(CONFIG)/obj/ejsString.o
DEPS_106 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_106 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_106 += build/$(CONFIG)/obj/ejsType.o
DEPS_106 += build/$(CONFIG)/obj/ejsUri.o
DEPS_106 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_106 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_106 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_106 += build/$(CONFIG)/obj/ejsXML.o
DEPS_106 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_106 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_106 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_106 += build/$(CONFIG)/obj/ejsException.o
DEPS_106 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_106 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_106 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_106 += build/$(CONFIG)/obj/ejsModule.o
DEPS_106 += build/$(CONFIG)/obj/ejsScope.o
DEPS_106 += build/$(CONFIG)/obj/ejsService.o
DEPS_106 += build/$(CONFIG)/bin/libejs.a
DEPS_106 += build/$(CONFIG)/obj/ejsrun.o

LIBS_106 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_106 += -lhttp
endif
LIBS_106 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_106 += -lpcre
endif

build/$(CONFIG)/bin/ejsrun: $(DEPS_106)
	@echo '      [Link] build/$(CONFIG)/bin/ejsrun'
	$(CC) -o build/$(CONFIG)/bin/ejsrun -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/ejsrun.o" $(LIBPATHS_106) $(LIBS_106) $(LIBS_106) $(LIBS) -lpam 


#
#   http-ca-crt
#
DEPS_107 += src/paks/http/ca.crt

build/$(CONFIG)/bin/ca.crt: $(DEPS_107)
	@echo '      [Copy] build/$(CONFIG)/bin/ca.crt'
	mkdir -p "build/$(CONFIG)/bin"
	cp src/paks/http/ca.crt build/$(CONFIG)/bin/ca.crt

#
#   sqlite3.h
#
build/$(CONFIG)/inc/sqlite3.h: $(DEPS_108)
	@echo '      [Copy] build/$(CONFIG)/inc/sqlite3.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/sqlite/sqlite3.h build/$(CONFIG)/inc/sqlite3.h

#
#   sqlite3.o
#
DEPS_109 += build/$(CONFIG)/inc/me.h
DEPS_109 += build/$(CONFIG)/inc/sqlite3.h

build/$(CONFIG)/obj/sqlite3.o: \
    src/paks/sqlite/sqlite3.c $(DEPS_109)
	@echo '   [Compile] build/$(CONFIG)/obj/sqlite3.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/sqlite3.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/sqlite/sqlite3.c

ifeq ($(ME_COM_SQLITE),1)
#
#   libsql
#
DEPS_110 += build/$(CONFIG)/inc/sqlite3.h
DEPS_110 += build/$(CONFIG)/inc/me.h
DEPS_110 += build/$(CONFIG)/obj/sqlite3.o

build/$(CONFIG)/bin/libsql.a: $(DEPS_110)
	@echo '      [Link] build/$(CONFIG)/bin/libsql.a'
	ar -cr build/$(CONFIG)/bin/libsql.a "build/$(CONFIG)/obj/sqlite3.o"
endif

#
#   ejsSqlite.o
#
DEPS_111 += build/$(CONFIG)/inc/me.h
DEPS_111 += build/$(CONFIG)/inc/mpr.h
DEPS_111 += build/$(CONFIG)/inc/http.h
DEPS_111 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_111 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_111 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_111 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_111 += build/$(CONFIG)/inc/ejs.h
DEPS_111 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h

build/$(CONFIG)/obj/ejsSqlite.o: \
    src/ejs.db.sqlite/ejsSqlite.c $(DEPS_111)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsSqlite.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsSqlite.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.db.sqlite/ejsSqlite.c

#
#   libejs.db.sqlite
#
DEPS_112 += build/$(CONFIG)/inc/mpr.h
DEPS_112 += build/$(CONFIG)/inc/me.h
DEPS_112 += build/$(CONFIG)/inc/osdep.h
DEPS_112 += build/$(CONFIG)/obj/mprLib.o
DEPS_112 += build/$(CONFIG)/bin/libmpr.a
DEPS_112 += slots
DEPS_112 += build/$(CONFIG)/inc/pcre.h
DEPS_112 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_112 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_112 += build/$(CONFIG)/inc/http.h
DEPS_112 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_112 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_112 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_112 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_112 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_112 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_112 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_112 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_112 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_112 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_112 += build/$(CONFIG)/inc/ejs.h
DEPS_112 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_112 += build/$(CONFIG)/obj/ecAst.o
DEPS_112 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_112 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_112 += build/$(CONFIG)/obj/ecLex.o
DEPS_112 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_112 += build/$(CONFIG)/obj/ecParser.o
DEPS_112 += build/$(CONFIG)/obj/ecState.o
DEPS_112 += build/$(CONFIG)/obj/dtoa.o
DEPS_112 += build/$(CONFIG)/obj/ejsApp.o
DEPS_112 += build/$(CONFIG)/obj/ejsArray.o
DEPS_112 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_112 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_112 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_112 += build/$(CONFIG)/obj/ejsCache.o
DEPS_112 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_112 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_112 += build/$(CONFIG)/obj/ejsDate.o
DEPS_112 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_112 += build/$(CONFIG)/obj/ejsError.o
DEPS_112 += build/$(CONFIG)/obj/ejsFile.o
DEPS_112 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_112 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_112 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_112 += build/$(CONFIG)/obj/ejsGC.o
DEPS_112 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_112 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_112 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_112 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_112 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_112 += build/$(CONFIG)/obj/ejsMath.o
DEPS_112 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_112 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_112 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_112 += build/$(CONFIG)/obj/ejsNull.o
DEPS_112 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_112 += build/$(CONFIG)/obj/ejsObject.o
DEPS_112 += build/$(CONFIG)/obj/ejsPath.o
DEPS_112 += build/$(CONFIG)/obj/ejsPot.o
DEPS_112 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_112 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_112 += build/$(CONFIG)/obj/ejsString.o
DEPS_112 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_112 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_112 += build/$(CONFIG)/obj/ejsType.o
DEPS_112 += build/$(CONFIG)/obj/ejsUri.o
DEPS_112 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_112 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_112 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_112 += build/$(CONFIG)/obj/ejsXML.o
DEPS_112 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_112 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_112 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_112 += build/$(CONFIG)/obj/ejsException.o
DEPS_112 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_112 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_112 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_112 += build/$(CONFIG)/obj/ejsModule.o
DEPS_112 += build/$(CONFIG)/obj/ejsScope.o
DEPS_112 += build/$(CONFIG)/obj/ejsService.o
DEPS_112 += build/$(CONFIG)/bin/libejs.a
DEPS_112 += build/$(CONFIG)/obj/ejsc.o
DEPS_112 += build/$(CONFIG)/bin/ejsc
DEPS_112 += src/cmd/ejsmod.h
DEPS_112 += build/$(CONFIG)/obj/ejsmod.o
DEPS_112 += build/$(CONFIG)/obj/doc.o
DEPS_112 += build/$(CONFIG)/obj/docFiles.o
DEPS_112 += build/$(CONFIG)/obj/listing.o
DEPS_112 += build/$(CONFIG)/obj/slotGen.o
DEPS_112 += build/$(CONFIG)/bin/ejsmod
DEPS_112 += build/$(CONFIG)/bin/ejs.mod
DEPS_112 += build/$(CONFIG)/bin/ejs.db.sqlite.mod
DEPS_112 += build/$(CONFIG)/inc/sqlite3.h
DEPS_112 += build/$(CONFIG)/obj/sqlite3.o
ifeq ($(ME_COM_SQLITE),1)
    DEPS_112 += build/$(CONFIG)/bin/libsql.a
endif
DEPS_112 += build/$(CONFIG)/obj/ejsSqlite.o

build/$(CONFIG)/bin/libejs.db.sqlite.a: $(DEPS_112)
	@echo '      [Link] build/$(CONFIG)/bin/libejs.db.sqlite.a'
	ar -cr build/$(CONFIG)/bin/libejs.db.sqlite.a "build/$(CONFIG)/obj/ejsSqlite.o"

#
#   ejsWeb.h
#
build/$(CONFIG)/inc/ejsWeb.h: $(DEPS_113)
	@echo '      [Copy] build/$(CONFIG)/inc/ejsWeb.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/ejs.web/ejsWeb.h build/$(CONFIG)/inc/ejsWeb.h

#
#   ejsHttpServer.o
#
DEPS_114 += build/$(CONFIG)/inc/me.h
DEPS_114 += build/$(CONFIG)/inc/mpr.h
DEPS_114 += build/$(CONFIG)/inc/http.h
DEPS_114 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_114 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_114 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_114 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_114 += build/$(CONFIG)/inc/ejs.h
DEPS_114 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_114 += build/$(CONFIG)/inc/ejsWeb.h
DEPS_114 += build/$(CONFIG)/inc/ejs.web.slots.h

build/$(CONFIG)/obj/ejsHttpServer.o: \
    src/ejs.web/ejsHttpServer.c $(DEPS_114)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsHttpServer.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsHttpServer.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.web/ejsHttpServer.c

#
#   ejsRequest.o
#
DEPS_115 += build/$(CONFIG)/inc/me.h
DEPS_115 += build/$(CONFIG)/inc/mpr.h
DEPS_115 += build/$(CONFIG)/inc/http.h
DEPS_115 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_115 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_115 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_115 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_115 += build/$(CONFIG)/inc/ejs.h
DEPS_115 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_115 += build/$(CONFIG)/inc/ejsWeb.h
DEPS_115 += build/$(CONFIG)/inc/ejs.web.slots.h

build/$(CONFIG)/obj/ejsRequest.o: \
    src/ejs.web/ejsRequest.c $(DEPS_115)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsRequest.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsRequest.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.web/ejsRequest.c

#
#   ejsSession.o
#
DEPS_116 += build/$(CONFIG)/inc/me.h
DEPS_116 += build/$(CONFIG)/inc/mpr.h
DEPS_116 += build/$(CONFIG)/inc/http.h
DEPS_116 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_116 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_116 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_116 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_116 += build/$(CONFIG)/inc/ejs.h
DEPS_116 += build/$(CONFIG)/inc/ejsWeb.h

build/$(CONFIG)/obj/ejsSession.o: \
    src/ejs.web/ejsSession.c $(DEPS_116)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsSession.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsSession.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.web/ejsSession.c

#
#   ejsWeb.o
#
DEPS_117 += build/$(CONFIG)/inc/me.h
DEPS_117 += build/$(CONFIG)/inc/mpr.h
DEPS_117 += build/$(CONFIG)/inc/http.h
DEPS_117 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_117 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_117 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_117 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_117 += build/$(CONFIG)/inc/ejs.h
DEPS_117 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_117 += build/$(CONFIG)/inc/ejsWeb.h
DEPS_117 += build/$(CONFIG)/inc/ejs.web.slots.h

build/$(CONFIG)/obj/ejsWeb.o: \
    src/ejs.web/ejsWeb.c $(DEPS_117)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsWeb.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsWeb.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.web/ejsWeb.c

#
#   libejs.web
#
DEPS_118 += slots
DEPS_118 += build/$(CONFIG)/inc/mpr.h
DEPS_118 += build/$(CONFIG)/inc/me.h
DEPS_118 += build/$(CONFIG)/inc/osdep.h
DEPS_118 += build/$(CONFIG)/obj/mprLib.o
DEPS_118 += build/$(CONFIG)/bin/libmpr.a
DEPS_118 += build/$(CONFIG)/inc/pcre.h
DEPS_118 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_118 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_118 += build/$(CONFIG)/inc/http.h
DEPS_118 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_118 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_118 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_118 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_118 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_118 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_118 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_118 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_118 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_118 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_118 += build/$(CONFIG)/inc/ejs.h
DEPS_118 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_118 += build/$(CONFIG)/obj/ecAst.o
DEPS_118 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_118 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_118 += build/$(CONFIG)/obj/ecLex.o
DEPS_118 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_118 += build/$(CONFIG)/obj/ecParser.o
DEPS_118 += build/$(CONFIG)/obj/ecState.o
DEPS_118 += build/$(CONFIG)/obj/dtoa.o
DEPS_118 += build/$(CONFIG)/obj/ejsApp.o
DEPS_118 += build/$(CONFIG)/obj/ejsArray.o
DEPS_118 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_118 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_118 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_118 += build/$(CONFIG)/obj/ejsCache.o
DEPS_118 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_118 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_118 += build/$(CONFIG)/obj/ejsDate.o
DEPS_118 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_118 += build/$(CONFIG)/obj/ejsError.o
DEPS_118 += build/$(CONFIG)/obj/ejsFile.o
DEPS_118 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_118 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_118 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_118 += build/$(CONFIG)/obj/ejsGC.o
DEPS_118 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_118 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_118 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_118 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_118 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_118 += build/$(CONFIG)/obj/ejsMath.o
DEPS_118 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_118 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_118 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_118 += build/$(CONFIG)/obj/ejsNull.o
DEPS_118 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_118 += build/$(CONFIG)/obj/ejsObject.o
DEPS_118 += build/$(CONFIG)/obj/ejsPath.o
DEPS_118 += build/$(CONFIG)/obj/ejsPot.o
DEPS_118 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_118 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_118 += build/$(CONFIG)/obj/ejsString.o
DEPS_118 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_118 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_118 += build/$(CONFIG)/obj/ejsType.o
DEPS_118 += build/$(CONFIG)/obj/ejsUri.o
DEPS_118 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_118 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_118 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_118 += build/$(CONFIG)/obj/ejsXML.o
DEPS_118 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_118 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_118 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_118 += build/$(CONFIG)/obj/ejsException.o
DEPS_118 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_118 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_118 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_118 += build/$(CONFIG)/obj/ejsModule.o
DEPS_118 += build/$(CONFIG)/obj/ejsScope.o
DEPS_118 += build/$(CONFIG)/obj/ejsService.o
DEPS_118 += build/$(CONFIG)/bin/libejs.a
DEPS_118 += build/$(CONFIG)/obj/ejsc.o
DEPS_118 += build/$(CONFIG)/bin/ejsc
DEPS_118 += src/cmd/ejsmod.h
DEPS_118 += build/$(CONFIG)/obj/ejsmod.o
DEPS_118 += build/$(CONFIG)/obj/doc.o
DEPS_118 += build/$(CONFIG)/obj/docFiles.o
DEPS_118 += build/$(CONFIG)/obj/listing.o
DEPS_118 += build/$(CONFIG)/obj/slotGen.o
DEPS_118 += build/$(CONFIG)/bin/ejsmod
DEPS_118 += build/$(CONFIG)/bin/ejs.mod
DEPS_118 += build/$(CONFIG)/inc/ejsWeb.h
DEPS_118 += build/$(CONFIG)/obj/ejsHttpServer.o
DEPS_118 += build/$(CONFIG)/obj/ejsRequest.o
DEPS_118 += build/$(CONFIG)/obj/ejsSession.o
DEPS_118 += build/$(CONFIG)/obj/ejsWeb.o

build/$(CONFIG)/bin/libejs.web.a: $(DEPS_118)
	@echo '      [Link] build/$(CONFIG)/bin/libejs.web.a'
	ar -cr build/$(CONFIG)/bin/libejs.web.a "build/$(CONFIG)/obj/ejsHttpServer.o" "build/$(CONFIG)/obj/ejsRequest.o" "build/$(CONFIG)/obj/ejsSession.o" "build/$(CONFIG)/obj/ejsWeb.o"

#
#   zlib.h
#
build/$(CONFIG)/inc/zlib.h: $(DEPS_119)
	@echo '      [Copy] build/$(CONFIG)/inc/zlib.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/zlib/zlib.h build/$(CONFIG)/inc/zlib.h

#
#   zlib.o
#
DEPS_120 += build/$(CONFIG)/inc/me.h
DEPS_120 += build/$(CONFIG)/inc/zlib.h

build/$(CONFIG)/obj/zlib.o: \
    src/paks/zlib/zlib.c $(DEPS_120)
	@echo '   [Compile] build/$(CONFIG)/obj/zlib.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/zlib.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/zlib/zlib.c

ifeq ($(ME_COM_ZLIB),1)
#
#   libzlib
#
DEPS_121 += build/$(CONFIG)/inc/zlib.h
DEPS_121 += build/$(CONFIG)/inc/me.h
DEPS_121 += build/$(CONFIG)/obj/zlib.o

build/$(CONFIG)/bin/libzlib.a: $(DEPS_121)
	@echo '      [Link] build/$(CONFIG)/bin/libzlib.a'
	ar -cr build/$(CONFIG)/bin/libzlib.a "build/$(CONFIG)/obj/zlib.o"
endif

#
#   ejsZlib.o
#
DEPS_122 += build/$(CONFIG)/inc/me.h
DEPS_122 += build/$(CONFIG)/inc/mpr.h
DEPS_122 += build/$(CONFIG)/inc/http.h
DEPS_122 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_122 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_122 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_122 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_122 += build/$(CONFIG)/inc/ejs.h
DEPS_122 += build/$(CONFIG)/inc/zlib.h
DEPS_122 += build/$(CONFIG)/inc/ejs.zlib.slots.h

build/$(CONFIG)/obj/ejsZlib.o: \
    src/ejs.zlib/ejsZlib.c $(DEPS_122)
	@echo '   [Compile] build/$(CONFIG)/obj/ejsZlib.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/ejsZlib.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.zlib/ejsZlib.c

#
#   libejs.zlib
#
DEPS_123 += slots
DEPS_123 += build/$(CONFIG)/inc/mpr.h
DEPS_123 += build/$(CONFIG)/inc/me.h
DEPS_123 += build/$(CONFIG)/inc/osdep.h
DEPS_123 += build/$(CONFIG)/obj/mprLib.o
DEPS_123 += build/$(CONFIG)/bin/libmpr.a
DEPS_123 += build/$(CONFIG)/inc/pcre.h
DEPS_123 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_123 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_123 += build/$(CONFIG)/inc/http.h
DEPS_123 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_123 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_123 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_123 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_123 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_123 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_123 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_123 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_123 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_123 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_123 += build/$(CONFIG)/inc/ejs.h
DEPS_123 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_123 += build/$(CONFIG)/obj/ecAst.o
DEPS_123 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_123 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_123 += build/$(CONFIG)/obj/ecLex.o
DEPS_123 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_123 += build/$(CONFIG)/obj/ecParser.o
DEPS_123 += build/$(CONFIG)/obj/ecState.o
DEPS_123 += build/$(CONFIG)/obj/dtoa.o
DEPS_123 += build/$(CONFIG)/obj/ejsApp.o
DEPS_123 += build/$(CONFIG)/obj/ejsArray.o
DEPS_123 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_123 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_123 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_123 += build/$(CONFIG)/obj/ejsCache.o
DEPS_123 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_123 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_123 += build/$(CONFIG)/obj/ejsDate.o
DEPS_123 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_123 += build/$(CONFIG)/obj/ejsError.o
DEPS_123 += build/$(CONFIG)/obj/ejsFile.o
DEPS_123 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_123 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_123 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_123 += build/$(CONFIG)/obj/ejsGC.o
DEPS_123 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_123 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_123 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_123 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_123 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_123 += build/$(CONFIG)/obj/ejsMath.o
DEPS_123 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_123 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_123 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_123 += build/$(CONFIG)/obj/ejsNull.o
DEPS_123 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_123 += build/$(CONFIG)/obj/ejsObject.o
DEPS_123 += build/$(CONFIG)/obj/ejsPath.o
DEPS_123 += build/$(CONFIG)/obj/ejsPot.o
DEPS_123 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_123 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_123 += build/$(CONFIG)/obj/ejsString.o
DEPS_123 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_123 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_123 += build/$(CONFIG)/obj/ejsType.o
DEPS_123 += build/$(CONFIG)/obj/ejsUri.o
DEPS_123 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_123 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_123 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_123 += build/$(CONFIG)/obj/ejsXML.o
DEPS_123 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_123 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_123 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_123 += build/$(CONFIG)/obj/ejsException.o
DEPS_123 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_123 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_123 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_123 += build/$(CONFIG)/obj/ejsModule.o
DEPS_123 += build/$(CONFIG)/obj/ejsScope.o
DEPS_123 += build/$(CONFIG)/obj/ejsService.o
DEPS_123 += build/$(CONFIG)/bin/libejs.a
DEPS_123 += build/$(CONFIG)/obj/ejsc.o
DEPS_123 += build/$(CONFIG)/bin/ejsc
DEPS_123 += src/cmd/ejsmod.h
DEPS_123 += build/$(CONFIG)/obj/ejsmod.o
DEPS_123 += build/$(CONFIG)/obj/doc.o
DEPS_123 += build/$(CONFIG)/obj/docFiles.o
DEPS_123 += build/$(CONFIG)/obj/listing.o
DEPS_123 += build/$(CONFIG)/obj/slotGen.o
DEPS_123 += build/$(CONFIG)/bin/ejsmod
DEPS_123 += build/$(CONFIG)/bin/ejs.mod
DEPS_123 += build/$(CONFIG)/bin/ejs.zlib.mod
DEPS_123 += build/$(CONFIG)/inc/zlib.h
DEPS_123 += build/$(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_123 += build/$(CONFIG)/bin/libzlib.a
endif
DEPS_123 += build/$(CONFIG)/obj/ejsZlib.o

build/$(CONFIG)/bin/libejs.zlib.a: $(DEPS_123)
	@echo '      [Link] build/$(CONFIG)/bin/libejs.zlib.a'
	ar -cr build/$(CONFIG)/bin/libejs.zlib.a "build/$(CONFIG)/obj/ejsZlib.o"

#
#   est.h
#
build/$(CONFIG)/inc/est.h: $(DEPS_124)
	@echo '      [Copy] build/$(CONFIG)/inc/est.h'
	mkdir -p "build/$(CONFIG)/inc"
	cp src/paks/est/est.h build/$(CONFIG)/inc/est.h

#
#   estLib.o
#
DEPS_125 += build/$(CONFIG)/inc/me.h
DEPS_125 += build/$(CONFIG)/inc/est.h
DEPS_125 += build/$(CONFIG)/inc/osdep.h

build/$(CONFIG)/obj/estLib.o: \
    src/paks/est/estLib.c $(DEPS_125)
	@echo '   [Compile] build/$(CONFIG)/obj/estLib.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/estLib.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/est/estLib.c

ifeq ($(ME_COM_EST),1)
#
#   libest
#
DEPS_126 += build/$(CONFIG)/inc/est.h
DEPS_126 += build/$(CONFIG)/inc/me.h
DEPS_126 += build/$(CONFIG)/inc/osdep.h
DEPS_126 += build/$(CONFIG)/obj/estLib.o

build/$(CONFIG)/bin/libest.a: $(DEPS_126)
	@echo '      [Link] build/$(CONFIG)/bin/libest.a'
	ar -cr build/$(CONFIG)/bin/libest.a "build/$(CONFIG)/obj/estLib.o"
endif

#
#   mprSsl.o
#
DEPS_127 += build/$(CONFIG)/inc/me.h
DEPS_127 += build/$(CONFIG)/inc/mpr.h
DEPS_127 += build/$(CONFIG)/inc/est.h

build/$(CONFIG)/obj/mprSsl.o: \
    src/paks/mpr/mprSsl.c $(DEPS_127)
	@echo '   [Compile] build/$(CONFIG)/obj/mprSsl.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/mprSsl.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" "-I$(ME_COM_MATRIXSSL_PATH)" "-I$(ME_COM_MATRIXSSL_PATH)/matrixssl" "-I$(ME_COM_NANOSSL_PATH)/src" src/paks/mpr/mprSsl.c

#
#   libmprssl
#
DEPS_128 += build/$(CONFIG)/inc/mpr.h
DEPS_128 += build/$(CONFIG)/inc/me.h
DEPS_128 += build/$(CONFIG)/inc/osdep.h
DEPS_128 += build/$(CONFIG)/obj/mprLib.o
DEPS_128 += build/$(CONFIG)/bin/libmpr.a
DEPS_128 += build/$(CONFIG)/inc/est.h
DEPS_128 += build/$(CONFIG)/obj/estLib.o
ifeq ($(ME_COM_EST),1)
    DEPS_128 += build/$(CONFIG)/bin/libest.a
endif
DEPS_128 += build/$(CONFIG)/obj/mprSsl.o

build/$(CONFIG)/bin/libmprssl.a: $(DEPS_128)
	@echo '      [Link] build/$(CONFIG)/bin/libmprssl.a'
	ar -cr build/$(CONFIG)/bin/libmprssl.a "build/$(CONFIG)/obj/mprSsl.o"

#
#   manager.o
#
DEPS_129 += build/$(CONFIG)/inc/me.h
DEPS_129 += build/$(CONFIG)/inc/mpr.h

build/$(CONFIG)/obj/manager.o: \
    src/paks/mpr/manager.c $(DEPS_129)
	@echo '   [Compile] build/$(CONFIG)/obj/manager.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/manager.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/mpr/manager.c

#
#   manager
#
DEPS_130 += build/$(CONFIG)/inc/mpr.h
DEPS_130 += build/$(CONFIG)/inc/me.h
DEPS_130 += build/$(CONFIG)/inc/osdep.h
DEPS_130 += build/$(CONFIG)/obj/mprLib.o
DEPS_130 += build/$(CONFIG)/bin/libmpr.a
DEPS_130 += build/$(CONFIG)/obj/manager.o

LIBS_130 += -lmpr

build/$(CONFIG)/bin/ejsman: $(DEPS_130)
	@echo '      [Link] build/$(CONFIG)/bin/ejsman'
	$(CC) -o build/$(CONFIG)/bin/ejsman -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/manager.o" $(LIBPATHS_130) $(LIBS_130) $(LIBS_130) $(LIBS) 

#
#   mvc.es
#
DEPS_131 += src/ejs.mvc/mvc.es

build/$(CONFIG)/bin/mvc.es: $(DEPS_131)
	( \
	cd src/ejs.mvc; \
	cp mvc.es ../../build/$(CONFIG)/bin ; \
	)

#
#   mvc
#
DEPS_132 += slots
DEPS_132 += build/$(CONFIG)/inc/mpr.h
DEPS_132 += build/$(CONFIG)/inc/me.h
DEPS_132 += build/$(CONFIG)/inc/osdep.h
DEPS_132 += build/$(CONFIG)/obj/mprLib.o
DEPS_132 += build/$(CONFIG)/bin/libmpr.a
DEPS_132 += build/$(CONFIG)/inc/pcre.h
DEPS_132 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_132 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_132 += build/$(CONFIG)/inc/http.h
DEPS_132 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_132 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_132 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_132 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_132 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_132 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_132 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_132 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_132 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_132 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_132 += build/$(CONFIG)/inc/ejs.h
DEPS_132 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_132 += build/$(CONFIG)/obj/ecAst.o
DEPS_132 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_132 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_132 += build/$(CONFIG)/obj/ecLex.o
DEPS_132 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_132 += build/$(CONFIG)/obj/ecParser.o
DEPS_132 += build/$(CONFIG)/obj/ecState.o
DEPS_132 += build/$(CONFIG)/obj/dtoa.o
DEPS_132 += build/$(CONFIG)/obj/ejsApp.o
DEPS_132 += build/$(CONFIG)/obj/ejsArray.o
DEPS_132 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_132 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_132 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_132 += build/$(CONFIG)/obj/ejsCache.o
DEPS_132 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_132 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_132 += build/$(CONFIG)/obj/ejsDate.o
DEPS_132 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_132 += build/$(CONFIG)/obj/ejsError.o
DEPS_132 += build/$(CONFIG)/obj/ejsFile.o
DEPS_132 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_132 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_132 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_132 += build/$(CONFIG)/obj/ejsGC.o
DEPS_132 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_132 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_132 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_132 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_132 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_132 += build/$(CONFIG)/obj/ejsMath.o
DEPS_132 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_132 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_132 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_132 += build/$(CONFIG)/obj/ejsNull.o
DEPS_132 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_132 += build/$(CONFIG)/obj/ejsObject.o
DEPS_132 += build/$(CONFIG)/obj/ejsPath.o
DEPS_132 += build/$(CONFIG)/obj/ejsPot.o
DEPS_132 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_132 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_132 += build/$(CONFIG)/obj/ejsString.o
DEPS_132 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_132 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_132 += build/$(CONFIG)/obj/ejsType.o
DEPS_132 += build/$(CONFIG)/obj/ejsUri.o
DEPS_132 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_132 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_132 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_132 += build/$(CONFIG)/obj/ejsXML.o
DEPS_132 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_132 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_132 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_132 += build/$(CONFIG)/obj/ejsException.o
DEPS_132 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_132 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_132 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_132 += build/$(CONFIG)/obj/ejsModule.o
DEPS_132 += build/$(CONFIG)/obj/ejsScope.o
DEPS_132 += build/$(CONFIG)/obj/ejsService.o
DEPS_132 += build/$(CONFIG)/bin/libejs.a
DEPS_132 += build/$(CONFIG)/bin/mvc.es
DEPS_132 += build/$(CONFIG)/obj/ejsrun.o

LIBS_132 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_132 += -lhttp
endif
LIBS_132 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_132 += -lpcre
endif

build/$(CONFIG)/bin/mvc: $(DEPS_132)
	@echo '      [Link] build/$(CONFIG)/bin/mvc'
	$(CC) -o build/$(CONFIG)/bin/mvc -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/ejsrun.o" $(LIBPATHS_132) $(LIBS_132) $(LIBS_132) $(LIBS) -lpam 

#
#   sqlite.o
#
DEPS_133 += build/$(CONFIG)/inc/me.h
DEPS_133 += build/$(CONFIG)/inc/sqlite3.h

build/$(CONFIG)/obj/sqlite.o: \
    src/paks/sqlite/sqlite.c $(DEPS_133)
	@echo '   [Compile] build/$(CONFIG)/obj/sqlite.o'
	$(CC) -c $(DFLAGS) -o build/$(CONFIG)/obj/sqlite.o -arch $(CC_ARCH) $(CFLAGS) $(IFLAGS) src/paks/sqlite/sqlite.c

ifeq ($(ME_COM_SQLITE),1)
#
#   sqliteshell
#
DEPS_134 += build/$(CONFIG)/inc/sqlite3.h
DEPS_134 += build/$(CONFIG)/inc/me.h
DEPS_134 += build/$(CONFIG)/obj/sqlite3.o
DEPS_134 += build/$(CONFIG)/bin/libsql.a
DEPS_134 += build/$(CONFIG)/obj/sqlite.o

LIBS_134 += -lsql

build/$(CONFIG)/bin/sqlite: $(DEPS_134)
	@echo '      [Link] build/$(CONFIG)/bin/sqlite'
	$(CC) -o build/$(CONFIG)/bin/sqlite -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/sqlite.o" $(LIBPATHS_134) $(LIBS_134) $(LIBS_134) $(LIBS) 
endif

#
#   utest.es
#
DEPS_135 += src/ejs.utest/utest.es

build/$(CONFIG)/bin/utest.es: $(DEPS_135)
	( \
	cd src/ejs.utest; \
	cp utest.es ../../build/$(CONFIG)/bin ; \
	)

#
#   utest.worker
#
DEPS_136 += src/ejs.utest/utest.worker

build/$(CONFIG)/bin/utest.worker: $(DEPS_136)
	( \
	cd src/ejs.utest; \
	cp utest.worker ../../build/$(CONFIG)/bin ; \
	)

#
#   utest
#
DEPS_137 += slots
DEPS_137 += build/$(CONFIG)/inc/mpr.h
DEPS_137 += build/$(CONFIG)/inc/me.h
DEPS_137 += build/$(CONFIG)/inc/osdep.h
DEPS_137 += build/$(CONFIG)/obj/mprLib.o
DEPS_137 += build/$(CONFIG)/bin/libmpr.a
DEPS_137 += build/$(CONFIG)/inc/pcre.h
DEPS_137 += build/$(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_137 += build/$(CONFIG)/bin/libpcre.a
endif
DEPS_137 += build/$(CONFIG)/inc/http.h
DEPS_137 += build/$(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_137 += build/$(CONFIG)/bin/libhttp.a
endif
DEPS_137 += build/$(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_137 += build/$(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_137 += build/$(CONFIG)/inc/ejs.slots.h
DEPS_137 += build/$(CONFIG)/inc/ejs.web.slots.h
DEPS_137 += build/$(CONFIG)/inc/ejs.zlib.slots.h
DEPS_137 += build/$(CONFIG)/inc/ejsByteCode.h
DEPS_137 += build/$(CONFIG)/inc/ejsByteCodeTable.h
DEPS_137 += build/$(CONFIG)/inc/ejsCustomize.h
DEPS_137 += build/$(CONFIG)/inc/ejs.h
DEPS_137 += build/$(CONFIG)/inc/ejsCompiler.h
DEPS_137 += build/$(CONFIG)/obj/ecAst.o
DEPS_137 += build/$(CONFIG)/obj/ecCodeGen.o
DEPS_137 += build/$(CONFIG)/obj/ecCompiler.o
DEPS_137 += build/$(CONFIG)/obj/ecLex.o
DEPS_137 += build/$(CONFIG)/obj/ecModuleWrite.o
DEPS_137 += build/$(CONFIG)/obj/ecParser.o
DEPS_137 += build/$(CONFIG)/obj/ecState.o
DEPS_137 += build/$(CONFIG)/obj/dtoa.o
DEPS_137 += build/$(CONFIG)/obj/ejsApp.o
DEPS_137 += build/$(CONFIG)/obj/ejsArray.o
DEPS_137 += build/$(CONFIG)/obj/ejsBlock.o
DEPS_137 += build/$(CONFIG)/obj/ejsBoolean.o
DEPS_137 += build/$(CONFIG)/obj/ejsByteArray.o
DEPS_137 += build/$(CONFIG)/obj/ejsCache.o
DEPS_137 += build/$(CONFIG)/obj/ejsCmd.o
DEPS_137 += build/$(CONFIG)/obj/ejsConfig.o
DEPS_137 += build/$(CONFIG)/obj/ejsDate.o
DEPS_137 += build/$(CONFIG)/obj/ejsDebug.o
DEPS_137 += build/$(CONFIG)/obj/ejsError.o
DEPS_137 += build/$(CONFIG)/obj/ejsFile.o
DEPS_137 += build/$(CONFIG)/obj/ejsFileSystem.o
DEPS_137 += build/$(CONFIG)/obj/ejsFrame.o
DEPS_137 += build/$(CONFIG)/obj/ejsFunction.o
DEPS_137 += build/$(CONFIG)/obj/ejsGC.o
DEPS_137 += build/$(CONFIG)/obj/ejsGlobal.o
DEPS_137 += build/$(CONFIG)/obj/ejsHttp.o
DEPS_137 += build/$(CONFIG)/obj/ejsIterator.o
DEPS_137 += build/$(CONFIG)/obj/ejsJSON.o
DEPS_137 += build/$(CONFIG)/obj/ejsLocalCache.o
DEPS_137 += build/$(CONFIG)/obj/ejsMath.o
DEPS_137 += build/$(CONFIG)/obj/ejsMemory.o
DEPS_137 += build/$(CONFIG)/obj/ejsMprLog.o
DEPS_137 += build/$(CONFIG)/obj/ejsNamespace.o
DEPS_137 += build/$(CONFIG)/obj/ejsNull.o
DEPS_137 += build/$(CONFIG)/obj/ejsNumber.o
DEPS_137 += build/$(CONFIG)/obj/ejsObject.o
DEPS_137 += build/$(CONFIG)/obj/ejsPath.o
DEPS_137 += build/$(CONFIG)/obj/ejsPot.o
DEPS_137 += build/$(CONFIG)/obj/ejsRegExp.o
DEPS_137 += build/$(CONFIG)/obj/ejsSocket.o
DEPS_137 += build/$(CONFIG)/obj/ejsString.o
DEPS_137 += build/$(CONFIG)/obj/ejsSystem.o
DEPS_137 += build/$(CONFIG)/obj/ejsTimer.o
DEPS_137 += build/$(CONFIG)/obj/ejsType.o
DEPS_137 += build/$(CONFIG)/obj/ejsUri.o
DEPS_137 += build/$(CONFIG)/obj/ejsVoid.o
DEPS_137 += build/$(CONFIG)/obj/ejsWebSocket.o
DEPS_137 += build/$(CONFIG)/obj/ejsWorker.o
DEPS_137 += build/$(CONFIG)/obj/ejsXML.o
DEPS_137 += build/$(CONFIG)/obj/ejsXMLList.o
DEPS_137 += build/$(CONFIG)/obj/ejsXMLLoader.o
DEPS_137 += build/$(CONFIG)/obj/ejsByteCode.o
DEPS_137 += build/$(CONFIG)/obj/ejsException.o
DEPS_137 += build/$(CONFIG)/obj/ejsHelper.o
DEPS_137 += build/$(CONFIG)/obj/ejsInterp.o
DEPS_137 += build/$(CONFIG)/obj/ejsLoader.o
DEPS_137 += build/$(CONFIG)/obj/ejsModule.o
DEPS_137 += build/$(CONFIG)/obj/ejsScope.o
DEPS_137 += build/$(CONFIG)/obj/ejsService.o
DEPS_137 += build/$(CONFIG)/bin/libejs.a
DEPS_137 += build/$(CONFIG)/bin/utest.es
DEPS_137 += build/$(CONFIG)/bin/utest.worker
DEPS_137 += build/$(CONFIG)/obj/ejsrun.o

LIBS_137 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_137 += -lhttp
endif
LIBS_137 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_137 += -lpcre
endif

build/$(CONFIG)/bin/utest: $(DEPS_137)
	@echo '      [Link] build/$(CONFIG)/bin/utest'
	$(CC) -o build/$(CONFIG)/bin/utest -arch $(CC_ARCH) $(LDFLAGS) $(LIBPATHS) "build/$(CONFIG)/obj/ejsrun.o" $(LIBPATHS_137) $(LIBS_137) $(LIBS_137) $(LIBS) -lpam 

#
#   www
#
DEPS_138 += src/ejs.web/www

build/$(CONFIG)/bin/www: $(DEPS_138)
	( \
	cd src/ejs.web; \
	rm -fr ../../build/$(CONFIG)/bin/www ; \
	cp -r www ../../build/$(CONFIG)/bin ; \
	)

#
#   stop
#
stop: $(DEPS_139)

#
#   installBinary
#
installBinary: $(DEPS_140)
	( \
	cd .; \
	mkdir -p "$(ME_APP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	ln -s "2.4.1" "$(ME_APP_PREFIX)/latest" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp build/$(CONFIG)/bin/ejs $(ME_VAPP_PREFIX)/bin/ejs ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/ejs" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejs" "$(ME_BIN_PREFIX)/ejs" ; \
	cp build/$(CONFIG)/bin/ejsc $(ME_VAPP_PREFIX)/bin/ejsc ; \
	rm -f "$(ME_BIN_PREFIX)/ejsc" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsc" "$(ME_BIN_PREFIX)/ejsc" ; \
	cp build/$(CONFIG)/bin/ejsman $(ME_VAPP_PREFIX)/bin/ejsman ; \
	rm -f "$(ME_BIN_PREFIX)/ejsman" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsman" "$(ME_BIN_PREFIX)/ejsman" ; \
	cp build/$(CONFIG)/bin/ejsmod $(ME_VAPP_PREFIX)/bin/ejsmod ; \
	rm -f "$(ME_BIN_PREFIX)/ejsmod" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsmod" "$(ME_BIN_PREFIX)/ejsmod" ; \
	cp build/$(CONFIG)/bin/ejsrun $(ME_VAPP_PREFIX)/bin/ejsrun ; \
	rm -f "$(ME_BIN_PREFIX)/ejsrun" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsrun" "$(ME_BIN_PREFIX)/ejsrun" ; \
	cp build/$(CONFIG)/bin/mvc $(ME_VAPP_PREFIX)/bin/mvc ; \
	rm -f "$(ME_BIN_PREFIX)/mvc" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/mvc" "$(ME_BIN_PREFIX)/mvc" ; \
	cp build/$(CONFIG)/bin/utest $(ME_VAPP_PREFIX)/bin/utest ; \
	rm -f "$(ME_BIN_PREFIX)/utest" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/utest" "$(ME_BIN_PREFIX)/utest" ; \
	cp build/$(CONFIG)/bin/ejs.db.mapper.mod $(ME_VAPP_PREFIX)/bin/ejs.db.mapper.mod ; \
	cp build/$(CONFIG)/bin/ejs.db.mod $(ME_VAPP_PREFIX)/bin/ejs.db.mod ; \
	cp build/$(CONFIG)/bin/ejs.db.sqlite.mod $(ME_VAPP_PREFIX)/bin/ejs.db.sqlite.mod ; \
	cp build/$(CONFIG)/bin/ejs.mod $(ME_VAPP_PREFIX)/bin/ejs.mod ; \
	cp build/$(CONFIG)/bin/ejs.mvc.mod $(ME_VAPP_PREFIX)/bin/ejs.mvc.mod ; \
	cp build/$(CONFIG)/bin/ejs.tar.mod $(ME_VAPP_PREFIX)/bin/ejs.tar.mod ; \
	cp build/$(CONFIG)/bin/ejs.template.mod $(ME_VAPP_PREFIX)/bin/ejs.template.mod ; \
	cp build/$(CONFIG)/bin/ejs.unix.mod $(ME_VAPP_PREFIX)/bin/ejs.unix.mod ; \
	cp build/$(CONFIG)/bin/ejs.web.mod $(ME_VAPP_PREFIX)/bin/ejs.web.mod ; \
	cp build/$(CONFIG)/bin/ejs.zlib.mod $(ME_VAPP_PREFIX)/bin/ejs.zlib.mod ; \
	cp build/$(CONFIG)/bin/mvc.es $(ME_VAPP_PREFIX)/bin/mvc.es ; \
	cp build/$(CONFIG)/bin/utest.es $(ME_VAPP_PREFIX)/bin/utest.es ; \
	cp build/$(CONFIG)/bin/utest.worker $(ME_VAPP_PREFIX)/bin/utest.worker ; \
	if [ "$(ME_COM_SSL)" = 1 ]; then true ; \
	cp src/paks/est/ca.crt $(ME_VAPP_PREFIX)/bin/ca.crt ; \
	fi ; \
	if [ "$(ME_COM_OPENSSL)" = 1 ]; then true ; \
	cp build/$(CONFIG)/bin/libssl*.dylib* $(ME_VAPP_PREFIX)/bin/libssl*.dylib* ; \
	cp build/$(CONFIG)/bin/libcrypto*.dylib* $(ME_VAPP_PREFIX)/bin/libcrypto*.dylib* ; \
	fi ; \
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
	mkdir -p "$(ME_VAPP_PREFIX)/bin/www" ; \
	cp src/ejs.web/www/layout.css $(ME_VAPP_PREFIX)/bin/www/layout.css ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin/www/themes" ; \
	cp src/ejs.web/www/themes/default.css $(ME_VAPP_PREFIX)/bin/www/themes/default.css ; \
	mkdir -p "$(ME_VAPP_PREFIX)/inc" ; \
	cp src/ejs.h $(ME_VAPP_PREFIX)/inc/ejs.h ; \
	mkdir -p "$(ME_INC_PREFIX)/ejs" ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.h" "$(ME_INC_PREFIX)/ejs/ejs.h" ; \
	cp src/ejsByteCode.h $(ME_VAPP_PREFIX)/inc/ejsByteCode.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsByteCode.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsByteCode.h" "$(ME_INC_PREFIX)/ejs/ejsByteCode.h" ; \
	cp src/ejsByteCodeTable.h $(ME_VAPP_PREFIX)/inc/ejsByteCodeTable.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsByteCodeTable.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsByteCodeTable.h" "$(ME_INC_PREFIX)/ejs/ejsByteCodeTable.h" ; \
	cp src/ejsCompiler.h $(ME_VAPP_PREFIX)/inc/ejsCompiler.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsCompiler.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsCompiler.h" "$(ME_INC_PREFIX)/ejs/ejsCompiler.h" ; \
	cp src/ejsCustomize.h $(ME_VAPP_PREFIX)/inc/ejsCustomize.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsCustomize.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsCustomize.h" "$(ME_INC_PREFIX)/ejs/ejsCustomize.h" ; \
	cp src/vm/ejsByteGoto.h $(ME_VAPP_PREFIX)/inc/ejsByteGoto.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsByteGoto.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsByteGoto.h" "$(ME_INC_PREFIX)/ejs/ejsByteGoto.h" ; \
	cp src/slots/ejs.cache.local.slots.h $(ME_VAPP_PREFIX)/inc/ejs.cache.local.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.cache.local.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.cache.local.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.cache.local.slots.h" ; \
	cp src/slots/ejs.db.sqlite.slots.h $(ME_VAPP_PREFIX)/inc/ejs.db.sqlite.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.db.sqlite.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.db.sqlite.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.db.sqlite.slots.h" ; \
	cp src/slots/ejs.slots.h $(ME_VAPP_PREFIX)/inc/ejs.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.slots.h" ; \
	cp src/slots/ejs.web.slots.h $(ME_VAPP_PREFIX)/inc/ejs.web.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.web.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.web.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.web.slots.h" ; \
	cp src/slots/ejs.zlib.slots.h $(ME_VAPP_PREFIX)/inc/ejs.zlib.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.zlib.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.zlib.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.zlib.slots.h" ; \
	cp src/paks/est/est.h $(ME_VAPP_PREFIX)/inc/est.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/est.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/est.h" "$(ME_INC_PREFIX)/ejs/est.h" ; \
	cp src/paks/http/http.h $(ME_VAPP_PREFIX)/inc/http.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/http.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/http.h" "$(ME_INC_PREFIX)/ejs/http.h" ; \
	cp src/paks/mpr/mpr.h $(ME_VAPP_PREFIX)/inc/mpr.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/mpr.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/mpr.h" "$(ME_INC_PREFIX)/ejs/mpr.h" ; \
	cp src/paks/osdep/osdep.h $(ME_VAPP_PREFIX)/inc/osdep.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/osdep.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/osdep.h" "$(ME_INC_PREFIX)/ejs/osdep.h" ; \
	cp src/paks/pcre/pcre.h $(ME_VAPP_PREFIX)/inc/pcre.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/pcre.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/pcre.h" "$(ME_INC_PREFIX)/ejs/pcre.h" ; \
	cp src/paks/sqlite/sqlite3.h $(ME_VAPP_PREFIX)/inc/sqlite3.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/sqlite3.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/sqlite3.h" "$(ME_INC_PREFIX)/ejs/sqlite3.h" ; \
	cp src/paks/zlib/zlib.h $(ME_VAPP_PREFIX)/inc/zlib.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/zlib.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/zlib.h" "$(ME_INC_PREFIX)/ejs/zlib.h" ; \
	cp src/cmd/ejsmod.h $(ME_VAPP_PREFIX)/inc/ejsmod.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsmod.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsmod.h" "$(ME_INC_PREFIX)/ejs/ejsmod.h" ; \
	cp src/core/default.slots.h $(ME_VAPP_PREFIX)/inc/default.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/default.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/default.slots.h" "$(ME_INC_PREFIX)/ejs/default.slots.h" ; \
	cp src/ejs.db.sqlite/ejs.db.sqlite.slots.h $(ME_VAPP_PREFIX)/inc/ejs.db.sqlite.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.db.sqlite.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.db.sqlite.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.db.sqlite.slots.h" ; \
	cp src/ejs.h $(ME_VAPP_PREFIX)/inc/ejs.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.h" "$(ME_INC_PREFIX)/ejs/ejs.h" ; \
	cp src/ejs.web/ejsWeb.h $(ME_VAPP_PREFIX)/inc/ejsWeb.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsWeb.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsWeb.h" "$(ME_INC_PREFIX)/ejs/ejsWeb.h" ; \
	cp src/ejsByteCode.h $(ME_VAPP_PREFIX)/inc/ejsByteCode.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsByteCode.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsByteCode.h" "$(ME_INC_PREFIX)/ejs/ejsByteCode.h" ; \
	cp src/ejsByteCodeTable.h $(ME_VAPP_PREFIX)/inc/ejsByteCodeTable.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsByteCodeTable.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsByteCodeTable.h" "$(ME_INC_PREFIX)/ejs/ejsByteCodeTable.h" ; \
	cp src/ejsCompiler.h $(ME_VAPP_PREFIX)/inc/ejsCompiler.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsCompiler.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsCompiler.h" "$(ME_INC_PREFIX)/ejs/ejsCompiler.h" ; \
	cp src/ejsCustomize.h $(ME_VAPP_PREFIX)/inc/ejsCustomize.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsCustomize.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsCustomize.h" "$(ME_INC_PREFIX)/ejs/ejsCustomize.h" ; \
	cp src/paks/est/est.h $(ME_VAPP_PREFIX)/inc/est.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/est.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/est.h" "$(ME_INC_PREFIX)/ejs/est.h" ; \
	cp src/paks/http/http.h $(ME_VAPP_PREFIX)/inc/http.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/http.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/http.h" "$(ME_INC_PREFIX)/ejs/http.h" ; \
	cp src/paks/mpr/mpr.h $(ME_VAPP_PREFIX)/inc/mpr.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/mpr.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/mpr.h" "$(ME_INC_PREFIX)/ejs/mpr.h" ; \
	cp src/paks/osdep/osdep.h $(ME_VAPP_PREFIX)/inc/osdep.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/osdep.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/osdep.h" "$(ME_INC_PREFIX)/ejs/osdep.h" ; \
	cp src/paks/pcre/pcre.h $(ME_VAPP_PREFIX)/inc/pcre.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/pcre.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/pcre.h" "$(ME_INC_PREFIX)/ejs/pcre.h" ; \
	cp src/paks/sqlite/sqlite3.h $(ME_VAPP_PREFIX)/inc/sqlite3.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/sqlite3.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/sqlite3.h" "$(ME_INC_PREFIX)/ejs/sqlite3.h" ; \
	cp src/paks/zlib/zlib.h $(ME_VAPP_PREFIX)/inc/zlib.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/zlib.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/zlib.h" "$(ME_INC_PREFIX)/ejs/zlib.h" ; \
	cp src/slots/ejs.cache.local.slots.h $(ME_VAPP_PREFIX)/inc/ejs.cache.local.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.cache.local.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.cache.local.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.cache.local.slots.h" ; \
	cp src/slots/ejs.db.sqlite.slots.h $(ME_VAPP_PREFIX)/inc/ejs.db.sqlite.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.db.sqlite.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.db.sqlite.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.db.sqlite.slots.h" ; \
	cp src/slots/ejs.slots.h $(ME_VAPP_PREFIX)/inc/ejs.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.slots.h" ; \
	cp src/slots/ejs.web.slots.h $(ME_VAPP_PREFIX)/inc/ejs.web.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.web.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.web.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.web.slots.h" ; \
	cp src/slots/ejs.zlib.slots.h $(ME_VAPP_PREFIX)/inc/ejs.zlib.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.zlib.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.zlib.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.zlib.slots.h" ; \
	cp src/vm/ejsByteGoto.h $(ME_VAPP_PREFIX)/inc/ejsByteGoto.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejsByteGoto.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejsByteGoto.h" "$(ME_INC_PREFIX)/ejs/ejsByteGoto.h" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/doc/man1" ; \
	cp doc/man/ejs.1 $(ME_VAPP_PREFIX)/doc/man1/ejs.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/ejs.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/ejs.1" "$(ME_MAN_PREFIX)/man1/ejs.1" ; \
	cp doc/man/ejsc.1 $(ME_VAPP_PREFIX)/doc/man1/ejsc.1 ; \
	rm -f "$(ME_MAN_PREFIX)/man1/ejsc.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/ejsc.1" "$(ME_MAN_PREFIX)/man1/ejsc.1" ; \
	cp doc/man/ejsmod.1 $(ME_VAPP_PREFIX)/doc/man1/ejsmod.1 ; \
	rm -f "$(ME_MAN_PREFIX)/man1/ejsmod.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/ejsmod.1" "$(ME_MAN_PREFIX)/man1/ejsmod.1" ; \
	cp doc/man/http.1 $(ME_VAPP_PREFIX)/doc/man1/http.1 ; \
	rm -f "$(ME_MAN_PREFIX)/man1/http.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/http.1" "$(ME_MAN_PREFIX)/man1/http.1" ; \
	cp doc/man/makerom.1 $(ME_VAPP_PREFIX)/doc/man1/makerom.1 ; \
	rm -f "$(ME_MAN_PREFIX)/man1/makerom.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/makerom.1" "$(ME_MAN_PREFIX)/man1/makerom.1" ; \
	cp doc/man/manager.1 $(ME_VAPP_PREFIX)/doc/man1/manager.1 ; \
	rm -f "$(ME_MAN_PREFIX)/man1/manager.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/manager.1" "$(ME_MAN_PREFIX)/man1/manager.1" ; \
	cp doc/man/mvc.1 $(ME_VAPP_PREFIX)/doc/man1/mvc.1 ; \
	rm -f "$(ME_MAN_PREFIX)/man1/mvc.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man1/mvc.1" "$(ME_MAN_PREFIX)/man1/mvc.1" ; \
	)

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
	( \
	cd .; \
	rm -fr "$(ME_VAPP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	rmdir -p "$(ME_APP_PREFIX)" 2>/dev/null ; true ; \
	)

#
#   version
#
version: $(DEPS_144)
	echo 2.4.1

