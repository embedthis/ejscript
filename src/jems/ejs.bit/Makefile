#
#	Makefile for the bit command
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 	.makedep

NAME		= ejs.bit
TARGETS		+= $(BLD_BIN_DIR)/bit.es $(BLD_BIN_DIR)/bit$(BLD_EXE)

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/bit.es: bit.es
	$(call log) "[Generate]" bit.es
	cp bit.es $(BLD_BIN_DIR)/bit.es

$(BLD_BIN_DIR)/bit$(BLD_EXE): $(BLD_BIN_DIR)/ejsrun$(BLD_EXE)
	$(call log) "[Create]" "bit$(BLD_EXE)"
	cp $(BLD_BIN_DIR)/ejsrun$(BLD_EXE) $(BLD_BIN_DIR)/bit$(BLD_EXE)
	chmod 755 $(BLD_BIN_DIR)/bit$(BLD_EXE)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
