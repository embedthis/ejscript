#
#	Makefile for the ejs.zlib - Zlib support
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 	.makedep

PRE_DIRS	= src
NAME		= ejs.zlib
MOD			= $(NAME).mod
JEM			= $(NAME)-$(BLD_VERSION)

ifeq		($(BLD_FEATURE_ZLIB),1)
	MODULE	+= $(BLD_LIB_DIR)/$(NAME).mod 
	TARGETS	= $(MODULE)
endif

compileExtra: $(TARGETS)

module: $(MODULE)

$(BLD_LIB_DIR)/$(MOD): *.es $(BLD_LIB_DIR)/ejs.mod
	$(call log) [Compile] "$(MOD)"
	rm -f *.mod *.slots.h $(BLD_BUILD_LIB_DIR)/$(MOD) $(BLD_LIB_DIR)/$(MOD)
	ejsc $(_ESFLAGS) *.es
	ejsmod --cslots $(MOD)
	diff $(NAME).slots.h $(BLD_INC_DIR) >/dev/null 2>&1 || cp $(NAME).slots.h $(BLD_INC_DIR) 
	cp *.mod $(BLD_BUILD_LIB_DIR) ; mv *.mod $(BLD_LIB_DIR)
	rm -f $(NAME).slots.h

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
