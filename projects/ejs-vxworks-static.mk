#
#   ejs-vxworks-static.mk -- Makefile to build Embedthis Ejscript for vxworks
#

NAME                  := ejs
VERSION               := 2.5.2
PROFILE               ?= static
ARCH                  ?= $(shell echo $(WIND_HOST_TYPE) | sed 's/-.*$(ME_ROOT_PREFIX)/')
CPU                   ?= $(subst X86,PENTIUM,$(shell echo $(ARCH) | tr a-z A-Z))
OS                    ?= vxworks
CC                    ?= cc$(subst x86,pentium,$(ARCH))
LD                    ?= ld
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
BUILD                 ?= build/$(CONFIG)
LBIN                  ?= $(BUILD)/bin
PATH                  := $(LBIN):$(PATH)

ME_COM_COMPILER       ?= 1
ME_COM_EST            ?= 0
ME_COM_HTTP           ?= 1
ME_COM_LIB            ?= 1
ME_COM_LINK           ?= 1
ME_COM_MPR            ?= 1
ME_COM_OPENSSL        ?= 1
ME_COM_OSDEP          ?= 1
ME_COM_PCRE           ?= 1
ME_COM_SQLITE         ?= 1
ME_COM_SSL            ?= 1
ME_COM_VXWORKS        ?= 1
ME_COM_WINSDK         ?= 1
ME_COM_ZLIB           ?= 1

ME_COM_OPENSSL_PATH   ?= "/usr"

ifeq ($(ME_COM_EST),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_LIB),1)
    ME_COM_COMPILER := 1
endif
ifeq ($(ME_COM_LINK),1)
    ME_COM_COMPILER := 1
endif
ifeq ($(ME_COM_OPENSSL),1)
    ME_COM_SSL := 1
endif

export WIND_HOME      ?= $(WIND_BASE)/..
export PATH           := $(WIND_GNU_PATH)/$(WIND_HOST_TYPE)/bin:$(PATH)

CFLAGS                += -fno-builtin -fno-defer-pop -fvolatile -w
DFLAGS                += -DVXWORKS -DRW_MULTI_THREAD -D_GNU_TOOL -DCPU=PENTIUM $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_COMPILER=$(ME_COM_COMPILER) -DME_COM_EST=$(ME_COM_EST) -DME_COM_HTTP=$(ME_COM_HTTP) -DME_COM_LIB=$(ME_COM_LIB) -DME_COM_LINK=$(ME_COM_LINK) -DME_COM_MPR=$(ME_COM_MPR) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_OSDEP=$(ME_COM_OSDEP) -DME_COM_PCRE=$(ME_COM_PCRE) -DME_COM_SQLITE=$(ME_COM_SQLITE) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) -DME_COM_WINSDK=$(ME_COM_WINSDK) -DME_COM_ZLIB=$(ME_COM_ZLIB) 
IFLAGS                += "-I$(BUILD)/inc -I$(WIND_BASE)/target/h -I$(WIND_BASE)/target/h/wrn/coreip"
LDFLAGS               += '-Wl,-r'
LIBPATHS              += -L$(BUILD)/bin
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


