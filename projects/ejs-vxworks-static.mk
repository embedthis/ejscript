#
#   ejs-vxworks-static.mk -- Makefile to build Embedthis Ejscript for vxworks
#

NAME                  := ejs
VERSION               := 2.3.5
PROFILE               ?= static
ARCH                  ?= $(shell echo $(WIND_HOST_TYPE) | sed 's/-.*//')
CPU                   ?= $(subst X86,PENTIUM,$(shell echo $(ARCH) | tr a-z A-Z))
OS                    ?= vxworks
CC                    ?= cc$(subst x86,pentium,$(ARCH))
LD                    ?= ld
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

ME_COM_COMPILER_PATH  ?= cc$(subst x86,pentium,$(ARCH))
ME_COM_LIB_PATH       ?= ar
ME_COM_LINK_PATH      ?= ld
ME_COM_MATRIXSSL_PATH ?= /usr/src/matrixssl
ME_COM_NANOSSL_PATH   ?= /usr/src/nanossl
ME_COM_OPENSSL_PATH   ?= /usr/src/openssl
ME_COM_VXWORKS_PATH   ?= $(WIND_BASE)

export WIND_HOME      ?= $(WIND_BASE)/..
export PATH           := $(WIND_GNU_PATH)/$(WIND_HOST_TYPE)/bin:$(PATH)

CFLAGS                += -fno-builtin -fno-defer-pop -fvolatile -w
DFLAGS                += -DVXWORKS -DRW_MULTI_THREAD -D_GNU_TOOL -DCPU=PENTIUM $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_EST=$(ME_COM_EST) -DME_COM_HTTP=$(ME_COM_HTTP) -DME_COM_MATRIXSSL=$(ME_COM_MATRIXSSL) -DME_COM_NANOSSL=$(ME_COM_NANOSSL) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_PCRE=$(ME_COM_PCRE) -DME_COM_SQLITE=$(ME_COM_SQLITE) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_WINSDK=$(ME_COM_WINSDK) -DME_COM_ZLIB=$(ME_COM_ZLIB) 
IFLAGS                += "-I$(CONFIG)/inc -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/h/wrn/coreip"
LDFLAGS               += '-Wl,-r'
LIBPATHS              += -L$(CONFIG)/bin
LIBS                  += -lgcc

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

ME_ROOT_PREFIX        ?= deploy
ME_BASE_PREFIX        ?= $(ME_ROOT_PREFIX)
ME_DATA_PREFIX        ?= $(ME_VAPP_PREFIX)
ME_STATE_PREFIX       ?= $(ME_VAPP_PREFIX)
ME_BIN_PREFIX         ?= $(ME_VAPP_PREFIX)
ME_INC_PREFIX         ?= $(ME_VAPP_PREFIX)/inc
ME_LIB_PREFIX         ?= $(ME_VAPP_PREFIX)
ME_MAN_PREFIX         ?= $(ME_VAPP_PREFIX)
ME_SBIN_PREFIX        ?= $(ME_VAPP_PREFIX)
ME_ETC_PREFIX         ?= $(ME_VAPP_PREFIX)
ME_WEB_PREFIX         ?= $(ME_VAPP_PREFIX)/web
ME_LOG_PREFIX         ?= $(ME_VAPP_PREFIX)
ME_SPOOL_PREFIX       ?= $(ME_VAPP_PREFIX)
ME_CACHE_PREFIX       ?= $(ME_VAPP_PREFIX)
ME_APP_PREFIX         ?= $(ME_BASE_PREFIX)
ME_VAPP_PREFIX        ?= $(ME_APP_PREFIX)
ME_SRC_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/src/$(NAME)-$(VERSION)


ifeq ($(ME_COM_EST),1)
    TARGETS           += $(CONFIG)/bin/ca.crt
endif
TARGETS               += $(CONFIG)/bin/ejs.out
TARGETS               += $(CONFIG)/bin/ejs.db.mapper.mod
TARGETS               += $(CONFIG)/bin/ejs.db.sqlite.mod
TARGETS               += $(CONFIG)/bin/ejs.mail.mod
TARGETS               += $(CONFIG)/bin/ejs.mvc.mod
TARGETS               += $(CONFIG)/bin/ejs.tar.mod
TARGETS               += $(CONFIG)/bin/ejs.zlib.mod
TARGETS               += $(CONFIG)/bin/ejsrun.out
ifeq ($(ME_COM_HTTP),1)
    TARGETS           += $(CONFIG)/bin/http.out
endif
TARGETS               += $(CONFIG)/bin/libejs.db.sqlite.a
TARGETS               += $(CONFIG)/bin/libejs.web.a
TARGETS               += $(CONFIG)/bin/libejs.zlib.a
TARGETS               += $(CONFIG)/bin/ejsman.out
TARGETS               += $(CONFIG)/bin/mvc.out
ifeq ($(ME_COM_SQLITE),1)
    TARGETS           += $(CONFIG)/bin/sqlite.out
