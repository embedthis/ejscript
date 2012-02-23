#
#	Makefile for the ejs.mvc jem
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 	.makedep

NAME		= ejs.mvc
MOD			= $(NAME).mod
JEM			= $(NAME)-$(BLD_VERSION)
TARGETS		+= $(BLD_BIN_DIR)/mvc.es \
			   $(BLD_BIN_DIR)/mvc$(BLD_EXE)

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/mvc.es: mvc.es
	$(call log) "[Export]" "mvc.es"
	ejsc mvc.es ; # Just to do a compilation check
	cp mvc.es $(BLD_BIN_DIR)/mvc.es
            
$(BLD_BIN_DIR)/mvc$(BLD_EXE): $(BLD_BIN_DIR)/ejsrun$(BLD_EXE)
	$(call log) "[Create]" "mvc$(BLD_EXE)"
	cp $(BLD_BIN_DIR)/ejsrun$(BLD_EXE) $(BLD_BIN_DIR)/mvc$(BLD_EXE)
	chmod 755 $(BLD_BIN_DIR)/mvc$(BLD_EXE)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
