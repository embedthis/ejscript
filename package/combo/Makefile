#
#	Makefile to build the combo and flat Ejscript distributions
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 		.makedep

STAGING			:= $(PWD)/staging
TDIR			:= $(STAGING)/ejs-$(BLD_VERSION)
REL_DIR			:= ../../releases
COMBO_VER_NAME	:= ejs-$(BLD_VERSION)-combo.tgz
COMBO_PERM_NAME	:= ejs-combo.tgz
COMBO_VER		:= $(REL_DIR)/$(COMBO_VER_NAME)
COMBO_PERM		:= $(REL_DIR)/$(COMBO_PERM_NAME)
FLAT_VER_NAME	:= ejs-$(BLD_VERSION)-flat.tgz
FLAT_PERM_NAME	:= ejs-flat.tgz
FLAT_VER		:= $(REL_DIR)/$(FLAT_VER_NAME)
FLAT_PERM		:= $(REL_DIR)/$(FLAT_PERM_NAME)
SRC				:= ../../src
JEMS			:= $(SRC)/jems
DEST			:= $(TDIR)/src/deps/ejs
INC_DEST		:= $(TDIR)/src/include/master
DOC_DEST		:= $(TDIR)/doc
EJS_HEADERS		+= $(patsubst %,$(SRC)/include/%.h, ejsTune ejsByteCode ejsByteCodeTable ejsCore ejsModule ejs ejsWeb)
SLOT_HEADERS	+= $(shell find $(SRC)/include -name 'ejs*slots.h')
EC_HEADERS		+= $(patsubst %,$(SRC)/include/%.h, ecCompiler)
EJS_SOURCES		+= $(shell find $(SRC)/vm $(SRC)/core -name '*.c') 
EC_SOURCES		+= $(shell find $(SRC)/compiler -name '*.c') 
ES_SOURCES		+= $(shell find $(SRC)/core $(JEMS)/ejs.cjs \
				    $(JEMS)/ejs.unix $(JEMS)/ejs.db $(JEMS)/ejs.db.mapper \
					$(JEMS)/ejs.db.sqlite $(JEMS)/ejs.web $(JEMS)/ejs.template -name '*.es' | \
					egrep -v 'SAVE|save|legacy|Archive|old|sample.es|XmlGlobal.es|ejsmvc.es|Jsdb.es|Mysql.es|ejspage.es|\/test\/') 
EJSMOD_HEADERS	+= $(SRC)/cmd/ejsmod.h
EJSMOD_SOURCES	+= $(patsubst %,$(SRC)/cmd/%.c, doc docFiles ejsmod listing slotGen)
DB_SOURCES		+= $(JEMS)/ejs.db.sqlite/src/ejsSqlite.c
SQLITE_HEADERS	+= $(SRC)/include/sqlite3.h
SQLITE_SOURCES	+= $(JEMS)/ejs.db.sqlite/src/sqlite3.c
WEB_SOURCES		+= $(shell find $(JEMS)/ejs.web -name '*.c' | egrep -v 'SAVE|save|legacy|Archive')
TARGETS			+= $(patsubst %,$(INC_DEST)/%, ejs.h ejs.slots.h sqlite3.h)
TARGETS			+= $(patsubst %,$(DEST)/%, ejsLib.c ejs.c ejsc.c ejsmod.c ejssql.c sqlite3.c ejs.es)

combo packageExtra:	prep $(TARGETS) $(COMBO_VER) $(FLAT_VER)
	
prep:
	@$(call log) "[Prepare]" "Combo package"
	rm -fr $(STAGING) $(COMBO_VER) $(COMBO_PERM) $(FLAT_VER) $(FLAT_PERM)
	mkdir -p $(REL_DIR)
	mkdir -p $(TDIR)/src/include/master $(TDIR)/src/deps/ejs $(TDIR)/doc/api $(TDIR)/doc/man

$(INC_DEST)/ejs.h: $(EJS_HEADERS)
	@combo $(EJS_HEADERS) $(EC_HEADERS) | egrep -v '#include.*ejs|#include.*ec|#include.*sql' >$(INC_DEST)/ejs.h
	@echo '#include "ejs.slots.h"' >>$(INC_DEST)/ejs.h

$(INC_DEST)/ejs.slots.h: $(SLOT_HEADERS)
	@combo $(SLOT_HEADERS) >$(INC_DEST)/ejs.slots.h

$(INC_DEST)/sqlite3.h: $(BLD_INC_DIR)/sqlite3.h
	cp $(BLD_INC_DIR)/sqlite3.h $(INC_DEST)/sqlite3.h

