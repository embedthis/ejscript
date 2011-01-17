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
MPR_HDR 	= $(BLD_INC_DIR)/buildConfig.h $(shell find $(SRC)/include -name 'mpr*.h')
LIBS		= ws2_32.lib
PROJECTS	= libmpr.vcproj libsqlite3.vcproj libmprssl.vcproj libpcre.vcproj libhttp.vcproj libejs.vcproj \
			  ejsmod.vcproj ejsc.vcproj ejs.vcproj ejssql.vcproj ejs.web.vcproj ejs.db.sqlite.vcproj ejs.web.vcproj

MAKE_IFLAGS	+= $(BLD_MPR_IFLAGS) -I$(BLD_TOP)/src/include/master $(BLD_PCRE_IFLAGS) $(BLD_HTTP_IFLAGS)
SSL_IFLAGS	+= $(BLD_OPENSSL_IFLAGS)
SSL_IFLAGS	+= $(BLD_MATRIXSSL_IFLAGS)

compileExtra: prep $(PROJECTS)

prep:
	if ls $(BLD_BIN_DIR)/*.def >/dev/null 2>&1 ; then \
		cp $(BLD_BIN_DIR)/*.def . ; \
	fi

libmpr.vcproj: Makefile buildConfig.override
	makeVsProj --project libmpr.vcproj --library mpr --dflags "$(DFLAGS)" \
		--def libmpr.def --libs "$(LIBS)" $(MPR_HDR) $(DEPS)/mpr/mprLib.c
	
libmprssl.vcproj: Makefile buildConfig.override
	makeVsProj --project libmprssl.vcproj --library mprSsl --dflags "$(DFLAGS)" \
		--iflags "$(SSL_IFLAGS)" --def libmprssl.def --libs "libmpr.lib $(LIBS)" \
	    $(MPR_HDR) $(DEPS)/mpr/mprSsl.c
	
libpcre.vcproj: Makefile buildConfig.override
	makeVsProj --project libpcre.vcproj --library pcre --dflags "$(DFLAGS)" \
		--def libpcre.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(DEPS)/pcre/pcre.c
	
libhttp.vcproj: Makefile buildConfig.override
	makeVsProj --project libhttp.vcproj --library http --dflags "$(DFLAGS)" \
		--def libhttp.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(DEPS)/http/httpLib.c
	
libsqlite3.vcproj: Makefile buildConfig.override
	makeVsProj --project libsqlite3.vcproj --library sqlite3 --dflags "$(DFLAGS)" \
		--def libsqlite3.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(JEMS)/ejs.db.sqlite/src/sqlite3.c
	
libejs.vcproj: Makefile buildConfig.override
	makeVsProj --project libejs.vcproj --library ejs --dflags "$(DFLAGS)" \
		--def libejs.def --libs "libmpr.lib libhttp.lib libsqlite3.lib libpcre.def $(LIBS)" $(MPR_HDR) \
	    $(shell find $(SRC)/include -name 'ejs*.h') \
	    $(shell find $(SRC)/core $(SRC)/vm $(SRC)/compiler -name '*.c')
	
ejsc.vcproj: Makefile buildConfig.override
	makeVsProj --project ejsc.vcproj --exe ejsc.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(DFLAGS)" \
		--custom ejsc.custom --libs "$(LIBS) libejs.lib libmpr.lib" $(MPR_HDR) $(SRC)/cmd/ejsc.c

ejsmod.vcproj: Makefile buildConfig.override
	makeVsProj --project ejsmod.vcproj --exe ejsmod.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "$(LIBS) libejs.lib libmpr.lib" $(MPR_HDR) \
	    $(CMD)/ejsmod.c $(CMD)/doc.c $(CMD)/docFiles.c $(CMD)/listing.c $(CMD)/slotGen.c

ejs.vcproj: Makefile buildConfig.override
	makeVsProj --project ejs.vcproj --exe ejs.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "$(LIBS) libejs.lib libmpr.lib" $(MPR_HDR) $(SRC)/cmd/ejs.c

ejssql.vcproj: Makefile buildConfig.override
	makeVsProj --project ejssql.vcproj --exe ejssql.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "$(LIBS) libsqlite3.lib" $(MPR_HDR) $(JEMS)/ejs.db.sqlite/src/ejssql.c

ejs.db.sqlite.vcproj: Makefile buildConfig.override
	makeVsProj --project ejs.db.sqlite.vcproj --library ec --dflags "$(DFLAGS)" \
		--def ejs.db.sqlite.def --libs "libmpr.lib libhttp.lib libejs.lib libsqlite3.lib $(LIBS)" $(MPR_HDR) \
	    $(JEMS)/ejs.db.sqlite/src/ejsSqlite.c

ejs.web.vcproj: Makefile buildConfig.override
	makeVsProj --project ejs.web.vcproj --library ec --dflags "$(DFLAGS)" \
		--def ejs.web.def --libs "libmpr.lib libhttp.lib libejs.lib $(LIBS)" $(MPR_HDR) \
	    $(shell find $(JEMS)/ejs.web/src/ -name '*.c')

cleanExtra:
	rm -fr Upgrade* _Upgrade* x64 Win32

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
