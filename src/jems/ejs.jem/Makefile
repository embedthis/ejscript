#
#	Makefile for the jem command
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 	.makedep

TARGETS		+= $(BLD_BIN_DIR)/jem
TARGETS		+= $(BLD_BIN_DIR)/jem.cmd

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/jem: jem
	$(call log) "[Generate]" $(BLD_BIN_DIR)/jem
	cp jem $(BLD_BIN_DIR)/jem
	chmod 755 $(BLD_BIN_DIR)/jem

$(BLD_BIN_DIR)/jem.cmd: jem.cmd
	$(call log) "[Generate]" "jem.cmd"
	cp jem.cmd $(BLD_BIN_DIR)/jem.cmd
	chmod 755 $(BLD_BIN_DIR)/jem.cmd
