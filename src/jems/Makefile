#
#	Makefile for the jems (Ejscript extension packages)
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

#
#	Order matters
#
LOADABLE	= ejs.jem ejs.unix ejs.db ejs.db.mapper ejs.db.sqlite ejs.web ejs.template ejs.tar
TOOLS		= ejs.jem ejs.mvc
FILTER	 	= /sample/|/legacy/|^ejs.jem/|ejsmvc|Archive|\/test\/
DOC_SRC		= $(shell find ../core -name '*.es' | egrep -v "$(FILTER)")
DOC_SRC		+= $(shell find $(LOADABLE) -name '*.es' | egrep -v "$(FILTER)")
PRE_DIRS	:= $(LOADABLE) $(TOOLS)

include 	.makedep

DOC_DIR		:= $(BLD_TOP)/doc/api/ejscript
DOC			:= $(DOC_DIR)/index.html

#
#	Build dependencies for ejs.jem first so that all jems can use the jem command
#
compileFirst: tools

compileLast: extraTools

#
#	Build modules required by build tools (jem)
#
tools:
	$(MAKE) --no-print-directory TRACE=$(TRACE) BUILDING_NATIVE=$(BUILDING_NATIVE) -C ejs.unix module
	$(MAKE) --no-print-directory TRACE=$(TRACE) BUILDING_NATIVE=$(BUILDING_NATIVE) -C ejs.tar module
	$(MAKE) --no-print-directory TRACE=$(TRACE) BUILDING_NATIVE=$(BUILDING_NATIVE) -C ejs.jem compile

#
#	For natively building extra tools (ejspage, ejssql, mvc)
#
extraTools:
ifeq ($(BLD_CROSS),1)
	$(MAKE) --no-print-directory TRACE=$(TRACE) BUILDING_NATIVE=1 -C ejs.template compile
	$(MAKE) --no-print-directory TRACE=$(TRACE) BUILDING_NATIVE=1 -C ejs.mvc compile
	$(MAKE) --no-print-directory TRACE=$(TRACE) BUILDING_NATIVE=1 -C ejs.db.sqlite compile
endif

#
#	Just for xcode
#
xmodules:
	for d in $(LOADABLE) ; do  \
		$(MAKE) --no-print-directory TRACE=$(TRACE) -C $$d module || exit 255; \
	done

packagePrep: $(DOC)

doc: $(DOC_DIR)/index.html

$(DOC_DIR)/index.html: Makefile $(DOC_SRC)
	$(call log) "[Generate]" documentation
	$(call setlibpath) ; $(BLD_BUILD_BIN_DIR)/ejsc --doc --bind $(_ESFLAGS) --out .doc.mod --require "" $(DOC_SRC)
	rm -rf $(DOC_DIR)/*.html $(DOC_DIR)/*.css $(DOC_DIR)/images/*
	$(call setlibpath) ; $(BLD_BUILD_BIN_DIR)/ejsmod --warn --html $(DOC_DIR) --require "" .doc.mod
	rm -f .doc.mod

cleanExtra:
	rm -f *.mod *.lst *.slots.h
	find . -name '*.lst' -o -name '*.mod' -o -name '*.slots.h' -o -name '*.jem' -type f | xargs rm -f

makeListForVs:
	find . -name '*.es' | egrep -v test\/|sample\/|Tar.es|jem.es|ejsmvc|ejspage|sample|jsgi

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
