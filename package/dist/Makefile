#
#	Makefile to build the Ejscript distributions
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep

STAGE_DIR		:= $(BLD_OUT_DIR)/dist
REL_DIR			:= $(BLD_OUT_DIR)/releases
TDIR			:= $(STAGE_DIR)/ejs-$(BLD_VERSION)
DIST_VER_NAME	:= ejs-$(BLD_VERSION)-dist.tgz
DIST_PERM_NAME	:= ejs-dist.tgz
FLAT_VER_NAME	:= ejs-$(BLD_VERSION)-flat.tgz
FLAT_PERM_NAME	:= ejs-flat.tgz
SRC				:= ../../src
JEMS			:= $(SRC)/jems
DEST			:= $(TDIR)/src/deps/ejs
INC_DEST  		:= $(DEST)
DOC_DEST		:= $(TDIR)/doc
EJS_HEADERS		+= $(patsubst %,$(SRC)/include/%.h, ejsByteCode ejsByteCodeTable ejs ejsWeb)
SLOT_HEADERS	+= $(shell find $(SRC)/include/master -name 'ejs*slots.h' | sort)
EC_HEADERS		+= $(patsubst %,$(SRC)/include/%.h, ecCompiler)
EJS_SOURCES		+= $(shell find $(SRC)/vm $(SRC)/core -name '*.c' | sort) 
EC_SOURCES		+= $(shell find $(SRC)/compiler -name '*.c' | sort) 
ES_SOURCES		+= $(shell find $(SRC)/core $(JEMS)/ejs.unix $(JEMS)/ejs.db $(JEMS)/ejs.db.mapper \
					$(JEMS)/ejs.db.sqlite $(JEMS)/ejs.web $(JEMS)/ejs.template -name '*.es' | \
					egrep -v 'SAVE|save|legacy|Archive|old|sample.es|XmlGlobal.es|mvc.es' | \
					egrep -v 'Jsdb.es|Mysql.es|ejspage.es|\/test\/' | sort) 
EJSMOD_HEADERS	+= $(SRC)/cmd/ejsmod.h
EJSMOD_SOURCES	+= $(patsubst %,$(SRC)/cmd/%.c, doc docFiles ejsmod listing slotGen)
DB_SOURCES		+= $(JEMS)/ejs.db.sqlite/src/ejsSqlite.c
SQLITE_HEADERS	+= $(SRC)/include/sqlite3.h
SQLITE_SOURCES	+= $(JEMS)/ejs.db.sqlite/src/sqlite3.c
WEB_SOURCES		+= $(shell find $(JEMS)/ejs.web -name '*.c' | egrep -v 'SAVE|save|legacy|Archive' | sort)
TARGETS			+= $(patsubst %,$(INC_DEST)/%, ejs.h sqlite3.h slots.h)
TARGETS			+= $(patsubst %,$(DEST)/%, ejsLib.c ejs.c ejsc.c ejsmod.c ejssql.c sqlite3.c ejs.es)

dist packageExtra:	clean prep $(TARGETS) $(REL_DIR)/$(DIST_VER_NAME) $(REL_DIR)/$(FLAT_VER_NAME) cleanup
	
prep:
	$(call log) "[Prepare]" "Package directories"
	mkdir -p $(STAGE_DIR) $(REL_DIR) $(TDIR)/src/include/master $(TDIR)/src/deps/ejs $(TDIR)/doc/api $(TDIR)/doc/man

$(INC_DEST)/ejs.h: $(EJS_HEADERS)
	TITLE="$(BLD_NAME) Header" \
	dist $(EJS_HEADERS) $(EC_HEADERS) | egrep -v '#include.*ejs|#include.*ec|#include.*sql' >$(INC_DEST)/ejs.h
	echo '#include "ejs.slots.h"' >>$(INC_DEST)/ejs.h

$(INC_DEST)/sqlite3.h: $(BLD_TOP)/src/include/sqlite3.h
	cp $(BLD_TOP)/src/include/sqlite3.h $(INC_DEST)/sqlite3.h

$(INC_DEST)/slots.h: $(SLOT_HEADERS)
	TITLE="$(BLD_NAME) Slot files" dist $(SLOT_HEADERS) >$(INC_DEST)/slots.h

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

$(REL_DIR)/$(DIST_VER_NAME):
	cp $(BLD_TOP)/doc/api/ejsBare.html $(TDIR)/doc/api
	cp $(BLD_TOP)/doc/man/*.1 $(TDIR)/doc/man
	cp -r $(BLD_LIB_DIR)/www $(DEST)
	cp Makefile.dist $(DEST)/Makefile
	cp $(BLD_BIN_DIR)/*.cmd $(BLD_BIN_DIR)/jem $(BLD_BIN_DIR)/mvc $(DEST)
	$(call log) "[Generate]" "tar cfz $(REL_DIR)/$(DIST_VER_NAME) -C $(STAGE_DIR) ."
	tar cfz $(REL_DIR)/$(DIST_VER_NAME) -C $(STAGE_DIR) .
	cd $(REL_DIR) ; ln -s $(DIST_VER_NAME) $(DIST_PERM_NAME)

flat $(REL_DIR)/$(FLAT_VER_NAME):
	mv $(TDIR)/src/deps/ejs/www $(TDIR)
	find $(TDIR)/src -type f | xargs -I % mv % $(TDIR) 
	cp $(SRC)/deps/pcre/pcre.h $(SRC)/deps/http/http.h $(SRC)/deps/mpr/mpr.h $(SRC)/deps/mpr/mprSsl.h $(TDIR) 
	cp $(SRC)/deps/mpr/mprLib.c $(SRC)/deps/mpr/mprSsl.c $(TDIR)
	cp $(SRC)/deps/http/httpLib.c $(SRC)/deps/pcre/pcre.c $(TDIR)
	rm -fr $(TDIR)/doc $(TDIR)/src $(TDIR)/Makefile
	$(call log) "[Generate]" "tar cfz $(REL_DIR)/$(FLAT_VER_NAME) -C $(STAGE_DIR) ."
	tar cfz $(REL_DIR)/$(FLAT_VER_NAME) -C $(STAGE_DIR) .
	ln -s $(FLAT_VER_NAME) $(REL_DIR)/$(FLAT_PERM_NAME)

cleanup:
	rm -fr $(STAGE_DIR)

cleanExtra:
	rm -fr $(STAGE_DIR) $(REL_DIR)/$(DIST_VER_NAME) $(REL_DIR)/$(DIST_PERM_NAME) 
	rm -fr $(REL_DIR)/$(FLAT_VER_NAME) $(REL_DIR)/$(FLAT_PERM_NAME)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
