#
#   build.mk -- Build It Makefile to build Embedthis Ejscript for win on i686
#

PLATFORM  := win-i686-debug
CC        := cl
CFLAGS    := -nologo -GR- -W3 -Zi -Od -MDd
DFLAGS    := -D_REENTRANT -D_MT
IFLAGS    := -I$(PLATFORM)/inc
LDFLAGS   := -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86
LIBS      := ws2_32.lib advapi32.lib user32.lib kernel32.lib oldnames.lib msvcrt.lib

export PATH := %VS%/Bin:%VS%/VC/Bin:%VS%/Common7/IDE:%VS%/Common7/Tools:%VS%/SDK/v3.5/bin:%VS%/VC/VCPackages
export INCLUDE := %VS%/INCLUDE:%VS%/VC/INCLUDE
export LIB := %VS%/lib:%VS%/VC/lib
all: prep \
        $(PLATFORM)/bin/libmpr.dll \
        $(PLATFORM)/bin/makerom.exe \
        $(PLATFORM)/bin/libpcre.dll \
        $(PLATFORM)/bin/libhttp.dll \
        $(PLATFORM)/bin/http.exe \
        $(PLATFORM)/bin/libsqlite3.dll \
        $(PLATFORM)/bin/libejs.dll \
        $(PLATFORM)/bin/ejs.exe \
        $(PLATFORM)/bin/ejsc.exe \
        $(PLATFORM)/bin/ejsmod.exe \
        $(PLATFORM)/bin/ejsrun.exe \
        $(PLATFORM)/bin/ejs.mod \
        $(PLATFORM)/bin/bit.es \
        $(PLATFORM)/bin/bit.exe \
        $(PLATFORM)/bin/utest.es \
        $(PLATFORM)/bin/utest.exe \
        $(PLATFORM)/bin/bits \
        $(PLATFORM)/bin/ejs.unix.mod \
        $(PLATFORM)/bin/jem.es \
        $(PLATFORM)/bin/jem.exe \
        $(PLATFORM)/bin/ejs.db.mod \
        $(PLATFORM)/bin/ejs.db.mapper.mod \
        $(PLATFORM)/bin/ejs.db.sqlite.mod \
        $(PLATFORM)/bin/ejs.db.sqlite.dll \
        $(PLATFORM)/bin/ejs.web.mod \
        $(PLATFORM)/bin/ejs.web.dll \
        $(PLATFORM)/bin/www \
        $(PLATFORM)/bin/ejs.template.mod \
        $(PLATFORM)/bin/ejs.tar.mod \
        $(PLATFORM)/bin/mvc.es \
        $(PLATFORM)/bin/mvc.exe \
        $(PLATFORM)/bin/ejs.mvc.mod \
        $(PLATFORM)/bin/utest.worker

.PHONY: prep

prep:
	@[ ! -x $(PLATFORM)/inc ] && mkdir -p $(PLATFORM)/inc $(PLATFORM)/obj $(PLATFORM)/lib $(PLATFORM)/bin ; true
	@[ ! -f $(PLATFORM)/inc/buildConfig.h ] && cp projects/buildConfig.$(PLATFORM) $(PLATFORM)/inc/buildConfig.h ; true

