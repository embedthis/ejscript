#
#	Makefile for the native ejs.store.local code
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#
#
include 	.makedep

JEM			:= ejs.store.local
TARGET		:= $(BLD_MOD_DIR)/$(JEM)$(BLD_LIB)
MAKE_IFLAGS  += $(BLD_MPR_IFLAGS) $(BLD_HTTP_IFLAGS)

compileExtra:	$(TARGET)

$(TARGET): $(OBJECTS) $(call lib, mpr) $(call vlib, ejs)
	bld --library $(TARGET) --libs "ejs $(BLD_MPR_LIBS)" $(OBJECTS)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
