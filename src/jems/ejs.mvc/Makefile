#
#	Makefile for the ejs.mvc jem
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 	.makedep

TARGETS		+= $(BLD_BIN_DIR)/mvc
TARGETS		+= $(BLD_BIN_DIR)/mvc.cmd

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/mvc: mvc
	$(call log) "[Generate]" "mvc"
	ejsc mvc ; # Just to do a compilation check
	cp mvc $(BLD_BIN_DIR)/mvc
	chmod 755 $(BLD_BIN_DIR)/mvc

$(BLD_BIN_DIR)/mvc.cmd: mvc.cmd
	$(call log) "[Generate]" "mvc.cmd"
	cp mvc.cmd $(BLD_BIN_DIR)/mvc.cmd
	chmod 755 $(BLD_BIN_DIR)/mvc.cmd

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
