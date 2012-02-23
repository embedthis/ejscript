#
#	Makefile for the jem command
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 	.makedep

NAME		= ejs.jem
TARGETS		+= $(BLD_BIN_DIR)/jem.es \
			   $(BLD_BIN_DIR)/jem$(BLD_EXE)

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/jem.es: jem.es
	$(call log) "[Generate]" jem
	ejsc jem.es ; # Just do a compilation check
	cp jem.es $(BLD_BIN_DIR)/jem.es

$(BLD_BIN_DIR)/jem$(BLD_EXE): $(BLD_BIN_DIR)/ejsrun$(BLD_EXE)
	$(call log) "[Create]" "jem$(BLD_EXE)"
	cp $(BLD_BIN_DIR)/ejsrun$(BLD_EXE) $(BLD_BIN_DIR)/jem$(BLD_EXE)
	chmod 755 $(BLD_BIN_DIR)/jem$(BLD_EXE)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
