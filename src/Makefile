#
#	Makefile for the Ejscript source code
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

LISTING			+= --listing
PRE_DIRS		+= include deps vm compiler core 
POST_DIRS		+= cmd jems
EJS_MOD			+= $(call mod, ejs)
EJS_FILES		+= $(shell echo core/*.es | sort)
EJS_LIB			+= $(BLD_LIB_DIR)/libejs$(BLD_LIB)
EJS_SOURCES		+= $(shell find core/src vm compiler -name '*.c') 
EJS_OBJECTS		+= $(patsubst %.c,$(BLD_OBJ_DIR)/%$(BLD_OBJ),$(notdir $(EJS_SOURCES)))

include 		.makedep

compileFirst: tools 

compileExtra: modules library

tools:
	echo
	$(call log) [Notice] "Building Native Ejs Toolchain"
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/deps -C deps compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/vm -C vm compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/compiler -C compiler compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/core -C core compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR) -C . library
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/cmd -C cmd tools
	echo
	$(call log) [Notice] "Building For Target"

modules: $(EJS_MOD)

#
#	Just for xcode
#
xmodules: modules

library: $(EJS_LIB)

$(EJS_MOD): $(EJS_FILES)
	$(call log) [Compile] "Core Ejs Classes"
	rm -f *.mod *.slots.h
	$(BLD_BUILD_BIN_DIR)/ejsc $(_ESFLAGS) --out ejs.mod --bind --require "" $(EJS_FILES)
	cp *.mod $(BLD_BUILD_MOD_DIR)
	mv *.mod $(BLD_MOD_DIR)
	$(call log) [Generate] "Core listings and slot files"
	$(BLD_BUILD_BIN_DIR)/ejsmod --require "" $(LISTING) --cslots $(call mod, ejs)
	changed=0 ; \
	for f in *.slots.h ; do \
		if [ -f $(BLD_INC_DIR)/$$f ] ; then \
			if ! diff $$f $(BLD_INC_DIR)/$$f >/dev/null 2>&1; then \
				printf "%12s %s\n" "[Install]" $(subst $(BLD_OUT_DIR),,$(BLD_INC_DIR)/$$f) ; \
				cp $$f $(BLD_INC_DIR) ; \
				changed=1 ; \
			fi ; \
		fi ; \
		rm -f $$f; \
	done ; \
	if [ "$$changed" = 1 -a "$(RETRY)" = "" ] ; then \
		echo ; \
		$(call log) [Notice] "Rebuild with Modified Slot Definitions" ; \
		$(MAKE) TRACE=$(TRACE) RETRY=1 $(MAKECMDGOALS) ; \
	fi

$(EJS_LIB): $(EJS_SOURCES) $(EJS_OBJECTS)
	bld --library $(EJS_LIB) --search "$(BLD_EJS_WITHPATHS)" --libs "$(BLD_EJS_WITHLIBS)" $(EJS_OBJECTS)

cleanExtra:
	find . -name '*.lst' -o -name '*.mod' -o -name '*.jem' -type f | xargs rm -f
	rm -f *.slots.h

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