endif
TARGETS               += $(CONFIG)/bin/utest.out
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
	@if [ "$(WIND_BASE)" = "" ] ; then echo WARNING: WIND_BASE not set. Run wrenv.sh. ; exit 255 ; fi
	@if [ "$(WIND_HOST_TYPE)" = "" ] ; then echo WARNING: WIND_HOST_TYPE not set. Run wrenv.sh. ; exit 255 ; fi
	@if [ "$(WIND_GNU_PATH)" = "" ] ; then echo WARNING: WIND_GNU_PATH not set. Run wrenv.sh. ; exit 255 ; fi
	@[ ! -x $(CONFIG)/bin ] && mkdir -p $(CONFIG)/bin; true
	@[ ! -x $(CONFIG)/inc ] && mkdir -p $(CONFIG)/inc; true
	@[ ! -x $(CONFIG)/obj ] && mkdir -p $(CONFIG)/obj; true
	@[ ! -f $(CONFIG)/inc/osdep.h ] && cp src/paks/osdep/osdep.h $(CONFIG)/inc/osdep.h ; true
	@if ! diff $(CONFIG)/inc/osdep.h src/paks/osdep/osdep.h >/dev/null ; then\
		cp src/paks/osdep/osdep.h $(CONFIG)/inc/osdep.h  ; \
	fi; true
	@[ ! -f $(CONFIG)/inc/me.h ] && cp projects/ejs-vxworks-static-me.h $(CONFIG)/inc/me.h ; true
	@if ! diff $(CONFIG)/inc/me.h projects/ejs-vxworks-static-me.h >/dev/null ; then\
		cp projects/ejs-vxworks-static-me.h $(CONFIG)/inc/me.h  ; \
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
	rm -f "$(CONFIG)/obj/ejsSqlite.o"
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
	rm -f "$(CONFIG)/bin/ejs.out"
	rm -f "$(CONFIG)/bin/ejsc.out"
	rm -f "$(CONFIG)/bin/ejsmod.out"
	rm -f "$(CONFIG)/bin/ejsrun.out"
	rm -f "$(CONFIG)/bin/http.out"
	rm -f "$(CONFIG)/bin/libejs.a"
	rm -f "$(CONFIG)/bin/libejs.db.sqlite.a"
	rm -f "$(CONFIG)/bin/libejs.web.a"
	rm -f "$(CONFIG)/bin/libejs.zlib.a"
	rm -f "$(CONFIG)/bin/libest.a"
	rm -f "$(CONFIG)/bin/libhttp.a"
	rm -f "$(CONFIG)/bin/libmpr.a"
	rm -f "$(CONFIG)/bin/libmprssl.a"
	rm -f "$(CONFIG)/bin/libpcre.a"
	rm -f "$(CONFIG)/bin/libsql.a"
	rm -f "$(CONFIG)/bin/libzlib.a"
	rm -f "$(CONFIG)/bin/makerom.out"
	rm -f "$(CONFIG)/bin/ejsman.out"
	rm -f "$(CONFIG)/bin/sqlite.out"
	rm -f "$(CONFIG)/bin/utest.out"

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
	$(CC) -c -o $(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/paks/mpr/mprLib.c

#
#   libmpr
#
DEPS_6 += $(CONFIG)/inc/mpr.h
DEPS_6 += $(CONFIG)/inc/me.h
DEPS_6 += $(CONFIG)/inc/osdep.h
DEPS_6 += $(CONFIG)/obj/mprLib.o

$(CONFIG)/bin/libmpr.a: $(DEPS_6)
	@echo '      [Link] $(CONFIG)/bin/libmpr.a'
	ar -cr $(CONFIG)/bin/libmpr.a "$(CONFIG)/obj/mprLib.o"

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
	$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/paks/pcre/pcre.c

ifeq ($(ME_COM_PCRE),1)
#
#   libpcre
#
DEPS_9 += $(CONFIG)/inc/pcre.h
DEPS_9 += $(CONFIG)/inc/me.h
DEPS_9 += $(CONFIG)/obj/pcre.o

$(CONFIG)/bin/libpcre.a: $(DEPS_9)
	@echo '      [Link] $(CONFIG)/bin/libpcre.a'
	ar -cr $(CONFIG)/bin/libpcre.a "$(CONFIG)/obj/pcre.o"
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
	$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/paks/http/httpLib.c

ifeq ($(ME_COM_HTTP),1)
#
#   libhttp
#
DEPS_12 += $(CONFIG)/inc/mpr.h
DEPS_12 += $(CONFIG)/inc/me.h
DEPS_12 += $(CONFIG)/inc/osdep.h
DEPS_12 += $(CONFIG)/obj/mprLib.o
DEPS_12 += $(CONFIG)/bin/libmpr.a
DEPS_12 += $(CONFIG)/inc/pcre.h
DEPS_12 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_12 += $(CONFIG)/bin/libpcre.a
endif
DEPS_12 += $(CONFIG)/inc/http.h
DEPS_12 += $(CONFIG)/obj/httpLib.o

$(CONFIG)/bin/libhttp.a: $(DEPS_12)
	@echo '      [Link] $(CONFIG)/bin/libhttp.a'
	ar -cr $(CONFIG)/bin/libhttp.a "$(CONFIG)/obj/httpLib.o"
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
	$(CC) -c -o $(CONFIG)/obj/ecAst.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecAst.c

#
#   ecCodeGen.o
#
DEPS_24 += $(CONFIG)/inc/me.h
DEPS_24 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecCodeGen.o: \
    src/compiler/ecCodeGen.c $(DEPS_24)
	@echo '   [Compile] $(CONFIG)/obj/ecCodeGen.o'
	$(CC) -c -o $(CONFIG)/obj/ecCodeGen.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecCodeGen.c

#
#   ecCompiler.o
#
DEPS_25 += $(CONFIG)/inc/me.h
DEPS_25 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecCompiler.o: \
    src/compiler/ecCompiler.c $(DEPS_25)
	@echo '   [Compile] $(CONFIG)/obj/ecCompiler.o'
	$(CC) -c -o $(CONFIG)/obj/ecCompiler.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecCompiler.c

#
#   ecLex.o
#
DEPS_26 += $(CONFIG)/inc/me.h
DEPS_26 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecLex.o: \
    src/compiler/ecLex.c $(DEPS_26)
	@echo '   [Compile] $(CONFIG)/obj/ecLex.o'
	$(CC) -c -o $(CONFIG)/obj/ecLex.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecLex.c

#
#   ecModuleWrite.o
#
DEPS_27 += $(CONFIG)/inc/me.h
DEPS_27 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecModuleWrite.o: \
    src/compiler/ecModuleWrite.c $(DEPS_27)
	@echo '   [Compile] $(CONFIG)/obj/ecModuleWrite.o'
	$(CC) -c -o $(CONFIG)/obj/ecModuleWrite.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecModuleWrite.c

#
#   ecParser.o
#
DEPS_28 += $(CONFIG)/inc/me.h
DEPS_28 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecParser.o: \
    src/compiler/ecParser.c $(DEPS_28)
	@echo '   [Compile] $(CONFIG)/obj/ecParser.o'
	$(CC) -c -o $(CONFIG)/obj/ecParser.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecParser.c

#
#   ecState.o
#
DEPS_29 += $(CONFIG)/inc/me.h
DEPS_29 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecState.o: \
    src/compiler/ecState.c $(DEPS_29)
	@echo '   [Compile] $(CONFIG)/obj/ecState.o'
	$(CC) -c -o $(CONFIG)/obj/ecState.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecState.c

#
#   dtoa.o
#
DEPS_30 += $(CONFIG)/inc/me.h
DEPS_30 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/dtoa.o: \
    src/core/src/dtoa.c $(DEPS_30)
	@echo '   [Compile] $(CONFIG)/obj/dtoa.o'
	$(CC) -c -o $(CONFIG)/obj/dtoa.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/dtoa.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsApp.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsApp.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsArray.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsArray.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsBlock.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsBlock.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsBoolean.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsBoolean.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsByteArray.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsByteArray.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsCache.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsCache.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsCmd.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsCmd.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsConfig.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsConfig.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsDate.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsDate.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsDebug.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsDebug.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsError.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsError.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsFile.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsFile.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsFileSystem.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsFileSystem.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsFrame.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsFrame.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsFunction.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsFunction.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsGC.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsGC.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsGlobal.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsGlobal.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsHttp.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsHttp.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsIterator.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsIterator.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsJSON.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsJSON.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsLocalCache.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsLocalCache.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsMath.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsMath.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsMemory.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsMemory.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsMprLog.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsMprLog.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsNamespace.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsNamespace.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsNull.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsNull.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsNumber.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsNumber.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsObject.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsObject.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsPath.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsPath.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsPot.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsPot.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsRegExp.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsRegExp.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsSocket.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsSocket.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsString.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsString.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsSystem.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsSystem.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsTimer.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsTimer.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsType.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsType.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsUri.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsUri.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsVoid.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsVoid.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsWebSocket.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsWebSocket.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsWorker.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsWorker.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsXML.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsXML.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsXMLList.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsXMLList.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsXMLLoader.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsXMLLoader.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsByteCode.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsByteCode.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsException.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsException.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsHelper.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsHelper.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsInterp.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsInterp.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsLoader.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsLoader.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsModule.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsModule.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsScope.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsScope.c

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
	$(CC) -c -o $(CONFIG)/obj/ejsService.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsService.c

#
#   libejs
#
DEPS_82 += $(CONFIG)/inc/mpr.h
DEPS_82 += $(CONFIG)/inc/me.h
DEPS_82 += $(CONFIG)/inc/osdep.h
DEPS_82 += $(CONFIG)/obj/mprLib.o
DEPS_82 += $(CONFIG)/bin/libmpr.a
DEPS_82 += $(CONFIG)/inc/pcre.h
DEPS_82 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_82 += $(CONFIG)/bin/libpcre.a
endif
DEPS_82 += $(CONFIG)/inc/http.h
DEPS_82 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_82 += $(CONFIG)/bin/libhttp.a
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

$(CONFIG)/bin/libejs.a: $(DEPS_82)
	@echo '      [Link] $(CONFIG)/bin/libejs.a'
	ar -cr $(CONFIG)/bin/libejs.a "$(CONFIG)/obj/ecAst.o" "$(CONFIG)/obj/ecCodeGen.o" "$(CONFIG)/obj/ecCompiler.o" "$(CONFIG)/obj/ecLex.o" "$(CONFIG)/obj/ecModuleWrite.o" "$(CONFIG)/obj/ecParser.o" "$(CONFIG)/obj/ecState.o" "$(CONFIG)/obj/dtoa.o" "$(CONFIG)/obj/ejsApp.o" "$(CONFIG)/obj/ejsArray.o" "$(CONFIG)/obj/ejsBlock.o" "$(CONFIG)/obj/ejsBoolean.o" "$(CONFIG)/obj/ejsByteArray.o" "$(CONFIG)/obj/ejsCache.o" "$(CONFIG)/obj/ejsCmd.o" "$(CONFIG)/obj/ejsConfig.o" "$(CONFIG)/obj/ejsDate.o" "$(CONFIG)/obj/ejsDebug.o" "$(CONFIG)/obj/ejsError.o" "$(CONFIG)/obj/ejsFile.o" "$(CONFIG)/obj/ejsFileSystem.o" "$(CONFIG)/obj/ejsFrame.o" "$(CONFIG)/obj/ejsFunction.o" "$(CONFIG)/obj/ejsGC.o" "$(CONFIG)/obj/ejsGlobal.o" "$(CONFIG)/obj/ejsHttp.o" "$(CONFIG)/obj/ejsIterator.o" "$(CONFIG)/obj/ejsJSON.o" "$(CONFIG)/obj/ejsLocalCache.o" "$(CONFIG)/obj/ejsMath.o" "$(CONFIG)/obj/ejsMemory.o" "$(CONFIG)/obj/ejsMprLog.o" "$(CONFIG)/obj/ejsNamespace.o" "$(CONFIG)/obj/ejsNull.o" "$(CONFIG)/obj/ejsNumber.o" "$(CONFIG)/obj/ejsObject.o" "$(CONFIG)/obj/ejsPath.o" "$(CONFIG)/obj/ejsPot.o" "$(CONFIG)/obj/ejsRegExp.o" "$(CONFIG)/obj/ejsSocket.o" "$(CONFIG)/obj/ejsString.o" "$(CONFIG)/obj/ejsSystem.o" "$(CONFIG)/obj/ejsTimer.o" "$(CONFIG)/obj/ejsType.o" "$(CONFIG)/obj/ejsUri.o" "$(CONFIG)/obj/ejsVoid.o" "$(CONFIG)/obj/ejsWebSocket.o" "$(CONFIG)/obj/ejsWorker.o" "$(CONFIG)/obj/ejsXML.o" "$(CONFIG)/obj/ejsXMLList.o" "$(CONFIG)/obj/ejsXMLLoader.o" "$(CONFIG)/obj/ejsByteCode.o" "$(CONFIG)/obj/ejsException.o" "$(CONFIG)/obj/ejsHelper.o" "$(CONFIG)/obj/ejsInterp.o" "$(CONFIG)/obj/ejsLoader.o" "$(CONFIG)/obj/ejsModule.o" "$(CONFIG)/obj/ejsScope.o" "$(CONFIG)/obj/ejsService.o"

#
#   ejs.o
#
DEPS_83 += $(CONFIG)/inc/me.h
DEPS_83 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejs.o: \
    src/cmd/ejs.c $(DEPS_83)
	@echo '   [Compile] $(CONFIG)/obj/ejs.o'
	$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/cmd/ejs.c

#
#   ejs
#
DEPS_84 += $(CONFIG)/inc/mpr.h
DEPS_84 += $(CONFIG)/inc/me.h
DEPS_84 += $(CONFIG)/inc/osdep.h
DEPS_84 += $(CONFIG)/obj/mprLib.o
DEPS_84 += $(CONFIG)/bin/libmpr.a
DEPS_84 += $(CONFIG)/inc/pcre.h
DEPS_84 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_84 += $(CONFIG)/bin/libpcre.a
endif
DEPS_84 += $(CONFIG)/inc/http.h
DEPS_84 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_84 += $(CONFIG)/bin/libhttp.a
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
DEPS_84 += $(CONFIG)/bin/libejs.a
DEPS_84 += $(CONFIG)/obj/ejs.o

LIBS_84 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_84 += -lhttp
endif
LIBS_84 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_84 += -lpcre
endif

$(CONFIG)/bin/ejs.out: $(DEPS_84)
	@echo '      [Link] $(CONFIG)/bin/ejs.out'
	$(CC) -o $(CONFIG)/bin/ejs.out $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejs.o" $(LIBPATHS_84) $(LIBS_84) $(LIBS_84) $(LIBS) -Wl,-r 

#
#   ejsc.o
#
DEPS_85 += $(CONFIG)/inc/me.h
DEPS_85 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejsc.o: \
    src/cmd/ejsc.c $(DEPS_85)
	@echo '   [Compile] $(CONFIG)/obj/ejsc.o'
	$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/cmd/ejsc.c

#
#   ejsc
#
DEPS_86 += $(CONFIG)/inc/mpr.h
DEPS_86 += $(CONFIG)/inc/me.h
DEPS_86 += $(CONFIG)/inc/osdep.h
DEPS_86 += $(CONFIG)/obj/mprLib.o
DEPS_86 += $(CONFIG)/bin/libmpr.a
DEPS_86 += $(CONFIG)/inc/pcre.h
DEPS_86 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_86 += $(CONFIG)/bin/libpcre.a
endif
DEPS_86 += $(CONFIG)/inc/http.h
DEPS_86 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_86 += $(CONFIG)/bin/libhttp.a
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
DEPS_86 += $(CONFIG)/bin/libejs.a
DEPS_86 += $(CONFIG)/obj/ejsc.o

LIBS_86 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_86 += -lhttp
endif
LIBS_86 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_86 += -lpcre
endif

$(CONFIG)/bin/ejsc.out: $(DEPS_86)
	@echo '      [Link] $(CONFIG)/bin/ejsc.out'
	$(CC) -o $(CONFIG)/bin/ejsc.out $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejsc.o" $(LIBPATHS_86) $(LIBS_86) $(LIBS_86) $(LIBS) -Wl,-r 

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
	$(CC) -c -o $(CONFIG)/obj/ejsmod.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-Isrc/cmd" src/cmd/ejsmod.c

#
#   doc.o
#
DEPS_89 += $(CONFIG)/inc/me.h
DEPS_89 += src/cmd/ejsmod.h

$(CONFIG)/obj/doc.o: \
    src/cmd/doc.c $(DEPS_89)
	@echo '   [Compile] $(CONFIG)/obj/doc.o'
	$(CC) -c -o $(CONFIG)/obj/doc.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-Isrc/cmd" src/cmd/doc.c

#
#   docFiles.o
#
DEPS_90 += $(CONFIG)/inc/me.h
DEPS_90 += src/cmd/ejsmod.h

$(CONFIG)/obj/docFiles.o: \
    src/cmd/docFiles.c $(DEPS_90)
	@echo '   [Compile] $(CONFIG)/obj/docFiles.o'
	$(CC) -c -o $(CONFIG)/obj/docFiles.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-Isrc/cmd" src/cmd/docFiles.c

#
#   listing.o
#
DEPS_91 += $(CONFIG)/inc/me.h
DEPS_91 += src/cmd/ejsmod.h
DEPS_91 += $(CONFIG)/inc/ejsByteCodeTable.h

$(CONFIG)/obj/listing.o: \
    src/cmd/listing.c $(DEPS_91)
	@echo '   [Compile] $(CONFIG)/obj/listing.o'
	$(CC) -c -o $(CONFIG)/obj/listing.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-Isrc/cmd" src/cmd/listing.c

#
#   slotGen.o
#
DEPS_92 += $(CONFIG)/inc/me.h
DEPS_92 += src/cmd/ejsmod.h

$(CONFIG)/obj/slotGen.o: \
    src/cmd/slotGen.c $(DEPS_92)
	@echo '   [Compile] $(CONFIG)/obj/slotGen.o'
	$(CC) -c -o $(CONFIG)/obj/slotGen.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-Isrc/cmd" src/cmd/slotGen.c

#
#   ejsmod
#
DEPS_93 += $(CONFIG)/inc/mpr.h
DEPS_93 += $(CONFIG)/inc/me.h
DEPS_93 += $(CONFIG)/inc/osdep.h
DEPS_93 += $(CONFIG)/obj/mprLib.o
DEPS_93 += $(CONFIG)/bin/libmpr.a
DEPS_93 += $(CONFIG)/inc/pcre.h
DEPS_93 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_93 += $(CONFIG)/bin/libpcre.a
endif
DEPS_93 += $(CONFIG)/inc/http.h
DEPS_93 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_93 += $(CONFIG)/bin/libhttp.a
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
DEPS_93 += $(CONFIG)/bin/libejs.a
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

$(CONFIG)/bin/ejsmod.out: $(DEPS_93)
	@echo '      [Link] $(CONFIG)/bin/ejsmod.out'
	$(CC) -o $(CONFIG)/bin/ejsmod.out $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejsmod.o" "$(CONFIG)/obj/doc.o" "$(CONFIG)/obj/docFiles.o" "$(CONFIG)/obj/listing.o" "$(CONFIG)/obj/slotGen.o" $(LIBPATHS_93) $(LIBS_93) $(LIBS_93) $(LIBS) -Wl,-r 

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
DEPS_94 += $(CONFIG)/bin/libmpr.a
DEPS_94 += $(CONFIG)/inc/pcre.h
DEPS_94 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_94 += $(CONFIG)/bin/libpcre.a
endif
DEPS_94 += $(CONFIG)/inc/http.h
DEPS_94 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_94 += $(CONFIG)/bin/libhttp.a
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
DEPS_94 += $(CONFIG)/bin/libejs.a
DEPS_94 += $(CONFIG)/obj/ejsc.o
DEPS_94 += $(CONFIG)/bin/ejsc.out
DEPS_94 += src/cmd/ejsmod.h
DEPS_94 += $(CONFIG)/obj/ejsmod.o
DEPS_94 += $(CONFIG)/obj/doc.o
DEPS_94 += $(CONFIG)/obj/docFiles.o
DEPS_94 += $(CONFIG)/obj/listing.o
DEPS_94 += $(CONFIG)/obj/slotGen.o
DEPS_94 += $(CONFIG)/bin/ejsmod.out

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
DEPS_95 += $(CONFIG)/bin/libmpr.a
DEPS_95 += $(CONFIG)/inc/pcre.h
DEPS_95 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_95 += $(CONFIG)/bin/libpcre.a
endif
DEPS_95 += $(CONFIG)/inc/http.h
DEPS_95 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_95 += $(CONFIG)/bin/libhttp.a
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
DEPS_95 += $(CONFIG)/bin/libejs.a
DEPS_95 += $(CONFIG)/obj/ejsc.o
DEPS_95 += $(CONFIG)/bin/ejsc.out
DEPS_95 += src/cmd/ejsmod.h
DEPS_95 += $(CONFIG)/obj/ejsmod.o
DEPS_95 += $(CONFIG)/obj/doc.o
DEPS_95 += $(CONFIG)/obj/docFiles.o
DEPS_95 += $(CONFIG)/obj/listing.o
DEPS_95 += $(CONFIG)/obj/slotGen.o
DEPS_95 += $(CONFIG)/bin/ejsmod.out
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
DEPS_96 += $(CONFIG)/bin/libmpr.a
DEPS_96 += $(CONFIG)/inc/pcre.h
DEPS_96 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_96 += $(CONFIG)/bin/libpcre.a
endif
DEPS_96 += $(CONFIG)/inc/http.h
DEPS_96 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_96 += $(CONFIG)/bin/libhttp.a
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
DEPS_96 += $(CONFIG)/bin/libejs.a
DEPS_96 += $(CONFIG)/obj/ejsc.o
DEPS_96 += $(CONFIG)/bin/ejsc.out
DEPS_96 += src/cmd/ejsmod.h
DEPS_96 += $(CONFIG)/obj/ejsmod.o
DEPS_96 += $(CONFIG)/obj/doc.o
DEPS_96 += $(CONFIG)/obj/docFiles.o
DEPS_96 += $(CONFIG)/obj/listing.o
DEPS_96 += $(CONFIG)/obj/slotGen.o
DEPS_96 += $(CONFIG)/bin/ejsmod.out
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
	$(CC) -c -o $(CONFIG)/obj/ejsrun.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/cmd/ejsrun.c

#
#   ejsrun
#
DEPS_98 += $(CONFIG)/inc/mpr.h
DEPS_98 += $(CONFIG)/inc/me.h
DEPS_98 += $(CONFIG)/inc/osdep.h
DEPS_98 += $(CONFIG)/obj/mprLib.o
DEPS_98 += $(CONFIG)/bin/libmpr.a
DEPS_98 += $(CONFIG)/inc/pcre.h
DEPS_98 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_98 += $(CONFIG)/bin/libpcre.a
endif
DEPS_98 += $(CONFIG)/inc/http.h
DEPS_98 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_98 += $(CONFIG)/bin/libhttp.a
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
DEPS_98 += $(CONFIG)/bin/libejs.a
DEPS_98 += $(CONFIG)/obj/ejsrun.o

LIBS_98 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_98 += -lhttp
endif
LIBS_98 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_98 += -lpcre
endif

$(CONFIG)/bin/ejsrun.out: $(DEPS_98)
	@echo '      [Link] $(CONFIG)/bin/ejsrun.out'
	$(CC) -o $(CONFIG)/bin/ejsrun.out $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejsrun.o" $(LIBPATHS_98) $(LIBS_98) $(LIBS_98) $(LIBS) -Wl,-r 


#
#   est.h
#
$(CONFIG)/inc/est.h: $(DEPS_99)
	@echo '      [Copy] $(CONFIG)/inc/est.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/est/est.h $(CONFIG)/inc/est.h

#
#   estLib.o
#
DEPS_100 += $(CONFIG)/inc/me.h
DEPS_100 += $(CONFIG)/inc/est.h
DEPS_100 += $(CONFIG)/inc/osdep.h

$(CONFIG)/obj/estLib.o: \
    src/paks/est/estLib.c $(DEPS_100)
	@echo '   [Compile] $(CONFIG)/obj/estLib.o'
	$(CC) -c -o $(CONFIG)/obj/estLib.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/paks/est/estLib.c

ifeq ($(ME_COM_EST),1)
#
#   libest
#
DEPS_101 += $(CONFIG)/inc/est.h
DEPS_101 += $(CONFIG)/inc/me.h
DEPS_101 += $(CONFIG)/inc/osdep.h
DEPS_101 += $(CONFIG)/obj/estLib.o

$(CONFIG)/bin/libest.a: $(DEPS_101)
	@echo '      [Link] $(CONFIG)/bin/libest.a'
	ar -cr $(CONFIG)/bin/libest.a "$(CONFIG)/obj/estLib.o"
endif

#
#   mprSsl.o
#
DEPS_102 += $(CONFIG)/inc/me.h
DEPS_102 += $(CONFIG)/inc/mpr.h
DEPS_102 += $(CONFIG)/inc/est.h

$(CONFIG)/obj/mprSsl.o: \
    src/paks/mpr/mprSsl.c $(DEPS_102)
	@echo '   [Compile] $(CONFIG)/obj/mprSsl.o'
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-I$(ME_COM_MATRIXSSL_PATH)" "-I$(ME_COM_MATRIXSSL_PATH)/matrixssl" "-I$(ME_COM_NANOSSL_PATH)/src" "-I$(ME_COM_OPENSSL_PATH)/include" src/paks/mpr/mprSsl.c

#
#   libmprssl
#
DEPS_103 += $(CONFIG)/inc/mpr.h
DEPS_103 += $(CONFIG)/inc/me.h
DEPS_103 += $(CONFIG)/inc/osdep.h
DEPS_103 += $(CONFIG)/obj/mprLib.o
DEPS_103 += $(CONFIG)/bin/libmpr.a
DEPS_103 += $(CONFIG)/inc/est.h
DEPS_103 += $(CONFIG)/obj/estLib.o
ifeq ($(ME_COM_EST),1)
    DEPS_103 += $(CONFIG)/bin/libest.a
endif
DEPS_103 += $(CONFIG)/obj/mprSsl.o

$(CONFIG)/bin/libmprssl.a: $(DEPS_103)
	@echo '      [Link] $(CONFIG)/bin/libmprssl.a'
	ar -cr $(CONFIG)/bin/libmprssl.a "$(CONFIG)/obj/mprSsl.o"

#
#   http.o
#
DEPS_104 += $(CONFIG)/inc/me.h
DEPS_104 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/http.o: \
    src/paks/http/http.c $(DEPS_104)
	@echo '   [Compile] $(CONFIG)/obj/http.o'
	$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-I$(ME_COM_MATRIXSSL_PATH)" "-I$(ME_COM_MATRIXSSL_PATH)/matrixssl" "-I$(ME_COM_NANOSSL_PATH)/src" "-I$(ME_COM_OPENSSL_PATH)/include" src/paks/http/http.c

ifeq ($(ME_COM_HTTP),1)
#
#   httpcmd
#
DEPS_105 += $(CONFIG)/inc/mpr.h
DEPS_105 += $(CONFIG)/inc/me.h
DEPS_105 += $(CONFIG)/inc/osdep.h
DEPS_105 += $(CONFIG)/obj/mprLib.o
DEPS_105 += $(CONFIG)/bin/libmpr.a
DEPS_105 += $(CONFIG)/inc/pcre.h
DEPS_105 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_105 += $(CONFIG)/bin/libpcre.a
endif
DEPS_105 += $(CONFIG)/inc/http.h
DEPS_105 += $(CONFIG)/obj/httpLib.o
DEPS_105 += $(CONFIG)/bin/libhttp.a
DEPS_105 += $(CONFIG)/inc/est.h
DEPS_105 += $(CONFIG)/obj/estLib.o
ifeq ($(ME_COM_EST),1)
    DEPS_105 += $(CONFIG)/bin/libest.a
endif
DEPS_105 += $(CONFIG)/obj/mprSsl.o
DEPS_105 += $(CONFIG)/bin/libmprssl.a
DEPS_105 += $(CONFIG)/obj/http.o

LIBS_105 += -lhttp
LIBS_105 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_105 += -lpcre
endif
LIBS_105 += -lmprssl
ifeq ($(ME_COM_EST),1)
    LIBS_105 += -lest
endif
ifeq ($(ME_COM_MATRIXSSL),1)
    LIBS_105 += -lmatrixssl
    LIBPATHS_105 += -L$(ME_COM_MATRIXSSL_PATH)
endif
ifeq ($(ME_COM_NANOSSL),1)
    LIBS_105 += -lssls
    LIBPATHS_105 += -L$(ME_COM_NANOSSL_PATH)/bin
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_105 += -lssl
    LIBPATHS_105 += -L$(ME_COM_OPENSSL_PATH)
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_105 += -lcrypto
    LIBPATHS_105 += -L$(ME_COM_OPENSSL_PATH)
endif

$(CONFIG)/bin/http.out: $(DEPS_105)
	@echo '      [Link] $(CONFIG)/bin/http.out'
	$(CC) -o $(CONFIG)/bin/http.out $(LDFLAGS) $(LIBPATHS)    "$(CONFIG)/obj/http.o" $(LIBPATHS_105) $(LIBS_105) $(LIBS_105) $(LIBS) -Wl,-r 
endif

#
#   sqlite3.h
#
$(CONFIG)/inc/sqlite3.h: $(DEPS_106)
	@echo '      [Copy] $(CONFIG)/inc/sqlite3.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/sqlite/sqlite3.h $(CONFIG)/inc/sqlite3.h

#
#   sqlite3.o
#
DEPS_107 += $(CONFIG)/inc/me.h
DEPS_107 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite3.o: \
    src/paks/sqlite/sqlite3.c $(DEPS_107)
	@echo '   [Compile] $(CONFIG)/obj/sqlite3.o'
	$(CC) -c -o $(CONFIG)/obj/sqlite3.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/paks/sqlite/sqlite3.c

ifeq ($(ME_COM_SQLITE),1)
#
#   libsql
#
DEPS_108 += $(CONFIG)/inc/sqlite3.h
DEPS_108 += $(CONFIG)/inc/me.h
DEPS_108 += $(CONFIG)/obj/sqlite3.o

$(CONFIG)/bin/libsql.a: $(DEPS_108)
	@echo '      [Link] $(CONFIG)/bin/libsql.a'
	ar -cr $(CONFIG)/bin/libsql.a "$(CONFIG)/obj/sqlite3.o"
endif

#
#   ejsSqlite.o
#
DEPS_109 += $(CONFIG)/inc/me.h
DEPS_109 += $(CONFIG)/inc/mpr.h
DEPS_109 += $(CONFIG)/inc/http.h
DEPS_109 += $(CONFIG)/inc/ejsByteCode.h
DEPS_109 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_109 += $(CONFIG)/inc/ejs.slots.h
DEPS_109 += $(CONFIG)/inc/ejsCustomize.h
DEPS_109 += $(CONFIG)/inc/ejs.h
DEPS_109 += $(CONFIG)/inc/ejs.db.sqlite.slots.h

$(CONFIG)/obj/ejsSqlite.o: \
    src/ejs.db.sqlite/ejsSqlite.c $(DEPS_109)
	@echo '   [Compile] $(CONFIG)/obj/ejsSqlite.o'
	$(CC) -c -o $(CONFIG)/obj/ejsSqlite.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-Isrc/cmd" src/ejs.db.sqlite/ejsSqlite.c

#
#   libejs.db.sqlite
#
DEPS_110 += $(CONFIG)/inc/mpr.h
DEPS_110 += $(CONFIG)/inc/me.h
DEPS_110 += $(CONFIG)/inc/osdep.h
DEPS_110 += $(CONFIG)/obj/mprLib.o
DEPS_110 += $(CONFIG)/bin/libmpr.a
DEPS_110 += $(CONFIG)/inc/pcre.h
DEPS_110 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_110 += $(CONFIG)/bin/libpcre.a
endif
DEPS_110 += $(CONFIG)/inc/http.h
DEPS_110 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_110 += $(CONFIG)/bin/libhttp.a
endif
DEPS_110 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_110 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_110 += $(CONFIG)/inc/ejs.slots.h
DEPS_110 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_110 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_110 += $(CONFIG)/inc/ejsByteCode.h
DEPS_110 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_110 += $(CONFIG)/inc/ejsCustomize.h
DEPS_110 += $(CONFIG)/inc/ejs.h
DEPS_110 += $(CONFIG)/inc/ejsCompiler.h
DEPS_110 += $(CONFIG)/obj/ecAst.o
DEPS_110 += $(CONFIG)/obj/ecCodeGen.o
DEPS_110 += $(CONFIG)/obj/ecCompiler.o
DEPS_110 += $(CONFIG)/obj/ecLex.o
DEPS_110 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_110 += $(CONFIG)/obj/ecParser.o
DEPS_110 += $(CONFIG)/obj/ecState.o
DEPS_110 += $(CONFIG)/obj/dtoa.o
DEPS_110 += $(CONFIG)/obj/ejsApp.o
DEPS_110 += $(CONFIG)/obj/ejsArray.o
DEPS_110 += $(CONFIG)/obj/ejsBlock.o
DEPS_110 += $(CONFIG)/obj/ejsBoolean.o
DEPS_110 += $(CONFIG)/obj/ejsByteArray.o
DEPS_110 += $(CONFIG)/obj/ejsCache.o
DEPS_110 += $(CONFIG)/obj/ejsCmd.o
DEPS_110 += $(CONFIG)/obj/ejsConfig.o
DEPS_110 += $(CONFIG)/obj/ejsDate.o
DEPS_110 += $(CONFIG)/obj/ejsDebug.o
DEPS_110 += $(CONFIG)/obj/ejsError.o
DEPS_110 += $(CONFIG)/obj/ejsFile.o
DEPS_110 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_110 += $(CONFIG)/obj/ejsFrame.o
DEPS_110 += $(CONFIG)/obj/ejsFunction.o
DEPS_110 += $(CONFIG)/obj/ejsGC.o
DEPS_110 += $(CONFIG)/obj/ejsGlobal.o
DEPS_110 += $(CONFIG)/obj/ejsHttp.o
DEPS_110 += $(CONFIG)/obj/ejsIterator.o
DEPS_110 += $(CONFIG)/obj/ejsJSON.o
DEPS_110 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_110 += $(CONFIG)/obj/ejsMath.o
DEPS_110 += $(CONFIG)/obj/ejsMemory.o
DEPS_110 += $(CONFIG)/obj/ejsMprLog.o
DEPS_110 += $(CONFIG)/obj/ejsNamespace.o
DEPS_110 += $(CONFIG)/obj/ejsNull.o
DEPS_110 += $(CONFIG)/obj/ejsNumber.o
DEPS_110 += $(CONFIG)/obj/ejsObject.o
DEPS_110 += $(CONFIG)/obj/ejsPath.o
DEPS_110 += $(CONFIG)/obj/ejsPot.o
DEPS_110 += $(CONFIG)/obj/ejsRegExp.o
DEPS_110 += $(CONFIG)/obj/ejsSocket.o
DEPS_110 += $(CONFIG)/obj/ejsString.o
DEPS_110 += $(CONFIG)/obj/ejsSystem.o
DEPS_110 += $(CONFIG)/obj/ejsTimer.o
DEPS_110 += $(CONFIG)/obj/ejsType.o
DEPS_110 += $(CONFIG)/obj/ejsUri.o
DEPS_110 += $(CONFIG)/obj/ejsVoid.o
DEPS_110 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_110 += $(CONFIG)/obj/ejsWorker.o
DEPS_110 += $(CONFIG)/obj/ejsXML.o
DEPS_110 += $(CONFIG)/obj/ejsXMLList.o
DEPS_110 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_110 += $(CONFIG)/obj/ejsByteCode.o
DEPS_110 += $(CONFIG)/obj/ejsException.o
DEPS_110 += $(CONFIG)/obj/ejsHelper.o
DEPS_110 += $(CONFIG)/obj/ejsInterp.o
DEPS_110 += $(CONFIG)/obj/ejsLoader.o
DEPS_110 += $(CONFIG)/obj/ejsModule.o
DEPS_110 += $(CONFIG)/obj/ejsScope.o
DEPS_110 += $(CONFIG)/obj/ejsService.o
DEPS_110 += $(CONFIG)/bin/libejs.a
DEPS_110 += $(CONFIG)/obj/ejsc.o
DEPS_110 += $(CONFIG)/bin/ejsc.out
DEPS_110 += src/cmd/ejsmod.h
DEPS_110 += $(CONFIG)/obj/ejsmod.o
DEPS_110 += $(CONFIG)/obj/doc.o
DEPS_110 += $(CONFIG)/obj/docFiles.o
DEPS_110 += $(CONFIG)/obj/listing.o
DEPS_110 += $(CONFIG)/obj/slotGen.o
DEPS_110 += $(CONFIG)/bin/ejsmod.out
DEPS_110 += $(CONFIG)/bin/ejs.mod
DEPS_110 += $(CONFIG)/bin/ejs.db.sqlite.mod
DEPS_110 += $(CONFIG)/inc/sqlite3.h
DEPS_110 += $(CONFIG)/obj/sqlite3.o
ifeq ($(ME_COM_SQLITE),1)
    DEPS_110 += $(CONFIG)/bin/libsql.a
endif
DEPS_110 += $(CONFIG)/obj/ejsSqlite.o

$(CONFIG)/bin/libejs.db.sqlite.a: $(DEPS_110)
	@echo '      [Link] $(CONFIG)/bin/libejs.db.sqlite.a'
	ar -cr $(CONFIG)/bin/libejs.db.sqlite.a "$(CONFIG)/obj/ejsSqlite.o"

#
#   ejsWeb.h
#
$(CONFIG)/inc/ejsWeb.h: $(DEPS_111)
	@echo '      [Copy] $(CONFIG)/inc/ejsWeb.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejs.web/ejsWeb.h $(CONFIG)/inc/ejsWeb.h

#
#   ejsHttpServer.o
#
DEPS_112 += $(CONFIG)/inc/me.h
DEPS_112 += $(CONFIG)/inc/mpr.h
DEPS_112 += $(CONFIG)/inc/http.h
DEPS_112 += $(CONFIG)/inc/ejsByteCode.h
DEPS_112 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_112 += $(CONFIG)/inc/ejs.slots.h
DEPS_112 += $(CONFIG)/inc/ejsCustomize.h
DEPS_112 += $(CONFIG)/inc/ejs.h
DEPS_112 += $(CONFIG)/inc/ejsCompiler.h
DEPS_112 += $(CONFIG)/inc/ejsWeb.h
DEPS_112 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsHttpServer.o: \
    src/ejs.web/ejsHttpServer.c $(DEPS_112)
	@echo '   [Compile] $(CONFIG)/obj/ejsHttpServer.o'
	$(CC) -c -o $(CONFIG)/obj/ejsHttpServer.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-Isrc/cmd" src/ejs.web/ejsHttpServer.c

#
#   ejsRequest.o
#
DEPS_113 += $(CONFIG)/inc/me.h
DEPS_113 += $(CONFIG)/inc/mpr.h
DEPS_113 += $(CONFIG)/inc/http.h
DEPS_113 += $(CONFIG)/inc/ejsByteCode.h
DEPS_113 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_113 += $(CONFIG)/inc/ejs.slots.h
DEPS_113 += $(CONFIG)/inc/ejsCustomize.h
DEPS_113 += $(CONFIG)/inc/ejs.h
DEPS_113 += $(CONFIG)/inc/ejsCompiler.h
DEPS_113 += $(CONFIG)/inc/ejsWeb.h
DEPS_113 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsRequest.o: \
    src/ejs.web/ejsRequest.c $(DEPS_113)
	@echo '   [Compile] $(CONFIG)/obj/ejsRequest.o'
	$(CC) -c -o $(CONFIG)/obj/ejsRequest.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-Isrc/cmd" src/ejs.web/ejsRequest.c

#
#   ejsSession.o
#
DEPS_114 += $(CONFIG)/inc/me.h
DEPS_114 += $(CONFIG)/inc/mpr.h
DEPS_114 += $(CONFIG)/inc/http.h
DEPS_114 += $(CONFIG)/inc/ejsByteCode.h
DEPS_114 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_114 += $(CONFIG)/inc/ejs.slots.h
DEPS_114 += $(CONFIG)/inc/ejsCustomize.h
DEPS_114 += $(CONFIG)/inc/ejs.h
DEPS_114 += $(CONFIG)/inc/ejsWeb.h

$(CONFIG)/obj/ejsSession.o: \
    src/ejs.web/ejsSession.c $(DEPS_114)
	@echo '   [Compile] $(CONFIG)/obj/ejsSession.o'
	$(CC) -c -o $(CONFIG)/obj/ejsSession.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-Isrc/cmd" src/ejs.web/ejsSession.c

#
#   ejsWeb.o
#
DEPS_115 += $(CONFIG)/inc/me.h
DEPS_115 += $(CONFIG)/inc/mpr.h
DEPS_115 += $(CONFIG)/inc/http.h
DEPS_115 += $(CONFIG)/inc/ejsByteCode.h
DEPS_115 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_115 += $(CONFIG)/inc/ejs.slots.h
DEPS_115 += $(CONFIG)/inc/ejsCustomize.h
DEPS_115 += $(CONFIG)/inc/ejs.h
DEPS_115 += $(CONFIG)/inc/ejsCompiler.h
DEPS_115 += $(CONFIG)/inc/ejsWeb.h
DEPS_115 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsWeb.o: \
    src/ejs.web/ejsWeb.c $(DEPS_115)
	@echo '   [Compile] $(CONFIG)/obj/ejsWeb.o'
	$(CC) -c -o $(CONFIG)/obj/ejsWeb.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-Isrc/cmd" src/ejs.web/ejsWeb.c

#
#   libejs.web
#
DEPS_116 += $(CONFIG)/inc/mpr.h
DEPS_116 += $(CONFIG)/inc/me.h
DEPS_116 += $(CONFIG)/inc/osdep.h
DEPS_116 += $(CONFIG)/obj/mprLib.o
DEPS_116 += $(CONFIG)/bin/libmpr.a
DEPS_116 += $(CONFIG)/inc/pcre.h
DEPS_116 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_116 += $(CONFIG)/bin/libpcre.a
endif
DEPS_116 += $(CONFIG)/inc/http.h
DEPS_116 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_116 += $(CONFIG)/bin/libhttp.a
endif
DEPS_116 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_116 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_116 += $(CONFIG)/inc/ejs.slots.h
DEPS_116 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_116 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_116 += $(CONFIG)/inc/ejsByteCode.h
DEPS_116 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_116 += $(CONFIG)/inc/ejsCustomize.h
DEPS_116 += $(CONFIG)/inc/ejs.h
DEPS_116 += $(CONFIG)/inc/ejsCompiler.h
DEPS_116 += $(CONFIG)/obj/ecAst.o
DEPS_116 += $(CONFIG)/obj/ecCodeGen.o
DEPS_116 += $(CONFIG)/obj/ecCompiler.o
DEPS_116 += $(CONFIG)/obj/ecLex.o
DEPS_116 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_116 += $(CONFIG)/obj/ecParser.o
DEPS_116 += $(CONFIG)/obj/ecState.o
DEPS_116 += $(CONFIG)/obj/dtoa.o
DEPS_116 += $(CONFIG)/obj/ejsApp.o
DEPS_116 += $(CONFIG)/obj/ejsArray.o
DEPS_116 += $(CONFIG)/obj/ejsBlock.o
DEPS_116 += $(CONFIG)/obj/ejsBoolean.o
DEPS_116 += $(CONFIG)/obj/ejsByteArray.o
DEPS_116 += $(CONFIG)/obj/ejsCache.o
DEPS_116 += $(CONFIG)/obj/ejsCmd.o
DEPS_116 += $(CONFIG)/obj/ejsConfig.o
DEPS_116 += $(CONFIG)/obj/ejsDate.o
DEPS_116 += $(CONFIG)/obj/ejsDebug.o
DEPS_116 += $(CONFIG)/obj/ejsError.o
DEPS_116 += $(CONFIG)/obj/ejsFile.o
DEPS_116 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_116 += $(CONFIG)/obj/ejsFrame.o
DEPS_116 += $(CONFIG)/obj/ejsFunction.o
DEPS_116 += $(CONFIG)/obj/ejsGC.o
DEPS_116 += $(CONFIG)/obj/ejsGlobal.o
DEPS_116 += $(CONFIG)/obj/ejsHttp.o
DEPS_116 += $(CONFIG)/obj/ejsIterator.o
DEPS_116 += $(CONFIG)/obj/ejsJSON.o
DEPS_116 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_116 += $(CONFIG)/obj/ejsMath.o
DEPS_116 += $(CONFIG)/obj/ejsMemory.o
DEPS_116 += $(CONFIG)/obj/ejsMprLog.o
DEPS_116 += $(CONFIG)/obj/ejsNamespace.o
DEPS_116 += $(CONFIG)/obj/ejsNull.o
DEPS_116 += $(CONFIG)/obj/ejsNumber.o
DEPS_116 += $(CONFIG)/obj/ejsObject.o
DEPS_116 += $(CONFIG)/obj/ejsPath.o
DEPS_116 += $(CONFIG)/obj/ejsPot.o
DEPS_116 += $(CONFIG)/obj/ejsRegExp.o
DEPS_116 += $(CONFIG)/obj/ejsSocket.o
DEPS_116 += $(CONFIG)/obj/ejsString.o
DEPS_116 += $(CONFIG)/obj/ejsSystem.o
DEPS_116 += $(CONFIG)/obj/ejsTimer.o
DEPS_116 += $(CONFIG)/obj/ejsType.o
DEPS_116 += $(CONFIG)/obj/ejsUri.o
DEPS_116 += $(CONFIG)/obj/ejsVoid.o
DEPS_116 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_116 += $(CONFIG)/obj/ejsWorker.o
DEPS_116 += $(CONFIG)/obj/ejsXML.o
DEPS_116 += $(CONFIG)/obj/ejsXMLList.o
DEPS_116 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_116 += $(CONFIG)/obj/ejsByteCode.o
DEPS_116 += $(CONFIG)/obj/ejsException.o
DEPS_116 += $(CONFIG)/obj/ejsHelper.o
DEPS_116 += $(CONFIG)/obj/ejsInterp.o
DEPS_116 += $(CONFIG)/obj/ejsLoader.o
DEPS_116 += $(CONFIG)/obj/ejsModule.o
DEPS_116 += $(CONFIG)/obj/ejsScope.o
DEPS_116 += $(CONFIG)/obj/ejsService.o
DEPS_116 += $(CONFIG)/bin/libejs.a
DEPS_116 += $(CONFIG)/obj/ejsc.o
DEPS_116 += $(CONFIG)/bin/ejsc.out
DEPS_116 += src/cmd/ejsmod.h
DEPS_116 += $(CONFIG)/obj/ejsmod.o
DEPS_116 += $(CONFIG)/obj/doc.o
DEPS_116 += $(CONFIG)/obj/docFiles.o
DEPS_116 += $(CONFIG)/obj/listing.o
DEPS_116 += $(CONFIG)/obj/slotGen.o
DEPS_116 += $(CONFIG)/bin/ejsmod.out
DEPS_116 += $(CONFIG)/bin/ejs.mod
DEPS_116 += $(CONFIG)/inc/ejsWeb.h
DEPS_116 += $(CONFIG)/obj/ejsHttpServer.o
DEPS_116 += $(CONFIG)/obj/ejsRequest.o
DEPS_116 += $(CONFIG)/obj/ejsSession.o
DEPS_116 += $(CONFIG)/obj/ejsWeb.o

$(CONFIG)/bin/libejs.web.a: $(DEPS_116)
	@echo '      [Link] $(CONFIG)/bin/libejs.web.a'
	ar -cr $(CONFIG)/bin/libejs.web.a "$(CONFIG)/obj/ejsHttpServer.o" "$(CONFIG)/obj/ejsRequest.o" "$(CONFIG)/obj/ejsSession.o" "$(CONFIG)/obj/ejsWeb.o"

#
#   zlib.h
#
$(CONFIG)/inc/zlib.h: $(DEPS_117)
	@echo '      [Copy] $(CONFIG)/inc/zlib.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/zlib/zlib.h $(CONFIG)/inc/zlib.h

#
#   zlib.o
#
DEPS_118 += $(CONFIG)/inc/me.h
DEPS_118 += $(CONFIG)/inc/zlib.h

$(CONFIG)/obj/zlib.o: \
    src/paks/zlib/zlib.c $(DEPS_118)
	@echo '   [Compile] $(CONFIG)/obj/zlib.o'
	$(CC) -c -o $(CONFIG)/obj/zlib.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/paks/zlib/zlib.c

ifeq ($(ME_COM_ZLIB),1)
#
#   libzlib
#
DEPS_119 += $(CONFIG)/inc/zlib.h
DEPS_119 += $(CONFIG)/inc/me.h
DEPS_119 += $(CONFIG)/obj/zlib.o

$(CONFIG)/bin/libzlib.a: $(DEPS_119)
	@echo '      [Link] $(CONFIG)/bin/libzlib.a'
	ar -cr $(CONFIG)/bin/libzlib.a "$(CONFIG)/obj/zlib.o"
endif

#
#   ejsZlib.o
#
DEPS_120 += $(CONFIG)/inc/me.h
DEPS_120 += $(CONFIG)/inc/mpr.h
DEPS_120 += $(CONFIG)/inc/http.h
DEPS_120 += $(CONFIG)/inc/ejsByteCode.h
DEPS_120 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_120 += $(CONFIG)/inc/ejs.slots.h
DEPS_120 += $(CONFIG)/inc/ejsCustomize.h
DEPS_120 += $(CONFIG)/inc/ejs.h
DEPS_120 += $(CONFIG)/inc/zlib.h
DEPS_120 += $(CONFIG)/inc/ejs.zlib.slots.h

$(CONFIG)/obj/ejsZlib.o: \
    src/ejs.zlib/ejsZlib.c $(DEPS_120)
	@echo '   [Compile] $(CONFIG)/obj/ejsZlib.o'
	$(CC) -c -o $(CONFIG)/obj/ejsZlib.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-Isrc/cmd" src/ejs.zlib/ejsZlib.c

#
#   libejs.zlib
#
DEPS_121 += $(CONFIG)/inc/mpr.h
DEPS_121 += $(CONFIG)/inc/me.h
DEPS_121 += $(CONFIG)/inc/osdep.h
DEPS_121 += $(CONFIG)/obj/mprLib.o
DEPS_121 += $(CONFIG)/bin/libmpr.a
DEPS_121 += $(CONFIG)/inc/pcre.h
DEPS_121 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_121 += $(CONFIG)/bin/libpcre.a
endif
DEPS_121 += $(CONFIG)/inc/http.h
DEPS_121 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_121 += $(CONFIG)/bin/libhttp.a
endif
DEPS_121 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_121 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_121 += $(CONFIG)/inc/ejs.slots.h
DEPS_121 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_121 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_121 += $(CONFIG)/inc/ejsByteCode.h
DEPS_121 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_121 += $(CONFIG)/inc/ejsCustomize.h
DEPS_121 += $(CONFIG)/inc/ejs.h
DEPS_121 += $(CONFIG)/inc/ejsCompiler.h
DEPS_121 += $(CONFIG)/obj/ecAst.o
DEPS_121 += $(CONFIG)/obj/ecCodeGen.o
DEPS_121 += $(CONFIG)/obj/ecCompiler.o
DEPS_121 += $(CONFIG)/obj/ecLex.o
DEPS_121 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_121 += $(CONFIG)/obj/ecParser.o
DEPS_121 += $(CONFIG)/obj/ecState.o
DEPS_121 += $(CONFIG)/obj/dtoa.o
DEPS_121 += $(CONFIG)/obj/ejsApp.o
DEPS_121 += $(CONFIG)/obj/ejsArray.o
DEPS_121 += $(CONFIG)/obj/ejsBlock.o
DEPS_121 += $(CONFIG)/obj/ejsBoolean.o
DEPS_121 += $(CONFIG)/obj/ejsByteArray.o
DEPS_121 += $(CONFIG)/obj/ejsCache.o
DEPS_121 += $(CONFIG)/obj/ejsCmd.o
DEPS_121 += $(CONFIG)/obj/ejsConfig.o
DEPS_121 += $(CONFIG)/obj/ejsDate.o
DEPS_121 += $(CONFIG)/obj/ejsDebug.o
DEPS_121 += $(CONFIG)/obj/ejsError.o
DEPS_121 += $(CONFIG)/obj/ejsFile.o
DEPS_121 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_121 += $(CONFIG)/obj/ejsFrame.o
DEPS_121 += $(CONFIG)/obj/ejsFunction.o
DEPS_121 += $(CONFIG)/obj/ejsGC.o
DEPS_121 += $(CONFIG)/obj/ejsGlobal.o
DEPS_121 += $(CONFIG)/obj/ejsHttp.o
DEPS_121 += $(CONFIG)/obj/ejsIterator.o
DEPS_121 += $(CONFIG)/obj/ejsJSON.o
DEPS_121 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_121 += $(CONFIG)/obj/ejsMath.o
DEPS_121 += $(CONFIG)/obj/ejsMemory.o
DEPS_121 += $(CONFIG)/obj/ejsMprLog.o
DEPS_121 += $(CONFIG)/obj/ejsNamespace.o
DEPS_121 += $(CONFIG)/obj/ejsNull.o
DEPS_121 += $(CONFIG)/obj/ejsNumber.o
DEPS_121 += $(CONFIG)/obj/ejsObject.o
DEPS_121 += $(CONFIG)/obj/ejsPath.o
DEPS_121 += $(CONFIG)/obj/ejsPot.o
DEPS_121 += $(CONFIG)/obj/ejsRegExp.o
DEPS_121 += $(CONFIG)/obj/ejsSocket.o
DEPS_121 += $(CONFIG)/obj/ejsString.o
DEPS_121 += $(CONFIG)/obj/ejsSystem.o
DEPS_121 += $(CONFIG)/obj/ejsTimer.o
DEPS_121 += $(CONFIG)/obj/ejsType.o
DEPS_121 += $(CONFIG)/obj/ejsUri.o
DEPS_121 += $(CONFIG)/obj/ejsVoid.o
DEPS_121 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_121 += $(CONFIG)/obj/ejsWorker.o
DEPS_121 += $(CONFIG)/obj/ejsXML.o
DEPS_121 += $(CONFIG)/obj/ejsXMLList.o
DEPS_121 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_121 += $(CONFIG)/obj/ejsByteCode.o
DEPS_121 += $(CONFIG)/obj/ejsException.o
DEPS_121 += $(CONFIG)/obj/ejsHelper.o
DEPS_121 += $(CONFIG)/obj/ejsInterp.o
DEPS_121 += $(CONFIG)/obj/ejsLoader.o
DEPS_121 += $(CONFIG)/obj/ejsModule.o
DEPS_121 += $(CONFIG)/obj/ejsScope.o
DEPS_121 += $(CONFIG)/obj/ejsService.o
DEPS_121 += $(CONFIG)/bin/libejs.a
DEPS_121 += $(CONFIG)/obj/ejsc.o
DEPS_121 += $(CONFIG)/bin/ejsc.out
DEPS_121 += src/cmd/ejsmod.h
DEPS_121 += $(CONFIG)/obj/ejsmod.o
DEPS_121 += $(CONFIG)/obj/doc.o
DEPS_121 += $(CONFIG)/obj/docFiles.o
DEPS_121 += $(CONFIG)/obj/listing.o
DEPS_121 += $(CONFIG)/obj/slotGen.o
DEPS_121 += $(CONFIG)/bin/ejsmod.out
DEPS_121 += $(CONFIG)/bin/ejs.mod
DEPS_121 += $(CONFIG)/bin/ejs.zlib.mod
DEPS_121 += $(CONFIG)/inc/zlib.h
DEPS_121 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_COM_ZLIB),1)
    DEPS_121 += $(CONFIG)/bin/libzlib.a
