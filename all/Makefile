#
#	Makefile to build the all-in-one Ejscript distribution
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 		.makedep

COMPILE			=
JDIR			= ../src/jems
TARGETS			+= ejs.h ejs.slots.h ejsLib.c ejs.c ejsc.c ejsmod.c ejssql.c sqlite3.c sqlite3.h ejs.es
EJS_HEADERS		+= $(patsubst %,../src/include/%.h, ejsTune ejsByteCode ejsByteCodeTable ejsCore ejsModule ejs ejsWeb)
SLOT_HEADERS	+= $(shell find ../src/include -name 'ejs*slots.h')
EC_HEADERS		+= $(patsubst %,../src/include/%.h, ecCompiler)
EJS_SOURCES		+= $(shell find ../src/vm ../src/core -name '*.c') 
EC_SOURCES		+= $(shell find ../src/compiler -name '*.c') 
ES_SOURCES		+= $(shell find ../src/core ../src/jems/ejs.cjs \
				    ../src/jems/ejs.unix ../src/jems/ejs.db ../src/jems/ejs.db.mapper \
					../src/jems/ejs.db.sqlite ../src/jems/ejs.web ../src/jems/ejs.web.template -name '*.es' | \
					egrep -v 'SAVE|save|legacy|Archive|old|sample.es|XmlGlobal.es|ejsmvc.es|Jsdb.es|Mysql.es' | \
					egrep -v 'ejspage.es|\/test\/') 
EJSMOD_HEADERS	+= ../src/cmd/ejsmod.h
EJSMOD_SOURCES	+= $(patsubst %,../src/cmd/%.c, doc docFiles ejsmod listing slotGen)

DB_SOURCES		+= ../src/jems/ejs.db.sqlite/src/ejsSqlite.c
SQLITE_HEADERS	+= ../src/include/sqlite3.h
SQLITE_SOURCES	+= ../src/jems/ejs.db.sqlite/src/sqlite3.c
WEB_SOURCES		+= $(shell find ../src/jems/ejs.web -name '*.c' | egrep -v 'SAVE|save|legacy|Archive')

compileExtra:		$(TARGETS) dummy

ejs.h: $(EJS_HEADERS)
	@$(call log) "[Generate]" all/ejs.h
	@all-in-one $(EJS_HEADERS) $(EC_HEADERS) | egrep -v '#include.*ejs|#include.*ec|#include.*sql' >ejs.h
	@echo '#include "ejs.slots.h"' >>ejs.h

ejs.slots.h: $(SLOT_HEADERS)
	@$(call log) "[Generate]" all/ejs.slots.h
	@all-in-one $(SLOT_HEADERS) >ejs.slots.h

ejsLib.c: $(EJS_SOURCES) $(DB_SOURCES) $(WEB_SOURCES)
	@$(call log) "[Generate]" all/ejsLib.c
	echo '#define EJS_DEFINE_OPTABLE 1' >ejsLib.c
	echo '#include "ejs.h"' >>ejsLib.c
	all-in-one $(EJS_SOURCES) $(DB_SOURCES) $(WEB_SOURCES) $(EC_SOURCES) | \
        csplit -s -f tmpEjsLib. - '/#include.*ejsByteGoto.h/'
	(cat tmpEjsLib.00 ; cat ../src/include/ejsByteGoto.h ; \
	 if [ "$(BLD_OS)" = MACOSX ] ; then tail +2 tmpEjsLib.01 ; else tail --lines=+2 tmpEjsLib.01 ; fi ) | \
	 egrep -v '#inc.*ejs|#inc.*ec|#inc.*sqlConf' >>ejsLib.c
	@rm -f tmpEjsLib.*

ejsc.c: $(EJS_HEADERS) $(EC_HEADERS) ../src/cmd/ejsc.c
	@$(call log) "[Generate]" all/ejsc.c
	echo '#include "ejs.h"' >ejsc.c
	all-in-one ../src/cmd/ejsc.c | egrep -v '#inc.*ejs|#inc.*ec|#inc.*sql' >>ejsc.c

ejs.c: $(EJS_HEADERS) ../src/cmd/ejs.c
	@$(call log) "[Generate]" all/ejsc.c
	echo '#include "ejs.h"' >ejs.c
	all-in-one ../src/cmd/ejs.c | egrep -v '#inc.*ejs|#inc.*ec|#inc.*sql' >>ejs.c

ejsmod.c: $(EJSMOD_HEADERS) $(EJSMOD_SOURCES)
	@$(call log) "[Generate]" all/ejsmod.c
	echo '#include "ejs.h"' >ejsmod.c
	all-in-one $(EJSMOD_HEADERS) $(EJSMOD_SOURCES) | egrep -v '#include.*ejs|#include.*ec|#include.*sql' >>ejsmod.c

ejssql.c: ../src/jems/ejs.db.sqlite/src/ejssql.c
	@$(call log) "[Generate]" all/ejssql.c
	all-in-one $(SQLITE_HEADERS) ../src/jems/ejs.db.sqlite/src/ejssql.c | \
		egrep -v '#include.*ejs|#include.*sqlite3' >ejssql.c

sqlite3.c: $(SQLITE_SOURCES) $(SQLITE_HEADERS)
	@$(call log) "[Generate]" all/sqlite3.c
	all-in-one $(SQLITE_HEADERS) $(SQLITE_SOURCES) | egrep -v '#include.*ejs|#include.*sqlite3' >sqlite3.c

sqlite3.h: $(BLD_INC_DIR)/sqlite3.h
	@$(call log) "[Generate]" all/sqlite3.h
	cp $(BLD_INC_DIR)/sqlite3.h sqlite3.h

ejs.es: $(ES_SOURCES)
	@$(call log) "[Generate]" all/ejs.es
	all-in-one $(ES_SOURCES)  | egrep -v 'use strict' >ejs.es

clobberExtra:
	rm -f *.es *.c *.h