$(DEST)/ejsLib.c: $(EJS_SOURCES) $(DB_SOURCES) $(WEB_SOURCES)
	echo '#define EJS_DEFINE_OPTABLE 1' >$(DEST)/ejsLib.c
	echo '#include "ejs.h"' >>$(DEST)/ejsLib.c
	combo $(EJS_SOURCES) $(DB_SOURCES) $(WEB_SOURCES) $(EC_SOURCES) | \
        csplit -s -f tmpEjsLib. - '/#include.*ejsByteGoto.h/'
	(cat tmpEjsLib.00 ; cat $(SRC)/include/ejsByteGoto.h ; \
	if [ "$(BLD_OS)" = MACOSX -o "$(BLD_OS)" = FREEBSD ] ; \
		then tail +2 tmpEjsLib.01 ; \
		else tail --lines=+2 tmpEjsLib.01 ; \
	fi ) | egrep -v '#inc.*ejs|#inc.*ec|#inc.*sqlConf' >>$(DEST)/ejsLib.c
	@rm -f tmpEjsLib.*

$(DEST)/ejsc.c: $(EJS_HEADERS) $(EC_HEADERS) $(SRC)/cmd/ejsc.c
	echo '#include "ejs.h"' >$(DEST)/ejsc.c
	combo $(SRC)/cmd/ejsc.c | egrep -v '#inc.*ejs|#inc.*ec|#inc.*sql' >>$(DEST)/ejsc.c

$(DEST)/ejs.c: $(EJS_HEADERS) $(SRC)/cmd/ejs.c
	echo '#include "ejs.h"' >$(DEST)/ejs.c
	combo $(SRC)/cmd/ejs.c | egrep -v '#inc.*ejs|#inc.*ec|#inc.*sql' >>$(DEST)/ejs.c

$(DEST)/ejsmod.c: $(EJSMOD_HEADERS) $(EJSMOD_SOURCES)
	echo '#include "ejs.h"' >$(DEST)/ejsmod.c
	combo $(EJSMOD_HEADERS) $(EJSMOD_SOURCES) | egrep -v '#include.*ejs|#include.*ec|#include.*sql' >>$(DEST)/ejsmod.c

$(DEST)/ejssql.c: $(JEMS)/ejs.db.sqlite/src/ejssql.c
	combo $(SQLITE_HEADERS) $(SRC)//jems/ejs.db.sqlite/src/ejssql.c | \
		egrep -v '#include.*ejs|#include.*sqlite3' >$(DEST)/ejssql.c

$(DEST)/sqlite3.c: $(SQLITE_SOURCES) $(SQLITE_HEADERS)
	combo $(SQLITE_HEADERS) $(SQLITE_SOURCES) | egrep -v '#include.*ejs|#include.*sqlite3' >$(DEST)/sqlite3.c

$(DEST)/ejs.es: $(ES_SOURCES)
	combo $(ES_SOURCES)  | egrep -v 'use strict' >$(DEST)/ejs.es

$(COMBO_VER):
	cp $(BLD_TOP)/doc/api/ejsBare.html $(TDIR)/doc/api
	cp $(BLD_TOP)/doc/man/*.1 $(TDIR)/doc/man
	cp -r $(BLD_LIB_DIR)/www $(DEST)
	cp Makefile.combo $(DEST)/Makefile
	cp $(BLD_BIN_DIR)/*.cmd $(DEST) $(BLD_BIN_DIR)/jem $(BLD_BIN_DIR)/mvc $(DEST)
	$(call log) "[Generate]" "tar cfz $(COMBO_VER) -C $(STAGING) ."
	tar cfz $(COMBO_VER) -C $(STAGING) .
	cd $(REL_DIR) ; ln -s $(COMBO_VER_NAME) $(COMBO_PERM_NAME)

flat $(FLAT_VER):
	mv $(TDIR)/src/deps/ejs/www $(TDIR)
	find $(TDIR)/src -type f | xargs -I % mv % $(TDIR) 
	cp $(SRC)/include/pcre.h $(SRC)/include/http.h $(SRC)/include/mpr.h $(SRC)/include/mprSsl.h $(TDIR) 
	cp $(SRC)/deps/mpr/mprLib.c $(SRC)/deps/mpr/mprSsl.c $(TDIR)
	cp $(SRC)/deps/http/httpLib.c $(SRC)/deps/pcre/pcre.c $(TDIR)
	rm -fr $(TDIR)/doc $(TDIR)/src $(TDIR)/Makefile
	$(call log) "[Generate]" "tar cfz $(FLAT_VER) -C $(STAGING) ."
	tar cfz $(FLAT_VER) -C $(STAGING) .
	cd $(REL_DIR) ; ln -s $(FLAT_VER_NAME) $(FLAT_PERM_NAME)

cleanExtra:
	rm -fr $(STAGING)
