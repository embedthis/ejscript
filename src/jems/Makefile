#
#	Makefile for the jems (Ejscript extension packages)
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

#
#	Stand-alone tool jems
#
TOOLS			+= ejs.jem ejs.web.cgi ejs.mvc

#
#	Loadable module jems (usable with require)
#
LOADABLE		+= ejs.cache ejs.cjs ejs.db ejs.db.couch ejs.db.mapper ejs.db.sqlite ejs.tar ejs.unix ejs.web ejs.web.jsgi \
			   		ejs.web.template 
LOADABLE_FILES	+= $(shell for d in $(LOADABLE); do echo $$d/*.es | sort; done)
MODULES			+= $(call mod, $(LOADABLE))
JEMS			+= $(LOADABLE) $(TOOLS)
FILTER		 	+= /sample/|/legacy/|^ejs.jem/|ejsmvc|Archive|\/test\/
LISTING			+= --listing
DOC_SRC			+= $(shell find ../core -name '*.es' | egrep -v "$(FILTER)")
DOC_SRC			+= $(shell find $(LOADABLE) -name '*.es' | egrep -v "$(FILTER)")
PRE_DIRS 		:= core 
POST_DIRS		:= $(LOADABLE) $(TOOLS)

include 		.makedep

DOC_DIR			:= $(BLD_TOP)/doc/api/ejscript
DOC				:= $(DOC_DIR)/index.html

compileExtra: $(MODULES) $(TOOLS)

$(MODULES): $(LOADABLE_FILES) $(call mod, ejs)
	$(call log) [Compile] "Jems"
	rm -f *.mod *.slots.h
	$(BLD_BUILD_BIN_DIR)/ejsc $(_ESFLAGS) $(LOADABLE_FILES)
	mv *.mod $(BLD_MOD_DIR)
	$(call log) [Generate] "Jem listings and slot files"
	$(BLD_BUILD_BIN_DIR)/ejsmod --showDebug $(LISTING) --cslots $(MODULES)
	for f in *.slots.h ; do \
		diff $$f $(BLD_INC_DIR) >/dev/null 2>&1 || (printf "%12s %s\n" "[Install]" $$f ; cp $$f $(BLD_INC_DIR)) ; rm -f $$f;\
	done

packageExtra: buildJems $(DOC)

#
#	Move all doc under "doc" dir
#
$(DOC_DIR)/index.html: Makefile $(MODULES)
	$(call log) "[Generate]" documentation
	$(call setlibpath) ; $(BLD_BUILD_BIN_DIR)/ejsc --doc --bind $(_ESFLAGS) --out .doc.mod --require "" $(DOC_SRC)
	rm -rf $(DOC_DIR)/*.html $(DOC_DIR)/*.css $(DOC_DIR)/images/*
	$(call setlibpath) ; $(BLD_BUILD_BIN_DIR)/ejsmod --warn --html $(DOC_DIR) --require "" .doc.mod
	rm -f .doc.mod

doc: $(DOC_DIR)/index.html

#
#	MOB - need to do this only when inputs change. Should then do at compile time
#
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
		$(call log) "[Publish]" "jem --to $(BLD_ABS_JEM_DIR) publish $$vj" ; \
		jem --to $(BLD_ABS_JEM_DIR) publish $$vj ; \
		cd - >/dev/null ; \
	done

cleanExtra:
	rm -f *.mod *.lst *.slots.h
	find . -name '*.lst' -o -name '*.mod' -o -name '*.slots.h' -o -name '*.jem' -type f | xargs rm -f

makeListForVs:
	find . -name '*.es' | egrep -v test\/|sample\/|Tar.es|jem.es|ejsmvc|ejspage|sample|jsgi
