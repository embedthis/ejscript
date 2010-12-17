#
#	Makefile to build the Ejscript distributions
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 		.makedep

STAGING			:= $(PWD)/staging
TDIR			:= $(STAGING)/ejs-$(BLD_VERSION)
REL_DIR			:= $(BLD_OUT_DIR)/releases
DIST_VER_NAME	:= ejs-$(BLD_VERSION)-dist.tgz
DIST_PERM_NAME	:= ejs-dist.tgz
DIST_VER		:= $(REL_DIR)/$(DIST_VER_NAME)
DIST_PERM		:= $(REL_DIR)/$(DIST_PERM_NAME)
FLAT_VER_NAME	:= ejs-$(BLD_VERSION)-flat.tgz
FLAT_PERM_NAME	:= ejs-flat.tgz
FLAT_VER		:= $(REL_DIR)/$(FLAT_VER_NAME)
FLAT_PERM		:= $(REL_DIR)/$(FLAT_PERM_NAME)
SRC				:= ../../src
JEMS			:= $(SRC)/jems
DEST			:= $(TDIR)/src/deps/ejs
INC_DEST  		:= $(TDIR)/src/include
MASTER  		:= $(INC_DEST)/master
DOC_DEST		:= $(TDIR)/doc
EJS_HEADERS		+= $(patsubst %,$(SRC)/include/%.h, ejsTune ejsByteCode ejsByteCodeTable ejsCore ejsModule ejs ejsWeb)
SLOT_HEADERS	+= $(shell find $(SRC)/include/master -name 'ejs*slots.h')
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
TARGETS			+= $(patsubst %,$(INC_DEST)/%, ejs.h sqlite3.h)
TARGETS			+= $(patsubst %,$(MASTER)/%, ejs.slots.h)
TARGETS			+= $(patsubst %,$(DEST)/%, ejsLib.c ejs.c ejsc.c ejsmod.c ejssql.c sqlite3.c ejs.es)

dist packageExtra:	prep $(TARGETS) $(DIST_VER) $(FLAT_VER) cleanup
	
prep:
	$(call log) "[Prepare]" "Package directories"
	rm -fr $(STAGING) $(DIST_VER) $(DIST_PERM) $(FLAT_VER) $(FLAT_PERM)
	mkdir -p $(REL_DIR) $(TDIR)/src/include/master $(TDIR)/src/deps/ejs $(TDIR)/doc/api $(TDIR)/doc/man

$(INC_DEST)/ejs.h: $(EJS_HEADERS)
	TITLE="$(BLD_NAME) Header" \
	dist $(EJS_HEADERS) $(EC_HEADERS) | egrep -v '#include.*ejs|#include.*ec|#include.*sql' >$(INC_DEST)/ejs.h
	echo '#include "ejs.slots.h"' >>$(INC_DEST)/ejs.h

$(INC_DEST)/sqlite3.h: $(BLD_TOP)/src/include/sqlite3.h
	cp $(BLD_TOP)/src/include/sqlite3.h $(INC_DEST)/sqlite3.h

$(MASTER)/ejs.slots.h: $(SLOT_HEADERS)
	TITLE="$(BLD_NAME) Slot files" \
	dist $(SLOT_HEADERS) >$(MASTER)/ejs.slots.h

$(DEST)/ejsLib.c: $(EJS_SOURCES) $(DB_SOURCES) $(WEB_SOURCES)
	echo '#define EJS_DEFINE_OPTABLE 1' >$(DEST)/ejsLib.c
	echo '#include "ejs.h"' >>$(DEST)/ejsLib.c
	TITLE="$BLD_NAME Library Source" \
	dist $(EJS_SOURCES) $(DB_SOURCES) $(WEB_SOURCES) $(EC_SOURCES) | \
        csplit -s -f tmpEjsLib. - '/#include.*ejsByteGoto.h/'
	(cat tmpEjsLib.00 ; cat $(SRC)/include/ejsByteGoto.h ; \
	if [ "$(BLD_OS)" = MACOSX -o "$(BLD_OS)" = FREEBSD ] ; \
		then tail +2 tmpEjsLib.01 ; \
		else tail --lines=+2 tmpEjsLib.01 ; \
	fi ) | egrep -v '#inc.*ejs|#inc.*ec|#inc.*sqlConf' >>$(DEST)/ejsLib.c
	rm -f tmpEjsLib.*

$(DEST)/ejsc.c: $(EJS_HEADERS) $(EC_HEADERS) $(SRC)/cmd/ejsc.c
	echo '#include "ejs.h"' >$(DEST)/ejsc.c
	TITLE="$(BLD_NAME) Compiler Source" \
	dist $(SRC)/cmd/ejsc.c | egrep -v '#inc.*ejs|#inc.*ec|#inc.*sql' >>$(DEST)/ejsc.c

$(DEST)/ejs.c: $(EJS_HEADERS) $(SRC)/cmd/ejs.c
	echo '#include "ejs.h"' >$(DEST)/ejs.c
	TITLE="$(BLD_NAME) Shell Source" \
	dist $(SRC)/cmd/ejs.c | egrep -v '#inc.*ejs|#inc.*ec|#inc.*sql' >>$(DEST)/ejs.c

$(DEST)/ejsmod.c: $(EJSMOD_HEADERS) $(EJSMOD_SOURCES)
	echo '#include "ejs.h"' >$(DEST)/ejsmod.c
	TITLE="$(BLD_NAME) Manager Source" \
	dist $(EJSMOD_HEADERS) $(EJSMOD_SOURCES) | egrep -v '#include.*ejs|#include.*ec|#include.*sql' >>$(DEST)/ejsmod.c

$(DEST)/ejssql.c: $(JEMS)/ejs.db.sqlite/src/ejssql.c
	TITLE="$(BLD_NAME) SQLite Shell Source" \
	dist $(SQLITE_HEADERS) $(SRC)//jems/ejs.db.sqlite/src/ejssql.c | \
		egrep -v '#include.*ejs|#include.*sqlite3' >$(DEST)/ejssql.c

$(DEST)/sqlite3.c: $(SQLITE_SOURCES) $(SQLITE_HEADERS)
	TITLE="$(BLD_NAME) SQLite Library Source" \
	dist $(SQLITE_HEADERS) $(SQLITE_SOURCES) | egrep -v '#include.*ejs|#include.*sqlite3' >$(DEST)/sqlite3.c

$(DEST)/ejs.es: $(ES_SOURCES)
	TITLE="$(BLD_NAME) Core Module" \
	dist $(ES_SOURCES)  | egrep -v 'use strict' >$(DEST)/ejs.es

$(DIST_VER):
	cp $(BLD_TOP)/doc/api/ejsBare.html $(TDIR)/doc/api
	cp $(BLD_TOP)/doc/man/*.1 $(TDIR)/doc/man
	cp -r $(BLD_LIB_DIR)/www $(DEST)
	cp Makefile.dist $(DEST)/Makefile
	cp $(BLD_BIN_DIR)/*.cmd $(BLD_BIN_DIR)/jem $(BLD_BIN_DIR)/mvc $(DEST)
	$(call log) "[Generate]" "tar cfz $(DIST_VER) -C $(STAGING) ."
	tar cfz $(DIST_VER) -C $(STAGING) .
	cd $(REL_DIR) ; ln -s $(DIST_VER_NAME) $(DIST_PERM_NAME)

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

cleanup:
	rm -fr $(STAGING)

cleanExtra:
	rm -fr $(STAGING) $(REL_DIR)/*.tgz
