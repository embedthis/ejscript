#
#	Makefile for Windows Visual Studio projects
#
#	Copyright (c) Embedthis Software LLC, 2003-2009. All Rights Reserved.
#

include     .makedep
include     buildConfig.override

SRC			= ../../../src
CMD			= ../../../src/cmd
DEPS		= ../../../src/deps
JEMS		= ../../../src/jems
MPR_HDR 	= $(BLD_TOP)/buildConfig.h $(shell find $(SRC)/include -name 'mpr*.h')
LIBS		= ws2_32.lib
PROJECTS	= libmpr.vcproj libsqlite3.vcproj libmprssl.vcproj libpcre.vcproj libhttp.vcproj libejs.vcproj \
			  ejsweb.vcproj ejsmod.vcproj ejsc.vcproj ejs.vcproj ejssql.vcproj \
	          ejs.web.vcproj ejs.db.sqlite.vcproj ejs.io.vcproj ejs.sys.vcproj ejs.web.vcproj ejs.xml.vcproj

MAKE_IFLAGS += $(BLD_REGEXP_IFLAGS) $(BLD_MPR_IFLAGS)
ifeq ($(BLD_FEATURE_OPENSSL),1)
	SSL_IFLAGS	+= $(BLD_OPENSSL_IFLAGS)
endif
ifeq ($(BLD_FEATURE_MATRIXSSL),1)
	SSL_IFLAGS	+= $(BLD_MATRIXSSL_IFLAGS)
endif

compileExtra: prep $(PROJECTS)

prep:
	cp ../../../bin/*.def .

libmpr.vcproj: Makefile buildConfig.override
	@makeVsProj --project libmpr.vcproj --library mpr --dflags "$(DFLAGS)" \
		--def libmpr.def --libs "$(LIBS)" $(MPR_HDR) $(DEPS)/mpr/mprLib.c
	
libmprssl.vcproj: Makefile buildConfig.override
	@makeVsProj --project libmprssl.vcproj --library mprSsl --dflags "$(DFLAGS)" \
		--iflags "$(SSL_IFLAGS)" --def libmprssl.def --libs "libmpr.lib $(LIBS)" \
	    $(MPR_HDR) $(DEPS)/mpr/mprSsl.c
	
libpcre.vcproj: Makefile buildConfig.override
	@makeVsProj --project libpcre.vcproj --library pcre --dflags "$(DFLAGS)" \
		--def libpcre.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(DEPS)/pcre/pcre.c
	
libhttp.vcproj: Makefile buildConfig.override
	@makeVsProj --project libhttp.vcproj --library http --dflags "$(DFLAGS)" \
		--def libhttp.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(DEPS)/http/httpLib.c
	
libsqlite3.vcproj: Makefile buildConfig.override
	@makeVsProj --project libsqlite3.vcproj --library sqlite3 --dflags "$(DFLAGS)" \
		--def libsqlite3.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(JEMS)/ejs.db.sqlite/src/sqlite3.c
	
libejs.vcproj: Makefile buildConfig.override
	@makeVsProj --project libejs.vcproj --library ejs --dflags "$(DFLAGS)" \
		--def libejs.def --libs "libmpr.lib libhttp.lib libsqlite3.lib $(LIBS)" $(MPR_HDR) \
	    $(shell find $(SRC)/include -name 'ejs*.h') \
	    $(shell find $(JEMS)/ejs/src $(SRC)/vm $(SRC)/compiler)
	
ejsc.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejsc.vcproj --exe ejsc.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(DFLAGS)" \
		--custom ejsc.custom --libs "$(LIBS) libejs.lib libmpr.lib" $(MPR_HDR) $(SRC)/cmd/ejsc.c

ejsmod.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejsmod.vcproj --exe ejsmod.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "$(LIBS) libejs.lib libmpr.lib" $(MPR_HDR) \
	    $(CMD)/ejsmod.c $(CMD)/doc.c $(CMD)/docFiles.c $(CMD)/listing.c $(CMD)/slotGen.c

ejs.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejs.vcproj --exe ejs.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "$(LIBS) libejs.lib libmpr.lib" $(MPR_HDR) $(SRC)/cmd/ejs.c

ejssql.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejssql.vcproj --exe ejssql.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "$(LIBS) libsqlite3.lib" $(MPR_HDR) $(JEMS)/ejs.db.sqlite/src/ejssql.c

ejsweb.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejsweb.vcproj --exe ejsweb.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "$(LIBS) libmpr.lib libhttp.lib libejs.lib ejs.web.lib" \
        $(MPR_HDR) $(DEPS)/appweb/appweb.c $(DEPS)/appweb/appwebLib.c

ejs.db.sqlite.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejs.db.sqlite.vcproj --library ec --dflags "$(DFLAGS)" \
		--def ejs.db.sqlite.def --libs "libmpr.lib libhttp.lib libejs.lib libsqlite3.lib $(LIBS)" $(MPR_HDR) \
	    $(JEMS)/ejs.db.sqlite/src/ejsSqlite.c

ejs.io.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejs.io.vcproj --library ec --dflags "$(DFLAGS)" \
		--def ejs.io.def --libs "libmpr.lib libhttp.lib libejs.lib $(LIBS)" $(MPR_HDR) \
	    $(shell find $(JEMS)/ejs.io/src/ -name '*.c')

ejs.sys.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejs.sys.vcproj --library ec --dflags "$(DFLAGS)" \
		--def ejs.sys.def --libs "libmpr.lib libhttp.lib libejs.lib $(LIBS)" $(MPR_HDR) \
	    $(shell find $(JEMS)/ejs.sys/src/ -name '*.c')

ejs.web.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejs.web.vcproj --library ec --dflags "$(DFLAGS)" \
		--def ejs.web.def --libs "libmpr.lib libhttp.lib libejs.lib $(LIBS)" $(MPR_HDR) \
	    $(shell find $(JEMS)/ejs.web/src/ -name '*.c')

ejs.xml.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejs.xml.vcproj --library ec --dflags "$(DFLAGS)" \
		--def ejs.xml.def --libs "libmpr.lib libhttp.lib libejs.lib $(LIBS)" $(MPR_HDR) \
	    $(shell find $(JEMS)/ejs.xml/src/ -name '*.c')

