#
#	Makefile for the ejs.web - Web Framework
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 	.makedep

NAME		= ejs.web
MOD			= $(NAME).mod
JEM			= $(NAME)-$(BLD_VERSION)
POST_DIRS	= src js
MODULE		+= $(BLD_LIB_DIR)/$(NAME).mod 
TARGETS		+= $(MODULE) $(BLD_LIB_DIR)/www

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

$(BLD_LIB_DIR)/www: www/*
	$(call log) [Copy] "www to $(BLD_LIB_DIR)/www"
	rm -fr $(BLD_LIB_DIR)/www
	mkdir -p $(BLD_LIB_DIR)/www/images
	mkdir -p $(BLD_LIB_DIR)/www/themes
	cp -r www/* $(BLD_LIB_DIR)/www

cleanExtra:
	rm -rf $(BLD_LIB_DIR)/www $(MODULE)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
