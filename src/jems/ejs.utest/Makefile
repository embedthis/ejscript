#
#	Makefile for the utest command
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 	.makedep

NAME		= ejs.utest
TARGETS		+= $(BLD_BIN_DIR)/utest.es \
			   $(BLD_BIN_DIR)/utest.worker \
			   $(BLD_BIN_DIR)/utest$(BLD_EXE) \

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/utest.es: utest.es
	$(call log) "[Generate]" utest.es
	cp utest.es $(BLD_BIN_DIR)/utest.es

$(BLD_BIN_DIR)/utest.worker: utest.worker
	$(call log) "[Generate]" utest.worker
	cp utest.worker $(BLD_BIN_DIR)/utest.worker


$(BLD_BIN_DIR)/utest$(BLD_EXE): $(BLD_BIN_DIR)/ejsrun$(BLD_EXE)
	$(call log) "[Create]" "utest$(BLD_EXE)"
	cp $(BLD_BIN_DIR)/ejsrun$(BLD_EXE) $(BLD_BIN_DIR)/utest$(BLD_EXE)
	chmod 755 $(BLD_BIN_DIR)/utest$(BLD_EXE)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
