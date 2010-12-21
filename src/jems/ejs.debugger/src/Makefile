#
#	Makefile to build the Debugger
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 			.makedep

JEM				:= ejs.debugger

TARGETS			+= $(BLD_MOD_DIR)/$(JEM)$(BLD_LIB)

compileExtra:   $(TARGETS)

$(BLD_MOD_DIR)/$(JEM)$(BLD_LIB): $(OBJECTS) $(call lib, mpr) $(call vlib, ejs)
	bld --library $(BLD_MOD_DIR)/$(JEM)$(BLD_LIB) --libs "ejs $(BLD_MPR_LIBS)" \
        $(BLD_OBJ_DIR)/ejsDebugger$(BLD_OBJ)



#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4
#