endif
DEPS_121 += $(CONFIG)/obj/ejsZlib.o

$(CONFIG)/bin/libejs.zlib.a: $(DEPS_121)
	@echo '      [Link] $(CONFIG)/bin/libejs.zlib.a'
	ar -cr $(CONFIG)/bin/libejs.zlib.a "$(CONFIG)/obj/ejsZlib.o"

#
#   manager.o
#
DEPS_122 += $(CONFIG)/inc/me.h
DEPS_122 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/manager.o: \
    src/paks/mpr/manager.c $(DEPS_122)
	@echo '   [Compile] $(CONFIG)/obj/manager.o'
	$(CC) -c -o $(CONFIG)/obj/manager.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/paks/mpr/manager.c

#
#   manager
#
DEPS_123 += $(CONFIG)/inc/mpr.h
DEPS_123 += $(CONFIG)/inc/me.h
DEPS_123 += $(CONFIG)/inc/osdep.h
DEPS_123 += $(CONFIG)/obj/mprLib.o
DEPS_123 += $(CONFIG)/bin/libmpr.a
DEPS_123 += $(CONFIG)/obj/manager.o

LIBS_123 += -lmpr

$(CONFIG)/bin/ejsman.out: $(DEPS_123)
	@echo '      [Link] $(CONFIG)/bin/ejsman.out'
	$(CC) -o $(CONFIG)/bin/ejsman.out $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/manager.o" $(LIBPATHS_123) $(LIBS_123) $(LIBS_123) $(LIBS) -Wl,-r 