clean:
	rm -rf $(PLATFORM)/bin/libmpr.dll
	rm -rf $(PLATFORM)/bin/libmprssl.dll
	rm -rf $(PLATFORM)/bin/${settings.manager}
	rm -rf $(PLATFORM)/bin/makerom.exe
	rm -rf $(PLATFORM)/bin/libpcre.dll
	rm -rf $(PLATFORM)/bin/libhttp.dll
	rm -rf $(PLATFORM)/bin/http.exe
	rm -rf $(PLATFORM)/bin/libsqlite3.dll
	rm -rf $(PLATFORM)/bin/libejs.dll
	rm -rf $(PLATFORM)/bin/ejs.exe
	rm -rf $(PLATFORM)/bin/ejsc.exe
	rm -rf $(PLATFORM)/bin/ejsmod.exe
	rm -rf $(PLATFORM)/bin/ejsrun.exe
	rm -rf $(PLATFORM)/bin/ejs.mod
	rm -rf $(PLATFORM)/bin/bit.es
	rm -rf $(PLATFORM)/bin/bit.exe
	rm -rf $(PLATFORM)/bin/utest.es
	rm -rf $(PLATFORM)/bin/utest.exe
	rm -rf $(PLATFORM)/bin/bits
	rm -rf $(PLATFORM)/bin/ejs.unix.mod
	rm -rf $(PLATFORM)/bin/jem.es
	rm -rf $(PLATFORM)/bin/jem.exe
	rm -rf $(PLATFORM)/bin/ejs.db.mod
	rm -rf $(PLATFORM)/bin/ejs.db.mapper.mod
	rm -rf $(PLATFORM)/bin/ejs.db.sqlite.mod
	rm -rf $(PLATFORM)/bin/ejs.db.sqlite.dll
	rm -rf $(PLATFORM)/bin/ejs.web.mod
	rm -rf $(PLATFORM)/bin/ejs.web.dll
	rm -rf $(PLATFORM)/bin/www
	rm -rf $(PLATFORM)/bin/ejs.template.mod
	rm -rf $(PLATFORM)/bin/ejs.tar.mod
	rm -rf $(PLATFORM)/bin/ejs.zlib.mod
	rm -rf $(PLATFORM)/bin/ejs.zlib.dll
	rm -rf $(PLATFORM)/bin/mvc.es
	rm -rf $(PLATFORM)/bin/mvc.exe
	rm -rf $(PLATFORM)/bin/ejs.mvc.mod
	rm -rf $(PLATFORM)/bin/utest.worker
	rm -rf $(PLATFORM)/obj/mprLib.obj
	rm -rf $(PLATFORM)/obj/mprSsl.obj
	rm -rf $(PLATFORM)/obj/manager.obj
	rm -rf $(PLATFORM)/obj/makerom.obj
	rm -rf $(PLATFORM)/obj/pcre.obj
	rm -rf $(PLATFORM)/obj/httpLib.obj
	rm -rf $(PLATFORM)/obj/http.obj
	rm -rf $(PLATFORM)/obj/sqlite3.obj
	rm -rf $(PLATFORM)/obj/ecAst.obj
	rm -rf $(PLATFORM)/obj/ecCodeGen.obj
	rm -rf $(PLATFORM)/obj/ecCompiler.obj
	rm -rf $(PLATFORM)/obj/ecLex.obj
	rm -rf $(PLATFORM)/obj/ecModuleWrite.obj
	rm -rf $(PLATFORM)/obj/ecParser.obj
	rm -rf $(PLATFORM)/obj/ecState.obj
	rm -rf $(PLATFORM)/obj/ejsApp.obj
	rm -rf $(PLATFORM)/obj/ejsArray.obj
	rm -rf $(PLATFORM)/obj/ejsBlock.obj
	rm -rf $(PLATFORM)/obj/ejsBoolean.obj
	rm -rf $(PLATFORM)/obj/ejsByteArray.obj
	rm -rf $(PLATFORM)/obj/ejsCache.obj
	rm -rf $(PLATFORM)/obj/ejsCmd.obj
	rm -rf $(PLATFORM)/obj/ejsConfig.obj
	rm -rf $(PLATFORM)/obj/ejsDate.obj
	rm -rf $(PLATFORM)/obj/ejsDebug.obj
	rm -rf $(PLATFORM)/obj/ejsError.obj
	rm -rf $(PLATFORM)/obj/ejsFile.obj
	rm -rf $(PLATFORM)/obj/ejsFileSystem.obj
	rm -rf $(PLATFORM)/obj/ejsFrame.obj
	rm -rf $(PLATFORM)/obj/ejsFunction.obj
	rm -rf $(PLATFORM)/obj/ejsGC.obj
	rm -rf $(PLATFORM)/obj/ejsGlobal.obj
	rm -rf $(PLATFORM)/obj/ejsHttp.obj
	rm -rf $(PLATFORM)/obj/ejsIterator.obj
	rm -rf $(PLATFORM)/obj/ejsJSON.obj
	rm -rf $(PLATFORM)/obj/ejsLocalCache.obj
	rm -rf $(PLATFORM)/obj/ejsMath.obj
	rm -rf $(PLATFORM)/obj/ejsMemory.obj
	rm -rf $(PLATFORM)/obj/ejsMprLog.obj
	rm -rf $(PLATFORM)/obj/ejsNamespace.obj
	rm -rf $(PLATFORM)/obj/ejsNull.obj
	rm -rf $(PLATFORM)/obj/ejsNumber.obj
	rm -rf $(PLATFORM)/obj/ejsObject.obj
	rm -rf $(PLATFORM)/obj/ejsPath.obj
	rm -rf $(PLATFORM)/obj/ejsPot.obj
	rm -rf $(PLATFORM)/obj/ejsRegExp.obj
	rm -rf $(PLATFORM)/obj/ejsSocket.obj
	rm -rf $(PLATFORM)/obj/ejsString.obj
	rm -rf $(PLATFORM)/obj/ejsSystem.obj
	rm -rf $(PLATFORM)/obj/ejsTimer.obj
	rm -rf $(PLATFORM)/obj/ejsType.obj
	rm -rf $(PLATFORM)/obj/ejsUri.obj
	rm -rf $(PLATFORM)/obj/ejsVoid.obj
	rm -rf $(PLATFORM)/obj/ejsWorker.obj
	rm -rf $(PLATFORM)/obj/ejsXML.obj
	rm -rf $(PLATFORM)/obj/ejsXMLList.obj
	rm -rf $(PLATFORM)/obj/ejsXMLLoader.obj
	rm -rf $(PLATFORM)/obj/ejsByteCode.obj
	rm -rf $(PLATFORM)/obj/ejsException.obj
	rm -rf $(PLATFORM)/obj/ejsHelper.obj
	rm -rf $(PLATFORM)/obj/ejsInterp.obj
	rm -rf $(PLATFORM)/obj/ejsLoader.obj
	rm -rf $(PLATFORM)/obj/ejsModule.obj
	rm -rf $(PLATFORM)/obj/ejsScope.obj
	rm -rf $(PLATFORM)/obj/ejsService.obj
	rm -rf $(PLATFORM)/obj/ejs.obj
	rm -rf $(PLATFORM)/obj/ejsc.obj
	rm -rf $(PLATFORM)/obj/ejsmod.obj
	rm -rf $(PLATFORM)/obj/doc.obj
	rm -rf $(PLATFORM)/obj/docFiles.obj
	rm -rf $(PLATFORM)/obj/listing.obj
	rm -rf $(PLATFORM)/obj/slotGen.obj
	rm -rf $(PLATFORM)/obj/ejsrun.obj
	rm -rf $(PLATFORM)/obj/ejsSqlite.obj
	rm -rf $(PLATFORM)/obj/ejsHttpServer.obj
	rm -rf $(PLATFORM)/obj/ejsRequest.obj
	rm -rf $(PLATFORM)/obj/ejsSession.obj
	rm -rf $(PLATFORM)/obj/ejsWeb.obj
	rm -rf $(PLATFORM)/obj/ejsZlib.obj

