#
#   ejs-freebsd-static.mk -- Makefile to build Embedthis Ejscript for freebsd
#

NAME                  := ejs
VERSION               := 2.3.5
PROFILE               ?= static
ARCH                  ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
CC_ARCH               ?= $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                    ?= freebsd
CC                    ?= gcc
LD                    ?= ld
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
LBIN                  ?= $(CONFIG)/bin
PATH                  := $(LBIN):$(PATH)

ME_EXT_EST            ?= 1
ME_EXT_MATRIXSSL      ?= 0
ME_EXT_NANOSSL        ?= 0
ME_EXT_OPENSSL        ?= 0
ME_EXT_PCRE           ?= 1
ME_EXT_SQLITE         ?= 1
ME_EXT_SSL            ?= 1
ME_EXT_ZLIB           ?= 1

ME_EXT_COMPILER_PATH  ?= gcc
ME_EXT_DOXYGEN_PATH   ?= doxygen
ME_EXT_DSI_PATH       ?= dsi
ME_EXT_EST_PATH       ?= src/paks/est/estLib.c
ME_EXT_HTTP_PATH      ?= src/paks/http/http.me
ME_EXT_LIB_PATH       ?= ar
ME_EXT_LINK_PATH      ?= ld
ME_EXT_MAN_PATH       ?= man
ME_EXT_MAN2HTML_PATH  ?= man2html
ME_EXT_MATRIXSSL_PATH ?= /usr/src/matrixssl
ME_EXT_MPR_PATH       ?= src/paks/mpr/mpr.me
ME_EXT_NANOSSL_PATH   ?= /usr/src/nanossl
ME_EXT_OPENSSL_PATH   ?= /usr/src/openssl
ME_EXT_OSDEP_PATH     ?= src/paks/osdep/osdep.me
ME_EXT_PAK_PATH       ?= pak
ME_EXT_PCRE_PATH      ?= src/paks/pcre/pcre.me
ME_EXT_PMAKER_PATH    ?= [function Function]
ME_EXT_SQLITE_PATH    ?= src/paks/sqlite
ME_EXT_VXWORKS_PATH   ?= $(WIND_BASE)
ME_EXT_ZIP_PATH       ?= zip
ME_EXT_ZLIB_PATH      ?= src/paks/zlib/zlib.me

export WIND_HOME      ?= $(WIND_BASE)/..

CFLAGS                += -fPIC -w
DFLAGS                += -D_REENTRANT -DPIC $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_EXT_EST=$(ME_EXT_EST) -DME_EXT_MATRIXSSL=$(ME_EXT_MATRIXSSL) -DME_EXT_NANOSSL=$(ME_EXT_NANOSSL) -DME_EXT_OPENSSL=$(ME_EXT_OPENSSL) -DME_EXT_PCRE=$(ME_EXT_PCRE) -DME_EXT_SQLITE=$(ME_EXT_SQLITE) -DME_EXT_SSL=$(ME_EXT_SSL) -DME_EXT_ZLIB=$(ME_EXT_ZLIB) 
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


ifeq ($(ME_EXT_EST),1)
    TARGETS           += $(CONFIG)/bin/libest.a
endif
TARGETS               += $(CONFIG)/bin/ca.crt
TARGETS               += $(CONFIG)/bin/libhttp.a
TARGETS               += $(CONFIG)/bin/http
TARGETS               += $(CONFIG)/bin/ejsman
ifeq ($(ME_EXT_SQLITE),1)
    TARGETS           += $(CONFIG)/bin/libsql.a
endif
ifeq ($(ME_EXT_SQLITE),1)
    TARGETS           += $(CONFIG)/bin/sqlite
endif
ifeq ($(ME_EXT_ZLIB),1)
    TARGETS           += $(CONFIG)/bin/libzlib.a
