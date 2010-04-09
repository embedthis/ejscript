#
#	Makefile for the native ejs.cache code
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#
#
include 	.makedep

JEM			:= ejs.cache
TARGET		:= $(BLD_MOD_DIR)/$(JEM)$(BLD_LIB)

compileExtra:	$(TARGET)

$(TARGET): $(OBJECTS) $(call lib, mpr) $(call vlib, ejs)
	bld --library $(TARGET) --libs "ejs $(BLD_MPR_LIBS)" $(OBJECTS)