$(PLATFORM)/inc/mpr.h: 
	rm -fr win-i686-debug/inc/mpr.h
	cp -r src/deps/mpr/mpr.h win-i686-debug/inc/mpr.h

$(PLATFORM)/inc/mprSsl.h: 
	rm -fr win-i686-debug/inc/mprSsl.h
	cp -r src/deps/mpr/mprSsl.h win-i686-debug/inc/mprSsl.h

$(PLATFORM)/obj/mprLib.obj: \
        src/deps/mpr/mprLib.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/mprLib.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/mpr/mprLib.c

$(PLATFORM)/bin/libmpr.dll:  \
        $(PLATFORM)/inc/mpr.h \
        $(PLATFORM)/inc/mprSsl.h \
        $(PLATFORM)/obj/mprLib.obj
	"link" -dll -out:$(PLATFORM)/bin/libmpr.dll -entry:_DllMainCRTStartup@12 -def:$(PLATFORM)/bin/libmpr.def -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/mprLib.obj $(LIBS)

$(PLATFORM)/obj/makerom.obj: \
        src/deps/mpr/makerom.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/makerom.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/mpr/makerom.c

$(PLATFORM)/bin/makerom.exe:  \
        $(PLATFORM)/bin/libmpr.dll \
        $(PLATFORM)/obj/makerom.obj
	"link" -out:$(PLATFORM)/bin/makerom.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/makerom.obj $(LIBS) mpr.lib

$(PLATFORM)/inc/pcre.h: 
	rm -fr win-i686-debug/inc/pcre.h
	cp -r src/deps/pcre/pcre.h win-i686-debug/inc/pcre.h

$(PLATFORM)/obj/pcre.obj: \
        src/deps/pcre/pcre.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/pcre.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/pcre/pcre.c

$(PLATFORM)/bin/libpcre.dll:  \
        $(PLATFORM)/inc/pcre.h \
        $(PLATFORM)/obj/pcre.obj
	"link" -dll -out:$(PLATFORM)/bin/libpcre.dll -entry:_DllMainCRTStartup@12 -def:$(PLATFORM)/bin/libpcre.def -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/pcre.obj $(LIBS)

$(PLATFORM)/inc/http.h: 
	rm -fr win-i686-debug/inc/http.h
	cp -r src/deps/http/http.h win-i686-debug/inc/http.h

$(PLATFORM)/obj/httpLib.obj: \
        src/deps/http/httpLib.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/httpLib.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/http/httpLib.c

$(PLATFORM)/bin/libhttp.dll:  \
        $(PLATFORM)/bin/libmpr.dll \
        $(PLATFORM)/bin/libpcre.dll \
        $(PLATFORM)/inc/http.h \
        $(PLATFORM)/obj/httpLib.obj
	"link" -dll -out:$(PLATFORM)/bin/libhttp.dll -entry:_DllMainCRTStartup@12 -def:$(PLATFORM)/bin/libhttp.def -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/httpLib.obj $(LIBS) mpr.lib pcre.lib

$(PLATFORM)/obj/http.obj: \
        src/deps/http/http.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/http.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/http/http.c

$(PLATFORM)/bin/http.exe:  \
        $(PLATFORM)/bin/libhttp.dll \
        $(PLATFORM)/obj/http.obj
	"link" -out:$(PLATFORM)/bin/http.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/http.obj $(LIBS) http.lib mpr.lib pcre.lib

$(PLATFORM)/inc/sqlite3.h: 
	rm -fr win-i686-debug/inc/sqlite3.h
	cp -r src/deps/sqlite/sqlite3.h win-i686-debug/inc/sqlite3.h

$(PLATFORM)/obj/sqlite3.obj: \
        src/deps/sqlite/sqlite3.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/sqlite3.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/sqlite/sqlite3.c

$(PLATFORM)/bin/libsqlite3.dll:  \
        $(PLATFORM)/inc/sqlite3.h \
        $(PLATFORM)/obj/sqlite3.obj
	"link" -dll -out:$(PLATFORM)/bin/libsqlite3.dll -entry:_DllMainCRTStartup@12 -def:$(PLATFORM)/bin/libsqlite3.def -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/sqlite3.obj $(LIBS)

$(PLATFORM)/inc/ejs.cache.local.slots.h: 
	rm -fr win-i686-debug/inc/ejs.cache.local.slots.h
	cp -r src/slots/ejs.cache.local.slots.h win-i686-debug/inc/ejs.cache.local.slots.h

$(PLATFORM)/inc/ejs.db.sqlite.slots.h: 
	rm -fr win-i686-debug/inc/ejs.db.sqlite.slots.h
	cp -r src/slots/ejs.db.sqlite.slots.h win-i686-debug/inc/ejs.db.sqlite.slots.h

$(PLATFORM)/inc/ejs.slots.h: 
	rm -fr win-i686-debug/inc/ejs.slots.h
	cp -r src/slots/ejs.slots.h win-i686-debug/inc/ejs.slots.h

$(PLATFORM)/inc/ejs.web.slots.h: 
	rm -fr win-i686-debug/inc/ejs.web.slots.h
	cp -r src/slots/ejs.web.slots.h win-i686-debug/inc/ejs.web.slots.h

