#
#	Makefile for the Ejscript source code
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

PRE_DIRS		+= slots deps vm compiler
POST_DIRS		+= core cmd jems samples
EJS_LIB			+= $(BLD_LIB_DIR)/libejs$(BLD_LIB)
EJS_SOURCES		+= $(shell find core/src vm compiler -name '*.c') 
EJS_OBJECTS		+= $(patsubst %.c,$(BLD_OBJ_DIR)/%$(BLD_OBJ),$(notdir $(EJS_SOURCES)))
SLOTS     		+= $(patsubst slots/%.h,$(BLD_INC_DIR)/%.h,$(shell find slots/*.slots.h))

include 		.makedep

compileFirst: prep $(HEADERS) tools

compileExtra: core library $(SLOTS)

#
#	These are the required tools to build Ejscript: (ejs, ejsc, ejsmod, jem)
#
tools:
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/slots -C slots compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/deps -C deps compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/vm -C vm compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/compiler -C compiler compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/core/src -C core/src compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR) -C . library
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/cmd -C cmd compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/core -C core compile
	: # Rebuild the core, compiler, library and commands incase the headers changed 
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/vm -C vm compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/compiler -C compiler compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/core/src -C core/src compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR) -C . library
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/cmd -C cmd compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/jems -C jems tools

core: dummy
	$(MAKE) -S --no-print-directory TRACE=$(TRACE) DIR=$(DIR)/core -C core 

library: $(EJS_LIB)

$(EJS_LIB): $(EJS_SOURCES) $(EJS_OBJECTS)
	bld --library $(EJS_LIB) --libs "$(BLD_EJSCRIPT_WITHLIBS)" $(EJS_OBJECTS)

dependExtra:
	cp slots/*slots.h $(BLD_INC_DIR)

cleanExtra:
	find . -name '*.lst' -o -name '*.mod' -o -name '*.jem' -type f | xargs rm -f
	rm -f *.slots.h
	rm -f $(BLD_INC_DIR)/ejs*.h f $(BLD_INC_DIR)/ejs.*.h
	rm -f $(EJS_LIB) $(BLD_LIB_DIR)/ejs.lst 
	mkdir -p $(BLD_INC_DIR)

prep:
	if [ -f $(BLD_BIN_DIR)/.ideBuild ] ; then \
		rm -f $(BLD_BIN_DIR)/* $(BLD_BIN_DIR)/.ideBuild ; \
		make -C ../build/src ; \
	fi

.SUFFIXES: .h

$(BLD_INC_DIR)/%.h: slots/%.h
	cp $< $@

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
#
