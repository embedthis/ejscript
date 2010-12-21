#
#	Makefile for the Ejscript web framework
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#
#
include 		.makedep

JEM				:= ejs.web
TARGET			:= $(BLD_MOD_DIR)/$(JEM)$(BLD_LIB)

compileExtra:	$(TARGET)

$(TARGET): $(OBJECTS) $(call lib, mpr) $(call vlib, ejs) $(call mod, ejs, ejs.web)
	bld --library $(TARGET) --libs "ejs http $(BLD_SSL_LIBS) $(BLD_MPR_LIBS)" $(OBJECTS)