$(PLATFORM)/inc/ejs.zlib.slots.h: 
	rm -fr win-i686-debug/inc/ejs.zlib.slots.h
	cp -r src/slots/ejs.zlib.slots.h win-i686-debug/inc/ejs.zlib.slots.h

$(PLATFORM)/inc/ejs.h: 
	rm -fr win-i686-debug/inc/ejs.h
	cp -r src/ejs.h win-i686-debug/inc/ejs.h

$(PLATFORM)/inc/ejsByteCode.h: 
	rm -fr win-i686-debug/inc/ejsByteCode.h
	cp -r src/ejsByteCode.h win-i686-debug/inc/ejsByteCode.h

$(PLATFORM)/inc/ejsByteCodeTable.h: 
	rm -fr win-i686-debug/inc/ejsByteCodeTable.h
	cp -r src/ejsByteCodeTable.h win-i686-debug/inc/ejsByteCodeTable.h

$(PLATFORM)/inc/ejsCompiler.h: 
	rm -fr win-i686-debug/inc/ejsCompiler.h
	cp -r src/ejsCompiler.h win-i686-debug/inc/ejsCompiler.h

$(PLATFORM)/inc/ejsCustomize.h: 
	rm -fr win-i686-debug/inc/ejsCustomize.h
	cp -r src/ejsCustomize.h win-i686-debug/inc/ejsCustomize.h

$(PLATFORM)/obj/ecAst.obj: \
        src/compiler/ecAst.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ecAst.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecAst.c

$(PLATFORM)/obj/ecCodeGen.obj: \
        src/compiler/ecCodeGen.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ecCodeGen.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecCodeGen.c

$(PLATFORM)/obj/ecCompiler.obj: \
        src/compiler/ecCompiler.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ecCompiler.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecCompiler.c

$(PLATFORM)/obj/ecLex.obj: \
        src/compiler/ecLex.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ecLex.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecLex.c

$(PLATFORM)/obj/ecModuleWrite.obj: \
        src/compiler/ecModuleWrite.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ecModuleWrite.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecModuleWrite.c

$(PLATFORM)/obj/ecParser.obj: \
        src/compiler/ecParser.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ecParser.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecParser.c

$(PLATFORM)/obj/ecState.obj: \
        src/compiler/ecState.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ecState.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/compiler/ecState.c

$(PLATFORM)/obj/ejsApp.obj: \
        src/core/src/ejsApp.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsApp.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsApp.c

$(PLATFORM)/obj/ejsArray.obj: \
        src/core/src/ejsArray.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsArray.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsArray.c

$(PLATFORM)/obj/ejsBlock.obj: \
        src/core/src/ejsBlock.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsBlock.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsBlock.c

$(PLATFORM)/obj/ejsBoolean.obj: \
        src/core/src/ejsBoolean.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsBoolean.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsBoolean.c

$(PLATFORM)/obj/ejsByteArray.obj: \
        src/core/src/ejsByteArray.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsByteArray.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsByteArray.c

$(PLATFORM)/obj/ejsCache.obj: \
        src/core/src/ejsCache.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsCache.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsCache.c

$(PLATFORM)/obj/ejsCmd.obj: \
        src/core/src/ejsCmd.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsCmd.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsCmd.c

$(PLATFORM)/obj/ejsConfig.obj: \
        src/core/src/ejsConfig.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsConfig.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsConfig.c

$(PLATFORM)/obj/ejsDate.obj: \
        src/core/src/ejsDate.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsDate.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsDate.c

$(PLATFORM)/obj/ejsDebug.obj: \
        src/core/src/ejsDebug.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsDebug.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsDebug.c

$(PLATFORM)/obj/ejsError.obj: \
        src/core/src/ejsError.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsError.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsError.c

$(PLATFORM)/obj/ejsFile.obj: \
        src/core/src/ejsFile.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsFile.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsFile.c

$(PLATFORM)/obj/ejsFileSystem.obj: \
        src/core/src/ejsFileSystem.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsFileSystem.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsFileSystem.c

$(PLATFORM)/obj/ejsFrame.obj: \
        src/core/src/ejsFrame.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsFrame.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsFrame.c

$(PLATFORM)/obj/ejsFunction.obj: \
        src/core/src/ejsFunction.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsFunction.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsFunction.c

$(PLATFORM)/obj/ejsGC.obj: \
        src/core/src/ejsGC.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsGC.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsGC.c

$(PLATFORM)/obj/ejsGlobal.obj: \
        src/core/src/ejsGlobal.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsGlobal.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsGlobal.c

$(PLATFORM)/obj/ejsHttp.obj: \
        src/core/src/ejsHttp.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsHttp.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsHttp.c

$(PLATFORM)/obj/ejsIterator.obj: \
        src/core/src/ejsIterator.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsIterator.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsIterator.c

$(PLATFORM)/obj/ejsJSON.obj: \
        src/core/src/ejsJSON.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsJSON.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsJSON.c

$(PLATFORM)/obj/ejsLocalCache.obj: \
        src/core/src/ejsLocalCache.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsLocalCache.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsLocalCache.c

$(PLATFORM)/obj/ejsMath.obj: \
        src/core/src/ejsMath.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsMath.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsMath.c

$(PLATFORM)/obj/ejsMemory.obj: \
        src/core/src/ejsMemory.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsMemory.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsMemory.c

$(PLATFORM)/obj/ejsMprLog.obj: \
        src/core/src/ejsMprLog.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsMprLog.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsMprLog.c