#
#   mvc
#
DEPS_124 += $(CONFIG)/inc/mpr.h
DEPS_124 += $(CONFIG)/inc/me.h
DEPS_124 += $(CONFIG)/inc/osdep.h
DEPS_124 += $(CONFIG)/obj/mprLib.o
DEPS_124 += $(CONFIG)/bin/libmpr.a
DEPS_124 += $(CONFIG)/inc/pcre.h
DEPS_124 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_124 += $(CONFIG)/bin/libpcre.a
endif
DEPS_124 += $(CONFIG)/inc/http.h
DEPS_124 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_124 += $(CONFIG)/bin/libhttp.a
endif
DEPS_124 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_124 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_124 += $(CONFIG)/inc/ejs.slots.h
DEPS_124 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_124 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_124 += $(CONFIG)/inc/ejsByteCode.h
DEPS_124 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_124 += $(CONFIG)/inc/ejsCustomize.h
DEPS_124 += $(CONFIG)/inc/ejs.h
DEPS_124 += $(CONFIG)/inc/ejsCompiler.h
DEPS_124 += $(CONFIG)/obj/ecAst.o
DEPS_124 += $(CONFIG)/obj/ecCodeGen.o
DEPS_124 += $(CONFIG)/obj/ecCompiler.o
DEPS_124 += $(CONFIG)/obj/ecLex.o
DEPS_124 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_124 += $(CONFIG)/obj/ecParser.o
DEPS_124 += $(CONFIG)/obj/ecState.o
DEPS_124 += $(CONFIG)/obj/dtoa.o
DEPS_124 += $(CONFIG)/obj/ejsApp.o
DEPS_124 += $(CONFIG)/obj/ejsArray.o
DEPS_124 += $(CONFIG)/obj/ejsBlock.o
DEPS_124 += $(CONFIG)/obj/ejsBoolean.o
DEPS_124 += $(CONFIG)/obj/ejsByteArray.o
DEPS_124 += $(CONFIG)/obj/ejsCache.o
DEPS_124 += $(CONFIG)/obj/ejsCmd.o
DEPS_124 += $(CONFIG)/obj/ejsConfig.o
DEPS_124 += $(CONFIG)/obj/ejsDate.o
DEPS_124 += $(CONFIG)/obj/ejsDebug.o
DEPS_124 += $(CONFIG)/obj/ejsError.o
DEPS_124 += $(CONFIG)/obj/ejsFile.o
DEPS_124 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_124 += $(CONFIG)/obj/ejsFrame.o
DEPS_124 += $(CONFIG)/obj/ejsFunction.o
DEPS_124 += $(CONFIG)/obj/ejsGC.o
DEPS_124 += $(CONFIG)/obj/ejsGlobal.o
DEPS_124 += $(CONFIG)/obj/ejsHttp.o
DEPS_124 += $(CONFIG)/obj/ejsIterator.o
DEPS_124 += $(CONFIG)/obj/ejsJSON.o
DEPS_124 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_124 += $(CONFIG)/obj/ejsMath.o
DEPS_124 += $(CONFIG)/obj/ejsMemory.o
DEPS_124 += $(CONFIG)/obj/ejsMprLog.o
DEPS_124 += $(CONFIG)/obj/ejsNamespace.o
DEPS_124 += $(CONFIG)/obj/ejsNull.o
DEPS_124 += $(CONFIG)/obj/ejsNumber.o
DEPS_124 += $(CONFIG)/obj/ejsObject.o
DEPS_124 += $(CONFIG)/obj/ejsPath.o
DEPS_124 += $(CONFIG)/obj/ejsPot.o
DEPS_124 += $(CONFIG)/obj/ejsRegExp.o
DEPS_124 += $(CONFIG)/obj/ejsSocket.o
DEPS_124 += $(CONFIG)/obj/ejsString.o
DEPS_124 += $(CONFIG)/obj/ejsSystem.o
DEPS_124 += $(CONFIG)/obj/ejsTimer.o
DEPS_124 += $(CONFIG)/obj/ejsType.o
DEPS_124 += $(CONFIG)/obj/ejsUri.o
DEPS_124 += $(CONFIG)/obj/ejsVoid.o
DEPS_124 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_124 += $(CONFIG)/obj/ejsWorker.o
DEPS_124 += $(CONFIG)/obj/ejsXML.o
DEPS_124 += $(CONFIG)/obj/ejsXMLList.o
DEPS_124 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_124 += $(CONFIG)/obj/ejsByteCode.o
DEPS_124 += $(CONFIG)/obj/ejsException.o
DEPS_124 += $(CONFIG)/obj/ejsHelper.o
DEPS_124 += $(CONFIG)/obj/ejsInterp.o
DEPS_124 += $(CONFIG)/obj/ejsLoader.o
DEPS_124 += $(CONFIG)/obj/ejsModule.o
DEPS_124 += $(CONFIG)/obj/ejsScope.o
DEPS_124 += $(CONFIG)/obj/ejsService.o
DEPS_124 += $(CONFIG)/bin/libejs.a
DEPS_124 += $(CONFIG)/bin/mvc.es
DEPS_124 += $(CONFIG)/obj/ejsrun.o

