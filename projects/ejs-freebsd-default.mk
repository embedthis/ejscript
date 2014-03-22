#
#   ejs-freebsd-default.mk -- Makefile to build Embedthis Ejscript for freebsd
#

NAME                  := ejs
VERSION               := 2.3.5
PROFILE               ?= default
ARCH                  ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
CC_ARCH               ?= $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                    ?= freebsd
CC                    ?= gcc
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
LBIN                  ?= $(CONFIG)/bin
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
ME_COM_WINSDK         ?= 0
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

ME_COM_COMPILER_PATH  ?= gcc
ME_COM_EST_PATH       ?= src/paks/est
ME_COM_HTTP_PATH      ?= src/paks/http
ME_COM_LIB_PATH       ?= ar
ME_COM_MATRIXSSL_PATH ?= /usr/src/matrixssl
ME_COM_MPR_PATH       ?= src/paks/mpr
ME_COM_NANOSSL_PATH   ?= /usr/src/nanossl
ME_COM_OPENSSL_PATH   ?= [object Object]
ME_COM_OSDEP_PATH     ?= src/paks/osdep
ME_COM_PCRE_PATH      ?= src/paks/pcre
ME_COM_SQLITE_PATH    ?= src/paks/sqlite
ME_COM_SSL_PATH       ?= src/paks/ssl
ME_COM_ZLIB_PATH      ?= src/paks/zlib

CFLAGS                += -fPIC -w
DFLAGS                += -D_REENTRANT -DPIC $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_EST=$(ME_COM_EST) -DME_COM_HTTP=$(ME_COM_HTTP) -DME_COM_MATRIXSSL=$(ME_COM_MATRIXSSL) -DME_COM_NANOSSL=$(ME_COM_NANOSSL) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_PCRE=$(ME_COM_PCRE) -DME_COM_SQLITE=$(ME_COM_SQLITE) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) -DME_COM_WINSDK=$(ME_COM_WINSDK) -DME_COM_ZLIB=$(ME_COM_ZLIB) 
IFLAGS                += "-I$(CONFIG)/inc"
LDFLAGS               += 
LIBPATHS              += -L$(CONFIG)/bin
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


ifeq ($(ME_COM_EST),1)
    TARGETS           += $(CONFIG)/bin/ca.crt
endif
TARGETS               += $(CONFIG)/bin/ejs
TARGETS               += $(CONFIG)/bin/ejs.db.mapper.mod
TARGETS               += $(CONFIG)/bin/ejs.mail.mod
TARGETS               += $(CONFIG)/bin/ejs.mvc.mod
TARGETS               += $(CONFIG)/bin/ejs.tar.mod
TARGETS               += $(CONFIG)/bin/ejs.zlib.mod
TARGETS               += $(CONFIG)/bin/ejsrun
ifeq ($(ME_COM_HTTP),1)
    TARGETS           += $(CONFIG)/bin/http
endif
TARGETS               += $(CONFIG)/bin/libejs.web.so
TARGETS               += $(CONFIG)/bin/libejs.zlib.so
ifeq ($(ME_COM_EST),1)
    TARGETS           += $(CONFIG)/bin/libest.so
endif
TARGETS               += $(CONFIG)/bin/libmprssl.so
ifeq ($(ME_COM_SQLITE),1)
    TARGETS           += $(CONFIG)/bin/libsql.so
endif
TARGETS               += $(CONFIG)/bin/ejsman
TARGETS               += $(CONFIG)/bin/mvc
ifeq ($(ME_COM_SQLITE),1)
    TARGETS           += $(CONFIG)/bin/sqlite