$(PLATFORM)/obj/ejsNamespace.obj: \
        src/core/src/ejsNamespace.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsNamespace.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsNamespace.c

$(PLATFORM)/obj/ejsNull.obj: \
        src/core/src/ejsNull.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsNull.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsNull.c

$(PLATFORM)/obj/ejsNumber.obj: \
        src/core/src/ejsNumber.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsNumber.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsNumber.c

$(PLATFORM)/obj/ejsObject.obj: \
        src/core/src/ejsObject.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsObject.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsObject.c

$(PLATFORM)/obj/ejsPath.obj: \
        src/core/src/ejsPath.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsPath.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsPath.c

$(PLATFORM)/obj/ejsPot.obj: \
        src/core/src/ejsPot.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsPot.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsPot.c

$(PLATFORM)/obj/ejsRegExp.obj: \
        src/core/src/ejsRegExp.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsRegExp.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsRegExp.c

$(PLATFORM)/obj/ejsSocket.obj: \
        src/core/src/ejsSocket.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsSocket.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsSocket.c

$(PLATFORM)/obj/ejsString.obj: \
        src/core/src/ejsString.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsString.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsString.c

$(PLATFORM)/obj/ejsSystem.obj: \
        src/core/src/ejsSystem.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsSystem.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsSystem.c

$(PLATFORM)/obj/ejsTimer.obj: \
        src/core/src/ejsTimer.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsTimer.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsTimer.c

$(PLATFORM)/obj/ejsType.obj: \
        src/core/src/ejsType.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsType.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsType.c

$(PLATFORM)/obj/ejsUri.obj: \
        src/core/src/ejsUri.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsUri.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsUri.c

$(PLATFORM)/obj/ejsVoid.obj: \
        src/core/src/ejsVoid.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsVoid.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsVoid.c

$(PLATFORM)/obj/ejsWorker.obj: \
        src/core/src/ejsWorker.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsWorker.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsWorker.c

$(PLATFORM)/obj/ejsXML.obj: \
        src/core/src/ejsXML.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsXML.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsXML.c

$(PLATFORM)/obj/ejsXMLList.obj: \
        src/core/src/ejsXMLList.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsXMLList.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsXMLList.c

$(PLATFORM)/obj/ejsXMLLoader.obj: \
        src/core/src/ejsXMLLoader.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsXMLLoader.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/core/src/ejsXMLLoader.c

$(PLATFORM)/obj/ejsByteCode.obj: \
        src/vm/ejsByteCode.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsByteCode.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsByteCode.c

$(PLATFORM)/obj/ejsException.obj: \
        src/vm/ejsException.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsException.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsException.c

$(PLATFORM)/obj/ejsHelper.obj: \
        src/vm/ejsHelper.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsHelper.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsHelper.c

$(PLATFORM)/obj/ejsInterp.obj: \
        src/vm/ejsInterp.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsInterp.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsInterp.c

$(PLATFORM)/obj/ejsLoader.obj: \
        src/vm/ejsLoader.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsLoader.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsLoader.c

$(PLATFORM)/obj/ejsModule.obj: \
        src/vm/ejsModule.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsModule.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsModule.c

$(PLATFORM)/obj/ejsScope.obj: \
        src/vm/ejsScope.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsScope.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsScope.c

$(PLATFORM)/obj/ejsService.obj: \
        src/vm/ejsService.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsService.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/vm/ejsService.c