LIBS_124 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_124 += -lhttp
endif
LIBS_124 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_124 += -lpcre
endif

$(CONFIG)/bin/mvc.out: $(DEPS_124)
	@echo '      [Link] $(CONFIG)/bin/mvc.out'
	$(CC) -o $(CONFIG)/bin/mvc.out $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejsrun.o" $(LIBPATHS_124) $(LIBS_124) $(LIBS_124) $(LIBS) -Wl,-r 

#
#   sqlite.o
#
DEPS_125 += $(CONFIG)/inc/me.h
DEPS_125 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite.o: \
    src/paks/sqlite/sqlite.c $(DEPS_125)
	@echo '   [Compile] $(CONFIG)/obj/sqlite.o'
	$(CC) -c -o $(CONFIG)/obj/sqlite.o $(CFLAGS) $(DFLAGS) "-I$(CONFIG)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/paks/sqlite/sqlite.c

ifeq ($(ME_COM_SQLITE),1)
#
#   sqliteshell
#
DEPS_126 += $(CONFIG)/inc/sqlite3.h
DEPS_126 += $(CONFIG)/inc/me.h
DEPS_126 += $(CONFIG)/obj/sqlite3.o
DEPS_126 += $(CONFIG)/bin/libsql.a
DEPS_126 += $(CONFIG)/obj/sqlite.o

