#
#	Makefile for the ejs Windows Visual Studio projects
#
#	Copyright (c) Embedthis Software LLC, 2003-2009. All Rights Reserved.
#

CONFIG		= buildConfig.default

include     .makedep
include     $(CONFIG)

VSP			= makeVsProj
VSS			= makeVsSol
SRC			= $(BLD_TOP)/src
CMD			= $(SRC)/cmd
DEPS		= $(SRC)/deps
JEMS		= $(SRC)/jems
OUT			= $(BLD_OUT_DIR)

MPR_HDR 	= $(OUT)/inc/buildConfig.h $(shell find $(OUT)/inc -name 'mpr*.h')
HTTP_HDR 	= $(shell find $(OUT)/inc -name 'http*.h')
MPR_SRC		= $(SRC)/deps/mpr/mprLib.c
LIBS		= ws2_32.lib
SSL_IFLAGS	+= $(BLD_OPENSSL_IFLAGS)
PHP_IFLAGS	+= $(BLD_PHP_IFLAGS)

PROJECTS	= setup.vcxproj libmpr.vcxproj libsqlite3.vcxproj libmprssl.vcxproj libpcre.vcxproj libhttp.vcxproj \
			  libejs.vcxproj ejsmod.vcxproj ejsc.vcxproj ejs.vcxproj sqlite.vcxproj ejs.web.vcxproj \
			  ejs.db.sqlite.vcxproj http.vcxproj
SOLUTIONS	= ejs-all.sln
PROPS		+= product.props

#
#	Don't want to do this every build, just on demand
#
gen: prep $(PROJECTS) $(SOLUTIONS)

prep:
	touch Makefile
	if ls $(BLD_BIN_DIR)/*.def >/dev/null 2>&1 ; then \
		for f in $(BLD_BIN_DIR)/*.def ; do \
			base=`basename "$$f"` ; \
			cp $$f $$.tmp ; cat $$.tmp | grep -v mask@@NegDouble >"$$base" ; rm -f $$.tmp ; \
		done ; \
	fi

ejs-all.sln: Makefile
	$(VSS) --solution ejs-all.sln \
		--deps "libmpr setup" \
		--deps "libpcre setup" \
		--deps "ejsmod ejsc" \
		--deps "ejs ejsc ejsmod" \
		--deps "ejs.web ejsc" \
		--deps "ejs.db.sqlite ejsc" \
		$(PROJECTS)

setup.vcxproj: setup.custom Makefile
	$(VSP) --project setup.vcxproj --props "$(PROPS)" --custom setup.custom

libmpr.vcxproj: libmpr.custom Makefile
	$(VSP) --project libmpr.vcxproj --library mpr --dflags "$(DFLAGS)" \
		--props "$(PROPS)" \
		--custom libmpr.custom \
		--def libmpr.def --libs "$(LIBS)" $(MPR_HDR) $(MPR_SRC)
	
libmprssl.vcxproj: libmprssl.custom Makefile
	$(VSP) --project libmprssl.vcxproj --library mprssl --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps libmpr \
		--custom libmprssl.custom \
		--iflags "$(SSL_IFLAGS)" --def libmprssl.def --libs "libmpr.lib $(LIBS)" \
		$(MPR_HDR) $(SRC)/deps/mpr/mprSsl.c
	
libpcre.vcxproj: libpcre.custom Makefile
	$(VSP) --project libpcre.vcxproj --library pcre --dflags "$(DFLAGS)" \
		--props "$(PROPS)" \
		--custom libpcre.custom \
		--def libpcre.def --libs "$(LIBS)" $(SRC)/deps/pcre/pcre.c
	
libhttp.vcxproj: libhttp.custom Makefile
	$(VSP) --project libhttp.vcxproj --library http --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libmpr libmprssl libpcre" \
		--custom libhttp.custom \
		--def libhttp.def --libs "libmpr.lib libmprssl.lib $(LIBS)" $(HTTP_HDR) $(SRC)/deps/http/httpLib.c
	
libsqlite3.vcxproj: Makefile
	$(VSP) --project libsqlite3.vcxproj --library sqlite3 --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libmpr" \
		--def libsqlite3.def --libs "libmpr.lib $(LIBS)" $(SRC)/deps/sqlite/sqlite3.c
	
libejs.vcxproj: Makefile
	$(VSP) --project libejs.vcxproj --library ejs --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libmpr libmprssl libhttp libsqlite3 libpcre" \
		--custom libejs.custom \
		--def libejs.def --libs "libmpr.lib libhttp.lib libsqlite3.lib $(LIBS)" \
	    $(shell find $(SRC)/include -name 'ejs*.h') \
	    $(shell find $(SRC)/core $(SRC)/vm $(SRC)/compiler -name '*.c')
	
ejsc.vcxproj: Makefile ejsc.custom
	$(VSP) --project ejsc.vcxproj --exe ejsc.exe --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libmpr libmprssl libejs" \
		--custom ejsc.custom --libs "$(LIBS) libejs.lib libmpr.lib" $(SRC)/cmd/ejsc.c

ejsmod.vcxproj: Makefile
	$(VSP) --project ejsmod.vcxproj --exe ejsmod.exe --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libmpr libmprssl libejs" \
		--libs "$(LIBS) libejs.lib libmpr.lib" \
	    $(CMD)/ejsmod.c $(CMD)/doc.c $(CMD)/docFiles.c $(CMD)/listing.c $(CMD)/slotGen.c

ejs.db.sqlite.vcxproj: Makefile
	$(VSP) --project ejs.db.sqlite.vcxproj --library ec --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libmpr libmprssl libhttp libejs libsqlite3" \
		--def ejs.db.sqlite.def --libs "libmpr.lib libhttp.lib libejs.lib libsqlite3.lib $(LIBS)" \
	    $(JEMS)/ejs.db.sqlite/src/ejsSqlite.c

ejs.web.vcxproj: Makefile
	$(VSP) --project ejs.web.vcxproj --library ec --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libmpr libmprssl libhttp libejs" \
		--def ejs.web.def --libs "libmpr.lib libhttp.lib libejs.lib $(LIBS)" \
	    $(shell find $(JEMS)/ejs.web/src/ -name '*.c')

ejs.vcxproj: Makefile
	$(VSP) --project ejs.vcxproj --exe ejs.exe --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libmpr libmprssl libejs" \
		--libs "$(LIBS) libejs.lib libmpr.lib" $(SRC)/cmd/ejs.c

sqlite.vcxproj: Makefile
	$(VSP) --project sqlite.vcxproj --exe sqlite.exe --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libsqlite3" \
		--libs "$(LIBS) libsqlite3.lib" $(SRC)/deps/sqlite/sqlite.c

http.vcxproj: Makefile
	$(VSP) --project http.vcxproj --exe http.exe --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libmpr libmprssl libhttp" \
		--libs "$(LIBS) libhttp.lib libmpr.lib" $(SRC)/deps/http/http.c

cleanExtra:
	rm -fr Upgrade* _Upgrade* x64 Win32 *.suo *.sdf *.filters

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