$(PLATFORM)/bin/libejs.dll:  \
        $(PLATFORM)/bin/libmpr.dll \
        $(PLATFORM)/bin/libpcre.dll \
        $(PLATFORM)/bin/libhttp.dll \
        $(PLATFORM)/inc/ejs.cache.local.slots.h \
        $(PLATFORM)/inc/ejs.db.sqlite.slots.h \
        $(PLATFORM)/inc/ejs.slots.h \
        $(PLATFORM)/inc/ejs.web.slots.h \
        $(PLATFORM)/inc/ejs.zlib.slots.h \
        $(PLATFORM)/inc/ejs.h \
        $(PLATFORM)/inc/ejsByteCode.h \
        $(PLATFORM)/inc/ejsByteCodeTable.h \
        $(PLATFORM)/inc/ejsCompiler.h \
        $(PLATFORM)/inc/ejsCustomize.h \
        $(PLATFORM)/obj/ecAst.obj \
        $(PLATFORM)/obj/ecCodeGen.obj \
        $(PLATFORM)/obj/ecCompiler.obj \
        $(PLATFORM)/obj/ecLex.obj \
        $(PLATFORM)/obj/ecModuleWrite.obj \
        $(PLATFORM)/obj/ecParser.obj \
        $(PLATFORM)/obj/ecState.obj \
        $(PLATFORM)/obj/ejsApp.obj \
        $(PLATFORM)/obj/ejsArray.obj \
        $(PLATFORM)/obj/ejsBlock.obj \
        $(PLATFORM)/obj/ejsBoolean.obj \
        $(PLATFORM)/obj/ejsByteArray.obj \
        $(PLATFORM)/obj/ejsCache.obj \
        $(PLATFORM)/obj/ejsCmd.obj \
        $(PLATFORM)/obj/ejsConfig.obj \
        $(PLATFORM)/obj/ejsDate.obj \
        $(PLATFORM)/obj/ejsDebug.obj \
        $(PLATFORM)/obj/ejsError.obj \
        $(PLATFORM)/obj/ejsFile.obj \
        $(PLATFORM)/obj/ejsFileSystem.obj \
        $(PLATFORM)/obj/ejsFrame.obj \
        $(PLATFORM)/obj/ejsFunction.obj \
        $(PLATFORM)/obj/ejsGC.obj \
        $(PLATFORM)/obj/ejsGlobal.obj \
        $(PLATFORM)/obj/ejsHttp.obj \
        $(PLATFORM)/obj/ejsIterator.obj \
        $(PLATFORM)/obj/ejsJSON.obj \
        $(PLATFORM)/obj/ejsLocalCache.obj \
        $(PLATFORM)/obj/ejsMath.obj \
        $(PLATFORM)/obj/ejsMemory.obj \
        $(PLATFORM)/obj/ejsMprLog.obj \
        $(PLATFORM)/obj/ejsNamespace.obj \
        $(PLATFORM)/obj/ejsNull.obj \
        $(PLATFORM)/obj/ejsNumber.obj \
        $(PLATFORM)/obj/ejsObject.obj \
        $(PLATFORM)/obj/ejsPath.obj \
        $(PLATFORM)/obj/ejsPot.obj \
        $(PLATFORM)/obj/ejsRegExp.obj \
        $(PLATFORM)/obj/ejsSocket.obj \
        $(PLATFORM)/obj/ejsString.obj \
        $(PLATFORM)/obj/ejsSystem.obj \
        $(PLATFORM)/obj/ejsTimer.obj \
        $(PLATFORM)/obj/ejsType.obj \
        $(PLATFORM)/obj/ejsUri.obj \
        $(PLATFORM)/obj/ejsVoid.obj \
        $(PLATFORM)/obj/ejsWorker.obj \
        $(PLATFORM)/obj/ejsXML.obj \
        $(PLATFORM)/obj/ejsXMLList.obj \
        $(PLATFORM)/obj/ejsXMLLoader.obj \
        $(PLATFORM)/obj/ejsByteCode.obj \
        $(PLATFORM)/obj/ejsException.obj \
        $(PLATFORM)/obj/ejsHelper.obj \
        $(PLATFORM)/obj/ejsInterp.obj \
        $(PLATFORM)/obj/ejsLoader.obj \
        $(PLATFORM)/obj/ejsModule.obj \
        $(PLATFORM)/obj/ejsScope.obj \
        $(PLATFORM)/obj/ejsService.obj
	"link" -dll -out:$(PLATFORM)/bin/libejs.dll -entry:_DllMainCRTStartup@12 -def:$(PLATFORM)/bin/libejs.def -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/ecAst.obj $(PLATFORM)/obj/ecCodeGen.obj $(PLATFORM)/obj/ecCompiler.obj $(PLATFORM)/obj/ecLex.obj $(PLATFORM)/obj/ecModuleWrite.obj $(PLATFORM)/obj/ecParser.obj $(PLATFORM)/obj/ecState.obj $(PLATFORM)/obj/ejsApp.obj $(PLATFORM)/obj/ejsArray.obj $(PLATFORM)/obj/ejsBlock.obj $(PLATFORM)/obj/ejsBoolean.obj $(PLATFORM)/obj/ejsByteArray.obj $(PLATFORM)/obj/ejsCache.obj $(PLATFORM)/obj/ejsCmd.obj $(PLATFORM)/obj/ejsConfig.obj $(PLATFORM)/obj/ejsDate.obj $(PLATFORM)/obj/ejsDebug.obj $(PLATFORM)/obj/ejsError.obj $(PLATFORM)/obj/ejsFile.obj $(PLATFORM)/obj/ejsFileSystem.obj $(PLATFORM)/obj/ejsFrame.obj $(PLATFORM)/obj/ejsFunction.obj $(PLATFORM)/obj/ejsGC.obj $(PLATFORM)/obj/ejsGlobal.obj $(PLATFORM)/obj/ejsHttp.obj $(PLATFORM)/obj/ejsIterator.obj $(PLATFORM)/obj/ejsJSON.obj $(PLATFORM)/obj/ejsLocalCache.obj $(PLATFORM)/obj/ejsMath.obj $(PLATFORM)/obj/ejsMemory.obj $(PLATFORM)/obj/ejsMprLog.obj $(PLATFORM)/obj/ejsNamespace.obj $(PLATFORM)/obj/ejsNull.obj $(PLATFORM)/obj/ejsNumber.obj $(PLATFORM)/obj/ejsObject.obj $(PLATFORM)/obj/ejsPath.obj $(PLATFORM)/obj/ejsPot.obj $(PLATFORM)/obj/ejsRegExp.obj $(PLATFORM)/obj/ejsSocket.obj $(PLATFORM)/obj/ejsString.obj $(PLATFORM)/obj/ejsSystem.obj $(PLATFORM)/obj/ejsTimer.obj $(PLATFORM)/obj/ejsType.obj $(PLATFORM)/obj/ejsUri.obj $(PLATFORM)/obj/ejsVoid.obj $(PLATFORM)/obj/ejsWorker.obj $(PLATFORM)/obj/ejsXML.obj $(PLATFORM)/obj/ejsXMLList.obj $(PLATFORM)/obj/ejsXMLLoader.obj $(PLATFORM)/obj/ejsByteCode.obj $(PLATFORM)/obj/ejsException.obj $(PLATFORM)/obj/ejsHelper.obj $(PLATFORM)/obj/ejsInterp.obj $(PLATFORM)/obj/ejsLoader.obj $(PLATFORM)/obj/ejsModule.obj $(PLATFORM)/obj/ejsScope.obj $(PLATFORM)/obj/ejsService.obj $(LIBS) mpr.lib pcre.lib http.lib

