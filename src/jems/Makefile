#
#	Makefile for the Ejscript script code. 
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

EXT			+= ejs.cache ejs.cjs ejs.db ejs.db.couch ejs.db.mapper ejs.db.sqlite ejs.tar ejs.unix ejs.web \
			   ejs.web.google ejs.web.jsgi ejs.web.template
APPS		+= ejs.jem ejs.web.cgi

PRE_DIRS 	+= core 
POST_DIRS	+= ejs $(EXT) $(APPS)

EXT_MODS	+= $(call mod, $(EXT))

FILTER		 = '/sample/|/legacy/|^ejs.jem/|ejsmvc|Archive|\/test\/'
EXT_FILES	+= $(shell for d in $(EXT); do echo $$d/*.es | sort; done)

LISTING		+= --listing

DOC_SRC		+= $(shell find $(JEMS) -name '*.es' | egrep -v "$(FILTER)")
DOC_DIR		:= $(BLD_TOP)/doc/api/ejscript
DOC			:= $(DOC_DIR)/index.html

include 	.makedep

compileExtra: $(EXT_MODS) $(APPS)

$(EXT_MODS): $(EXT_FILES) $(call mod, ejs)
	$(call log) [Compile] "Jems"
	rm -f *.mod *.slots.h
	$(BLD_BUILD_BIN_DIR)/ejsc $(_ESFLAGS) $(EXT_FILES)
	mv *.mod $(BLD_MOD_DIR)
	$(call log) [Generate] "Jem listings and slot files"
	$(BLD_BUILD_BIN_DIR)/ejsmod --showDebug $(LISTING) --cslots $(EXT_MODS)
	for f in *.slots.h ; do \
		diff $$f $(BLD_INC_DIR) >/dev/null 2>&1 || (printf "%12s %s\n" "[Install]" $$f ; cp $$f $(BLD_INC_DIR)) ; rm -f $$f;\
	done

packageExtra: buildJems $(DOC)

#
#	Move all doc under "doc" dir
#
$(DOC_DIR)/index.html: Makefile $(CORE_MODS) $(EXT_MODS)
	$(call log) "[Generate]" documentation
	$(call setlibpath) ; $(BLD_BUILD_BIN_DIR)/ejsc --doc --bind $(_ESFLAGS) --out .doc.mod --require "" $(DOC_SRC)
	rm -rf $(DOC_DIR)/*.html $(DOC_DIR)/*.css $(DOC_DIR)/images/*
	$(call setlibpath) ; $(BLD_BUILD_BIN_DIR)/ejsmod --warn --html $(DOC_DIR) --require "" .doc.mod
	rm -f .doc.mod

doc: $(DOC_DIR)/index.html

buildJems:
	set -e ; \
	mkdir -p "$(BLD_ABS_JEM_DIR)" ; \
	for j in $(JEMS) ; do \
		cd $$j >/dev/null ; \
		vj="$$j-$(BLD_VERSION)" ; \
		dest="$(BLD_ABS_JEM_DIR)/$$vj" ; \
		jem --field version --value $(BLD_VERSION) edit ; \
		[ -f $(BLD_ABS_MOD_DIR)/$$vj.mod ] && jem setdeps $(BLD_ABS_MOD_DIR)/$$vj.mod ; \
		jem build ; \
		mkdir -p $$dest ; \
		cp $$vj.jem $$dest ; \
		true jem --to $(BLD_ABS_JEM_DIR) publish $$j ; \
		cd $$dest >/dev/null ; \
		tar xfz $$vj.jem package.json ; \
		cd - >/dev/null ; \
	done

cleanExtra:
	rm -f *.mod *.lst *.slots.h
	find . -name '*.lst' -o -name '*.mod' -o -name '*.slots.h' -o -name '*.jem' -type f | xargs rm -f

makeListForVs:
	find . -name '*.es' | egrep -v test\/|sample\/|Tar.es|jem.es|ejsmvc|ejspage|sample|jsgi
