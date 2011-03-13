#
#	Makefile for the Ejscript source code
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

PRE_DIRS		+= include deps vm compiler
POST_DIRS		+= core cmd jems
EJS_LIB			+= $(BLD_LIB_DIR)/libejs$(BLD_LIB)
EJS_SOURCES		+= $(shell find core/src vm compiler -name '*.c') 
EJS_OBJECTS		+= $(patsubst %.c,$(BLD_OBJ_DIR)/%$(BLD_OBJ),$(notdir $(EJS_SOURCES)))

include 		.makedep

compileFirst:
	$(call log) [Notice] "Building Native Ejs Toolchain"
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/deps -C deps compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/vm -C vm compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/compiler -C compiler compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/core/src -C core/src compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR) -C . library
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/cmd -C cmd compile
	echo
	$(call log) [Notice] "Building For Target"

compileExtra: core library

core:
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/core -C core 

library: $(EJS_LIB)

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
