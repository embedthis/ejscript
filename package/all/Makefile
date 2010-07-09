#
#	Makefile to build the all-in-one combined Ejscript distribution
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 		.makedep

SRC				:= ../../src
JEMS			:= $(SRC)/jems
STAGING			:= staging
DEST			:= $(STAGING)/src/deps/ejs
INC_DEST		:= $(STAGING)/src/include
DOC_DEST		:= $(STAGING)/doc
ARCHIVE			+= $(BLD_TOP)/releases/ejs-all.tgz
EJS_HEADERS		+= $(patsubst %,$(SRC)/include/%.h, ejsTune ejsByteCode ejsByteCodeTable ejsCore ejsModule ejs ejsWeb)
SLOT_HEADERS	+= $(shell find $(SRC)/include -name 'ejs*slots.h')
EC_HEADERS		+= $(patsubst %,$(SRC)/include/%.h, ecCompiler)
EJS_SOURCES		+= $(shell find $(SRC)/vm $(SRC)/core -name '*.c') 
EC_SOURCES		+= $(shell find $(SRC)/compiler -name '*.c') 
ES_SOURCES		+= $(shell find $(SRC)/core $(JEMS)/ejs.cjs \
				    $(JEMS)/ejs.unix $(JEMS)/ejs.db $(JEMS)/ejs.db.mapper \
					$(JEMS)/ejs.db.sqlite $(JEMS)/ejs.web $(JEMS)/ejs.web.template -name '*.es' | \
					egrep -v 'SAVE|save|legacy|Archive|old|sample.es|XmlGlobal.es|ejsmvc.es|Jsdb.es|Mysql.es|ejspage.es|\/test\/') 
EJSMOD_HEADERS	+= $(SRC)/cmd/ejsmod.h
EJSMOD_SOURCES	+= $(patsubst %,$(SRC)/cmd/%.c, doc docFiles ejsmod listing slotGen)
DB_SOURCES		+= $(JEMS)/ejs.db.sqlite/src/ejsSqlite.c
SQLITE_HEADERS	+= $(SRC)/include/sqlite3.h
SQLITE_SOURCES	+= $(JEMS)/ejs.db.sqlite/src/sqlite3.c
WEB_SOURCES		+= $(shell find $(JEMS)/ejs.web -name '*.c' | egrep -v 'SAVE|save|legacy|Archive')
TARGETS			+= $(patsubst %,$(INC_DEST)/%, ejs.h ejs.slots.h sqlite3.h)
TARGETS			+= $(patsubst %,$(DEST)/%, ejsLib.c ejs.c ejsc.c ejsmod.c ejssql.c sqlite3.c ejs.es)

combo packageExtra:	prep $(TARGETS) $(ARCHIVE)
	
prep:
	@$(call log) "[Prepare]" "All-in-one combo package"
	rm -fr $(STAGING)
	mkdir -p $(STAGING)/src/include $(STAGING)/src/deps/ejs $(STAGING)/doc/api $(STAGING)/doc/man

$(INC_DEST)/ejs.h: $(EJS_HEADERS)
	@all-in-one $(EJS_HEADERS) $(EC_HEADERS) | egrep -v '#include.*ejs|#include.*ec|#include.*sql' >$(INC_DEST)/ejs.h
	@echo '#include "ejs.slots.h"' >>$(INC_DEST)/ejs.h

$(INC_DEST)/ejs.slots.h: $(SLOT_HEADERS)
	@all-in-one $(SLOT_HEADERS) >$(INC_DEST)/ejs.slots.h

$(INC_DEST)/sqlite3.h: $(BLD_INC_DIR)/sqlite3.h
	cp $(BLD_INC_DIR)/sqlite3.h $(INC_DEST)/sqlite3.h

$(DEST)/ejsLib.c: $(EJS_SOURCES) $(DB_SOURCES) $(WEB_SOURCES)
	echo '#define EJS_DEFINE_OPTABLE 1' >$(DEST)/ejsLib.c
	echo '#include "ejs.h"' >>$(DEST)/ejsLib.c
	all-in-one $(EJS_SOURCES) $(DB_SOURCES) $(WEB_SOURCES) $(EC_SOURCES) | \
        csplit -s -f tmpEjsLib. - '/#include.*ejsByteGoto.h/'
	(cat tmpEjsLib.00 ; cat $(SRC)/include/ejsByteGoto.h ; \
	if [ "$(BLD_OS)" = MACOSX -o "$(BLD_OS)" = FREEBSD ] ; \
		then tail +2 tmpEjsLib.01 ; \
		else tail --lines=+2 tmpEjsLib.01 ; \
	fi ) | egrep -v '#inc.*ejs|#inc.*ec|#inc.*sqlConf' >>$(DEST)/ejsLib.c
	@rm -f tmpEjsLib.*

$(DEST)/ejsc.c: $(EJS_HEADERS) $(EC_HEADERS) $(SRC)/cmd/ejsc.c
	echo '#include "ejs.h"' >$(DEST)/ejsc.c
	all-in-one $(SRC)/cmd/ejsc.c | egrep -v '#inc.*ejs|#inc.*ec|#inc.*sql' >>$(DEST)/ejsc.c

$(DEST)/ejs.c: $(EJS_HEADERS) $(SRC)/cmd/ejs.c
	echo '#include "ejs.h"' >$(DEST)/ejs.c
	all-in-one $(SRC)/cmd/ejs.c | egrep -v '#inc.*ejs|#inc.*ec|#inc.*sql' >>$(DEST)/ejs.c

$(DEST)/ejsmod.c: $(EJSMOD_HEADERS) $(EJSMOD_SOURCES)
	echo '#include "ejs.h"' >$(DEST)/ejsmod.c
	all-in-one $(EJSMOD_HEADERS) $(EJSMOD_SOURCES) | egrep -v '#include.*ejs|#include.*ec|#include.*sql' >>$(DEST)/ejsmod.c

$(DEST)/ejssql.c: $(JEMS)/ejs.db.sqlite/src/ejssql.c
	all-in-one $(SQLITE_HEADERS) $(SRC)//jems/ejs.db.sqlite/src/ejssql.c | \
		egrep -v '#include.*ejs|#include.*sqlite3' >$(DEST)/ejssql.c

$(DEST)/sqlite3.c: $(SQLITE_SOURCES) $(SQLITE_HEADERS)
	all-in-one $(SQLITE_HEADERS) $(SQLITE_SOURCES) | egrep -v '#include.*ejs|#include.*sqlite3' >$(DEST)/sqlite3.c

$(DEST)/ejs.es: $(ES_SOURCES)
	all-in-one $(ES_SOURCES)  | egrep -v 'use strict' >$(DEST)/ejs.es

$(ARCHIVE): Makefile $(TARGETS)
	cp $(BLD_TOP)/doc/api/ejsBare.html $(STAGING)/doc/api
	cp $(BLD_TOP)/doc/man/*.1 $(STAGING)/doc/man
	cp -r $(BLD_LIB_DIR)/www $(DEST)
	cp Makefile.all $(DEST)/Makefile
	cp -r $(BLD_BIN_DIR)/*.cmd $(DEST)
	@$(call log) "[Generate]" "tar cfz $(ARCHIVE) -C $(STAGING) ."
	rm -f $(ARCHIVE)
	tar cfz $(ARCHIVE) -C $(STAGING) .
	rm -fr $(STAGING)

cleanExtra:
	rm -fr $(STAGING)