endif
TARGETS               += $(CONFIG)/bin/utest
TARGETS               += $(CONFIG)/bin/www

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
	@[ ! -x $(CONFIG)/bin ] && mkdir -p $(CONFIG)/bin; true
	@[ ! -x $(CONFIG)/inc ] && mkdir -p $(CONFIG)/inc; true
	@[ ! -x $(CONFIG)/obj ] && mkdir -p $(CONFIG)/obj; true
	@[ ! -f $(CONFIG)/inc/osdep.h ] && cp src/paks/osdep/osdep.h $(CONFIG)/inc/osdep.h ; true
	@if ! diff $(CONFIG)/inc/osdep.h src/paks/osdep/osdep.h >/dev/null ; then\
		cp src/paks/osdep/osdep.h $(CONFIG)/inc/osdep.h  ; \
	fi; true
	@[ ! -f $(CONFIG)/inc/me.h ] && cp projects/ejs-freebsd-default-me.h $(CONFIG)/inc/me.h ; true
	@if ! diff $(CONFIG)/inc/me.h projects/ejs-freebsd-default-me.h >/dev/null ; then\
		cp projects/ejs-freebsd-default-me.h $(CONFIG)/inc/me.h  ; \
	fi; true
	@if [ -f "$(CONFIG)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != " ` cat $(CONFIG)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build: "`cat $(CONFIG)/.makeflags`"" ; \
		fi ; \
	fi
	@echo $(MAKEFLAGS) >$(CONFIG)/.makeflags

clean:
	rm -f "$(CONFIG)/obj/doc.o"
	rm -f "$(CONFIG)/obj/docFiles.o"
	rm -f "$(CONFIG)/obj/dtoa.o"
	rm -f "$(CONFIG)/obj/ecAst.o"
	rm -f "$(CONFIG)/obj/ecCodeGen.o"
	rm -f "$(CONFIG)/obj/ecCompiler.o"
	rm -f "$(CONFIG)/obj/ecLex.o"
	rm -f "$(CONFIG)/obj/ecModuleWrite.o"
	rm -f "$(CONFIG)/obj/ecParser.o"
	rm -f "$(CONFIG)/obj/ecState.o"
	rm -f "$(CONFIG)/obj/ejs.o"
	rm -f "$(CONFIG)/obj/ejsApp.o"
	rm -f "$(CONFIG)/obj/ejsArray.o"
	rm -f "$(CONFIG)/obj/ejsBlock.o"
	rm -f "$(CONFIG)/obj/ejsBoolean.o"
	rm -f "$(CONFIG)/obj/ejsByteArray.o"
	rm -f "$(CONFIG)/obj/ejsByteCode.o"
	rm -f "$(CONFIG)/obj/ejsCache.o"
	rm -f "$(CONFIG)/obj/ejsCmd.o"
	rm -f "$(CONFIG)/obj/ejsConfig.o"
	rm -f "$(CONFIG)/obj/ejsDate.o"
	rm -f "$(CONFIG)/obj/ejsDebug.o"
	rm -f "$(CONFIG)/obj/ejsError.o"
	rm -f "$(CONFIG)/obj/ejsException.o"
	rm -f "$(CONFIG)/obj/ejsFile.o"
	rm -f "$(CONFIG)/obj/ejsFileSystem.o"
	rm -f "$(CONFIG)/obj/ejsFrame.o"
	rm -f "$(CONFIG)/obj/ejsFunction.o"
	rm -f "$(CONFIG)/obj/ejsGC.o"
	rm -f "$(CONFIG)/obj/ejsGlobal.o"
	rm -f "$(CONFIG)/obj/ejsHelper.o"
	rm -f "$(CONFIG)/obj/ejsHttp.o"
	rm -f "$(CONFIG)/obj/ejsHttpServer.o"
	rm -f "$(CONFIG)/obj/ejsInterp.o"
	rm -f "$(CONFIG)/obj/ejsIterator.o"
	rm -f "$(CONFIG)/obj/ejsJSON.o"
	rm -f "$(CONFIG)/obj/ejsLoader.o"
	rm -f "$(CONFIG)/obj/ejsLocalCache.o"
	rm -f "$(CONFIG)/obj/ejsMath.o"
	rm -f "$(CONFIG)/obj/ejsMemory.o"
	rm -f "$(CONFIG)/obj/ejsModule.o"
	rm -f "$(CONFIG)/obj/ejsMprLog.o"
	rm -f "$(CONFIG)/obj/ejsNamespace.o"
	rm -f "$(CONFIG)/obj/ejsNull.o"
	rm -f "$(CONFIG)/obj/ejsNumber.o"
	rm -f "$(CONFIG)/obj/ejsObject.o"
	rm -f "$(CONFIG)/obj/ejsPath.o"
	rm -f "$(CONFIG)/obj/ejsPot.o"
	rm -f "$(CONFIG)/obj/ejsRegExp.o"
	rm -f "$(CONFIG)/obj/ejsRequest.o"
	rm -f "$(CONFIG)/obj/ejsScope.o"
	rm -f "$(CONFIG)/obj/ejsService.o"
	rm -f "$(CONFIG)/obj/ejsSession.o"
	rm -f "$(CONFIG)/obj/ejsSocket.o"
	rm -f "$(CONFIG)/obj/ejsString.o"
	rm -f "$(CONFIG)/obj/ejsSystem.o"
	rm -f "$(CONFIG)/obj/ejsTimer.o"
	rm -f "$(CONFIG)/obj/ejsType.o"
	rm -f "$(CONFIG)/obj/ejsUri.o"
	rm -f "$(CONFIG)/obj/ejsVoid.o"
	rm -f "$(CONFIG)/obj/ejsWeb.o"
	rm -f "$(CONFIG)/obj/ejsWebSocket.o"
	rm -f "$(CONFIG)/obj/ejsWorker.o"
	rm -f "$(CONFIG)/obj/ejsXML.o"
	rm -f "$(CONFIG)/obj/ejsXMLList.o"
	rm -f "$(CONFIG)/obj/ejsXMLLoader.o"
	rm -f "$(CONFIG)/obj/ejsZlib.o"
	rm -f "$(CONFIG)/obj/ejsc.o"
	rm -f "$(CONFIG)/obj/ejsmod.o"
	rm -f "$(CONFIG)/obj/ejsrun.o"
	rm -f "$(CONFIG)/obj/estLib.o"
	rm -f "$(CONFIG)/obj/http.o"
	rm -f "$(CONFIG)/obj/httpLib.o"
	rm -f "$(CONFIG)/obj/listing.o"
	rm -f "$(CONFIG)/obj/makerom.o"
	rm -f "$(CONFIG)/obj/manager.o"
	rm -f "$(CONFIG)/obj/mprLib.o"
	rm -f "$(CONFIG)/obj/mprSsl.o"
	rm -f "$(CONFIG)/obj/pcre.o"
	rm -f "$(CONFIG)/obj/slotGen.o"
	rm -f "$(CONFIG)/obj/sqlite.o"
	rm -f "$(CONFIG)/obj/sqlite3.o"
	rm -f "$(CONFIG)/obj/zlib.o"
	rm -f "$(CONFIG)/bin/ca.crt"
	rm -f "$(CONFIG)/bin/ejs"
	rm -f "$(CONFIG)/bin/ejsc"
	rm -f "$(CONFIG)/bin/ejsmod"
	rm -f "$(CONFIG)/bin/ejsrun"
	rm -f "$(CONFIG)/bin/http"
	rm -f "$(CONFIG)/bin/libejs.so"
	rm -f "$(CONFIG)/bin/libejs.web.so"
	rm -f "$(CONFIG)/bin/libejs.zlib.so"
	rm -f "$(CONFIG)/bin/libest.so"
	rm -f "$(CONFIG)/bin/libhttp.so"
	rm -f "$(CONFIG)/bin/libmpr.so"
	rm -f "$(CONFIG)/bin/libmprssl.so"
	rm -f "$(CONFIG)/bin/libpcre.so"
	rm -f "$(CONFIG)/bin/libsql.so"
	rm -f "$(CONFIG)/bin/libzlib.so"
	rm -f "$(CONFIG)/bin/makerom"
	rm -f "$(CONFIG)/bin/ejsman"
	rm -f "$(CONFIG)/bin/sqlite"
	rm -f "$(CONFIG)/bin/utest"

clobber: clean
	rm -fr ./$(CONFIG)


ifeq ($(ME_COM_EST),1)
#
#   ca-crt
#
DEPS_1 += src/paks/est/ca.crt

$(CONFIG)/bin/ca.crt: $(DEPS_1)
	@echo '      [Copy] $(CONFIG)/bin/ca.crt'
	mkdir -p "$(CONFIG)/bin"
	cp src/paks/est/ca.crt $(CONFIG)/bin/ca.crt
endif

#
#   mpr.h
#
$(CONFIG)/inc/mpr.h: $(DEPS_2)
	@echo '      [Copy] $(CONFIG)/inc/mpr.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/mpr/mpr.h $(CONFIG)/inc/mpr.h

#
#   me.h
#
$(CONFIG)/inc/me.h: $(DEPS_3)
	@echo '      [Copy] $(CONFIG)/inc/me.h'

#
#   osdep.h
#
$(CONFIG)/inc/osdep.h: $(DEPS_4)
	@echo '      [Copy] $(CONFIG)/inc/osdep.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/osdep/osdep.h $(CONFIG)/inc/osdep.h

#
#   mprLib.o
#
DEPS_5 += $(CONFIG)/inc/me.h
DEPS_5 += $(CONFIG)/inc/mpr.h
DEPS_5 += $(CONFIG)/inc/osdep.h

$(CONFIG)/obj/mprLib.o: \
    src/paks/mpr/mprLib.c $(DEPS_5)
	@echo '   [Compile] $(CONFIG)/obj/mprLib.o'
	$(CC) -c -o $(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/mprLib.c

#
#   libmpr
#
DEPS_6 += $(CONFIG)/inc/mpr.h
DEPS_6 += $(CONFIG)/inc/me.h
DEPS_6 += $(CONFIG)/inc/osdep.h
DEPS_6 += $(CONFIG)/obj/mprLib.o

$(CONFIG)/bin/libmpr.so: $(DEPS_6)
	@echo '      [Link] $(CONFIG)/bin/libmpr.so'
	$(CC) -shared -o $(CONFIG)/bin/libmpr.so $(LIBPATHS) "$(CONFIG)/obj/mprLib.o" $(LIBS) 

#
#   pcre.h
#
$(CONFIG)/inc/pcre.h: $(DEPS_7)
	@echo '      [Copy] $(CONFIG)/inc/pcre.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/pcre/pcre.h $(CONFIG)/inc/pcre.h

#
#   pcre.o
#
DEPS_8 += $(CONFIG)/inc/me.h
DEPS_8 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
    src/paks/pcre/pcre.c $(DEPS_8)
	@echo '   [Compile] $(CONFIG)/obj/pcre.o'
	$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/pcre/pcre.c

ifeq ($(ME_COM_PCRE),1)
#
#   libpcre
#
DEPS_9 += $(CONFIG)/inc/pcre.h
DEPS_9 += $(CONFIG)/inc/me.h
DEPS_9 += $(CONFIG)/obj/pcre.o

$(CONFIG)/bin/libpcre.so: $(DEPS_9)
	@echo '      [Link] $(CONFIG)/bin/libpcre.so'
	$(CC) -shared -o $(CONFIG)/bin/libpcre.so $(LIBPATHS) "$(CONFIG)/obj/pcre.o" $(LIBS) 
endif

#
#   http.h
#
$(CONFIG)/inc/http.h: $(DEPS_10)
	@echo '      [Copy] $(CONFIG)/inc/http.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/http/http.h $(CONFIG)/inc/http.h

#
#   httpLib.o
#
DEPS_11 += $(CONFIG)/inc/me.h
DEPS_11 += $(CONFIG)/inc/http.h
DEPS_11 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/httpLib.o: \
    src/paks/http/httpLib.c $(DEPS_11)
	@echo '   [Compile] $(CONFIG)/obj/httpLib.o'
	$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/httpLib.c

ifeq ($(ME_COM_HTTP),1)
#
#   libhttp
#
DEPS_12 += $(CONFIG)/inc/mpr.h
DEPS_12 += $(CONFIG)/inc/me.h
DEPS_12 += $(CONFIG)/inc/osdep.h
DEPS_12 += $(CONFIG)/obj/mprLib.o
DEPS_12 += $(CONFIG)/bin/libmpr.so
DEPS_12 += $(CONFIG)/inc/pcre.h
DEPS_12 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_12 += $(CONFIG)/bin/libpcre.so
endif
DEPS_12 += $(CONFIG)/inc/http.h
DEPS_12 += $(CONFIG)/obj/httpLib.o

LIBS_12 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_12 += -lpcre
endif

$(CONFIG)/bin/libhttp.so: $(DEPS_12)
	@echo '      [Link] $(CONFIG)/bin/libhttp.so'
	$(CC) -shared -o $(CONFIG)/bin/libhttp.so $(LIBPATHS) "$(CONFIG)/obj/httpLib.o" $(LIBPATHS_12) $(LIBS_12) $(LIBS_12) $(LIBS) 
endif

#
#   ejs.cache.local.slots.h
#
$(CONFIG)/inc/ejs.cache.local.slots.h: $(DEPS_13)
	@echo '      [Copy] $(CONFIG)/inc/ejs.cache.local.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/slots/ejs.cache.local.slots.h $(CONFIG)/inc/ejs.cache.local.slots.h

#
#   ejs.db.sqlite.slots.h
#
$(CONFIG)/inc/ejs.db.sqlite.slots.h: $(DEPS_14)
	@echo '      [Copy] $(CONFIG)/inc/ejs.db.sqlite.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/slots/ejs.db.sqlite.slots.h $(CONFIG)/inc/ejs.db.sqlite.slots.h

#
#   ejs.slots.h
#
$(CONFIG)/inc/ejs.slots.h: $(DEPS_15)
	@echo '      [Copy] $(CONFIG)/inc/ejs.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/slots/ejs.slots.h $(CONFIG)/inc/ejs.slots.h

#
#   ejs.web.slots.h
#
$(CONFIG)/inc/ejs.web.slots.h: $(DEPS_16)
	@echo '      [Copy] $(CONFIG)/inc/ejs.web.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/slots/ejs.web.slots.h $(CONFIG)/inc/ejs.web.slots.h

#
#   ejs.zlib.slots.h
#
$(CONFIG)/inc/ejs.zlib.slots.h: $(DEPS_17)
	@echo '      [Copy] $(CONFIG)/inc/ejs.zlib.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/slots/ejs.zlib.slots.h $(CONFIG)/inc/ejs.zlib.slots.h

#
#   ejsByteCode.h
#
$(CONFIG)/inc/ejsByteCode.h: $(DEPS_18)
	@echo '      [Copy] $(CONFIG)/inc/ejsByteCode.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejsByteCode.h $(CONFIG)/inc/ejsByteCode.h

#
#   ejsByteCodeTable.h
#
$(CONFIG)/inc/ejsByteCodeTable.h: $(DEPS_19)
	@echo '      [Copy] $(CONFIG)/inc/ejsByteCodeTable.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejsByteCodeTable.h $(CONFIG)/inc/ejsByteCodeTable.h

#
#   ejsCustomize.h
#
$(CONFIG)/inc/ejsCustomize.h: $(DEPS_20)
	@echo '      [Copy] $(CONFIG)/inc/ejsCustomize.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejsCustomize.h $(CONFIG)/inc/ejsCustomize.h

#
#   ejs.h
#
DEPS_21 += $(CONFIG)/inc/mpr.h
DEPS_21 += $(CONFIG)/inc/http.h
DEPS_21 += $(CONFIG)/inc/ejsByteCode.h
DEPS_21 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_21 += $(CONFIG)/inc/ejs.slots.h
DEPS_21 += $(CONFIG)/inc/ejsCustomize.h

$(CONFIG)/inc/ejs.h: $(DEPS_21)
	@echo '      [Copy] $(CONFIG)/inc/ejs.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejs.h $(CONFIG)/inc/ejs.h

#
#   ejsCompiler.h
#
$(CONFIG)/inc/ejsCompiler.h: $(DEPS_22)
	@echo '      [Copy] $(CONFIG)/inc/ejsCompiler.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejsCompiler.h $(CONFIG)/inc/ejsCompiler.h

#
#   ecAst.o
#
DEPS_23 += $(CONFIG)/inc/me.h
DEPS_23 += $(CONFIG)/inc/ejsCompiler.h
DEPS_23 += $(CONFIG)/inc/mpr.h
DEPS_23 += $(CONFIG)/inc/http.h
DEPS_23 += $(CONFIG)/inc/ejsByteCode.h
DEPS_23 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_23 += $(CONFIG)/inc/ejs.slots.h
DEPS_23 += $(CONFIG)/inc/ejsCustomize.h
DEPS_23 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ecAst.o: \
    src/compiler/ecAst.c $(DEPS_23)
	@echo '   [Compile] $(CONFIG)/obj/ecAst.o'
	$(CC) -c -o $(CONFIG)/obj/ecAst.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecAst.c

#
#   ecCodeGen.o
#
DEPS_24 += $(CONFIG)/inc/me.h
DEPS_24 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecCodeGen.o: \
    src/compiler/ecCodeGen.c $(DEPS_24)
	@echo '   [Compile] $(CONFIG)/obj/ecCodeGen.o'
	$(CC) -c -o $(CONFIG)/obj/ecCodeGen.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCodeGen.c

#
#   ecCompiler.o
#
DEPS_25 += $(CONFIG)/inc/me.h
DEPS_25 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecCompiler.o: \
    src/compiler/ecCompiler.c $(DEPS_25)
	@echo '   [Compile] $(CONFIG)/obj/ecCompiler.o'
	$(CC) -c -o $(CONFIG)/obj/ecCompiler.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCompiler.c

#
#   ecLex.o
#
DEPS_26 += $(CONFIG)/inc/me.h
DEPS_26 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecLex.o: \
    src/compiler/ecLex.c $(DEPS_26)
	@echo '   [Compile] $(CONFIG)/obj/ecLex.o'
	$(CC) -c -o $(CONFIG)/obj/ecLex.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecLex.c

#
#   ecModuleWrite.o
#
DEPS_27 += $(CONFIG)/inc/me.h
DEPS_27 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecModuleWrite.o: \
    src/compiler/ecModuleWrite.c $(DEPS_27)
	@echo '   [Compile] $(CONFIG)/obj/ecModuleWrite.o'
	$(CC) -c -o $(CONFIG)/obj/ecModuleWrite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecModuleWrite.c

#
#   ecParser.o
#
DEPS_28 += $(CONFIG)/inc/me.h
DEPS_28 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecParser.o: \
    src/compiler/ecParser.c $(DEPS_28)
	@echo '   [Compile] $(CONFIG)/obj/ecParser.o'
	$(CC) -c -o $(CONFIG)/obj/ecParser.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecParser.c

#
#   ecState.o
#
DEPS_29 += $(CONFIG)/inc/me.h
DEPS_29 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecState.o: \
    src/compiler/ecState.c $(DEPS_29)
	@echo '   [Compile] $(CONFIG)/obj/ecState.o'
	$(CC) -c -o $(CONFIG)/obj/ecState.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecState.c

#
#   dtoa.o
#
DEPS_30 += $(CONFIG)/inc/me.h
DEPS_30 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/dtoa.o: \
    src/core/src/dtoa.c $(DEPS_30)
	@echo '   [Compile] $(CONFIG)/obj/dtoa.o'
	$(CC) -c -o $(CONFIG)/obj/dtoa.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/dtoa.c

#
#   ejsApp.o
#
DEPS_31 += $(CONFIG)/inc/me.h
DEPS_31 += $(CONFIG)/inc/mpr.h
DEPS_31 += $(CONFIG)/inc/http.h
DEPS_31 += $(CONFIG)/inc/ejsByteCode.h
DEPS_31 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_31 += $(CONFIG)/inc/ejs.slots.h
DEPS_31 += $(CONFIG)/inc/ejsCustomize.h
DEPS_31 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsApp.o: \
    src/core/src/ejsApp.c $(DEPS_31)
	@echo '   [Compile] $(CONFIG)/obj/ejsApp.o'
	$(CC) -c -o $(CONFIG)/obj/ejsApp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsApp.c

#
#   ejsArray.o
#
DEPS_32 += $(CONFIG)/inc/me.h
DEPS_32 += $(CONFIG)/inc/mpr.h
DEPS_32 += $(CONFIG)/inc/http.h
DEPS_32 += $(CONFIG)/inc/ejsByteCode.h
DEPS_32 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_32 += $(CONFIG)/inc/ejs.slots.h
DEPS_32 += $(CONFIG)/inc/ejsCustomize.h
DEPS_32 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsArray.o: \
    src/core/src/ejsArray.c $(DEPS_32)
	@echo '   [Compile] $(CONFIG)/obj/ejsArray.o'
	$(CC) -c -o $(CONFIG)/obj/ejsArray.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsArray.c

#
#   ejsBlock.o
#
DEPS_33 += $(CONFIG)/inc/me.h
DEPS_33 += $(CONFIG)/inc/mpr.h
DEPS_33 += $(CONFIG)/inc/http.h
DEPS_33 += $(CONFIG)/inc/ejsByteCode.h
DEPS_33 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_33 += $(CONFIG)/inc/ejs.slots.h
DEPS_33 += $(CONFIG)/inc/ejsCustomize.h
DEPS_33 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsBlock.o: \
    src/core/src/ejsBlock.c $(DEPS_33)
	@echo '   [Compile] $(CONFIG)/obj/ejsBlock.o'
	$(CC) -c -o $(CONFIG)/obj/ejsBlock.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBlock.c

#
#   ejsBoolean.o
#
DEPS_34 += $(CONFIG)/inc/me.h
DEPS_34 += $(CONFIG)/inc/mpr.h
DEPS_34 += $(CONFIG)/inc/http.h
DEPS_34 += $(CONFIG)/inc/ejsByteCode.h
DEPS_34 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_34 += $(CONFIG)/inc/ejs.slots.h
DEPS_34 += $(CONFIG)/inc/ejsCustomize.h
DEPS_34 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsBoolean.o: \
    src/core/src/ejsBoolean.c $(DEPS_34)
	@echo '   [Compile] $(CONFIG)/obj/ejsBoolean.o'
	$(CC) -c -o $(CONFIG)/obj/ejsBoolean.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBoolean.c

#
#   ejsByteArray.o
#
DEPS_35 += $(CONFIG)/inc/me.h
DEPS_35 += $(CONFIG)/inc/mpr.h
DEPS_35 += $(CONFIG)/inc/http.h
DEPS_35 += $(CONFIG)/inc/ejsByteCode.h
DEPS_35 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_35 += $(CONFIG)/inc/ejs.slots.h
DEPS_35 += $(CONFIG)/inc/ejsCustomize.h
DEPS_35 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsByteArray.o: \
    src/core/src/ejsByteArray.c $(DEPS_35)
	@echo '   [Compile] $(CONFIG)/obj/ejsByteArray.o'
	$(CC) -c -o $(CONFIG)/obj/ejsByteArray.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsByteArray.c

#
#   ejsCache.o
#
DEPS_36 += $(CONFIG)/inc/me.h
DEPS_36 += $(CONFIG)/inc/mpr.h
DEPS_36 += $(CONFIG)/inc/http.h
DEPS_36 += $(CONFIG)/inc/ejsByteCode.h
DEPS_36 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_36 += $(CONFIG)/inc/ejs.slots.h
DEPS_36 += $(CONFIG)/inc/ejsCustomize.h
DEPS_36 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsCache.o: \
    src/core/src/ejsCache.c $(DEPS_36)
	@echo '   [Compile] $(CONFIG)/obj/ejsCache.o'
	$(CC) -c -o $(CONFIG)/obj/ejsCache.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCache.c

#
#   ejsCmd.o
#
DEPS_37 += $(CONFIG)/inc/me.h
DEPS_37 += $(CONFIG)/inc/mpr.h
DEPS_37 += $(CONFIG)/inc/http.h
DEPS_37 += $(CONFIG)/inc/ejsByteCode.h
DEPS_37 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_37 += $(CONFIG)/inc/ejs.slots.h
DEPS_37 += $(CONFIG)/inc/ejsCustomize.h
DEPS_37 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsCmd.o: \
    src/core/src/ejsCmd.c $(DEPS_37)
	@echo '   [Compile] $(CONFIG)/obj/ejsCmd.o'
	$(CC) -c -o $(CONFIG)/obj/ejsCmd.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCmd.c

#
#   ejsConfig.o
#
DEPS_38 += $(CONFIG)/inc/me.h
DEPS_38 += $(CONFIG)/inc/mpr.h
DEPS_38 += $(CONFIG)/inc/http.h
DEPS_38 += $(CONFIG)/inc/ejsByteCode.h
DEPS_38 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_38 += $(CONFIG)/inc/ejs.slots.h
DEPS_38 += $(CONFIG)/inc/ejsCustomize.h
DEPS_38 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsConfig.o: \
    src/core/src/ejsConfig.c $(DEPS_38)
	@echo '   [Compile] $(CONFIG)/obj/ejsConfig.o'
	$(CC) -c -o $(CONFIG)/obj/ejsConfig.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsConfig.c

#
#   ejsDate.o
#
DEPS_39 += $(CONFIG)/inc/me.h
DEPS_39 += $(CONFIG)/inc/mpr.h
DEPS_39 += $(CONFIG)/inc/http.h
DEPS_39 += $(CONFIG)/inc/ejsByteCode.h
DEPS_39 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_39 += $(CONFIG)/inc/ejs.slots.h
DEPS_39 += $(CONFIG)/inc/ejsCustomize.h
DEPS_39 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsDate.o: \
    src/core/src/ejsDate.c $(DEPS_39)
	@echo '   [Compile] $(CONFIG)/obj/ejsDate.o'
	$(CC) -c -o $(CONFIG)/obj/ejsDate.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDate.c

#
#   ejsDebug.o
#
DEPS_40 += $(CONFIG)/inc/me.h
DEPS_40 += $(CONFIG)/inc/mpr.h
DEPS_40 += $(CONFIG)/inc/http.h
DEPS_40 += $(CONFIG)/inc/ejsByteCode.h
DEPS_40 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_40 += $(CONFIG)/inc/ejs.slots.h
DEPS_40 += $(CONFIG)/inc/ejsCustomize.h
DEPS_40 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsDebug.o: \
    src/core/src/ejsDebug.c $(DEPS_40)
	@echo '   [Compile] $(CONFIG)/obj/ejsDebug.o'
	$(CC) -c -o $(CONFIG)/obj/ejsDebug.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDebug.c

#
#   ejsError.o
#
DEPS_41 += $(CONFIG)/inc/me.h
DEPS_41 += $(CONFIG)/inc/mpr.h
DEPS_41 += $(CONFIG)/inc/http.h
DEPS_41 += $(CONFIG)/inc/ejsByteCode.h
DEPS_41 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_41 += $(CONFIG)/inc/ejs.slots.h
DEPS_41 += $(CONFIG)/inc/ejsCustomize.h
DEPS_41 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsError.o: \
    src/core/src/ejsError.c $(DEPS_41)
	@echo '   [Compile] $(CONFIG)/obj/ejsError.o'
	$(CC) -c -o $(CONFIG)/obj/ejsError.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsError.c

#
#   ejsFile.o
#
DEPS_42 += $(CONFIG)/inc/me.h
DEPS_42 += $(CONFIG)/inc/mpr.h
DEPS_42 += $(CONFIG)/inc/http.h
DEPS_42 += $(CONFIG)/inc/ejsByteCode.h
DEPS_42 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_42 += $(CONFIG)/inc/ejs.slots.h
DEPS_42 += $(CONFIG)/inc/ejsCustomize.h
DEPS_42 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFile.o: \
    src/core/src/ejsFile.c $(DEPS_42)
	@echo '   [Compile] $(CONFIG)/obj/ejsFile.o'
	$(CC) -c -o $(CONFIG)/obj/ejsFile.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFile.c

#
#   ejsFileSystem.o
#
DEPS_43 += $(CONFIG)/inc/me.h
DEPS_43 += $(CONFIG)/inc/mpr.h
DEPS_43 += $(CONFIG)/inc/http.h
DEPS_43 += $(CONFIG)/inc/ejsByteCode.h
DEPS_43 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_43 += $(CONFIG)/inc/ejs.slots.h
DEPS_43 += $(CONFIG)/inc/ejsCustomize.h
DEPS_43 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFileSystem.o: \
    src/core/src/ejsFileSystem.c $(DEPS_43)
	@echo '   [Compile] $(CONFIG)/obj/ejsFileSystem.o'
	$(CC) -c -o $(CONFIG)/obj/ejsFileSystem.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFileSystem.c

#
#   ejsFrame.o
#
DEPS_44 += $(CONFIG)/inc/me.h
DEPS_44 += $(CONFIG)/inc/mpr.h
DEPS_44 += $(CONFIG)/inc/http.h
DEPS_44 += $(CONFIG)/inc/ejsByteCode.h
DEPS_44 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_44 += $(CONFIG)/inc/ejs.slots.h
DEPS_44 += $(CONFIG)/inc/ejsCustomize.h
DEPS_44 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFrame.o: \
    src/core/src/ejsFrame.c $(DEPS_44)
	@echo '   [Compile] $(CONFIG)/obj/ejsFrame.o'
	$(CC) -c -o $(CONFIG)/obj/ejsFrame.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFrame.c

#
#   ejsFunction.o
#
DEPS_45 += $(CONFIG)/inc/me.h
DEPS_45 += $(CONFIG)/inc/mpr.h
DEPS_45 += $(CONFIG)/inc/http.h
DEPS_45 += $(CONFIG)/inc/ejsByteCode.h
DEPS_45 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_45 += $(CONFIG)/inc/ejs.slots.h
DEPS_45 += $(CONFIG)/inc/ejsCustomize.h
DEPS_45 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFunction.o: \
    src/core/src/ejsFunction.c $(DEPS_45)
	@echo '   [Compile] $(CONFIG)/obj/ejsFunction.o'
	$(CC) -c -o $(CONFIG)/obj/ejsFunction.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFunction.c

#
#   ejsGC.o
#
DEPS_46 += $(CONFIG)/inc/me.h
DEPS_46 += $(CONFIG)/inc/mpr.h
DEPS_46 += $(CONFIG)/inc/http.h
DEPS_46 += $(CONFIG)/inc/ejsByteCode.h
DEPS_46 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_46 += $(CONFIG)/inc/ejs.slots.h
DEPS_46 += $(CONFIG)/inc/ejsCustomize.h
DEPS_46 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsGC.o: \
    src/core/src/ejsGC.c $(DEPS_46)
	@echo '   [Compile] $(CONFIG)/obj/ejsGC.o'
	$(CC) -c -o $(CONFIG)/obj/ejsGC.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGC.c

#
#   ejsGlobal.o
#
DEPS_47 += $(CONFIG)/inc/me.h
DEPS_47 += $(CONFIG)/inc/mpr.h
DEPS_47 += $(CONFIG)/inc/http.h
DEPS_47 += $(CONFIG)/inc/ejsByteCode.h
DEPS_47 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_47 += $(CONFIG)/inc/ejs.slots.h
DEPS_47 += $(CONFIG)/inc/ejsCustomize.h
DEPS_47 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsGlobal.o: \
    src/core/src/ejsGlobal.c $(DEPS_47)
	@echo '   [Compile] $(CONFIG)/obj/ejsGlobal.o'
	$(CC) -c -o $(CONFIG)/obj/ejsGlobal.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGlobal.c

#
#   ejsHttp.o
#
DEPS_48 += $(CONFIG)/inc/me.h
DEPS_48 += $(CONFIG)/inc/mpr.h
DEPS_48 += $(CONFIG)/inc/http.h
DEPS_48 += $(CONFIG)/inc/ejsByteCode.h
DEPS_48 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_48 += $(CONFIG)/inc/ejs.slots.h
DEPS_48 += $(CONFIG)/inc/ejsCustomize.h
DEPS_48 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsHttp.o: \
    src/core/src/ejsHttp.c $(DEPS_48)
	@echo '   [Compile] $(CONFIG)/obj/ejsHttp.o'
	$(CC) -c -o $(CONFIG)/obj/ejsHttp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsHttp.c

#
#   ejsIterator.o
#
DEPS_49 += $(CONFIG)/inc/me.h
DEPS_49 += $(CONFIG)/inc/mpr.h
DEPS_49 += $(CONFIG)/inc/http.h
DEPS_49 += $(CONFIG)/inc/ejsByteCode.h
DEPS_49 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_49 += $(CONFIG)/inc/ejs.slots.h
DEPS_49 += $(CONFIG)/inc/ejsCustomize.h
DEPS_49 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsIterator.o: \
    src/core/src/ejsIterator.c $(DEPS_49)
	@echo '   [Compile] $(CONFIG)/obj/ejsIterator.o'
	$(CC) -c -o $(CONFIG)/obj/ejsIterator.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsIterator.c

#
#   ejsJSON.o
#
DEPS_50 += $(CONFIG)/inc/me.h
DEPS_50 += $(CONFIG)/inc/mpr.h
DEPS_50 += $(CONFIG)/inc/http.h
DEPS_50 += $(CONFIG)/inc/ejsByteCode.h
DEPS_50 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_50 += $(CONFIG)/inc/ejs.slots.h
DEPS_50 += $(CONFIG)/inc/ejsCustomize.h
DEPS_50 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsJSON.o: \
    src/core/src/ejsJSON.c $(DEPS_50)
	@echo '   [Compile] $(CONFIG)/obj/ejsJSON.o'
	$(CC) -c -o $(CONFIG)/obj/ejsJSON.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsJSON.c

#
#   ejsLocalCache.o
#
DEPS_51 += $(CONFIG)/inc/me.h
DEPS_51 += $(CONFIG)/inc/mpr.h
DEPS_51 += $(CONFIG)/inc/http.h
DEPS_51 += $(CONFIG)/inc/ejsByteCode.h
DEPS_51 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_51 += $(CONFIG)/inc/ejs.slots.h
DEPS_51 += $(CONFIG)/inc/ejsCustomize.h
DEPS_51 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsLocalCache.o: \
    src/core/src/ejsLocalCache.c $(DEPS_51)
	@echo '   [Compile] $(CONFIG)/obj/ejsLocalCache.o'
	$(CC) -c -o $(CONFIG)/obj/ejsLocalCache.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsLocalCache.c

#
#   ejsMath.o
#
DEPS_52 += $(CONFIG)/inc/me.h
DEPS_52 += $(CONFIG)/inc/mpr.h
DEPS_52 += $(CONFIG)/inc/http.h
DEPS_52 += $(CONFIG)/inc/ejsByteCode.h
DEPS_52 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_52 += $(CONFIG)/inc/ejs.slots.h
DEPS_52 += $(CONFIG)/inc/ejsCustomize.h
DEPS_52 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMath.o: \
    src/core/src/ejsMath.c $(DEPS_52)
	@echo '   [Compile] $(CONFIG)/obj/ejsMath.o'
	$(CC) -c -o $(CONFIG)/obj/ejsMath.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMath.c

#
#   ejsMemory.o
#
DEPS_53 += $(CONFIG)/inc/me.h
DEPS_53 += $(CONFIG)/inc/mpr.h
DEPS_53 += $(CONFIG)/inc/http.h
DEPS_53 += $(CONFIG)/inc/ejsByteCode.h
DEPS_53 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_53 += $(CONFIG)/inc/ejs.slots.h
DEPS_53 += $(CONFIG)/inc/ejsCustomize.h
DEPS_53 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMemory.o: \
    src/core/src/ejsMemory.c $(DEPS_53)
	@echo '   [Compile] $(CONFIG)/obj/ejsMemory.o'
	$(CC) -c -o $(CONFIG)/obj/ejsMemory.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMemory.c

#
#   ejsMprLog.o
#
DEPS_54 += $(CONFIG)/inc/me.h
DEPS_54 += $(CONFIG)/inc/mpr.h
DEPS_54 += $(CONFIG)/inc/http.h
DEPS_54 += $(CONFIG)/inc/ejsByteCode.h
DEPS_54 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_54 += $(CONFIG)/inc/ejs.slots.h
DEPS_54 += $(CONFIG)/inc/ejsCustomize.h
DEPS_54 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMprLog.o: \
    src/core/src/ejsMprLog.c $(DEPS_54)
	@echo '   [Compile] $(CONFIG)/obj/ejsMprLog.o'
	$(CC) -c -o $(CONFIG)/obj/ejsMprLog.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMprLog.c

#
#   ejsNamespace.o
#
DEPS_55 += $(CONFIG)/inc/me.h
DEPS_55 += $(CONFIG)/inc/mpr.h
DEPS_55 += $(CONFIG)/inc/http.h
DEPS_55 += $(CONFIG)/inc/ejsByteCode.h
DEPS_55 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_55 += $(CONFIG)/inc/ejs.slots.h
DEPS_55 += $(CONFIG)/inc/ejsCustomize.h
DEPS_55 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNamespace.o: \
    src/core/src/ejsNamespace.c $(DEPS_55)
	@echo '   [Compile] $(CONFIG)/obj/ejsNamespace.o'
	$(CC) -c -o $(CONFIG)/obj/ejsNamespace.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNamespace.c

#
#   ejsNull.o
#
DEPS_56 += $(CONFIG)/inc/me.h
DEPS_56 += $(CONFIG)/inc/mpr.h
DEPS_56 += $(CONFIG)/inc/http.h
DEPS_56 += $(CONFIG)/inc/ejsByteCode.h
DEPS_56 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_56 += $(CONFIG)/inc/ejs.slots.h
DEPS_56 += $(CONFIG)/inc/ejsCustomize.h
DEPS_56 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNull.o: \
    src/core/src/ejsNull.c $(DEPS_56)
	@echo '   [Compile] $(CONFIG)/obj/ejsNull.o'
	$(CC) -c -o $(CONFIG)/obj/ejsNull.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNull.c

#
#   ejsNumber.o
#
DEPS_57 += $(CONFIG)/inc/me.h
DEPS_57 += $(CONFIG)/inc/mpr.h
DEPS_57 += $(CONFIG)/inc/http.h
DEPS_57 += $(CONFIG)/inc/ejsByteCode.h
DEPS_57 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_57 += $(CONFIG)/inc/ejs.slots.h
DEPS_57 += $(CONFIG)/inc/ejsCustomize.h
DEPS_57 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNumber.o: \
    src/core/src/ejsNumber.c $(DEPS_57)
	@echo '   [Compile] $(CONFIG)/obj/ejsNumber.o'
	$(CC) -c -o $(CONFIG)/obj/ejsNumber.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNumber.c

#
#   ejsObject.o
#
DEPS_58 += $(CONFIG)/inc/me.h
DEPS_58 += $(CONFIG)/inc/mpr.h
DEPS_58 += $(CONFIG)/inc/http.h
DEPS_58 += $(CONFIG)/inc/ejsByteCode.h
DEPS_58 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_58 += $(CONFIG)/inc/ejs.slots.h
DEPS_58 += $(CONFIG)/inc/ejsCustomize.h
DEPS_58 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsObject.o: \
    src/core/src/ejsObject.c $(DEPS_58)
	@echo '   [Compile] $(CONFIG)/obj/ejsObject.o'
	$(CC) -c -o $(CONFIG)/obj/ejsObject.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsObject.c

#
#   ejsPath.o
#
DEPS_59 += $(CONFIG)/inc/me.h
DEPS_59 += $(CONFIG)/inc/mpr.h
DEPS_59 += $(CONFIG)/inc/http.h
DEPS_59 += $(CONFIG)/inc/ejsByteCode.h
DEPS_59 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_59 += $(CONFIG)/inc/ejs.slots.h
DEPS_59 += $(CONFIG)/inc/ejsCustomize.h
DEPS_59 += $(CONFIG)/inc/ejs.h
DEPS_59 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsPath.o: \
    src/core/src/ejsPath.c $(DEPS_59)
	@echo '   [Compile] $(CONFIG)/obj/ejsPath.o'
	$(CC) -c -o $(CONFIG)/obj/ejsPath.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPath.c

#
#   ejsPot.o
#
DEPS_60 += $(CONFIG)/inc/me.h
DEPS_60 += $(CONFIG)/inc/mpr.h
DEPS_60 += $(CONFIG)/inc/http.h
DEPS_60 += $(CONFIG)/inc/ejsByteCode.h
DEPS_60 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_60 += $(CONFIG)/inc/ejs.slots.h
DEPS_60 += $(CONFIG)/inc/ejsCustomize.h
DEPS_60 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsPot.o: \
    src/core/src/ejsPot.c $(DEPS_60)
	@echo '   [Compile] $(CONFIG)/obj/ejsPot.o'
	$(CC) -c -o $(CONFIG)/obj/ejsPot.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPot.c

#
#   ejsRegExp.o
#
DEPS_61 += $(CONFIG)/inc/me.h
DEPS_61 += $(CONFIG)/inc/mpr.h
DEPS_61 += $(CONFIG)/inc/http.h
DEPS_61 += $(CONFIG)/inc/ejsByteCode.h
DEPS_61 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_61 += $(CONFIG)/inc/ejs.slots.h
DEPS_61 += $(CONFIG)/inc/ejsCustomize.h
DEPS_61 += $(CONFIG)/inc/ejs.h
DEPS_61 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsRegExp.o: \
    src/core/src/ejsRegExp.c $(DEPS_61)
	@echo '   [Compile] $(CONFIG)/obj/ejsRegExp.o'
	$(CC) -c -o $(CONFIG)/obj/ejsRegExp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsRegExp.c

#
#   ejsSocket.o
#
DEPS_62 += $(CONFIG)/inc/me.h
DEPS_62 += $(CONFIG)/inc/mpr.h
DEPS_62 += $(CONFIG)/inc/http.h
DEPS_62 += $(CONFIG)/inc/ejsByteCode.h
DEPS_62 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_62 += $(CONFIG)/inc/ejs.slots.h
DEPS_62 += $(CONFIG)/inc/ejsCustomize.h
DEPS_62 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsSocket.o: \
    src/core/src/ejsSocket.c $(DEPS_62)
	@echo '   [Compile] $(CONFIG)/obj/ejsSocket.o'
	$(CC) -c -o $(CONFIG)/obj/ejsSocket.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSocket.c

#
#   ejsString.o
#
DEPS_63 += $(CONFIG)/inc/me.h
DEPS_63 += $(CONFIG)/inc/mpr.h
DEPS_63 += $(CONFIG)/inc/http.h
DEPS_63 += $(CONFIG)/inc/ejsByteCode.h
DEPS_63 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_63 += $(CONFIG)/inc/ejs.slots.h
DEPS_63 += $(CONFIG)/inc/ejsCustomize.h
DEPS_63 += $(CONFIG)/inc/ejs.h
DEPS_63 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsString.o: \
    src/core/src/ejsString.c $(DEPS_63)
	@echo '   [Compile] $(CONFIG)/obj/ejsString.o'
	$(CC) -c -o $(CONFIG)/obj/ejsString.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsString.c

#
#   ejsSystem.o
#
DEPS_64 += $(CONFIG)/inc/me.h
DEPS_64 += $(CONFIG)/inc/mpr.h
DEPS_64 += $(CONFIG)/inc/http.h
DEPS_64 += $(CONFIG)/inc/ejsByteCode.h
DEPS_64 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_64 += $(CONFIG)/inc/ejs.slots.h
DEPS_64 += $(CONFIG)/inc/ejsCustomize.h
DEPS_64 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsSystem.o: \
    src/core/src/ejsSystem.c $(DEPS_64)
	@echo '   [Compile] $(CONFIG)/obj/ejsSystem.o'
	$(CC) -c -o $(CONFIG)/obj/ejsSystem.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSystem.c

#
#   ejsTimer.o
#
DEPS_65 += $(CONFIG)/inc/me.h
DEPS_65 += $(CONFIG)/inc/mpr.h
DEPS_65 += $(CONFIG)/inc/http.h
DEPS_65 += $(CONFIG)/inc/ejsByteCode.h
DEPS_65 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_65 += $(CONFIG)/inc/ejs.slots.h
DEPS_65 += $(CONFIG)/inc/ejsCustomize.h
DEPS_65 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsTimer.o: \
    src/core/src/ejsTimer.c $(DEPS_65)
	@echo '   [Compile] $(CONFIG)/obj/ejsTimer.o'
	$(CC) -c -o $(CONFIG)/obj/ejsTimer.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsTimer.c

#
#   ejsType.o
#
DEPS_66 += $(CONFIG)/inc/me.h
DEPS_66 += $(CONFIG)/inc/mpr.h
DEPS_66 += $(CONFIG)/inc/http.h
DEPS_66 += $(CONFIG)/inc/ejsByteCode.h
DEPS_66 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_66 += $(CONFIG)/inc/ejs.slots.h
DEPS_66 += $(CONFIG)/inc/ejsCustomize.h
DEPS_66 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsType.o: \
    src/core/src/ejsType.c $(DEPS_66)
	@echo '   [Compile] $(CONFIG)/obj/ejsType.o'
	$(CC) -c -o $(CONFIG)/obj/ejsType.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsType.c

#
#   ejsUri.o
#
DEPS_67 += $(CONFIG)/inc/me.h
DEPS_67 += $(CONFIG)/inc/mpr.h
DEPS_67 += $(CONFIG)/inc/http.h
DEPS_67 += $(CONFIG)/inc/ejsByteCode.h
DEPS_67 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_67 += $(CONFIG)/inc/ejs.slots.h
DEPS_67 += $(CONFIG)/inc/ejsCustomize.h
DEPS_67 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsUri.o: \
    src/core/src/ejsUri.c $(DEPS_67)
	@echo '   [Compile] $(CONFIG)/obj/ejsUri.o'
	$(CC) -c -o $(CONFIG)/obj/ejsUri.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsUri.c

#
#   ejsVoid.o
#
DEPS_68 += $(CONFIG)/inc/me.h
DEPS_68 += $(CONFIG)/inc/mpr.h
DEPS_68 += $(CONFIG)/inc/http.h
DEPS_68 += $(CONFIG)/inc/ejsByteCode.h
DEPS_68 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_68 += $(CONFIG)/inc/ejs.slots.h
DEPS_68 += $(CONFIG)/inc/ejsCustomize.h
DEPS_68 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsVoid.o: \
    src/core/src/ejsVoid.c $(DEPS_68)
	@echo '   [Compile] $(CONFIG)/obj/ejsVoid.o'
	$(CC) -c -o $(CONFIG)/obj/ejsVoid.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsVoid.c

#
#   ejsWebSocket.o
#
DEPS_69 += $(CONFIG)/inc/me.h
DEPS_69 += $(CONFIG)/inc/mpr.h
DEPS_69 += $(CONFIG)/inc/http.h
DEPS_69 += $(CONFIG)/inc/ejsByteCode.h
DEPS_69 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_69 += $(CONFIG)/inc/ejs.slots.h
DEPS_69 += $(CONFIG)/inc/ejsCustomize.h
DEPS_69 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsWebSocket.o: \
    src/core/src/ejsWebSocket.c $(DEPS_69)
	@echo '   [Compile] $(CONFIG)/obj/ejsWebSocket.o'
	$(CC) -c -o $(CONFIG)/obj/ejsWebSocket.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWebSocket.c

#
#   ejsWorker.o
#
DEPS_70 += $(CONFIG)/inc/me.h
DEPS_70 += $(CONFIG)/inc/mpr.h
DEPS_70 += $(CONFIG)/inc/http.h
DEPS_70 += $(CONFIG)/inc/ejsByteCode.h
DEPS_70 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_70 += $(CONFIG)/inc/ejs.slots.h
DEPS_70 += $(CONFIG)/inc/ejsCustomize.h
DEPS_70 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsWorker.o: \
    src/core/src/ejsWorker.c $(DEPS_70)
	@echo '   [Compile] $(CONFIG)/obj/ejsWorker.o'
	$(CC) -c -o $(CONFIG)/obj/ejsWorker.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWorker.c

#
#   ejsXML.o
#
DEPS_71 += $(CONFIG)/inc/me.h
DEPS_71 += $(CONFIG)/inc/mpr.h
DEPS_71 += $(CONFIG)/inc/http.h
DEPS_71 += $(CONFIG)/inc/ejsByteCode.h
DEPS_71 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_71 += $(CONFIG)/inc/ejs.slots.h
DEPS_71 += $(CONFIG)/inc/ejsCustomize.h
DEPS_71 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXML.o: \
    src/core/src/ejsXML.c $(DEPS_71)
	@echo '   [Compile] $(CONFIG)/obj/ejsXML.o'
	$(CC) -c -o $(CONFIG)/obj/ejsXML.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXML.c

#
#   ejsXMLList.o
#
DEPS_72 += $(CONFIG)/inc/me.h
DEPS_72 += $(CONFIG)/inc/mpr.h
DEPS_72 += $(CONFIG)/inc/http.h
DEPS_72 += $(CONFIG)/inc/ejsByteCode.h
DEPS_72 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_72 += $(CONFIG)/inc/ejs.slots.h
DEPS_72 += $(CONFIG)/inc/ejsCustomize.h
DEPS_72 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXMLList.o: \
    src/core/src/ejsXMLList.c $(DEPS_72)
	@echo '   [Compile] $(CONFIG)/obj/ejsXMLList.o'
	$(CC) -c -o $(CONFIG)/obj/ejsXMLList.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLList.c

#
#   ejsXMLLoader.o
#
DEPS_73 += $(CONFIG)/inc/me.h
DEPS_73 += $(CONFIG)/inc/mpr.h
DEPS_73 += $(CONFIG)/inc/http.h
DEPS_73 += $(CONFIG)/inc/ejsByteCode.h
DEPS_73 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_73 += $(CONFIG)/inc/ejs.slots.h
DEPS_73 += $(CONFIG)/inc/ejsCustomize.h
DEPS_73 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXMLLoader.o: \
    src/core/src/ejsXMLLoader.c $(DEPS_73)
	@echo '   [Compile] $(CONFIG)/obj/ejsXMLLoader.o'
	$(CC) -c -o $(CONFIG)/obj/ejsXMLLoader.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLLoader.c

#
#   ejsByteCode.o
#
DEPS_74 += $(CONFIG)/inc/me.h
DEPS_74 += $(CONFIG)/inc/mpr.h
DEPS_74 += $(CONFIG)/inc/http.h
DEPS_74 += $(CONFIG)/inc/ejsByteCode.h
DEPS_74 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_74 += $(CONFIG)/inc/ejs.slots.h
DEPS_74 += $(CONFIG)/inc/ejsCustomize.h
DEPS_74 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsByteCode.o: \
    src/vm/ejsByteCode.c $(DEPS_74)
	@echo '   [Compile] $(CONFIG)/obj/ejsByteCode.o'
	$(CC) -c -o $(CONFIG)/obj/ejsByteCode.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsByteCode.c

#
#   ejsException.o
#
DEPS_75 += $(CONFIG)/inc/me.h
DEPS_75 += $(CONFIG)/inc/mpr.h
DEPS_75 += $(CONFIG)/inc/http.h
DEPS_75 += $(CONFIG)/inc/ejsByteCode.h
DEPS_75 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_75 += $(CONFIG)/inc/ejs.slots.h
DEPS_75 += $(CONFIG)/inc/ejsCustomize.h
DEPS_75 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsException.o: \
    src/vm/ejsException.c $(DEPS_75)
	@echo '   [Compile] $(CONFIG)/obj/ejsException.o'
	$(CC) -c -o $(CONFIG)/obj/ejsException.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsException.c

#
#   ejsHelper.o
#
DEPS_76 += $(CONFIG)/inc/me.h
DEPS_76 += $(CONFIG)/inc/mpr.h
DEPS_76 += $(CONFIG)/inc/http.h
DEPS_76 += $(CONFIG)/inc/ejsByteCode.h
DEPS_76 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_76 += $(CONFIG)/inc/ejs.slots.h
DEPS_76 += $(CONFIG)/inc/ejsCustomize.h
DEPS_76 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsHelper.o: \
    src/vm/ejsHelper.c $(DEPS_76)
	@echo '   [Compile] $(CONFIG)/obj/ejsHelper.o'
	$(CC) -c -o $(CONFIG)/obj/ejsHelper.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsHelper.c

#
#   ejsInterp.o
#
DEPS_77 += $(CONFIG)/inc/me.h
DEPS_77 += $(CONFIG)/inc/mpr.h
DEPS_77 += $(CONFIG)/inc/http.h
DEPS_77 += $(CONFIG)/inc/ejsByteCode.h
DEPS_77 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_77 += $(CONFIG)/inc/ejs.slots.h
DEPS_77 += $(CONFIG)/inc/ejsCustomize.h
DEPS_77 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsInterp.o: \
    src/vm/ejsInterp.c $(DEPS_77)
	@echo '   [Compile] $(CONFIG)/obj/ejsInterp.o'
	$(CC) -c -o $(CONFIG)/obj/ejsInterp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsInterp.c

#
#   ejsLoader.o
#
DEPS_78 += $(CONFIG)/inc/me.h
DEPS_78 += $(CONFIG)/inc/mpr.h
DEPS_78 += $(CONFIG)/inc/http.h
DEPS_78 += $(CONFIG)/inc/ejsByteCode.h
DEPS_78 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_78 += $(CONFIG)/inc/ejs.slots.h
DEPS_78 += $(CONFIG)/inc/ejsCustomize.h
DEPS_78 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsLoader.o: \
    src/vm/ejsLoader.c $(DEPS_78)
	@echo '   [Compile] $(CONFIG)/obj/ejsLoader.o'
	$(CC) -c -o $(CONFIG)/obj/ejsLoader.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsLoader.c

#
#   ejsModule.o
#
DEPS_79 += $(CONFIG)/inc/me.h
DEPS_79 += $(CONFIG)/inc/mpr.h
DEPS_79 += $(CONFIG)/inc/http.h
DEPS_79 += $(CONFIG)/inc/ejsByteCode.h
DEPS_79 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_79 += $(CONFIG)/inc/ejs.slots.h
DEPS_79 += $(CONFIG)/inc/ejsCustomize.h
DEPS_79 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsModule.o: \
    src/vm/ejsModule.c $(DEPS_79)
	@echo '   [Compile] $(CONFIG)/obj/ejsModule.o'
	$(CC) -c -o $(CONFIG)/obj/ejsModule.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsModule.c

#
#   ejsScope.o
#
DEPS_80 += $(CONFIG)/inc/me.h
DEPS_80 += $(CONFIG)/inc/mpr.h
DEPS_80 += $(CONFIG)/inc/http.h
DEPS_80 += $(CONFIG)/inc/ejsByteCode.h
DEPS_80 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_80 += $(CONFIG)/inc/ejs.slots.h
DEPS_80 += $(CONFIG)/inc/ejsCustomize.h
DEPS_80 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsScope.o: \
    src/vm/ejsScope.c $(DEPS_80)
	@echo '   [Compile] $(CONFIG)/obj/ejsScope.o'
	$(CC) -c -o $(CONFIG)/obj/ejsScope.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsScope.c

#
#   ejsService.o
#
DEPS_81 += $(CONFIG)/inc/me.h
DEPS_81 += $(CONFIG)/inc/mpr.h
DEPS_81 += $(CONFIG)/inc/http.h
DEPS_81 += $(CONFIG)/inc/ejsByteCode.h
DEPS_81 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_81 += $(CONFIG)/inc/ejs.slots.h
DEPS_81 += $(CONFIG)/inc/ejsCustomize.h
DEPS_81 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsService.o: \
    src/vm/ejsService.c $(DEPS_81)
	@echo '   [Compile] $(CONFIG)/obj/ejsService.o'
	$(CC) -c -o $(CONFIG)/obj/ejsService.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsService.c

#
#   libejs
#
DEPS_82 += $(CONFIG)/inc/mpr.h
DEPS_82 += $(CONFIG)/inc/me.h
DEPS_82 += $(CONFIG)/inc/osdep.h
DEPS_82 += $(CONFIG)/obj/mprLib.o
DEPS_82 += $(CONFIG)/bin/libmpr.so
DEPS_82 += $(CONFIG)/inc/pcre.h
DEPS_82 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_82 += $(CONFIG)/bin/libpcre.so
endif
DEPS_82 += $(CONFIG)/inc/http.h
DEPS_82 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_82 += $(CONFIG)/bin/libhttp.so
endif
DEPS_82 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_82 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_82 += $(CONFIG)/inc/ejs.slots.h
DEPS_82 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_82 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_82 += $(CONFIG)/inc/ejsByteCode.h
DEPS_82 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_82 += $(CONFIG)/inc/ejsCustomize.h
DEPS_82 += $(CONFIG)/inc/ejs.h
DEPS_82 += $(CONFIG)/inc/ejsCompiler.h
DEPS_82 += $(CONFIG)/obj/ecAst.o
DEPS_82 += $(CONFIG)/obj/ecCodeGen.o
DEPS_82 += $(CONFIG)/obj/ecCompiler.o
DEPS_82 += $(CONFIG)/obj/ecLex.o
DEPS_82 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_82 += $(CONFIG)/obj/ecParser.o
DEPS_82 += $(CONFIG)/obj/ecState.o
DEPS_82 += $(CONFIG)/obj/dtoa.o
DEPS_82 += $(CONFIG)/obj/ejsApp.o
DEPS_82 += $(CONFIG)/obj/ejsArray.o
DEPS_82 += $(CONFIG)/obj/ejsBlock.o
DEPS_82 += $(CONFIG)/obj/ejsBoolean.o
DEPS_82 += $(CONFIG)/obj/ejsByteArray.o
DEPS_82 += $(CONFIG)/obj/ejsCache.o
DEPS_82 += $(CONFIG)/obj/ejsCmd.o
DEPS_82 += $(CONFIG)/obj/ejsConfig.o
DEPS_82 += $(CONFIG)/obj/ejsDate.o
DEPS_82 += $(CONFIG)/obj/ejsDebug.o
DEPS_82 += $(CONFIG)/obj/ejsError.o
DEPS_82 += $(CONFIG)/obj/ejsFile.o
DEPS_82 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_82 += $(CONFIG)/obj/ejsFrame.o
DEPS_82 += $(CONFIG)/obj/ejsFunction.o
DEPS_82 += $(CONFIG)/obj/ejsGC.o
DEPS_82 += $(CONFIG)/obj/ejsGlobal.o
DEPS_82 += $(CONFIG)/obj/ejsHttp.o
DEPS_82 += $(CONFIG)/obj/ejsIterator.o
DEPS_82 += $(CONFIG)/obj/ejsJSON.o
DEPS_82 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_82 += $(CONFIG)/obj/ejsMath.o
DEPS_82 += $(CONFIG)/obj/ejsMemory.o
DEPS_82 += $(CONFIG)/obj/ejsMprLog.o
DEPS_82 += $(CONFIG)/obj/ejsNamespace.o
DEPS_82 += $(CONFIG)/obj/ejsNull.o
DEPS_82 += $(CONFIG)/obj/ejsNumber.o
DEPS_82 += $(CONFIG)/obj/ejsObject.o
DEPS_82 += $(CONFIG)/obj/ejsPath.o
DEPS_82 += $(CONFIG)/obj/ejsPot.o
DEPS_82 += $(CONFIG)/obj/ejsRegExp.o
DEPS_82 += $(CONFIG)/obj/ejsSocket.o
DEPS_82 += $(CONFIG)/obj/ejsString.o
DEPS_82 += $(CONFIG)/obj/ejsSystem.o
DEPS_82 += $(CONFIG)/obj/ejsTimer.o
DEPS_82 += $(CONFIG)/obj/ejsType.o
DEPS_82 += $(CONFIG)/obj/ejsUri.o
DEPS_82 += $(CONFIG)/obj/ejsVoid.o
DEPS_82 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_82 += $(CONFIG)/obj/ejsWorker.o
DEPS_82 += $(CONFIG)/obj/ejsXML.o
DEPS_82 += $(CONFIG)/obj/ejsXMLList.o
DEPS_82 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_82 += $(CONFIG)/obj/ejsByteCode.o
DEPS_82 += $(CONFIG)/obj/ejsException.o
DEPS_82 += $(CONFIG)/obj/ejsHelper.o
DEPS_82 += $(CONFIG)/obj/ejsInterp.o
DEPS_82 += $(CONFIG)/obj/ejsLoader.o
DEPS_82 += $(CONFIG)/obj/ejsModule.o
DEPS_82 += $(CONFIG)/obj/ejsScope.o
DEPS_82 += $(CONFIG)/obj/ejsService.o

ifeq ($(ME_COM_HTTP),1)
    LIBS_82 += -lhttp
endif
LIBS_82 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_82 += -lpcre
endif

$(CONFIG)/bin/libejs.so: $(DEPS_82)
	@echo '      [Link] $(CONFIG)/bin/libejs.so'
	$(CC) -shared -o $(CONFIG)/bin/libejs.so $(LIBPATHS) "$(CONFIG)/obj/ecAst.o" "$(CONFIG)/obj/ecCodeGen.o" "$(CONFIG)/obj/ecCompiler.o" "$(CONFIG)/obj/ecLex.o" "$(CONFIG)/obj/ecModuleWrite.o" "$(CONFIG)/obj/ecParser.o" "$(CONFIG)/obj/ecState.o" "$(CONFIG)/obj/dtoa.o" "$(CONFIG)/obj/ejsApp.o" "$(CONFIG)/obj/ejsArray.o" "$(CONFIG)/obj/ejsBlock.o" "$(CONFIG)/obj/ejsBoolean.o" "$(CONFIG)/obj/ejsByteArray.o" "$(CONFIG)/obj/ejsCache.o" "$(CONFIG)/obj/ejsCmd.o" "$(CONFIG)/obj/ejsConfig.o" "$(CONFIG)/obj/ejsDate.o" "$(CONFIG)/obj/ejsDebug.o" "$(CONFIG)/obj/ejsError.o" "$(CONFIG)/obj/ejsFile.o" "$(CONFIG)/obj/ejsFileSystem.o" "$(CONFIG)/obj/ejsFrame.o" "$(CONFIG)/obj/ejsFunction.o" "$(CONFIG)/obj/ejsGC.o" "$(CONFIG)/obj/ejsGlobal.o" "$(CONFIG)/obj/ejsHttp.o" "$(CONFIG)/obj/ejsIterator.o" "$(CONFIG)/obj/ejsJSON.o" "$(CONFIG)/obj/ejsLocalCache.o" "$(CONFIG)/obj/ejsMath.o" "$(CONFIG)/obj/ejsMemory.o" "$(CONFIG)/obj/ejsMprLog.o" "$(CONFIG)/obj/ejsNamespace.o" "$(CONFIG)/obj/ejsNull.o" "$(CONFIG)/obj/ejsNumber.o" "$(CONFIG)/obj/ejsObject.o" "$(CONFIG)/obj/ejsPath.o" "$(CONFIG)/obj/ejsPot.o" "$(CONFIG)/obj/ejsRegExp.o" "$(CONFIG)/obj/ejsSocket.o" "$(CONFIG)/obj/ejsString.o" "$(CONFIG)/obj/ejsSystem.o" "$(CONFIG)/obj/ejsTimer.o" "$(CONFIG)/obj/ejsType.o" "$(CONFIG)/obj/ejsUri.o" "$(CONFIG)/obj/ejsVoid.o" "$(CONFIG)/obj/ejsWebSocket.o" "$(CONFIG)/obj/ejsWorker.o" "$(CONFIG)/obj/ejsXML.o" "$(CONFIG)/obj/ejsXMLList.o" "$(CONFIG)/obj/ejsXMLLoader.o" "$(CONFIG)/obj/ejsByteCode.o" "$(CONFIG)/obj/ejsException.o" "$(CONFIG)/obj/ejsHelper.o" "$(CONFIG)/obj/ejsInterp.o" "$(CONFIG)/obj/ejsLoader.o" "$(CONFIG)/obj/ejsModule.o" "$(CONFIG)/obj/ejsScope.o" "$(CONFIG)/obj/ejsService.o" $(LIBPATHS_82) $(LIBS_82) $(LIBS_82) $(LIBS) 

#
#   ejs.o
#
DEPS_83 += $(CONFIG)/inc/me.h
DEPS_83 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejs.o: \
    src/cmd/ejs.c $(DEPS_83)
	@echo '   [Compile] $(CONFIG)/obj/ejs.o'
	$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejs.c

#
#   ejs
#
DEPS_84 += $(CONFIG)/inc/mpr.h
DEPS_84 += $(CONFIG)/inc/me.h
DEPS_84 += $(CONFIG)/inc/osdep.h
DEPS_84 += $(CONFIG)/obj/mprLib.o
DEPS_84 += $(CONFIG)/bin/libmpr.so
DEPS_84 += $(CONFIG)/inc/pcre.h
DEPS_84 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_84 += $(CONFIG)/bin/libpcre.so
endif
DEPS_84 += $(CONFIG)/inc/http.h
DEPS_84 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_84 += $(CONFIG)/bin/libhttp.so
endif
DEPS_84 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_84 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_84 += $(CONFIG)/inc/ejs.slots.h
DEPS_84 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_84 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_84 += $(CONFIG)/inc/ejsByteCode.h
DEPS_84 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_84 += $(CONFIG)/inc/ejsCustomize.h
DEPS_84 += $(CONFIG)/inc/ejs.h
DEPS_84 += $(CONFIG)/inc/ejsCompiler.h
DEPS_84 += $(CONFIG)/obj/ecAst.o
DEPS_84 += $(CONFIG)/obj/ecCodeGen.o
DEPS_84 += $(CONFIG)/obj/ecCompiler.o
DEPS_84 += $(CONFIG)/obj/ecLex.o
DEPS_84 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_84 += $(CONFIG)/obj/ecParser.o
DEPS_84 += $(CONFIG)/obj/ecState.o
DEPS_84 += $(CONFIG)/obj/dtoa.o
DEPS_84 += $(CONFIG)/obj/ejsApp.o
DEPS_84 += $(CONFIG)/obj/ejsArray.o
DEPS_84 += $(CONFIG)/obj/ejsBlock.o
DEPS_84 += $(CONFIG)/obj/ejsBoolean.o
DEPS_84 += $(CONFIG)/obj/ejsByteArray.o
DEPS_84 += $(CONFIG)/obj/ejsCache.o
DEPS_84 += $(CONFIG)/obj/ejsCmd.o
DEPS_84 += $(CONFIG)/obj/ejsConfig.o
DEPS_84 += $(CONFIG)/obj/ejsDate.o
DEPS_84 += $(CONFIG)/obj/ejsDebug.o
DEPS_84 += $(CONFIG)/obj/ejsError.o
DEPS_84 += $(CONFIG)/obj/ejsFile.o
DEPS_84 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_84 += $(CONFIG)/obj/ejsFrame.o
DEPS_84 += $(CONFIG)/obj/ejsFunction.o
DEPS_84 += $(CONFIG)/obj/ejsGC.o
DEPS_84 += $(CONFIG)/obj/ejsGlobal.o
DEPS_84 += $(CONFIG)/obj/ejsHttp.o
DEPS_84 += $(CONFIG)/obj/ejsIterator.o
DEPS_84 += $(CONFIG)/obj/ejsJSON.o
DEPS_84 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_84 += $(CONFIG)/obj/ejsMath.o
DEPS_84 += $(CONFIG)/obj/ejsMemory.o
DEPS_84 += $(CONFIG)/obj/ejsMprLog.o
DEPS_84 += $(CONFIG)/obj/ejsNamespace.o
DEPS_84 += $(CONFIG)/obj/ejsNull.o
DEPS_84 += $(CONFIG)/obj/ejsNumber.o
DEPS_84 += $(CONFIG)/obj/ejsObject.o
DEPS_84 += $(CONFIG)/obj/ejsPath.o
DEPS_84 += $(CONFIG)/obj/ejsPot.o
DEPS_84 += $(CONFIG)/obj/ejsRegExp.o
DEPS_84 += $(CONFIG)/obj/ejsSocket.o
DEPS_84 += $(CONFIG)/obj/ejsString.o
DEPS_84 += $(CONFIG)/obj/ejsSystem.o
DEPS_84 += $(CONFIG)/obj/ejsTimer.o
DEPS_84 += $(CONFIG)/obj/ejsType.o
DEPS_84 += $(CONFIG)/obj/ejsUri.o
DEPS_84 += $(CONFIG)/obj/ejsVoid.o
DEPS_84 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_84 += $(CONFIG)/obj/ejsWorker.o
DEPS_84 += $(CONFIG)/obj/ejsXML.o
DEPS_84 += $(CONFIG)/obj/ejsXMLList.o
DEPS_84 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_84 += $(CONFIG)/obj/ejsByteCode.o
DEPS_84 += $(CONFIG)/obj/ejsException.o
DEPS_84 += $(CONFIG)/obj/ejsHelper.o
DEPS_84 += $(CONFIG)/obj/ejsInterp.o
DEPS_84 += $(CONFIG)/obj/ejsLoader.o
DEPS_84 += $(CONFIG)/obj/ejsModule.o
DEPS_84 += $(CONFIG)/obj/ejsScope.o
DEPS_84 += $(CONFIG)/obj/ejsService.o
DEPS_84 += $(CONFIG)/bin/libejs.so
DEPS_84 += $(CONFIG)/obj/ejs.o

LIBS_84 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_84 += -lhttp
endif
LIBS_84 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_84 += -lpcre
endif

$(CONFIG)/bin/ejs: $(DEPS_84)
	@echo '      [Link] $(CONFIG)/bin/ejs'
	$(CC) -o $(CONFIG)/bin/ejs $(LIBPATHS) "$(CONFIG)/obj/ejs.o" $(LIBPATHS_84) $(LIBS_84) $(LIBS_84) $(LIBS) $(LIBS) 

#
#   ejsc.o
#
DEPS_85 += $(CONFIG)/inc/me.h
DEPS_85 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejsc.o: \
    src/cmd/ejsc.c $(DEPS_85)
	@echo '   [Compile] $(CONFIG)/obj/ejsc.o'
	$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsc.c

#
#   ejsc
#
DEPS_86 += $(CONFIG)/inc/mpr.h
DEPS_86 += $(CONFIG)/inc/me.h
DEPS_86 += $(CONFIG)/inc/osdep.h
DEPS_86 += $(CONFIG)/obj/mprLib.o
DEPS_86 += $(CONFIG)/bin/libmpr.so
DEPS_86 += $(CONFIG)/inc/pcre.h
DEPS_86 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_86 += $(CONFIG)/bin/libpcre.so
endif
DEPS_86 += $(CONFIG)/inc/http.h
DEPS_86 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_86 += $(CONFIG)/bin/libhttp.so
endif
DEPS_86 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_86 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_86 += $(CONFIG)/inc/ejs.slots.h
DEPS_86 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_86 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_86 += $(CONFIG)/inc/ejsByteCode.h
DEPS_86 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_86 += $(CONFIG)/inc/ejsCustomize.h
DEPS_86 += $(CONFIG)/inc/ejs.h
DEPS_86 += $(CONFIG)/inc/ejsCompiler.h
DEPS_86 += $(CONFIG)/obj/ecAst.o
DEPS_86 += $(CONFIG)/obj/ecCodeGen.o
DEPS_86 += $(CONFIG)/obj/ecCompiler.o
DEPS_86 += $(CONFIG)/obj/ecLex.o
DEPS_86 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_86 += $(CONFIG)/obj/ecParser.o
DEPS_86 += $(CONFIG)/obj/ecState.o
DEPS_86 += $(CONFIG)/obj/dtoa.o
DEPS_86 += $(CONFIG)/obj/ejsApp.o
DEPS_86 += $(CONFIG)/obj/ejsArray.o
DEPS_86 += $(CONFIG)/obj/ejsBlock.o
DEPS_86 += $(CONFIG)/obj/ejsBoolean.o
DEPS_86 += $(CONFIG)/obj/ejsByteArray.o
DEPS_86 += $(CONFIG)/obj/ejsCache.o
DEPS_86 += $(CONFIG)/obj/ejsCmd.o
DEPS_86 += $(CONFIG)/obj/ejsConfig.o
DEPS_86 += $(CONFIG)/obj/ejsDate.o
DEPS_86 += $(CONFIG)/obj/ejsDebug.o
DEPS_86 += $(CONFIG)/obj/ejsError.o
DEPS_86 += $(CONFIG)/obj/ejsFile.o
DEPS_86 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_86 += $(CONFIG)/obj/ejsFrame.o
DEPS_86 += $(CONFIG)/obj/ejsFunction.o
DEPS_86 += $(CONFIG)/obj/ejsGC.o
DEPS_86 += $(CONFIG)/obj/ejsGlobal.o
DEPS_86 += $(CONFIG)/obj/ejsHttp.o
DEPS_86 += $(CONFIG)/obj/ejsIterator.o
DEPS_86 += $(CONFIG)/obj/ejsJSON.o
DEPS_86 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_86 += $(CONFIG)/obj/ejsMath.o
DEPS_86 += $(CONFIG)/obj/ejsMemory.o
DEPS_86 += $(CONFIG)/obj/ejsMprLog.o
DEPS_86 += $(CONFIG)/obj/ejsNamespace.o
DEPS_86 += $(CONFIG)/obj/ejsNull.o
DEPS_86 += $(CONFIG)/obj/ejsNumber.o
DEPS_86 += $(CONFIG)/obj/ejsObject.o
DEPS_86 += $(CONFIG)/obj/ejsPath.o
DEPS_86 += $(CONFIG)/obj/ejsPot.o
DEPS_86 += $(CONFIG)/obj/ejsRegExp.o
DEPS_86 += $(CONFIG)/obj/ejsSocket.o
DEPS_86 += $(CONFIG)/obj/ejsString.o
DEPS_86 += $(CONFIG)/obj/ejsSystem.o
DEPS_86 += $(CONFIG)/obj/ejsTimer.o
DEPS_86 += $(CONFIG)/obj/ejsType.o
DEPS_86 += $(CONFIG)/obj/ejsUri.o
DEPS_86 += $(CONFIG)/obj/ejsVoid.o
DEPS_86 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_86 += $(CONFIG)/obj/ejsWorker.o
DEPS_86 += $(CONFIG)/obj/ejsXML.o
DEPS_86 += $(CONFIG)/obj/ejsXMLList.o
DEPS_86 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_86 += $(CONFIG)/obj/ejsByteCode.o
DEPS_86 += $(CONFIG)/obj/ejsException.o
DEPS_86 += $(CONFIG)/obj/ejsHelper.o
DEPS_86 += $(CONFIG)/obj/ejsInterp.o
DEPS_86 += $(CONFIG)/obj/ejsLoader.o
DEPS_86 += $(CONFIG)/obj/ejsModule.o
DEPS_86 += $(CONFIG)/obj/ejsScope.o
DEPS_86 += $(CONFIG)/obj/ejsService.o
DEPS_86 += $(CONFIG)/bin/libejs.so
DEPS_86 += $(CONFIG)/obj/ejsc.o

LIBS_86 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_86 += -lhttp
endif
LIBS_86 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_86 += -lpcre
endif

$(CONFIG)/bin/ejsc: $(DEPS_86)
	@echo '      [Link] $(CONFIG)/bin/ejsc'
	$(CC) -o $(CONFIG)/bin/ejsc $(LIBPATHS) "$(CONFIG)/obj/ejsc.o" $(LIBPATHS_86) $(LIBS_86) $(LIBS_86) $(LIBS) $(LIBS) 

#
#   ejsmod.h
#
src/cmd/ejsmod.h: $(DEPS_87)
	@echo '      [Copy] src/cmd/ejsmod.h'

#
#   ejsmod.o
#
DEPS_88 += $(CONFIG)/inc/me.h
DEPS_88 += src/cmd/ejsmod.h
DEPS_88 += $(CONFIG)/inc/mpr.h
DEPS_88 += $(CONFIG)/inc/http.h
DEPS_88 += $(CONFIG)/inc/ejsByteCode.h
DEPS_88 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_88 += $(CONFIG)/inc/ejs.slots.h
DEPS_88 += $(CONFIG)/inc/ejsCustomize.h
DEPS_88 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsmod.o: \
    src/cmd/ejsmod.c $(DEPS_88)
	@echo '   [Compile] $(CONFIG)/obj/ejsmod.o'
	$(CC) -c -o $(CONFIG)/obj/ejsmod.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/ejsmod.c

#
#   doc.o
#
DEPS_89 += $(CONFIG)/inc/me.h
DEPS_89 += src/cmd/ejsmod.h

$(CONFIG)/obj/doc.o: \
    src/cmd/doc.c $(DEPS_89)
	@echo '   [Compile] $(CONFIG)/obj/doc.o'
	$(CC) -c -o $(CONFIG)/obj/doc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/doc.c

#
#   docFiles.o
#
DEPS_90 += $(CONFIG)/inc/me.h
DEPS_90 += src/cmd/ejsmod.h

$(CONFIG)/obj/docFiles.o: \
    src/cmd/docFiles.c $(DEPS_90)
	@echo '   [Compile] $(CONFIG)/obj/docFiles.o'
	$(CC) -c -o $(CONFIG)/obj/docFiles.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/docFiles.c

#
#   listing.o
#
DEPS_91 += $(CONFIG)/inc/me.h
DEPS_91 += src/cmd/ejsmod.h
DEPS_91 += $(CONFIG)/inc/ejsByteCodeTable.h

$(CONFIG)/obj/listing.o: \
    src/cmd/listing.c $(DEPS_91)
	@echo '   [Compile] $(CONFIG)/obj/listing.o'
	$(CC) -c -o $(CONFIG)/obj/listing.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/listing.c

#
#   slotGen.o
#
DEPS_92 += $(CONFIG)/inc/me.h
DEPS_92 += src/cmd/ejsmod.h

$(CONFIG)/obj/slotGen.o: \
    src/cmd/slotGen.c $(DEPS_92)
	@echo '   [Compile] $(CONFIG)/obj/slotGen.o'
	$(CC) -c -o $(CONFIG)/obj/slotGen.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/slotGen.c

#
#   ejsmod
#
DEPS_93 += $(CONFIG)/inc/mpr.h
DEPS_93 += $(CONFIG)/inc/me.h
DEPS_93 += $(CONFIG)/inc/osdep.h
DEPS_93 += $(CONFIG)/obj/mprLib.o
DEPS_93 += $(CONFIG)/bin/libmpr.so
DEPS_93 += $(CONFIG)/inc/pcre.h
DEPS_93 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_93 += $(CONFIG)/bin/libpcre.so
endif
DEPS_93 += $(CONFIG)/inc/http.h
DEPS_93 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_93 += $(CONFIG)/bin/libhttp.so
endif
DEPS_93 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_93 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_93 += $(CONFIG)/inc/ejs.slots.h
DEPS_93 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_93 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_93 += $(CONFIG)/inc/ejsByteCode.h
DEPS_93 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_93 += $(CONFIG)/inc/ejsCustomize.h
DEPS_93 += $(CONFIG)/inc/ejs.h
DEPS_93 += $(CONFIG)/inc/ejsCompiler.h
DEPS_93 += $(CONFIG)/obj/ecAst.o
DEPS_93 += $(CONFIG)/obj/ecCodeGen.o
DEPS_93 += $(CONFIG)/obj/ecCompiler.o
DEPS_93 += $(CONFIG)/obj/ecLex.o
DEPS_93 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_93 += $(CONFIG)/obj/ecParser.o
DEPS_93 += $(CONFIG)/obj/ecState.o
DEPS_93 += $(CONFIG)/obj/dtoa.o
DEPS_93 += $(CONFIG)/obj/ejsApp.o
DEPS_93 += $(CONFIG)/obj/ejsArray.o
DEPS_93 += $(CONFIG)/obj/ejsBlock.o
DEPS_93 += $(CONFIG)/obj/ejsBoolean.o
DEPS_93 += $(CONFIG)/obj/ejsByteArray.o
DEPS_93 += $(CONFIG)/obj/ejsCache.o
DEPS_93 += $(CONFIG)/obj/ejsCmd.o
DEPS_93 += $(CONFIG)/obj/ejsConfig.o
DEPS_93 += $(CONFIG)/obj/ejsDate.o
DEPS_93 += $(CONFIG)/obj/ejsDebug.o
DEPS_93 += $(CONFIG)/obj/ejsError.o
DEPS_93 += $(CONFIG)/obj/ejsFile.o
DEPS_93 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_93 += $(CONFIG)/obj/ejsFrame.o
DEPS_93 += $(CONFIG)/obj/ejsFunction.o
DEPS_93 += $(CONFIG)/obj/ejsGC.o
DEPS_93 += $(CONFIG)/obj/ejsGlobal.o
DEPS_93 += $(CONFIG)/obj/ejsHttp.o
DEPS_93 += $(CONFIG)/obj/ejsIterator.o
DEPS_93 += $(CONFIG)/obj/ejsJSON.o
DEPS_93 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_93 += $(CONFIG)/obj/ejsMath.o
DEPS_93 += $(CONFIG)/obj/ejsMemory.o
DEPS_93 += $(CONFIG)/obj/ejsMprLog.o
DEPS_93 += $(CONFIG)/obj/ejsNamespace.o
DEPS_93 += $(CONFIG)/obj/ejsNull.o
DEPS_93 += $(CONFIG)/obj/ejsNumber.o
DEPS_93 += $(CONFIG)/obj/ejsObject.o
DEPS_93 += $(CONFIG)/obj/ejsPath.o
DEPS_93 += $(CONFIG)/obj/ejsPot.o
DEPS_93 += $(CONFIG)/obj/ejsRegExp.o
DEPS_93 += $(CONFIG)/obj/ejsSocket.o
DEPS_93 += $(CONFIG)/obj/ejsString.o
DEPS_93 += $(CONFIG)/obj/ejsSystem.o
DEPS_93 += $(CONFIG)/obj/ejsTimer.o
DEPS_93 += $(CONFIG)/obj/ejsType.o
DEPS_93 += $(CONFIG)/obj/ejsUri.o
DEPS_93 += $(CONFIG)/obj/ejsVoid.o
DEPS_93 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_93 += $(CONFIG)/obj/ejsWorker.o
DEPS_93 += $(CONFIG)/obj/ejsXML.o
DEPS_93 += $(CONFIG)/obj/ejsXMLList.o
DEPS_93 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_93 += $(CONFIG)/obj/ejsByteCode.o
DEPS_93 += $(CONFIG)/obj/ejsException.o
DEPS_93 += $(CONFIG)/obj/ejsHelper.o
DEPS_93 += $(CONFIG)/obj/ejsInterp.o
DEPS_93 += $(CONFIG)/obj/ejsLoader.o
DEPS_93 += $(CONFIG)/obj/ejsModule.o
DEPS_93 += $(CONFIG)/obj/ejsScope.o
DEPS_93 += $(CONFIG)/obj/ejsService.o
DEPS_93 += $(CONFIG)/bin/libejs.so
DEPS_93 += src/cmd/ejsmod.h
DEPS_93 += $(CONFIG)/obj/ejsmod.o
DEPS_93 += $(CONFIG)/obj/doc.o
DEPS_93 += $(CONFIG)/obj/docFiles.o
DEPS_93 += $(CONFIG)/obj/listing.o
DEPS_93 += $(CONFIG)/obj/slotGen.o

LIBS_93 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_93 += -lhttp
endif
LIBS_93 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_93 += -lpcre
endif

$(CONFIG)/bin/ejsmod: $(DEPS_93)
	@echo '      [Link] $(CONFIG)/bin/ejsmod'
	$(CC) -o $(CONFIG)/bin/ejsmod $(LIBPATHS) "$(CONFIG)/obj/ejsmod.o" "$(CONFIG)/obj/doc.o" "$(CONFIG)/obj/docFiles.o" "$(CONFIG)/obj/listing.o" "$(CONFIG)/obj/slotGen.o" $(LIBPATHS_93) $(LIBS_93) $(LIBS_93) $(LIBS) $(LIBS) 

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
DEPS_94 += $(CONFIG)/inc/mpr.h
DEPS_94 += $(CONFIG)/inc/me.h
DEPS_94 += $(CONFIG)/inc/osdep.h
DEPS_94 += $(CONFIG)/obj/mprLib.o
DEPS_94 += $(CONFIG)/bin/libmpr.so
DEPS_94 += $(CONFIG)/inc/pcre.h
DEPS_94 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_94 += $(CONFIG)/bin/libpcre.so
endif
DEPS_94 += $(CONFIG)/inc/http.h
DEPS_94 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_94 += $(CONFIG)/bin/libhttp.so
endif
DEPS_94 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_94 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_94 += $(CONFIG)/inc/ejs.slots.h
DEPS_94 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_94 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_94 += $(CONFIG)/inc/ejsByteCode.h
DEPS_94 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_94 += $(CONFIG)/inc/ejsCustomize.h
DEPS_94 += $(CONFIG)/inc/ejs.h
DEPS_94 += $(CONFIG)/inc/ejsCompiler.h
DEPS_94 += $(CONFIG)/obj/ecAst.o
DEPS_94 += $(CONFIG)/obj/ecCodeGen.o
DEPS_94 += $(CONFIG)/obj/ecCompiler.o
DEPS_94 += $(CONFIG)/obj/ecLex.o
DEPS_94 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_94 += $(CONFIG)/obj/ecParser.o
DEPS_94 += $(CONFIG)/obj/ecState.o
DEPS_94 += $(CONFIG)/obj/dtoa.o
DEPS_94 += $(CONFIG)/obj/ejsApp.o
DEPS_94 += $(CONFIG)/obj/ejsArray.o
DEPS_94 += $(CONFIG)/obj/ejsBlock.o
DEPS_94 += $(CONFIG)/obj/ejsBoolean.o
DEPS_94 += $(CONFIG)/obj/ejsByteArray.o
DEPS_94 += $(CONFIG)/obj/ejsCache.o
DEPS_94 += $(CONFIG)/obj/ejsCmd.o
DEPS_94 += $(CONFIG)/obj/ejsConfig.o
DEPS_94 += $(CONFIG)/obj/ejsDate.o
DEPS_94 += $(CONFIG)/obj/ejsDebug.o
DEPS_94 += $(CONFIG)/obj/ejsError.o
DEPS_94 += $(CONFIG)/obj/ejsFile.o
DEPS_94 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_94 += $(CONFIG)/obj/ejsFrame.o
DEPS_94 += $(CONFIG)/obj/ejsFunction.o
DEPS_94 += $(CONFIG)/obj/ejsGC.o
DEPS_94 += $(CONFIG)/obj/ejsGlobal.o
DEPS_94 += $(CONFIG)/obj/ejsHttp.o
DEPS_94 += $(CONFIG)/obj/ejsIterator.o
DEPS_94 += $(CONFIG)/obj/ejsJSON.o
DEPS_94 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_94 += $(CONFIG)/obj/ejsMath.o
DEPS_94 += $(CONFIG)/obj/ejsMemory.o
DEPS_94 += $(CONFIG)/obj/ejsMprLog.o
DEPS_94 += $(CONFIG)/obj/ejsNamespace.o
DEPS_94 += $(CONFIG)/obj/ejsNull.o
DEPS_94 += $(CONFIG)/obj/ejsNumber.o
DEPS_94 += $(CONFIG)/obj/ejsObject.o
DEPS_94 += $(CONFIG)/obj/ejsPath.o
DEPS_94 += $(CONFIG)/obj/ejsPot.o
DEPS_94 += $(CONFIG)/obj/ejsRegExp.o
DEPS_94 += $(CONFIG)/obj/ejsSocket.o
DEPS_94 += $(CONFIG)/obj/ejsString.o
DEPS_94 += $(CONFIG)/obj/ejsSystem.o
DEPS_94 += $(CONFIG)/obj/ejsTimer.o
DEPS_94 += $(CONFIG)/obj/ejsType.o
DEPS_94 += $(CONFIG)/obj/ejsUri.o
DEPS_94 += $(CONFIG)/obj/ejsVoid.o
DEPS_94 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_94 += $(CONFIG)/obj/ejsWorker.o
DEPS_94 += $(CONFIG)/obj/ejsXML.o
DEPS_94 += $(CONFIG)/obj/ejsXMLList.o
DEPS_94 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_94 += $(CONFIG)/obj/ejsByteCode.o
DEPS_94 += $(CONFIG)/obj/ejsException.o
DEPS_94 += $(CONFIG)/obj/ejsHelper.o
DEPS_94 += $(CONFIG)/obj/ejsInterp.o
DEPS_94 += $(CONFIG)/obj/ejsLoader.o
DEPS_94 += $(CONFIG)/obj/ejsModule.o
DEPS_94 += $(CONFIG)/obj/ejsScope.o
DEPS_94 += $(CONFIG)/obj/ejsService.o
DEPS_94 += $(CONFIG)/bin/libejs.so
DEPS_94 += $(CONFIG)/obj/ejsc.o
DEPS_94 += $(CONFIG)/bin/ejsc
DEPS_94 += src/cmd/ejsmod.h
DEPS_94 += $(CONFIG)/obj/ejsmod.o
DEPS_94 += $(CONFIG)/obj/doc.o
DEPS_94 += $(CONFIG)/obj/docFiles.o
DEPS_94 += $(CONFIG)/obj/listing.o
DEPS_94 += $(CONFIG)/obj/slotGen.o
DEPS_94 += $(CONFIG)/bin/ejsmod

$(CONFIG)/bin/ejs.mod: $(DEPS_94)
	( \
	cd src/core; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.mod  --optimize 9 --bind --require null *.es  ; \
	../../$(CONFIG)/bin/ejsmod --require null --cslots ../../$(CONFIG)/bin/ejs.mod ; \
	if ! diff ejs.slots.h ../../$(CONFIG)/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ../../$(CONFIG)/inc; fi ; \
	rm -f ejs.slots.h ; \
	)



#
#   ejs.mail.mod
#
DEPS_95 += src/ejs.mail/Mail.es
DEPS_95 += $(CONFIG)/inc/mpr.h
DEPS_95 += $(CONFIG)/inc/me.h
DEPS_95 += $(CONFIG)/inc/osdep.h
DEPS_95 += $(CONFIG)/obj/mprLib.o
DEPS_95 += $(CONFIG)/bin/libmpr.so
DEPS_95 += $(CONFIG)/inc/pcre.h
DEPS_95 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_95 += $(CONFIG)/bin/libpcre.so
endif
DEPS_95 += $(CONFIG)/inc/http.h
DEPS_95 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_95 += $(CONFIG)/bin/libhttp.so
endif
DEPS_95 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_95 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_95 += $(CONFIG)/inc/ejs.slots.h
DEPS_95 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_95 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_95 += $(CONFIG)/inc/ejsByteCode.h
DEPS_95 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_95 += $(CONFIG)/inc/ejsCustomize.h
DEPS_95 += $(CONFIG)/inc/ejs.h
DEPS_95 += $(CONFIG)/inc/ejsCompiler.h
DEPS_95 += $(CONFIG)/obj/ecAst.o
DEPS_95 += $(CONFIG)/obj/ecCodeGen.o
DEPS_95 += $(CONFIG)/obj/ecCompiler.o
DEPS_95 += $(CONFIG)/obj/ecLex.o
DEPS_95 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_95 += $(CONFIG)/obj/ecParser.o
DEPS_95 += $(CONFIG)/obj/ecState.o
DEPS_95 += $(CONFIG)/obj/dtoa.o
DEPS_95 += $(CONFIG)/obj/ejsApp.o
DEPS_95 += $(CONFIG)/obj/ejsArray.o
DEPS_95 += $(CONFIG)/obj/ejsBlock.o
DEPS_95 += $(CONFIG)/obj/ejsBoolean.o
DEPS_95 += $(CONFIG)/obj/ejsByteArray.o
DEPS_95 += $(CONFIG)/obj/ejsCache.o
DEPS_95 += $(CONFIG)/obj/ejsCmd.o
DEPS_95 += $(CONFIG)/obj/ejsConfig.o
DEPS_95 += $(CONFIG)/obj/ejsDate.o
DEPS_95 += $(CONFIG)/obj/ejsDebug.o
DEPS_95 += $(CONFIG)/obj/ejsError.o
DEPS_95 += $(CONFIG)/obj/ejsFile.o
DEPS_95 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_95 += $(CONFIG)/obj/ejsFrame.o
DEPS_95 += $(CONFIG)/obj/ejsFunction.o
DEPS_95 += $(CONFIG)/obj/ejsGC.o
DEPS_95 += $(CONFIG)/obj/ejsGlobal.o
DEPS_95 += $(CONFIG)/obj/ejsHttp.o
DEPS_95 += $(CONFIG)/obj/ejsIterator.o
DEPS_95 += $(CONFIG)/obj/ejsJSON.o
DEPS_95 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_95 += $(CONFIG)/obj/ejsMath.o
DEPS_95 += $(CONFIG)/obj/ejsMemory.o
DEPS_95 += $(CONFIG)/obj/ejsMprLog.o
DEPS_95 += $(CONFIG)/obj/ejsNamespace.o
DEPS_95 += $(CONFIG)/obj/ejsNull.o
DEPS_95 += $(CONFIG)/obj/ejsNumber.o
DEPS_95 += $(CONFIG)/obj/ejsObject.o
DEPS_95 += $(CONFIG)/obj/ejsPath.o
DEPS_95 += $(CONFIG)/obj/ejsPot.o
DEPS_95 += $(CONFIG)/obj/ejsRegExp.o
DEPS_95 += $(CONFIG)/obj/ejsSocket.o
DEPS_95 += $(CONFIG)/obj/ejsString.o
DEPS_95 += $(CONFIG)/obj/ejsSystem.o
DEPS_95 += $(CONFIG)/obj/ejsTimer.o
DEPS_95 += $(CONFIG)/obj/ejsType.o
DEPS_95 += $(CONFIG)/obj/ejsUri.o
DEPS_95 += $(CONFIG)/obj/ejsVoid.o
DEPS_95 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_95 += $(CONFIG)/obj/ejsWorker.o
DEPS_95 += $(CONFIG)/obj/ejsXML.o
DEPS_95 += $(CONFIG)/obj/ejsXMLList.o
DEPS_95 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_95 += $(CONFIG)/obj/ejsByteCode.o
DEPS_95 += $(CONFIG)/obj/ejsException.o
DEPS_95 += $(CONFIG)/obj/ejsHelper.o
DEPS_95 += $(CONFIG)/obj/ejsInterp.o
DEPS_95 += $(CONFIG)/obj/ejsLoader.o
DEPS_95 += $(CONFIG)/obj/ejsModule.o
DEPS_95 += $(CONFIG)/obj/ejsScope.o
DEPS_95 += $(CONFIG)/obj/ejsService.o
DEPS_95 += $(CONFIG)/bin/libejs.so
DEPS_95 += $(CONFIG)/obj/ejsc.o
DEPS_95 += $(CONFIG)/bin/ejsc
DEPS_95 += src/cmd/ejsmod.h
DEPS_95 += $(CONFIG)/obj/ejsmod.o
DEPS_95 += $(CONFIG)/obj/doc.o
DEPS_95 += $(CONFIG)/obj/docFiles.o
DEPS_95 += $(CONFIG)/obj/listing.o
DEPS_95 += $(CONFIG)/obj/slotGen.o
DEPS_95 += $(CONFIG)/bin/ejsmod
DEPS_95 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.mail.mod: $(DEPS_95)
	( \
	cd src/ejs.mail; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.mail.mod  --optimize 9 *.es ; \
	)

#
#   ejs.web.mod
#
DEPS_96 += src/ejs.web/Cascade.es
DEPS_96 += src/ejs.web/CommonLog.es
DEPS_96 += src/ejs.web/ContentType.es
DEPS_96 += src/ejs.web/Controller.es
DEPS_96 += src/ejs.web/Dir.es
DEPS_96 += src/ejs.web/Google.es
DEPS_96 += src/ejs.web/Head.es
DEPS_96 += src/ejs.web/Html.es
DEPS_96 += src/ejs.web/HttpServer.es
DEPS_96 += src/ejs.web/MethodOverride.es
DEPS_96 += src/ejs.web/Middleware.es
DEPS_96 += src/ejs.web/Mvc.es
DEPS_96 += src/ejs.web/Request.es
DEPS_96 += src/ejs.web/Router.es
DEPS_96 += src/ejs.web/Script.es
DEPS_96 += src/ejs.web/Session.es
DEPS_96 += src/ejs.web/ShowExceptions.es
DEPS_96 += src/ejs.web/Static.es
DEPS_96 += src/ejs.web/Template.es
DEPS_96 += src/ejs.web/UploadFile.es
DEPS_96 += src/ejs.web/UrlMap.es
DEPS_96 += src/ejs.web/Utils.es
DEPS_96 += src/ejs.web/View.es
DEPS_96 += $(CONFIG)/inc/mpr.h
DEPS_96 += $(CONFIG)/inc/me.h
DEPS_96 += $(CONFIG)/inc/osdep.h
DEPS_96 += $(CONFIG)/obj/mprLib.o
DEPS_96 += $(CONFIG)/bin/libmpr.so
DEPS_96 += $(CONFIG)/inc/pcre.h
DEPS_96 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_96 += $(CONFIG)/bin/libpcre.so
endif
DEPS_96 += $(CONFIG)/inc/http.h
DEPS_96 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_96 += $(CONFIG)/bin/libhttp.so
endif
DEPS_96 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_96 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_96 += $(CONFIG)/inc/ejs.slots.h
DEPS_96 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_96 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_96 += $(CONFIG)/inc/ejsByteCode.h
DEPS_96 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_96 += $(CONFIG)/inc/ejsCustomize.h
DEPS_96 += $(CONFIG)/inc/ejs.h
DEPS_96 += $(CONFIG)/inc/ejsCompiler.h
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
DEPS_96 += $(CONFIG)/bin/libejs.so
DEPS_96 += $(CONFIG)/obj/ejsc.o
DEPS_96 += $(CONFIG)/bin/ejsc
DEPS_96 += src/cmd/ejsmod.h
DEPS_96 += $(CONFIG)/obj/ejsmod.o
DEPS_96 += $(CONFIG)/obj/doc.o
DEPS_96 += $(CONFIG)/obj/docFiles.o
DEPS_96 += $(CONFIG)/obj/listing.o
DEPS_96 += $(CONFIG)/obj/slotGen.o
DEPS_96 += $(CONFIG)/bin/ejsmod
DEPS_96 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.web.mod: $(DEPS_96)
	( \
	cd src/ejs.web; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.web.mod  --optimize 9 *.es ; \
	../../$(CONFIG)/bin/ejsmod --cslots ../../$(CONFIG)/bin/ejs.web.mod ; \
	if ! diff ejs.web.slots.h ../../$(CONFIG)/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../$(CONFIG)/inc; fi ; \
	rm -f ejs.web.slots.h ; \
	)






#
#   ejsrun.o
#
DEPS_97 += $(CONFIG)/inc/me.h
DEPS_97 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejsrun.o: \
    src/cmd/ejsrun.c $(DEPS_97)
	@echo '   [Compile] $(CONFIG)/obj/ejsrun.o'
	$(CC) -c -o $(CONFIG)/obj/ejsrun.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsrun.c

#
#   ejsrun
#
DEPS_98 += $(CONFIG)/inc/mpr.h
DEPS_98 += $(CONFIG)/inc/me.h
DEPS_98 += $(CONFIG)/inc/osdep.h
DEPS_98 += $(CONFIG)/obj/mprLib.o
DEPS_98 += $(CONFIG)/bin/libmpr.so
DEPS_98 += $(CONFIG)/inc/pcre.h
DEPS_98 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_98 += $(CONFIG)/bin/libpcre.so
endif
DEPS_98 += $(CONFIG)/inc/http.h
DEPS_98 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_98 += $(CONFIG)/bin/libhttp.so
endif
DEPS_98 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_98 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_98 += $(CONFIG)/inc/ejs.slots.h
DEPS_98 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_98 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_98 += $(CONFIG)/inc/ejsByteCode.h
DEPS_98 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_98 += $(CONFIG)/inc/ejsCustomize.h
DEPS_98 += $(CONFIG)/inc/ejs.h
DEPS_98 += $(CONFIG)/inc/ejsCompiler.h
DEPS_98 += $(CONFIG)/obj/ecAst.o
DEPS_98 += $(CONFIG)/obj/ecCodeGen.o
DEPS_98 += $(CONFIG)/obj/ecCompiler.o
DEPS_98 += $(CONFIG)/obj/ecLex.o
DEPS_98 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_98 += $(CONFIG)/obj/ecParser.o
DEPS_98 += $(CONFIG)/obj/ecState.o
DEPS_98 += $(CONFIG)/obj/dtoa.o
DEPS_98 += $(CONFIG)/obj/ejsApp.o
DEPS_98 += $(CONFIG)/obj/ejsArray.o
DEPS_98 += $(CONFIG)/obj/ejsBlock.o
DEPS_98 += $(CONFIG)/obj/ejsBoolean.o
DEPS_98 += $(CONFIG)/obj/ejsByteArray.o
DEPS_98 += $(CONFIG)/obj/ejsCache.o
DEPS_98 += $(CONFIG)/obj/ejsCmd.o
DEPS_98 += $(CONFIG)/obj/ejsConfig.o
DEPS_98 += $(CONFIG)/obj/ejsDate.o
DEPS_98 += $(CONFIG)/obj/ejsDebug.o
DEPS_98 += $(CONFIG)/obj/ejsError.o
DEPS_98 += $(CONFIG)/obj/ejsFile.o
DEPS_98 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_98 += $(CONFIG)/obj/ejsFrame.o
DEPS_98 += $(CONFIG)/obj/ejsFunction.o
DEPS_98 += $(CONFIG)/obj/ejsGC.o
DEPS_98 += $(CONFIG)/obj/ejsGlobal.o
DEPS_98 += $(CONFIG)/obj/ejsHttp.o
DEPS_98 += $(CONFIG)/obj/ejsIterator.o
DEPS_98 += $(CONFIG)/obj/ejsJSON.o
DEPS_98 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_98 += $(CONFIG)/obj/ejsMath.o
DEPS_98 += $(CONFIG)/obj/ejsMemory.o
DEPS_98 += $(CONFIG)/obj/ejsMprLog.o
DEPS_98 += $(CONFIG)/obj/ejsNamespace.o
DEPS_98 += $(CONFIG)/obj/ejsNull.o
DEPS_98 += $(CONFIG)/obj/ejsNumber.o
DEPS_98 += $(CONFIG)/obj/ejsObject.o
DEPS_98 += $(CONFIG)/obj/ejsPath.o
DEPS_98 += $(CONFIG)/obj/ejsPot.o
DEPS_98 += $(CONFIG)/obj/ejsRegExp.o
DEPS_98 += $(CONFIG)/obj/ejsSocket.o
DEPS_98 += $(CONFIG)/obj/ejsString.o
DEPS_98 += $(CONFIG)/obj/ejsSystem.o
DEPS_98 += $(CONFIG)/obj/ejsTimer.o
DEPS_98 += $(CONFIG)/obj/ejsType.o
DEPS_98 += $(CONFIG)/obj/ejsUri.o
DEPS_98 += $(CONFIG)/obj/ejsVoid.o
DEPS_98 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_98 += $(CONFIG)/obj/ejsWorker.o
DEPS_98 += $(CONFIG)/obj/ejsXML.o
DEPS_98 += $(CONFIG)/obj/ejsXMLList.o
DEPS_98 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_98 += $(CONFIG)/obj/ejsByteCode.o
DEPS_98 += $(CONFIG)/obj/ejsException.o
DEPS_98 += $(CONFIG)/obj/ejsHelper.o
DEPS_98 += $(CONFIG)/obj/ejsInterp.o
DEPS_98 += $(CONFIG)/obj/ejsLoader.o
DEPS_98 += $(CONFIG)/obj/ejsModule.o
DEPS_98 += $(CONFIG)/obj/ejsScope.o
DEPS_98 += $(CONFIG)/obj/ejsService.o
DEPS_98 += $(CONFIG)/bin/libejs.so
DEPS_98 += $(CONFIG)/obj/ejsrun.o

LIBS_98 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_98 += -lhttp
endif
LIBS_98 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_98 += -lpcre
endif

$(CONFIG)/bin/ejsrun: $(DEPS_98)
	@echo '      [Link] $(CONFIG)/bin/ejsrun'
	$(CC) -o $(CONFIG)/bin/ejsrun $(LIBPATHS) "$(CONFIG)/obj/ejsrun.o" $(LIBPATHS_98) $(LIBS_98) $(LIBS_98) $(LIBS) $(LIBS) 


#
#   http.o
#
DEPS_99 += $(CONFIG)/inc/me.h
DEPS_99 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/http.o: \
    src/paks/http/http.c $(DEPS_99)
	@echo '   [Compile] $(CONFIG)/obj/http.o'
	$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/http.c

ifeq ($(ME_COM_HTTP),1)
#
#   httpcmd
#
DEPS_100 += $(CONFIG)/inc/mpr.h
DEPS_100 += $(CONFIG)/inc/me.h
DEPS_100 += $(CONFIG)/inc/osdep.h
DEPS_100 += $(CONFIG)/obj/mprLib.o
DEPS_100 += $(CONFIG)/bin/libmpr.so
DEPS_100 += $(CONFIG)/inc/pcre.h
DEPS_100 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_100 += $(CONFIG)/bin/libpcre.so
endif
DEPS_100 += $(CONFIG)/inc/http.h
DEPS_100 += $(CONFIG)/obj/httpLib.o
DEPS_100 += $(CONFIG)/bin/libhttp.so
DEPS_100 += $(CONFIG)/obj/http.o

LIBS_100 += -lhttp
LIBS_100 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_100 += -lpcre
endif

$(CONFIG)/bin/http: $(DEPS_100)
	@echo '      [Link] $(CONFIG)/bin/http'
	$(CC) -o $(CONFIG)/bin/http $(LIBPATHS) "$(CONFIG)/obj/http.o" $(LIBPATHS_100) $(LIBS_100) $(LIBS_100) $(LIBS) $(LIBS) 
endif

#
#   ejsWeb.h
#
$(CONFIG)/inc/ejsWeb.h: $(DEPS_101)
	@echo '      [Copy] $(CONFIG)/inc/ejsWeb.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejs.web/ejsWeb.h $(CONFIG)/inc/ejsWeb.h

#
#   ejsHttpServer.o
#
DEPS_102 += $(CONFIG)/inc/me.h
DEPS_102 += $(CONFIG)/inc/mpr.h
DEPS_102 += $(CONFIG)/inc/http.h
DEPS_102 += $(CONFIG)/inc/ejsByteCode.h
DEPS_102 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_102 += $(CONFIG)/inc/ejs.slots.h
DEPS_102 += $(CONFIG)/inc/ejsCustomize.h
DEPS_102 += $(CONFIG)/inc/ejs.h
DEPS_102 += $(CONFIG)/inc/ejsCompiler.h
DEPS_102 += $(CONFIG)/inc/ejsWeb.h
DEPS_102 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsHttpServer.o: \
    src/ejs.web/ejsHttpServer.c $(DEPS_102)
	@echo '   [Compile] $(CONFIG)/obj/ejsHttpServer.o'
	$(CC) -c -o $(CONFIG)/obj/ejsHttpServer.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.web/ejsHttpServer.c

#
#   ejsRequest.o
#
DEPS_103 += $(CONFIG)/inc/me.h
DEPS_103 += $(CONFIG)/inc/mpr.h
DEPS_103 += $(CONFIG)/inc/http.h
DEPS_103 += $(CONFIG)/inc/ejsByteCode.h
DEPS_103 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_103 += $(CONFIG)/inc/ejs.slots.h
DEPS_103 += $(CONFIG)/inc/ejsCustomize.h
DEPS_103 += $(CONFIG)/inc/ejs.h
DEPS_103 += $(CONFIG)/inc/ejsCompiler.h
DEPS_103 += $(CONFIG)/inc/ejsWeb.h
DEPS_103 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsRequest.o: \
    src/ejs.web/ejsRequest.c $(DEPS_103)
	@echo '   [Compile] $(CONFIG)/obj/ejsRequest.o'
	$(CC) -c -o $(CONFIG)/obj/ejsRequest.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.web/ejsRequest.c

#
#   ejsSession.o
#
DEPS_104 += $(CONFIG)/inc/me.h
DEPS_104 += $(CONFIG)/inc/mpr.h
DEPS_104 += $(CONFIG)/inc/http.h
DEPS_104 += $(CONFIG)/inc/ejsByteCode.h
DEPS_104 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_104 += $(CONFIG)/inc/ejs.slots.h
DEPS_104 += $(CONFIG)/inc/ejsCustomize.h
DEPS_104 += $(CONFIG)/inc/ejs.h
DEPS_104 += $(CONFIG)/inc/ejsWeb.h

$(CONFIG)/obj/ejsSession.o: \
    src/ejs.web/ejsSession.c $(DEPS_104)
	@echo '   [Compile] $(CONFIG)/obj/ejsSession.o'
	$(CC) -c -o $(CONFIG)/obj/ejsSession.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.web/ejsSession.c

#
#   ejsWeb.o
#
DEPS_105 += $(CONFIG)/inc/me.h
DEPS_105 += $(CONFIG)/inc/mpr.h
DEPS_105 += $(CONFIG)/inc/http.h
DEPS_105 += $(CONFIG)/inc/ejsByteCode.h
DEPS_105 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_105 += $(CONFIG)/inc/ejs.slots.h
DEPS_105 += $(CONFIG)/inc/ejsCustomize.h
DEPS_105 += $(CONFIG)/inc/ejs.h
DEPS_105 += $(CONFIG)/inc/ejsCompiler.h
DEPS_105 += $(CONFIG)/inc/ejsWeb.h
DEPS_105 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsWeb.o: \
    src/ejs.web/ejsWeb.c $(DEPS_105)
	@echo '   [Compile] $(CONFIG)/obj/ejsWeb.o'
	$(CC) -c -o $(CONFIG)/obj/ejsWeb.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.web/ejsWeb.c

#
#   libejs.web
#
DEPS_106 += $(CONFIG)/inc/mpr.h
DEPS_106 += $(CONFIG)/inc/me.h
DEPS_106 += $(CONFIG)/inc/osdep.h
DEPS_106 += $(CONFIG)/obj/mprLib.o
DEPS_106 += $(CONFIG)/bin/libmpr.so
DEPS_106 += $(CONFIG)/inc/pcre.h
DEPS_106 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_106 += $(CONFIG)/bin/libpcre.so
endif
DEPS_106 += $(CONFIG)/inc/http.h
DEPS_106 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_106 += $(CONFIG)/bin/libhttp.so
endif
DEPS_106 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_106 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_106 += $(CONFIG)/inc/ejs.slots.h
DEPS_106 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_106 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_106 += $(CONFIG)/inc/ejsByteCode.h
DEPS_106 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_106 += $(CONFIG)/inc/ejsCustomize.h
DEPS_106 += $(CONFIG)/inc/ejs.h
DEPS_106 += $(CONFIG)/inc/ejsCompiler.h
DEPS_106 += $(CONFIG)/obj/ecAst.o
DEPS_106 += $(CONFIG)/obj/ecCodeGen.o
DEPS_106 += $(CONFIG)/obj/ecCompiler.o
DEPS_106 += $(CONFIG)/obj/ecLex.o
DEPS_106 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_106 += $(CONFIG)/obj/ecParser.o
DEPS_106 += $(CONFIG)/obj/ecState.o
DEPS_106 += $(CONFIG)/obj/dtoa.o
DEPS_106 += $(CONFIG)/obj/ejsApp.o
DEPS_106 += $(CONFIG)/obj/ejsArray.o
DEPS_106 += $(CONFIG)/obj/ejsBlock.o
DEPS_106 += $(CONFIG)/obj/ejsBoolean.o
DEPS_106 += $(CONFIG)/obj/ejsByteArray.o
DEPS_106 += $(CONFIG)/obj/ejsCache.o
DEPS_106 += $(CONFIG)/obj/ejsCmd.o
DEPS_106 += $(CONFIG)/obj/ejsConfig.o
DEPS_106 += $(CONFIG)/obj/ejsDate.o
DEPS_106 += $(CONFIG)/obj/ejsDebug.o
DEPS_106 += $(CONFIG)/obj/ejsError.o
DEPS_106 += $(CONFIG)/obj/ejsFile.o
DEPS_106 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_106 += $(CONFIG)/obj/ejsFrame.o
DEPS_106 += $(CONFIG)/obj/ejsFunction.o
DEPS_106 += $(CONFIG)/obj/ejsGC.o
DEPS_106 += $(CONFIG)/obj/ejsGlobal.o
DEPS_106 += $(CONFIG)/obj/ejsHttp.o
DEPS_106 += $(CONFIG)/obj/ejsIterator.o
DEPS_106 += $(CONFIG)/obj/ejsJSON.o
DEPS_106 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_106 += $(CONFIG)/obj/ejsMath.o
DEPS_106 += $(CONFIG)/obj/ejsMemory.o
DEPS_106 += $(CONFIG)/obj/ejsMprLog.o
DEPS_106 += $(CONFIG)/obj/ejsNamespace.o
DEPS_106 += $(CONFIG)/obj/ejsNull.o
DEPS_106 += $(CONFIG)/obj/ejsNumber.o
DEPS_106 += $(CONFIG)/obj/ejsObject.o
DEPS_106 += $(CONFIG)/obj/ejsPath.o
DEPS_106 += $(CONFIG)/obj/ejsPot.o
DEPS_106 += $(CONFIG)/obj/ejsRegExp.o
DEPS_106 += $(CONFIG)/obj/ejsSocket.o
DEPS_106 += $(CONFIG)/obj/ejsString.o
DEPS_106 += $(CONFIG)/obj/ejsSystem.o
DEPS_106 += $(CONFIG)/obj/ejsTimer.o
DEPS_106 += $(CONFIG)/obj/ejsType.o
DEPS_106 += $(CONFIG)/obj/ejsUri.o
DEPS_106 += $(CONFIG)/obj/ejsVoid.o
DEPS_106 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_106 += $(CONFIG)/obj/ejsWorker.o
DEPS_106 += $(CONFIG)/obj/ejsXML.o
DEPS_106 += $(CONFIG)/obj/ejsXMLList.o
DEPS_106 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_106 += $(CONFIG)/obj/ejsByteCode.o
DEPS_106 += $(CONFIG)/obj/ejsException.o
DEPS_106 += $(CONFIG)/obj/ejsHelper.o
DEPS_106 += $(CONFIG)/obj/ejsInterp.o
DEPS_106 += $(CONFIG)/obj/ejsLoader.o
DEPS_106 += $(CONFIG)/obj/ejsModule.o
DEPS_106 += $(CONFIG)/obj/ejsScope.o
DEPS_106 += $(CONFIG)/obj/ejsService.o
DEPS_106 += $(CONFIG)/bin/libejs.so
DEPS_106 += $(CONFIG)/obj/ejsc.o
DEPS_106 += $(CONFIG)/bin/ejsc
DEPS_106 += src/cmd/ejsmod.h
DEPS_106 += $(CONFIG)/obj/ejsmod.o
DEPS_106 += $(CONFIG)/obj/doc.o
DEPS_106 += $(CONFIG)/obj/docFiles.o
DEPS_106 += $(CONFIG)/obj/listing.o
DEPS_106 += $(CONFIG)/obj/slotGen.o
DEPS_106 += $(CONFIG)/bin/ejsmod
DEPS_106 += $(CONFIG)/bin/ejs.mod
DEPS_106 += $(CONFIG)/inc/ejsWeb.h
DEPS_106 += $(CONFIG)/obj/ejsHttpServer.o
DEPS_106 += $(CONFIG)/obj/ejsRequest.o
DEPS_106 += $(CONFIG)/obj/ejsSession.o
DEPS_106 += $(CONFIG)/obj/ejsWeb.o

LIBS_106 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_106 += -lhttp
endif
LIBS_106 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_106 += -lpcre
endif

$(CONFIG)/bin/libejs.web.so: $(DEPS_106)
	@echo '      [Link] $(CONFIG)/bin/libejs.web.so'
	$(CC) -shared -o $(CONFIG)/bin/libejs.web.so $(LIBPATHS) "$(CONFIG)/obj/ejsHttpServer.o" "$(CONFIG)/obj/ejsRequest.o" "$(CONFIG)/obj/ejsSession.o" "$(CONFIG)/obj/ejsWeb.o" $(LIBPATHS_106) $(LIBS_106) $(LIBS_106) $(LIBS) 

#
#   zlib.h
#
$(CONFIG)/inc/zlib.h: $(DEPS_107)
	@echo '      [Copy] $(CONFIG)/inc/zlib.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/zlib/zlib.h $(CONFIG)/inc/zlib.h

#
#   zlib.o
#
DEPS_108 += $(CONFIG)/inc/me.h
DEPS_108 += $(CONFIG)/inc/zlib.h

$(CONFIG)/obj/zlib.o: \
    src/paks/zlib/zlib.c $(DEPS_108)
	@echo '   [Compile] $(CONFIG)/obj/zlib.o'
	$(CC) -c -o $(CONFIG)/obj/zlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/zlib/zlib.c

ifeq ($(ME_COM_ZLIB),1)
#
#   libzlib
#
DEPS_109 += $(CONFIG)/inc/zlib.h
DEPS_109 += $(CONFIG)/inc/me.h
DEPS_109 += $(CONFIG)/obj/zlib.o

$(CONFIG)/bin/libzlib.so: $(DEPS_109)
	@echo '      [Link] $(CONFIG)/bin/libzlib.so'
	$(CC) -shared -o $(CONFIG)/bin/libzlib.so $(LIBPATHS) "$(CONFIG)/obj/zlib.o" $(LIBS) 
endif

#
#   ejsZlib.o
#
DEPS_110 += $(CONFIG)/inc/me.h
DEPS_110 += $(CONFIG)/inc/mpr.h
DEPS_110 += $(CONFIG)/inc/http.h
DEPS_110 += $(CONFIG)/inc/ejsByteCode.h
DEPS_110 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_110 += $(CONFIG)/inc/ejs.slots.h
DEPS_110 += $(CONFIG)/inc/ejsCustomize.h
DEPS_110 += $(CONFIG)/inc/ejs.h
DEPS_110 += $(CONFIG)/inc/zlib.h
DEPS_110 += $(CONFIG)/inc/ejs.zlib.slots.h

$(CONFIG)/obj/ejsZlib.o: \
    src/ejs.zlib/ejsZlib.c $(DEPS_110)
	@echo '   [Compile] $(CONFIG)/obj/ejsZlib.o'
	$(CC) -c -o $(CONFIG)/obj/ejsZlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.zlib/ejsZlib.c

#
#   libejs.zlib
#
DEPS_111 += $(CONFIG)/inc/mpr.h
DEPS_111 += $(CONFIG)/inc/me.h
DEPS_111 += $(CONFIG)/inc/osdep.h
DEPS_111 += $(CONFIG)/obj/mprLib.o
DEPS_111 += $(CONFIG)/bin/libmpr.so
DEPS_111 += $(CONFIG)/inc/pcre.h
DEPS_111 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_111 += $(CONFIG)/bin/libpcre.so
endif
DEPS_111 += $(CONFIG)/inc/http.h
DEPS_111 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_111 += $(CONFIG)/bin/libhttp.so
endif
DEPS_111 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_111 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_111 += $(CONFIG)/inc/ejs.slots.h
DEPS_111 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_111 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_111 += $(CONFIG)/inc/ejsByteCode.h
DEPS_111 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_111 += $(CONFIG)/inc/ejsCustomize.h
DEPS_111 += $(CONFIG)/inc/ejs.h
DEPS_111 += $(CONFIG)/inc/ejsCompiler.h
DEPS_111 += $(CONFIG)/obj/ecAst.o
DEPS_111 += $(CONFIG)/obj/ecCodeGen.o
DEPS_111 += $(CONFIG)/obj/ecCompiler.o
DEPS_111 += $(CONFIG)/obj/ecLex.o
DEPS_111 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_111 += $(CONFIG)/obj/ecParser.o
DEPS_111 += $(CONFIG)/obj/ecState.o
DEPS_111 += $(CONFIG)/obj/dtoa.o
DEPS_111 += $(CONFIG)/obj/ejsApp.o
DEPS_111 += $(CONFIG)/obj/ejsArray.o
DEPS_111 += $(CONFIG)/obj/ejsBlock.o
DEPS_111 += $(CONFIG)/obj/ejsBoolean.o
DEPS_111 += $(CONFIG)/obj/ejsByteArray.o
DEPS_111 += $(CONFIG)/obj/ejsCache.o
DEPS_111 += $(CONFIG)/obj/ejsCmd.o
DEPS_111 += $(CONFIG)/obj/ejsConfig.o
DEPS_111 += $(CONFIG)/obj/ejsDate.o
DEPS_111 += $(CONFIG)/obj/ejsDebug.o
DEPS_111 += $(CONFIG)/obj/ejsError.o
DEPS_111 += $(CONFIG)/obj/ejsFile.o
DEPS_111 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_111 += $(CONFIG)/obj/ejsFrame.o
DEPS_111 += $(CONFIG)/obj/ejsFunction.o
DEPS_111 += $(CONFIG)/obj/ejsGC.o
DEPS_111 += $(CONFIG)/obj/ejsGlobal.o
DEPS_111 += $(CONFIG)/obj/ejsHttp.o
DEPS_111 += $(CONFIG)/obj/ejsIterator.o
DEPS_111 += $(CONFIG)/obj/ejsJSON.o
DEPS_111 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_111 += $(CONFIG)/obj/ejsMath.o
DEPS_111 += $(CONFIG)/obj/ejsMemory.o
DEPS_111 += $(CONFIG)/obj/ejsMprLog.o
DEPS_111 += $(CONFIG)/obj/ejsNamespace.o
DEPS_111 += $(CONFIG)/obj/ejsNull.o
DEPS_111 += $(CONFIG)/obj/ejsNumber.o
DEPS_111 += $(CONFIG)/obj/ejsObject.o
DEPS_111 += $(CONFIG)/obj/ejsPath.o
DEPS_111 += $(CONFIG)/obj/ejsPot.o
DEPS_111 += $(CONFIG)/obj/ejsRegExp.o
DEPS_111 += $(CONFIG)/obj/ejsSocket.o
DEPS_111 += $(CONFIG)/obj/ejsString.o
DEPS_111 += $(CONFIG)/obj/ejsSystem.o
DEPS_111 += $(CONFIG)/obj/ejsTimer.o
DEPS_111 += $(CONFIG)/obj/ejsType.o
DEPS_111 += $(CONFIG)/obj/ejsUri.o
DEPS_111 += $(CONFIG)/obj/ejsVoid.o
DEPS_111 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_111 += $(CONFIG)/obj/ejsWorker.o
DEPS_111 += $(CONFIG)/obj/ejsXML.o
DEPS_111 += $(CONFIG)/obj/ejsXMLList.o
DEPS_111 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_111 += $(CONFIG)/obj/ejsByteCode.o
DEPS_111 += $(CONFIG)/obj/ejsException.o
DEPS_111 += $(CONFIG)/obj/ejsHelper.o
DEPS_111 += $(CONFIG)/obj/ejsInterp.o
DEPS_111 += $(CONFIG)/obj/ejsLoader.o
DEPS_111 += $(CONFIG)/obj/ejsModule.o
DEPS_111 += $(CONFIG)/obj/ejsScope.o
DEPS_111 += $(CONFIG)/obj/ejsService.o
DEPS_111 += $(CONFIG)/bin/libejs.so
DEPS_111 += $(CONFIG)/obj/ejsc.o
DEPS_111 += $(CONFIG)/bin/ejsc
DEPS_111 += src/cmd/ejsmod.h
DEPS_111 += $(CONFIG)/obj/ejsmod.o
DEPS_111 += $(CONFIG)/obj/doc.o
DEPS_111 += $(CONFIG)/obj/docFiles.o
DEPS_111 += $(CONFIG)/obj/listing.o
DEPS_111 += $(CONFIG)/obj/slotGen.o
DEPS_111 += $(CONFIG)/bin/ejsmod
DEPS_111 += $(CONFIG)/bin/ejs.mod
DEPS_111 += $(CONFIG)/bin/ejs.zlib.mod
DEPS_111 += $(CONFIG)/inc/zlib.h
DEPS_111 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_111 += $(CONFIG)/bin/libzlib.so
endif
DEPS_111 += $(CONFIG)/obj/ejsZlib.o

LIBS_111 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_111 += -lhttp
endif
LIBS_111 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_111 += -lpcre
endif
ifeq ($(ME_COM_ZLIB),1)
    LIBS_111 += -lzlib
endif

$(CONFIG)/bin/libejs.zlib.so: $(DEPS_111)
	@echo '      [Link] $(CONFIG)/bin/libejs.zlib.so'
	$(CC) -shared -o $(CONFIG)/bin/libejs.zlib.so $(LIBPATHS) "$(CONFIG)/obj/ejsZlib.o" $(LIBPATHS_111) $(LIBS_111) $(LIBS_111) $(LIBS) 

#
#   est.h
#
$(CONFIG)/inc/est.h: $(DEPS_112)
	@echo '      [Copy] $(CONFIG)/inc/est.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/est/est.h $(CONFIG)/inc/est.h

#
#   estLib.o
#
DEPS_113 += $(CONFIG)/inc/me.h
DEPS_113 += $(CONFIG)/inc/est.h
DEPS_113 += $(CONFIG)/inc/osdep.h

$(CONFIG)/obj/estLib.o: \
    src/paks/est/estLib.c $(DEPS_113)
	@echo '   [Compile] $(CONFIG)/obj/estLib.o'
	$(CC) -c -o $(CONFIG)/obj/estLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/est/estLib.c

ifeq ($(ME_COM_EST),1)
#
#   libest
#
DEPS_114 += $(CONFIG)/inc/est.h
DEPS_114 += $(CONFIG)/inc/me.h
DEPS_114 += $(CONFIG)/inc/osdep.h
DEPS_114 += $(CONFIG)/obj/estLib.o

$(CONFIG)/bin/libest.so: $(DEPS_114)
	@echo '      [Link] $(CONFIG)/bin/libest.so'
	$(CC) -shared -o $(CONFIG)/bin/libest.so $(LIBPATHS) "$(CONFIG)/obj/estLib.o" $(LIBS) 
endif

#
#   mprSsl.o
#
DEPS_115 += $(CONFIG)/inc/me.h
DEPS_115 += $(CONFIG)/inc/mpr.h
DEPS_115 += $(CONFIG)/inc/est.h

$(CONFIG)/obj/mprSsl.o: \
    src/paks/mpr/mprSsl.c $(DEPS_115)
	@echo '   [Compile] $(CONFIG)/obj/mprSsl.o'
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-I$(ME_COM_MATRIXSSL_PATH)" "-I$(ME_COM_MATRIXSSL_PATH)/matrixssl" "-I$(ME_COM_NANOSSL_PATH)/src" src/paks/mpr/mprSsl.c

#
#   libmprssl
#
DEPS_116 += $(CONFIG)/inc/mpr.h
DEPS_116 += $(CONFIG)/inc/me.h
DEPS_116 += $(CONFIG)/inc/osdep.h
DEPS_116 += $(CONFIG)/obj/mprLib.o
DEPS_116 += $(CONFIG)/bin/libmpr.so
DEPS_116 += $(CONFIG)/inc/est.h
DEPS_116 += $(CONFIG)/obj/estLib.o
ifeq ($(ME_COM_EST),1)
    DEPS_116 += $(CONFIG)/bin/libest.so
endif
DEPS_116 += $(CONFIG)/obj/mprSsl.o

LIBS_116 += -lmpr
ifeq ($(ME_COM_EST),1)
    LIBS_116 += -lest
endif
ifeq ($(ME_COM_MATRIXSSL),1)
    LIBS_116 += -lmatrixssl
    LIBPATHS_116 += -L$(ME_COM_MATRIXSSL_PATH)
endif
ifeq ($(ME_COM_NANOSSL),1)
    LIBS_116 += -lssls
    LIBPATHS_116 += -L$(ME_COM_NANOSSL_PATH)/bin
endif

$(CONFIG)/bin/libmprssl.so: $(DEPS_116)
	@echo '      [Link] $(CONFIG)/bin/libmprssl.so'
	$(CC) -shared -o $(CONFIG)/bin/libmprssl.so $(LIBPATHS)   "$(CONFIG)/obj/mprSsl.o" $(LIBPATHS_116) $(LIBS_116) $(LIBS_116) $(LIBS) 

#
#   sqlite3.h
#
$(CONFIG)/inc/sqlite3.h: $(DEPS_117)
	@echo '      [Copy] $(CONFIG)/inc/sqlite3.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/sqlite/sqlite3.h $(CONFIG)/inc/sqlite3.h

#
#   sqlite3.o
#
DEPS_118 += $(CONFIG)/inc/me.h
DEPS_118 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite3.o: \
    src/paks/sqlite/sqlite3.c $(DEPS_118)
	@echo '   [Compile] $(CONFIG)/obj/sqlite3.o'
	$(CC) -c -o $(CONFIG)/obj/sqlite3.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/sqlite/sqlite3.c

ifeq ($(ME_COM_SQLITE),1)
#
#   libsql
#
DEPS_119 += $(CONFIG)/inc/sqlite3.h
DEPS_119 += $(CONFIG)/inc/me.h
DEPS_119 += $(CONFIG)/obj/sqlite3.o

$(CONFIG)/bin/libsql.so: $(DEPS_119)
	@echo '      [Link] $(CONFIG)/bin/libsql.so'
	$(CC) -shared -o $(CONFIG)/bin/libsql.so $(LIBPATHS) "$(CONFIG)/obj/sqlite3.o" $(LIBS) 
endif

#
#   manager.o
#
DEPS_120 += $(CONFIG)/inc/me.h
DEPS_120 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/manager.o: \
    src/paks/mpr/manager.c $(DEPS_120)
	@echo '   [Compile] $(CONFIG)/obj/manager.o'
	$(CC) -c -o $(CONFIG)/obj/manager.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/manager.c

#
#   manager
#
DEPS_121 += $(CONFIG)/inc/mpr.h
DEPS_121 += $(CONFIG)/inc/me.h
DEPS_121 += $(CONFIG)/inc/osdep.h
DEPS_121 += $(CONFIG)/obj/mprLib.o
DEPS_121 += $(CONFIG)/bin/libmpr.so
DEPS_121 += $(CONFIG)/obj/manager.o

LIBS_121 += -lmpr

$(CONFIG)/bin/ejsman: $(DEPS_121)
	@echo '      [Link] $(CONFIG)/bin/ejsman'
	$(CC) -o $(CONFIG)/bin/ejsman $(LIBPATHS) "$(CONFIG)/obj/manager.o" $(LIBPATHS_121) $(LIBS_121) $(LIBS_121) $(LIBS) $(LIBS) 


#
#   mvc
#
DEPS_122 += $(CONFIG)/inc/mpr.h
DEPS_122 += $(CONFIG)/inc/me.h
DEPS_122 += $(CONFIG)/inc/osdep.h
DEPS_122 += $(CONFIG)/obj/mprLib.o
DEPS_122 += $(CONFIG)/bin/libmpr.so
DEPS_122 += $(CONFIG)/inc/pcre.h
DEPS_122 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_122 += $(CONFIG)/bin/libpcre.so
endif
DEPS_122 += $(CONFIG)/inc/http.h
DEPS_122 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_122 += $(CONFIG)/bin/libhttp.so
endif
DEPS_122 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_122 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_122 += $(CONFIG)/inc/ejs.slots.h
DEPS_122 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_122 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_122 += $(CONFIG)/inc/ejsByteCode.h
DEPS_122 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_122 += $(CONFIG)/inc/ejsCustomize.h
DEPS_122 += $(CONFIG)/inc/ejs.h
DEPS_122 += $(CONFIG)/inc/ejsCompiler.h
DEPS_122 += $(CONFIG)/obj/ecAst.o
DEPS_122 += $(CONFIG)/obj/ecCodeGen.o
DEPS_122 += $(CONFIG)/obj/ecCompiler.o
DEPS_122 += $(CONFIG)/obj/ecLex.o
DEPS_122 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_122 += $(CONFIG)/obj/ecParser.o
DEPS_122 += $(CONFIG)/obj/ecState.o
DEPS_122 += $(CONFIG)/obj/dtoa.o
DEPS_122 += $(CONFIG)/obj/ejsApp.o
DEPS_122 += $(CONFIG)/obj/ejsArray.o
DEPS_122 += $(CONFIG)/obj/ejsBlock.o
DEPS_122 += $(CONFIG)/obj/ejsBoolean.o
DEPS_122 += $(CONFIG)/obj/ejsByteArray.o
DEPS_122 += $(CONFIG)/obj/ejsCache.o
DEPS_122 += $(CONFIG)/obj/ejsCmd.o
DEPS_122 += $(CONFIG)/obj/ejsConfig.o
DEPS_122 += $(CONFIG)/obj/ejsDate.o
DEPS_122 += $(CONFIG)/obj/ejsDebug.o
DEPS_122 += $(CONFIG)/obj/ejsError.o
DEPS_122 += $(CONFIG)/obj/ejsFile.o
DEPS_122 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_122 += $(CONFIG)/obj/ejsFrame.o
DEPS_122 += $(CONFIG)/obj/ejsFunction.o
DEPS_122 += $(CONFIG)/obj/ejsGC.o
DEPS_122 += $(CONFIG)/obj/ejsGlobal.o
DEPS_122 += $(CONFIG)/obj/ejsHttp.o
DEPS_122 += $(CONFIG)/obj/ejsIterator.o
DEPS_122 += $(CONFIG)/obj/ejsJSON.o
DEPS_122 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_122 += $(CONFIG)/obj/ejsMath.o
DEPS_122 += $(CONFIG)/obj/ejsMemory.o
DEPS_122 += $(CONFIG)/obj/ejsMprLog.o
DEPS_122 += $(CONFIG)/obj/ejsNamespace.o
DEPS_122 += $(CONFIG)/obj/ejsNull.o
DEPS_122 += $(CONFIG)/obj/ejsNumber.o
DEPS_122 += $(CONFIG)/obj/ejsObject.o
DEPS_122 += $(CONFIG)/obj/ejsPath.o
DEPS_122 += $(CONFIG)/obj/ejsPot.o
DEPS_122 += $(CONFIG)/obj/ejsRegExp.o
DEPS_122 += $(CONFIG)/obj/ejsSocket.o
DEPS_122 += $(CONFIG)/obj/ejsString.o
DEPS_122 += $(CONFIG)/obj/ejsSystem.o
DEPS_122 += $(CONFIG)/obj/ejsTimer.o
DEPS_122 += $(CONFIG)/obj/ejsType.o
DEPS_122 += $(CONFIG)/obj/ejsUri.o
DEPS_122 += $(CONFIG)/obj/ejsVoid.o
DEPS_122 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_122 += $(CONFIG)/obj/ejsWorker.o
DEPS_122 += $(CONFIG)/obj/ejsXML.o
DEPS_122 += $(CONFIG)/obj/ejsXMLList.o
DEPS_122 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_122 += $(CONFIG)/obj/ejsByteCode.o
DEPS_122 += $(CONFIG)/obj/ejsException.o
DEPS_122 += $(CONFIG)/obj/ejsHelper.o
DEPS_122 += $(CONFIG)/obj/ejsInterp.o
DEPS_122 += $(CONFIG)/obj/ejsLoader.o
DEPS_122 += $(CONFIG)/obj/ejsModule.o
DEPS_122 += $(CONFIG)/obj/ejsScope.o
DEPS_122 += $(CONFIG)/obj/ejsService.o
DEPS_122 += $(CONFIG)/bin/libejs.so
DEPS_122 += $(CONFIG)/bin/mvc.es
DEPS_122 += $(CONFIG)/obj/ejsrun.o

LIBS_122 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_122 += -lhttp
endif
LIBS_122 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_122 += -lpcre
endif

$(CONFIG)/bin/mvc: $(DEPS_122)
	@echo '      [Link] $(CONFIG)/bin/mvc'
	$(CC) -o $(CONFIG)/bin/mvc $(LIBPATHS) "$(CONFIG)/obj/ejsrun.o" $(LIBPATHS_122) $(LIBS_122) $(LIBS_122) $(LIBS) $(LIBS) 

#
#   sqlite.o
#
DEPS_123 += $(CONFIG)/inc/me.h
DEPS_123 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite.o: \
    src/paks/sqlite/sqlite.c $(DEPS_123)
	@echo '   [Compile] $(CONFIG)/obj/sqlite.o'
	$(CC) -c -o $(CONFIG)/obj/sqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/sqlite/sqlite.c

ifeq ($(ME_COM_SQLITE),1)
#
#   sqliteshell
#
DEPS_124 += $(CONFIG)/inc/sqlite3.h
DEPS_124 += $(CONFIG)/inc/me.h
DEPS_124 += $(CONFIG)/obj/sqlite3.o
DEPS_124 += $(CONFIG)/bin/libsql.so
DEPS_124 += $(CONFIG)/obj/sqlite.o

LIBS_124 += -lsql

$(CONFIG)/bin/sqlite: $(DEPS_124)
	@echo '      [Link] $(CONFIG)/bin/sqlite'
	$(CC) -o $(CONFIG)/bin/sqlite $(LIBPATHS) "$(CONFIG)/obj/sqlite.o" $(LIBPATHS_124) $(LIBS_124) $(LIBS_124) $(LIBS) $(LIBS) 
endif



#
#   utest
#
DEPS_125 += $(CONFIG)/inc/mpr.h
DEPS_125 += $(CONFIG)/inc/me.h
DEPS_125 += $(CONFIG)/inc/osdep.h
DEPS_125 += $(CONFIG)/obj/mprLib.o
DEPS_125 += $(CONFIG)/bin/libmpr.so
DEPS_125 += $(CONFIG)/inc/pcre.h
DEPS_125 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_125 += $(CONFIG)/bin/libpcre.so
endif
DEPS_125 += $(CONFIG)/inc/http.h
DEPS_125 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_125 += $(CONFIG)/bin/libhttp.so
endif
DEPS_125 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_125 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_125 += $(CONFIG)/inc/ejs.slots.h
DEPS_125 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_125 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_125 += $(CONFIG)/inc/ejsByteCode.h
DEPS_125 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_125 += $(CONFIG)/inc/ejsCustomize.h
DEPS_125 += $(CONFIG)/inc/ejs.h
DEPS_125 += $(CONFIG)/inc/ejsCompiler.h
DEPS_125 += $(CONFIG)/obj/ecAst.o
DEPS_125 += $(CONFIG)/obj/ecCodeGen.o
DEPS_125 += $(CONFIG)/obj/ecCompiler.o
DEPS_125 += $(CONFIG)/obj/ecLex.o
DEPS_125 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_125 += $(CONFIG)/obj/ecParser.o
DEPS_125 += $(CONFIG)/obj/ecState.o
DEPS_125 += $(CONFIG)/obj/dtoa.o
DEPS_125 += $(CONFIG)/obj/ejsApp.o
DEPS_125 += $(CONFIG)/obj/ejsArray.o
DEPS_125 += $(CONFIG)/obj/ejsBlock.o
DEPS_125 += $(CONFIG)/obj/ejsBoolean.o
DEPS_125 += $(CONFIG)/obj/ejsByteArray.o
DEPS_125 += $(CONFIG)/obj/ejsCache.o
DEPS_125 += $(CONFIG)/obj/ejsCmd.o
DEPS_125 += $(CONFIG)/obj/ejsConfig.o
DEPS_125 += $(CONFIG)/obj/ejsDate.o
DEPS_125 += $(CONFIG)/obj/ejsDebug.o
DEPS_125 += $(CONFIG)/obj/ejsError.o
DEPS_125 += $(CONFIG)/obj/ejsFile.o
DEPS_125 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_125 += $(CONFIG)/obj/ejsFrame.o
DEPS_125 += $(CONFIG)/obj/ejsFunction.o
DEPS_125 += $(CONFIG)/obj/ejsGC.o
DEPS_125 += $(CONFIG)/obj/ejsGlobal.o
DEPS_125 += $(CONFIG)/obj/ejsHttp.o
DEPS_125 += $(CONFIG)/obj/ejsIterator.o
DEPS_125 += $(CONFIG)/obj/ejsJSON.o
DEPS_125 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_125 += $(CONFIG)/obj/ejsMath.o
DEPS_125 += $(CONFIG)/obj/ejsMemory.o
DEPS_125 += $(CONFIG)/obj/ejsMprLog.o
DEPS_125 += $(CONFIG)/obj/ejsNamespace.o
DEPS_125 += $(CONFIG)/obj/ejsNull.o
DEPS_125 += $(CONFIG)/obj/ejsNumber.o
DEPS_125 += $(CONFIG)/obj/ejsObject.o
DEPS_125 += $(CONFIG)/obj/ejsPath.o
DEPS_125 += $(CONFIG)/obj/ejsPot.o
DEPS_125 += $(CONFIG)/obj/ejsRegExp.o
DEPS_125 += $(CONFIG)/obj/ejsSocket.o
DEPS_125 += $(CONFIG)/obj/ejsString.o
DEPS_125 += $(CONFIG)/obj/ejsSystem.o
DEPS_125 += $(CONFIG)/obj/ejsTimer.o
DEPS_125 += $(CONFIG)/obj/ejsType.o
DEPS_125 += $(CONFIG)/obj/ejsUri.o
DEPS_125 += $(CONFIG)/obj/ejsVoid.o
DEPS_125 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_125 += $(CONFIG)/obj/ejsWorker.o
DEPS_125 += $(CONFIG)/obj/ejsXML.o
DEPS_125 += $(CONFIG)/obj/ejsXMLList.o
DEPS_125 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_125 += $(CONFIG)/obj/ejsByteCode.o
DEPS_125 += $(CONFIG)/obj/ejsException.o
DEPS_125 += $(CONFIG)/obj/ejsHelper.o
DEPS_125 += $(CONFIG)/obj/ejsInterp.o
DEPS_125 += $(CONFIG)/obj/ejsLoader.o
DEPS_125 += $(CONFIG)/obj/ejsModule.o
DEPS_125 += $(CONFIG)/obj/ejsScope.o
DEPS_125 += $(CONFIG)/obj/ejsService.o
DEPS_125 += $(CONFIG)/bin/libejs.so
DEPS_125 += $(CONFIG)/bin/utest.es
DEPS_125 += $(CONFIG)/bin/utest.worker
DEPS_125 += $(CONFIG)/obj/ejsrun.o

LIBS_125 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_125 += -lhttp
endif
LIBS_125 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_125 += -lpcre
endif

$(CONFIG)/bin/utest: $(DEPS_125)
	@echo '      [Link] $(CONFIG)/bin/utest'
	$(CC) -o $(CONFIG)/bin/utest $(LIBPATHS) "$(CONFIG)/obj/ejsrun.o" $(LIBPATHS_125) $(LIBS_125) $(LIBS_125) $(LIBS) $(LIBS) 

#
#   www
#
DEPS_126 += src/ejs.web/www

$(CONFIG)/bin/www: $(DEPS_126)
	( \
	cd src/ejs.web; \
	rm -fr ../../$(CONFIG)/bin/www ; \
	cp -r www ../../$(CONFIG)/bin ; \
	)

#
#   installBinary
#
installBinary: $(DEPS_127)
	( \
	cd .; \
	mkdir -p "$(ME_APP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	ln -s "2.3.5" "$(ME_APP_PREFIX)/latest" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(CONFIG)/bin/ejs $(ME_VAPP_PREFIX)/bin/ejs ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/ejs" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejs" "$(ME_BIN_PREFIX)/ejs" ; \
	cp $(CONFIG)/bin/ejsc $(ME_VAPP_PREFIX)/bin/ejsc ; \
	rm -f "$(ME_BIN_PREFIX)/ejsc" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsc" "$(ME_BIN_PREFIX)/ejsc" ; \
	cp $(CONFIG)/bin/ejsman $(ME_VAPP_PREFIX)/bin/ejsman ; \
	rm -f "$(ME_BIN_PREFIX)/ejsman" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsman" "$(ME_BIN_PREFIX)/ejsman" ; \
	cp $(CONFIG)/bin/ejsmod $(ME_VAPP_PREFIX)/bin/ejsmod ; \
	rm -f "$(ME_BIN_PREFIX)/ejsmod" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsmod" "$(ME_BIN_PREFIX)/ejsmod" ; \
	cp $(CONFIG)/bin/ejsrun $(ME_VAPP_PREFIX)/bin/ejsrun ; \
	rm -f "$(ME_BIN_PREFIX)/ejsrun" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/ejsrun" "$(ME_BIN_PREFIX)/ejsrun" ; \
	cp $(CONFIG)/bin/mvc $(ME_VAPP_PREFIX)/bin/mvc ; \
	rm -f "$(ME_BIN_PREFIX)/mvc" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/mvc" "$(ME_BIN_PREFIX)/mvc" ; \
	cp $(CONFIG)/bin/utest $(ME_VAPP_PREFIX)/bin/utest ; \
	rm -f "$(ME_BIN_PREFIX)/utest" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/utest" "$(ME_BIN_PREFIX)/utest" ; \
	cp $(CONFIG)/bin/libejs.db.sqlite.so $(ME_VAPP_PREFIX)/bin/libejs.db.sqlite.so ; \
	cp $(CONFIG)/bin/libejs.so $(ME_VAPP_PREFIX)/bin/libejs.so ; \
	cp $(CONFIG)/bin/libejs.web.so $(ME_VAPP_PREFIX)/bin/libejs.web.so ; \
	cp $(CONFIG)/bin/libejs.zlib.so $(ME_VAPP_PREFIX)/bin/libejs.zlib.so ; \
	cp $(CONFIG)/bin/libhttp.so $(ME_VAPP_PREFIX)/bin/libhttp.so ; \
	cp $(CONFIG)/bin/libmpr.so $(ME_VAPP_PREFIX)/bin/libmpr.so ; \
	cp $(CONFIG)/bin/libpcre.so $(ME_VAPP_PREFIX)/bin/libpcre.so ; \
	cp $(CONFIG)/bin/libsql.so $(ME_VAPP_PREFIX)/bin/libsql.so ; \
	cp $(CONFIG)/bin/libzlib.so $(ME_VAPP_PREFIX)/bin/libzlib.so ; \
	cp $(CONFIG)/bin/ejs.db.mapper.mod $(ME_VAPP_PREFIX)/bin/ejs.db.mapper.mod ; \
	cp $(CONFIG)/bin/ejs.db.mod $(ME_VAPP_PREFIX)/bin/ejs.db.mod ; \
	cp $(CONFIG)/bin/ejs.db.sqlite.mod $(ME_VAPP_PREFIX)/bin/ejs.db.sqlite.mod ; \
	cp $(CONFIG)/bin/ejs.mod $(ME_VAPP_PREFIX)/bin/ejs.mod ; \
	cp $(CONFIG)/bin/ejs.mvc.mod $(ME_VAPP_PREFIX)/bin/ejs.mvc.mod ; \
	cp $(CONFIG)/bin/ejs.tar.mod $(ME_VAPP_PREFIX)/bin/ejs.tar.mod ; \
	cp $(CONFIG)/bin/ejs.template.mod $(ME_VAPP_PREFIX)/bin/ejs.template.mod ; \
	cp $(CONFIG)/bin/ejs.unix.mod $(ME_VAPP_PREFIX)/bin/ejs.unix.mod ; \
	cp $(CONFIG)/bin/ejs.web.mod $(ME_VAPP_PREFIX)/bin/ejs.web.mod ; \
	cp $(CONFIG)/bin/ejs.zlib.mod $(ME_VAPP_PREFIX)/bin/ejs.zlib.mod ; \
	cp $(CONFIG)/bin/mvc.es $(ME_VAPP_PREFIX)/bin/mvc.es ; \
	cp $(CONFIG)/bin/utest.es $(ME_VAPP_PREFIX)/bin/utest.es ; \
	cp $(CONFIG)/bin/utest.worker $(ME_VAPP_PREFIX)/bin/utest.worker ; \
	if [ "$(ME_COM_SSL)" = 1 ]; then true ; \
	cp $(CONFIG)/bin/libmprssl.so $(ME_VAPP_PREFIX)/bin/libmprssl.so ; \
	fi ; \
	if [ "$(ME_COM_SSL)" = 1 ]; then true ; \
	cp $(CONFIG)/bin/ca.crt $(ME_VAPP_PREFIX)/bin/ca.crt ; \
	fi ; \
	if [ "$(ME_COM_OPENSSL)" = 1 ]; then true ; \
	cp $(CONFIG)/bin/libssl*.so* $(ME_VAPP_PREFIX)/bin/libssl*.so* ; \
	cp $(CONFIG)/bin/libcrypto*.so* $(ME_VAPP_PREFIX)/bin/libcrypto*.so* ; \
	fi ; \
	if [ "$(ME_COM_EST)" = 1 ]; then true ; \
	cp $(CONFIG)/bin/libest.so $(ME_VAPP_PREFIX)/bin/libest.so ; \
	fi ; \
	cp $(CONFIG)/bin/libejs.so $(ME_VAPP_PREFIX)/bin/libejs.so ; \
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
	cp src/core/ejs.slots.h $(ME_VAPP_PREFIX)/inc/ejs.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.slots.h" ; \
	cp src/ejs.db.sqlite/ejs.db.sqlite.slots.h $(ME_VAPP_PREFIX)/inc/ejs.db.sqlite.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.db.sqlite.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.db.sqlite.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.db.sqlite.slots.h" ; \
	cp src/ejs.h $(ME_VAPP_PREFIX)/inc/ejs.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.h" "$(ME_INC_PREFIX)/ejs/ejs.h" ; \
	cp src/ejs.web/ejs.web.slots.h $(ME_VAPP_PREFIX)/inc/ejs.web.slots.h ; \
	rm -f "$(ME_INC_PREFIX)/ejs/ejs.web.slots.h" ; \
	ln -s "$(ME_VAPP_PREFIX)/inc/ejs.web.slots.h" "$(ME_INC_PREFIX)/ejs/ejs.web.slots.h" ; \
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
start: $(DEPS_128)

#
#   stop
#
stop: $(DEPS_129)

#
#   uninstall
#
DEPS_130 += stop

uninstall: $(DEPS_130)