TARGETS               += $(BUILD)/bin/ejs.out
TARGETS               += $(BUILD)/bin/ejs.db.mapper.mod
TARGETS               += $(BUILD)/bin/ejs.db.sqlite.mod
TARGETS               += $(BUILD)/bin/ejs.mail.mod
TARGETS               += $(BUILD)/bin/ejs.mvc.mod
TARGETS               += $(BUILD)/bin/ejs.tar.mod
TARGETS               += $(BUILD)/bin/ejsrun.out
TARGETS               += $(BUILD)/bin/ca.crt
TARGETS               += $(BUILD)/bin/libejs.db.sqlite.out
TARGETS               += $(BUILD)/bin/libejs.web.out
TARGETS               += $(BUILD)/bin/libmprssl.out
TARGETS               += $(BUILD)/bin/ejsman.out
TARGETS               += $(BUILD)/bin/mvc.out
TARGETS               += $(BUILD)/bin/utest.out
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
	@if [ "$(WIND_BASE)" = "" ] ; then echo WARNING: WIND_BASE not set. Run wrenv.sh. ; exit 255 ; fi
	@if [ "$(WIND_HOST_TYPE)" = "" ] ; then echo WARNING: WIND_HOST_TYPE not set. Run wrenv.sh. ; exit 255 ; fi
	@if [ "$(WIND_GNU_PATH)" = "" ] ; then echo WARNING: WIND_GNU_PATH not set. Run wrenv.sh. ; exit 255 ; fi
	@[ ! -x $(BUILD)/bin ] && mkdir -p $(BUILD)/bin; true
	@[ ! -x $(BUILD)/inc ] && mkdir -p $(BUILD)/inc; true
	@[ ! -x $(BUILD)/obj ] && mkdir -p $(BUILD)/obj; true
	@[ ! -f $(BUILD)/inc/me.h ] && cp projects/ejs-vxworks-static-me.h $(BUILD)/inc/me.h ; true
	@if ! diff $(BUILD)/inc/me.h projects/ejs-vxworks-static-me.h >/dev/null ; then\
		cp projects/ejs-vxworks-static-me.h $(BUILD)/inc/me.h  ; \
	fi; true
	@if [ -f "$(BUILD)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != "`cat $(BUILD)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build: "`cat $(BUILD)/.makeflags`"" ; \
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
	rm -f "$(BUILD)/obj/ejsJson.o"
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
	rm -f "$(BUILD)/obj/httpLib.o"
	rm -f "$(BUILD)/obj/listing.o"
	rm -f "$(BUILD)/obj/makerom.o"
	rm -f "$(BUILD)/obj/manager.o"
	rm -f "$(BUILD)/obj/mprLib.o"
	rm -f "$(BUILD)/obj/mprSsl.o"
	rm -f "$(BUILD)/obj/pcre.o"
	rm -f "$(BUILD)/obj/slotGen.o"
	rm -f "$(BUILD)/obj/sqlite.o"
	rm -f "$(BUILD)/obj/sqlite3.o"
	rm -f "$(BUILD)/obj/zlib.o"
	rm -f "$(BUILD)/bin/ejs.out"
	rm -f "$(BUILD)/bin/ejsc.out"
	rm -f "$(BUILD)/bin/ejsmod.out"
	rm -f "$(BUILD)/bin/ejsrun.out"
	rm -f "$(BUILD)/bin/ca.crt"
	rm -f "$(BUILD)/bin/libejs.out"
	rm -f "$(BUILD)/bin/libejs.db.sqlite.out"
	rm -f "$(BUILD)/bin/libejs.web.out"
	rm -f "$(BUILD)/bin/libejs.zlib.out"
	rm -f "$(BUILD)/bin/libhttp.out"
	rm -f "$(BUILD)/bin/libmpr.out"
	rm -f "$(BUILD)/bin/libmprssl.out"
	rm -f "$(BUILD)/bin/libpcre.out"
	rm -f "$(BUILD)/bin/libsql.out"
	rm -f "$(BUILD)/bin/libzlib.out"
	rm -f "$(BUILD)/bin/ejsman.out"
	rm -f "$(BUILD)/bin/mvc.es"
	rm -f "$(BUILD)/bin/utest.out"
	rm -f "$(BUILD)/bin/utest.es"
	rm -f "$(BUILD)/bin/utest.worker"
	rm -f "$(BUILD)/bin/www"

clobber: clean
	rm -fr ./$(BUILD)

#
#   ejs.cache.local.slots.h
#
DEPS_143 += src/slots/ejs.cache.local.slots.h

$(BUILD)/inc/ejs.cache.local.slots.h: $(DEPS_143)
	@echo '      [Copy] $(BUILD)/inc/ejs.cache.local.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/slots/ejs.cache.local.slots.h $(BUILD)/inc/ejs.cache.local.slots.h

#
#   ejs.db.sqlite.slots.h
#
DEPS_144 += src/slots/ejs.db.sqlite.slots.h

$(BUILD)/inc/ejs.db.sqlite.slots.h: $(DEPS_144)
	@echo '      [Copy] $(BUILD)/inc/ejs.db.sqlite.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/slots/ejs.db.sqlite.slots.h $(BUILD)/inc/ejs.db.sqlite.slots.h

#
#   me.h
#

$(BUILD)/inc/me.h: $(DEPS_145)

#
#   osdep.h
#
DEPS_146 += src/osdep/osdep.h
DEPS_146 += $(BUILD)/inc/me.h

$(BUILD)/inc/osdep.h: $(DEPS_146)
	@echo '      [Copy] $(BUILD)/inc/osdep.h'
	mkdir -p "$(BUILD)/inc"
	cp src/osdep/osdep.h $(BUILD)/inc/osdep.h

#
#   mpr.h
#
DEPS_147 += src/mpr/mpr.h
DEPS_147 += $(BUILD)/inc/me.h
DEPS_147 += $(BUILD)/inc/osdep.h

$(BUILD)/inc/mpr.h: $(DEPS_147)
	@echo '      [Copy] $(BUILD)/inc/mpr.h'
	mkdir -p "$(BUILD)/inc"
	cp src/mpr/mpr.h $(BUILD)/inc/mpr.h

#
#   http.h
#
DEPS_148 += src/http/http.h
DEPS_148 += $(BUILD)/inc/mpr.h

$(BUILD)/inc/http.h: $(DEPS_148)
	@echo '      [Copy] $(BUILD)/inc/http.h'
	mkdir -p "$(BUILD)/inc"
	cp src/http/http.h $(BUILD)/inc/http.h

#
#   ejsByteCode.h
#

src/ejsByteCode.h: $(DEPS_149)

#
#   ejsByteCodeTable.h
#

src/ejsByteCodeTable.h: $(DEPS_150)

#
#   ejs.slots.h
#
DEPS_151 += src/slots/ejs.slots.h

$(BUILD)/inc/ejs.slots.h: $(DEPS_151)
	@echo '      [Copy] $(BUILD)/inc/ejs.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/slots/ejs.slots.h $(BUILD)/inc/ejs.slots.h

#
#   ejsCustomize.h
#

src/ejsCustomize.h: $(DEPS_152)

#
#   ejs.h
#
DEPS_153 += src/ejs.h
DEPS_153 += $(BUILD)/inc/mpr.h
DEPS_153 += $(BUILD)/inc/http.h
DEPS_153 += src/ejsByteCode.h
DEPS_153 += src/ejsByteCodeTable.h
DEPS_153 += $(BUILD)/inc/ejs.slots.h
DEPS_153 += src/ejsCustomize.h

$(BUILD)/inc/ejs.h: $(DEPS_153)
	@echo '      [Copy] $(BUILD)/inc/ejs.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejs.h $(BUILD)/inc/ejs.h

#
#   ejs.web.slots.h
#
DEPS_154 += src/slots/ejs.web.slots.h

$(BUILD)/inc/ejs.web.slots.h: $(DEPS_154)
	@echo '      [Copy] $(BUILD)/inc/ejs.web.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/slots/ejs.web.slots.h $(BUILD)/inc/ejs.web.slots.h

#
#   ejs.zlib.slots.h
#
DEPS_155 += src/slots/ejs.zlib.slots.h

$(BUILD)/inc/ejs.zlib.slots.h: $(DEPS_155)
	@echo '      [Copy] $(BUILD)/inc/ejs.zlib.slots.h'
	mkdir -p "$(BUILD)/inc"
	cp src/slots/ejs.zlib.slots.h $(BUILD)/inc/ejs.zlib.slots.h

#
#   ejsByteCode.h
#
DEPS_156 += src/ejsByteCode.h

$(BUILD)/inc/ejsByteCode.h: $(DEPS_156)
	@echo '      [Copy] $(BUILD)/inc/ejsByteCode.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejsByteCode.h $(BUILD)/inc/ejsByteCode.h

#
#   ejsByteCodeTable.h
#
DEPS_157 += src/ejsByteCodeTable.h

$(BUILD)/inc/ejsByteCodeTable.h: $(DEPS_157)
	@echo '      [Copy] $(BUILD)/inc/ejsByteCodeTable.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejsByteCodeTable.h $(BUILD)/inc/ejsByteCodeTable.h

#
#   ejs.h
#

src/ejs.h: $(DEPS_158)

#
#   ejsCompiler.h
#
DEPS_159 += src/ejsCompiler.h
DEPS_159 += src/ejs.h

$(BUILD)/inc/ejsCompiler.h: $(DEPS_159)
	@echo '      [Copy] $(BUILD)/inc/ejsCompiler.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejsCompiler.h $(BUILD)/inc/ejsCompiler.h

#
#   ejsCustomize.h
#
DEPS_160 += src/ejsCustomize.h

$(BUILD)/inc/ejsCustomize.h: $(DEPS_160)
	@echo '      [Copy] $(BUILD)/inc/ejsCustomize.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejsCustomize.h $(BUILD)/inc/ejsCustomize.h

#
#   ejsWeb.h
#
DEPS_161 += src/ejs.web/ejsWeb.h

$(BUILD)/inc/ejsWeb.h: $(DEPS_161)
	@echo '      [Copy] $(BUILD)/inc/ejsWeb.h'
	mkdir -p "$(BUILD)/inc"
	cp src/ejs.web/ejsWeb.h $(BUILD)/inc/ejsWeb.h

#
#   ejsmod.h
#
DEPS_162 += src/cmd/ejsmod.h

$(BUILD)/inc/ejsmod.h: $(DEPS_162)
	@echo '      [Copy] $(BUILD)/inc/ejsmod.h'
	mkdir -p "$(BUILD)/inc"
	cp src/cmd/ejsmod.h $(BUILD)/inc/ejsmod.h

#
#   pcre.h
#
DEPS_163 += src/pcre/pcre.h

$(BUILD)/inc/pcre.h: $(DEPS_163)
	@echo '      [Copy] $(BUILD)/inc/pcre.h'
	mkdir -p "$(BUILD)/inc"
	cp src/pcre/pcre.h $(BUILD)/inc/pcre.h

#
#   sqlite3.h
#
DEPS_164 += src/sqlite/sqlite3.h

$(BUILD)/inc/sqlite3.h: $(DEPS_164)
	@echo '      [Copy] $(BUILD)/inc/sqlite3.h'
	mkdir -p "$(BUILD)/inc"
	cp src/sqlite/sqlite3.h $(BUILD)/inc/sqlite3.h

#
#   zlib.h
#
DEPS_165 += src/zlib/zlib.h
DEPS_165 += $(BUILD)/inc/me.h

$(BUILD)/inc/zlib.h: $(DEPS_165)
	@echo '      [Copy] $(BUILD)/inc/zlib.h'
	mkdir -p "$(BUILD)/inc"
	cp src/zlib/zlib.h $(BUILD)/inc/zlib.h

#
#   ejsmod.h
#

src/cmd/ejsmod.h: $(DEPS_166)

#
#   doc.o
#
DEPS_167 += src/cmd/ejsmod.h

$(BUILD)/obj/doc.o: \
    src/cmd/doc.c $(DEPS_167)
	@echo '   [Compile] $(BUILD)/obj/doc.o'
	$(CC) -c -o $(BUILD)/obj/doc.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/cmd/doc.c

#
#   docFiles.o
#
DEPS_168 += src/cmd/ejsmod.h

$(BUILD)/obj/docFiles.o: \
    src/cmd/docFiles.c $(DEPS_168)
	@echo '   [Compile] $(BUILD)/obj/docFiles.o'
	$(CC) -c -o $(BUILD)/obj/docFiles.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/cmd/docFiles.c

#
#   dtoa.o
#
DEPS_169 += $(BUILD)/inc/mpr.h

$(BUILD)/obj/dtoa.o: \
    src/core/src/dtoa.c $(DEPS_169)
	@echo '   [Compile] $(BUILD)/obj/dtoa.o'
	$(CC) -c -o $(BUILD)/obj/dtoa.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/dtoa.c

#
#   ejsCompiler.h
#

src/ejsCompiler.h: $(DEPS_170)

#
#   ecAst.o
#
DEPS_171 += src/ejsCompiler.h

$(BUILD)/obj/ecAst.o: \
    src/compiler/ecAst.c $(DEPS_171)
	@echo '   [Compile] $(BUILD)/obj/ecAst.o'
	$(CC) -c -o $(BUILD)/obj/ecAst.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecAst.c

#
#   ecCodeGen.o
#
DEPS_172 += src/ejsCompiler.h

$(BUILD)/obj/ecCodeGen.o: \
    src/compiler/ecCodeGen.c $(DEPS_172)
	@echo '   [Compile] $(BUILD)/obj/ecCodeGen.o'
	$(CC) -c -o $(BUILD)/obj/ecCodeGen.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecCodeGen.c

#
#   ecCompiler.o
#
DEPS_173 += src/ejsCompiler.h

$(BUILD)/obj/ecCompiler.o: \
    src/compiler/ecCompiler.c $(DEPS_173)
	@echo '   [Compile] $(BUILD)/obj/ecCompiler.o'
	$(CC) -c -o $(BUILD)/obj/ecCompiler.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecCompiler.c

#
#   ecLex.o
#
DEPS_174 += src/ejsCompiler.h

$(BUILD)/obj/ecLex.o: \
    src/compiler/ecLex.c $(DEPS_174)
	@echo '   [Compile] $(BUILD)/obj/ecLex.o'
	$(CC) -c -o $(BUILD)/obj/ecLex.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecLex.c

#
#   ecModuleWrite.o
#
DEPS_175 += src/ejsCompiler.h

$(BUILD)/obj/ecModuleWrite.o: \
    src/compiler/ecModuleWrite.c $(DEPS_175)
	@echo '   [Compile] $(BUILD)/obj/ecModuleWrite.o'
	$(CC) -c -o $(BUILD)/obj/ecModuleWrite.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecModuleWrite.c

#
#   ecParser.o
#
DEPS_176 += src/ejsCompiler.h

$(BUILD)/obj/ecParser.o: \
    src/compiler/ecParser.c $(DEPS_176)
	@echo '   [Compile] $(BUILD)/obj/ecParser.o'
	$(CC) -c -o $(BUILD)/obj/ecParser.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecParser.c

#
#   ecState.o
#
DEPS_177 += src/ejsCompiler.h

$(BUILD)/obj/ecState.o: \
    src/compiler/ecState.c $(DEPS_177)
	@echo '   [Compile] $(BUILD)/obj/ecState.o'
	$(CC) -c -o $(BUILD)/obj/ecState.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/compiler/ecState.c

#
#   ejs.o
#
DEPS_178 += $(BUILD)/inc/me.h
DEPS_178 += $(BUILD)/inc/ejsCompiler.h

$(BUILD)/obj/ejs.o: \
    src/cmd/ejs.c $(DEPS_178)
	@echo '   [Compile] $(BUILD)/obj/ejs.o'
	$(CC) -c -o $(BUILD)/obj/ejs.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/cmd/ejs.c

#
#   ejsApp.o
#
DEPS_179 += src/ejs.h

$(BUILD)/obj/ejsApp.o: \
    src/core/src/ejsApp.c $(DEPS_179)
	@echo '   [Compile] $(BUILD)/obj/ejsApp.o'
	$(CC) -c -o $(BUILD)/obj/ejsApp.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsApp.c

#
#   ejsArray.o
#
DEPS_180 += src/ejs.h

$(BUILD)/obj/ejsArray.o: \
    src/core/src/ejsArray.c $(DEPS_180)
	@echo '   [Compile] $(BUILD)/obj/ejsArray.o'
	$(CC) -c -o $(BUILD)/obj/ejsArray.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsArray.c

#
#   ejsBlock.o
#
DEPS_181 += src/ejs.h

$(BUILD)/obj/ejsBlock.o: \
    src/core/src/ejsBlock.c $(DEPS_181)
	@echo '   [Compile] $(BUILD)/obj/ejsBlock.o'
	$(CC) -c -o $(BUILD)/obj/ejsBlock.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsBlock.c

#
#   ejsBoolean.o
#
DEPS_182 += src/ejs.h

$(BUILD)/obj/ejsBoolean.o: \
    src/core/src/ejsBoolean.c $(DEPS_182)
	@echo '   [Compile] $(BUILD)/obj/ejsBoolean.o'
	$(CC) -c -o $(BUILD)/obj/ejsBoolean.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsBoolean.c

#
#   ejsByteArray.o
#
DEPS_183 += src/ejs.h

$(BUILD)/obj/ejsByteArray.o: \
    src/core/src/ejsByteArray.c $(DEPS_183)
	@echo '   [Compile] $(BUILD)/obj/ejsByteArray.o'
	$(CC) -c -o $(BUILD)/obj/ejsByteArray.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsByteArray.c

#
#   ejsByteCode.o
#
DEPS_184 += src/ejs.h

$(BUILD)/obj/ejsByteCode.o: \
    src/vm/ejsByteCode.c $(DEPS_184)
	@echo '   [Compile] $(BUILD)/obj/ejsByteCode.o'
	$(CC) -c -o $(BUILD)/obj/ejsByteCode.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsByteCode.c

#
#   ejsCache.o
#
DEPS_185 += src/ejs.h

$(BUILD)/obj/ejsCache.o: \
    src/core/src/ejsCache.c $(DEPS_185)
	@echo '   [Compile] $(BUILD)/obj/ejsCache.o'
	$(CC) -c -o $(BUILD)/obj/ejsCache.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsCache.c

#
#   ejsCmd.o
#
DEPS_186 += src/ejs.h

$(BUILD)/obj/ejsCmd.o: \
    src/core/src/ejsCmd.c $(DEPS_186)
	@echo '   [Compile] $(BUILD)/obj/ejsCmd.o'
	$(CC) -c -o $(BUILD)/obj/ejsCmd.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsCmd.c

#
#   ejsConfig.o
#
DEPS_187 += src/ejs.h

$(BUILD)/obj/ejsConfig.o: \
    src/core/src/ejsConfig.c $(DEPS_187)
	@echo '   [Compile] $(BUILD)/obj/ejsConfig.o'
	$(CC) -c -o $(BUILD)/obj/ejsConfig.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsConfig.c

#
#   ejsDate.o
#
DEPS_188 += src/ejs.h

$(BUILD)/obj/ejsDate.o: \
    src/core/src/ejsDate.c $(DEPS_188)
	@echo '   [Compile] $(BUILD)/obj/ejsDate.o'
	$(CC) -c -o $(BUILD)/obj/ejsDate.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsDate.c

#
#   ejsDebug.o
#
DEPS_189 += src/ejs.h

$(BUILD)/obj/ejsDebug.o: \
    src/core/src/ejsDebug.c $(DEPS_189)
	@echo '   [Compile] $(BUILD)/obj/ejsDebug.o'
	$(CC) -c -o $(BUILD)/obj/ejsDebug.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsDebug.c

#
#   ejsError.o
#
DEPS_190 += src/ejs.h

$(BUILD)/obj/ejsError.o: \
    src/core/src/ejsError.c $(DEPS_190)
	@echo '   [Compile] $(BUILD)/obj/ejsError.o'
	$(CC) -c -o $(BUILD)/obj/ejsError.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsError.c

#
#   ejsException.o
#
DEPS_191 += src/ejs.h

$(BUILD)/obj/ejsException.o: \
    src/vm/ejsException.c $(DEPS_191)
	@echo '   [Compile] $(BUILD)/obj/ejsException.o'
	$(CC) -c -o $(BUILD)/obj/ejsException.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsException.c

#
#   ejsFile.o
#
DEPS_192 += src/ejs.h

$(BUILD)/obj/ejsFile.o: \
    src/core/src/ejsFile.c $(DEPS_192)
	@echo '   [Compile] $(BUILD)/obj/ejsFile.o'
	$(CC) -c -o $(BUILD)/obj/ejsFile.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsFile.c

#
#   ejsFileSystem.o
#
DEPS_193 += src/ejs.h

$(BUILD)/obj/ejsFileSystem.o: \
    src/core/src/ejsFileSystem.c $(DEPS_193)
	@echo '   [Compile] $(BUILD)/obj/ejsFileSystem.o'
	$(CC) -c -o $(BUILD)/obj/ejsFileSystem.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsFileSystem.c

#
#   ejsFrame.o
#
DEPS_194 += src/ejs.h

$(BUILD)/obj/ejsFrame.o: \
    src/core/src/ejsFrame.c $(DEPS_194)
	@echo '   [Compile] $(BUILD)/obj/ejsFrame.o'
	$(CC) -c -o $(BUILD)/obj/ejsFrame.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsFrame.c

#
#   ejsFunction.o
#
DEPS_195 += src/ejs.h

$(BUILD)/obj/ejsFunction.o: \
    src/core/src/ejsFunction.c $(DEPS_195)
	@echo '   [Compile] $(BUILD)/obj/ejsFunction.o'
	$(CC) -c -o $(BUILD)/obj/ejsFunction.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsFunction.c

#
#   ejsGC.o
#
DEPS_196 += src/ejs.h

$(BUILD)/obj/ejsGC.o: \
    src/core/src/ejsGC.c $(DEPS_196)
	@echo '   [Compile] $(BUILD)/obj/ejsGC.o'
	$(CC) -c -o $(BUILD)/obj/ejsGC.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsGC.c

#
#   ejsGlobal.o
#
DEPS_197 += src/ejs.h

$(BUILD)/obj/ejsGlobal.o: \
    src/core/src/ejsGlobal.c $(DEPS_197)
	@echo '   [Compile] $(BUILD)/obj/ejsGlobal.o'
	$(CC) -c -o $(BUILD)/obj/ejsGlobal.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsGlobal.c

#
#   ejsHelper.o
#
DEPS_198 += src/ejs.h

$(BUILD)/obj/ejsHelper.o: \
    src/vm/ejsHelper.c $(DEPS_198)
	@echo '   [Compile] $(BUILD)/obj/ejsHelper.o'
	$(CC) -c -o $(BUILD)/obj/ejsHelper.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsHelper.c

#
#   ejsHttp.o
#
DEPS_199 += src/ejs.h

$(BUILD)/obj/ejsHttp.o: \
    src/core/src/ejsHttp.c $(DEPS_199)
	@echo '   [Compile] $(BUILD)/obj/ejsHttp.o'
	$(CC) -c -o $(BUILD)/obj/ejsHttp.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsHttp.c

#
#   ejsWeb.h
#

src/ejs.web/ejsWeb.h: $(DEPS_200)

#
#   ejsHttpServer.o
#
DEPS_201 += $(BUILD)/inc/me.h
DEPS_201 += $(BUILD)/inc/ejs.h
DEPS_201 += $(BUILD)/inc/ejsCompiler.h
DEPS_201 += src/ejs.web/ejsWeb.h
DEPS_201 += $(BUILD)/inc/ejs.web.slots.h

$(BUILD)/obj/ejsHttpServer.o: \
    src/ejs.web/ejsHttpServer.c $(DEPS_201)
	@echo '   [Compile] $(BUILD)/obj/ejsHttpServer.o'
	$(CC) -c -o $(BUILD)/obj/ejsHttpServer.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/ejs.web/ejsHttpServer.c

#
#   ejsInterp.o
#
DEPS_202 += src/ejs.h

$(BUILD)/obj/ejsInterp.o: \
    src/vm/ejsInterp.c $(DEPS_202)
	@echo '   [Compile] $(BUILD)/obj/ejsInterp.o'
	$(CC) -c -o $(BUILD)/obj/ejsInterp.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsInterp.c

#
#   ejsIterator.o
#
DEPS_203 += src/ejs.h

$(BUILD)/obj/ejsIterator.o: \
    src/core/src/ejsIterator.c $(DEPS_203)
	@echo '   [Compile] $(BUILD)/obj/ejsIterator.o'
	$(CC) -c -o $(BUILD)/obj/ejsIterator.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsIterator.c

#
#   ejsJson.o
#
DEPS_204 += src/ejs.h

$(BUILD)/obj/ejsJson.o: \
    src/core/src/ejsJson.c $(DEPS_204)
	@echo '   [Compile] $(BUILD)/obj/ejsJson.o'
	$(CC) -c -o $(BUILD)/obj/ejsJson.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsJson.c

#
#   ejsLoader.o
#
DEPS_205 += src/ejs.h

$(BUILD)/obj/ejsLoader.o: \
    src/vm/ejsLoader.c $(DEPS_205)
	@echo '   [Compile] $(BUILD)/obj/ejsLoader.o'
	$(CC) -c -o $(BUILD)/obj/ejsLoader.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsLoader.c

#
#   ejsLocalCache.o
#
DEPS_206 += src/ejs.h

$(BUILD)/obj/ejsLocalCache.o: \
    src/core/src/ejsLocalCache.c $(DEPS_206)
	@echo '   [Compile] $(BUILD)/obj/ejsLocalCache.o'
	$(CC) -c -o $(BUILD)/obj/ejsLocalCache.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsLocalCache.c

#
#   ejsMath.o
#
DEPS_207 += src/ejs.h

$(BUILD)/obj/ejsMath.o: \
    src/core/src/ejsMath.c $(DEPS_207)
	@echo '   [Compile] $(BUILD)/obj/ejsMath.o'
	$(CC) -c -o $(BUILD)/obj/ejsMath.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsMath.c

#
#   ejsMemory.o
#
DEPS_208 += src/ejs.h

$(BUILD)/obj/ejsMemory.o: \
    src/core/src/ejsMemory.c $(DEPS_208)
	@echo '   [Compile] $(BUILD)/obj/ejsMemory.o'
	$(CC) -c -o $(BUILD)/obj/ejsMemory.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsMemory.c

#
#   ejsModule.o
#
DEPS_209 += src/ejs.h

$(BUILD)/obj/ejsModule.o: \
    src/vm/ejsModule.c $(DEPS_209)
	@echo '   [Compile] $(BUILD)/obj/ejsModule.o'
	$(CC) -c -o $(BUILD)/obj/ejsModule.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsModule.c

#
#   ejsMprLog.o
#
DEPS_210 += src/ejs.h

$(BUILD)/obj/ejsMprLog.o: \
    src/core/src/ejsMprLog.c $(DEPS_210)
	@echo '   [Compile] $(BUILD)/obj/ejsMprLog.o'
	$(CC) -c -o $(BUILD)/obj/ejsMprLog.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsMprLog.c

#
#   ejsNamespace.o
#
DEPS_211 += src/ejs.h

$(BUILD)/obj/ejsNamespace.o: \
    src/core/src/ejsNamespace.c $(DEPS_211)
	@echo '   [Compile] $(BUILD)/obj/ejsNamespace.o'
	$(CC) -c -o $(BUILD)/obj/ejsNamespace.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsNamespace.c

#
#   ejsNull.o
#
DEPS_212 += src/ejs.h

$(BUILD)/obj/ejsNull.o: \
    src/core/src/ejsNull.c $(DEPS_212)
	@echo '   [Compile] $(BUILD)/obj/ejsNull.o'
	$(CC) -c -o $(BUILD)/obj/ejsNull.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsNull.c

#
#   ejsNumber.o
#
DEPS_213 += src/ejs.h

$(BUILD)/obj/ejsNumber.o: \
    src/core/src/ejsNumber.c $(DEPS_213)
	@echo '   [Compile] $(BUILD)/obj/ejsNumber.o'
	$(CC) -c -o $(BUILD)/obj/ejsNumber.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsNumber.c

#
#   ejsObject.o
#
DEPS_214 += src/ejs.h

$(BUILD)/obj/ejsObject.o: \
    src/core/src/ejsObject.c $(DEPS_214)
	@echo '   [Compile] $(BUILD)/obj/ejsObject.o'
	$(CC) -c -o $(BUILD)/obj/ejsObject.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsObject.c

#
#   ejsPath.o
#
DEPS_215 += src/ejs.h
DEPS_215 += $(BUILD)/inc/pcre.h

$(BUILD)/obj/ejsPath.o: \
    src/core/src/ejsPath.c $(DEPS_215)
	@echo '   [Compile] $(BUILD)/obj/ejsPath.o'
	$(CC) -c -o $(BUILD)/obj/ejsPath.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsPath.c

#
#   ejsPot.o
#
DEPS_216 += src/ejs.h

$(BUILD)/obj/ejsPot.o: \
    src/core/src/ejsPot.c $(DEPS_216)
	@echo '   [Compile] $(BUILD)/obj/ejsPot.o'
	$(CC) -c -o $(BUILD)/obj/ejsPot.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsPot.c

#
#   ejsRegExp.o
#
DEPS_217 += src/ejs.h
DEPS_217 += $(BUILD)/inc/pcre.h

$(BUILD)/obj/ejsRegExp.o: \
    src/core/src/ejsRegExp.c $(DEPS_217)
	@echo '   [Compile] $(BUILD)/obj/ejsRegExp.o'
	$(CC) -c -o $(BUILD)/obj/ejsRegExp.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsRegExp.c

#
#   ejsRequest.o
#
DEPS_218 += $(BUILD)/inc/me.h
DEPS_218 += $(BUILD)/inc/ejs.h
DEPS_218 += $(BUILD)/inc/ejsCompiler.h
DEPS_218 += src/ejs.web/ejsWeb.h
DEPS_218 += $(BUILD)/inc/ejs.web.slots.h

$(BUILD)/obj/ejsRequest.o: \
    src/ejs.web/ejsRequest.c $(DEPS_218)
	@echo '   [Compile] $(BUILD)/obj/ejsRequest.o'
	$(CC) -c -o $(BUILD)/obj/ejsRequest.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/ejs.web/ejsRequest.c

#
#   ejsScope.o
#
DEPS_219 += src/ejs.h

$(BUILD)/obj/ejsScope.o: \
    src/vm/ejsScope.c $(DEPS_219)
	@echo '   [Compile] $(BUILD)/obj/ejsScope.o'
	$(CC) -c -o $(BUILD)/obj/ejsScope.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsScope.c

#
#   ejsService.o
#
DEPS_220 += src/ejs.h

$(BUILD)/obj/ejsService.o: \
    src/vm/ejsService.c $(DEPS_220)
	@echo '   [Compile] $(BUILD)/obj/ejsService.o'
	$(CC) -c -o $(BUILD)/obj/ejsService.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/vm/ejsService.c

#
#   ejsSession.o
#
DEPS_221 += $(BUILD)/inc/me.h
DEPS_221 += $(BUILD)/inc/ejs.h
DEPS_221 += src/ejs.web/ejsWeb.h

$(BUILD)/obj/ejsSession.o: \
    src/ejs.web/ejsSession.c $(DEPS_221)
	@echo '   [Compile] $(BUILD)/obj/ejsSession.o'
	$(CC) -c -o $(BUILD)/obj/ejsSession.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/ejs.web/ejsSession.c

#
#   ejsSocket.o
#
DEPS_222 += src/ejs.h

$(BUILD)/obj/ejsSocket.o: \
    src/core/src/ejsSocket.c $(DEPS_222)
	@echo '   [Compile] $(BUILD)/obj/ejsSocket.o'
	$(CC) -c -o $(BUILD)/obj/ejsSocket.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsSocket.c

#
#   ejsSqlite.o
#
DEPS_223 += $(BUILD)/inc/ejs.h
DEPS_223 += $(BUILD)/inc/ejs.db.sqlite.slots.h

$(BUILD)/obj/ejsSqlite.o: \
    src/ejs.db.sqlite/ejsSqlite.c $(DEPS_223)
	@echo '   [Compile] $(BUILD)/obj/ejsSqlite.o'
	$(CC) -c -o $(BUILD)/obj/ejsSqlite.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/ejs.db.sqlite/ejsSqlite.c

#
#   ejsString.o
#
DEPS_224 += src/ejs.h
DEPS_224 += $(BUILD)/inc/pcre.h

$(BUILD)/obj/ejsString.o: \
    src/core/src/ejsString.c $(DEPS_224)
	@echo '   [Compile] $(BUILD)/obj/ejsString.o'
	$(CC) -c -o $(BUILD)/obj/ejsString.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsString.c

#
#   ejsSystem.o
#
DEPS_225 += src/ejs.h

$(BUILD)/obj/ejsSystem.o: \
    src/core/src/ejsSystem.c $(DEPS_225)
	@echo '   [Compile] $(BUILD)/obj/ejsSystem.o'
	$(CC) -c -o $(BUILD)/obj/ejsSystem.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsSystem.c

#
#   ejsTimer.o
#
DEPS_226 += src/ejs.h

$(BUILD)/obj/ejsTimer.o: \
    src/core/src/ejsTimer.c $(DEPS_226)
	@echo '   [Compile] $(BUILD)/obj/ejsTimer.o'
	$(CC) -c -o $(BUILD)/obj/ejsTimer.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsTimer.c

#
#   ejsType.o
#
DEPS_227 += src/ejs.h

$(BUILD)/obj/ejsType.o: \
    src/core/src/ejsType.c $(DEPS_227)
	@echo '   [Compile] $(BUILD)/obj/ejsType.o'
	$(CC) -c -o $(BUILD)/obj/ejsType.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsType.c

#
#   ejsUri.o
#
DEPS_228 += src/ejs.h

$(BUILD)/obj/ejsUri.o: \
    src/core/src/ejsUri.c $(DEPS_228)
	@echo '   [Compile] $(BUILD)/obj/ejsUri.o'
	$(CC) -c -o $(BUILD)/obj/ejsUri.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsUri.c

#
#   ejsVoid.o
#
DEPS_229 += src/ejs.h

$(BUILD)/obj/ejsVoid.o: \
    src/core/src/ejsVoid.c $(DEPS_229)
	@echo '   [Compile] $(BUILD)/obj/ejsVoid.o'
	$(CC) -c -o $(BUILD)/obj/ejsVoid.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsVoid.c

#
#   ejsWeb.o
#
DEPS_230 += $(BUILD)/inc/me.h
DEPS_230 += $(BUILD)/inc/ejs.h
DEPS_230 += $(BUILD)/inc/ejsCompiler.h
DEPS_230 += src/ejs.web/ejsWeb.h
DEPS_230 += $(BUILD)/inc/ejs.web.slots.h

$(BUILD)/obj/ejsWeb.o: \
    src/ejs.web/ejsWeb.c $(DEPS_230)
	@echo '   [Compile] $(BUILD)/obj/ejsWeb.o'
	$(CC) -c -o $(BUILD)/obj/ejsWeb.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/ejs.web/ejsWeb.c

#
#   ejsWebSocket.o
#
DEPS_231 += src/ejs.h

$(BUILD)/obj/ejsWebSocket.o: \
    src/core/src/ejsWebSocket.c $(DEPS_231)
	@echo '   [Compile] $(BUILD)/obj/ejsWebSocket.o'
	$(CC) -c -o $(BUILD)/obj/ejsWebSocket.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsWebSocket.c

#
#   ejsWorker.o
#
DEPS_232 += src/ejs.h

$(BUILD)/obj/ejsWorker.o: \
    src/core/src/ejsWorker.c $(DEPS_232)
	@echo '   [Compile] $(BUILD)/obj/ejsWorker.o'
	$(CC) -c -o $(BUILD)/obj/ejsWorker.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsWorker.c

#
#   ejsXML.o
#
DEPS_233 += src/ejs.h

$(BUILD)/obj/ejsXML.o: \
    src/core/src/ejsXML.c $(DEPS_233)
	@echo '   [Compile] $(BUILD)/obj/ejsXML.o'
	$(CC) -c -o $(BUILD)/obj/ejsXML.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsXML.c

#
#   ejsXMLList.o
#
DEPS_234 += src/ejs.h

$(BUILD)/obj/ejsXMLList.o: \
    src/core/src/ejsXMLList.c $(DEPS_234)
	@echo '   [Compile] $(BUILD)/obj/ejsXMLList.o'
	$(CC) -c -o $(BUILD)/obj/ejsXMLList.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsXMLList.c

#
#   ejsXMLLoader.o
#
DEPS_235 += src/ejs.h

$(BUILD)/obj/ejsXMLLoader.o: \
    src/core/src/ejsXMLLoader.c $(DEPS_235)
	@echo '   [Compile] $(BUILD)/obj/ejsXMLLoader.o'
	$(CC) -c -o $(BUILD)/obj/ejsXMLLoader.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/core/src/ejsXMLLoader.c

#
#   ejsZlib.o
#
DEPS_236 += $(BUILD)/inc/ejs.h
DEPS_236 += $(BUILD)/inc/zlib.h
DEPS_236 += $(BUILD)/inc/ejs.zlib.slots.h

$(BUILD)/obj/ejsZlib.o: \
    src/ejs.zlib/ejsZlib.c $(DEPS_236)
	@echo '   [Compile] $(BUILD)/obj/ejsZlib.o'
	$(CC) -c -o $(BUILD)/obj/ejsZlib.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/ejs.zlib/ejsZlib.c

#
#   ejsc.o
#
DEPS_237 += $(BUILD)/inc/ejsCompiler.h

$(BUILD)/obj/ejsc.o: \
    src/cmd/ejsc.c $(DEPS_237)
	@echo '   [Compile] $(BUILD)/obj/ejsc.o'
	$(CC) -c -o $(BUILD)/obj/ejsc.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/cmd/ejsc.c

#
#   ejsmod.o
#
DEPS_238 += src/cmd/ejsmod.h

$(BUILD)/obj/ejsmod.o: \
    src/cmd/ejsmod.c $(DEPS_238)
	@echo '   [Compile] $(BUILD)/obj/ejsmod.o'
	$(CC) -c -o $(BUILD)/obj/ejsmod.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/cmd/ejsmod.c

#
#   ejsrun.o
#
DEPS_239 += $(BUILD)/inc/ejsCompiler.h

$(BUILD)/obj/ejsrun.o: \
    src/cmd/ejsrun.c $(DEPS_239)
	@echo '   [Compile] $(BUILD)/obj/ejsrun.o'
	$(CC) -c -o $(BUILD)/obj/ejsrun.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/cmd/ejsrun.c

#
#   http.h
#

src/http/http.h: $(DEPS_240)

#
#   httpLib.o
#
DEPS_241 += src/http/http.h

$(BUILD)/obj/httpLib.o: \
    src/http/httpLib.c $(DEPS_241)
	@echo '   [Compile] $(BUILD)/obj/httpLib.o'
	$(CC) -c -o $(BUILD)/obj/httpLib.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/http/httpLib.c

#
#   listing.o
#
DEPS_242 += src/cmd/ejsmod.h
DEPS_242 += $(BUILD)/inc/ejsByteCodeTable.h

$(BUILD)/obj/listing.o: \
    src/cmd/listing.c $(DEPS_242)
	@echo '   [Compile] $(BUILD)/obj/listing.o'
	$(CC) -c -o $(BUILD)/obj/listing.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/cmd/listing.c

#
#   mpr.h
#

src/mpr/mpr.h: $(DEPS_243)

#
#   makerom.o
#
DEPS_244 += src/mpr/mpr.h

$(BUILD)/obj/makerom.o: \
    src/mpr/makerom.c $(DEPS_244)
	@echo '   [Compile] $(BUILD)/obj/makerom.o'
	$(CC) -c -o $(BUILD)/obj/makerom.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/mpr/makerom.c

#
#   manager.o
#
DEPS_245 += src/mpr/mpr.h

$(BUILD)/obj/manager.o: \
    src/mpr/manager.c $(DEPS_245)
	@echo '   [Compile] $(BUILD)/obj/manager.o'
	$(CC) -c -o $(BUILD)/obj/manager.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/mpr/manager.c

#
#   mprLib.o
#
DEPS_246 += src/mpr/mpr.h

$(BUILD)/obj/mprLib.o: \
    src/mpr/mprLib.c $(DEPS_246)
	@echo '   [Compile] $(BUILD)/obj/mprLib.o'
	$(CC) -c -o $(BUILD)/obj/mprLib.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/mpr/mprLib.c

#
#   mprSsl.o
#
DEPS_247 += src/mpr/mpr.h

$(BUILD)/obj/mprSsl.o: \
    src/mpr/mprSsl.c $(DEPS_247)
	@echo '   [Compile] $(BUILD)/obj/mprSsl.o'
	$(CC) -c -o $(BUILD)/obj/mprSsl.o $(CFLAGS) $(DFLAGS) -DME_COM_OPENSSL_PATH="$(ME_COM_OPENSSL_PATH)" "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" "-I$(ME_COM_OPENSSL_PATH)/include" src/mpr/mprSsl.c

#
#   pcre.h
#

src/pcre/pcre.h: $(DEPS_248)

#
#   pcre.o
#
DEPS_249 += $(BUILD)/inc/me.h
DEPS_249 += src/pcre/pcre.h

$(BUILD)/obj/pcre.o: \
    src/pcre/pcre.c $(DEPS_249)
	@echo '   [Compile] $(BUILD)/obj/pcre.o'
	$(CC) -c -o $(BUILD)/obj/pcre.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/pcre/pcre.c

#
#   slotGen.o
#
DEPS_250 += src/cmd/ejsmod.h

$(BUILD)/obj/slotGen.o: \
    src/cmd/slotGen.c $(DEPS_250)
	@echo '   [Compile] $(BUILD)/obj/slotGen.o'
	$(CC) -c -o $(BUILD)/obj/slotGen.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/cmd/slotGen.c

#
#   sqlite3.h
#

src/sqlite/sqlite3.h: $(DEPS_251)

#
#   sqlite.o
#
DEPS_252 += $(BUILD)/inc/me.h
DEPS_252 += src/sqlite/sqlite3.h

$(BUILD)/obj/sqlite.o: \
    src/sqlite/sqlite.c $(DEPS_252)
	@echo '   [Compile] $(BUILD)/obj/sqlite.o'
	$(CC) -c -o $(BUILD)/obj/sqlite.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/sqlite/sqlite.c

#
#   sqlite3.o
#
DEPS_253 += $(BUILD)/inc/me.h
DEPS_253 += src/sqlite/sqlite3.h

$(BUILD)/obj/sqlite3.o: \
    src/sqlite/sqlite3.c $(DEPS_253)
	@echo '   [Compile] $(BUILD)/obj/sqlite3.o'
	$(CC) -c -o $(BUILD)/obj/sqlite3.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/sqlite/sqlite3.c

#
#   zlib.h
#

src/zlib/zlib.h: $(DEPS_254)

#
#   zlib.o
#
DEPS_255 += $(BUILD)/inc/me.h
DEPS_255 += src/zlib/zlib.h

$(BUILD)/obj/zlib.o: \
    src/zlib/zlib.c $(DEPS_255)
	@echo '   [Compile] $(BUILD)/obj/zlib.o'
	$(CC) -c -o $(BUILD)/obj/zlib.o $(CFLAGS) $(DFLAGS) "-I$(BUILD)/inc" "-I$(WIND_BASE)/target/h" "-I$(WIND_BASE)/target/h/wrn/coreip" src/zlib/zlib.c

#
#   slots
#

slots: $(DEPS_256)

#
#   libmpr
#
DEPS_257 += $(BUILD)/inc/osdep.h
DEPS_257 += $(BUILD)/inc/mpr.h
DEPS_257 += $(BUILD)/obj/mprLib.o

$(BUILD)/bin/libmpr.out: $(DEPS_257)
	@echo '      [Link] $(BUILD)/bin/libmpr.out'
	$(CC) -r -o $(BUILD)/bin/libmpr.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/mprLib.o" $(LIBS) 

ifeq ($(ME_COM_PCRE),1)
#
#   libpcre
#
DEPS_258 += $(BUILD)/inc/pcre.h
DEPS_258 += $(BUILD)/obj/pcre.o

$(BUILD)/bin/libpcre.out: $(DEPS_258)
	@echo '      [Link] $(BUILD)/bin/libpcre.out'
	$(CC) -r -o $(BUILD)/bin/libpcre.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/pcre.o" $(LIBS) 
endif

ifeq ($(ME_COM_HTTP),1)
#
#   libhttp
#
DEPS_259 += $(BUILD)/bin/libmpr.out
ifeq ($(ME_COM_PCRE),1)
    DEPS_259 += $(BUILD)/bin/libpcre.out
endif
DEPS_259 += $(BUILD)/inc/http.h
DEPS_259 += $(BUILD)/obj/httpLib.o

$(BUILD)/bin/libhttp.out: $(DEPS_259)
	@echo '      [Link] $(BUILD)/bin/libhttp.out'
	$(CC) -r -o $(BUILD)/bin/libhttp.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/httpLib.o" $(LIBS) 
endif

#
#   libejs
#
DEPS_260 += slots
ifeq ($(ME_COM_HTTP),1)
    DEPS_260 += $(BUILD)/bin/libhttp.out
endif
DEPS_260 += $(BUILD)/inc/ejs.cache.local.slots.h
DEPS_260 += $(BUILD)/inc/ejs.db.sqlite.slots.h
DEPS_260 += $(BUILD)/inc/ejs.slots.h
DEPS_260 += $(BUILD)/inc/ejs.web.slots.h
DEPS_260 += $(BUILD)/inc/ejs.zlib.slots.h
DEPS_260 += $(BUILD)/inc/ejs.h
DEPS_260 += $(BUILD)/inc/ejsByteCode.h
DEPS_260 += $(BUILD)/inc/ejsByteCodeTable.h
DEPS_260 += $(BUILD)/inc/ejsCompiler.h
DEPS_260 += $(BUILD)/inc/ejsCustomize.h
DEPS_260 += $(BUILD)/obj/ecAst.o
DEPS_260 += $(BUILD)/obj/ecCodeGen.o
DEPS_260 += $(BUILD)/obj/ecCompiler.o
DEPS_260 += $(BUILD)/obj/ecLex.o
DEPS_260 += $(BUILD)/obj/ecModuleWrite.o
DEPS_260 += $(BUILD)/obj/ecParser.o
DEPS_260 += $(BUILD)/obj/ecState.o
DEPS_260 += $(BUILD)/obj/dtoa.o
DEPS_260 += $(BUILD)/obj/ejsApp.o
DEPS_260 += $(BUILD)/obj/ejsArray.o
DEPS_260 += $(BUILD)/obj/ejsBlock.o
DEPS_260 += $(BUILD)/obj/ejsBoolean.o
DEPS_260 += $(BUILD)/obj/ejsByteArray.o
DEPS_260 += $(BUILD)/obj/ejsCache.o
DEPS_260 += $(BUILD)/obj/ejsCmd.o
DEPS_260 += $(BUILD)/obj/ejsConfig.o
DEPS_260 += $(BUILD)/obj/ejsDate.o
DEPS_260 += $(BUILD)/obj/ejsDebug.o
DEPS_260 += $(BUILD)/obj/ejsError.o
DEPS_260 += $(BUILD)/obj/ejsFile.o
DEPS_260 += $(BUILD)/obj/ejsFileSystem.o
DEPS_260 += $(BUILD)/obj/ejsFrame.o
DEPS_260 += $(BUILD)/obj/ejsFunction.o
DEPS_260 += $(BUILD)/obj/ejsGC.o
DEPS_260 += $(BUILD)/obj/ejsGlobal.o
DEPS_260 += $(BUILD)/obj/ejsHttp.o
DEPS_260 += $(BUILD)/obj/ejsIterator.o
DEPS_260 += $(BUILD)/obj/ejsJson.o
DEPS_260 += $(BUILD)/obj/ejsLocalCache.o
DEPS_260 += $(BUILD)/obj/ejsMath.o
DEPS_260 += $(BUILD)/obj/ejsMemory.o
DEPS_260 += $(BUILD)/obj/ejsMprLog.o
DEPS_260 += $(BUILD)/obj/ejsNamespace.o
DEPS_260 += $(BUILD)/obj/ejsNull.o
DEPS_260 += $(BUILD)/obj/ejsNumber.o
DEPS_260 += $(BUILD)/obj/ejsObject.o
DEPS_260 += $(BUILD)/obj/ejsPath.o
DEPS_260 += $(BUILD)/obj/ejsPot.o
DEPS_260 += $(BUILD)/obj/ejsRegExp.o
DEPS_260 += $(BUILD)/obj/ejsSocket.o
DEPS_260 += $(BUILD)/obj/ejsString.o
DEPS_260 += $(BUILD)/obj/ejsSystem.o
DEPS_260 += $(BUILD)/obj/ejsTimer.o
DEPS_260 += $(BUILD)/obj/ejsType.o
DEPS_260 += $(BUILD)/obj/ejsUri.o
DEPS_260 += $(BUILD)/obj/ejsVoid.o
DEPS_260 += $(BUILD)/obj/ejsWebSocket.o
DEPS_260 += $(BUILD)/obj/ejsWorker.o
DEPS_260 += $(BUILD)/obj/ejsXML.o
DEPS_260 += $(BUILD)/obj/ejsXMLList.o
DEPS_260 += $(BUILD)/obj/ejsXMLLoader.o
DEPS_260 += $(BUILD)/obj/ejsByteCode.o
DEPS_260 += $(BUILD)/obj/ejsException.o
DEPS_260 += $(BUILD)/obj/ejsHelper.o
DEPS_260 += $(BUILD)/obj/ejsInterp.o
DEPS_260 += $(BUILD)/obj/ejsLoader.o
DEPS_260 += $(BUILD)/obj/ejsModule.o
DEPS_260 += $(BUILD)/obj/ejsScope.o
DEPS_260 += $(BUILD)/obj/ejsService.o

$(BUILD)/bin/libejs.out: $(DEPS_260)
	@echo '      [Link] $(BUILD)/bin/libejs.out'
	$(CC) -r -o $(BUILD)/bin/libejs.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ecAst.o" "$(BUILD)/obj/ecCodeGen.o" "$(BUILD)/obj/ecCompiler.o" "$(BUILD)/obj/ecLex.o" "$(BUILD)/obj/ecModuleWrite.o" "$(BUILD)/obj/ecParser.o" "$(BUILD)/obj/ecState.o" "$(BUILD)/obj/dtoa.o" "$(BUILD)/obj/ejsApp.o" "$(BUILD)/obj/ejsArray.o" "$(BUILD)/obj/ejsBlock.o" "$(BUILD)/obj/ejsBoolean.o" "$(BUILD)/obj/ejsByteArray.o" "$(BUILD)/obj/ejsCache.o" "$(BUILD)/obj/ejsCmd.o" "$(BUILD)/obj/ejsConfig.o" "$(BUILD)/obj/ejsDate.o" "$(BUILD)/obj/ejsDebug.o" "$(BUILD)/obj/ejsError.o" "$(BUILD)/obj/ejsFile.o" "$(BUILD)/obj/ejsFileSystem.o" "$(BUILD)/obj/ejsFrame.o" "$(BUILD)/obj/ejsFunction.o" "$(BUILD)/obj/ejsGC.o" "$(BUILD)/obj/ejsGlobal.o" "$(BUILD)/obj/ejsHttp.o" "$(BUILD)/obj/ejsIterator.o" "$(BUILD)/obj/ejsJson.o" "$(BUILD)/obj/ejsLocalCache.o" "$(BUILD)/obj/ejsMath.o" "$(BUILD)/obj/ejsMemory.o" "$(BUILD)/obj/ejsMprLog.o" "$(BUILD)/obj/ejsNamespace.o" "$(BUILD)/obj/ejsNull.o" "$(BUILD)/obj/ejsNumber.o" "$(BUILD)/obj/ejsObject.o" "$(BUILD)/obj/ejsPath.o" "$(BUILD)/obj/ejsPot.o" "$(BUILD)/obj/ejsRegExp.o" "$(BUILD)/obj/ejsSocket.o" "$(BUILD)/obj/ejsString.o" "$(BUILD)/obj/ejsSystem.o" "$(BUILD)/obj/ejsTimer.o" "$(BUILD)/obj/ejsType.o" "$(BUILD)/obj/ejsUri.o" "$(BUILD)/obj/ejsVoid.o" "$(BUILD)/obj/ejsWebSocket.o" "$(BUILD)/obj/ejsWorker.o" "$(BUILD)/obj/ejsXML.o" "$(BUILD)/obj/ejsXMLList.o" "$(BUILD)/obj/ejsXMLLoader.o" "$(BUILD)/obj/ejsByteCode.o" "$(BUILD)/obj/ejsException.o" "$(BUILD)/obj/ejsHelper.o" "$(BUILD)/obj/ejsInterp.o" "$(BUILD)/obj/ejsLoader.o" "$(BUILD)/obj/ejsModule.o" "$(BUILD)/obj/ejsScope.o" "$(BUILD)/obj/ejsService.o" $(LIBS) 

#
#   ejs
#
DEPS_261 += $(BUILD)/bin/libejs.out
DEPS_261 += $(BUILD)/obj/ejs.o

$(BUILD)/bin/ejs.out: $(DEPS_261)
	@echo '      [Link] $(BUILD)/bin/ejs.out'
	$(CC) -o $(BUILD)/bin/ejs.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejs.o" $(LIBS) -Wl,-r 

#
#   ejsc
#
DEPS_262 += $(BUILD)/bin/libejs.out
DEPS_262 += $(BUILD)/obj/ejsc.o

$(BUILD)/bin/ejsc.out: $(DEPS_262)
	@echo '      [Link] $(BUILD)/bin/ejsc.out'
	$(CC) -o $(BUILD)/bin/ejsc.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsc.o" $(LIBS) -Wl,-r 

#
#   ejsmod
#
DEPS_263 += $(BUILD)/bin/libejs.out
DEPS_263 += $(BUILD)/inc/ejsmod.h
DEPS_263 += $(BUILD)/obj/ejsmod.o
DEPS_263 += $(BUILD)/obj/doc.o
DEPS_263 += $(BUILD)/obj/docFiles.o
DEPS_263 += $(BUILD)/obj/listing.o
DEPS_263 += $(BUILD)/obj/slotGen.o

$(BUILD)/bin/ejsmod.out: $(DEPS_263)
	@echo '      [Link] $(BUILD)/bin/ejsmod.out'
	$(CC) -o $(BUILD)/bin/ejsmod.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsmod.o" "$(BUILD)/obj/doc.o" "$(BUILD)/obj/docFiles.o" "$(BUILD)/obj/listing.o" "$(BUILD)/obj/slotGen.o" $(LIBS) -Wl,-r 

#
#   ejs.mod
#
DEPS_264 += src/core/App.es
DEPS_264 += src/core/Args.es
DEPS_264 += src/core/Array.es
DEPS_264 += src/core/BinaryStream.es
DEPS_264 += src/core/Block.es
DEPS_264 += src/core/Boolean.es
DEPS_264 += src/core/ByteArray.es
DEPS_264 += src/core/Cache.es
DEPS_264 += src/core/Cmd.es
DEPS_264 += src/core/Compat.es
DEPS_264 += src/core/Config.es
DEPS_264 += src/core/Date.es
DEPS_264 += src/core/Debug.es
DEPS_264 += src/core/Emitter.es
DEPS_264 += src/core/Error.es
DEPS_264 += src/core/File.es
DEPS_264 += src/core/FileSystem.es
DEPS_264 += src/core/Frame.es
DEPS_264 += src/core/Function.es
DEPS_264 += src/core/GC.es
DEPS_264 += src/core/Global.es
DEPS_264 += src/core/Http.es
DEPS_264 += src/core/Inflector.es
DEPS_264 += src/core/Iterator.es
DEPS_264 += src/core/JSON.es
DEPS_264 += src/core/Loader.es
DEPS_264 += src/core/LocalCache.es
DEPS_264 += src/core/Locale.es
DEPS_264 += src/core/Logger.es
DEPS_264 += src/core/Math.es
DEPS_264 += src/core/Memory.es
DEPS_264 += src/core/MprLog.es
DEPS_264 += src/core/Name.es
DEPS_264 += src/core/Namespace.es
DEPS_264 += src/core/Null.es
DEPS_264 += src/core/Number.es
DEPS_264 += src/core/Object.es
DEPS_264 += src/core/Path.es
DEPS_264 += src/core/Promise.es
DEPS_264 += src/core/RegExp.es
DEPS_264 += src/core/Socket.es
DEPS_264 += src/core/Stream.es
DEPS_264 += src/core/String.es
DEPS_264 += src/core/System.es
DEPS_264 += src/core/TextStream.es
DEPS_264 += src/core/Timer.es
DEPS_264 += src/core/Type.es
DEPS_264 += src/core/Uri.es
DEPS_264 += src/core/Void.es
DEPS_264 += src/core/WebSocket.es
DEPS_264 += src/core/Worker.es
DEPS_264 += src/core/XML.es
DEPS_264 += src/core/XMLHttp.es
DEPS_264 += src/core/XMLList.es
DEPS_264 += $(BUILD)/bin/ejsc.out
DEPS_264 += $(BUILD)/bin/ejsmod.out

$(BUILD)/bin/ejs.mod: $(DEPS_264)
	( \
	cd src/core; \
	echo '   [Compile] Core EJS classes' ; \
	../../$(BUILD)/bin/ejsc --out ../../$(BUILD)/bin/ejs.mod  --optimize 9 --bind --require null App.es Args.es Array.es BinaryStream.es Block.es Boolean.es ByteArray.es Cache.es Cmd.es Compat.es Config.es Date.es Debug.es Emitter.es Error.es File.es FileSystem.es Frame.es Function.es GC.es Global.es Http.es Inflector.es Iterator.es JSON.es Loader.es LocalCache.es Locale.es Logger.es Math.es Memory.es MprLog.es Name.es Namespace.es Null.es Number.es Object.es Path.es Promise.es RegExp.es Socket.es Stream.es String.es System.es TextStream.es Timer.es Type.es Uri.es Void.es WebSocket.es Worker.es XML.es XMLHttp.es XMLList.es ; \
	../../$(BUILD)/bin/ejsmod --cslots --dir ../../$(BUILD)/inc --require null ../../$(BUILD)/bin/ejs.mod ; \
	)

#
#   ejs.db.mod
#
DEPS_265 += src/ejs.db/Database.es
DEPS_265 += src/ejs.db/DatabaseConnector.es
DEPS_265 += $(BUILD)/bin/ejsc.out
DEPS_265 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.db.mod: $(DEPS_265)
	( \
	cd src/ejs.db; \
	echo '   [Compile] ejs.db.mod' ; \
	../../$(BUILD)/bin/ejsc --out ../../$(BUILD)/bin/ejs.db.mod  --optimize 9 Database.es DatabaseConnector.es ; \
	)

#
#   ejs.db.mapper.mod
#
DEPS_266 += src/ejs.db.mapper/Record.es
DEPS_266 += $(BUILD)/bin/ejsc.out
DEPS_266 += $(BUILD)/bin/ejs.mod
DEPS_266 += $(BUILD)/bin/ejs.db.mod

$(BUILD)/bin/ejs.db.mapper.mod: $(DEPS_266)
	( \
	cd src/ejs.db.mapper; \
	echo '   [Compile] ejs.db.mapper.mod' ; \
	../../$(BUILD)/bin/ejsc --out ../../$(BUILD)/bin/ejs.db.mapper.mod  --optimize 9 Record.es ; \
	)

#
#   ejs.db.sqlite.mod
#
DEPS_267 += src/ejs.db.sqlite/Sqlite.es
DEPS_267 += $(BUILD)/bin/ejsc.out
DEPS_267 += $(BUILD)/bin/ejsmod.out
DEPS_267 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.db.sqlite.mod: $(DEPS_267)
	( \
	cd src/ejs.db.sqlite; \
	echo '   [Compile] ejs.db.sqlite.mod' ; \
	../../$(BUILD)/bin/ejsc --out ../../$(BUILD)/bin/ejs.db.sqlite.mod  --optimize 9 Sqlite.es ; \
	../../$(BUILD)/bin/ejsmod --cslots --dir ../../$(BUILD)/bin ../../$(BUILD)/bin/ejs.db.sqlite.mod ; \
	)

#
#   ejs.mail.mod
#
DEPS_268 += src/ejs.mail/Mail.es
DEPS_268 += $(BUILD)/bin/ejsc.out
DEPS_268 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.mail.mod: $(DEPS_268)
	( \
	cd src/ejs.mail; \
	../../$(BUILD)/bin/ejsc --out ../../$(BUILD)/bin/ejs.mail.mod  --optimize 9 Mail.es ; \
	)

#
#   ejs.web.mod
#
DEPS_269 += src/ejs.web/Cascade.es
DEPS_269 += src/ejs.web/CommonLog.es
DEPS_269 += src/ejs.web/ContentType.es
DEPS_269 += src/ejs.web/Controller.es
DEPS_269 += src/ejs.web/Dir.es
DEPS_269 += src/ejs.web/Google.es
DEPS_269 += src/ejs.web/Head.es
DEPS_269 += src/ejs.web/Html.es
DEPS_269 += src/ejs.web/HttpServer.es
DEPS_269 += src/ejs.web/MethodOverride.es
DEPS_269 += src/ejs.web/Middleware.es
DEPS_269 += src/ejs.web/Mvc.es
DEPS_269 += src/ejs.web/Request.es
DEPS_269 += src/ejs.web/Router.es
DEPS_269 += src/ejs.web/Script.es
DEPS_269 += src/ejs.web/Session.es
DEPS_269 += src/ejs.web/ShowExceptions.es
DEPS_269 += src/ejs.web/Static.es
DEPS_269 += src/ejs.web/Template.es
DEPS_269 += src/ejs.web/UploadFile.es
DEPS_269 += src/ejs.web/UrlMap.es
DEPS_269 += src/ejs.web/Utils.es
DEPS_269 += src/ejs.web/View.es
DEPS_269 += $(BUILD)/bin/ejsc.out
DEPS_269 += $(BUILD)/bin/ejsmod.out
DEPS_269 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.web.mod: $(DEPS_269)
	( \
	cd src/ejs.web; \
	echo '   [Compile] ejs.web.mod' ; \
	../../$(BUILD)/bin/ejsc --out ../../$(BUILD)/bin/ejs.web.mod  --optimize 9 Cascade.es CommonLog.es ContentType.es Controller.es Dir.es Google.es Head.es Html.es HttpServer.es MethodOverride.es Middleware.es Mvc.es Request.es Router.es Script.es Session.es ShowExceptions.es Static.es Template.es UploadFile.es UrlMap.es Utils.es View.es ; \
	../../$(BUILD)/bin/ejsmod --cslots --dir ../../$(BUILD)/bin ../../$(BUILD)/bin/ejs.web.mod ; \
	)

#
#   ejs.template.mod
#
DEPS_270 += src/ejs.template/TemplateParser.es
DEPS_270 += $(BUILD)/bin/ejsc.out
DEPS_270 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.template.mod: $(DEPS_270)
	( \
	cd src/ejs.template; \
	echo '   [Compile] ejs.template.mod' ; \
	../../$(BUILD)/bin/ejsc --out ../../$(BUILD)/bin/ejs.template.mod  --optimize 9 TemplateParser.es ; \
	)

#
#   ejs.unix.mod
#
DEPS_271 += src/ejs.unix/Unix.es
DEPS_271 += $(BUILD)/bin/ejsc.out
DEPS_271 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.unix.mod: $(DEPS_271)
	( \
	cd src/ejs.unix; \
	echo '   [Compile] ejs.unix.mod' ; \
	../../$(BUILD)/bin/ejsc --out ../../$(BUILD)/bin/ejs.unix.mod  --optimize 9 Unix.es ; \
	)

#
#   ejs.mvc.mod
#
DEPS_272 += src/ejs.mvc/mvc.es
DEPS_272 += $(BUILD)/bin/ejsc.out
DEPS_272 += $(BUILD)/bin/ejs.mod
DEPS_272 += $(BUILD)/bin/ejs.web.mod
DEPS_272 += $(BUILD)/bin/ejs.template.mod
DEPS_272 += $(BUILD)/bin/ejs.unix.mod

$(BUILD)/bin/ejs.mvc.mod: $(DEPS_272)
	( \
	cd src/ejs.mvc; \
	echo '   [Compile] ejs.mvc.mod' ; \
	../../$(BUILD)/bin/ejsc --out ../../$(BUILD)/bin/ejs.mvc.mod  --optimize 9 mvc.es ; \
	)

#
#   ejs.zlib.mod
#
DEPS_273 += src/ejs.zlib/Zlib.es
DEPS_273 += $(BUILD)/bin/ejsc.out
DEPS_273 += $(BUILD)/bin/ejs.mod

$(BUILD)/bin/ejs.zlib.mod: $(DEPS_273)
	( \
	cd src/ejs.zlib; \
	echo '   [Compile] ejs.zlib.mod' ; \
	../../$(BUILD)/bin/ejsc --out ../../$(BUILD)/bin/ejs.zlib.mod  --optimize 9 Zlib.es ; \
	)

ifeq ($(ME_COM_ZLIB),1)
#
#   libzlib
#
DEPS_274 += $(BUILD)/inc/zlib.h
DEPS_274 += $(BUILD)/obj/zlib.o

$(BUILD)/bin/libzlib.out: $(DEPS_274)
	@echo '      [Link] $(BUILD)/bin/libzlib.out'
	$(CC) -r -o $(BUILD)/bin/libzlib.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/zlib.o" $(LIBS) 
endif

#
#   libejs.zlib
#
DEPS_275 += $(BUILD)/bin/libejs.out
DEPS_275 += $(BUILD)/bin/ejs.mod
DEPS_275 += $(BUILD)/bin/ejs.zlib.mod
ifeq ($(ME_COM_ZLIB),1)
    DEPS_275 += $(BUILD)/bin/libzlib.out
endif
DEPS_275 += $(BUILD)/obj/ejsZlib.o

$(BUILD)/bin/libejs.zlib.out: $(DEPS_275)
	@echo '      [Link] $(BUILD)/bin/libejs.zlib.out'
	$(CC) -r -o $(BUILD)/bin/libejs.zlib.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsZlib.o" $(LIBS) 

#
#   ejs.tar.mod
#
DEPS_276 += src/ejs.tar/Tar.es
DEPS_276 += $(BUILD)/bin/ejsc.out
DEPS_276 += $(BUILD)/bin/ejs.mod
DEPS_276 += $(BUILD)/bin/libejs.zlib.out

$(BUILD)/bin/ejs.tar.mod: $(DEPS_276)
	( \
	cd src/ejs.tar; \
	echo '   [Compile] ejs.tar.mod' ; \
	../../$(BUILD)/bin/ejsc --out ../../$(BUILD)/bin/ejs.tar.mod  --optimize 9 Tar.es ; \
	)

#
#   ejsrun
#
DEPS_277 += $(BUILD)/bin/libejs.out
DEPS_277 += $(BUILD)/obj/ejsrun.o

$(BUILD)/bin/ejsrun.out: $(DEPS_277)
	@echo '      [Link] $(BUILD)/bin/ejsrun.out'
	$(CC) -o $(BUILD)/bin/ejsrun.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsrun.o" $(LIBS) -Wl,-r 

#
#   http-ca-crt
#
DEPS_278 += src/http/ca.crt

$(BUILD)/bin/ca.crt: $(DEPS_278)
	@echo '      [Copy] $(BUILD)/bin/ca.crt'
	mkdir -p "$(BUILD)/bin"
	cp src/http/ca.crt $(BUILD)/bin/ca.crt

ifeq ($(ME_COM_SQLITE),1)
#
#   libsql
#
DEPS_279 += $(BUILD)/inc/sqlite3.h
DEPS_279 += $(BUILD)/obj/sqlite3.o

$(BUILD)/bin/libsql.out: $(DEPS_279)
	@echo '      [Link] $(BUILD)/bin/libsql.out'
	$(CC) -r -o $(BUILD)/bin/libsql.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/sqlite3.o" $(LIBS) 
endif

#
#   libejs.db.sqlite
#
DEPS_280 += $(BUILD)/bin/libmpr.out
DEPS_280 += $(BUILD)/bin/libejs.out
DEPS_280 += $(BUILD)/bin/ejs.mod
DEPS_280 += $(BUILD)/bin/ejs.db.sqlite.mod
ifeq ($(ME_COM_SQLITE),1)
    DEPS_280 += $(BUILD)/bin/libsql.out
endif
DEPS_280 += $(BUILD)/obj/ejsSqlite.o

$(BUILD)/bin/libejs.db.sqlite.out: $(DEPS_280)
	@echo '      [Link] $(BUILD)/bin/libejs.db.sqlite.out'
	$(CC) -r -o $(BUILD)/bin/libejs.db.sqlite.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsSqlite.o" $(LIBS) 

#
#   libejs.web
#
DEPS_281 += $(BUILD)/bin/libejs.out
DEPS_281 += $(BUILD)/bin/ejs.mod
DEPS_281 += $(BUILD)/inc/ejsWeb.h
DEPS_281 += $(BUILD)/obj/ejsHttpServer.o
DEPS_281 += $(BUILD)/obj/ejsRequest.o
DEPS_281 += $(BUILD)/obj/ejsSession.o
DEPS_281 += $(BUILD)/obj/ejsWeb.o

$(BUILD)/bin/libejs.web.out: $(DEPS_281)
	@echo '      [Link] $(BUILD)/bin/libejs.web.out'
	$(CC) -r -o $(BUILD)/bin/libejs.web.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsHttpServer.o" "$(BUILD)/obj/ejsRequest.o" "$(BUILD)/obj/ejsSession.o" "$(BUILD)/obj/ejsWeb.o" $(LIBS) 

#
#   libmprssl
#
DEPS_282 += $(BUILD)/bin/libmpr.out
DEPS_282 += $(BUILD)/obj/mprSsl.o

ifeq ($(ME_COM_OPENSSL),1)
    LIBS_282 += -lssl
    LIBPATHS_282 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_282 += -L"$(ME_COM_OPENSSL_PATH)"
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_282 += -lcrypto
    LIBPATHS_282 += -L"$(ME_COM_OPENSSL_PATH)/lib"
    LIBPATHS_282 += -L"$(ME_COM_OPENSSL_PATH)"
endif

$(BUILD)/bin/libmprssl.out: $(DEPS_282)
	@echo '      [Link] $(BUILD)/bin/libmprssl.out'
	$(CC) -r -o $(BUILD)/bin/libmprssl.out $(LDFLAGS) $(LIBPATHS)   "$(BUILD)/obj/mprSsl.o" $(LIBPATHS_282) $(LIBS_282) $(LIBS_282) $(LIBS) 

#
#   manager
#
DEPS_283 += $(BUILD)/bin/libmpr.out
DEPS_283 += $(BUILD)/obj/manager.o

$(BUILD)/bin/ejsman.out: $(DEPS_283)
	@echo '      [Link] $(BUILD)/bin/ejsman.out'
	$(CC) -o $(BUILD)/bin/ejsman.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/manager.o" $(LIBS) -Wl,-r 

#
#   mvc.es
#
DEPS_284 += src/ejs.mvc/mvc.es

$(BUILD)/bin/mvc.es: $(DEPS_284)
	@echo '      [Copy] $(BUILD)/bin/mvc.es'
	mkdir -p "$(BUILD)/bin"
	cp src/ejs.mvc/mvc.es $(BUILD)/bin/mvc.es

#
#   mvc
#
DEPS_285 += $(BUILD)/bin/libejs.out
DEPS_285 += $(BUILD)/bin/mvc.es
DEPS_285 += $(BUILD)/obj/ejsrun.o

$(BUILD)/bin/mvc.out: $(DEPS_285)
	@echo '      [Link] $(BUILD)/bin/mvc.out'
	$(CC) -o $(BUILD)/bin/mvc.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsrun.o" $(LIBS) -Wl,-r 

#
#   utest.es
#
DEPS_286 += src/ejs.utest/utest.es

$(BUILD)/bin/utest.es: $(DEPS_286)
	@echo '      [Copy] $(BUILD)/bin/utest.es'
	mkdir -p "$(BUILD)/bin"
	cp src/ejs.utest/utest.es $(BUILD)/bin/utest.es

#
#   utest.worker
#
DEPS_287 += src/ejs.utest/utest.worker

$(BUILD)/bin/utest.worker: $(DEPS_287)
	@echo '      [Copy] $(BUILD)/bin/utest.worker'
	mkdir -p "$(BUILD)/bin"
	cp src/ejs.utest/utest.worker $(BUILD)/bin/utest.worker

#
#   utest
#
DEPS_288 += $(BUILD)/bin/libejs.out
DEPS_288 += $(BUILD)/bin/utest.es
DEPS_288 += $(BUILD)/bin/utest.worker
DEPS_288 += $(BUILD)/obj/ejsrun.o

$(BUILD)/bin/utest.out: $(DEPS_288)
	@echo '      [Link] $(BUILD)/bin/utest.out'
	$(CC) -o $(BUILD)/bin/utest.out $(LDFLAGS) $(LIBPATHS) "$(BUILD)/obj/ejsrun.o" $(LIBS) -Wl,-r 

#
#   www
#
DEPS_289 += src/ejs.web/www/images/banner.jpg
DEPS_289 += src/ejs.web/www/images/favicon.ico
DEPS_289 += src/ejs.web/www/images/splash.jpg
DEPS_289 += src/ejs.web/www/js/jquery.ejs.min.js
DEPS_289 += src/ejs.web/www/js/jquery.min.js
DEPS_289 += src/ejs.web/www/js/jquery.simplemodal.min.js
DEPS_289 += src/ejs.web/www/js/jquery.tablesorter.js
DEPS_289 += src/ejs.web/www/js/jquery.tablesorter.min.js
DEPS_289 += src/ejs.web/www/js/jquery.treeview.min.js
DEPS_289 += src/ejs.web/www/js/tree-images/file.gif
DEPS_289 += src/ejs.web/www/js/tree-images/folder-closed.gif
DEPS_289 += src/ejs.web/www/js/tree-images/folder.gif
DEPS_289 += src/ejs.web/www/js/tree-images/minus.gif
DEPS_289 += src/ejs.web/www/js/tree-images/plus.gif
DEPS_289 += src/ejs.web/www/js/tree-images/treeview-black-line.gif
DEPS_289 += src/ejs.web/www/js/tree-images/treeview-black.gif
DEPS_289 += src/ejs.web/www/js/tree-images/treeview-default-line.gif
DEPS_289 += src/ejs.web/www/js/tree-images/treeview-default.gif
DEPS_289 += src/ejs.web/www/js/tree-images/treeview-famfamfam-line.gif
DEPS_289 += src/ejs.web/www/js/tree-images/treeview-famfamfam.gif
DEPS_289 += src/ejs.web/www/js/tree-images/treeview-gray-line.gif
DEPS_289 += src/ejs.web/www/js/tree-images/treeview-gray.gif
DEPS_289 += src/ejs.web/www/js/tree-images/treeview-red-line.gif
DEPS_289 += src/ejs.web/www/js/tree-images/treeview-red.gif
DEPS_289 += src/ejs.web/www/js/treeview.css
DEPS_289 += src/ejs.web/www/layout.css
DEPS_289 += src/ejs.web/www/themes/default.css

$(BUILD)/bin/www: $(DEPS_289)
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

installPrep: $(DEPS_290)
	if [ "`id -u`" != 0 ] ; \
	then echo "Must run as root. Rerun with "sudo"" ; \
	exit 255 ; \
	fi

#
#   stop
#

stop: $(DEPS_291)

#
#   installBinary
#

installBinary: $(DEPS_292)

#
#   start
#

start: $(DEPS_293)

#
#   install
#
DEPS_294 += installPrep
DEPS_294 += stop
DEPS_294 += installBinary
DEPS_294 += start

install: $(DEPS_294)

#
#   uninstall
#
DEPS_295 += stop

uninstall: $(DEPS_295)

#
#   version
#

version: $(DEPS_296)
	echo 2.5.2