endif
TARGETS               += $(CONFIG)/bin/libejs.a
TARGETS               += $(CONFIG)/bin/ejs
TARGETS               += $(CONFIG)/bin/ejsc
TARGETS               += $(CONFIG)/bin/ejsmod
TARGETS               += $(CONFIG)/bin/ejsrun
TARGETS               += $(CONFIG)/bin/ejs.mod
TARGETS               += $(CONFIG)/bin/ejs.unix.mod
TARGETS               += $(CONFIG)/bin/ejs.db.mod
TARGETS               += $(CONFIG)/bin/ejs.db.mapper.mod
TARGETS               += $(CONFIG)/bin/ejs.db.sqlite.mod
TARGETS               += $(CONFIG)/bin/libejs.db.sqlite.a
TARGETS               += $(CONFIG)/bin/ejs.mail.mod
TARGETS               += $(CONFIG)/bin/ejs.web.mod
TARGETS               += $(CONFIG)/bin/libejs.web.a
TARGETS               += $(CONFIG)/bin/www
TARGETS               += $(CONFIG)/bin/ejs.template.mod
TARGETS               += $(CONFIG)/bin/ejs.zlib.mod
TARGETS               += $(CONFIG)/bin/libejs.zlib.a
TARGETS               += $(CONFIG)/bin/ejs.tar.mod
TARGETS               += $(CONFIG)/bin/mvc.es
TARGETS               += $(CONFIG)/bin/mvc
TARGETS               += $(CONFIG)/bin/ejs.mvc.mod
TARGETS               += $(CONFIG)/bin/utest.es
TARGETS               += $(CONFIG)/bin/utest.worker
TARGETS               += $(CONFIG)/bin/utest

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
	@[ ! -f $(CONFIG)/inc/me.h ] && cp projects/ejs-freebsd-static-me.h $(CONFIG)/inc/me.h ; true
	@if ! diff $(CONFIG)/inc/me.h projects/ejs-freebsd-static-me.h >/dev/null ; then\
		cp projects/ejs-freebsd-static-me.h $(CONFIG)/inc/me.h  ; \
	fi; true
	@if [ -f "$(CONFIG)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != " ` cat $(CONFIG)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build: "`cat $(CONFIG)/.makeflags`"" ; \
		fi ; \
	fi
	@echo $(MAKEFLAGS) >$(CONFIG)/.makeflags

clean:
	rm -f "$(CONFIG)/bin/libest.a"
	rm -f "$(CONFIG)/bin/ca.crt"
	rm -f "$(CONFIG)/bin/libhttp.a"
	rm -f "$(CONFIG)/bin/http"
	rm -f "$(CONFIG)/bin/libmpr.a"
	rm -f "$(CONFIG)/bin/libmprssl.a"
	rm -f "$(CONFIG)/bin/ejsman"
	rm -f "$(CONFIG)/bin/makerom"
	rm -f "$(CONFIG)/bin/libpcre.a"
	rm -f "$(CONFIG)/bin/libsql.a"
	rm -f "$(CONFIG)/bin/sqlite"
	rm -f "$(CONFIG)/bin/libzlib.a"
	rm -f "$(CONFIG)/bin/libejs.a"
	rm -f "$(CONFIG)/bin/ejs"
	rm -f "$(CONFIG)/bin/ejsc"
	rm -f "$(CONFIG)/bin/ejsmod"
	rm -f "$(CONFIG)/bin/ejsrun"
	rm -f "$(CONFIG)/bin/libejs.db.sqlite.a"
	rm -f "$(CONFIG)/bin/libejs.web.a"
	rm -f "$(CONFIG)/bin/libejs.zlib.a"
	rm -f "$(CONFIG)/bin/utest"
	rm -f "$(CONFIG)/obj/estLib.o"
	rm -f "$(CONFIG)/obj/httpLib.o"
	rm -f "$(CONFIG)/obj/http.o"
	rm -f "$(CONFIG)/obj/mprLib.o"
	rm -f "$(CONFIG)/obj/mprSsl.o"
	rm -f "$(CONFIG)/obj/manager.o"
	rm -f "$(CONFIG)/obj/makerom.o"
	rm -f "$(CONFIG)/obj/pcre.o"
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
	echo 2.3.5

#
#   est.h
#
$(CONFIG)/inc/est.h: $(DEPS_2)
	@echo '      [Copy] $(CONFIG)/inc/est.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/est/est.h $(CONFIG)/inc/est.h

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
#   estLib.o
#
DEPS_5 += $(CONFIG)/inc/me.h
DEPS_5 += $(CONFIG)/inc/est.h
DEPS_5 += $(CONFIG)/inc/osdep.h

$(CONFIG)/obj/estLib.o: \
    src/paks/est/estLib.c $(DEPS_5)
	@echo '   [Compile] $(CONFIG)/obj/estLib.o'
	$(CC) -c -o $(CONFIG)/obj/estLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/est/estLib.c

ifeq ($(ME_EXT_EST),1)
#
#   libest
#
DEPS_6 += $(CONFIG)/inc/est.h
DEPS_6 += $(CONFIG)/inc/me.h
DEPS_6 += $(CONFIG)/inc/osdep.h
DEPS_6 += $(CONFIG)/obj/estLib.o

$(CONFIG)/bin/libest.a: $(DEPS_6)
	@echo '      [Link] $(CONFIG)/bin/libest.a'
	ar -cr $(CONFIG)/bin/libest.a "$(CONFIG)/obj/estLib.o"
endif

#
#   ca-crt
#
DEPS_7 += src/paks/est/ca.crt

$(CONFIG)/bin/ca.crt: $(DEPS_7)
	@echo '      [Copy] $(CONFIG)/bin/ca.crt'
	mkdir -p "$(CONFIG)/bin"
	cp src/paks/est/ca.crt $(CONFIG)/bin/ca.crt

#
#   mpr.h
#
$(CONFIG)/inc/mpr.h: $(DEPS_8)
	@echo '      [Copy] $(CONFIG)/inc/mpr.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/mpr/mpr.h $(CONFIG)/inc/mpr.h

#
#   mprLib.o
#
DEPS_9 += $(CONFIG)/inc/me.h
DEPS_9 += $(CONFIG)/inc/mpr.h
DEPS_9 += $(CONFIG)/inc/osdep.h

$(CONFIG)/obj/mprLib.o: \
    src/paks/mpr/mprLib.c $(DEPS_9)
	@echo '   [Compile] $(CONFIG)/obj/mprLib.o'
	$(CC) -c -o $(CONFIG)/obj/mprLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/mprLib.c

#
#   libmpr
#
DEPS_10 += $(CONFIG)/inc/mpr.h
DEPS_10 += $(CONFIG)/inc/me.h
DEPS_10 += $(CONFIG)/inc/osdep.h
DEPS_10 += $(CONFIG)/obj/mprLib.o

$(CONFIG)/bin/libmpr.a: $(DEPS_10)
	@echo '      [Link] $(CONFIG)/bin/libmpr.a'
	ar -cr $(CONFIG)/bin/libmpr.a "$(CONFIG)/obj/mprLib.o"

#
#   pcre.h
#
$(CONFIG)/inc/pcre.h: $(DEPS_11)
	@echo '      [Copy] $(CONFIG)/inc/pcre.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/pcre/pcre.h $(CONFIG)/inc/pcre.h

#
#   pcre.o
#
DEPS_12 += $(CONFIG)/inc/me.h
DEPS_12 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
    src/paks/pcre/pcre.c $(DEPS_12)
	@echo '   [Compile] $(CONFIG)/obj/pcre.o'
	$(CC) -c -o $(CONFIG)/obj/pcre.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/pcre/pcre.c

ifeq ($(ME_EXT_PCRE),1)
#
#   libpcre
#
DEPS_13 += $(CONFIG)/inc/pcre.h
DEPS_13 += $(CONFIG)/inc/me.h
DEPS_13 += $(CONFIG)/obj/pcre.o

$(CONFIG)/bin/libpcre.a: $(DEPS_13)
	@echo '      [Link] $(CONFIG)/bin/libpcre.a'
	ar -cr $(CONFIG)/bin/libpcre.a "$(CONFIG)/obj/pcre.o"
endif

#
#   http.h
#
$(CONFIG)/inc/http.h: $(DEPS_14)
	@echo '      [Copy] $(CONFIG)/inc/http.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/http/http.h $(CONFIG)/inc/http.h

#
#   httpLib.o
#
DEPS_15 += $(CONFIG)/inc/me.h
DEPS_15 += $(CONFIG)/inc/http.h
DEPS_15 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/httpLib.o: \
    src/paks/http/httpLib.c $(DEPS_15)
	@echo '   [Compile] $(CONFIG)/obj/httpLib.o'
	$(CC) -c -o $(CONFIG)/obj/httpLib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/httpLib.c

#
#   libhttp
#
DEPS_16 += $(CONFIG)/inc/mpr.h
DEPS_16 += $(CONFIG)/inc/me.h
DEPS_16 += $(CONFIG)/inc/osdep.h
DEPS_16 += $(CONFIG)/obj/mprLib.o
DEPS_16 += $(CONFIG)/bin/libmpr.a
DEPS_16 += $(CONFIG)/inc/pcre.h
DEPS_16 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_16 += $(CONFIG)/bin/libpcre.a
endif
DEPS_16 += $(CONFIG)/inc/http.h
DEPS_16 += $(CONFIG)/obj/httpLib.o

$(CONFIG)/bin/libhttp.a: $(DEPS_16)
	@echo '      [Link] $(CONFIG)/bin/libhttp.a'
	ar -cr $(CONFIG)/bin/libhttp.a "$(CONFIG)/obj/httpLib.o"

#
#   mprSsl.o
#
DEPS_17 += $(CONFIG)/inc/me.h
DEPS_17 += $(CONFIG)/inc/mpr.h
DEPS_17 += $(CONFIG)/inc/est.h

$(CONFIG)/obj/mprSsl.o: \
    src/paks/mpr/mprSsl.c $(DEPS_17)
	@echo '   [Compile] $(CONFIG)/obj/mprSsl.o'
	$(CC) -c -o $(CONFIG)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/mprSsl.c

#
#   libmprssl
#
DEPS_18 += $(CONFIG)/inc/mpr.h
DEPS_18 += $(CONFIG)/inc/me.h
DEPS_18 += $(CONFIG)/inc/osdep.h
DEPS_18 += $(CONFIG)/obj/mprLib.o
DEPS_18 += $(CONFIG)/bin/libmpr.a
DEPS_18 += $(CONFIG)/inc/est.h
DEPS_18 += $(CONFIG)/obj/estLib.o
ifeq ($(ME_EXT_EST),1)
    DEPS_18 += $(CONFIG)/bin/libest.a
endif
DEPS_18 += $(CONFIG)/obj/mprSsl.o

$(CONFIG)/bin/libmprssl.a: $(DEPS_18)
	@echo '      [Link] $(CONFIG)/bin/libmprssl.a'
	ar -cr $(CONFIG)/bin/libmprssl.a "$(CONFIG)/obj/mprSsl.o"

#
#   http.o
#
DEPS_19 += $(CONFIG)/inc/me.h
DEPS_19 += $(CONFIG)/inc/http.h

$(CONFIG)/obj/http.o: \
    src/paks/http/http.c $(DEPS_19)
	@echo '   [Compile] $(CONFIG)/obj/http.o'
	$(CC) -c -o $(CONFIG)/obj/http.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/http/http.c

#
#   httpcmd
#
DEPS_20 += $(CONFIG)/inc/mpr.h
DEPS_20 += $(CONFIG)/inc/me.h
DEPS_20 += $(CONFIG)/inc/osdep.h
DEPS_20 += $(CONFIG)/obj/mprLib.o
DEPS_20 += $(CONFIG)/bin/libmpr.a
DEPS_20 += $(CONFIG)/inc/pcre.h
DEPS_20 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_20 += $(CONFIG)/bin/libpcre.a
endif
DEPS_20 += $(CONFIG)/inc/http.h
DEPS_20 += $(CONFIG)/obj/httpLib.o
DEPS_20 += $(CONFIG)/bin/libhttp.a
DEPS_20 += $(CONFIG)/inc/est.h
DEPS_20 += $(CONFIG)/obj/estLib.o
ifeq ($(ME_EXT_EST),1)
    DEPS_20 += $(CONFIG)/bin/libest.a
endif
DEPS_20 += $(CONFIG)/obj/mprSsl.o
DEPS_20 += $(CONFIG)/bin/libmprssl.a
DEPS_20 += $(CONFIG)/obj/http.o

LIBS_20 += -lhttp
LIBS_20 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_20 += -lpcre
endif
LIBS_20 += -lmprssl
ifeq ($(ME_EXT_EST),1)
    LIBS_20 += -lest
endif

$(CONFIG)/bin/http: $(DEPS_20)
	@echo '      [Link] $(CONFIG)/bin/http'
	$(CC) -o $(CONFIG)/bin/http $(LIBPATHS) "$(CONFIG)/obj/http.o" $(LIBPATHS_20) $(LIBS_20) $(LIBS_20) $(LIBS) $(LIBS) 

#
#   manager.o
#
DEPS_21 += $(CONFIG)/inc/me.h
DEPS_21 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/manager.o: \
    src/paks/mpr/manager.c $(DEPS_21)
	@echo '   [Compile] $(CONFIG)/obj/manager.o'
	$(CC) -c -o $(CONFIG)/obj/manager.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/mpr/manager.c

#
#   manager
#
DEPS_22 += $(CONFIG)/inc/mpr.h
DEPS_22 += $(CONFIG)/inc/me.h
DEPS_22 += $(CONFIG)/inc/osdep.h
DEPS_22 += $(CONFIG)/obj/mprLib.o
DEPS_22 += $(CONFIG)/bin/libmpr.a
DEPS_22 += $(CONFIG)/obj/manager.o

LIBS_22 += -lmpr

$(CONFIG)/bin/ejsman: $(DEPS_22)
	@echo '      [Link] $(CONFIG)/bin/ejsman'
	$(CC) -o $(CONFIG)/bin/ejsman $(LIBPATHS) "$(CONFIG)/obj/manager.o" $(LIBPATHS_22) $(LIBS_22) $(LIBS_22) $(LIBS) $(LIBS) 

#
#   sqlite3.h
#
$(CONFIG)/inc/sqlite3.h: $(DEPS_23)
	@echo '      [Copy] $(CONFIG)/inc/sqlite3.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/sqlite/sqlite3.h $(CONFIG)/inc/sqlite3.h

#
#   sqlite3.o
#
DEPS_24 += $(CONFIG)/inc/me.h
DEPS_24 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite3.o: \
    src/paks/sqlite/sqlite3.c $(DEPS_24)
	@echo '   [Compile] $(CONFIG)/obj/sqlite3.o'
	$(CC) -c -o $(CONFIG)/obj/sqlite3.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/sqlite/sqlite3.c

ifeq ($(ME_EXT_SQLITE),1)
#
#   libsql
#
DEPS_25 += $(CONFIG)/inc/sqlite3.h
DEPS_25 += $(CONFIG)/inc/me.h
DEPS_25 += $(CONFIG)/obj/sqlite3.o

$(CONFIG)/bin/libsql.a: $(DEPS_25)
	@echo '      [Link] $(CONFIG)/bin/libsql.a'
	ar -cr $(CONFIG)/bin/libsql.a "$(CONFIG)/obj/sqlite3.o"
endif

#
#   sqlite.o
#
DEPS_26 += $(CONFIG)/inc/me.h
DEPS_26 += $(CONFIG)/inc/sqlite3.h

$(CONFIG)/obj/sqlite.o: \
    src/paks/sqlite/sqlite.c $(DEPS_26)
	@echo '   [Compile] $(CONFIG)/obj/sqlite.o'
	$(CC) -c -o $(CONFIG)/obj/sqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/sqlite/sqlite.c

ifeq ($(ME_EXT_SQLITE),1)
#
#   sqliteshell
#
DEPS_27 += $(CONFIG)/inc/sqlite3.h
DEPS_27 += $(CONFIG)/inc/me.h
DEPS_27 += $(CONFIG)/obj/sqlite3.o
DEPS_27 += $(CONFIG)/bin/libsql.a
DEPS_27 += $(CONFIG)/obj/sqlite.o

LIBS_27 += -lsql

$(CONFIG)/bin/sqlite: $(DEPS_27)
	@echo '      [Link] $(CONFIG)/bin/sqlite'
	$(CC) -o $(CONFIG)/bin/sqlite $(LIBPATHS) "$(CONFIG)/obj/sqlite.o" $(LIBPATHS_27) $(LIBS_27) $(LIBS_27) $(LIBS) $(LIBS) 
endif

#
#   zlib.h
#
$(CONFIG)/inc/zlib.h: $(DEPS_28)
	@echo '      [Copy] $(CONFIG)/inc/zlib.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/paks/zlib/zlib.h $(CONFIG)/inc/zlib.h

#
#   zlib.o
#
DEPS_29 += $(CONFIG)/inc/me.h
DEPS_29 += $(CONFIG)/inc/zlib.h

$(CONFIG)/obj/zlib.o: \
    src/paks/zlib/zlib.c $(DEPS_29)
	@echo '   [Compile] $(CONFIG)/obj/zlib.o'
	$(CC) -c -o $(CONFIG)/obj/zlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/paks/zlib/zlib.c

ifeq ($(ME_EXT_ZLIB),1)
#
#   libzlib
#
DEPS_30 += $(CONFIG)/inc/zlib.h
DEPS_30 += $(CONFIG)/inc/me.h
DEPS_30 += $(CONFIG)/obj/zlib.o

$(CONFIG)/bin/libzlib.a: $(DEPS_30)
	@echo '      [Link] $(CONFIG)/bin/libzlib.a'
	ar -cr $(CONFIG)/bin/libzlib.a "$(CONFIG)/obj/zlib.o"
endif

#
#   ejs.cache.local.slots.h
#
$(CONFIG)/inc/ejs.cache.local.slots.h: $(DEPS_31)
	@echo '      [Copy] $(CONFIG)/inc/ejs.cache.local.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/slots/ejs.cache.local.slots.h $(CONFIG)/inc/ejs.cache.local.slots.h

#
#   ejs.db.sqlite.slots.h
#
$(CONFIG)/inc/ejs.db.sqlite.slots.h: $(DEPS_32)
	@echo '      [Copy] $(CONFIG)/inc/ejs.db.sqlite.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/slots/ejs.db.sqlite.slots.h $(CONFIG)/inc/ejs.db.sqlite.slots.h

#
#   ejs.slots.h
#
$(CONFIG)/inc/ejs.slots.h: $(DEPS_33)
	@echo '      [Copy] $(CONFIG)/inc/ejs.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/slots/ejs.slots.h $(CONFIG)/inc/ejs.slots.h

#
#   ejs.web.slots.h
#
$(CONFIG)/inc/ejs.web.slots.h: $(DEPS_34)
	@echo '      [Copy] $(CONFIG)/inc/ejs.web.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/slots/ejs.web.slots.h $(CONFIG)/inc/ejs.web.slots.h

#
#   ejs.zlib.slots.h
#
$(CONFIG)/inc/ejs.zlib.slots.h: $(DEPS_35)
	@echo '      [Copy] $(CONFIG)/inc/ejs.zlib.slots.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/slots/ejs.zlib.slots.h $(CONFIG)/inc/ejs.zlib.slots.h

#
#   ejsByteCode.h
#
$(CONFIG)/inc/ejsByteCode.h: $(DEPS_36)
	@echo '      [Copy] $(CONFIG)/inc/ejsByteCode.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejsByteCode.h $(CONFIG)/inc/ejsByteCode.h

#
#   ejsByteCodeTable.h
#
$(CONFIG)/inc/ejsByteCodeTable.h: $(DEPS_37)
	@echo '      [Copy] $(CONFIG)/inc/ejsByteCodeTable.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejsByteCodeTable.h $(CONFIG)/inc/ejsByteCodeTable.h

#
#   ejsCustomize.h
#
$(CONFIG)/inc/ejsCustomize.h: $(DEPS_38)
	@echo '      [Copy] $(CONFIG)/inc/ejsCustomize.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejsCustomize.h $(CONFIG)/inc/ejsCustomize.h

#
#   ejs.h
#
DEPS_39 += $(CONFIG)/inc/mpr.h
DEPS_39 += $(CONFIG)/inc/http.h
DEPS_39 += $(CONFIG)/inc/ejsByteCode.h
DEPS_39 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_39 += $(CONFIG)/inc/ejs.slots.h
DEPS_39 += $(CONFIG)/inc/ejsCustomize.h

$(CONFIG)/inc/ejs.h: $(DEPS_39)
	@echo '      [Copy] $(CONFIG)/inc/ejs.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejs.h $(CONFIG)/inc/ejs.h

#
#   ejsCompiler.h
#
$(CONFIG)/inc/ejsCompiler.h: $(DEPS_40)
	@echo '      [Copy] $(CONFIG)/inc/ejsCompiler.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejsCompiler.h $(CONFIG)/inc/ejsCompiler.h

#
#   ecAst.o
#
DEPS_41 += $(CONFIG)/inc/me.h
DEPS_41 += $(CONFIG)/inc/ejsCompiler.h
DEPS_41 += $(CONFIG)/inc/mpr.h
DEPS_41 += $(CONFIG)/inc/http.h
DEPS_41 += $(CONFIG)/inc/ejsByteCode.h
DEPS_41 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_41 += $(CONFIG)/inc/ejs.slots.h
DEPS_41 += $(CONFIG)/inc/ejsCustomize.h
DEPS_41 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ecAst.o: \
    src/compiler/ecAst.c $(DEPS_41)
	@echo '   [Compile] $(CONFIG)/obj/ecAst.o'
	$(CC) -c -o $(CONFIG)/obj/ecAst.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecAst.c

#
#   ecCodeGen.o
#
DEPS_42 += $(CONFIG)/inc/me.h
DEPS_42 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecCodeGen.o: \
    src/compiler/ecCodeGen.c $(DEPS_42)
	@echo '   [Compile] $(CONFIG)/obj/ecCodeGen.o'
	$(CC) -c -o $(CONFIG)/obj/ecCodeGen.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCodeGen.c

#
#   ecCompiler.o
#
DEPS_43 += $(CONFIG)/inc/me.h
DEPS_43 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecCompiler.o: \
    src/compiler/ecCompiler.c $(DEPS_43)
	@echo '   [Compile] $(CONFIG)/obj/ecCompiler.o'
	$(CC) -c -o $(CONFIG)/obj/ecCompiler.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecCompiler.c

#
#   ecLex.o
#
DEPS_44 += $(CONFIG)/inc/me.h
DEPS_44 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecLex.o: \
    src/compiler/ecLex.c $(DEPS_44)
	@echo '   [Compile] $(CONFIG)/obj/ecLex.o'
	$(CC) -c -o $(CONFIG)/obj/ecLex.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecLex.c

#
#   ecModuleWrite.o
#
DEPS_45 += $(CONFIG)/inc/me.h
DEPS_45 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecModuleWrite.o: \
    src/compiler/ecModuleWrite.c $(DEPS_45)
	@echo '   [Compile] $(CONFIG)/obj/ecModuleWrite.o'
	$(CC) -c -o $(CONFIG)/obj/ecModuleWrite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecModuleWrite.c

#
#   ecParser.o
#
DEPS_46 += $(CONFIG)/inc/me.h
DEPS_46 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecParser.o: \
    src/compiler/ecParser.c $(DEPS_46)
	@echo '   [Compile] $(CONFIG)/obj/ecParser.o'
	$(CC) -c -o $(CONFIG)/obj/ecParser.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecParser.c

#
#   ecState.o
#
DEPS_47 += $(CONFIG)/inc/me.h
DEPS_47 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ecState.o: \
    src/compiler/ecState.c $(DEPS_47)
	@echo '   [Compile] $(CONFIG)/obj/ecState.o'
	$(CC) -c -o $(CONFIG)/obj/ecState.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/compiler/ecState.c

#
#   dtoa.o
#
DEPS_48 += $(CONFIG)/inc/me.h
DEPS_48 += $(CONFIG)/inc/mpr.h

$(CONFIG)/obj/dtoa.o: \
    src/core/src/dtoa.c $(DEPS_48)
	@echo '   [Compile] $(CONFIG)/obj/dtoa.o'
	$(CC) -c -o $(CONFIG)/obj/dtoa.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/dtoa.c

#
#   ejsApp.o
#
DEPS_49 += $(CONFIG)/inc/me.h
DEPS_49 += $(CONFIG)/inc/mpr.h
DEPS_49 += $(CONFIG)/inc/http.h
DEPS_49 += $(CONFIG)/inc/ejsByteCode.h
DEPS_49 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_49 += $(CONFIG)/inc/ejs.slots.h
DEPS_49 += $(CONFIG)/inc/ejsCustomize.h
DEPS_49 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsApp.o: \
    src/core/src/ejsApp.c $(DEPS_49)
	@echo '   [Compile] $(CONFIG)/obj/ejsApp.o'
	$(CC) -c -o $(CONFIG)/obj/ejsApp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsApp.c

#
#   ejsArray.o
#
DEPS_50 += $(CONFIG)/inc/me.h
DEPS_50 += $(CONFIG)/inc/mpr.h
DEPS_50 += $(CONFIG)/inc/http.h
DEPS_50 += $(CONFIG)/inc/ejsByteCode.h
DEPS_50 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_50 += $(CONFIG)/inc/ejs.slots.h
DEPS_50 += $(CONFIG)/inc/ejsCustomize.h
DEPS_50 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsArray.o: \
    src/core/src/ejsArray.c $(DEPS_50)
	@echo '   [Compile] $(CONFIG)/obj/ejsArray.o'
	$(CC) -c -o $(CONFIG)/obj/ejsArray.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsArray.c

#
#   ejsBlock.o
#
DEPS_51 += $(CONFIG)/inc/me.h
DEPS_51 += $(CONFIG)/inc/mpr.h
DEPS_51 += $(CONFIG)/inc/http.h
DEPS_51 += $(CONFIG)/inc/ejsByteCode.h
DEPS_51 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_51 += $(CONFIG)/inc/ejs.slots.h
DEPS_51 += $(CONFIG)/inc/ejsCustomize.h
DEPS_51 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsBlock.o: \
    src/core/src/ejsBlock.c $(DEPS_51)
	@echo '   [Compile] $(CONFIG)/obj/ejsBlock.o'
	$(CC) -c -o $(CONFIG)/obj/ejsBlock.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBlock.c

#
#   ejsBoolean.o
#
DEPS_52 += $(CONFIG)/inc/me.h
DEPS_52 += $(CONFIG)/inc/mpr.h
DEPS_52 += $(CONFIG)/inc/http.h
DEPS_52 += $(CONFIG)/inc/ejsByteCode.h
DEPS_52 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_52 += $(CONFIG)/inc/ejs.slots.h
DEPS_52 += $(CONFIG)/inc/ejsCustomize.h
DEPS_52 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsBoolean.o: \
    src/core/src/ejsBoolean.c $(DEPS_52)
	@echo '   [Compile] $(CONFIG)/obj/ejsBoolean.o'
	$(CC) -c -o $(CONFIG)/obj/ejsBoolean.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsBoolean.c

#
#   ejsByteArray.o
#
DEPS_53 += $(CONFIG)/inc/me.h
DEPS_53 += $(CONFIG)/inc/mpr.h
DEPS_53 += $(CONFIG)/inc/http.h
DEPS_53 += $(CONFIG)/inc/ejsByteCode.h
DEPS_53 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_53 += $(CONFIG)/inc/ejs.slots.h
DEPS_53 += $(CONFIG)/inc/ejsCustomize.h
DEPS_53 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsByteArray.o: \
    src/core/src/ejsByteArray.c $(DEPS_53)
	@echo '   [Compile] $(CONFIG)/obj/ejsByteArray.o'
	$(CC) -c -o $(CONFIG)/obj/ejsByteArray.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsByteArray.c

#
#   ejsCache.o
#
DEPS_54 += $(CONFIG)/inc/me.h
DEPS_54 += $(CONFIG)/inc/mpr.h
DEPS_54 += $(CONFIG)/inc/http.h
DEPS_54 += $(CONFIG)/inc/ejsByteCode.h
DEPS_54 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_54 += $(CONFIG)/inc/ejs.slots.h
DEPS_54 += $(CONFIG)/inc/ejsCustomize.h
DEPS_54 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsCache.o: \
    src/core/src/ejsCache.c $(DEPS_54)
	@echo '   [Compile] $(CONFIG)/obj/ejsCache.o'
	$(CC) -c -o $(CONFIG)/obj/ejsCache.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCache.c

#
#   ejsCmd.o
#
DEPS_55 += $(CONFIG)/inc/me.h
DEPS_55 += $(CONFIG)/inc/mpr.h
DEPS_55 += $(CONFIG)/inc/http.h
DEPS_55 += $(CONFIG)/inc/ejsByteCode.h
DEPS_55 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_55 += $(CONFIG)/inc/ejs.slots.h
DEPS_55 += $(CONFIG)/inc/ejsCustomize.h
DEPS_55 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsCmd.o: \
    src/core/src/ejsCmd.c $(DEPS_55)
	@echo '   [Compile] $(CONFIG)/obj/ejsCmd.o'
	$(CC) -c -o $(CONFIG)/obj/ejsCmd.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsCmd.c

#
#   ejsConfig.o
#
DEPS_56 += $(CONFIG)/inc/me.h
DEPS_56 += $(CONFIG)/inc/mpr.h
DEPS_56 += $(CONFIG)/inc/http.h
DEPS_56 += $(CONFIG)/inc/ejsByteCode.h
DEPS_56 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_56 += $(CONFIG)/inc/ejs.slots.h
DEPS_56 += $(CONFIG)/inc/ejsCustomize.h
DEPS_56 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsConfig.o: \
    src/core/src/ejsConfig.c $(DEPS_56)
	@echo '   [Compile] $(CONFIG)/obj/ejsConfig.o'
	$(CC) -c -o $(CONFIG)/obj/ejsConfig.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsConfig.c

#
#   ejsDate.o
#
DEPS_57 += $(CONFIG)/inc/me.h
DEPS_57 += $(CONFIG)/inc/mpr.h
DEPS_57 += $(CONFIG)/inc/http.h
DEPS_57 += $(CONFIG)/inc/ejsByteCode.h
DEPS_57 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_57 += $(CONFIG)/inc/ejs.slots.h
DEPS_57 += $(CONFIG)/inc/ejsCustomize.h
DEPS_57 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsDate.o: \
    src/core/src/ejsDate.c $(DEPS_57)
	@echo '   [Compile] $(CONFIG)/obj/ejsDate.o'
	$(CC) -c -o $(CONFIG)/obj/ejsDate.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDate.c

#
#   ejsDebug.o
#
DEPS_58 += $(CONFIG)/inc/me.h
DEPS_58 += $(CONFIG)/inc/mpr.h
DEPS_58 += $(CONFIG)/inc/http.h
DEPS_58 += $(CONFIG)/inc/ejsByteCode.h
DEPS_58 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_58 += $(CONFIG)/inc/ejs.slots.h
DEPS_58 += $(CONFIG)/inc/ejsCustomize.h
DEPS_58 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsDebug.o: \
    src/core/src/ejsDebug.c $(DEPS_58)
	@echo '   [Compile] $(CONFIG)/obj/ejsDebug.o'
	$(CC) -c -o $(CONFIG)/obj/ejsDebug.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsDebug.c

#
#   ejsError.o
#
DEPS_59 += $(CONFIG)/inc/me.h
DEPS_59 += $(CONFIG)/inc/mpr.h
DEPS_59 += $(CONFIG)/inc/http.h
DEPS_59 += $(CONFIG)/inc/ejsByteCode.h
DEPS_59 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_59 += $(CONFIG)/inc/ejs.slots.h
DEPS_59 += $(CONFIG)/inc/ejsCustomize.h
DEPS_59 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsError.o: \
    src/core/src/ejsError.c $(DEPS_59)
	@echo '   [Compile] $(CONFIG)/obj/ejsError.o'
	$(CC) -c -o $(CONFIG)/obj/ejsError.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsError.c

#
#   ejsFile.o
#
DEPS_60 += $(CONFIG)/inc/me.h
DEPS_60 += $(CONFIG)/inc/mpr.h
DEPS_60 += $(CONFIG)/inc/http.h
DEPS_60 += $(CONFIG)/inc/ejsByteCode.h
DEPS_60 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_60 += $(CONFIG)/inc/ejs.slots.h
DEPS_60 += $(CONFIG)/inc/ejsCustomize.h
DEPS_60 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFile.o: \
    src/core/src/ejsFile.c $(DEPS_60)
	@echo '   [Compile] $(CONFIG)/obj/ejsFile.o'
	$(CC) -c -o $(CONFIG)/obj/ejsFile.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFile.c

#
#   ejsFileSystem.o
#
DEPS_61 += $(CONFIG)/inc/me.h
DEPS_61 += $(CONFIG)/inc/mpr.h
DEPS_61 += $(CONFIG)/inc/http.h
DEPS_61 += $(CONFIG)/inc/ejsByteCode.h
DEPS_61 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_61 += $(CONFIG)/inc/ejs.slots.h
DEPS_61 += $(CONFIG)/inc/ejsCustomize.h
DEPS_61 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFileSystem.o: \
    src/core/src/ejsFileSystem.c $(DEPS_61)
	@echo '   [Compile] $(CONFIG)/obj/ejsFileSystem.o'
	$(CC) -c -o $(CONFIG)/obj/ejsFileSystem.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFileSystem.c

#
#   ejsFrame.o
#
DEPS_62 += $(CONFIG)/inc/me.h
DEPS_62 += $(CONFIG)/inc/mpr.h
DEPS_62 += $(CONFIG)/inc/http.h
DEPS_62 += $(CONFIG)/inc/ejsByteCode.h
DEPS_62 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_62 += $(CONFIG)/inc/ejs.slots.h
DEPS_62 += $(CONFIG)/inc/ejsCustomize.h
DEPS_62 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFrame.o: \
    src/core/src/ejsFrame.c $(DEPS_62)
	@echo '   [Compile] $(CONFIG)/obj/ejsFrame.o'
	$(CC) -c -o $(CONFIG)/obj/ejsFrame.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFrame.c

#
#   ejsFunction.o
#
DEPS_63 += $(CONFIG)/inc/me.h
DEPS_63 += $(CONFIG)/inc/mpr.h
DEPS_63 += $(CONFIG)/inc/http.h
DEPS_63 += $(CONFIG)/inc/ejsByteCode.h
DEPS_63 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_63 += $(CONFIG)/inc/ejs.slots.h
DEPS_63 += $(CONFIG)/inc/ejsCustomize.h
DEPS_63 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsFunction.o: \
    src/core/src/ejsFunction.c $(DEPS_63)
	@echo '   [Compile] $(CONFIG)/obj/ejsFunction.o'
	$(CC) -c -o $(CONFIG)/obj/ejsFunction.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsFunction.c

#
#   ejsGC.o
#
DEPS_64 += $(CONFIG)/inc/me.h
DEPS_64 += $(CONFIG)/inc/mpr.h
DEPS_64 += $(CONFIG)/inc/http.h
DEPS_64 += $(CONFIG)/inc/ejsByteCode.h
DEPS_64 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_64 += $(CONFIG)/inc/ejs.slots.h
DEPS_64 += $(CONFIG)/inc/ejsCustomize.h
DEPS_64 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsGC.o: \
    src/core/src/ejsGC.c $(DEPS_64)
	@echo '   [Compile] $(CONFIG)/obj/ejsGC.o'
	$(CC) -c -o $(CONFIG)/obj/ejsGC.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGC.c

#
#   ejsGlobal.o
#
DEPS_65 += $(CONFIG)/inc/me.h
DEPS_65 += $(CONFIG)/inc/mpr.h
DEPS_65 += $(CONFIG)/inc/http.h
DEPS_65 += $(CONFIG)/inc/ejsByteCode.h
DEPS_65 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_65 += $(CONFIG)/inc/ejs.slots.h
DEPS_65 += $(CONFIG)/inc/ejsCustomize.h
DEPS_65 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsGlobal.o: \
    src/core/src/ejsGlobal.c $(DEPS_65)
	@echo '   [Compile] $(CONFIG)/obj/ejsGlobal.o'
	$(CC) -c -o $(CONFIG)/obj/ejsGlobal.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsGlobal.c

#
#   ejsHttp.o
#
DEPS_66 += $(CONFIG)/inc/me.h
DEPS_66 += $(CONFIG)/inc/mpr.h
DEPS_66 += $(CONFIG)/inc/http.h
DEPS_66 += $(CONFIG)/inc/ejsByteCode.h
DEPS_66 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_66 += $(CONFIG)/inc/ejs.slots.h
DEPS_66 += $(CONFIG)/inc/ejsCustomize.h
DEPS_66 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsHttp.o: \
    src/core/src/ejsHttp.c $(DEPS_66)
	@echo '   [Compile] $(CONFIG)/obj/ejsHttp.o'
	$(CC) -c -o $(CONFIG)/obj/ejsHttp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsHttp.c

#
#   ejsIterator.o
#
DEPS_67 += $(CONFIG)/inc/me.h
DEPS_67 += $(CONFIG)/inc/mpr.h
DEPS_67 += $(CONFIG)/inc/http.h
DEPS_67 += $(CONFIG)/inc/ejsByteCode.h
DEPS_67 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_67 += $(CONFIG)/inc/ejs.slots.h
DEPS_67 += $(CONFIG)/inc/ejsCustomize.h
DEPS_67 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsIterator.o: \
    src/core/src/ejsIterator.c $(DEPS_67)
	@echo '   [Compile] $(CONFIG)/obj/ejsIterator.o'
	$(CC) -c -o $(CONFIG)/obj/ejsIterator.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsIterator.c

#
#   ejsJSON.o
#
DEPS_68 += $(CONFIG)/inc/me.h
DEPS_68 += $(CONFIG)/inc/mpr.h
DEPS_68 += $(CONFIG)/inc/http.h
DEPS_68 += $(CONFIG)/inc/ejsByteCode.h
DEPS_68 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_68 += $(CONFIG)/inc/ejs.slots.h
DEPS_68 += $(CONFIG)/inc/ejsCustomize.h
DEPS_68 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsJSON.o: \
    src/core/src/ejsJSON.c $(DEPS_68)
	@echo '   [Compile] $(CONFIG)/obj/ejsJSON.o'
	$(CC) -c -o $(CONFIG)/obj/ejsJSON.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsJSON.c

#
#   ejsLocalCache.o
#
DEPS_69 += $(CONFIG)/inc/me.h
DEPS_69 += $(CONFIG)/inc/mpr.h
DEPS_69 += $(CONFIG)/inc/http.h
DEPS_69 += $(CONFIG)/inc/ejsByteCode.h
DEPS_69 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_69 += $(CONFIG)/inc/ejs.slots.h
DEPS_69 += $(CONFIG)/inc/ejsCustomize.h
DEPS_69 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsLocalCache.o: \
    src/core/src/ejsLocalCache.c $(DEPS_69)
	@echo '   [Compile] $(CONFIG)/obj/ejsLocalCache.o'
	$(CC) -c -o $(CONFIG)/obj/ejsLocalCache.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsLocalCache.c

#
#   ejsMath.o
#
DEPS_70 += $(CONFIG)/inc/me.h
DEPS_70 += $(CONFIG)/inc/mpr.h
DEPS_70 += $(CONFIG)/inc/http.h
DEPS_70 += $(CONFIG)/inc/ejsByteCode.h
DEPS_70 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_70 += $(CONFIG)/inc/ejs.slots.h
DEPS_70 += $(CONFIG)/inc/ejsCustomize.h
DEPS_70 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMath.o: \
    src/core/src/ejsMath.c $(DEPS_70)
	@echo '   [Compile] $(CONFIG)/obj/ejsMath.o'
	$(CC) -c -o $(CONFIG)/obj/ejsMath.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMath.c

#
#   ejsMemory.o
#
DEPS_71 += $(CONFIG)/inc/me.h
DEPS_71 += $(CONFIG)/inc/mpr.h
DEPS_71 += $(CONFIG)/inc/http.h
DEPS_71 += $(CONFIG)/inc/ejsByteCode.h
DEPS_71 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_71 += $(CONFIG)/inc/ejs.slots.h
DEPS_71 += $(CONFIG)/inc/ejsCustomize.h
DEPS_71 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMemory.o: \
    src/core/src/ejsMemory.c $(DEPS_71)
	@echo '   [Compile] $(CONFIG)/obj/ejsMemory.o'
	$(CC) -c -o $(CONFIG)/obj/ejsMemory.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMemory.c

#
#   ejsMprLog.o
#
DEPS_72 += $(CONFIG)/inc/me.h
DEPS_72 += $(CONFIG)/inc/mpr.h
DEPS_72 += $(CONFIG)/inc/http.h
DEPS_72 += $(CONFIG)/inc/ejsByteCode.h
DEPS_72 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_72 += $(CONFIG)/inc/ejs.slots.h
DEPS_72 += $(CONFIG)/inc/ejsCustomize.h
DEPS_72 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsMprLog.o: \
    src/core/src/ejsMprLog.c $(DEPS_72)
	@echo '   [Compile] $(CONFIG)/obj/ejsMprLog.o'
	$(CC) -c -o $(CONFIG)/obj/ejsMprLog.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsMprLog.c

#
#   ejsNamespace.o
#
DEPS_73 += $(CONFIG)/inc/me.h
DEPS_73 += $(CONFIG)/inc/mpr.h
DEPS_73 += $(CONFIG)/inc/http.h
DEPS_73 += $(CONFIG)/inc/ejsByteCode.h
DEPS_73 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_73 += $(CONFIG)/inc/ejs.slots.h
DEPS_73 += $(CONFIG)/inc/ejsCustomize.h
DEPS_73 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNamespace.o: \
    src/core/src/ejsNamespace.c $(DEPS_73)
	@echo '   [Compile] $(CONFIG)/obj/ejsNamespace.o'
	$(CC) -c -o $(CONFIG)/obj/ejsNamespace.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNamespace.c

#
#   ejsNull.o
#
DEPS_74 += $(CONFIG)/inc/me.h
DEPS_74 += $(CONFIG)/inc/mpr.h
DEPS_74 += $(CONFIG)/inc/http.h
DEPS_74 += $(CONFIG)/inc/ejsByteCode.h
DEPS_74 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_74 += $(CONFIG)/inc/ejs.slots.h
DEPS_74 += $(CONFIG)/inc/ejsCustomize.h
DEPS_74 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNull.o: \
    src/core/src/ejsNull.c $(DEPS_74)
	@echo '   [Compile] $(CONFIG)/obj/ejsNull.o'
	$(CC) -c -o $(CONFIG)/obj/ejsNull.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNull.c

#
#   ejsNumber.o
#
DEPS_75 += $(CONFIG)/inc/me.h
DEPS_75 += $(CONFIG)/inc/mpr.h
DEPS_75 += $(CONFIG)/inc/http.h
DEPS_75 += $(CONFIG)/inc/ejsByteCode.h
DEPS_75 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_75 += $(CONFIG)/inc/ejs.slots.h
DEPS_75 += $(CONFIG)/inc/ejsCustomize.h
DEPS_75 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsNumber.o: \
    src/core/src/ejsNumber.c $(DEPS_75)
	@echo '   [Compile] $(CONFIG)/obj/ejsNumber.o'
	$(CC) -c -o $(CONFIG)/obj/ejsNumber.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsNumber.c

#
#   ejsObject.o
#
DEPS_76 += $(CONFIG)/inc/me.h
DEPS_76 += $(CONFIG)/inc/mpr.h
DEPS_76 += $(CONFIG)/inc/http.h
DEPS_76 += $(CONFIG)/inc/ejsByteCode.h
DEPS_76 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_76 += $(CONFIG)/inc/ejs.slots.h
DEPS_76 += $(CONFIG)/inc/ejsCustomize.h
DEPS_76 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsObject.o: \
    src/core/src/ejsObject.c $(DEPS_76)
	@echo '   [Compile] $(CONFIG)/obj/ejsObject.o'
	$(CC) -c -o $(CONFIG)/obj/ejsObject.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsObject.c

#
#   ejsPath.o
#
DEPS_77 += $(CONFIG)/inc/me.h
DEPS_77 += $(CONFIG)/inc/mpr.h
DEPS_77 += $(CONFIG)/inc/http.h
DEPS_77 += $(CONFIG)/inc/ejsByteCode.h
DEPS_77 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_77 += $(CONFIG)/inc/ejs.slots.h
DEPS_77 += $(CONFIG)/inc/ejsCustomize.h
DEPS_77 += $(CONFIG)/inc/ejs.h
DEPS_77 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsPath.o: \
    src/core/src/ejsPath.c $(DEPS_77)
	@echo '   [Compile] $(CONFIG)/obj/ejsPath.o'
	$(CC) -c -o $(CONFIG)/obj/ejsPath.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPath.c

#
#   ejsPot.o
#
DEPS_78 += $(CONFIG)/inc/me.h
DEPS_78 += $(CONFIG)/inc/mpr.h
DEPS_78 += $(CONFIG)/inc/http.h
DEPS_78 += $(CONFIG)/inc/ejsByteCode.h
DEPS_78 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_78 += $(CONFIG)/inc/ejs.slots.h
DEPS_78 += $(CONFIG)/inc/ejsCustomize.h
DEPS_78 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsPot.o: \
    src/core/src/ejsPot.c $(DEPS_78)
	@echo '   [Compile] $(CONFIG)/obj/ejsPot.o'
	$(CC) -c -o $(CONFIG)/obj/ejsPot.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsPot.c

#
#   ejsRegExp.o
#
DEPS_79 += $(CONFIG)/inc/me.h
DEPS_79 += $(CONFIG)/inc/mpr.h
DEPS_79 += $(CONFIG)/inc/http.h
DEPS_79 += $(CONFIG)/inc/ejsByteCode.h
DEPS_79 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_79 += $(CONFIG)/inc/ejs.slots.h
DEPS_79 += $(CONFIG)/inc/ejsCustomize.h
DEPS_79 += $(CONFIG)/inc/ejs.h
DEPS_79 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsRegExp.o: \
    src/core/src/ejsRegExp.c $(DEPS_79)
	@echo '   [Compile] $(CONFIG)/obj/ejsRegExp.o'
	$(CC) -c -o $(CONFIG)/obj/ejsRegExp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsRegExp.c

#
#   ejsSocket.o
#
DEPS_80 += $(CONFIG)/inc/me.h
DEPS_80 += $(CONFIG)/inc/mpr.h
DEPS_80 += $(CONFIG)/inc/http.h
DEPS_80 += $(CONFIG)/inc/ejsByteCode.h
DEPS_80 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_80 += $(CONFIG)/inc/ejs.slots.h
DEPS_80 += $(CONFIG)/inc/ejsCustomize.h
DEPS_80 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsSocket.o: \
    src/core/src/ejsSocket.c $(DEPS_80)
	@echo '   [Compile] $(CONFIG)/obj/ejsSocket.o'
	$(CC) -c -o $(CONFIG)/obj/ejsSocket.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSocket.c

#
#   ejsString.o
#
DEPS_81 += $(CONFIG)/inc/me.h
DEPS_81 += $(CONFIG)/inc/mpr.h
DEPS_81 += $(CONFIG)/inc/http.h
DEPS_81 += $(CONFIG)/inc/ejsByteCode.h
DEPS_81 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_81 += $(CONFIG)/inc/ejs.slots.h
DEPS_81 += $(CONFIG)/inc/ejsCustomize.h
DEPS_81 += $(CONFIG)/inc/ejs.h
DEPS_81 += $(CONFIG)/inc/pcre.h

$(CONFIG)/obj/ejsString.o: \
    src/core/src/ejsString.c $(DEPS_81)
	@echo '   [Compile] $(CONFIG)/obj/ejsString.o'
	$(CC) -c -o $(CONFIG)/obj/ejsString.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsString.c

#
#   ejsSystem.o
#
DEPS_82 += $(CONFIG)/inc/me.h
DEPS_82 += $(CONFIG)/inc/mpr.h
DEPS_82 += $(CONFIG)/inc/http.h
DEPS_82 += $(CONFIG)/inc/ejsByteCode.h
DEPS_82 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_82 += $(CONFIG)/inc/ejs.slots.h
DEPS_82 += $(CONFIG)/inc/ejsCustomize.h
DEPS_82 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsSystem.o: \
    src/core/src/ejsSystem.c $(DEPS_82)
	@echo '   [Compile] $(CONFIG)/obj/ejsSystem.o'
	$(CC) -c -o $(CONFIG)/obj/ejsSystem.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsSystem.c

#
#   ejsTimer.o
#
DEPS_83 += $(CONFIG)/inc/me.h
DEPS_83 += $(CONFIG)/inc/mpr.h
DEPS_83 += $(CONFIG)/inc/http.h
DEPS_83 += $(CONFIG)/inc/ejsByteCode.h
DEPS_83 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_83 += $(CONFIG)/inc/ejs.slots.h
DEPS_83 += $(CONFIG)/inc/ejsCustomize.h
DEPS_83 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsTimer.o: \
    src/core/src/ejsTimer.c $(DEPS_83)
	@echo '   [Compile] $(CONFIG)/obj/ejsTimer.o'
	$(CC) -c -o $(CONFIG)/obj/ejsTimer.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsTimer.c

#
#   ejsType.o
#
DEPS_84 += $(CONFIG)/inc/me.h
DEPS_84 += $(CONFIG)/inc/mpr.h
DEPS_84 += $(CONFIG)/inc/http.h
DEPS_84 += $(CONFIG)/inc/ejsByteCode.h
DEPS_84 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_84 += $(CONFIG)/inc/ejs.slots.h
DEPS_84 += $(CONFIG)/inc/ejsCustomize.h
DEPS_84 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsType.o: \
    src/core/src/ejsType.c $(DEPS_84)
	@echo '   [Compile] $(CONFIG)/obj/ejsType.o'
	$(CC) -c -o $(CONFIG)/obj/ejsType.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsType.c

#
#   ejsUri.o
#
DEPS_85 += $(CONFIG)/inc/me.h
DEPS_85 += $(CONFIG)/inc/mpr.h
DEPS_85 += $(CONFIG)/inc/http.h
DEPS_85 += $(CONFIG)/inc/ejsByteCode.h
DEPS_85 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_85 += $(CONFIG)/inc/ejs.slots.h
DEPS_85 += $(CONFIG)/inc/ejsCustomize.h
DEPS_85 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsUri.o: \
    src/core/src/ejsUri.c $(DEPS_85)
	@echo '   [Compile] $(CONFIG)/obj/ejsUri.o'
	$(CC) -c -o $(CONFIG)/obj/ejsUri.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsUri.c

#
#   ejsVoid.o
#
DEPS_86 += $(CONFIG)/inc/me.h
DEPS_86 += $(CONFIG)/inc/mpr.h
DEPS_86 += $(CONFIG)/inc/http.h
DEPS_86 += $(CONFIG)/inc/ejsByteCode.h
DEPS_86 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_86 += $(CONFIG)/inc/ejs.slots.h
DEPS_86 += $(CONFIG)/inc/ejsCustomize.h
DEPS_86 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsVoid.o: \
    src/core/src/ejsVoid.c $(DEPS_86)
	@echo '   [Compile] $(CONFIG)/obj/ejsVoid.o'
	$(CC) -c -o $(CONFIG)/obj/ejsVoid.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsVoid.c

#
#   ejsWebSocket.o
#
DEPS_87 += $(CONFIG)/inc/me.h
DEPS_87 += $(CONFIG)/inc/mpr.h
DEPS_87 += $(CONFIG)/inc/http.h
DEPS_87 += $(CONFIG)/inc/ejsByteCode.h
DEPS_87 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_87 += $(CONFIG)/inc/ejs.slots.h
DEPS_87 += $(CONFIG)/inc/ejsCustomize.h
DEPS_87 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsWebSocket.o: \
    src/core/src/ejsWebSocket.c $(DEPS_87)
	@echo '   [Compile] $(CONFIG)/obj/ejsWebSocket.o'
	$(CC) -c -o $(CONFIG)/obj/ejsWebSocket.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWebSocket.c

#
#   ejsWorker.o
#
DEPS_88 += $(CONFIG)/inc/me.h
DEPS_88 += $(CONFIG)/inc/mpr.h
DEPS_88 += $(CONFIG)/inc/http.h
DEPS_88 += $(CONFIG)/inc/ejsByteCode.h
DEPS_88 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_88 += $(CONFIG)/inc/ejs.slots.h
DEPS_88 += $(CONFIG)/inc/ejsCustomize.h
DEPS_88 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsWorker.o: \
    src/core/src/ejsWorker.c $(DEPS_88)
	@echo '   [Compile] $(CONFIG)/obj/ejsWorker.o'
	$(CC) -c -o $(CONFIG)/obj/ejsWorker.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsWorker.c

#
#   ejsXML.o
#
DEPS_89 += $(CONFIG)/inc/me.h
DEPS_89 += $(CONFIG)/inc/mpr.h
DEPS_89 += $(CONFIG)/inc/http.h
DEPS_89 += $(CONFIG)/inc/ejsByteCode.h
DEPS_89 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_89 += $(CONFIG)/inc/ejs.slots.h
DEPS_89 += $(CONFIG)/inc/ejsCustomize.h
DEPS_89 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXML.o: \
    src/core/src/ejsXML.c $(DEPS_89)
	@echo '   [Compile] $(CONFIG)/obj/ejsXML.o'
	$(CC) -c -o $(CONFIG)/obj/ejsXML.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXML.c

#
#   ejsXMLList.o
#
DEPS_90 += $(CONFIG)/inc/me.h
DEPS_90 += $(CONFIG)/inc/mpr.h
DEPS_90 += $(CONFIG)/inc/http.h
DEPS_90 += $(CONFIG)/inc/ejsByteCode.h
DEPS_90 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_90 += $(CONFIG)/inc/ejs.slots.h
DEPS_90 += $(CONFIG)/inc/ejsCustomize.h
DEPS_90 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXMLList.o: \
    src/core/src/ejsXMLList.c $(DEPS_90)
	@echo '   [Compile] $(CONFIG)/obj/ejsXMLList.o'
	$(CC) -c -o $(CONFIG)/obj/ejsXMLList.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLList.c

#
#   ejsXMLLoader.o
#
DEPS_91 += $(CONFIG)/inc/me.h
DEPS_91 += $(CONFIG)/inc/mpr.h
DEPS_91 += $(CONFIG)/inc/http.h
DEPS_91 += $(CONFIG)/inc/ejsByteCode.h
DEPS_91 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_91 += $(CONFIG)/inc/ejs.slots.h
DEPS_91 += $(CONFIG)/inc/ejsCustomize.h
DEPS_91 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsXMLLoader.o: \
    src/core/src/ejsXMLLoader.c $(DEPS_91)
	@echo '   [Compile] $(CONFIG)/obj/ejsXMLLoader.o'
	$(CC) -c -o $(CONFIG)/obj/ejsXMLLoader.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/core/src/ejsXMLLoader.c

#
#   ejsByteCode.o
#
DEPS_92 += $(CONFIG)/inc/me.h
DEPS_92 += $(CONFIG)/inc/mpr.h
DEPS_92 += $(CONFIG)/inc/http.h
DEPS_92 += $(CONFIG)/inc/ejsByteCode.h
DEPS_92 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_92 += $(CONFIG)/inc/ejs.slots.h
DEPS_92 += $(CONFIG)/inc/ejsCustomize.h
DEPS_92 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsByteCode.o: \
    src/vm/ejsByteCode.c $(DEPS_92)
	@echo '   [Compile] $(CONFIG)/obj/ejsByteCode.o'
	$(CC) -c -o $(CONFIG)/obj/ejsByteCode.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsByteCode.c

#
#   ejsException.o
#
DEPS_93 += $(CONFIG)/inc/me.h
DEPS_93 += $(CONFIG)/inc/mpr.h
DEPS_93 += $(CONFIG)/inc/http.h
DEPS_93 += $(CONFIG)/inc/ejsByteCode.h
DEPS_93 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_93 += $(CONFIG)/inc/ejs.slots.h
DEPS_93 += $(CONFIG)/inc/ejsCustomize.h
DEPS_93 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsException.o: \
    src/vm/ejsException.c $(DEPS_93)
	@echo '   [Compile] $(CONFIG)/obj/ejsException.o'
	$(CC) -c -o $(CONFIG)/obj/ejsException.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsException.c

#
#   ejsHelper.o
#
DEPS_94 += $(CONFIG)/inc/me.h
DEPS_94 += $(CONFIG)/inc/mpr.h
DEPS_94 += $(CONFIG)/inc/http.h
DEPS_94 += $(CONFIG)/inc/ejsByteCode.h
DEPS_94 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_94 += $(CONFIG)/inc/ejs.slots.h
DEPS_94 += $(CONFIG)/inc/ejsCustomize.h
DEPS_94 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsHelper.o: \
    src/vm/ejsHelper.c $(DEPS_94)
	@echo '   [Compile] $(CONFIG)/obj/ejsHelper.o'
	$(CC) -c -o $(CONFIG)/obj/ejsHelper.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsHelper.c

#
#   ejsInterp.o
#
DEPS_95 += $(CONFIG)/inc/me.h
DEPS_95 += $(CONFIG)/inc/mpr.h
DEPS_95 += $(CONFIG)/inc/http.h
DEPS_95 += $(CONFIG)/inc/ejsByteCode.h
DEPS_95 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_95 += $(CONFIG)/inc/ejs.slots.h
DEPS_95 += $(CONFIG)/inc/ejsCustomize.h
DEPS_95 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsInterp.o: \
    src/vm/ejsInterp.c $(DEPS_95)
	@echo '   [Compile] $(CONFIG)/obj/ejsInterp.o'
	$(CC) -c -o $(CONFIG)/obj/ejsInterp.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsInterp.c

#
#   ejsLoader.o
#
DEPS_96 += $(CONFIG)/inc/me.h
DEPS_96 += $(CONFIG)/inc/mpr.h
DEPS_96 += $(CONFIG)/inc/http.h
DEPS_96 += $(CONFIG)/inc/ejsByteCode.h
DEPS_96 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_96 += $(CONFIG)/inc/ejs.slots.h
DEPS_96 += $(CONFIG)/inc/ejsCustomize.h
DEPS_96 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsLoader.o: \
    src/vm/ejsLoader.c $(DEPS_96)
	@echo '   [Compile] $(CONFIG)/obj/ejsLoader.o'
	$(CC) -c -o $(CONFIG)/obj/ejsLoader.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsLoader.c

#
#   ejsModule.o
#
DEPS_97 += $(CONFIG)/inc/me.h
DEPS_97 += $(CONFIG)/inc/mpr.h
DEPS_97 += $(CONFIG)/inc/http.h
DEPS_97 += $(CONFIG)/inc/ejsByteCode.h
DEPS_97 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_97 += $(CONFIG)/inc/ejs.slots.h
DEPS_97 += $(CONFIG)/inc/ejsCustomize.h
DEPS_97 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsModule.o: \
    src/vm/ejsModule.c $(DEPS_97)
	@echo '   [Compile] $(CONFIG)/obj/ejsModule.o'
	$(CC) -c -o $(CONFIG)/obj/ejsModule.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsModule.c

#
#   ejsScope.o
#
DEPS_98 += $(CONFIG)/inc/me.h
DEPS_98 += $(CONFIG)/inc/mpr.h
DEPS_98 += $(CONFIG)/inc/http.h
DEPS_98 += $(CONFIG)/inc/ejsByteCode.h
DEPS_98 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_98 += $(CONFIG)/inc/ejs.slots.h
DEPS_98 += $(CONFIG)/inc/ejsCustomize.h
DEPS_98 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsScope.o: \
    src/vm/ejsScope.c $(DEPS_98)
	@echo '   [Compile] $(CONFIG)/obj/ejsScope.o'
	$(CC) -c -o $(CONFIG)/obj/ejsScope.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsScope.c

#
#   ejsService.o
#
DEPS_99 += $(CONFIG)/inc/me.h
DEPS_99 += $(CONFIG)/inc/mpr.h
DEPS_99 += $(CONFIG)/inc/http.h
DEPS_99 += $(CONFIG)/inc/ejsByteCode.h
DEPS_99 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_99 += $(CONFIG)/inc/ejs.slots.h
DEPS_99 += $(CONFIG)/inc/ejsCustomize.h
DEPS_99 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsService.o: \
    src/vm/ejsService.c $(DEPS_99)
	@echo '   [Compile] $(CONFIG)/obj/ejsService.o'
	$(CC) -c -o $(CONFIG)/obj/ejsService.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/vm/ejsService.c

#
#   libejs
#
DEPS_100 += $(CONFIG)/inc/mpr.h
DEPS_100 += $(CONFIG)/inc/me.h
DEPS_100 += $(CONFIG)/inc/osdep.h
DEPS_100 += $(CONFIG)/obj/mprLib.o
DEPS_100 += $(CONFIG)/bin/libmpr.a
DEPS_100 += $(CONFIG)/inc/pcre.h
DEPS_100 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_100 += $(CONFIG)/bin/libpcre.a
endif
DEPS_100 += $(CONFIG)/inc/http.h
DEPS_100 += $(CONFIG)/obj/httpLib.o
DEPS_100 += $(CONFIG)/bin/libhttp.a
DEPS_100 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_100 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_100 += $(CONFIG)/inc/ejs.slots.h
DEPS_100 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_100 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_100 += $(CONFIG)/inc/ejsByteCode.h
DEPS_100 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_100 += $(CONFIG)/inc/ejsCustomize.h
DEPS_100 += $(CONFIG)/inc/ejs.h
DEPS_100 += $(CONFIG)/inc/ejsCompiler.h
DEPS_100 += $(CONFIG)/obj/ecAst.o
DEPS_100 += $(CONFIG)/obj/ecCodeGen.o
DEPS_100 += $(CONFIG)/obj/ecCompiler.o
DEPS_100 += $(CONFIG)/obj/ecLex.o
DEPS_100 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_100 += $(CONFIG)/obj/ecParser.o
DEPS_100 += $(CONFIG)/obj/ecState.o
DEPS_100 += $(CONFIG)/obj/dtoa.o
DEPS_100 += $(CONFIG)/obj/ejsApp.o
DEPS_100 += $(CONFIG)/obj/ejsArray.o
DEPS_100 += $(CONFIG)/obj/ejsBlock.o
DEPS_100 += $(CONFIG)/obj/ejsBoolean.o
DEPS_100 += $(CONFIG)/obj/ejsByteArray.o
DEPS_100 += $(CONFIG)/obj/ejsCache.o
DEPS_100 += $(CONFIG)/obj/ejsCmd.o
DEPS_100 += $(CONFIG)/obj/ejsConfig.o
DEPS_100 += $(CONFIG)/obj/ejsDate.o
DEPS_100 += $(CONFIG)/obj/ejsDebug.o
DEPS_100 += $(CONFIG)/obj/ejsError.o
DEPS_100 += $(CONFIG)/obj/ejsFile.o
DEPS_100 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_100 += $(CONFIG)/obj/ejsFrame.o
DEPS_100 += $(CONFIG)/obj/ejsFunction.o
DEPS_100 += $(CONFIG)/obj/ejsGC.o
DEPS_100 += $(CONFIG)/obj/ejsGlobal.o
DEPS_100 += $(CONFIG)/obj/ejsHttp.o
DEPS_100 += $(CONFIG)/obj/ejsIterator.o
DEPS_100 += $(CONFIG)/obj/ejsJSON.o
DEPS_100 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_100 += $(CONFIG)/obj/ejsMath.o
DEPS_100 += $(CONFIG)/obj/ejsMemory.o
DEPS_100 += $(CONFIG)/obj/ejsMprLog.o
DEPS_100 += $(CONFIG)/obj/ejsNamespace.o
DEPS_100 += $(CONFIG)/obj/ejsNull.o
DEPS_100 += $(CONFIG)/obj/ejsNumber.o
DEPS_100 += $(CONFIG)/obj/ejsObject.o
DEPS_100 += $(CONFIG)/obj/ejsPath.o
DEPS_100 += $(CONFIG)/obj/ejsPot.o
DEPS_100 += $(CONFIG)/obj/ejsRegExp.o
DEPS_100 += $(CONFIG)/obj/ejsSocket.o
DEPS_100 += $(CONFIG)/obj/ejsString.o
DEPS_100 += $(CONFIG)/obj/ejsSystem.o
DEPS_100 += $(CONFIG)/obj/ejsTimer.o
DEPS_100 += $(CONFIG)/obj/ejsType.o
DEPS_100 += $(CONFIG)/obj/ejsUri.o
DEPS_100 += $(CONFIG)/obj/ejsVoid.o
DEPS_100 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_100 += $(CONFIG)/obj/ejsWorker.o
DEPS_100 += $(CONFIG)/obj/ejsXML.o
DEPS_100 += $(CONFIG)/obj/ejsXMLList.o
DEPS_100 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_100 += $(CONFIG)/obj/ejsByteCode.o
DEPS_100 += $(CONFIG)/obj/ejsException.o
DEPS_100 += $(CONFIG)/obj/ejsHelper.o
DEPS_100 += $(CONFIG)/obj/ejsInterp.o
DEPS_100 += $(CONFIG)/obj/ejsLoader.o
DEPS_100 += $(CONFIG)/obj/ejsModule.o
DEPS_100 += $(CONFIG)/obj/ejsScope.o
DEPS_100 += $(CONFIG)/obj/ejsService.o

$(CONFIG)/bin/libejs.a: $(DEPS_100)
	@echo '      [Link] $(CONFIG)/bin/libejs.a'
	ar -cr $(CONFIG)/bin/libejs.a "$(CONFIG)/obj/ecAst.o" "$(CONFIG)/obj/ecCodeGen.o" "$(CONFIG)/obj/ecCompiler.o" "$(CONFIG)/obj/ecLex.o" "$(CONFIG)/obj/ecModuleWrite.o" "$(CONFIG)/obj/ecParser.o" "$(CONFIG)/obj/ecState.o" "$(CONFIG)/obj/dtoa.o" "$(CONFIG)/obj/ejsApp.o" "$(CONFIG)/obj/ejsArray.o" "$(CONFIG)/obj/ejsBlock.o" "$(CONFIG)/obj/ejsBoolean.o" "$(CONFIG)/obj/ejsByteArray.o" "$(CONFIG)/obj/ejsCache.o" "$(CONFIG)/obj/ejsCmd.o" "$(CONFIG)/obj/ejsConfig.o" "$(CONFIG)/obj/ejsDate.o" "$(CONFIG)/obj/ejsDebug.o" "$(CONFIG)/obj/ejsError.o" "$(CONFIG)/obj/ejsFile.o" "$(CONFIG)/obj/ejsFileSystem.o" "$(CONFIG)/obj/ejsFrame.o" "$(CONFIG)/obj/ejsFunction.o" "$(CONFIG)/obj/ejsGC.o" "$(CONFIG)/obj/ejsGlobal.o" "$(CONFIG)/obj/ejsHttp.o" "$(CONFIG)/obj/ejsIterator.o" "$(CONFIG)/obj/ejsJSON.o" "$(CONFIG)/obj/ejsLocalCache.o" "$(CONFIG)/obj/ejsMath.o" "$(CONFIG)/obj/ejsMemory.o" "$(CONFIG)/obj/ejsMprLog.o" "$(CONFIG)/obj/ejsNamespace.o" "$(CONFIG)/obj/ejsNull.o" "$(CONFIG)/obj/ejsNumber.o" "$(CONFIG)/obj/ejsObject.o" "$(CONFIG)/obj/ejsPath.o" "$(CONFIG)/obj/ejsPot.o" "$(CONFIG)/obj/ejsRegExp.o" "$(CONFIG)/obj/ejsSocket.o" "$(CONFIG)/obj/ejsString.o" "$(CONFIG)/obj/ejsSystem.o" "$(CONFIG)/obj/ejsTimer.o" "$(CONFIG)/obj/ejsType.o" "$(CONFIG)/obj/ejsUri.o" "$(CONFIG)/obj/ejsVoid.o" "$(CONFIG)/obj/ejsWebSocket.o" "$(CONFIG)/obj/ejsWorker.o" "$(CONFIG)/obj/ejsXML.o" "$(CONFIG)/obj/ejsXMLList.o" "$(CONFIG)/obj/ejsXMLLoader.o" "$(CONFIG)/obj/ejsByteCode.o" "$(CONFIG)/obj/ejsException.o" "$(CONFIG)/obj/ejsHelper.o" "$(CONFIG)/obj/ejsInterp.o" "$(CONFIG)/obj/ejsLoader.o" "$(CONFIG)/obj/ejsModule.o" "$(CONFIG)/obj/ejsScope.o" "$(CONFIG)/obj/ejsService.o"

#
#   ejs.o
#
DEPS_101 += $(CONFIG)/inc/me.h
DEPS_101 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejs.o: \
    src/cmd/ejs.c $(DEPS_101)
	@echo '   [Compile] $(CONFIG)/obj/ejs.o'
	$(CC) -c -o $(CONFIG)/obj/ejs.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejs.c

#
#   ejs
#
DEPS_102 += $(CONFIG)/inc/mpr.h
DEPS_102 += $(CONFIG)/inc/me.h
DEPS_102 += $(CONFIG)/inc/osdep.h
DEPS_102 += $(CONFIG)/obj/mprLib.o
DEPS_102 += $(CONFIG)/bin/libmpr.a
DEPS_102 += $(CONFIG)/inc/pcre.h
DEPS_102 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_102 += $(CONFIG)/bin/libpcre.a
endif
DEPS_102 += $(CONFIG)/inc/http.h
DEPS_102 += $(CONFIG)/obj/httpLib.o
DEPS_102 += $(CONFIG)/bin/libhttp.a
DEPS_102 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_102 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_102 += $(CONFIG)/inc/ejs.slots.h
DEPS_102 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_102 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_102 += $(CONFIG)/inc/ejsByteCode.h
DEPS_102 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_102 += $(CONFIG)/inc/ejsCustomize.h
DEPS_102 += $(CONFIG)/inc/ejs.h
DEPS_102 += $(CONFIG)/inc/ejsCompiler.h
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
DEPS_102 += $(CONFIG)/bin/libejs.a
DEPS_102 += $(CONFIG)/obj/ejs.o

LIBS_102 += -lejs
LIBS_102 += -lhttp
LIBS_102 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_102 += -lpcre
endif

$(CONFIG)/bin/ejs: $(DEPS_102)
	@echo '      [Link] $(CONFIG)/bin/ejs'
	$(CC) -o $(CONFIG)/bin/ejs $(LIBPATHS) "$(CONFIG)/obj/ejs.o" $(LIBPATHS_102) $(LIBS_102) $(LIBS_102) $(LIBS) $(LIBS) 

#
#   ejsc.o
#
DEPS_103 += $(CONFIG)/inc/me.h
DEPS_103 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejsc.o: \
    src/cmd/ejsc.c $(DEPS_103)
	@echo '   [Compile] $(CONFIG)/obj/ejsc.o'
	$(CC) -c -o $(CONFIG)/obj/ejsc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsc.c

#
#   ejsc
#
DEPS_104 += $(CONFIG)/inc/mpr.h
DEPS_104 += $(CONFIG)/inc/me.h
DEPS_104 += $(CONFIG)/inc/osdep.h
DEPS_104 += $(CONFIG)/obj/mprLib.o
DEPS_104 += $(CONFIG)/bin/libmpr.a
DEPS_104 += $(CONFIG)/inc/pcre.h
DEPS_104 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_104 += $(CONFIG)/bin/libpcre.a
endif
DEPS_104 += $(CONFIG)/inc/http.h
DEPS_104 += $(CONFIG)/obj/httpLib.o
DEPS_104 += $(CONFIG)/bin/libhttp.a
DEPS_104 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_104 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_104 += $(CONFIG)/inc/ejs.slots.h
DEPS_104 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_104 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_104 += $(CONFIG)/inc/ejsByteCode.h
DEPS_104 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_104 += $(CONFIG)/inc/ejsCustomize.h
DEPS_104 += $(CONFIG)/inc/ejs.h
DEPS_104 += $(CONFIG)/inc/ejsCompiler.h
DEPS_104 += $(CONFIG)/obj/ecAst.o
DEPS_104 += $(CONFIG)/obj/ecCodeGen.o
DEPS_104 += $(CONFIG)/obj/ecCompiler.o
DEPS_104 += $(CONFIG)/obj/ecLex.o
DEPS_104 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_104 += $(CONFIG)/obj/ecParser.o
DEPS_104 += $(CONFIG)/obj/ecState.o
DEPS_104 += $(CONFIG)/obj/dtoa.o
DEPS_104 += $(CONFIG)/obj/ejsApp.o
DEPS_104 += $(CONFIG)/obj/ejsArray.o
DEPS_104 += $(CONFIG)/obj/ejsBlock.o
DEPS_104 += $(CONFIG)/obj/ejsBoolean.o
DEPS_104 += $(CONFIG)/obj/ejsByteArray.o
DEPS_104 += $(CONFIG)/obj/ejsCache.o
DEPS_104 += $(CONFIG)/obj/ejsCmd.o
DEPS_104 += $(CONFIG)/obj/ejsConfig.o
DEPS_104 += $(CONFIG)/obj/ejsDate.o
DEPS_104 += $(CONFIG)/obj/ejsDebug.o
DEPS_104 += $(CONFIG)/obj/ejsError.o
DEPS_104 += $(CONFIG)/obj/ejsFile.o
DEPS_104 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_104 += $(CONFIG)/obj/ejsFrame.o
DEPS_104 += $(CONFIG)/obj/ejsFunction.o
DEPS_104 += $(CONFIG)/obj/ejsGC.o
DEPS_104 += $(CONFIG)/obj/ejsGlobal.o
DEPS_104 += $(CONFIG)/obj/ejsHttp.o
DEPS_104 += $(CONFIG)/obj/ejsIterator.o
DEPS_104 += $(CONFIG)/obj/ejsJSON.o
DEPS_104 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_104 += $(CONFIG)/obj/ejsMath.o
DEPS_104 += $(CONFIG)/obj/ejsMemory.o
DEPS_104 += $(CONFIG)/obj/ejsMprLog.o
DEPS_104 += $(CONFIG)/obj/ejsNamespace.o
DEPS_104 += $(CONFIG)/obj/ejsNull.o
DEPS_104 += $(CONFIG)/obj/ejsNumber.o
DEPS_104 += $(CONFIG)/obj/ejsObject.o
DEPS_104 += $(CONFIG)/obj/ejsPath.o
DEPS_104 += $(CONFIG)/obj/ejsPot.o
DEPS_104 += $(CONFIG)/obj/ejsRegExp.o
DEPS_104 += $(CONFIG)/obj/ejsSocket.o
DEPS_104 += $(CONFIG)/obj/ejsString.o
DEPS_104 += $(CONFIG)/obj/ejsSystem.o
DEPS_104 += $(CONFIG)/obj/ejsTimer.o
DEPS_104 += $(CONFIG)/obj/ejsType.o
DEPS_104 += $(CONFIG)/obj/ejsUri.o
DEPS_104 += $(CONFIG)/obj/ejsVoid.o
DEPS_104 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_104 += $(CONFIG)/obj/ejsWorker.o
DEPS_104 += $(CONFIG)/obj/ejsXML.o
DEPS_104 += $(CONFIG)/obj/ejsXMLList.o
DEPS_104 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_104 += $(CONFIG)/obj/ejsByteCode.o
DEPS_104 += $(CONFIG)/obj/ejsException.o
DEPS_104 += $(CONFIG)/obj/ejsHelper.o
DEPS_104 += $(CONFIG)/obj/ejsInterp.o
DEPS_104 += $(CONFIG)/obj/ejsLoader.o
DEPS_104 += $(CONFIG)/obj/ejsModule.o
DEPS_104 += $(CONFIG)/obj/ejsScope.o
DEPS_104 += $(CONFIG)/obj/ejsService.o
DEPS_104 += $(CONFIG)/bin/libejs.a
DEPS_104 += $(CONFIG)/obj/ejsc.o

LIBS_104 += -lejs
LIBS_104 += -lhttp
LIBS_104 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_104 += -lpcre
endif

$(CONFIG)/bin/ejsc: $(DEPS_104)
	@echo '      [Link] $(CONFIG)/bin/ejsc'
	$(CC) -o $(CONFIG)/bin/ejsc $(LIBPATHS) "$(CONFIG)/obj/ejsc.o" $(LIBPATHS_104) $(LIBS_104) $(LIBS_104) $(LIBS) $(LIBS) 

#
#   ejsmod.h
#
src/cmd/ejsmod.h: $(DEPS_105)
	@echo '      [Copy] src/cmd/ejsmod.h'

#
#   ejsmod.o
#
DEPS_106 += $(CONFIG)/inc/me.h
DEPS_106 += src/cmd/ejsmod.h
DEPS_106 += $(CONFIG)/inc/mpr.h
DEPS_106 += $(CONFIG)/inc/http.h
DEPS_106 += $(CONFIG)/inc/ejsByteCode.h
DEPS_106 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_106 += $(CONFIG)/inc/ejs.slots.h
DEPS_106 += $(CONFIG)/inc/ejsCustomize.h
DEPS_106 += $(CONFIG)/inc/ejs.h

$(CONFIG)/obj/ejsmod.o: \
    src/cmd/ejsmod.c $(DEPS_106)
	@echo '   [Compile] $(CONFIG)/obj/ejsmod.o'
	$(CC) -c -o $(CONFIG)/obj/ejsmod.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/ejsmod.c

#
#   doc.o
#
DEPS_107 += $(CONFIG)/inc/me.h
DEPS_107 += src/cmd/ejsmod.h

$(CONFIG)/obj/doc.o: \
    src/cmd/doc.c $(DEPS_107)
	@echo '   [Compile] $(CONFIG)/obj/doc.o'
	$(CC) -c -o $(CONFIG)/obj/doc.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/doc.c

#
#   docFiles.o
#
DEPS_108 += $(CONFIG)/inc/me.h
DEPS_108 += src/cmd/ejsmod.h

$(CONFIG)/obj/docFiles.o: \
    src/cmd/docFiles.c $(DEPS_108)
	@echo '   [Compile] $(CONFIG)/obj/docFiles.o'
	$(CC) -c -o $(CONFIG)/obj/docFiles.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/docFiles.c

#
#   listing.o
#
DEPS_109 += $(CONFIG)/inc/me.h
DEPS_109 += src/cmd/ejsmod.h
DEPS_109 += $(CONFIG)/inc/ejsByteCodeTable.h

$(CONFIG)/obj/listing.o: \
    src/cmd/listing.c $(DEPS_109)
	@echo '   [Compile] $(CONFIG)/obj/listing.o'
	$(CC) -c -o $(CONFIG)/obj/listing.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/listing.c

#
#   slotGen.o
#
DEPS_110 += $(CONFIG)/inc/me.h
DEPS_110 += src/cmd/ejsmod.h

$(CONFIG)/obj/slotGen.o: \
    src/cmd/slotGen.c $(DEPS_110)
	@echo '   [Compile] $(CONFIG)/obj/slotGen.o'
	$(CC) -c -o $(CONFIG)/obj/slotGen.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/cmd/slotGen.c

#
#   ejsmod
#
DEPS_111 += $(CONFIG)/inc/mpr.h
DEPS_111 += $(CONFIG)/inc/me.h
DEPS_111 += $(CONFIG)/inc/osdep.h
DEPS_111 += $(CONFIG)/obj/mprLib.o
DEPS_111 += $(CONFIG)/bin/libmpr.a
DEPS_111 += $(CONFIG)/inc/pcre.h
DEPS_111 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_111 += $(CONFIG)/bin/libpcre.a
endif
DEPS_111 += $(CONFIG)/inc/http.h
DEPS_111 += $(CONFIG)/obj/httpLib.o
DEPS_111 += $(CONFIG)/bin/libhttp.a
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
DEPS_111 += $(CONFIG)/bin/libejs.a
DEPS_111 += src/cmd/ejsmod.h
DEPS_111 += $(CONFIG)/obj/ejsmod.o
DEPS_111 += $(CONFIG)/obj/doc.o
DEPS_111 += $(CONFIG)/obj/docFiles.o
DEPS_111 += $(CONFIG)/obj/listing.o
DEPS_111 += $(CONFIG)/obj/slotGen.o

LIBS_111 += -lejs
LIBS_111 += -lhttp
LIBS_111 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_111 += -lpcre
endif

$(CONFIG)/bin/ejsmod: $(DEPS_111)
	@echo '      [Link] $(CONFIG)/bin/ejsmod'
	$(CC) -o $(CONFIG)/bin/ejsmod $(LIBPATHS) "$(CONFIG)/obj/ejsmod.o" "$(CONFIG)/obj/doc.o" "$(CONFIG)/obj/docFiles.o" "$(CONFIG)/obj/listing.o" "$(CONFIG)/obj/slotGen.o" $(LIBPATHS_111) $(LIBS_111) $(LIBS_111) $(LIBS) $(LIBS) 

#
#   ejsrun.o
#
DEPS_112 += $(CONFIG)/inc/me.h
DEPS_112 += $(CONFIG)/inc/ejsCompiler.h

$(CONFIG)/obj/ejsrun.o: \
    src/cmd/ejsrun.c $(DEPS_112)
	@echo '   [Compile] $(CONFIG)/obj/ejsrun.o'
	$(CC) -c -o $(CONFIG)/obj/ejsrun.o $(CFLAGS) $(DFLAGS) $(IFLAGS) src/cmd/ejsrun.c

#
#   ejsrun
#
DEPS_113 += $(CONFIG)/inc/mpr.h
DEPS_113 += $(CONFIG)/inc/me.h
DEPS_113 += $(CONFIG)/inc/osdep.h
DEPS_113 += $(CONFIG)/obj/mprLib.o
DEPS_113 += $(CONFIG)/bin/libmpr.a
DEPS_113 += $(CONFIG)/inc/pcre.h
DEPS_113 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_113 += $(CONFIG)/bin/libpcre.a
endif
DEPS_113 += $(CONFIG)/inc/http.h
DEPS_113 += $(CONFIG)/obj/httpLib.o
DEPS_113 += $(CONFIG)/bin/libhttp.a
DEPS_113 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_113 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_113 += $(CONFIG)/inc/ejs.slots.h
DEPS_113 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_113 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_113 += $(CONFIG)/inc/ejsByteCode.h
DEPS_113 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_113 += $(CONFIG)/inc/ejsCustomize.h
DEPS_113 += $(CONFIG)/inc/ejs.h
DEPS_113 += $(CONFIG)/inc/ejsCompiler.h
DEPS_113 += $(CONFIG)/obj/ecAst.o
DEPS_113 += $(CONFIG)/obj/ecCodeGen.o
DEPS_113 += $(CONFIG)/obj/ecCompiler.o
DEPS_113 += $(CONFIG)/obj/ecLex.o
DEPS_113 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_113 += $(CONFIG)/obj/ecParser.o
DEPS_113 += $(CONFIG)/obj/ecState.o
DEPS_113 += $(CONFIG)/obj/dtoa.o
DEPS_113 += $(CONFIG)/obj/ejsApp.o
DEPS_113 += $(CONFIG)/obj/ejsArray.o
DEPS_113 += $(CONFIG)/obj/ejsBlock.o
DEPS_113 += $(CONFIG)/obj/ejsBoolean.o
DEPS_113 += $(CONFIG)/obj/ejsByteArray.o
DEPS_113 += $(CONFIG)/obj/ejsCache.o
DEPS_113 += $(CONFIG)/obj/ejsCmd.o
DEPS_113 += $(CONFIG)/obj/ejsConfig.o
DEPS_113 += $(CONFIG)/obj/ejsDate.o
DEPS_113 += $(CONFIG)/obj/ejsDebug.o
DEPS_113 += $(CONFIG)/obj/ejsError.o
DEPS_113 += $(CONFIG)/obj/ejsFile.o
DEPS_113 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_113 += $(CONFIG)/obj/ejsFrame.o
DEPS_113 += $(CONFIG)/obj/ejsFunction.o
DEPS_113 += $(CONFIG)/obj/ejsGC.o
DEPS_113 += $(CONFIG)/obj/ejsGlobal.o
DEPS_113 += $(CONFIG)/obj/ejsHttp.o
DEPS_113 += $(CONFIG)/obj/ejsIterator.o
DEPS_113 += $(CONFIG)/obj/ejsJSON.o
DEPS_113 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_113 += $(CONFIG)/obj/ejsMath.o
DEPS_113 += $(CONFIG)/obj/ejsMemory.o
DEPS_113 += $(CONFIG)/obj/ejsMprLog.o
DEPS_113 += $(CONFIG)/obj/ejsNamespace.o
DEPS_113 += $(CONFIG)/obj/ejsNull.o
DEPS_113 += $(CONFIG)/obj/ejsNumber.o
DEPS_113 += $(CONFIG)/obj/ejsObject.o
DEPS_113 += $(CONFIG)/obj/ejsPath.o
DEPS_113 += $(CONFIG)/obj/ejsPot.o
DEPS_113 += $(CONFIG)/obj/ejsRegExp.o
DEPS_113 += $(CONFIG)/obj/ejsSocket.o
DEPS_113 += $(CONFIG)/obj/ejsString.o
DEPS_113 += $(CONFIG)/obj/ejsSystem.o
DEPS_113 += $(CONFIG)/obj/ejsTimer.o
DEPS_113 += $(CONFIG)/obj/ejsType.o
DEPS_113 += $(CONFIG)/obj/ejsUri.o
DEPS_113 += $(CONFIG)/obj/ejsVoid.o
DEPS_113 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_113 += $(CONFIG)/obj/ejsWorker.o
DEPS_113 += $(CONFIG)/obj/ejsXML.o
DEPS_113 += $(CONFIG)/obj/ejsXMLList.o
DEPS_113 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_113 += $(CONFIG)/obj/ejsByteCode.o
DEPS_113 += $(CONFIG)/obj/ejsException.o
DEPS_113 += $(CONFIG)/obj/ejsHelper.o
DEPS_113 += $(CONFIG)/obj/ejsInterp.o
DEPS_113 += $(CONFIG)/obj/ejsLoader.o
DEPS_113 += $(CONFIG)/obj/ejsModule.o
DEPS_113 += $(CONFIG)/obj/ejsScope.o
DEPS_113 += $(CONFIG)/obj/ejsService.o
DEPS_113 += $(CONFIG)/bin/libejs.a
DEPS_113 += $(CONFIG)/obj/ejsrun.o

LIBS_113 += -lejs
LIBS_113 += -lhttp
LIBS_113 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_113 += -lpcre
endif

$(CONFIG)/bin/ejsrun: $(DEPS_113)
	@echo '      [Link] $(CONFIG)/bin/ejsrun'
	$(CC) -o $(CONFIG)/bin/ejsrun $(LIBPATHS) "$(CONFIG)/obj/ejsrun.o" $(LIBPATHS_113) $(LIBS_113) $(LIBS_113) $(LIBS) $(LIBS) 

#
#   ejs.mod
#
DEPS_114 += src/core/App.es
DEPS_114 += src/core/Args.es
DEPS_114 += src/core/Array.es
DEPS_114 += src/core/BinaryStream.es
DEPS_114 += src/core/Block.es
DEPS_114 += src/core/Boolean.es
DEPS_114 += src/core/ByteArray.es
DEPS_114 += src/core/Cache.es
DEPS_114 += src/core/Cmd.es
DEPS_114 += src/core/Compat.es
DEPS_114 += src/core/Config.es
DEPS_114 += src/core/Date.es
DEPS_114 += src/core/Debug.es
DEPS_114 += src/core/Emitter.es
DEPS_114 += src/core/Error.es
DEPS_114 += src/core/File.es
DEPS_114 += src/core/FileSystem.es
DEPS_114 += src/core/Frame.es
DEPS_114 += src/core/Function.es
DEPS_114 += src/core/GC.es
DEPS_114 += src/core/Global.es
DEPS_114 += src/core/Http.es
DEPS_114 += src/core/Inflector.es
DEPS_114 += src/core/Iterator.es
DEPS_114 += src/core/JSON.es
DEPS_114 += src/core/Loader.es
DEPS_114 += src/core/LocalCache.es
DEPS_114 += src/core/Locale.es
DEPS_114 += src/core/Logger.es
DEPS_114 += src/core/Math.es
DEPS_114 += src/core/Memory.es
DEPS_114 += src/core/MprLog.es
DEPS_114 += src/core/Name.es
DEPS_114 += src/core/Namespace.es
DEPS_114 += src/core/Null.es
DEPS_114 += src/core/Number.es
DEPS_114 += src/core/Object.es
DEPS_114 += src/core/Path.es
DEPS_114 += src/core/Promise.es
DEPS_114 += src/core/RegExp.es
DEPS_114 += src/core/Socket.es
DEPS_114 += src/core/Stream.es
DEPS_114 += src/core/String.es
DEPS_114 += src/core/System.es
DEPS_114 += src/core/TextStream.es
DEPS_114 += src/core/Timer.es
DEPS_114 += src/core/Type.es
DEPS_114 += src/core/Uri.es
DEPS_114 += src/core/Void.es
DEPS_114 += src/core/WebSocket.es
DEPS_114 += src/core/Worker.es
DEPS_114 += src/core/XML.es
DEPS_114 += src/core/XMLHttp.es
DEPS_114 += src/core/XMLList.es
DEPS_114 += $(CONFIG)/inc/mpr.h
DEPS_114 += $(CONFIG)/inc/me.h
DEPS_114 += $(CONFIG)/inc/osdep.h
DEPS_114 += $(CONFIG)/obj/mprLib.o
DEPS_114 += $(CONFIG)/bin/libmpr.a
DEPS_114 += $(CONFIG)/inc/pcre.h
DEPS_114 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_114 += $(CONFIG)/bin/libpcre.a
endif
DEPS_114 += $(CONFIG)/inc/http.h
DEPS_114 += $(CONFIG)/obj/httpLib.o
DEPS_114 += $(CONFIG)/bin/libhttp.a
DEPS_114 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_114 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_114 += $(CONFIG)/inc/ejs.slots.h
DEPS_114 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_114 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_114 += $(CONFIG)/inc/ejsByteCode.h
DEPS_114 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_114 += $(CONFIG)/inc/ejsCustomize.h
DEPS_114 += $(CONFIG)/inc/ejs.h
DEPS_114 += $(CONFIG)/inc/ejsCompiler.h
DEPS_114 += $(CONFIG)/obj/ecAst.o
DEPS_114 += $(CONFIG)/obj/ecCodeGen.o
DEPS_114 += $(CONFIG)/obj/ecCompiler.o
DEPS_114 += $(CONFIG)/obj/ecLex.o
DEPS_114 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_114 += $(CONFIG)/obj/ecParser.o
DEPS_114 += $(CONFIG)/obj/ecState.o
DEPS_114 += $(CONFIG)/obj/dtoa.o
DEPS_114 += $(CONFIG)/obj/ejsApp.o
DEPS_114 += $(CONFIG)/obj/ejsArray.o
DEPS_114 += $(CONFIG)/obj/ejsBlock.o
DEPS_114 += $(CONFIG)/obj/ejsBoolean.o
DEPS_114 += $(CONFIG)/obj/ejsByteArray.o
DEPS_114 += $(CONFIG)/obj/ejsCache.o
DEPS_114 += $(CONFIG)/obj/ejsCmd.o
DEPS_114 += $(CONFIG)/obj/ejsConfig.o
DEPS_114 += $(CONFIG)/obj/ejsDate.o
DEPS_114 += $(CONFIG)/obj/ejsDebug.o
DEPS_114 += $(CONFIG)/obj/ejsError.o
DEPS_114 += $(CONFIG)/obj/ejsFile.o
DEPS_114 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_114 += $(CONFIG)/obj/ejsFrame.o
DEPS_114 += $(CONFIG)/obj/ejsFunction.o
DEPS_114 += $(CONFIG)/obj/ejsGC.o
DEPS_114 += $(CONFIG)/obj/ejsGlobal.o
DEPS_114 += $(CONFIG)/obj/ejsHttp.o
DEPS_114 += $(CONFIG)/obj/ejsIterator.o
DEPS_114 += $(CONFIG)/obj/ejsJSON.o
DEPS_114 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_114 += $(CONFIG)/obj/ejsMath.o
DEPS_114 += $(CONFIG)/obj/ejsMemory.o
DEPS_114 += $(CONFIG)/obj/ejsMprLog.o
DEPS_114 += $(CONFIG)/obj/ejsNamespace.o
DEPS_114 += $(CONFIG)/obj/ejsNull.o
DEPS_114 += $(CONFIG)/obj/ejsNumber.o
DEPS_114 += $(CONFIG)/obj/ejsObject.o
DEPS_114 += $(CONFIG)/obj/ejsPath.o
DEPS_114 += $(CONFIG)/obj/ejsPot.o
DEPS_114 += $(CONFIG)/obj/ejsRegExp.o
DEPS_114 += $(CONFIG)/obj/ejsSocket.o
DEPS_114 += $(CONFIG)/obj/ejsString.o
DEPS_114 += $(CONFIG)/obj/ejsSystem.o
DEPS_114 += $(CONFIG)/obj/ejsTimer.o
DEPS_114 += $(CONFIG)/obj/ejsType.o
DEPS_114 += $(CONFIG)/obj/ejsUri.o
DEPS_114 += $(CONFIG)/obj/ejsVoid.o
DEPS_114 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_114 += $(CONFIG)/obj/ejsWorker.o
DEPS_114 += $(CONFIG)/obj/ejsXML.o
DEPS_114 += $(CONFIG)/obj/ejsXMLList.o
DEPS_114 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_114 += $(CONFIG)/obj/ejsByteCode.o
DEPS_114 += $(CONFIG)/obj/ejsException.o
DEPS_114 += $(CONFIG)/obj/ejsHelper.o
DEPS_114 += $(CONFIG)/obj/ejsInterp.o
DEPS_114 += $(CONFIG)/obj/ejsLoader.o
DEPS_114 += $(CONFIG)/obj/ejsModule.o
DEPS_114 += $(CONFIG)/obj/ejsScope.o
DEPS_114 += $(CONFIG)/obj/ejsService.o
DEPS_114 += $(CONFIG)/bin/libejs.a
DEPS_114 += $(CONFIG)/obj/ejsc.o
DEPS_114 += $(CONFIG)/bin/ejsc
DEPS_114 += src/cmd/ejsmod.h
DEPS_114 += $(CONFIG)/obj/ejsmod.o
DEPS_114 += $(CONFIG)/obj/doc.o
DEPS_114 += $(CONFIG)/obj/docFiles.o
DEPS_114 += $(CONFIG)/obj/listing.o
DEPS_114 += $(CONFIG)/obj/slotGen.o
DEPS_114 += $(CONFIG)/bin/ejsmod

$(CONFIG)/bin/ejs.mod: $(DEPS_114)
	( \
	cd src/core; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.mod  --optimize 9 --bind --require null *.es  ; \
	../../$(CONFIG)/bin/ejsmod --require null --cslots ../../$(CONFIG)/bin/ejs.mod ; \
	if ! diff ejs.slots.h ../../$(CONFIG)/inc/ejs.slots.h >/dev/null; then cp ejs.slots.h ../../$(CONFIG)/inc; fi ; \
	rm -f ejs.slots.h ; \
	)

#
#   ejs.unix.mod
#
DEPS_115 += src/ejs.unix/Unix.es
DEPS_115 += $(CONFIG)/inc/mpr.h
DEPS_115 += $(CONFIG)/inc/me.h
DEPS_115 += $(CONFIG)/inc/osdep.h
DEPS_115 += $(CONFIG)/obj/mprLib.o
DEPS_115 += $(CONFIG)/bin/libmpr.a
DEPS_115 += $(CONFIG)/inc/pcre.h
DEPS_115 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_115 += $(CONFIG)/bin/libpcre.a
endif
DEPS_115 += $(CONFIG)/inc/http.h
DEPS_115 += $(CONFIG)/obj/httpLib.o
DEPS_115 += $(CONFIG)/bin/libhttp.a
DEPS_115 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_115 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_115 += $(CONFIG)/inc/ejs.slots.h
DEPS_115 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_115 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_115 += $(CONFIG)/inc/ejsByteCode.h
DEPS_115 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_115 += $(CONFIG)/inc/ejsCustomize.h
DEPS_115 += $(CONFIG)/inc/ejs.h
DEPS_115 += $(CONFIG)/inc/ejsCompiler.h
DEPS_115 += $(CONFIG)/obj/ecAst.o
DEPS_115 += $(CONFIG)/obj/ecCodeGen.o
DEPS_115 += $(CONFIG)/obj/ecCompiler.o
DEPS_115 += $(CONFIG)/obj/ecLex.o
DEPS_115 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_115 += $(CONFIG)/obj/ecParser.o
DEPS_115 += $(CONFIG)/obj/ecState.o
DEPS_115 += $(CONFIG)/obj/dtoa.o
DEPS_115 += $(CONFIG)/obj/ejsApp.o
DEPS_115 += $(CONFIG)/obj/ejsArray.o
DEPS_115 += $(CONFIG)/obj/ejsBlock.o
DEPS_115 += $(CONFIG)/obj/ejsBoolean.o
DEPS_115 += $(CONFIG)/obj/ejsByteArray.o
DEPS_115 += $(CONFIG)/obj/ejsCache.o
DEPS_115 += $(CONFIG)/obj/ejsCmd.o
DEPS_115 += $(CONFIG)/obj/ejsConfig.o
DEPS_115 += $(CONFIG)/obj/ejsDate.o
DEPS_115 += $(CONFIG)/obj/ejsDebug.o
DEPS_115 += $(CONFIG)/obj/ejsError.o
DEPS_115 += $(CONFIG)/obj/ejsFile.o
DEPS_115 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_115 += $(CONFIG)/obj/ejsFrame.o
DEPS_115 += $(CONFIG)/obj/ejsFunction.o
DEPS_115 += $(CONFIG)/obj/ejsGC.o
DEPS_115 += $(CONFIG)/obj/ejsGlobal.o
DEPS_115 += $(CONFIG)/obj/ejsHttp.o
DEPS_115 += $(CONFIG)/obj/ejsIterator.o
DEPS_115 += $(CONFIG)/obj/ejsJSON.o
DEPS_115 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_115 += $(CONFIG)/obj/ejsMath.o
DEPS_115 += $(CONFIG)/obj/ejsMemory.o
DEPS_115 += $(CONFIG)/obj/ejsMprLog.o
DEPS_115 += $(CONFIG)/obj/ejsNamespace.o
DEPS_115 += $(CONFIG)/obj/ejsNull.o
DEPS_115 += $(CONFIG)/obj/ejsNumber.o
DEPS_115 += $(CONFIG)/obj/ejsObject.o
DEPS_115 += $(CONFIG)/obj/ejsPath.o
DEPS_115 += $(CONFIG)/obj/ejsPot.o
DEPS_115 += $(CONFIG)/obj/ejsRegExp.o
DEPS_115 += $(CONFIG)/obj/ejsSocket.o
DEPS_115 += $(CONFIG)/obj/ejsString.o
DEPS_115 += $(CONFIG)/obj/ejsSystem.o
DEPS_115 += $(CONFIG)/obj/ejsTimer.o
DEPS_115 += $(CONFIG)/obj/ejsType.o
DEPS_115 += $(CONFIG)/obj/ejsUri.o
DEPS_115 += $(CONFIG)/obj/ejsVoid.o
DEPS_115 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_115 += $(CONFIG)/obj/ejsWorker.o
DEPS_115 += $(CONFIG)/obj/ejsXML.o
DEPS_115 += $(CONFIG)/obj/ejsXMLList.o
DEPS_115 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_115 += $(CONFIG)/obj/ejsByteCode.o
DEPS_115 += $(CONFIG)/obj/ejsException.o
DEPS_115 += $(CONFIG)/obj/ejsHelper.o
DEPS_115 += $(CONFIG)/obj/ejsInterp.o
DEPS_115 += $(CONFIG)/obj/ejsLoader.o
DEPS_115 += $(CONFIG)/obj/ejsModule.o
DEPS_115 += $(CONFIG)/obj/ejsScope.o
DEPS_115 += $(CONFIG)/obj/ejsService.o
DEPS_115 += $(CONFIG)/bin/libejs.a
DEPS_115 += $(CONFIG)/obj/ejsc.o
DEPS_115 += $(CONFIG)/bin/ejsc
DEPS_115 += src/cmd/ejsmod.h
DEPS_115 += $(CONFIG)/obj/ejsmod.o
DEPS_115 += $(CONFIG)/obj/doc.o
DEPS_115 += $(CONFIG)/obj/docFiles.o
DEPS_115 += $(CONFIG)/obj/listing.o
DEPS_115 += $(CONFIG)/obj/slotGen.o
DEPS_115 += $(CONFIG)/bin/ejsmod
DEPS_115 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.unix.mod: $(DEPS_115)
	( \
	cd src/ejs.unix; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.unix.mod  --optimize 9 Unix.es ; \
	)

#
#   ejs.db.mod
#
DEPS_116 += src/ejs.db/Database.es
DEPS_116 += src/ejs.db/DatabaseConnector.es
DEPS_116 += $(CONFIG)/inc/mpr.h
DEPS_116 += $(CONFIG)/inc/me.h
DEPS_116 += $(CONFIG)/inc/osdep.h
DEPS_116 += $(CONFIG)/obj/mprLib.o
DEPS_116 += $(CONFIG)/bin/libmpr.a
DEPS_116 += $(CONFIG)/inc/pcre.h
DEPS_116 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_116 += $(CONFIG)/bin/libpcre.a
endif
DEPS_116 += $(CONFIG)/inc/http.h
DEPS_116 += $(CONFIG)/obj/httpLib.o
DEPS_116 += $(CONFIG)/bin/libhttp.a
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
DEPS_116 += $(CONFIG)/bin/ejsc
DEPS_116 += src/cmd/ejsmod.h
DEPS_116 += $(CONFIG)/obj/ejsmod.o
DEPS_116 += $(CONFIG)/obj/doc.o
DEPS_116 += $(CONFIG)/obj/docFiles.o
DEPS_116 += $(CONFIG)/obj/listing.o
DEPS_116 += $(CONFIG)/obj/slotGen.o
DEPS_116 += $(CONFIG)/bin/ejsmod
DEPS_116 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.db.mod: $(DEPS_116)
	( \
	cd src/ejs.db; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.db.mod  --optimize 9 *.es ; \
	)

#
#   ejs.db.mapper.mod
#
DEPS_117 += src/ejs.db.mapper/Record.es
DEPS_117 += $(CONFIG)/inc/mpr.h
DEPS_117 += $(CONFIG)/inc/me.h
DEPS_117 += $(CONFIG)/inc/osdep.h
DEPS_117 += $(CONFIG)/obj/mprLib.o
DEPS_117 += $(CONFIG)/bin/libmpr.a
DEPS_117 += $(CONFIG)/inc/pcre.h
DEPS_117 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_117 += $(CONFIG)/bin/libpcre.a
endif
DEPS_117 += $(CONFIG)/inc/http.h
DEPS_117 += $(CONFIG)/obj/httpLib.o
DEPS_117 += $(CONFIG)/bin/libhttp.a
DEPS_117 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_117 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_117 += $(CONFIG)/inc/ejs.slots.h
DEPS_117 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_117 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_117 += $(CONFIG)/inc/ejsByteCode.h
DEPS_117 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_117 += $(CONFIG)/inc/ejsCustomize.h
DEPS_117 += $(CONFIG)/inc/ejs.h
DEPS_117 += $(CONFIG)/inc/ejsCompiler.h
DEPS_117 += $(CONFIG)/obj/ecAst.o
DEPS_117 += $(CONFIG)/obj/ecCodeGen.o
DEPS_117 += $(CONFIG)/obj/ecCompiler.o
DEPS_117 += $(CONFIG)/obj/ecLex.o
DEPS_117 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_117 += $(CONFIG)/obj/ecParser.o
DEPS_117 += $(CONFIG)/obj/ecState.o
DEPS_117 += $(CONFIG)/obj/dtoa.o
DEPS_117 += $(CONFIG)/obj/ejsApp.o
DEPS_117 += $(CONFIG)/obj/ejsArray.o
DEPS_117 += $(CONFIG)/obj/ejsBlock.o
DEPS_117 += $(CONFIG)/obj/ejsBoolean.o
DEPS_117 += $(CONFIG)/obj/ejsByteArray.o
DEPS_117 += $(CONFIG)/obj/ejsCache.o
DEPS_117 += $(CONFIG)/obj/ejsCmd.o
DEPS_117 += $(CONFIG)/obj/ejsConfig.o
DEPS_117 += $(CONFIG)/obj/ejsDate.o
DEPS_117 += $(CONFIG)/obj/ejsDebug.o
DEPS_117 += $(CONFIG)/obj/ejsError.o
DEPS_117 += $(CONFIG)/obj/ejsFile.o
DEPS_117 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_117 += $(CONFIG)/obj/ejsFrame.o
DEPS_117 += $(CONFIG)/obj/ejsFunction.o
DEPS_117 += $(CONFIG)/obj/ejsGC.o
DEPS_117 += $(CONFIG)/obj/ejsGlobal.o
DEPS_117 += $(CONFIG)/obj/ejsHttp.o
DEPS_117 += $(CONFIG)/obj/ejsIterator.o
DEPS_117 += $(CONFIG)/obj/ejsJSON.o
DEPS_117 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_117 += $(CONFIG)/obj/ejsMath.o
DEPS_117 += $(CONFIG)/obj/ejsMemory.o
DEPS_117 += $(CONFIG)/obj/ejsMprLog.o
DEPS_117 += $(CONFIG)/obj/ejsNamespace.o
DEPS_117 += $(CONFIG)/obj/ejsNull.o
DEPS_117 += $(CONFIG)/obj/ejsNumber.o
DEPS_117 += $(CONFIG)/obj/ejsObject.o
DEPS_117 += $(CONFIG)/obj/ejsPath.o
DEPS_117 += $(CONFIG)/obj/ejsPot.o
DEPS_117 += $(CONFIG)/obj/ejsRegExp.o
DEPS_117 += $(CONFIG)/obj/ejsSocket.o
DEPS_117 += $(CONFIG)/obj/ejsString.o
DEPS_117 += $(CONFIG)/obj/ejsSystem.o
DEPS_117 += $(CONFIG)/obj/ejsTimer.o
DEPS_117 += $(CONFIG)/obj/ejsType.o
DEPS_117 += $(CONFIG)/obj/ejsUri.o
DEPS_117 += $(CONFIG)/obj/ejsVoid.o
DEPS_117 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_117 += $(CONFIG)/obj/ejsWorker.o
DEPS_117 += $(CONFIG)/obj/ejsXML.o
DEPS_117 += $(CONFIG)/obj/ejsXMLList.o
DEPS_117 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_117 += $(CONFIG)/obj/ejsByteCode.o
DEPS_117 += $(CONFIG)/obj/ejsException.o
DEPS_117 += $(CONFIG)/obj/ejsHelper.o
DEPS_117 += $(CONFIG)/obj/ejsInterp.o
DEPS_117 += $(CONFIG)/obj/ejsLoader.o
DEPS_117 += $(CONFIG)/obj/ejsModule.o
DEPS_117 += $(CONFIG)/obj/ejsScope.o
DEPS_117 += $(CONFIG)/obj/ejsService.o
DEPS_117 += $(CONFIG)/bin/libejs.a
DEPS_117 += $(CONFIG)/obj/ejsc.o
DEPS_117 += $(CONFIG)/bin/ejsc
DEPS_117 += src/cmd/ejsmod.h
DEPS_117 += $(CONFIG)/obj/ejsmod.o
DEPS_117 += $(CONFIG)/obj/doc.o
DEPS_117 += $(CONFIG)/obj/docFiles.o
DEPS_117 += $(CONFIG)/obj/listing.o
DEPS_117 += $(CONFIG)/obj/slotGen.o
DEPS_117 += $(CONFIG)/bin/ejsmod
DEPS_117 += $(CONFIG)/bin/ejs.mod
DEPS_117 += $(CONFIG)/bin/ejs.db.mod

$(CONFIG)/bin/ejs.db.mapper.mod: $(DEPS_117)
	( \
	cd src/ejs.db.mapper; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.db.mapper.mod  --optimize 9 *.es ; \
	)

#
#   ejs.db.sqlite.mod
#
DEPS_118 += src/ejs.db.sqlite/Sqlite.es
DEPS_118 += $(CONFIG)/inc/mpr.h
DEPS_118 += $(CONFIG)/inc/me.h
DEPS_118 += $(CONFIG)/inc/osdep.h
DEPS_118 += $(CONFIG)/obj/mprLib.o
DEPS_118 += $(CONFIG)/bin/libmpr.a
DEPS_118 += $(CONFIG)/inc/pcre.h
DEPS_118 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_118 += $(CONFIG)/bin/libpcre.a
endif
DEPS_118 += $(CONFIG)/inc/http.h
DEPS_118 += $(CONFIG)/obj/httpLib.o
DEPS_118 += $(CONFIG)/bin/libhttp.a
DEPS_118 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_118 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_118 += $(CONFIG)/inc/ejs.slots.h
DEPS_118 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_118 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_118 += $(CONFIG)/inc/ejsByteCode.h
DEPS_118 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_118 += $(CONFIG)/inc/ejsCustomize.h
DEPS_118 += $(CONFIG)/inc/ejs.h
DEPS_118 += $(CONFIG)/inc/ejsCompiler.h
DEPS_118 += $(CONFIG)/obj/ecAst.o
DEPS_118 += $(CONFIG)/obj/ecCodeGen.o
DEPS_118 += $(CONFIG)/obj/ecCompiler.o
DEPS_118 += $(CONFIG)/obj/ecLex.o
DEPS_118 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_118 += $(CONFIG)/obj/ecParser.o
DEPS_118 += $(CONFIG)/obj/ecState.o
DEPS_118 += $(CONFIG)/obj/dtoa.o
DEPS_118 += $(CONFIG)/obj/ejsApp.o
DEPS_118 += $(CONFIG)/obj/ejsArray.o
DEPS_118 += $(CONFIG)/obj/ejsBlock.o
DEPS_118 += $(CONFIG)/obj/ejsBoolean.o
DEPS_118 += $(CONFIG)/obj/ejsByteArray.o
DEPS_118 += $(CONFIG)/obj/ejsCache.o
DEPS_118 += $(CONFIG)/obj/ejsCmd.o
DEPS_118 += $(CONFIG)/obj/ejsConfig.o
DEPS_118 += $(CONFIG)/obj/ejsDate.o
DEPS_118 += $(CONFIG)/obj/ejsDebug.o
DEPS_118 += $(CONFIG)/obj/ejsError.o
DEPS_118 += $(CONFIG)/obj/ejsFile.o
DEPS_118 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_118 += $(CONFIG)/obj/ejsFrame.o
DEPS_118 += $(CONFIG)/obj/ejsFunction.o
DEPS_118 += $(CONFIG)/obj/ejsGC.o
DEPS_118 += $(CONFIG)/obj/ejsGlobal.o
DEPS_118 += $(CONFIG)/obj/ejsHttp.o
DEPS_118 += $(CONFIG)/obj/ejsIterator.o
DEPS_118 += $(CONFIG)/obj/ejsJSON.o
DEPS_118 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_118 += $(CONFIG)/obj/ejsMath.o
DEPS_118 += $(CONFIG)/obj/ejsMemory.o
DEPS_118 += $(CONFIG)/obj/ejsMprLog.o
DEPS_118 += $(CONFIG)/obj/ejsNamespace.o
DEPS_118 += $(CONFIG)/obj/ejsNull.o
DEPS_118 += $(CONFIG)/obj/ejsNumber.o
DEPS_118 += $(CONFIG)/obj/ejsObject.o
DEPS_118 += $(CONFIG)/obj/ejsPath.o
DEPS_118 += $(CONFIG)/obj/ejsPot.o
DEPS_118 += $(CONFIG)/obj/ejsRegExp.o
DEPS_118 += $(CONFIG)/obj/ejsSocket.o
DEPS_118 += $(CONFIG)/obj/ejsString.o
DEPS_118 += $(CONFIG)/obj/ejsSystem.o
DEPS_118 += $(CONFIG)/obj/ejsTimer.o
DEPS_118 += $(CONFIG)/obj/ejsType.o
DEPS_118 += $(CONFIG)/obj/ejsUri.o
DEPS_118 += $(CONFIG)/obj/ejsVoid.o
DEPS_118 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_118 += $(CONFIG)/obj/ejsWorker.o
DEPS_118 += $(CONFIG)/obj/ejsXML.o
DEPS_118 += $(CONFIG)/obj/ejsXMLList.o
DEPS_118 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_118 += $(CONFIG)/obj/ejsByteCode.o
DEPS_118 += $(CONFIG)/obj/ejsException.o
DEPS_118 += $(CONFIG)/obj/ejsHelper.o
DEPS_118 += $(CONFIG)/obj/ejsInterp.o
DEPS_118 += $(CONFIG)/obj/ejsLoader.o
DEPS_118 += $(CONFIG)/obj/ejsModule.o
DEPS_118 += $(CONFIG)/obj/ejsScope.o
DEPS_118 += $(CONFIG)/obj/ejsService.o
DEPS_118 += $(CONFIG)/bin/libejs.a
DEPS_118 += $(CONFIG)/obj/ejsc.o
DEPS_118 += $(CONFIG)/bin/ejsc
DEPS_118 += src/cmd/ejsmod.h
DEPS_118 += $(CONFIG)/obj/ejsmod.o
DEPS_118 += $(CONFIG)/obj/doc.o
DEPS_118 += $(CONFIG)/obj/docFiles.o
DEPS_118 += $(CONFIG)/obj/listing.o
DEPS_118 += $(CONFIG)/obj/slotGen.o
DEPS_118 += $(CONFIG)/bin/ejsmod
DEPS_118 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.db.sqlite.mod: $(DEPS_118)
	( \
	cd src/ejs.db.sqlite; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.db.sqlite.mod  --optimize 9 *.es ; \
	)

#
#   ejsSqlite.o
#
DEPS_119 += $(CONFIG)/inc/me.h
DEPS_119 += $(CONFIG)/inc/mpr.h
DEPS_119 += $(CONFIG)/inc/http.h
DEPS_119 += $(CONFIG)/inc/ejsByteCode.h
DEPS_119 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_119 += $(CONFIG)/inc/ejs.slots.h
DEPS_119 += $(CONFIG)/inc/ejsCustomize.h
DEPS_119 += $(CONFIG)/inc/ejs.h
DEPS_119 += $(CONFIG)/inc/ejs.db.sqlite.slots.h

$(CONFIG)/obj/ejsSqlite.o: \
    src/ejs.db.sqlite/ejsSqlite.c $(DEPS_119)
	@echo '   [Compile] $(CONFIG)/obj/ejsSqlite.o'
	$(CC) -c -o $(CONFIG)/obj/ejsSqlite.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.db.sqlite/ejsSqlite.c

#
#   libejs.db.sqlite
#
DEPS_120 += $(CONFIG)/inc/mpr.h
DEPS_120 += $(CONFIG)/inc/me.h
DEPS_120 += $(CONFIG)/inc/osdep.h
DEPS_120 += $(CONFIG)/obj/mprLib.o
DEPS_120 += $(CONFIG)/bin/libmpr.a
DEPS_120 += $(CONFIG)/inc/pcre.h
DEPS_120 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_120 += $(CONFIG)/bin/libpcre.a
endif
DEPS_120 += $(CONFIG)/inc/http.h
DEPS_120 += $(CONFIG)/obj/httpLib.o
DEPS_120 += $(CONFIG)/bin/libhttp.a
DEPS_120 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_120 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_120 += $(CONFIG)/inc/ejs.slots.h
DEPS_120 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_120 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_120 += $(CONFIG)/inc/ejsByteCode.h
DEPS_120 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_120 += $(CONFIG)/inc/ejsCustomize.h
DEPS_120 += $(CONFIG)/inc/ejs.h
DEPS_120 += $(CONFIG)/inc/ejsCompiler.h
DEPS_120 += $(CONFIG)/obj/ecAst.o
DEPS_120 += $(CONFIG)/obj/ecCodeGen.o
DEPS_120 += $(CONFIG)/obj/ecCompiler.o
DEPS_120 += $(CONFIG)/obj/ecLex.o
DEPS_120 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_120 += $(CONFIG)/obj/ecParser.o
DEPS_120 += $(CONFIG)/obj/ecState.o
DEPS_120 += $(CONFIG)/obj/dtoa.o
DEPS_120 += $(CONFIG)/obj/ejsApp.o
DEPS_120 += $(CONFIG)/obj/ejsArray.o
DEPS_120 += $(CONFIG)/obj/ejsBlock.o
DEPS_120 += $(CONFIG)/obj/ejsBoolean.o
DEPS_120 += $(CONFIG)/obj/ejsByteArray.o
DEPS_120 += $(CONFIG)/obj/ejsCache.o
DEPS_120 += $(CONFIG)/obj/ejsCmd.o
DEPS_120 += $(CONFIG)/obj/ejsConfig.o
DEPS_120 += $(CONFIG)/obj/ejsDate.o
DEPS_120 += $(CONFIG)/obj/ejsDebug.o
DEPS_120 += $(CONFIG)/obj/ejsError.o
DEPS_120 += $(CONFIG)/obj/ejsFile.o
DEPS_120 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_120 += $(CONFIG)/obj/ejsFrame.o
DEPS_120 += $(CONFIG)/obj/ejsFunction.o
DEPS_120 += $(CONFIG)/obj/ejsGC.o
DEPS_120 += $(CONFIG)/obj/ejsGlobal.o
DEPS_120 += $(CONFIG)/obj/ejsHttp.o
DEPS_120 += $(CONFIG)/obj/ejsIterator.o
DEPS_120 += $(CONFIG)/obj/ejsJSON.o
DEPS_120 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_120 += $(CONFIG)/obj/ejsMath.o
DEPS_120 += $(CONFIG)/obj/ejsMemory.o
DEPS_120 += $(CONFIG)/obj/ejsMprLog.o
DEPS_120 += $(CONFIG)/obj/ejsNamespace.o
DEPS_120 += $(CONFIG)/obj/ejsNull.o
DEPS_120 += $(CONFIG)/obj/ejsNumber.o
DEPS_120 += $(CONFIG)/obj/ejsObject.o
DEPS_120 += $(CONFIG)/obj/ejsPath.o
DEPS_120 += $(CONFIG)/obj/ejsPot.o
DEPS_120 += $(CONFIG)/obj/ejsRegExp.o
DEPS_120 += $(CONFIG)/obj/ejsSocket.o
DEPS_120 += $(CONFIG)/obj/ejsString.o
DEPS_120 += $(CONFIG)/obj/ejsSystem.o
DEPS_120 += $(CONFIG)/obj/ejsTimer.o
DEPS_120 += $(CONFIG)/obj/ejsType.o
DEPS_120 += $(CONFIG)/obj/ejsUri.o
DEPS_120 += $(CONFIG)/obj/ejsVoid.o
DEPS_120 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_120 += $(CONFIG)/obj/ejsWorker.o
DEPS_120 += $(CONFIG)/obj/ejsXML.o
DEPS_120 += $(CONFIG)/obj/ejsXMLList.o
DEPS_120 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_120 += $(CONFIG)/obj/ejsByteCode.o
DEPS_120 += $(CONFIG)/obj/ejsException.o
DEPS_120 += $(CONFIG)/obj/ejsHelper.o
DEPS_120 += $(CONFIG)/obj/ejsInterp.o
DEPS_120 += $(CONFIG)/obj/ejsLoader.o
DEPS_120 += $(CONFIG)/obj/ejsModule.o
DEPS_120 += $(CONFIG)/obj/ejsScope.o
DEPS_120 += $(CONFIG)/obj/ejsService.o
DEPS_120 += $(CONFIG)/bin/libejs.a
DEPS_120 += $(CONFIG)/obj/ejsc.o
DEPS_120 += $(CONFIG)/bin/ejsc
DEPS_120 += src/cmd/ejsmod.h
DEPS_120 += $(CONFIG)/obj/ejsmod.o
DEPS_120 += $(CONFIG)/obj/doc.o
DEPS_120 += $(CONFIG)/obj/docFiles.o
DEPS_120 += $(CONFIG)/obj/listing.o
DEPS_120 += $(CONFIG)/obj/slotGen.o
DEPS_120 += $(CONFIG)/bin/ejsmod
DEPS_120 += $(CONFIG)/bin/ejs.mod
DEPS_120 += $(CONFIG)/bin/ejs.db.sqlite.mod
DEPS_120 += $(CONFIG)/inc/sqlite3.h
DEPS_120 += $(CONFIG)/obj/sqlite3.o
ifeq ($(ME_EXT_SQLITE),1)
    DEPS_120 += $(CONFIG)/bin/libsql.a
endif
DEPS_120 += $(CONFIG)/obj/ejsSqlite.o

$(CONFIG)/bin/libejs.db.sqlite.a: $(DEPS_120)
	@echo '      [Link] $(CONFIG)/bin/libejs.db.sqlite.a'
	ar -cr $(CONFIG)/bin/libejs.db.sqlite.a "$(CONFIG)/obj/ejsSqlite.o"

#
#   ejs.mail.mod
#
DEPS_121 += src/ejs.mail/Mail.es
DEPS_121 += $(CONFIG)/inc/mpr.h
DEPS_121 += $(CONFIG)/inc/me.h
DEPS_121 += $(CONFIG)/inc/osdep.h
DEPS_121 += $(CONFIG)/obj/mprLib.o
DEPS_121 += $(CONFIG)/bin/libmpr.a
DEPS_121 += $(CONFIG)/inc/pcre.h
DEPS_121 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_121 += $(CONFIG)/bin/libpcre.a
endif
DEPS_121 += $(CONFIG)/inc/http.h
DEPS_121 += $(CONFIG)/obj/httpLib.o
DEPS_121 += $(CONFIG)/bin/libhttp.a
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
DEPS_121 += $(CONFIG)/bin/ejsc
DEPS_121 += src/cmd/ejsmod.h
DEPS_121 += $(CONFIG)/obj/ejsmod.o
DEPS_121 += $(CONFIG)/obj/doc.o
DEPS_121 += $(CONFIG)/obj/docFiles.o
DEPS_121 += $(CONFIG)/obj/listing.o
DEPS_121 += $(CONFIG)/obj/slotGen.o
DEPS_121 += $(CONFIG)/bin/ejsmod
DEPS_121 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.mail.mod: $(DEPS_121)
	( \
	cd src/ejs.mail; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.mail.mod  --optimize 9 *.es ; \
	)

#
#   ejs.web.mod
#
DEPS_122 += src/ejs.web/Cascade.es
DEPS_122 += src/ejs.web/CommonLog.es
DEPS_122 += src/ejs.web/ContentType.es
DEPS_122 += src/ejs.web/Controller.es
DEPS_122 += src/ejs.web/Dir.es
DEPS_122 += src/ejs.web/Google.es
DEPS_122 += src/ejs.web/Head.es
DEPS_122 += src/ejs.web/Html.es
DEPS_122 += src/ejs.web/HttpServer.es
DEPS_122 += src/ejs.web/MethodOverride.es
DEPS_122 += src/ejs.web/Middleware.es
DEPS_122 += src/ejs.web/Mvc.es
DEPS_122 += src/ejs.web/Request.es
DEPS_122 += src/ejs.web/Router.es
DEPS_122 += src/ejs.web/Script.es
DEPS_122 += src/ejs.web/Session.es
DEPS_122 += src/ejs.web/ShowExceptions.es
DEPS_122 += src/ejs.web/Static.es
DEPS_122 += src/ejs.web/Template.es
DEPS_122 += src/ejs.web/UploadFile.es
DEPS_122 += src/ejs.web/UrlMap.es
DEPS_122 += src/ejs.web/Utils.es
DEPS_122 += src/ejs.web/View.es
DEPS_122 += $(CONFIG)/inc/mpr.h
DEPS_122 += $(CONFIG)/inc/me.h
DEPS_122 += $(CONFIG)/inc/osdep.h
DEPS_122 += $(CONFIG)/obj/mprLib.o
DEPS_122 += $(CONFIG)/bin/libmpr.a
DEPS_122 += $(CONFIG)/inc/pcre.h
DEPS_122 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_122 += $(CONFIG)/bin/libpcre.a
endif
DEPS_122 += $(CONFIG)/inc/http.h
DEPS_122 += $(CONFIG)/obj/httpLib.o
DEPS_122 += $(CONFIG)/bin/libhttp.a
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
DEPS_122 += $(CONFIG)/bin/libejs.a
DEPS_122 += $(CONFIG)/obj/ejsc.o
DEPS_122 += $(CONFIG)/bin/ejsc
DEPS_122 += src/cmd/ejsmod.h
DEPS_122 += $(CONFIG)/obj/ejsmod.o
DEPS_122 += $(CONFIG)/obj/doc.o
DEPS_122 += $(CONFIG)/obj/docFiles.o
DEPS_122 += $(CONFIG)/obj/listing.o
DEPS_122 += $(CONFIG)/obj/slotGen.o
DEPS_122 += $(CONFIG)/bin/ejsmod
DEPS_122 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.web.mod: $(DEPS_122)
	( \
	cd src/ejs.web; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.web.mod  --optimize 9 *.es ; \
	../../$(CONFIG)/bin/ejsmod --cslots ../../$(CONFIG)/bin/ejs.web.mod ; \
	if ! diff ejs.web.slots.h ../../$(CONFIG)/inc/ejs.web.slots.h >/dev/null; then cp ejs.web.slots.h ../../$(CONFIG)/inc; fi ; \
	rm -f ejs.web.slots.h ; \
	)

#
#   ejsWeb.h
#
$(CONFIG)/inc/ejsWeb.h: $(DEPS_123)
	@echo '      [Copy] $(CONFIG)/inc/ejsWeb.h'
	mkdir -p "$(CONFIG)/inc"
	cp src/ejs.web/ejsWeb.h $(CONFIG)/inc/ejsWeb.h

#
#   ejsHttpServer.o
#
DEPS_124 += $(CONFIG)/inc/me.h
DEPS_124 += $(CONFIG)/inc/mpr.h
DEPS_124 += $(CONFIG)/inc/http.h
DEPS_124 += $(CONFIG)/inc/ejsByteCode.h
DEPS_124 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_124 += $(CONFIG)/inc/ejs.slots.h
DEPS_124 += $(CONFIG)/inc/ejsCustomize.h
DEPS_124 += $(CONFIG)/inc/ejs.h
DEPS_124 += $(CONFIG)/inc/ejsCompiler.h
DEPS_124 += $(CONFIG)/inc/ejsWeb.h
DEPS_124 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsHttpServer.o: \
    src/ejs.web/ejsHttpServer.c $(DEPS_124)
	@echo '   [Compile] $(CONFIG)/obj/ejsHttpServer.o'
	$(CC) -c -o $(CONFIG)/obj/ejsHttpServer.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.web/ejsHttpServer.c

#
#   ejsRequest.o
#
DEPS_125 += $(CONFIG)/inc/me.h
DEPS_125 += $(CONFIG)/inc/mpr.h
DEPS_125 += $(CONFIG)/inc/http.h
DEPS_125 += $(CONFIG)/inc/ejsByteCode.h
DEPS_125 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_125 += $(CONFIG)/inc/ejs.slots.h
DEPS_125 += $(CONFIG)/inc/ejsCustomize.h
DEPS_125 += $(CONFIG)/inc/ejs.h
DEPS_125 += $(CONFIG)/inc/ejsCompiler.h
DEPS_125 += $(CONFIG)/inc/ejsWeb.h
DEPS_125 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsRequest.o: \
    src/ejs.web/ejsRequest.c $(DEPS_125)
	@echo '   [Compile] $(CONFIG)/obj/ejsRequest.o'
	$(CC) -c -o $(CONFIG)/obj/ejsRequest.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.web/ejsRequest.c

#
#   ejsSession.o
#
DEPS_126 += $(CONFIG)/inc/me.h
DEPS_126 += $(CONFIG)/inc/mpr.h
DEPS_126 += $(CONFIG)/inc/http.h
DEPS_126 += $(CONFIG)/inc/ejsByteCode.h
DEPS_126 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_126 += $(CONFIG)/inc/ejs.slots.h
DEPS_126 += $(CONFIG)/inc/ejsCustomize.h
DEPS_126 += $(CONFIG)/inc/ejs.h
DEPS_126 += $(CONFIG)/inc/ejsWeb.h

$(CONFIG)/obj/ejsSession.o: \
    src/ejs.web/ejsSession.c $(DEPS_126)
	@echo '   [Compile] $(CONFIG)/obj/ejsSession.o'
	$(CC) -c -o $(CONFIG)/obj/ejsSession.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.web/ejsSession.c

#
#   ejsWeb.o
#
DEPS_127 += $(CONFIG)/inc/me.h
DEPS_127 += $(CONFIG)/inc/mpr.h
DEPS_127 += $(CONFIG)/inc/http.h
DEPS_127 += $(CONFIG)/inc/ejsByteCode.h
DEPS_127 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_127 += $(CONFIG)/inc/ejs.slots.h
DEPS_127 += $(CONFIG)/inc/ejsCustomize.h
DEPS_127 += $(CONFIG)/inc/ejs.h
DEPS_127 += $(CONFIG)/inc/ejsCompiler.h
DEPS_127 += $(CONFIG)/inc/ejsWeb.h
DEPS_127 += $(CONFIG)/inc/ejs.web.slots.h

$(CONFIG)/obj/ejsWeb.o: \
    src/ejs.web/ejsWeb.c $(DEPS_127)
	@echo '   [Compile] $(CONFIG)/obj/ejsWeb.o'
	$(CC) -c -o $(CONFIG)/obj/ejsWeb.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.web/ejsWeb.c

#
#   libejs.web
#
DEPS_128 += $(CONFIG)/inc/mpr.h
DEPS_128 += $(CONFIG)/inc/me.h
DEPS_128 += $(CONFIG)/inc/osdep.h
DEPS_128 += $(CONFIG)/obj/mprLib.o
DEPS_128 += $(CONFIG)/bin/libmpr.a
DEPS_128 += $(CONFIG)/inc/pcre.h
DEPS_128 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_128 += $(CONFIG)/bin/libpcre.a
endif
DEPS_128 += $(CONFIG)/inc/http.h
DEPS_128 += $(CONFIG)/obj/httpLib.o
DEPS_128 += $(CONFIG)/bin/libhttp.a
DEPS_128 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_128 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_128 += $(CONFIG)/inc/ejs.slots.h
DEPS_128 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_128 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_128 += $(CONFIG)/inc/ejsByteCode.h
DEPS_128 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_128 += $(CONFIG)/inc/ejsCustomize.h
DEPS_128 += $(CONFIG)/inc/ejs.h
DEPS_128 += $(CONFIG)/inc/ejsCompiler.h
DEPS_128 += $(CONFIG)/obj/ecAst.o
DEPS_128 += $(CONFIG)/obj/ecCodeGen.o
DEPS_128 += $(CONFIG)/obj/ecCompiler.o
DEPS_128 += $(CONFIG)/obj/ecLex.o
DEPS_128 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_128 += $(CONFIG)/obj/ecParser.o
DEPS_128 += $(CONFIG)/obj/ecState.o
DEPS_128 += $(CONFIG)/obj/dtoa.o
DEPS_128 += $(CONFIG)/obj/ejsApp.o
DEPS_128 += $(CONFIG)/obj/ejsArray.o
DEPS_128 += $(CONFIG)/obj/ejsBlock.o
DEPS_128 += $(CONFIG)/obj/ejsBoolean.o
DEPS_128 += $(CONFIG)/obj/ejsByteArray.o
DEPS_128 += $(CONFIG)/obj/ejsCache.o
DEPS_128 += $(CONFIG)/obj/ejsCmd.o
DEPS_128 += $(CONFIG)/obj/ejsConfig.o
DEPS_128 += $(CONFIG)/obj/ejsDate.o
DEPS_128 += $(CONFIG)/obj/ejsDebug.o
DEPS_128 += $(CONFIG)/obj/ejsError.o
DEPS_128 += $(CONFIG)/obj/ejsFile.o
DEPS_128 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_128 += $(CONFIG)/obj/ejsFrame.o
DEPS_128 += $(CONFIG)/obj/ejsFunction.o
DEPS_128 += $(CONFIG)/obj/ejsGC.o
DEPS_128 += $(CONFIG)/obj/ejsGlobal.o
DEPS_128 += $(CONFIG)/obj/ejsHttp.o
DEPS_128 += $(CONFIG)/obj/ejsIterator.o
DEPS_128 += $(CONFIG)/obj/ejsJSON.o
DEPS_128 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_128 += $(CONFIG)/obj/ejsMath.o
DEPS_128 += $(CONFIG)/obj/ejsMemory.o
DEPS_128 += $(CONFIG)/obj/ejsMprLog.o
DEPS_128 += $(CONFIG)/obj/ejsNamespace.o
DEPS_128 += $(CONFIG)/obj/ejsNull.o
DEPS_128 += $(CONFIG)/obj/ejsNumber.o
DEPS_128 += $(CONFIG)/obj/ejsObject.o
DEPS_128 += $(CONFIG)/obj/ejsPath.o
DEPS_128 += $(CONFIG)/obj/ejsPot.o
DEPS_128 += $(CONFIG)/obj/ejsRegExp.o
DEPS_128 += $(CONFIG)/obj/ejsSocket.o
DEPS_128 += $(CONFIG)/obj/ejsString.o
DEPS_128 += $(CONFIG)/obj/ejsSystem.o
DEPS_128 += $(CONFIG)/obj/ejsTimer.o
DEPS_128 += $(CONFIG)/obj/ejsType.o
DEPS_128 += $(CONFIG)/obj/ejsUri.o
DEPS_128 += $(CONFIG)/obj/ejsVoid.o
DEPS_128 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_128 += $(CONFIG)/obj/ejsWorker.o
DEPS_128 += $(CONFIG)/obj/ejsXML.o
DEPS_128 += $(CONFIG)/obj/ejsXMLList.o
DEPS_128 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_128 += $(CONFIG)/obj/ejsByteCode.o
DEPS_128 += $(CONFIG)/obj/ejsException.o
DEPS_128 += $(CONFIG)/obj/ejsHelper.o
DEPS_128 += $(CONFIG)/obj/ejsInterp.o
DEPS_128 += $(CONFIG)/obj/ejsLoader.o
DEPS_128 += $(CONFIG)/obj/ejsModule.o
DEPS_128 += $(CONFIG)/obj/ejsScope.o
DEPS_128 += $(CONFIG)/obj/ejsService.o
DEPS_128 += $(CONFIG)/bin/libejs.a
DEPS_128 += $(CONFIG)/obj/ejsc.o
DEPS_128 += $(CONFIG)/bin/ejsc
DEPS_128 += src/cmd/ejsmod.h
DEPS_128 += $(CONFIG)/obj/ejsmod.o
DEPS_128 += $(CONFIG)/obj/doc.o
DEPS_128 += $(CONFIG)/obj/docFiles.o
DEPS_128 += $(CONFIG)/obj/listing.o
DEPS_128 += $(CONFIG)/obj/slotGen.o
DEPS_128 += $(CONFIG)/bin/ejsmod
DEPS_128 += $(CONFIG)/bin/ejs.mod
DEPS_128 += $(CONFIG)/inc/ejsWeb.h
DEPS_128 += $(CONFIG)/obj/ejsHttpServer.o
DEPS_128 += $(CONFIG)/obj/ejsRequest.o
DEPS_128 += $(CONFIG)/obj/ejsSession.o
DEPS_128 += $(CONFIG)/obj/ejsWeb.o

$(CONFIG)/bin/libejs.web.a: $(DEPS_128)
	@echo '      [Link] $(CONFIG)/bin/libejs.web.a'
	ar -cr $(CONFIG)/bin/libejs.web.a "$(CONFIG)/obj/ejsHttpServer.o" "$(CONFIG)/obj/ejsRequest.o" "$(CONFIG)/obj/ejsSession.o" "$(CONFIG)/obj/ejsWeb.o"

#
#   www
#
DEPS_129 += src/ejs.web/www

$(CONFIG)/bin/www: $(DEPS_129)
	( \
	cd src/ejs.web; \
	rm -fr ../../$(CONFIG)/bin/www ; \
	cp -r www ../../$(CONFIG)/bin ; \
	)

#
#   ejs.template.mod
#
DEPS_130 += src/ejs.template/TemplateParser.es
DEPS_130 += $(CONFIG)/inc/mpr.h
DEPS_130 += $(CONFIG)/inc/me.h
DEPS_130 += $(CONFIG)/inc/osdep.h
DEPS_130 += $(CONFIG)/obj/mprLib.o
DEPS_130 += $(CONFIG)/bin/libmpr.a
DEPS_130 += $(CONFIG)/inc/pcre.h
DEPS_130 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_130 += $(CONFIG)/bin/libpcre.a
endif
DEPS_130 += $(CONFIG)/inc/http.h
DEPS_130 += $(CONFIG)/obj/httpLib.o
DEPS_130 += $(CONFIG)/bin/libhttp.a
DEPS_130 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_130 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_130 += $(CONFIG)/inc/ejs.slots.h
DEPS_130 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_130 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_130 += $(CONFIG)/inc/ejsByteCode.h
DEPS_130 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_130 += $(CONFIG)/inc/ejsCustomize.h
DEPS_130 += $(CONFIG)/inc/ejs.h
DEPS_130 += $(CONFIG)/inc/ejsCompiler.h
DEPS_130 += $(CONFIG)/obj/ecAst.o
DEPS_130 += $(CONFIG)/obj/ecCodeGen.o
DEPS_130 += $(CONFIG)/obj/ecCompiler.o
DEPS_130 += $(CONFIG)/obj/ecLex.o
DEPS_130 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_130 += $(CONFIG)/obj/ecParser.o
DEPS_130 += $(CONFIG)/obj/ecState.o
DEPS_130 += $(CONFIG)/obj/dtoa.o
DEPS_130 += $(CONFIG)/obj/ejsApp.o
DEPS_130 += $(CONFIG)/obj/ejsArray.o
DEPS_130 += $(CONFIG)/obj/ejsBlock.o
DEPS_130 += $(CONFIG)/obj/ejsBoolean.o
DEPS_130 += $(CONFIG)/obj/ejsByteArray.o
DEPS_130 += $(CONFIG)/obj/ejsCache.o
DEPS_130 += $(CONFIG)/obj/ejsCmd.o
DEPS_130 += $(CONFIG)/obj/ejsConfig.o
DEPS_130 += $(CONFIG)/obj/ejsDate.o
DEPS_130 += $(CONFIG)/obj/ejsDebug.o
DEPS_130 += $(CONFIG)/obj/ejsError.o
DEPS_130 += $(CONFIG)/obj/ejsFile.o
DEPS_130 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_130 += $(CONFIG)/obj/ejsFrame.o
DEPS_130 += $(CONFIG)/obj/ejsFunction.o
DEPS_130 += $(CONFIG)/obj/ejsGC.o
DEPS_130 += $(CONFIG)/obj/ejsGlobal.o
DEPS_130 += $(CONFIG)/obj/ejsHttp.o
DEPS_130 += $(CONFIG)/obj/ejsIterator.o
DEPS_130 += $(CONFIG)/obj/ejsJSON.o
DEPS_130 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_130 += $(CONFIG)/obj/ejsMath.o
DEPS_130 += $(CONFIG)/obj/ejsMemory.o
DEPS_130 += $(CONFIG)/obj/ejsMprLog.o
DEPS_130 += $(CONFIG)/obj/ejsNamespace.o
DEPS_130 += $(CONFIG)/obj/ejsNull.o
DEPS_130 += $(CONFIG)/obj/ejsNumber.o
DEPS_130 += $(CONFIG)/obj/ejsObject.o
DEPS_130 += $(CONFIG)/obj/ejsPath.o
DEPS_130 += $(CONFIG)/obj/ejsPot.o
DEPS_130 += $(CONFIG)/obj/ejsRegExp.o
DEPS_130 += $(CONFIG)/obj/ejsSocket.o
DEPS_130 += $(CONFIG)/obj/ejsString.o
DEPS_130 += $(CONFIG)/obj/ejsSystem.o
DEPS_130 += $(CONFIG)/obj/ejsTimer.o
DEPS_130 += $(CONFIG)/obj/ejsType.o
DEPS_130 += $(CONFIG)/obj/ejsUri.o
DEPS_130 += $(CONFIG)/obj/ejsVoid.o
DEPS_130 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_130 += $(CONFIG)/obj/ejsWorker.o
DEPS_130 += $(CONFIG)/obj/ejsXML.o
DEPS_130 += $(CONFIG)/obj/ejsXMLList.o
DEPS_130 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_130 += $(CONFIG)/obj/ejsByteCode.o
DEPS_130 += $(CONFIG)/obj/ejsException.o
DEPS_130 += $(CONFIG)/obj/ejsHelper.o
DEPS_130 += $(CONFIG)/obj/ejsInterp.o
DEPS_130 += $(CONFIG)/obj/ejsLoader.o
DEPS_130 += $(CONFIG)/obj/ejsModule.o
DEPS_130 += $(CONFIG)/obj/ejsScope.o
DEPS_130 += $(CONFIG)/obj/ejsService.o
DEPS_130 += $(CONFIG)/bin/libejs.a
DEPS_130 += $(CONFIG)/obj/ejsc.o
DEPS_130 += $(CONFIG)/bin/ejsc
DEPS_130 += src/cmd/ejsmod.h
DEPS_130 += $(CONFIG)/obj/ejsmod.o
DEPS_130 += $(CONFIG)/obj/doc.o
DEPS_130 += $(CONFIG)/obj/docFiles.o
DEPS_130 += $(CONFIG)/obj/listing.o
DEPS_130 += $(CONFIG)/obj/slotGen.o
DEPS_130 += $(CONFIG)/bin/ejsmod
DEPS_130 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.template.mod: $(DEPS_130)
	( \
	cd src/ejs.template; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.template.mod  --optimize 9 TemplateParser.es ; \
	)

#
#   ejs.zlib.mod
#
DEPS_131 += src/ejs.zlib/Zlib.es
DEPS_131 += $(CONFIG)/inc/mpr.h
DEPS_131 += $(CONFIG)/inc/me.h
DEPS_131 += $(CONFIG)/inc/osdep.h
DEPS_131 += $(CONFIG)/obj/mprLib.o
DEPS_131 += $(CONFIG)/bin/libmpr.a
DEPS_131 += $(CONFIG)/inc/pcre.h
DEPS_131 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_131 += $(CONFIG)/bin/libpcre.a
endif
DEPS_131 += $(CONFIG)/inc/http.h
DEPS_131 += $(CONFIG)/obj/httpLib.o
DEPS_131 += $(CONFIG)/bin/libhttp.a
DEPS_131 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_131 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_131 += $(CONFIG)/inc/ejs.slots.h
DEPS_131 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_131 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_131 += $(CONFIG)/inc/ejsByteCode.h
DEPS_131 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_131 += $(CONFIG)/inc/ejsCustomize.h
DEPS_131 += $(CONFIG)/inc/ejs.h
DEPS_131 += $(CONFIG)/inc/ejsCompiler.h
DEPS_131 += $(CONFIG)/obj/ecAst.o
DEPS_131 += $(CONFIG)/obj/ecCodeGen.o
DEPS_131 += $(CONFIG)/obj/ecCompiler.o
DEPS_131 += $(CONFIG)/obj/ecLex.o
DEPS_131 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_131 += $(CONFIG)/obj/ecParser.o
DEPS_131 += $(CONFIG)/obj/ecState.o
DEPS_131 += $(CONFIG)/obj/dtoa.o
DEPS_131 += $(CONFIG)/obj/ejsApp.o
DEPS_131 += $(CONFIG)/obj/ejsArray.o
DEPS_131 += $(CONFIG)/obj/ejsBlock.o
DEPS_131 += $(CONFIG)/obj/ejsBoolean.o
DEPS_131 += $(CONFIG)/obj/ejsByteArray.o
DEPS_131 += $(CONFIG)/obj/ejsCache.o
DEPS_131 += $(CONFIG)/obj/ejsCmd.o
DEPS_131 += $(CONFIG)/obj/ejsConfig.o
DEPS_131 += $(CONFIG)/obj/ejsDate.o
DEPS_131 += $(CONFIG)/obj/ejsDebug.o
DEPS_131 += $(CONFIG)/obj/ejsError.o
DEPS_131 += $(CONFIG)/obj/ejsFile.o
DEPS_131 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_131 += $(CONFIG)/obj/ejsFrame.o
DEPS_131 += $(CONFIG)/obj/ejsFunction.o
DEPS_131 += $(CONFIG)/obj/ejsGC.o
DEPS_131 += $(CONFIG)/obj/ejsGlobal.o
DEPS_131 += $(CONFIG)/obj/ejsHttp.o
DEPS_131 += $(CONFIG)/obj/ejsIterator.o
DEPS_131 += $(CONFIG)/obj/ejsJSON.o
DEPS_131 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_131 += $(CONFIG)/obj/ejsMath.o
DEPS_131 += $(CONFIG)/obj/ejsMemory.o
DEPS_131 += $(CONFIG)/obj/ejsMprLog.o
DEPS_131 += $(CONFIG)/obj/ejsNamespace.o
DEPS_131 += $(CONFIG)/obj/ejsNull.o
DEPS_131 += $(CONFIG)/obj/ejsNumber.o
DEPS_131 += $(CONFIG)/obj/ejsObject.o
DEPS_131 += $(CONFIG)/obj/ejsPath.o
DEPS_131 += $(CONFIG)/obj/ejsPot.o
DEPS_131 += $(CONFIG)/obj/ejsRegExp.o
DEPS_131 += $(CONFIG)/obj/ejsSocket.o
DEPS_131 += $(CONFIG)/obj/ejsString.o
DEPS_131 += $(CONFIG)/obj/ejsSystem.o
DEPS_131 += $(CONFIG)/obj/ejsTimer.o
DEPS_131 += $(CONFIG)/obj/ejsType.o
DEPS_131 += $(CONFIG)/obj/ejsUri.o
DEPS_131 += $(CONFIG)/obj/ejsVoid.o
DEPS_131 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_131 += $(CONFIG)/obj/ejsWorker.o
DEPS_131 += $(CONFIG)/obj/ejsXML.o
DEPS_131 += $(CONFIG)/obj/ejsXMLList.o
DEPS_131 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_131 += $(CONFIG)/obj/ejsByteCode.o
DEPS_131 += $(CONFIG)/obj/ejsException.o
DEPS_131 += $(CONFIG)/obj/ejsHelper.o
DEPS_131 += $(CONFIG)/obj/ejsInterp.o
DEPS_131 += $(CONFIG)/obj/ejsLoader.o
DEPS_131 += $(CONFIG)/obj/ejsModule.o
DEPS_131 += $(CONFIG)/obj/ejsScope.o
DEPS_131 += $(CONFIG)/obj/ejsService.o
DEPS_131 += $(CONFIG)/bin/libejs.a
DEPS_131 += $(CONFIG)/obj/ejsc.o
DEPS_131 += $(CONFIG)/bin/ejsc
DEPS_131 += src/cmd/ejsmod.h
DEPS_131 += $(CONFIG)/obj/ejsmod.o
DEPS_131 += $(CONFIG)/obj/doc.o
DEPS_131 += $(CONFIG)/obj/docFiles.o
DEPS_131 += $(CONFIG)/obj/listing.o
DEPS_131 += $(CONFIG)/obj/slotGen.o
DEPS_131 += $(CONFIG)/bin/ejsmod
DEPS_131 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.zlib.mod: $(DEPS_131)
	( \
	cd src/ejs.zlib; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.zlib.mod  --optimize 9 *.es ; \
	)

#
#   ejsZlib.o
#
DEPS_132 += $(CONFIG)/inc/me.h
DEPS_132 += $(CONFIG)/inc/mpr.h
DEPS_132 += $(CONFIG)/inc/http.h
DEPS_132 += $(CONFIG)/inc/ejsByteCode.h
DEPS_132 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_132 += $(CONFIG)/inc/ejs.slots.h
DEPS_132 += $(CONFIG)/inc/ejsCustomize.h
DEPS_132 += $(CONFIG)/inc/ejs.h
DEPS_132 += $(CONFIG)/inc/zlib.h
DEPS_132 += $(CONFIG)/inc/ejs.zlib.slots.h

$(CONFIG)/obj/ejsZlib.o: \
    src/ejs.zlib/ejsZlib.c $(DEPS_132)
	@echo '   [Compile] $(CONFIG)/obj/ejsZlib.o'
	$(CC) -c -o $(CONFIG)/obj/ejsZlib.o $(CFLAGS) $(DFLAGS) $(IFLAGS) "-Isrc/cmd" src/ejs.zlib/ejsZlib.c

#
#   libejs.zlib
#
DEPS_133 += $(CONFIG)/inc/mpr.h
DEPS_133 += $(CONFIG)/inc/me.h
DEPS_133 += $(CONFIG)/inc/osdep.h
DEPS_133 += $(CONFIG)/obj/mprLib.o
DEPS_133 += $(CONFIG)/bin/libmpr.a
DEPS_133 += $(CONFIG)/inc/pcre.h
DEPS_133 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_133 += $(CONFIG)/bin/libpcre.a
endif
DEPS_133 += $(CONFIG)/inc/http.h
DEPS_133 += $(CONFIG)/obj/httpLib.o
DEPS_133 += $(CONFIG)/bin/libhttp.a
DEPS_133 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_133 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_133 += $(CONFIG)/inc/ejs.slots.h
DEPS_133 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_133 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_133 += $(CONFIG)/inc/ejsByteCode.h
DEPS_133 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_133 += $(CONFIG)/inc/ejsCustomize.h
DEPS_133 += $(CONFIG)/inc/ejs.h
DEPS_133 += $(CONFIG)/inc/ejsCompiler.h
DEPS_133 += $(CONFIG)/obj/ecAst.o
DEPS_133 += $(CONFIG)/obj/ecCodeGen.o
DEPS_133 += $(CONFIG)/obj/ecCompiler.o
DEPS_133 += $(CONFIG)/obj/ecLex.o
DEPS_133 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_133 += $(CONFIG)/obj/ecParser.o
DEPS_133 += $(CONFIG)/obj/ecState.o
DEPS_133 += $(CONFIG)/obj/dtoa.o
DEPS_133 += $(CONFIG)/obj/ejsApp.o
DEPS_133 += $(CONFIG)/obj/ejsArray.o
DEPS_133 += $(CONFIG)/obj/ejsBlock.o
DEPS_133 += $(CONFIG)/obj/ejsBoolean.o
DEPS_133 += $(CONFIG)/obj/ejsByteArray.o
DEPS_133 += $(CONFIG)/obj/ejsCache.o
DEPS_133 += $(CONFIG)/obj/ejsCmd.o
DEPS_133 += $(CONFIG)/obj/ejsConfig.o
DEPS_133 += $(CONFIG)/obj/ejsDate.o
DEPS_133 += $(CONFIG)/obj/ejsDebug.o
DEPS_133 += $(CONFIG)/obj/ejsError.o
DEPS_133 += $(CONFIG)/obj/ejsFile.o
DEPS_133 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_133 += $(CONFIG)/obj/ejsFrame.o
DEPS_133 += $(CONFIG)/obj/ejsFunction.o
DEPS_133 += $(CONFIG)/obj/ejsGC.o
DEPS_133 += $(CONFIG)/obj/ejsGlobal.o
DEPS_133 += $(CONFIG)/obj/ejsHttp.o
DEPS_133 += $(CONFIG)/obj/ejsIterator.o
DEPS_133 += $(CONFIG)/obj/ejsJSON.o
DEPS_133 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_133 += $(CONFIG)/obj/ejsMath.o
DEPS_133 += $(CONFIG)/obj/ejsMemory.o
DEPS_133 += $(CONFIG)/obj/ejsMprLog.o
DEPS_133 += $(CONFIG)/obj/ejsNamespace.o
DEPS_133 += $(CONFIG)/obj/ejsNull.o
DEPS_133 += $(CONFIG)/obj/ejsNumber.o
DEPS_133 += $(CONFIG)/obj/ejsObject.o
DEPS_133 += $(CONFIG)/obj/ejsPath.o
DEPS_133 += $(CONFIG)/obj/ejsPot.o
DEPS_133 += $(CONFIG)/obj/ejsRegExp.o
DEPS_133 += $(CONFIG)/obj/ejsSocket.o
DEPS_133 += $(CONFIG)/obj/ejsString.o
DEPS_133 += $(CONFIG)/obj/ejsSystem.o
DEPS_133 += $(CONFIG)/obj/ejsTimer.o
DEPS_133 += $(CONFIG)/obj/ejsType.o
DEPS_133 += $(CONFIG)/obj/ejsUri.o
DEPS_133 += $(CONFIG)/obj/ejsVoid.o
DEPS_133 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_133 += $(CONFIG)/obj/ejsWorker.o
DEPS_133 += $(CONFIG)/obj/ejsXML.o
DEPS_133 += $(CONFIG)/obj/ejsXMLList.o
DEPS_133 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_133 += $(CONFIG)/obj/ejsByteCode.o
DEPS_133 += $(CONFIG)/obj/ejsException.o
DEPS_133 += $(CONFIG)/obj/ejsHelper.o
DEPS_133 += $(CONFIG)/obj/ejsInterp.o
DEPS_133 += $(CONFIG)/obj/ejsLoader.o
DEPS_133 += $(CONFIG)/obj/ejsModule.o
DEPS_133 += $(CONFIG)/obj/ejsScope.o
DEPS_133 += $(CONFIG)/obj/ejsService.o
DEPS_133 += $(CONFIG)/bin/libejs.a
DEPS_133 += $(CONFIG)/obj/ejsc.o
DEPS_133 += $(CONFIG)/bin/ejsc
DEPS_133 += src/cmd/ejsmod.h
DEPS_133 += $(CONFIG)/obj/ejsmod.o
DEPS_133 += $(CONFIG)/obj/doc.o
DEPS_133 += $(CONFIG)/obj/docFiles.o
DEPS_133 += $(CONFIG)/obj/listing.o
DEPS_133 += $(CONFIG)/obj/slotGen.o
DEPS_133 += $(CONFIG)/bin/ejsmod
DEPS_133 += $(CONFIG)/bin/ejs.mod
DEPS_133 += $(CONFIG)/bin/ejs.zlib.mod
DEPS_133 += $(CONFIG)/inc/zlib.h
DEPS_133 += $(CONFIG)/obj/zlib.o
ifeq ($(ME_EXT_ZLIB),1)
    DEPS_133 += $(CONFIG)/bin/libzlib.a
endif
DEPS_133 += $(CONFIG)/obj/ejsZlib.o

$(CONFIG)/bin/libejs.zlib.a: $(DEPS_133)
	@echo '      [Link] $(CONFIG)/bin/libejs.zlib.a'
	ar -cr $(CONFIG)/bin/libejs.zlib.a "$(CONFIG)/obj/ejsZlib.o"

#
#   ejs.tar.mod
#
DEPS_134 += src/ejs.tar/Tar.es
DEPS_134 += $(CONFIG)/inc/mpr.h
DEPS_134 += $(CONFIG)/inc/me.h
DEPS_134 += $(CONFIG)/inc/osdep.h
DEPS_134 += $(CONFIG)/obj/mprLib.o
DEPS_134 += $(CONFIG)/bin/libmpr.a
DEPS_134 += $(CONFIG)/inc/pcre.h
DEPS_134 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_134 += $(CONFIG)/bin/libpcre.a
endif
DEPS_134 += $(CONFIG)/inc/http.h
DEPS_134 += $(CONFIG)/obj/httpLib.o
DEPS_134 += $(CONFIG)/bin/libhttp.a
DEPS_134 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_134 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_134 += $(CONFIG)/inc/ejs.slots.h
DEPS_134 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_134 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_134 += $(CONFIG)/inc/ejsByteCode.h
DEPS_134 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_134 += $(CONFIG)/inc/ejsCustomize.h
DEPS_134 += $(CONFIG)/inc/ejs.h
DEPS_134 += $(CONFIG)/inc/ejsCompiler.h
DEPS_134 += $(CONFIG)/obj/ecAst.o
DEPS_134 += $(CONFIG)/obj/ecCodeGen.o
DEPS_134 += $(CONFIG)/obj/ecCompiler.o
DEPS_134 += $(CONFIG)/obj/ecLex.o
DEPS_134 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_134 += $(CONFIG)/obj/ecParser.o
DEPS_134 += $(CONFIG)/obj/ecState.o
DEPS_134 += $(CONFIG)/obj/dtoa.o
DEPS_134 += $(CONFIG)/obj/ejsApp.o
DEPS_134 += $(CONFIG)/obj/ejsArray.o
DEPS_134 += $(CONFIG)/obj/ejsBlock.o
DEPS_134 += $(CONFIG)/obj/ejsBoolean.o
DEPS_134 += $(CONFIG)/obj/ejsByteArray.o
DEPS_134 += $(CONFIG)/obj/ejsCache.o
DEPS_134 += $(CONFIG)/obj/ejsCmd.o
DEPS_134 += $(CONFIG)/obj/ejsConfig.o
DEPS_134 += $(CONFIG)/obj/ejsDate.o
DEPS_134 += $(CONFIG)/obj/ejsDebug.o
DEPS_134 += $(CONFIG)/obj/ejsError.o
DEPS_134 += $(CONFIG)/obj/ejsFile.o
DEPS_134 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_134 += $(CONFIG)/obj/ejsFrame.o
DEPS_134 += $(CONFIG)/obj/ejsFunction.o
DEPS_134 += $(CONFIG)/obj/ejsGC.o
DEPS_134 += $(CONFIG)/obj/ejsGlobal.o
DEPS_134 += $(CONFIG)/obj/ejsHttp.o
DEPS_134 += $(CONFIG)/obj/ejsIterator.o
DEPS_134 += $(CONFIG)/obj/ejsJSON.o
DEPS_134 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_134 += $(CONFIG)/obj/ejsMath.o
DEPS_134 += $(CONFIG)/obj/ejsMemory.o
DEPS_134 += $(CONFIG)/obj/ejsMprLog.o
DEPS_134 += $(CONFIG)/obj/ejsNamespace.o
DEPS_134 += $(CONFIG)/obj/ejsNull.o
DEPS_134 += $(CONFIG)/obj/ejsNumber.o
DEPS_134 += $(CONFIG)/obj/ejsObject.o
DEPS_134 += $(CONFIG)/obj/ejsPath.o
DEPS_134 += $(CONFIG)/obj/ejsPot.o
DEPS_134 += $(CONFIG)/obj/ejsRegExp.o
DEPS_134 += $(CONFIG)/obj/ejsSocket.o
DEPS_134 += $(CONFIG)/obj/ejsString.o
DEPS_134 += $(CONFIG)/obj/ejsSystem.o
DEPS_134 += $(CONFIG)/obj/ejsTimer.o
DEPS_134 += $(CONFIG)/obj/ejsType.o
DEPS_134 += $(CONFIG)/obj/ejsUri.o
DEPS_134 += $(CONFIG)/obj/ejsVoid.o
DEPS_134 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_134 += $(CONFIG)/obj/ejsWorker.o
DEPS_134 += $(CONFIG)/obj/ejsXML.o
DEPS_134 += $(CONFIG)/obj/ejsXMLList.o
DEPS_134 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_134 += $(CONFIG)/obj/ejsByteCode.o
DEPS_134 += $(CONFIG)/obj/ejsException.o
DEPS_134 += $(CONFIG)/obj/ejsHelper.o
DEPS_134 += $(CONFIG)/obj/ejsInterp.o
DEPS_134 += $(CONFIG)/obj/ejsLoader.o
DEPS_134 += $(CONFIG)/obj/ejsModule.o
DEPS_134 += $(CONFIG)/obj/ejsScope.o
DEPS_134 += $(CONFIG)/obj/ejsService.o
DEPS_134 += $(CONFIG)/bin/libejs.a
DEPS_134 += $(CONFIG)/obj/ejsc.o
DEPS_134 += $(CONFIG)/bin/ejsc
DEPS_134 += src/cmd/ejsmod.h
DEPS_134 += $(CONFIG)/obj/ejsmod.o
DEPS_134 += $(CONFIG)/obj/doc.o
DEPS_134 += $(CONFIG)/obj/docFiles.o
DEPS_134 += $(CONFIG)/obj/listing.o
DEPS_134 += $(CONFIG)/obj/slotGen.o
DEPS_134 += $(CONFIG)/bin/ejsmod
DEPS_134 += $(CONFIG)/bin/ejs.mod

$(CONFIG)/bin/ejs.tar.mod: $(DEPS_134)
	( \
	cd src/ejs.tar; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.tar.mod  --optimize 9 *.es ; \
	)

#
#   mvc.es
#
DEPS_135 += src/ejs.mvc/mvc.es

$(CONFIG)/bin/mvc.es: $(DEPS_135)
	( \
	cd src/ejs.mvc; \
	cp mvc.es ../../$(CONFIG)/bin ; \
	)

#
#   mvc
#
DEPS_136 += $(CONFIG)/inc/mpr.h
DEPS_136 += $(CONFIG)/inc/me.h
DEPS_136 += $(CONFIG)/inc/osdep.h
DEPS_136 += $(CONFIG)/obj/mprLib.o
DEPS_136 += $(CONFIG)/bin/libmpr.a
DEPS_136 += $(CONFIG)/inc/pcre.h
DEPS_136 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_136 += $(CONFIG)/bin/libpcre.a
endif
DEPS_136 += $(CONFIG)/inc/http.h
DEPS_136 += $(CONFIG)/obj/httpLib.o
DEPS_136 += $(CONFIG)/bin/libhttp.a
DEPS_136 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_136 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_136 += $(CONFIG)/inc/ejs.slots.h
DEPS_136 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_136 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_136 += $(CONFIG)/inc/ejsByteCode.h
DEPS_136 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_136 += $(CONFIG)/inc/ejsCustomize.h
DEPS_136 += $(CONFIG)/inc/ejs.h
DEPS_136 += $(CONFIG)/inc/ejsCompiler.h
DEPS_136 += $(CONFIG)/obj/ecAst.o
DEPS_136 += $(CONFIG)/obj/ecCodeGen.o
DEPS_136 += $(CONFIG)/obj/ecCompiler.o
DEPS_136 += $(CONFIG)/obj/ecLex.o
DEPS_136 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_136 += $(CONFIG)/obj/ecParser.o
DEPS_136 += $(CONFIG)/obj/ecState.o
DEPS_136 += $(CONFIG)/obj/dtoa.o
DEPS_136 += $(CONFIG)/obj/ejsApp.o
DEPS_136 += $(CONFIG)/obj/ejsArray.o
DEPS_136 += $(CONFIG)/obj/ejsBlock.o
DEPS_136 += $(CONFIG)/obj/ejsBoolean.o
DEPS_136 += $(CONFIG)/obj/ejsByteArray.o
DEPS_136 += $(CONFIG)/obj/ejsCache.o
DEPS_136 += $(CONFIG)/obj/ejsCmd.o
DEPS_136 += $(CONFIG)/obj/ejsConfig.o
DEPS_136 += $(CONFIG)/obj/ejsDate.o
DEPS_136 += $(CONFIG)/obj/ejsDebug.o
DEPS_136 += $(CONFIG)/obj/ejsError.o
DEPS_136 += $(CONFIG)/obj/ejsFile.o
DEPS_136 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_136 += $(CONFIG)/obj/ejsFrame.o
DEPS_136 += $(CONFIG)/obj/ejsFunction.o
DEPS_136 += $(CONFIG)/obj/ejsGC.o
DEPS_136 += $(CONFIG)/obj/ejsGlobal.o
DEPS_136 += $(CONFIG)/obj/ejsHttp.o
DEPS_136 += $(CONFIG)/obj/ejsIterator.o
DEPS_136 += $(CONFIG)/obj/ejsJSON.o
DEPS_136 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_136 += $(CONFIG)/obj/ejsMath.o
DEPS_136 += $(CONFIG)/obj/ejsMemory.o
DEPS_136 += $(CONFIG)/obj/ejsMprLog.o
DEPS_136 += $(CONFIG)/obj/ejsNamespace.o
DEPS_136 += $(CONFIG)/obj/ejsNull.o
DEPS_136 += $(CONFIG)/obj/ejsNumber.o
DEPS_136 += $(CONFIG)/obj/ejsObject.o
DEPS_136 += $(CONFIG)/obj/ejsPath.o
DEPS_136 += $(CONFIG)/obj/ejsPot.o
DEPS_136 += $(CONFIG)/obj/ejsRegExp.o
DEPS_136 += $(CONFIG)/obj/ejsSocket.o
DEPS_136 += $(CONFIG)/obj/ejsString.o
DEPS_136 += $(CONFIG)/obj/ejsSystem.o
DEPS_136 += $(CONFIG)/obj/ejsTimer.o
DEPS_136 += $(CONFIG)/obj/ejsType.o
DEPS_136 += $(CONFIG)/obj/ejsUri.o
DEPS_136 += $(CONFIG)/obj/ejsVoid.o
DEPS_136 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_136 += $(CONFIG)/obj/ejsWorker.o
DEPS_136 += $(CONFIG)/obj/ejsXML.o
DEPS_136 += $(CONFIG)/obj/ejsXMLList.o
DEPS_136 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_136 += $(CONFIG)/obj/ejsByteCode.o
DEPS_136 += $(CONFIG)/obj/ejsException.o
DEPS_136 += $(CONFIG)/obj/ejsHelper.o
DEPS_136 += $(CONFIG)/obj/ejsInterp.o
DEPS_136 += $(CONFIG)/obj/ejsLoader.o
DEPS_136 += $(CONFIG)/obj/ejsModule.o
DEPS_136 += $(CONFIG)/obj/ejsScope.o
DEPS_136 += $(CONFIG)/obj/ejsService.o
DEPS_136 += $(CONFIG)/bin/libejs.a
DEPS_136 += $(CONFIG)/bin/mvc.es
DEPS_136 += $(CONFIG)/obj/ejsrun.o

LIBS_136 += -lejs
LIBS_136 += -lhttp
LIBS_136 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_136 += -lpcre
endif

$(CONFIG)/bin/mvc: $(DEPS_136)
	@echo '      [Link] $(CONFIG)/bin/mvc'
	$(CC) -o $(CONFIG)/bin/mvc $(LIBPATHS) "$(CONFIG)/obj/ejsrun.o" $(LIBPATHS_136) $(LIBS_136) $(LIBS_136) $(LIBS) $(LIBS) 

#
#   ejs.mvc.mod
#
DEPS_137 += src/ejs.mvc/mvc.es
DEPS_137 += $(CONFIG)/inc/mpr.h
DEPS_137 += $(CONFIG)/inc/me.h
DEPS_137 += $(CONFIG)/inc/osdep.h
DEPS_137 += $(CONFIG)/obj/mprLib.o
DEPS_137 += $(CONFIG)/bin/libmpr.a
DEPS_137 += $(CONFIG)/inc/pcre.h
DEPS_137 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_137 += $(CONFIG)/bin/libpcre.a
endif
DEPS_137 += $(CONFIG)/inc/http.h
DEPS_137 += $(CONFIG)/obj/httpLib.o
DEPS_137 += $(CONFIG)/bin/libhttp.a
DEPS_137 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_137 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_137 += $(CONFIG)/inc/ejs.slots.h
DEPS_137 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_137 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_137 += $(CONFIG)/inc/ejsByteCode.h
DEPS_137 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_137 += $(CONFIG)/inc/ejsCustomize.h
DEPS_137 += $(CONFIG)/inc/ejs.h
DEPS_137 += $(CONFIG)/inc/ejsCompiler.h
DEPS_137 += $(CONFIG)/obj/ecAst.o
DEPS_137 += $(CONFIG)/obj/ecCodeGen.o
DEPS_137 += $(CONFIG)/obj/ecCompiler.o
DEPS_137 += $(CONFIG)/obj/ecLex.o
DEPS_137 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_137 += $(CONFIG)/obj/ecParser.o
DEPS_137 += $(CONFIG)/obj/ecState.o
DEPS_137 += $(CONFIG)/obj/dtoa.o
DEPS_137 += $(CONFIG)/obj/ejsApp.o
DEPS_137 += $(CONFIG)/obj/ejsArray.o
DEPS_137 += $(CONFIG)/obj/ejsBlock.o
DEPS_137 += $(CONFIG)/obj/ejsBoolean.o
DEPS_137 += $(CONFIG)/obj/ejsByteArray.o
DEPS_137 += $(CONFIG)/obj/ejsCache.o
DEPS_137 += $(CONFIG)/obj/ejsCmd.o
DEPS_137 += $(CONFIG)/obj/ejsConfig.o
DEPS_137 += $(CONFIG)/obj/ejsDate.o
DEPS_137 += $(CONFIG)/obj/ejsDebug.o
DEPS_137 += $(CONFIG)/obj/ejsError.o
DEPS_137 += $(CONFIG)/obj/ejsFile.o
DEPS_137 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_137 += $(CONFIG)/obj/ejsFrame.o
DEPS_137 += $(CONFIG)/obj/ejsFunction.o
DEPS_137 += $(CONFIG)/obj/ejsGC.o
DEPS_137 += $(CONFIG)/obj/ejsGlobal.o
DEPS_137 += $(CONFIG)/obj/ejsHttp.o
DEPS_137 += $(CONFIG)/obj/ejsIterator.o
DEPS_137 += $(CONFIG)/obj/ejsJSON.o
DEPS_137 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_137 += $(CONFIG)/obj/ejsMath.o
DEPS_137 += $(CONFIG)/obj/ejsMemory.o
DEPS_137 += $(CONFIG)/obj/ejsMprLog.o
DEPS_137 += $(CONFIG)/obj/ejsNamespace.o
DEPS_137 += $(CONFIG)/obj/ejsNull.o
DEPS_137 += $(CONFIG)/obj/ejsNumber.o
DEPS_137 += $(CONFIG)/obj/ejsObject.o
DEPS_137 += $(CONFIG)/obj/ejsPath.o
DEPS_137 += $(CONFIG)/obj/ejsPot.o
DEPS_137 += $(CONFIG)/obj/ejsRegExp.o
DEPS_137 += $(CONFIG)/obj/ejsSocket.o
DEPS_137 += $(CONFIG)/obj/ejsString.o
DEPS_137 += $(CONFIG)/obj/ejsSystem.o
DEPS_137 += $(CONFIG)/obj/ejsTimer.o
DEPS_137 += $(CONFIG)/obj/ejsType.o
DEPS_137 += $(CONFIG)/obj/ejsUri.o
DEPS_137 += $(CONFIG)/obj/ejsVoid.o
DEPS_137 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_137 += $(CONFIG)/obj/ejsWorker.o
DEPS_137 += $(CONFIG)/obj/ejsXML.o
DEPS_137 += $(CONFIG)/obj/ejsXMLList.o
DEPS_137 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_137 += $(CONFIG)/obj/ejsByteCode.o
DEPS_137 += $(CONFIG)/obj/ejsException.o
DEPS_137 += $(CONFIG)/obj/ejsHelper.o
DEPS_137 += $(CONFIG)/obj/ejsInterp.o
DEPS_137 += $(CONFIG)/obj/ejsLoader.o
DEPS_137 += $(CONFIG)/obj/ejsModule.o
DEPS_137 += $(CONFIG)/obj/ejsScope.o
DEPS_137 += $(CONFIG)/obj/ejsService.o
DEPS_137 += $(CONFIG)/bin/libejs.a
DEPS_137 += $(CONFIG)/obj/ejsc.o
DEPS_137 += $(CONFIG)/bin/ejsc
DEPS_137 += src/cmd/ejsmod.h
DEPS_137 += $(CONFIG)/obj/ejsmod.o
DEPS_137 += $(CONFIG)/obj/doc.o
DEPS_137 += $(CONFIG)/obj/docFiles.o
DEPS_137 += $(CONFIG)/obj/listing.o
DEPS_137 += $(CONFIG)/obj/slotGen.o
DEPS_137 += $(CONFIG)/bin/ejsmod
DEPS_137 += $(CONFIG)/bin/ejs.mod
DEPS_137 += $(CONFIG)/bin/ejs.web.mod
DEPS_137 += $(CONFIG)/bin/ejs.template.mod
DEPS_137 += $(CONFIG)/bin/ejs.unix.mod

$(CONFIG)/bin/ejs.mvc.mod: $(DEPS_137)
	( \
	cd src/ejs.mvc; \
	../../$(CONFIG)/bin/ejsc --out ../../$(CONFIG)/bin/ejs.mvc.mod  --optimize 9 *.es ; \
	)

#
#   utest.es
#
DEPS_138 += src/ejs.utest/utest.es

$(CONFIG)/bin/utest.es: $(DEPS_138)
	( \
	cd src/ejs.utest; \
	cp utest.es ../../$(CONFIG)/bin ; \
	)

#
#   utest.worker
#
DEPS_139 += src/ejs.utest/utest.worker

$(CONFIG)/bin/utest.worker: $(DEPS_139)
	( \
	cd src/ejs.utest; \
	cp utest.worker ../../$(CONFIG)/bin ; \
	)

#
#   utest
#
DEPS_140 += $(CONFIG)/inc/mpr.h
DEPS_140 += $(CONFIG)/inc/me.h
DEPS_140 += $(CONFIG)/inc/osdep.h
DEPS_140 += $(CONFIG)/obj/mprLib.o
DEPS_140 += $(CONFIG)/bin/libmpr.a
DEPS_140 += $(CONFIG)/inc/pcre.h
DEPS_140 += $(CONFIG)/obj/pcre.o
ifeq ($(ME_EXT_PCRE),1)
    DEPS_140 += $(CONFIG)/bin/libpcre.a
endif
DEPS_140 += $(CONFIG)/inc/http.h
DEPS_140 += $(CONFIG)/obj/httpLib.o
DEPS_140 += $(CONFIG)/bin/libhttp.a
DEPS_140 += $(CONFIG)/inc/ejs.cache.local.slots.h
DEPS_140 += $(CONFIG)/inc/ejs.db.sqlite.slots.h
DEPS_140 += $(CONFIG)/inc/ejs.slots.h
DEPS_140 += $(CONFIG)/inc/ejs.web.slots.h
DEPS_140 += $(CONFIG)/inc/ejs.zlib.slots.h
DEPS_140 += $(CONFIG)/inc/ejsByteCode.h
DEPS_140 += $(CONFIG)/inc/ejsByteCodeTable.h
DEPS_140 += $(CONFIG)/inc/ejsCustomize.h
DEPS_140 += $(CONFIG)/inc/ejs.h
DEPS_140 += $(CONFIG)/inc/ejsCompiler.h
DEPS_140 += $(CONFIG)/obj/ecAst.o
DEPS_140 += $(CONFIG)/obj/ecCodeGen.o
DEPS_140 += $(CONFIG)/obj/ecCompiler.o
DEPS_140 += $(CONFIG)/obj/ecLex.o
DEPS_140 += $(CONFIG)/obj/ecModuleWrite.o
DEPS_140 += $(CONFIG)/obj/ecParser.o
DEPS_140 += $(CONFIG)/obj/ecState.o
DEPS_140 += $(CONFIG)/obj/dtoa.o
DEPS_140 += $(CONFIG)/obj/ejsApp.o
DEPS_140 += $(CONFIG)/obj/ejsArray.o
DEPS_140 += $(CONFIG)/obj/ejsBlock.o
DEPS_140 += $(CONFIG)/obj/ejsBoolean.o
DEPS_140 += $(CONFIG)/obj/ejsByteArray.o
DEPS_140 += $(CONFIG)/obj/ejsCache.o
DEPS_140 += $(CONFIG)/obj/ejsCmd.o
DEPS_140 += $(CONFIG)/obj/ejsConfig.o
DEPS_140 += $(CONFIG)/obj/ejsDate.o
DEPS_140 += $(CONFIG)/obj/ejsDebug.o
DEPS_140 += $(CONFIG)/obj/ejsError.o
DEPS_140 += $(CONFIG)/obj/ejsFile.o
DEPS_140 += $(CONFIG)/obj/ejsFileSystem.o
DEPS_140 += $(CONFIG)/obj/ejsFrame.o
DEPS_140 += $(CONFIG)/obj/ejsFunction.o
DEPS_140 += $(CONFIG)/obj/ejsGC.o
DEPS_140 += $(CONFIG)/obj/ejsGlobal.o
DEPS_140 += $(CONFIG)/obj/ejsHttp.o
DEPS_140 += $(CONFIG)/obj/ejsIterator.o
DEPS_140 += $(CONFIG)/obj/ejsJSON.o
DEPS_140 += $(CONFIG)/obj/ejsLocalCache.o
DEPS_140 += $(CONFIG)/obj/ejsMath.o
DEPS_140 += $(CONFIG)/obj/ejsMemory.o
DEPS_140 += $(CONFIG)/obj/ejsMprLog.o
DEPS_140 += $(CONFIG)/obj/ejsNamespace.o
DEPS_140 += $(CONFIG)/obj/ejsNull.o
DEPS_140 += $(CONFIG)/obj/ejsNumber.o
DEPS_140 += $(CONFIG)/obj/ejsObject.o
DEPS_140 += $(CONFIG)/obj/ejsPath.o
DEPS_140 += $(CONFIG)/obj/ejsPot.o
DEPS_140 += $(CONFIG)/obj/ejsRegExp.o
DEPS_140 += $(CONFIG)/obj/ejsSocket.o
DEPS_140 += $(CONFIG)/obj/ejsString.o
DEPS_140 += $(CONFIG)/obj/ejsSystem.o
DEPS_140 += $(CONFIG)/obj/ejsTimer.o
DEPS_140 += $(CONFIG)/obj/ejsType.o
DEPS_140 += $(CONFIG)/obj/ejsUri.o
DEPS_140 += $(CONFIG)/obj/ejsVoid.o
DEPS_140 += $(CONFIG)/obj/ejsWebSocket.o
DEPS_140 += $(CONFIG)/obj/ejsWorker.o
DEPS_140 += $(CONFIG)/obj/ejsXML.o
DEPS_140 += $(CONFIG)/obj/ejsXMLList.o
DEPS_140 += $(CONFIG)/obj/ejsXMLLoader.o
DEPS_140 += $(CONFIG)/obj/ejsByteCode.o
DEPS_140 += $(CONFIG)/obj/ejsException.o
DEPS_140 += $(CONFIG)/obj/ejsHelper.o
DEPS_140 += $(CONFIG)/obj/ejsInterp.o
DEPS_140 += $(CONFIG)/obj/ejsLoader.o
DEPS_140 += $(CONFIG)/obj/ejsModule.o
DEPS_140 += $(CONFIG)/obj/ejsScope.o
DEPS_140 += $(CONFIG)/obj/ejsService.o
DEPS_140 += $(CONFIG)/bin/libejs.a
DEPS_140 += $(CONFIG)/bin/utest.es
DEPS_140 += $(CONFIG)/bin/utest.worker
DEPS_140 += $(CONFIG)/obj/ejsrun.o

LIBS_140 += -lejs
LIBS_140 += -lhttp
LIBS_140 += -lmpr
ifeq ($(ME_EXT_PCRE),1)
    LIBS_140 += -lpcre
endif

$(CONFIG)/bin/utest: $(DEPS_140)
	@echo '      [Link] $(CONFIG)/bin/utest'
	$(CC) -o $(CONFIG)/bin/utest $(LIBPATHS) "$(CONFIG)/obj/ejsrun.o" $(LIBPATHS_140) $(LIBS_140) $(LIBS_140) $(LIBS) $(LIBS) 

#
#   stop
#
stop: $(DEPS_141)

#
#   installBinary
#
installBinary: $(DEPS_142)
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
	if [ "$(ME_EXT_SSL)" = 1 ]; then true ; \
	cp $(CONFIG)/bin/ca.crt $(ME_VAPP_PREFIX)/bin/ca.crt ; \
	fi ; \
	if [ "$(ME_EXT_OPENSSL)" = 1 ]; then true ; \
	cp $(CONFIG)/bin/libssl*.so* $(ME_VAPP_PREFIX)/bin/libssl*.so* ; \
	cp $(CONFIG)/bin/libcrypto*.so* $(ME_VAPP_PREFIX)/bin/libcrypto*.so* ; \
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
start: $(DEPS_143)

#
#   install
#
DEPS_144 += stop
DEPS_144 += installBinary
DEPS_144 += start

install: $(DEPS_144)

#
#   uninstall
#
DEPS_145 += stop

uninstall: $(DEPS_145)

