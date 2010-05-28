#
#	Makefile for the Ejscript source code.
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

LISTING			+= --listing
PRE_DIRS		+= include deps vm compiler core
POST_DIRS		+= jems
EJS_MOD			+= $(call mod, ejs)
EJS_FILES		+= $(shell echo core/*.es | sort)
EJS_LIB			+= $(BLD_LIB_DIR)/libejs$(BLD_LIB)
EJS_SOURCES		+= $(shell find core/src vm compiler -name '*.c') 
EJS_OBJECTS		+= $(patsubst %.c,$(BLD_OBJ_DIR)/%$(BLD_OBJ),$(notdir $(EJS_SOURCES)))

include 		.makedep

compileExtra: $(EJS_LIB) ejsc $(EJS_MOD)

#	compileFinal: 
#		@+T=modules ; D="deps/appweb" ; $(DO_RECURSE) 

$(EJS_LIB): $(EJS_SOURCES) $(EJS_OBJECTS) $(call lib, mpr)
	bld --library $(EJS_LIB) --libs "$(BLD_EJS_WITHLIBS)" $(EJS_OBJECTS)

ejsc:
	@+T=compile ; D="cmd" ; $(DO_RECURSE) 

$(EJS_MOD): $(EJS_FILES)
	$(call log) [Compile] "Core system types"
	rm -f *.mod *.slots.h
	$(BLD_BUILD_BIN_DIR)/ejsc $(_ESFLAGS) --out ejs.mod --bind --require "" $(EJS_FILES)
	mv *.mod $(BLD_MOD_DIR)
	$(call log) [Generate] "Core listings and slot files"
	$(BLD_BUILD_BIN_DIR)/ejsmod --showDebug --require "" $(LISTING) --cslots $(call mod, ejs)
	for f in *.slots.h ; do \
		diff $$f $(BLD_INC_DIR) >/dev/null 2>&1 || (printf "%12s %s\n" "[Install]" $$f ; cp $$f $(BLD_INC_DIR)) ; rm -f $$f;\
	done
	if [ "$(RETRY)" = "" ] ; then \
		$(MAKE) TRACE=$(TRACE) RETRY=1 ; \
	fi

dependExtra:
	@+T=depend ; D="cmd" ; $(DO_RECURSE) 

cleanExtra:
	find . -name '*.lst' -o -name '*.mod' -o -name '*.jem' -type f | xargs rm -f
	rm -f *.slots.h

makeListForVs:
	find . -name '*.es' | egrep -v test\/|sample\/|Tar.es|jem.es|ejsmvc|ejspage|sample|jsgi
