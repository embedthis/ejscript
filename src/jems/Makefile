#
#	Makefile for the jems (Ejscript extension packages)
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

#
#	Stand-alone tool jems
#
TOOLS			+= ejs.jem ejs.mvc

#
#	Loadable module jems (usable with require)
#
LOADABLE		= ejs.cjs ejs.db ejs.db.couch ejs.db.mapper ejs.db.sqlite ejs.tar ejs.unix ejs.web ejs.template 
LOADABLE_FILES	= $(shell for d in $(LOADABLE); do echo $$d/*.es | sort; done)
MODULES			= $(call mod, $(LOADABLE))
JEMS			= $(LOADABLE) $(TOOLS)
FILTER		 	= /sample/|/legacy/|^ejs.jem/|ejsmvc|Archive|\/test\/
LISTING			= --listing
DOC_SRC			= $(shell find ../core -name '*.es' | egrep -v "$(FILTER)")
DOC_SRC			+= $(shell find $(LOADABLE) -name '*.es' | egrep -v "$(FILTER)")
PRE_DIRS 		:= core 
POST_DIRS		:= $(LOADABLE) $(TOOLS)
BIN_DIR			?= $(BLD_BUILD_BIN_DIR)

include 		.makedep

DOC_DIR			:= $(BLD_TOP)/doc/api/ejscript
DOC				:= $(DOC_DIR)/index.html

compileExtra: $(MODULES) $(TOOLS)

modules: $(MODULES) $(TOOLS)

$(MODULES): $(LOADABLE_FILES) $(call mod, ejs)
	$(call log) [Compile] "Jems"
	rm -f *.mod *.slots.h
	$(BIN_DIR)/ejsc $(_ESFLAGS) $(LOADABLE_FILES)
	mv *.mod $(BLD_MOD_DIR)
	$(call log) [Generate] "Jem listings and slot files"
	$(BIN_DIR)/ejsmod --showDebug $(LISTING) --cslots $(MODULES)
	for f in *.slots.h ; do \
		diff $$f $(BLD_INC_DIR) >/dev/null 2>&1 || (printf "%12s %s\n" "[Install]" $$f ; cp $$f $(BLD_INC_DIR)) ; rm -f $$f;\
	done

packageExtra: buildJems $(DOC)

#
#	Move all doc under "doc" dir
#
$(DOC_DIR)/index.html: Makefile $(MODULES) $(DOC_SRC)
	$(call log) "[Generate]" documentation
	$(call setlibpath) ; $(BIN_DIR)/ejsc --doc --bind $(_ESFLAGS) --out .doc.mod --require "" $(DOC_SRC)
	rm -rf $(DOC_DIR)/*.html $(DOC_DIR)/*.css $(DOC_DIR)/images/*
	$(call setlibpath) ; $(BIN_DIR)/ejsmod --warn --html $(DOC_DIR) --require "" .doc.mod
	rm -f .doc.mod

doc: $(DOC_DIR)/index.html

buildJems:
	set -e ; \
	mkdir -p "$(BLD_ABS_JEM_DIR)" ; \
	for j in $(JEMS) ; do \
		vj="$$j-$(BLD_VERSION)" ; \
		target="$(BLD_ABS_JEM_DIR)/$$vj/$$vj.jem" ; \
		if [ ! -f $$target -o $(BLD_ABS_MOD_DIR)/$$j.mod -nt $$target ] ; then \
			cd $$j >/dev/null ; \
			jem --field version --value $(BLD_VERSION) edit ; \
			[ -f $(BLD_ABS_MOD_DIR)/$$vj.mod ] && jem setdeps $(BLD_ABS_MOD_DIR)/$$vj.mod ; \
			jem build ; \
			$(call log) "[Publish]" "jem --to $(BLD_ABS_JEM_DIR) publish $$vj" ; \
			jem --to $(BLD_ABS_JEM_DIR) publish $$vj ; \
			cd - >/dev/null ; \
		fi ; \
	done

cleanExtra:
	rm -f *.mod *.lst *.slots.h
	find . -name '*.lst' -o -name '*.mod' -o -name '*.slots.h' -o -name '*.jem' -type f | xargs rm -f

makeListForVs:
	find . -name '*.es' | egrep -v test\/|sample\/|Tar.es|jem.es|ejsmvc|ejspage|sample|jsgi