$(PLATFORM)/obj/ejs.obj: \
        src/cmd/ejs.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejs.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/cmd/ejs.c

$(PLATFORM)/bin/ejs.exe:  \
        $(PLATFORM)/bin/libejs.dll \
        $(PLATFORM)/obj/ejs.obj
	"link" -out:$(PLATFORM)/bin/ejs.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/ejs.obj $(LIBS) ejs.lib mpr.lib pcre.lib http.lib

$(PLATFORM)/obj/ejsc.obj: \
        src/cmd/ejsc.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsc.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/cmd/ejsc.c

$(PLATFORM)/bin/ejsc.exe:  \
        $(PLATFORM)/bin/libejs.dll \
        $(PLATFORM)/obj/ejsc.obj
	"link" -out:$(PLATFORM)/bin/ejsc.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/ejsc.obj $(LIBS) ejs.lib mpr.lib pcre.lib http.lib

$(PLATFORM)/obj/ejsmod.obj: \
        src/cmd/ejsmod.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsmod.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/cmd src/cmd/ejsmod.c

$(PLATFORM)/obj/doc.obj: \
        src/cmd/doc.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/doc.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/cmd src/cmd/doc.c

$(PLATFORM)/obj/docFiles.obj: \
        src/cmd/docFiles.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/docFiles.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/cmd src/cmd/docFiles.c

$(PLATFORM)/obj/listing.obj: \
        src/cmd/listing.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/listing.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/cmd src/cmd/listing.c

$(PLATFORM)/obj/slotGen.obj: \
        src/cmd/slotGen.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/cmd/ejsmod.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/slotGen.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/cmd src/cmd/slotGen.c

$(PLATFORM)/bin/ejsmod.exe:  \
        $(PLATFORM)/bin/libejs.dll \
        $(PLATFORM)/obj/ejsmod.obj \
        $(PLATFORM)/obj/doc.obj \
        $(PLATFORM)/obj/docFiles.obj \
        $(PLATFORM)/obj/listing.obj \
        $(PLATFORM)/obj/slotGen.obj
	"link" -out:$(PLATFORM)/bin/ejsmod.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/ejsmod.obj $(PLATFORM)/obj/doc.obj $(PLATFORM)/obj/docFiles.obj $(PLATFORM)/obj/listing.obj $(PLATFORM)/obj/slotGen.obj $(LIBS) ejs.lib mpr.lib pcre.lib http.lib

$(PLATFORM)/obj/ejsrun.obj: \
        src/cmd/ejsrun.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsrun.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/cmd/ejsrun.c

$(PLATFORM)/bin/ejsrun.exe:  \
        $(PLATFORM)/bin/libejs.dll \
        $(PLATFORM)/obj/ejsrun.obj
	"link" -out:$(PLATFORM)/bin/ejsrun.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/ejsrun.obj $(LIBS) ejs.lib mpr.lib pcre.lib http.lib

