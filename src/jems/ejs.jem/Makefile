#
#	Makefile for the jem command
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 	.makedep

TARGET		+= $(BLD_BIN_DIR)/jem

compileExtra: $(TARGET)

$(TARGET): jem
	$(call log) "[Generate]" $(TARGET)
	cp jem $(TARGET)
	chmod 755 $(TARGET)

build:
	jem build .
