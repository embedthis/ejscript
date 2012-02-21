#
#	Makefile for the jem command
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 	.makedep

NAME		= ejs.jem
TARGETS		+= $(BLD_BIN_DIR)/jem $(BLD_BIN_DIR)/jem.cmd

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/jem: jem.es
	$(call log) "[Generate]" jem
	cp jem.es $(BLD_BIN_DIR)/jem
	chmod 755 $(BLD_BIN_DIR)/jem

$(BLD_BIN_DIR)/jem.cmd: jem.cmd
	$(call log) "[Generate]" "jem.cmd"
	cp jem.cmd $(BLD_BIN_DIR)/jem.cmd
	chmod 755 $(BLD_BIN_DIR)/jem.cmd

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