win-i686-debug/bin/ejs.mod:  \
        $(PLATFORM)/bin/ejsc.exe \
        $(PLATFORM)/bin/ejsmod.exe
	ejsc --out $(PLATFORM)/bin/ejs.mod --debug --optimize 9 --bind --require null src/core/*.es 
	ejsmod --require null --listing --cslots $(PLATFORM)/bin/ejs.mod
	cp ejs.slots.h $(PLATFORM)/inc

win-i686-debug/bin/bit.es: 
	cp src/jems/ejs.bit/bit.es $(PLATFORM)/bin

win-i686-debug/bin/utest.es: 
	cp src/jems/ejs.utest/utest.es $(PLATFORM)/bin

win-i686-debug/bin/bits: 
	rm -fr $(PLATFORM)/bin/bits
	cp -r src/jems/ejs.bit/bits $(PLATFORM)/bin

win-i686-debug/bin/ejs.unix.mod:  \
        $(PLATFORM)/bin/ejsc.exe \
        $(PLATFORM)/bin/ejsmod.exe \
        $(PLATFORM)/bin/ejs.mod
	ejsc --out $(PLATFORM)/bin/ejs.unix.mod --debug --optimize 9 src/jems/ejs.unix/Unix.es

win-i686-debug/bin/jem.es: 
	cp src/jems/ejs.jem/jem.es $(PLATFORM)/bin

win-i686-debug/bin/ejs.db.mod:  \
        $(PLATFORM)/bin/ejsc.exe \
        $(PLATFORM)/bin/ejsmod.exe \
        $(PLATFORM)/bin/ejs.mod
	ejsc --out $(PLATFORM)/bin/ejs.db.mod --debug --optimize 9 src/jems/ejs.db/*.es

win-i686-debug/bin/ejs.db.mapper.mod:  \
        $(PLATFORM)/bin/ejsc.exe \
        $(PLATFORM)/bin/ejsmod.exe \
        $(PLATFORM)/bin/ejs.mod \
        $(PLATFORM)/bin/ejs.db.mod
	ejsc --out $(PLATFORM)/bin/ejs.db.mapper.mod --debug --optimize 9 src/jems/ejs.db.mapper/*.es

win-i686-debug/bin/ejs.db.sqlite.mod:  \
        $(PLATFORM)/bin/ejsc.exe \
        $(PLATFORM)/bin/ejsmod.exe \
        $(PLATFORM)/bin/ejs.mod
	ejsc --out $(PLATFORM)/bin/ejs.db.sqlite.mod --debug --optimize 9 src/jems/ejs.db.sqlite/*.es

$(PLATFORM)/obj/ejsSqlite.obj: \
        src/jems/ejs.db.sqlite/src/ejsSqlite.c \
        $(PLATFORM)/inc/buildConfig.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsSqlite.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/jems/ejs.db.sqlite/src/ejsSqlite.c

$(PLATFORM)/bin/ejs.db.sqlite.dll:  \
        $(PLATFORM)/bin/libmpr.dll \
        $(PLATFORM)/bin/libejs.dll \
        $(PLATFORM)/bin/ejs.mod \
        $(PLATFORM)/bin/ejs.db.sqlite.mod \
        $(PLATFORM)/bin/libsqlite3.dll \
        $(PLATFORM)/obj/ejsSqlite.obj
	"link" -dll -out:$(PLATFORM)/bin/ejs.db.sqlite.dll -entry:_DllMainCRTStartup@12 -def:$(PLATFORM)/bin/ejs.db.sqlite.def -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/ejsSqlite.obj $(LIBS) mpr.lib ejs.lib pcre.lib http.lib sqlite3.lib

win-i686-debug/bin/ejs.web.mod:  \
        $(PLATFORM)/bin/ejsc.exe \
        $(PLATFORM)/bin/ejsmod.exe \
        $(PLATFORM)/bin/ejs.mod
	ejsc --out $(PLATFORM)/bin/ejs.web.mod --debug --optimize 9 src/jems/ejs.web/*.es
	ejsmod --cslots $(PLATFORM)/bin/ejs.web.mod
	cp ejs.web.slots.h $(PLATFORM)/inc

$(PLATFORM)/obj/ejsHttpServer.obj: \
        src/jems/ejs.web/src/ejsHttpServer.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/jems/ejs.web/src/ejsWeb.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsHttpServer.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsHttpServer.c

$(PLATFORM)/obj/ejsRequest.obj: \
        src/jems/ejs.web/src/ejsRequest.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/jems/ejs.web/src/ejsWeb.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsRequest.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsRequest.c

$(PLATFORM)/obj/ejsSession.obj: \
        src/jems/ejs.web/src/ejsSession.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/jems/ejs.web/src/ejsWeb.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsSession.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsSession.c

$(PLATFORM)/obj/ejsWeb.obj: \
        src/jems/ejs.web/src/ejsWeb.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/jems/ejs.web/src/ejsWeb.h
	"$(CC)" -c -Fo$(PLATFORM)/obj/ejsWeb.obj -Fd$(PLATFORM)/obj $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/jems/ejs.web/src src/jems/ejs.web/src/ejsWeb.c

$(PLATFORM)/bin/ejs.web.dll:  \
        $(PLATFORM)/bin/libmpr.dll \
        $(PLATFORM)/bin/libhttp.dll \
        $(PLATFORM)/bin/libpcre.dll \
        $(PLATFORM)/bin/libejs.dll \
        $(PLATFORM)/bin/ejs.mod \
        $(PLATFORM)/obj/ejsHttpServer.obj \
        $(PLATFORM)/obj/ejsRequest.obj \
        $(PLATFORM)/obj/ejsSession.obj \
        $(PLATFORM)/obj/ejsWeb.obj
	"link" -dll -out:$(PLATFORM)/bin/ejs.web.dll -entry:_DllMainCRTStartup@12 -def:$(PLATFORM)/bin/ejs.web.def -nologo -nodefaultlib -incremental:no -libpath:$(PLATFORM)/bin -debug -machine:x86 $(PLATFORM)/obj/ejsHttpServer.obj $(PLATFORM)/obj/ejsRequest.obj $(PLATFORM)/obj/ejsSession.obj $(PLATFORM)/obj/ejsWeb.obj $(LIBS) mpr.lib http.lib pcre.lib pcre.lib ejs.lib

win-i686-debug/bin/www: 
	rm -fr $(PLATFORM)/bin/www
	cp -r src/jems/ejs.web/www $(PLATFORM)/bin

win-i686-debug/bin/ejs.template.mod:  \
        $(PLATFORM)/bin/ejsc.exe \
        $(PLATFORM)/bin/ejsmod.exe \
        $(PLATFORM)/bin/ejs.mod
	ejsc --out $(PLATFORM)/bin/ejs.template.mod --debug --optimize 9 src/jems/ejs.template/TemplateParser.es

win-i686-debug/bin/ejs.tar.mod:  \
        $(PLATFORM)/bin/ejsc.exe \
        $(PLATFORM)/bin/ejsmod.exe \
        $(PLATFORM)/bin/ejs.mod
	ejsc --out $(PLATFORM)/bin/ejs.tar.mod/ --debug --optimize 9 src/jems/ejs.tar/*.es

win-i686-debug/bin/mvc.es: 
	cp src/jems/ejs.mvc/mvc.es $(PLATFORM)/bin

win-i686-debug/bin/ejs.mvc.mod:  \
        $(PLATFORM)/bin/ejsc.exe \
        $(PLATFORM)/bin/ejsmod.exe \
        $(PLATFORM)/bin/ejs.mod \
        $(PLATFORM)/bin/ejs.web.mod \
        $(PLATFORM)/bin/ejs.template.mod \
        $(PLATFORM)/bin/ejs.unix.mod
	ejsc --out $(PLATFORM)/bin/ejs.mvc.mod/ --debug --optimize 9 src/jems/ejs.mvc/*.es

win-i686-debug/bin/utest.worker: 
	cp src/jems/ejs.utest/utest.worker $(PLATFORM)/bin

