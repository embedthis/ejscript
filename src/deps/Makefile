#
#	Makefile for the Ejscript dependencies.
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep

PRE_DIRS    	+= mpr pcre http sqlite
EJSMAN          := $(BLD_BIN_DIR)/ejsman$(BLD_EXE)
EHTTP           := $(BLD_BIN_DIR)/ehttp$(BLD_EXE)

ifneq ($(BLD_OS),VXWORKS)
	TARGETS		+= $(EJSMAN)
endif
TARGETS			+= $(EHTTP)

compileExtra:   $(TARGETS)

$(EJSMAN): $(BLD_BIN_DIR)/manager$(BLD_EXE)
	cp $(BLD_BIN_DIR)/manager$(BLD_EXE) $(EJSMAN)

$(EHTTP): $(BLD_BIN_DIR)/http$(BLD_EXE)
	cp $(BLD_BIN_DIR)/http$(BLD_EXE) $(EHTTP)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