LIBS_126 += -lsql

$(CONFIG)/bin/sqlite.out: $(DEPS_126)
	@echo '      [Link] $(CONFIG)/bin/sqlite.out'
	$(CC) -o $(CONFIG)/bin/sqlite.out $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/sqlite.o" $(LIBPATHS_126) $(LIBS_126) $(LIBS_126) $(LIBS) -Wl,-r 
endif



#
#   utest
#
DEPS_127 += $(CONFIG)/inc/mpr.h
DEPS_127 += $(CONFIG)/inc/me.h
DEPS_127 += $(CONFIG)/inc/osdep.h
DEPS_127 += $(CONFIG)/obj/mprLib.o
DEPS_127 += $(CONFIG)/bin/libmpr.a
DEPS_127 += $(CONFIG)/inc/pcre.h
DEPS_127 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_COM_PCRE),1)
    DEPS_127 += $(CONFIG)/bin/libpcre.a
endif
DEPS_127 += $(CONFIG)/inc/http.h
DEPS_127 += $(CONFIG)/obj/httpLib.o
ifeq ($(ME_COM_HTTP),1)
    DEPS_127 += $(CONFIG)/bin/libhttp.a
endif
DEPS_127 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_127 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_127 += $(CONFIG)/inc/ejs.slots.h
DEPS_127 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_127 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_127 += $(CONFIG)/inc/ejsByteCode.h
DEPS_127 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_127 += $(CONFIG)/inc/ejsCustomize.h
DEPS_127 += $(CONFIG)/inc/ejs.h
DEPS_127 += $(CONFIG)/inc/ejsCompiler.h
DEPS_127 += $(CONFIG)/obj/ecAst.o
DEPS_127 += $(CONFIG)/obj/ecCodeGen.o
DEPS_127 += $(CONFIG)/obj/ecCompiler.o
DEPS_127 += $(CONFIG)/obj/ecLex.o
DEPS_127 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_127 += $(CONFIG)/obj/ecParser.o
DEPS_127 += $(CONFIG)/obj/ecState.o
DEPS_127 += $(CONFIG)/obj/dtoa.o
DEPS_127 += $(CONFIG)/obj/ejsApp.o
DEPS_127 += $(CONFIG)/obj/ejsArray.o
DEPS_127 += $(CONFIG)/obj/ejsBlock.o
DEPS_127 += $(CONFIG)/obj/ejsBoolean.o
DEPS_127 += $(CONFIG)/obj/ejsByteArray.o
DEPS_127 += $(CONFIG)/obj/ejsCache.o
DEPS_127 += $(CONFIG)/obj/ejsCmd.o
DEPS_127 += $(CONFIG)/obj/ejsConfig.o
DEPS_127 += $(CONFIG)/obj/ejsDate.o
DEPS_127 += $(CONFIG)/obj/ejsDebug.o
DEPS_127 += $(CONFIG)/obj/ejsError.o
DEPS_127 += $(CONFIG)/obj/ejsFile.o
DEPS_127 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_127 += $(CONFIG)/obj/ejsFrame.o
DEPS_127 += $(CONFIG)/obj/ejsFunction.o
DEPS_127 += $(CONFIG)/obj/ejsGC.o
DEPS_127 += $(CONFIG)/obj/ejsGlobal.o
DEPS_127 += $(CONFIG)/obj/ejsHttp.o
DEPS_127 += $(CONFIG)/obj/ejsIterator.o
DEPS_127 += $(CONFIG)/obj/ejsJSON.o
DEPS_127 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_127 += $(CONFIG)/obj/ejsMath.o
DEPS_127 += $(CONFIG)/obj/ejsMemory.o
DEPS_127 += $(CONFIG)/obj/ejsMprLog.o
DEPS_127 += $(CONFIG)/obj/ejsNamespace.o
DEPS_127 += $(CONFIG)/obj/ejsNull.o
DEPS_127 += $(CONFIG)/obj/ejsNumber.o
DEPS_127 += $(CONFIG)/obj/ejsObject.o
DEPS_127 += $(CONFIG)/obj/ejsPath.o
DEPS_127 += $(CONFIG)/obj/ejsPot.o
DEPS_127 += $(CONFIG)/obj/ejsRegExp.o
DEPS_127 += $(CONFIG)/obj/ejsSocket.o
DEPS_127 += $(CONFIG)/obj/ejsString.o
DEPS_127 += $(CONFIG)/obj/ejsSystem.o
DEPS_127 += $(CONFIG)/obj/ejsTimer.o
DEPS_127 += $(CONFIG)/obj/ejsType.o
DEPS_127 += $(CONFIG)/obj/ejsUri.o
DEPS_127 += $(CONFIG)/obj/ejsVoid.o
DEPS_127 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_127 += $(CONFIG)/obj/ejsWorker.o
DEPS_127 += $(CONFIG)/obj/ejsXML.o
DEPS_127 += $(CONFIG)/obj/ejsXMLList.o
DEPS_127 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_127 += $(CONFIG)/obj/ejsByteCode.o
DEPS_127 += $(CONFIG)/obj/ejsException.o
DEPS_127 += $(CONFIG)/obj/ejsHelper.o
DEPS_127 += $(CONFIG)/obj/ejsInterp.o
DEPS_127 += $(CONFIG)/obj/ejsLoader.o
DEPS_127 += $(CONFIG)/obj/ejsModule.o
DEPS_127 += $(CONFIG)/obj/ejsScope.o
DEPS_127 += $(CONFIG)/obj/ejsService.o
DEPS_127 += $(CONFIG)/bin/libejs.a
DEPS_127 += $(CONFIG)/bin/utest.es
DEPS_127 += $(CONFIG)/bin/utest.worker
DEPS_127 += $(CONFIG)/obj/ejsrun.o

LIBS_127 += -lejs
ifeq ($(ME_COM_HTTP),1)
    LIBS_127 += -lhttp
endif
LIBS_127 += -lmpr
ifeq ($(ME_COM_PCRE),1)
    LIBS_127 += -lpcre
endif

$(CONFIG)/bin/utest.out: $(DEPS_127)
	@echo '      [Link] $(CONFIG)/bin/utest.out'
	$(CC) -o $(CONFIG)/bin/utest.out $(LDFLAGS) $(LIBPATHS) "$(CONFIG)/obj/ejsrun.o" $(LIBPATHS_127) $(LIBS_127) $(LIBS_127) $(LIBS) -Wl,-r 

#
#   www
#
DEPS_128 += src/ejs.web/www

$(CONFIG)/bin/www: $(DEPS_128)
	( \
	cd src/ejs.web; \
	rm -fr ../../$(CONFIG)/bin/www ; \
	cp -r www ../../$(CONFIG)/bin ; \
	)

#
#   installBinary
#
installBinary: $(DEPS_129)

#
#   start
#
start: $(DEPS_130)

#
#   stop
#
stop: $(DEPS_131)

#
#   uninstall
#
DEPS_132 += stop

uninstall: $(DEPS_132)

