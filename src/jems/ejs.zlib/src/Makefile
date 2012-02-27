#
#	Makefile to build the Zlib Class
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 		.makedep

JEM				:= ejs.zlib

ifeq ($(BLD_FEATURE_ZLIB),1)
	TARGETS		+= $(BLD_LIB_DIR)/$(JEM)$(BLD_LIB)
endif

compileExtra:   $(TARGETS)

$(BLD_LIB_DIR)/$(JEM)$(BLD_LIB): $(OBJECTS) $(call lib, mpr) $(call vlib, ejs)
	bld --library $(BLD_LIB_DIR)/$(JEM)$(BLD_LIB) --libs "ejs $(BLD_MPR_LIBS) $(BLD_ZLIB_LIBS)" \
        $(BLD_OBJ_DIR)/ejsZlib$(BLD_OBJ)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
